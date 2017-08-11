;;; ************************************************************************
;;; ************************************************************************
;;; Appendices to 
;;; Desain, P., & Honing, H. (1992). The quantization problem: traditional 
;;; and connectionist approaches. In M. Balaban, K. Ebcioglu, & O. Laske (eds.), 
;;; Understanding Music with AI: Perspectives on Music Cognition. 448-463. 
;;; Cambridge: MIT Press.

;;; ************************************************************************
;;; ************************************************************************
;;; MICRO TRADITIONAL QUANTIZER
;;; (C)1990, Desain & Honing
;;; in Common Lisp (uses loop macro)

;;; utilities

(defun square (x)(* x x))

(defun quantize (intervals &key (speed 0.0) (trust 1.0) 
                           (quantum (first intervals)))
  "Quantize time intervals in multiples of quantum"
  ;; speed = 0, trust = 1 :inter-onset quantizer
  ;; 0<speed<1, trust = 1 :tempo tracker
  ;; 0<speed<1, 0<trust<1 :tempo tracker with confidence
  (loop for in in intervals 
        as out = (quantize-ioi in quantum)
        as error = (quantization-error in out quantum)
        do  (incf quantum 
                  (* (delta-quantum error out quantum)
                     (confidence error trust) 
                     speed))
        collect out))

(defun quantize-ioi (time quantum)
  "Return approximation of time in multiples of quantum"
  (round (/ time quantum)))

(defun quantization-error (in out quantum)
  "Return error of quantization"
  (- (/ in quantum) out))

(defun delta-quantum (error out quantum)
  "Return the quantum change that would have given a zero error"
  (* quantum (/ error out)))

(defun confidence (error trust)
  "Return amount of confidence in a possible tempo adjustment"
  (- 1 (* (- 1 trust) (square (* 2 error)))))


;;; example: real performance data: no luck
(quantize '(1.177 0.592 0.288 0.337 0.436 0.337 0.387 0.600 
                  0.634 0.296 0.280 0.296 0.346 1.193)
          :quantum 0.1 :speed 0.5)
-> (12 6 3 3 4 3 4 6 6 3 3 3 4 13)


;;; ************************************************************************
;;; ************************************************************************

;;; LONGUET-HIGGINS QUANTIZER
;;; (C)1990, Desain
;;; Stripped version: no articulation analysis, metrical structure or tempo tracking
;;; in Common Lisp (uses loop macro)

;;; utilities

(defun make-onsets (intervals)
  "Translate inter-onset intervals to onset times"
  (loop for interval in intervals
        sum interval into onset
        collect onset into onsets
        finally (return (cons 0.0 onsets))))

(defun make-intervals (onsets)
    "Translate onset times to inter-onset intervals"
  (loop for onset1 in onsets
        for onset2 in (rest onsets)
        collect (- onset2 onset1))) 

(defun alternative (metre &rest states)
  "Return alternative metre plus unaltered states"
  (cons (case (first metre) (2 '(3)) (3 '(2)))
        states))

