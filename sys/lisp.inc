;import symbols
(import "sys/symbols.inc")

(ffi load-path "sys/load/lisp_path" 0)
(ffi task-mailbox "sys/task/lisp_mailbox" 0)
(ffi task-sleep "sys/task/lisp_sleep" 0)
(ffi mail-alloc-mbox "sys/mail/lisp_alloc_mbox" 0)
(ffi mail-declare "sys/mail/lisp_declare" 0)
(ffi mail-nodes "sys/mail/lisp_devices" 0)
(ffi mail-enquire "sys/mail/lisp_enquire" 0)
(ffi mail-forget "sys/mail/lisp_forget" 0)
(ffi mail-free-mbox "sys/mail/lisp_free_mbox" 0)
(ffi mail-poll "sys/mail/lisp_poll" 0)
(ffi mail-read "sys/mail/lisp_read" 0)
(ffi mail-select "sys/mail/lisp_select" 0)
(ffi mail-send "sys/mail/lisp_send" 0)
(ffi mem-stats "sys/mem/lisp_stats" 0)

;some simple structure helpers
(defmacro structure (n o &rest b)
	; (structure name offset [body])
	`(progn (defq _e (env) _n ,n _o ,(eval o)) ~b (def _e (sym (cat _n "_size")) _o '_e nil)))
(defun struct (s &rest b)
	; (struct size field ...)
	(each (lambda (_) (def _e (sym (cat _n "_" _)) _o) (setq _o (+ _o s))) b))
(defun offset (&rest b)
	; (offset field ...)
	(apply struct (cat (list 0) b)))
(defun ptr (&rest b)
	; (ptr field ...)
	(setq _o (align _o ptr_size)) (apply struct (cat (list ptr_size) b)))
(defun long (&rest b)
	; (long field ...)
	(setq _o (align _o long_size)) (apply struct (cat (list long_size) b)))
(defun int (&rest b)
	; (int field ...)
	(setq _o (align _o int_size)) (apply struct (cat (list int_size) b)))
(defun short (&rest b)
	; (short field ...)
	(setq _o (align _o short_size)) (apply struct (cat (list short_size) b)))
(defun byte (&rest b)
	; (byte field ...)
	(setq _o (align _o byte_size)) (apply struct (cat (list byte_size) b)))
(defun nodeid (&rest b)
	; (net_id field ...)
	(setq _o (align _o long_size)) (apply struct (cat (list node_id_size) b)))
(defun netid (&rest b)
	; (net_id field ...)
	(setq _o (align _o long_size)) (apply struct (cat (list net_id_size) b)))

(defun to-net-id (sid)
	; (to-net-id service_id) -> net_id
	(bind '(e o) (unzip sid (list (list) (list))))
	(apply cat (map (# (char
		(+ (- (code %0) (ascii-code "a"))
		(<< (- (code %1) (ascii-code "a")) 4)))) e o)))

(defun to-service-id (nid)
	; (to-service-id net_id) -> service_id
	(apply cat (map (# (cat
		(char (+ (logand 0x0f (code %0)) (ascii-code "a")))
		(char (+ (logand 0x0f (>> (code %0) 4)) (ascii-code "a"))))) nid)))

(defun open-child (task mode)
	; (open-child task mode) -> str
	(defq mbox (mail-alloc-mbox))
	(mail-send (cat (char 0 (const long_size)) (slice (const long_size) -1 (task-mailbox)))
		(cat
			(char 0 (const long_size))
			mbox
			(char mode (const long_size))
			task (char 0)))
	(defq id (slice (const long_size) (const (+ long_size net_id_size)) (mail-read mbox)))
	(mail-free-mbox mbox) id)

(defun open-remote (task node mode)
	; (open-remote task node mode) -> str
	(defq mbox (mail-alloc-mbox))
	(mail-send (cat (char 0 (const long_size)) node)
		(cat
			(char 0 (const long_size))
			mbox
			(char mode (const long_size))
			task (char 0)))
	(defq id (slice (const long_size) (const (+ long_size net_id_size)) (mail-read mbox)))
	(mail-free-mbox mbox) id)

(defun open-farm (task num mode &optional devices)
	; (open-farm task num mode [devices]) -> (list str ...)
	(if (= 0 (length (defq devices (opt devices (mail-nodes)))))
		(setq devices (list (slice (const long_size) -1 (task-mailbox)))))
	(defq ids (cap num (list)) mbox (mail-alloc-mbox) index -1
		center (+ (/ num 2) (length devices) (opt (find (slice (const long_size) -1 (task-mailbox)) devices) 0)))
	(while (< (setq index (inc index)) num)
		(mail-send (cat (char 0 (const long_size)) (elem (% (+ index center) (length devices)) devices))
			(cat
				(char index (const long_size))
				mbox
				(char mode (const long_size))
				task (char 0))))
	(while (>= (setq index (dec index)) 0)
		(defq msg (mail-read mbox))
		(push ids (slice (const long_size) (const (+ long_size net_id_size)) msg)))
	(mail-free-mbox mbox) ids)

(defun open-pipe (tasks)
	; (open-pipe tasks) -> (list str ...)
	(defq devices (mail-nodes) ids (cap (length tasks) (list))
		mbox (mail-alloc-mbox) index -1 tail (task-mailbox))
	(while (< (setq index (inc index)) (length tasks))
		(mail-send (cat (char 0 (const long_size)) (slice (const long_size) -1 tail))
			(cat
				(char index (const long_size))
				mbox
				(char kn_call_child (const long_size))
				(elem index tasks) (char 0)))
		(defq tail (slice (const long_size) (const (+ long_size net_id_size)) (mail-read mbox)))
		(push ids tail))
	(mail-free-mbox mbox) ids)
