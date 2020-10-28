;;;;;;;;;;;;;;;;;;;;;;;;;
; generic hash set object
;;;;;;;;;;;;;;;;;;;;;;;;;

(defmacro-bind xset-slot ()
	`(defq x (get :buckets this)
		b (% ((get :hash_fnc this) key) (get :num_buckets this))
		e (some (# (if ((get :cmp_fnc this) %0 key) _)) (elem b x))))

(class xset (&optional num_buckets cmp_fnc hash_fnc)
	;(xset [num_buckets cmp_fnc hash_fnc]) -> xset
	(def this :buckets (list) :num_buckets (setq num_buckets (opt num_buckets 1))
		:cmp_fnc (opt cmp_fnc eql) :hash_fnc (opt hash_fnc hash) :xset t)
	(while (>= (setq num_buckets (dec num_buckets)) 0)
		(push (get :buckets this) (list)))

	(method-bind :find (this key)
		;(. xset :find key) -> nil|key
		(xset-slot)
		(if e (elem e (elem b x))))

	(method-bind :insert (this key)
		;(. xset :insert key) -> nil|t
		(xset-slot)
		(cond
			(e (elem-set e (elem b x) key) nil)
			(t (push (elem b x) key) t)))

	(method-bind :erase (this key)
		;(. xset :erase key) -> nil|t
		(xset-slot)
		(when e
			(elem-set e (defq b (elem b x)) (elem -2 b))
			(pop b) t))

	(method-bind :each (_this _hf)
		;(. xset :each lambda)
		(each (lambda (_) (each _hf _)) (get :buckets _this)))

	(method-bind :copy (this)
		;(. xset :copy) -> xset
		(defq that (xset (get :num_buckets this) (get :cmp_fnc this) (get :hash_fnc this)))
		(each (lambda (this_bucket that_bucket)
			(each (lambda (key)
				(push that_bucket key)) this_bucket)) (get :buckets this) (get :buckets that))
		that)

	(method-bind :deep_copy (this)
		;(. xset :deep_copy) -> xset
		(defq that (xset (get :num_buckets this) (get :cmp_fnc this) (get :hash_fnc this)))
		(each (lambda (this_bucket that_bucket)
			(each (lambda (key)
				(push that_bucket (copy key))) this_bucket)) (get :buckets this) (get :buckets that))
		that)

	(method-bind :union (this that)
		;(. xset :union xset) -> xset
		(unless (eql this that)
			(. that :each (# (. this :insert %0))))
		this)

	(method-bind :difference (this that)
		;(. xset :difference xset) -> xset
		(cond
			((eql this that)
				(. this :clear))
			(t	(. that :each (# (. this :erase %0)))
				this)))

	(method-bind :intersect (this that)
		;(. xset :intersect xset) -> xset
		(unless (eql this that)
			(each (# (elem-set _ b (filter (# (. that :find %0)) %0)))
				(defq b (get :buckets this))))
		this)

	(method-bind :not_intersect (this that)
		;(. xset :not_intersect xset) -> xset
		(cond
			((eql this that)
				(. this :clear))
			(t	(. (defq other (. that :copy)) :difference this)
				(. this :difference that)
				(. this :union other))))

	(method-bind :clear (this)
		;(. xset :clear) -> xset
		(each (# (clear %0)) (get :buckets this))
		this)

	(method-bind :move (this)
		;(. xset :move) -> xset
		(defq that (xset (get :num_buckets this) (get :cmp_fnc this) (get :hash_fnc this))
			this_buckets (get :buckets this) that_buckets (get :buckets that))
		(set this :buckets that_buckets)
		(set that :buckets this_buckets)
		that)

	(method-bind :resize (this num_buckets)
		;(. xset :resize num_buckets) -> xset
		(defq hash_fnc (get :hash_fnc this) old_buckets (get :buckets this) new_buckets (list))
		(set this :buckets new_buckets :num_buckets num_buckets)
		(while (>= (setq num_buckets (dec num_buckets)) 0)
			(push new_buckets (list)))
		(setq num_buckets (get :num_buckets this))
		(each (lambda (old_bucket)
			(while (defq key (pop old_bucket))
				(push (elem (% (hash_fnc key) num_buckets) new_buckets) key))) old_buckets)
		this)
	)

(defun-bind xset-k (&rest _)
	;(xset-k [key ...]) -> xset
	(defq this (xset))
	(each (# (. this :insert %0)) _)
	this)

(defun-bind xset? (this)
	;(xset? xset) -> nil|t
	(and (env? this) (get :xset this)))

(undef (env) 'xset-slot)

;tests
(when nil
	(defq a (xset-k :a :b :c :z) b (xset-k :a :x :y :z))
	(defq a (. a :resize 2) b (. b :resize 7))
	(print "originals")
	(. a :each prin)
	(print)
	(. b :each prin)
	(print)
	(print "copy")
	(defq x (. a :copy))
	(. x :each prin)
	(print)
	(print "union")
	(defq x (. a :copy))
	(. x :union b)
	(. x :each prin)
	(print)
	(print "difference 1")
	(defq x (. a :copy))
	(. x :difference b)
	(. x :each prin)
	(print)
	(print "difference 2")
	(defq x (. b :copy))
	(. x :difference a)
	(. x :each prin)
	(print)
	(print "intersect")
	(defq x (. a :copy))
	(. x :intersect b)
	(. x :each prin)
	(print)
	(print "not_intersect")
	(defq x (. a :copy))
	(. x :not_intersect b)
	(. x :each prin)
	(print)
	(print "originals")
	(. a :each prin)
	(print)
	(. b :each prin)
	(print))