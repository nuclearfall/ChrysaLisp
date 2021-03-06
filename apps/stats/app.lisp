;imports
(import "sys/lisp.inc")
(import "class/lisp.inc")
(import "gui/lisp.inc")

(structure '+event 0
	(byte 'close+))

(defq stat_data (list) stat_scale (list) devices (mail-nodes) frame_cnt 0
	cpu_count (length devices) max_stats 1 last_max_stats 0
	farm (open-farm "apps/stats/child.lisp" cpu_count kn_call_open devices) last_max_classes 0 max_classes 1
	select (list (task-mailbox) (mail-alloc-mbox)) sample_msg (elem -2 select))

(ui-window mywindow ()
	(ui-title-bar _ "Object Monitor" (0xea19) +event_close+)
	(ui-grid _ (:grid_width 2 :grid_height 1 :flow_flags +flow_down_fill+ :maximum 100 :value 0)
		(ui-flow name_flow (:color +argb_grey8+)
			(ui-label _ (:text "Class" :color +argb_white+))
			(ui-grid _ (:grid_width 1 :grid_height 1 :color +argb_white+ :font *env_medium_terminal_font*)
				(ui-label _ (:text "")))
			(ui-view name_view))
		(ui-flow stat_flow (:color +argb_red+)
			(ui-label _ (:text "Count" :color +argb_white+))
			(ui-grid _ (:grid_width 4 :grid_height 1 :color +argb_white+ :font *env_medium_terminal_font*)
				(times 4 (push stat_scale (ui-label _
					(:text "|" :flow_flags (logior +flow_flag_align_vcenter+ +flow_flag_align_hright+))))))
			(ui-view stat_view))))

(defun main ()
	(while (progn
		;new batch of samples ?
		(when (= cpu_count (length devices))
			;set scales
			(setq last_max_stats max_stats max_stats 1)
			(each (lambda (stat)
				(defq vt (* (inc _) (/ (* last_max_stats 100) (length stat_scale))))
				(def stat :text (str (/ vt 100) "|"))
				(. stat :layout)) stat_scale)
			;build new stats info
			(sort (lambda (x y)
				(- (elem 1 y) (elem 1 x))) stat_data)
			(defq new_name_view (Grid) new_stat_view (Grid))
			(def new_name_view :grid_width 1 :grid_height max_classes)
			(def new_stat_view :grid_width 1 :grid_height max_classes)
			(each (lambda ((name stat))
				(defq n (Label) p (Progress))
				(def n :border 0 :text name)
				(def p :maximum last_max_stats :value stat)
				(. new_name_view :add_child n)
				(. new_stat_view :add_child p)) stat_data)
			(. stat_view :sub) (. name_view :sub)
			(. name_flow :add_child (setq name_view new_name_view))
			(. stat_flow :add_child (setq stat_view new_stat_view))
			(. name_flow :layout) (. stat_flow :layout)
			(. mywindow :dirty_all)
			;open the window once we have data
			(when (= (setq frame_cnt (inc frame_cnt)) 2)
				(bind '(x y w h) (apply view-locate (. mywindow :pref_size)))
				(gui-add (. mywindow :change x y w h)))
			;resize if number of classes change
			(when (/= last_max_classes max_classes)
				(setq last_max_classes max_classes)
				(bind '(x y) (. mywindow :get_pos))
				(bind '(w h) (. mywindow :pref_size))
				(. mywindow :change_dirty x y w h))
			;send out multi-cast sample command
			(while (/= cpu_count 0)
				(setq cpu_count (dec cpu_count))
				(mail-send (elem cpu_count farm) sample_msg))
			(clear stat_data) (setq max_classes 1))

		;wait for next event
		(defq id (mail-select select) msg (mail-read (elem id select)))
		(cond
			((= id 0)
				;main mailbox
				(cond
					((= (setq id (get-long msg ev_msg_target_id)) +event_close+)
						;close button
						nil)
					(t (. mywindow :event msg))))
			(t	;child info, merge with current frames information
				(bind '(data _) (read (string-stream msg) (ascii-code " ")))
				(setq max_classes (max max_classes (length data)))
				(each (lambda (ent)
					(bind '(name stat) ent)
					(if (defq _ (some (lambda (_) (if (eql (elem 0 _) name) _)) stat_data))
						(progn
							(elem-set 1 _ (+ (elem 1 _) stat))
							(setq max_stats (max max_stats (elem 1 _))))
						(progn
							(push stat_data ent)
							(setq max_stats (max max_stats stat))))) data)
				;count up replies
				(task-sleep 10000)
				(setq cpu_count (inc cpu_count))))))
	;close window and children
	(. mywindow :hide)
	(mail-free-mbox (pop select))
	(while (defq mbox (pop farm))
		(mail-send mbox "")))
