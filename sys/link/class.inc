(include "sys/list/class.inc")
(include "sys/mail/class.inc")
(include "class/str/class.inc")

(def-class 'sys_link)
(dec-method :in 'sys/link/in)
(dec-method :out 'sys/link/out)

;;;;;;;;;;;;;;;;;
; link structures
;;;;;;;;;;;;;;;;;

;msg data area max size when allocated as a string object
;allowing for heap pointer and null byte, rounded down for faster memcpy
(defcvar 'lk_data_size (- 512 str_size ptr_size long_size) 'lk_page_size 4096)

(def-enum 'lk_chan_status)
	(enum 'ready 'busy)
(def-enum-end)

(def-struct 'lk_node)
	(struct 'peer_node_id 'node_id)
	(uint 'task_count)
(def-struct-end)

(def-struct 'lk_msg)
	(uint 'status 'hash)
	(struct 'peer_node_id 'node_id)
	(uint 'task_count)
	(local-align)
	(struct 'stamp msg_stamp_size)
	(struct 'data lk_data_size)
(def-struct-end)

(def-struct 'lk_chan)
	(struct 'msg0 'lk_msg)
	(struct 'msgs (* (/ (- (- (* lk_page_size 2) long_size) *struct_sp*) lk_msg_size) lk_msg_size))
(def-struct-end)

(def-struct 'lk_buffer)
	(struct 'chan_1 'lk_chan)
	(ulong 'towel)
	(local-align lk_page_size)
	(struct 'chan_2 'lk_chan)
(def-struct-end)