(defun extend (metre) 
  "Return alternative metre plus unaltered states"
  (or metre '(2)))

;;; main parsing routines
 
(defun quantize (intervals &key (metre '(2)) (tol 0.10) 
                 (beat (first intervals)))
  "Quantize intervals using initial metre and beat estimate"
  (loop with start = 0.0
        with onsets = (make-onsets intervals)
        for time from 0
        while onsets
        do (multiple-value-setq (start figure metre onsets)
             (rhythm start beat metre onsets time 1 tol))
        append figure into figures
        finally (return (make-intervals figures))))

(defun rhythm (start period metre onsets time factor tol)
  "Handle singlet and subdivide as continuation"
  (singlet 
    start (+ start period) metre onsets time tol
    #'(lambda (figure onsets)
       (tempo figure start period metre onsets time factor tol))))

(defun singlet (start stop metre onsets time tol cont)
  "Handle singlet note or rest" 
  (if (and onsets (< (first onsets) (+ start tol)))
    (singlet-figure stop metre (list time) (rest onsets) tol cont) 
    (singlet-figure stop metre nil onsets tol cont)))


(defun singlet-figure (stop metre figure onsets tol cont)
  "Create singlet figure and subdivide in case of more notes" 
  (let* ((onset (first onsets))
         (syncope (or (null onset) (>= onset (+ stop tol))))
         (more? (and onset (< onset (+ stop (- tol))))))
      (if more?
        (apply #'values (funcall cont figure onsets))
        (values (if syncope stop (first onsets)) 
                figure metre onsets syncope))))

(defun tempo (figure start period metre onsets time factor tol)
  "One or two trials of subdivision using alternative metres"
  (rest (generate-and-test #'trial 
                     #'(lambda (syncope stop &rest ignore) 
                         (and (not syncope) 
                              (< (- stop tol) 
                                 (+ start period) 
                                 (+ stop tol)))) 
                     #'alternative 
                     metre figure start period onsets time factor tol)))

(defun generate-and-test (generate test alternative &rest states)
    "Control structure for metre change"
    (let ((result1 (apply generate states)))
      (if (apply test result1)
        result1
        (let ((result2 (apply generate (apply alternative states))))
          (if (apply test result2)
            result2
            result1)))))

(defun trial (metre figure start period onsets time factor tol)
    "Try a subdivision of period"
  (loop with pulse = (pop metre)
        with sub-period = (/ period (float pulse))
        with sub-factor = (/ factor pulse)
        repeat pulse
        for sub-time from time by sub-factor
        do  (multiple-value-setq 
              (start sub-figure metre onsets syncope)
              (rhythm start sub-period (extend metre) onsets 
                      sub-time sub-factor tol))
        append sub-figure into sub-figures
        finally 
         (return 
          (list syncope start (append figure sub-figures) (cons pulse metre) onsets))))

;;; example 
(quantize '(1.177 0.592 0.288 0.337 0.436 0.337 0.387 0.600 0.634
            0.296 0.280 0.296 0.346 1.193) :tol 0.15) 
->(1 1/2 1/4 1/4 1/3 1/3 1/3 1/2 1/2 1/4 1/4 1/4 1/4 1)


;;; ************************************************************************
;;; ************************************************************************

;;; MICRO CONNECTIONIST QUANTIZER
;;; (C)1990, Desain & Honing
;;; in Common Lisp (uses loop macro)

;;; utilities

(define-modify-macro multf (factor) *)
(define-modify-macro divf (factor) /)
(define-modify-macro zerof () (lambda(x) 0))

(defmacro with-adjacent-intervals 
    (vector (a-begin a-end a-sum b-begin b-end b-sum) &body body)
  "Setup environment for each interaction of (sum-)intervals"
  `(loop with length = (length ,vector)
         for ,a-begin below (1- length)
         do (loop for ,a-end from ,a-begin below (1- length)
                  sum (aref ,vector ,a-end) into ,a-sum 
                  do (loop with ,b-begin = (1+ ,a-end)
                           for ,b-end from ,b-begin below length
                           sum (aref ,vector ,b-end) into ,b-sum
                           do ,@body))))

;;; interaction function

(defun delta (a b minimum peak decay)
  "Return change for two time intervals"
  (let* ((inverted? (<= a b))
         (ratio (if inverted? (/ b a)(/ a b)))
         (delta-ratio (interaction ratio peak decay))
         (proportion (/ delta-ratio (+ 1 ratio delta-ratio))))
    (* minimum (if inverted? (- proportion) proportion))))
  
(defun interaction (ratio peak decay)
  "Return change of time interval ratio"
  (* (- (round ratio) ratio)
     (expt (abs (* 2 (- ratio (floor ratio) 0.5))) peak)
     (expt (round ratio) decay)))

;;; quantization procedures

(defun quantize (intervals &key (iterations 20) (peak 5) (decay -1))
  "Quantize data of inter-onset intervals"
  (let* ((length (length intervals))
         (changes (make-array length :initial-element 0.0))
         (minimum (loop for index below length 
                        minimize (aref intervals index))))
    (loop for count to iterations 
          do (update intervals minimum changes peak decay)
          finally (return (coerce intervals 'list)))))


(defun update (intervals minimum changes peak decay)
  "Update all intervals synchronously"
  (with-adjacent-intervals intervals 
    (a-begin a-end a-sum b-begin b-end b-sum)
    (let ((delta (delta a-sum b-sum minimum peak decay)))
      (propagate changes a-begin a-end (/ delta a-sum))
      (propagate changes b-begin b-end (- (/ delta b-sum)))))
  (enforce changes intervals))

(defun propagate (changes begin end change)
  "Derive changes of basic-intervals from sum-interval change"
  (loop for index from begin to end 
        do (incf (aref changes index) change)))

(defun enforce (changes intervals)
  "Effectuate changes to intervals"
  (loop for index below (length intervals)
        do (multf (aref intervals index) 
                  (1+ (aref changes index)))
           (zerof (aref changes index))))


;;; example (the result is rounded)
(quantize (vector 1.177 0.592 0.288 0.337 0.436 0.337 0.387 0.600 
                  0.634 0.296 0.280 0.296 0.346 1.193))
->(1.2 .6 .3 .3 .4 .4 .4 .6 .6 .3 .3 .3 .3 1.2)

;;; ************************************************************************
;;; ************************************************************************



