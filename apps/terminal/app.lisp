;imports
(import "sys/lisp.inc")
(import "gui/lisp.inc")
(import "lib/pipe/pipe.inc")
(import "apps/terminal/input.inc")

(structure '+event 0
	(byte 'close+ 'max+ 'min+)
	(byte 'layout+ 'scroll+))

(defq cmd nil vdu_width 60 vdu_height 40 vdu_min_width 16 vdu_min_height 16 text_buf (list ""))

(ui-window mywindow (:color 0xc0000000)
	(ui-flow _ (:flow_flags +flow_down_fill+)
		(ui-title-bar _ "Terminal" (0xea19 0xea1b 0xea1a) +event_close+)
		(ui-flow _ (:flow_flags +flow_left_fill+)
			(. (ui-slider slider) :connect +event_scroll+)
			(ui-vdu vdu (:vdu_width vdu_width :vdu_height vdu_height :min_width vdu_width :min_height vdu_height
				:ink_color +argb_green+)))))

(defun vdu-print (vdu buf s)
	(each (lambda (c)
		(cond
			((eql c (ascii-char 10))
				;line feed and truncate
				(if (> (length (push buf "")) *env_terminal_lines*)
					(setq buf (slice (const (dec (neg *env_terminal_lines*))) -1 buf))))
			((eql c (ascii-char 128))
				;clear line
				(elem-set -2 buf ""))
			((eql c (ascii-char 9))
				;tab
				(elem-set -2 buf (cat (defq l (elem -2 buf)) (slice 0 (- 4 (logand (length l) 3)) "    "))))
			(t	;char
				(elem-set -2 buf (cat (elem -2 buf) c))))) s)
	;set cursor and offset
	(defq cx (if cmd *line_pos* (+ (length *env_terminal_prompt*) *line_pos*))
		cy (dec (length buf)) ox 0 oy 0)
	(cond
		((< cx ox)
			(setq ox cx))
		((>= cx (+ ox vdu_width))
			(setq ox (- cx vdu_width -1))))
	(cond
		((< cy oy)
			(setq oy cy))
		((>= cy (+ oy vdu_height))
			(setq oy (- cy vdu_height -1))))
	;set slider values
	(def slider :maximum (max 0 (- (length buf) vdu_height)) :portion vdu_height :value oy)
	(. slider :dirty)
	(. vdu :load buf ox oy cx cy) buf)

;override print for VDU output
(defun print (_)
	(setq text_buf (vdu-print vdu text_buf _)))

(defun print-edit-line ()
	(print (cat (ascii-char 128) (if cmd "" *env_terminal_prompt*) *line_buf*)))

(defun terminal-input (c)
	(line-input c)
	(cond
		((or (= c 10) (= c 13))
			;enter key
			(print-edit-line)
			(print (ascii-char 10))
			(defq cmdline *line_buf*)
			(line-clear)
			(cond
				(cmd
					;feed active pipe
					(pipe-write cmd (cat cmdline (ascii-char 10))))
				((/= (length cmdline) 0)
					;new pipe
					(catch (setq cmd (pipe-open cmdline)) (progn (setq cmd nil) t))
					(cond
						(cmd
							(. mywindow :dirty_all))
						(t
							(print (cat (const (cat "Pipe Error !" (ascii-char 10)))))
							(print-edit-line))))
				(t	;empty cmdline
					(print-edit-line))))
		((= c 27)
			;esc key
			(when cmd
				;feed active pipe, then EOF
				(when (/= (length *line_buf*) 0)
					(pipe-write cmd *line_buf*))
				(pipe-close cmd) (setq cmd nil) (line-clear)
				(. mywindow :dirty_all)
				(print-edit-line)))
		(t	;some key
			(print-edit-line))))

(defun window-resize (w h)
	(setq vdu_width w vdu_height h)
	(set vdu :vdu_width w :vdu_height h :min_width w :min_height h)
	(bind '(x y w h) (apply view-fit
		(cat (. mywindow :get_pos) (. mywindow :pref_size))))
	(set vdu :min_width vdu_min_width :min_height vdu_min_height)
	(. mywindow :change_dirty x y w h)
	(print-edit-line))

(defun window-layout (w h)
	(setq vdu_width w vdu_height h)
	(set vdu :vdu_width w :vdu_height h :min_width w :min_height h)
	(bind '(x y) (. vdu :get_pos))
	(bind '(w h) (. vdu :pref_size))
	(set vdu :min_width vdu_min_width :min_height vdu_min_height)
	(. vdu :change x y w h)
	(print-edit-line))

(defun main ()
	;add window
	(bind '(x y w h) (apply view-locate (.-> mywindow (:connect +event_layout+) :pref_size)))
	(gui-add (. mywindow :change x y w h))
	;sign on msg
	(print (str "ChrysaLisp Terminal 1.6" (ascii-char 10)))
	(print-edit-line)
	;main event loop
	(while (progn
		(defq data t)
		(if cmd (setq data (pipe-read cmd)))
		(cond
			((eql data t)
				;normal mailbox event
				(cond
					((= (defq id (get-long (defq msg (mail-read (task-mailbox))) ev_msg_target_id)) +event_close+)
						nil)
					((= id +event_layout+)
						;user window resize
						(apply window-layout (. vdu :max_size)))
					((= id +event_min+)
						;min button
						(window-resize 60 40))
					((= id +event_max+)
						;max button
						(window-resize 120 40))
					((= id +event_scroll+)
						;user scroll bar
						(defq cx (if cmd *line_pos* (+ (length *env_terminal_prompt*) *line_pos*))
							cy (dec (length text_buf)))
						(. vdu :load text_buf 0 (get :value slider) cx cy))
					(t	;gui event
						(. mywindow :event msg)
						(and (= (get-long msg ev_msg_type) ev_type_key)
							(> (get-int msg ev_msg_key_keycode) 0)
							(terminal-input (get-int msg ev_msg_key_key)))
						t)))
			((eql data nil)
				;pipe is closed
				(pipe-close cmd)
				(setq cmd nil)
				(print (cat (ascii-char 10) *env_terminal_prompt* *line_buf*))
				(. mywindow :dirty_all))
			(t	;string from pipe
				(print data)))))
	;close window and pipe
	(. mywindow :hide)
	(if cmd (pipe-close cmd)))
