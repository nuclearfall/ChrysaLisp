(include 'class/seq/class.inc)

(def-class 'str 'seq)
(dec-method 'vtable 'class/str/vtable)
(dec-method 'create_from_buffer 'class/str/create_from_buffer 'static '(r0 r1) '(r0))
(dec-method 'create_from_cstr 'class/str/create_from_cstr 'static '(r0) '(r0))
(dec-method 'create_from_file 'class/str/create_from_file 'static '(r0) '(r0))
(dec-method 'create_from_long 'class/str/create_from_long 'static '(r0 r1) '(r0))
(dec-method 'append 'class/str/append 'static '(r0 r1) '(r0))
(dec-method 'init 'class/str/init 'static '(r0 r1 r2 r3))
(dec-method 'init1 'class/str/init1 'static '(r0 r1 r2 r3) '(r0 r1))
(dec-method 'init2 'class/str/init2 'static '(r0 r1 r2 r3) '(r0 r1))
(dec-method 'init3 'class/str/init3 'static '(r0 r1 r2) '(r0 r1))
(dec-method 'split 'class/str/split 'static '(r0 r1) '(r0 r1))
(dec-method 'compare 'class/str/compare 'static '(r0 r1) '(r0 r1))
(dec-method 'same 'class/str/same 'static '(r0 r1) '(r0 r1))
(dec-method 'find 'class/str/find 'static '(r0 r1) '(r0 r1))

(dec-method 'hash 'class/str/hash 'final)
(dec-method 'get_length 'class/str/get_length 'final)
(dec-method 'ref_element 'class/str/ref_element 'final)
(dec-method 'slice 'class/str/slice 'final)
(dec-method 'cat 'class/str/cat 'final)

(dec-method 'lisp_str 'class/str/lisp_str 'static '(r0 r1) '(r0 r1))
(dec-method 'lisp_split 'class/str/lisp_split 'static '(r0 r1) '(r0 r1))
(dec-method 'lisp_code 'class/str/lisp_code 'static '(r0 r1) '(r0 r1))
(dec-method 'lisp_char 'class/str/lisp_char 'static '(r0 r1) '(r0 r1))
(dec-method 'lisp_cmp 'class/str/lisp_cmp 'static '(r0 r1) '(r0 r1))
(dec-method 'lisp_save 'class/str/lisp_save 'static '(r0 r1) '(r0 r1))
(dec-method 'lisp_load 'class/str/lisp_load 'static '(r0 r1) '(r0 r1))

(def-struct 'str 'seq)
	(uint 'length)
	(uint 'hashcode)
	(local-align)
	(offset 'data)
(def-struct-end)

(defcvar 'str_gap (sub 64 str_data ptr_size byte_size))

(defcvar 'char_backspace 8)
(defcvar 'char_tab 9)
(defcvar 'char_lf 10)
(defcvar 'char_cr 13)
(defcvar 'char_esc 27)
(defcvar 'char_space (ascii " "))
(defcvar 'char_underscore (ascii "_"))
(defcvar 'char_pipe (ascii "|"))
(defcvar 'char_lrb (ascii "("))
(defcvar 'char_rrb (ascii ")"))
(defcvar 'char_lsb (ascii "["))
(defcvar 'char_rsb (ascii "]"))
(defcvar 'char_lcb (ascii "{"))
(defcvar 'char_rcb (ascii "}"))
(defcvar 'char_quote (ascii "'"))
(defcvar 'char_double_quote 34)
(defcvar 'char_tick (ascii "`"))
(defcvar 'char_comma (ascii ","))
(defcvar 'char_tilda (ascii "~"))
(defcvar 'char_semi (ascii ";"))
(defcvar 'char_dot (ascii "."))
(defcvar 'char_at (ascii "@"))
(defcvar 'char_lt (ascii "<"))
(defcvar 'char_gt (ascii ">"))
(defcvar 'char_equal (ascii "="))
(defcvar 'char_plus (ascii "+"))
(defcvar 'char_minus (ascii "-"))
(defcvar 'char_multiply (ascii "*"))
(defcvar 'char_divide (ascii "/"))
(defcvar 'char_0 (ascii "0"))
(defcvar 'char_9 (ascii "9"))
(defcvar 'char_a (ascii "a"))
(defcvar 'char_z (ascii "z"))
(defcvar 'char_A (ascii "A"))
(defcvar 'char_Z (ascii "Z"))
(defcvar 'char_x (ascii "x"))
(defcvar 'char_b (ascii "b"))
(defcvar 'char_o (ascii "o"))

;;;;;;;;;;;;;;;
;inline methods
;;;;;;;;;;;;;;;

(defcfun class/str/get_length ()
	;inputs
	;r0 = string object (ptr)
	;outputs
	;r0 = string object (ptr)
	;r1 = string length (bytes)
	(vp-cpy-ir-ui r0 str_length r1))