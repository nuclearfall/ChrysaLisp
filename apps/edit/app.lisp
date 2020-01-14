;imports
(import 'sys/lisp.inc)
(import 'class/lisp.inc)
(import 'gui/lisp.inc)

(structure 'event 0
	(byte 'win_close 'win_min 'win_max))

(defq id t vdu_width 60 vdu_height 40 cursor_x 0 cursor_y 0 offset_x 0 offset_y 0 text_buf (list))

;this will nee scrollbars etc to change the text offset in the VDU area
(ui-tree window (create-window (+ window_flag_close window_flag_min window_flag_max))  ('color 0xc0000000)
	(ui-element vdu (create-vdu) ('vdu_width vdu_width 'vdu_height vdu_height 'ink_color argb_green
		'font (create-font "fonts/Hack-Regular.ttf" 16))))

(gui-add (apply view-change (cat (list window 128 32)
	(view-pref-size (window-set-title (window-connect-close (window-connect-min
		(window-connect-max window event_win_max) event_win_min) event_win_close) "Editor - Test code for now !!!")))))

;load some temp text for now
(each-line (lambda (_)
	(push text_buf _)) (file-stream "gui/make.inc"))

(defun-bind hide-cursor ()
	(cond
		;cursor off end ?
		((>= (defq y (min cursor_y (length text_buf))) (length text_buf)))
		;not off end
		(t	(defq line (elem y text_buf))
			(cond
				;cursor off side
				((>= (defq x (min cursor_x (length line))) (length line)))
				;not off side
				(t	(defq cursor (logand (code (elem x line)) 127)
						line (erase line x (inc x)) line (insert line x (char cursor)))
					(elem-set y text_buf line))))))

(defun-bind show-cursor ()
	(cond
		;cursor off end ?
		((>= (defq y (min cursor_y (length text_buf))) (length text_buf)))
		;not off end
		(t	(defq line (elem y text_buf))
			(cond
				;cursor off side
				((>= (defq x (min cursor_x (length line))) (length line)))
				;not off side
				(t	(defq cursor (logior (code (elem x line)) 128)
						line (erase line x (inc x)) line (insert line x (char cursor)))
					(elem-set y text_buf line))))))

(defun-bind update-vdu (line_list offset_x offset_y)
	;very crude and stupid, I will do a low level method to do this in a sencible manner !
	;it will refresh the VDU with the line_buffer taking the text offsets into account
	(vdu-print vdu (const (ascii-char 128)))
	(each (lambda (_)
		(vdu-print vdu (cat _ (const (ascii-char 10))))) line_list))

(defun-bind edit-input (c)
	;insert at cursor etc, cursor pos char will need to a spare char
	;for it at the end of line and end of buffer eventually !
	(hide-cursor)
	(cond
		((or (= c 10) (= c 13))
			;return key
			(cond
				((>= cursor_y (length text_buf))
					;off end of text so just append a blank line
					(push text_buf "")
					(setq cursor_y (length text_buf)))
				(t	;break this line
					(defq line (elem cursor_y text_buf)
						line_front (slice 0 (min cursor_x (length line)) line)
						line_back (slice (min cursor_x (length line)) -1 line))
					(elem-set cursor_y text_buf line_front)
					(setq cursor_y (min (inc cursor_y) (length text_buf))
						text_buf (insert text_buf cursor_y (list line_back)))))
				(setq cursor_x 0))
		((= c 0x40000050)
			;cursor left key
			(setq cursor_x (max (dec cursor_x) 0)))
		((= c 0x4000004f)
			;cursor right key
			(defq line (if (>= cursor_y (length text_buf)) "" (elem cursor_y text_buf)))
			(setq cursor_x (min (inc cursor_x) (length line))))
		((= c 0x40000052)
			;cursor up key
			(setq cursor_y (max (dec cursor_y) 0)))
		((= c 0x40000051)
			;cursor down key
			(setq cursor_y (min (inc cursor_y) (length text_buf))))
		((= c 8)
			;backspace key
			)
		((<= 32 c 127)
			;insert the char at cursor or append to end etc
			(defq line (if (>= cursor_y (length text_buf)) "" (elem cursor_y text_buf))
				line (insert line (min cursor_x (length line)) (ascii-char c)))
			(setq cursor_x (inc cursor_x))
			(if (>= cursor_y (length text_buf))
				(push text_buf line)
				(elem-set cursor_y text_buf line))))
	(show-cursor)
	(update-vdu text_buf offset_x offset_y))

(show-cursor)
(update-vdu text_buf offset_x offset_y)
(while id
	(cond
		((= (setq id (get-long (defq msg (mail-read (task-mailbox))) ev_msg_target_id)) event_win_close)
			;close button
			(setq id nil))
		((= id event_win_min)
			;min button
			(bind '(x y _ _) (view-get-bounds window))
			(bind '(w h) (view-pref-size window))
			(view-change-dirty window x y w h))
		((= id event_win_max)
			;max button
			(bind '(x y _ _) (view-get-bounds window))
			(bind '(w h) (view-pref-size window))
			(view-change-dirty window x y (fmul w 1.75) h))
		(t	;it's a GUI event
			(view-event window msg)
			(and (= (get-long msg ev_msg_type) ev_type_key)
				(> (get-int msg ev_msg_key_keycode) 0)
				(edit-input (get-int msg ev_msg_key_key))))))

(view-hide window)
