(include "sys/class.inc")

;;;;;;;;;;;;;;;;;;;
; math static class
;;;;;;;;;;;;;;;;;;;

(def-class 'sys_math)
(dec-method :i_rand 'sys/math/i_rand :static '(r0) '(r0))
(dec-method :i_sqrt 'sys/math/i_sqrt :static '(r0) '(r0))

(dec-method :f_sqrt 'sys/math/f_sqrt :static '(r0) '(r0))
(dec-method :f_sin 'sys/math/f_sin :static '(r0) '(r0))
(dec-method :f_cos 'sys/math/f_cos :static '(r0) '(r0))
(dec-method :f_intersect 'sys/math/f_intersect :static '(r0 r1 r2 r3 r4 r5 r6 r7) '(r0 r1))
(dec-method :f_dist_sqd 'sys/math/f_dist_sqd :static '(r0 r1 r2 r3 r4 r5) '(r0))

(dec-method :r_pack 'sys/math/r_pack :static '(r13 r14) '(r13))
(dec-method :r_add 'sys/math/r_add :static '(r13 r14) '(r13))
(dec-method :r_sub 'sys/math/r_sub :static '(r13 r14) '(r13))
(dec-method :r_mul 'sys/math/r_mul :static '(r13 r14) '(r13))
(dec-method :r_div 'sys/math/r_div :static '(r13 r14) '(r13))
(dec-method :r_mod 'sys/math/r_mod :static '(r13 r14) '(r13))
(dec-method :r_frac 'sys/math/r_frac :static '(r13) '(r13))
(dec-method :r_floor 'sys/math/r_floor :static '(r13) '(r13))
(dec-method :r_i2r 'sys/math/r_i2r :static '(r14) '(r13))
(dec-method :r_f2r 'sys/math/r_f2r :static '(r14) '(r13))
(dec-method :r_r2i 'sys/math/r_r2i :static '(r13) '(r14))
(dec-method :r_r2f 'sys/math/r_r2f :static '(r13) '(r14))

;;;;;;;;;;;;;;;;
; inline methods
;;;;;;;;;;;;;;;;

(defun sys/math/f_sqrt ()
	;inputs
	;r0 = number (fixed)
	;outputs
	;r0 = sqrt (fixed)
	;trashes
	;r0-r3
	(vp-shl-cr +fp_shift+ r0)
	(call 'sys_math :i_sqrt '(r0) '(r0)))

;;;;;;;;;;;;;;;;;;;;;
; vector math codegen
;;;;;;;;;;;;;;;;;;;;;

(defcvar '*vsp* 0 '*vr* '(r0 r1 r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 r12 r13 r14))
(defun vec-set (_) (setq *vsp* _))

(defun vec-goto (l)
	(vec-set 0)
	(goto l))

(defmacro vec-assign (_1 _2)
	(list 'assign _1 _2 '(slice *vsp* -1 *vr*)))

(defun vec-push (n)
	(slice *vsp* (setq *vsp* (+ *vsp* n)) *vr*))

(defun vec-pop (n)
	(slice (setq *vsp* (- *vsp* n)) (+ *vsp* n) *vr*))

(defun vec-top (n)
	(slice (- *vsp* n) *vsp* *vr*))

(defun vec-tmp (n)
	(slice *vsp* (+ *vsp* n) *vr*))

(defun vec-load (n &rest _)
	(each (lambda (_)
		(vec-assign _ (vec-push n))) _))

(defun vec-store (n &rest _)
	(each (lambda (_)
		(vec-assign (vec-pop n) _)) _))

(defun vec-tee (n &rest _)
	(each (lambda (_)
		(vec-assign (vec-top n) _)) _))

(defun vec-load-int (n r &optional i)
	(setd i 0)
	(each (lambda (v)
		(vp-cpy-ir-i r (+ i (* _ int_size)) v)) (vec-push n)))

(defun vec-store-int (n r &optional i)
	(setd i 0)
	(each (lambda (v)
		(vp-cpy-ri-i v r (+ i (* _ int_size)))) (vec-pop n)))

(defun vec-load-long (n r &optional i)
	(setd i 0)
	(each (lambda (v)
		(vp-cpy-ir r (+ i (* _ long_size)) v)) (vec-push n)))

(defun vec-store-long (n r &optional i)
	(setd i 0)
	(each (lambda (v)
		(vp-cpy-ri v r (+ i (* _ long_size)))) (vec-pop n)))

(defun vec-clr (n &optional v)
	(setd v (vec-top n))
	(each (lambda (v)
		(vp-xor-rr v v)) v))

(defun vec-cpy (n &optional v2 v1)
	(setd v2 (vec-pop n) v1 (vec-top n))
	(each (lambda (v2 v1)
		(vp-cpy-rr v2 v1)) v2 v1))

(defun vec-dup (n &optional v2 v1)
	(setd v2 (vec-top n) v1 (vec-push n))
	(vec-cpy n v2 v1))

(defun vec-over (n h)
	(vec-pop h)
	(defq v2 (vec-top n))
	(vec-push h)
	(vec-cpy n v2 (vec-push n)))

(defun vec-frac (n &optional v)
	(setd v (vec-top n))
	(bind '(r) (vec-tmp 1))
	(each (lambda (v)
		(vpif (list v '>= 0))
			(vp-and-cr +fp_frac_mask+ v)
		(else)
			(vp-cpy-rr v r)
			(vp-cpy-cr (<< 1 +fp_shift+) v)
			(vp-and-cr +fp_frac_mask+ r)
			(vp-sub-rr r v)
		(endif)) v))

(defun vec-floor (n &optional v)
	(setd v (vec-top n))
	(each (lambda (v)
		(vp-and-cr +fp_int_mask+ v)) v))

(defun vec-add (n &optional v2 v1)
	(setd v2 (vec-pop n) v1 (vec-top n))
	(each (lambda (v2 v1)
		(vp-add-rr v2 v1)) v2 v1))

(defun vec-sub (n &optional v2 v1)
	(setd v2 (vec-pop n) v1 (vec-top n))
	(each (lambda (v2 v1)
		(vp-sub-rr v2 v1)) v2 v1))

(defun vec-mul (n &optional v2 v1)
	(setd v2 (vec-pop n) v1 (vec-top n))
	(each (lambda (v2 v1)
		(vp-mul-rr v2 v1)) v2 v1))

(defun vec-div (n &optional v2 v1)
	(defq v3 (vec-tmp n))
	(setd v2 (vec-pop n) v1 (vec-top n))
	(each (lambda (v3 v1)
		(vp-ext-rr v1 v3)) v3 v1)
	(each (lambda (v3 v2 v1)
		(vp-div-rrr v2 v3 v1)) v3 v2 v1))

(defun vec-mod (n &optional v2 v1)
	(defq v3 (vec-tmp n))
	(setd v2 (vec-pop n) v1 (vec-top n))
	(each (lambda (v3 v1)
		(vp-ext-rr v1 v3)) v3 v1)
	(each (lambda (v3 v2 v1)
		(vp-div-rrr v2 v3 v1)) v3 v2 v1)
	(each (lambda (v3 v1)
		(vp-cpy-rr v3 v1)) v3 v1))

(defun vec-fmul (n &optional v2 v1)
	(vec-mul n v2 v1)
	(vec-asr n +fp_shift+ v1))

(defun vec-fdiv (n &optional v2 v1)
	(defq v3 (vec-tmp n))
	(setd v2 (vec-pop n) v1 (vec-top n))
	(each (lambda (v)
		(vp-shl-cr +fp_shift+ v)) v1)
	(each (lambda (v3 v1)
		(vp-ext-rr v1 v3)) v3 v1)
	(each (lambda (v3 v2 v1)
		(vp-div-rrr v2 v3 v1)) v3 v2 v1))

(defun vec-frecip (n &optional v1)
	(defq v (vec-tmp (* n 2)) v2 (slice 0 n v) v3 (slice n -1 v))
	(setd v1 (vec-top n))
	(each (lambda (v1 v2)
		(vp-cpy-rr v1 v2)) v1 v2)
	(each (lambda (v)
		(vp-cpy-cr (<< 1 (* 2 +fp_shift+)) v)) v1)
	(each (lambda (v)
		(vp-xor-rr v v)) v3)
	(each (lambda (v2 v3 v1)
		(vp-div-rrr v2 v3 v1)) v2 v3 v1))

(defun vec-fmod (n &optional v2 v1)
	(defq v3 (vec-push n) v4 (vec-tmp n))
	(vec-pop n)
	(setd v2 (vec-pop n) v1 (vec-top n))
	(each (lambda (v4 v1)
		(vp-cpy-rr v1 v4)) v4 v1)
	(each (lambda (v)
		(vp-shl-cr +fp_shift+ v)) v1)
	(each (lambda (v3 v1)
		(vp-ext-rr v1 v3)) v3 v1)
	(each (lambda (v3 v2 v1)
		(vp-div-rrr v2 v3 v1)) v3 v2 v1)
	(each (lambda (v)
		(vp-asr-cr +fp_shift+ v)) v1)
	(each (lambda (v2 v1)
		(vp-mul-rr v1 v2)) v2 v1)
	(each (lambda (v4 v1)
		(vp-cpy-rr v4 v1)) v4 v1)
	(each (lambda (v2 v1)
		(vp-sub-rr v2 v1)) v2 v1))

(defun vec-shl (n s &optional v)
	(setd v (vec-top n))
	(each (lambda (v)
		(if (reg s)
			(vp-shl-rr s v)
			(vp-shl-cr s v))) v))

(defun vec-shr (n s &optional v)
	(setd v (vec-top n))
	(each (lambda (v)
		(if (reg s)
			(vp-shr-rr s v)
			(vp-shr-cr s v))) v))

(defun vec-asr (n s &optional v)
	(setd v (vec-top n))
	(each (lambda (v)
		(if (reg s)
			(vp-asr-rr s v)
			(vp-asr-cr s v))) v))

(defun vec-abs (n &optional v)
	(setd v (vec-top n))
	(each (lambda (v)
		(vpif (list v '< 0))
			(vp-mul-cr -1 v)
		(endif)) v))

(defun vec-square (n &optional v)
	(setd v (vec-top n))
	(each (lambda (v)
		(vp-mul-rr v v)) v))

(defun vec-fsquare (n &optional v)
	(vec-square n v)
	(vec-asr n +fp_shift+ v))

(defun vec-scale (n &optional s v)
	(setd s (elem 0 (vec-pop 1)) v (vec-top n))
	(each (lambda (v)
		(if (reg s)
			(vp-mul-rr s v)
			(vp-mul-cr s v))) v))

(defun vec-fscale (n &optional s v)
	(vec-scale n s v)
	(vec-asr n +fp_shift+ v))

(defun vec-sum (n)
	(reduce (lambda (v1 v2)
		(vp-add-rr v2 v1) v1) (vec-pop n))
	(vec-push 1))

(defun vec-dif (n)
	(reduce (lambda (v1 v2)
		(vp-sub-rr v2 v1) v1) (vec-pop n))
	(vec-push 1))

(defun vec-min (n)
	(reduce (lambda (v1 v2)
		(vpif (list v2 '< v1))
			(vp-cpy-rr v2 v1)
		(endif) v1) (vec-pop n))
	(vec-push 1))

(defun vec-max (n)
	(reduce (lambda (v1 v2)
		(vpif (list v2 '> v1))
			(vp-cpy-rr v2 v1)
		(endif) v1) (vec-pop n))
	(vec-push 1))

(defun vec-perp ()
	(bind '(x y) (vec-top 2))
	(vp-swp-rr x y)
	(vp-mul-cr -1 y))

(defun vec-fdot (n &optional v2)
	(vec-fmul n v2)
	(vec-sum n))

(defun vec-fdet (n &optional v2)
	(setd v2 (vec-pop n))
	(each (lambda (v)
		(vp-mul-rr (elem (- n _ 1) v2) v)) (vec-top n))
	(vec-asr n +fp_shift+)
	(vec-dif n))

(defun vec-flength-squared (n)
	(vec-fsquare n)
	(vec-sum n))

(defun vec-flength (n)
	(vec-flength-squared n)
	(defq r (slice 0 (dec (if (>= *vsp* 4) 5 *vsp*)) *vr*))
	(if (/= (length r) 0) (apply vp-push r))
	(call 'sys_math :f_sqrt (vec-top 1) (vec-top 1))
	(if (/= (length r) 0) (apply vp-pop r)))

(defun vec-fdistance-squared (n)
	(vec-sub n)
	(vec-flength-squared n))

(defun vec-fnorm (n)
	(vec-dup n)
	(vec-flength n)
	(bind '(rl rh) (vec-tmp 2))
	(bind '(l) (vec-pop 1))
	(vpif (list l '= 0))
		(vec-clr n)
	(else)
		(vp-cpy-cr (<< (<< 1 +fp_shift+) +fp_shift+) rl)
		(vp-xor-rr rh rh)
		(vp-div-rrr l rh rl)
		(vec-fscale n rl)
	(endif))

(defun vec-manhattan (n)
	(vec-sub n)
	(vec-abs n)
	(vec-sum n))
