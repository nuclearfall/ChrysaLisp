;imports
(import 'sys/lisp.inc)
(import 'gui/lisp.inc)

(structure 'event 0
	(byte 'win_close))

(structure 'work 0
	(long 'parent_id)
	(long 'width)
	(long 'height)
	(long 'y))

(structure 'reply 0
	(long 'child_id)
	(int 'y)
	(offset 'data))

(defq canvas_width 800 canvas_height 800 canvas_scale 1 id t)

(ui-tree window (create-window window_flag_close) nil
	(ui-element canvas (create-canvas canvas_width canvas_height canvas_scale)))

(canvas-swap (canvas-fill (view-set-flags canvas view_flag_opaque view_flag_opaque) argb_black))
(gui-add (apply view-change (cat (list window 310 64)
	(view-pref-size (window-set-title (window-connect-close window event_win_close) "Raymarch")))))

;open farm and send out first batch of work
(defq sw (mul canvas_width canvas_scale) sh (mul canvas_height canvas_scale) sy 0 sc 0
	farm (open-farm "apps/raymarch/child.lisp" (mul (kernel-total) 2) kn_call_child))
(while (and (lt sy sh) (lt sy (length farm)))
	(mail-send (array (task-mailbox) sw sh sy) (elem sy farm))
	(setq sc (inc sc) sy (inc sy)))

(while id
	(cond
		((eq (setq id (get-long (defq msg (mail-mymail)) ev_msg_target_id)) event_win_close)
			(setq id nil))
		((gt id 0)
			(setq sc (dec sc))
			(defq x -1 y (get-int msg reply_y))
			(while (lt (setq x (inc x)) sw)
				(canvas-set-color canvas (get-int msg (add reply_data (mul x int_size))))
				(canvas-plot canvas x y))
			(canvas-swap canvas)
			(cond
				((lt sy sh)
					;can pass out more work
					(mail-send (array (task-mailbox) sw sh sy) id)
					(setq sc (inc sc) sy (inc sy)))
				((eq sc 0)
					;send out multi-cast exit command
					(defq exit (char 0 long_size))
					(while (defq mbox (pop farm))
						(mail-send exit mbox)))))
		(t (view-event window msg))))

;wait for outstanding replies
(setq window nil)
(while (ne sc 0)
	(if (gt (get-long (defq msg (mail-mymail)) ev_msg_target_id) 0)
		(setq sc (dec sc))))

;send out multi-cast exit command
(defq exit (char 0 long_size))
(while (defq mbox (pop farm))
	(mail-send exit mbox))
