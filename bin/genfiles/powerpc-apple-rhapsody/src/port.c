#ifndef _SETJMP_H_
#define _SETJMP_H_
#ifndef _jmp_buf_def_
#define _jmp_buf_def_
typedef int jmp_buf[192];
#endif
extern int setjmp(jmp_buf);
#endif
/* This is a C header file to be used by the output of the Cyclone to
   C translator.  The corresponding definitions are in file
   lib/runtime_cyc.c
*/
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

#ifdef NO_CYC_PREFIX
#define ADD_PREFIX(x) x
#else
#define ADD_PREFIX(x) Cyc_##x
#endif

#ifndef offsetof
/* should be size_t, but int is fine. */
#define offsetof(t,n) ((int)(&(((t *)0)->n)))
#endif

/* Tagged arrays */
struct _dyneither_ptr {
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};  

/* Discriminated Unions */
struct _xtunion_struct { char *tag; };

/* Need one of these per thread (we don't have threads)
   The runtime maintains a stack that contains either _handler_cons
   structs or _RegionHandle structs.  The tag is 0 for a handler_cons
   and 1 for a region handle.  */
struct _RuntimeStack {
  int tag; /* 0 for an exception handler, 1 for a region handle */
  struct _RuntimeStack *next;
};

/* Regions */
struct _RegionPage {
#ifdef CYC_REGION_PROFILE
  unsigned total_bytes;
  unsigned free_bytes;
#endif
  struct _RegionPage *next;
  char data[1];  /*FJS: used to be size 0, but that's forbidden in ansi c*/
};

struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
  char               *offset;
  char               *last_plus_one;
  struct _DynRegionHandle *sub_regions;
#ifdef CYC_REGION_PROFILE
  const char         *name;
#endif
};

struct _DynRegionFrame {
  struct _RuntimeStack s;
  struct _DynRegionHandle *x;
};

extern struct _RegionHandle _new_region(const char *);
extern void * _region_malloc(struct _RegionHandle *, unsigned);
extern void * _region_calloc(struct _RegionHandle *, unsigned t, unsigned n);
extern void   _free_region(struct _RegionHandle *);
extern void   _reset_region(struct _RegionHandle *);
extern struct _RegionHandle *_open_dynregion(struct _DynRegionFrame *f,
                                             struct _DynRegionHandle *h);
extern void   _pop_dynregion();

/* Exceptions */
struct _handler_cons {
  struct _RuntimeStack s;
  jmp_buf handler;
};
extern void _push_handler(struct _handler_cons *);
extern void _push_region(struct _RegionHandle *);
extern void _npop_handler(int);
extern void _pop_handler();
extern void _pop_region();

#ifndef _throw
extern int _throw_null();
extern int _throw_arraybounds();
extern int _throw_badalloc();
extern int _throw(void* e);
#endif

extern struct _xtunion_struct *_exn_thrown;

/* Built-in Exceptions */
extern struct _xtunion_struct ADD_PREFIX(Null_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Null_Exception);
extern struct _xtunion_struct ADD_PREFIX(Array_bounds_struct);
extern struct _xtunion_struct * ADD_PREFIX(Array_bounds);
extern struct _xtunion_struct ADD_PREFIX(Match_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Match_Exception);
extern struct _xtunion_struct ADD_PREFIX(Bad_alloc_struct);
extern struct _xtunion_struct * ADD_PREFIX(Bad_alloc);

/* Built-in Run-time Checks and company */
#ifdef __APPLE__
#define _INLINE_FUNCTIONS
#endif

#ifdef CYC_ANSI_OUTPUT
#define _INLINE  
#define _INLINE_FUNCTIONS
#else
#define _INLINE inline
#endif

#ifdef VC_C
#define _CYC_U_LONG_LONG_T __int64
#else
#ifdef GCC_C
#define _CYC_U_LONG_LONG_T unsigned long long
#else
#define _CYC_U_LONG_LONG_T unsigned long long
#endif
#endif

#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE void *
_check_null(void *ptr) {
  void*_check_null_temp = (void*)(ptr);
  if (!_check_null_temp) _throw_null();
  return _check_null_temp;
}
#else
#define _check_null(ptr) \
  ({ void*_check_null_temp = (void*)(ptr); \
     if (!_check_null_temp) _throw_null(); \
     _check_null_temp; })
#endif
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  ((char *)ptr) + (elt_sz)*(index); })
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE char *
_check_known_subscript_null(void *ptr, unsigned bound, unsigned elt_sz, unsigned index) {
  void*_cks_ptr = (void*)(ptr);
  unsigned _cks_bound = (bound);
  unsigned _cks_elt_sz = (elt_sz);
  unsigned _cks_index = (index);
  if (!_cks_ptr) _throw_null();
  if (_cks_index >= _cks_bound) _throw_arraybounds();
  return ((char *)_cks_ptr) + _cks_elt_sz*_cks_index;
}
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  void*_cks_ptr = (void*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (_cks_index >= _cks_bound) _throw_arraybounds(); \
  ((char *)_cks_ptr) + _cks_elt_sz*_cks_index; })
#endif
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_notnull(bound,index) (index)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned
_check_known_subscript_notnull(unsigned bound,unsigned index) { 
  unsigned _cksnn_bound = (bound); 
  unsigned _cksnn_index = (index); 
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); 
  return _cksnn_index;
}
#else
#define _check_known_subscript_notnull(bound,index) ({ \
  unsigned _cksnn_bound = (bound); \
  unsigned _cksnn_index = (index); \
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); \
  _cksnn_index; })
#endif
#endif

/* Add i to zero-terminated pointer x.  Checks for x being null and
   ensures that x[0..i-1] are not 0. */
#ifdef NO_CYC_BOUNDS_CHECK
#define _zero_arr_plus_char(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_short(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_int(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_float(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_double(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#else
static _INLINE char *
_zero_arr_plus_char(char *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE short *
_zero_arr_plus_short(short *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE int *
_zero_arr_plus_int(int *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE float *
_zero_arr_plus_float(float *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE double *
_zero_arr_plus_double(double *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE long double *
_zero_arr_plus_longdouble(long double *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE void *
_zero_arr_plus_voidstar(void **orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
#endif


/* Calculates the number of elements in a zero-terminated, thin array.
   If non-null, the array is guaranteed to have orig_offset elements. */
static _INLINE int
_get_zero_arr_size_char(const char *orig_x, unsigned int orig_offset) {
  const char *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_short(const short *orig_x, unsigned int orig_offset) {
  const short *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_int(const int *orig_x, unsigned int orig_offset) {
  const int *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_float(const float *orig_x, unsigned int orig_offset) {
  const float *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_double(const double *orig_x, unsigned int orig_offset) {
  const double *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_longdouble(const long double *orig_x, unsigned int orig_offset) {
  const long double *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_voidstar(const void **orig_x, unsigned int orig_offset) {
  const void **_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}


/* Does in-place addition of a zero-terminated pointer (x += e and ++x).  
   Note that this expands to call _zero_arr_plus. */
/*#define _zero_arr_inplace_plus(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  *_zap_x = _zero_arr_plus(*_zap_x,1,(orig_i)); })
  */
static _INLINE void 
_zero_arr_inplace_plus_char(char *x, int orig_i) {
  char **_zap_x = &x;
  *_zap_x = _zero_arr_plus_char(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_short(short *x, int orig_i) {
  short **_zap_x = &x;
  *_zap_x = _zero_arr_plus_short(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_int(int *x, int orig_i) {
  int **_zap_x = &x;
  *_zap_x = _zero_arr_plus_int(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_float(float *x, int orig_i) {
  float **_zap_x = &x;
  *_zap_x = _zero_arr_plus_float(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_double(double *x, int orig_i) {
  double **_zap_x = &x;
  *_zap_x = _zero_arr_plus_double(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_longdouble(long double *x, int orig_i) {
  long double **_zap_x = &x;
  *_zap_x = _zero_arr_plus_longdouble(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_voidstar(void **x, int orig_i) {
  void ***_zap_x = &x;
  *_zap_x = _zero_arr_plus_voidstar(*_zap_x,1,orig_i);
}




/* Does in-place increment of a zero-terminated pointer (e.g., x++).
   Note that this expands to call _zero_arr_plus. */
/*#define _zero_arr_inplace_plus_post(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  _zap_tx *_zap_res = *_zap_x; \
  *_zap_x = _zero_arr_plus(_zap_res,1,(orig_i)); \
  _zap_res; })*/
  
static _INLINE char *
_zero_arr_inplace_plus_post_char(char *x, int orig_i){
  char ** _zap_x = &x;
  char * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_char(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE short *
_zero_arr_inplace_plus_post_short(short *x, int orig_i){
  short **_zap_x = &x;
  short * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_short(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE int *
_zero_arr_inplace_plus_post_int(int *x, int orig_i){
  int **_zap_x = &x;
  int * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_int(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE float *
_zero_arr_inplace_plus_post_float(float *x, int orig_i){
  float **_zap_x = &x;
  float * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_float(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE double *
_zero_arr_inplace_plus_post_double(double *x, int orig_i){
  double **_zap_x = &x;
  double * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_double(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE long double *
_zero_arr_inplace_plus_post_longdouble(long double *x, int orig_i){
  long double **_zap_x = &x;
  long double * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_longdouble(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE void **
_zero_arr_inplace_plus_post_voidstar(void **x, int orig_i){
  void ***_zap_x = &x;
  void ** _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_voidstar(_zap_res,1,orig_i);
  return _zap_res;
}



/* functions for dealing with dynamically sized pointers */
#ifdef NO_CYC_BOUNDS_CHECKS
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  return _cus_ans;
}
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
#endif
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  /* JGM: not needed! if (!_cus_arr.base) _throw_null(); */ 
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds();
  return _cus_ans;
}
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  /* JGM: not needed! if (!_cus_arr.base) _throw_null();*/ \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_tag_dyneither(const void *tcurr,unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _tag_arr_ans;
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr);
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts);
  return _tag_arr_ans;
}
#else
#define _tag_dyneither(tcurr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr *
_init_dyneither_ptr(struct _dyneither_ptr *arr_ptr,
                    void *arr, unsigned elt_sz, unsigned num_elts) {
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr);
  void* _itarr = (arr);
  _itarr_ptr->base = _itarr_ptr->curr = _itarr;
  _itarr_ptr->last_plus_one = ((unsigned char *)_itarr) + (elt_sz) * (num_elts);
  return _itarr_ptr;
}
#else
#define _init_dyneither_ptr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->base = _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ((arr).curr)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_untag_dyneither_ptr(struct _dyneither_ptr arr, 
                     unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds();
  return _curr;
}
#else
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
    _throw_arraybounds(); \
  _curr; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned
_get_dyneither_size(struct _dyneither_ptr arr,unsigned elt_sz) {
  struct _dyneither_ptr _get_arr_size_temp = (arr);
  unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr;
  unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one;
  return (_get_arr_size_curr < _get_arr_size_temp.base ||
          _get_arr_size_curr >= _get_arr_size_last) ? 0 :
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));
}
#else
#define _get_dyneither_size(arr,elt_sz) \
  ({struct _dyneither_ptr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_plus(struct _dyneither_ptr arr,unsigned elt_sz,int change) {
  struct _dyneither_ptr _ans = (arr);
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_plus(arr,elt_sz,change) ({ \
  struct _dyneither_ptr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_inplace_plus(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,
                            int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return *_arr_ptr;
}
#else
#define _dyneither_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_inplace_plus_post(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  struct _dyneither_ptr _ans = *_arr_ptr;
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  struct _dyneither_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

/* Decrease the upper bound on a fat pointer by numelts where sz is
   the size of the pointer's type.  Note that this can't be a macro
   if we're to get initializers right. */
static struct 
_dyneither_ptr _dyneither_ptr_decrease_size(struct _dyneither_ptr x,
                                            unsigned int sz,
                                            unsigned int numelts) {
  x.last_plus_one -= sz * numelts; 
  return x; 
}

/* Allocation */
extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);

static _INLINE void* _cycalloc(int n) {
  void * ans = (void *)GC_malloc(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cycalloc_atomic(int n) {
  void * ans = (void *)GC_malloc_atomic(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cyccalloc(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cyccalloc_atomic(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc_atomic(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
#define MAX_MALLOC_SIZE (1 << 28)
static _INLINE unsigned int _check_times(unsigned x, unsigned y) {
  _CYC_U_LONG_LONG_T whole_ans = 
    ((_CYC_U_LONG_LONG_T)x)*((_CYC_U_LONG_LONG_T)y);
  unsigned word_ans = (unsigned)whole_ans;
  if(word_ans < whole_ans || word_ans > MAX_MALLOC_SIZE)
    _throw_badalloc();
  return word_ans;
}

#if defined(CYC_REGION_PROFILE) 
extern void* _profile_GC_malloc(int,char *file,int lineno);
extern void* _profile_GC_malloc_atomic(int,char *file,int lineno);
extern void* _profile_region_malloc(struct _RegionHandle *, unsigned,
                                     char *file,int lineno);
extern struct _RegionHandle _profile_new_region(const char *rgn_name,
						char *file,int lineno);
extern void _profile_free_region(struct _RegionHandle *,
				 char *file,int lineno);
#  if !defined(RUNTIME_CYC)
#define _new_region(n) _profile_new_region(n,__FILE__ ":" __FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__ ":" __FUNCTION__,__LINE__)
#define _region_malloc(rh,n) _profile_region_malloc(rh,n,__FILE__ ":" __FUNCTION__,__LINE__)
#  endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__ ":" __FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__ ":" __FUNCTION__,__LINE__)
#endif
#endif

/* the next two routines swap [x] and [y]; not thread safe! */
static _INLINE void _swap_word(void *x, void *y) {
  unsigned long *lx = (unsigned long *)x, *ly = (unsigned long *)y, tmp;
  tmp = *lx;
  *lx = *ly;
  *ly = tmp;
}
static _INLINE void _swap_dyneither(struct _dyneither_ptr *x, 
				   struct _dyneither_ptr *y) {
  struct _dyneither_ptr tmp = *x;
  *x = *y;
  *y = tmp;
}
 struct Cyc_Core_NewRegion{struct _DynRegionHandle*dynregion;};struct Cyc_Core_Opt{
void*v;};extern char Cyc_Core_Invalid_argument[21];struct Cyc_Core_Invalid_argument_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[12];struct Cyc_Core_Failure_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[15];struct Cyc_Core_Impossible_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[14];extern char Cyc_Core_Unreachable[
16];struct Cyc_Core_Unreachable_struct{char*tag;struct _dyneither_ptr f1;};extern
char Cyc_Core_Open_Region[16];extern char Cyc_Core_Free_Region[16];struct Cyc_List_List{
void*hd;struct Cyc_List_List*tl;};struct Cyc_List_List*Cyc_List_list(struct
_dyneither_ptr);struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*
x);struct Cyc_List_List*Cyc_List_map_c(void*(*f)(void*,void*),void*env,struct Cyc_List_List*
x);extern char Cyc_List_List_mismatch[18];void Cyc_List_iter_c(void(*f)(void*,void*),
void*env,struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*
x);struct Cyc_List_List*Cyc_List_merge_sort(int(*cmp)(void*,void*),struct Cyc_List_List*
x);extern char Cyc_List_Nth[8];int Cyc_strcmp(struct _dyneither_ptr s1,struct
_dyneither_ptr s2);int Cyc_strptrcmp(struct _dyneither_ptr*s1,struct _dyneither_ptr*
s2);struct _dyneither_ptr Cyc_strconcat_l(struct Cyc_List_List*);struct Cyc_Lineno_Pos{
struct _dyneither_ptr logical_file;struct _dyneither_ptr line;int line_no;int col;};
extern char Cyc_Position_Exit[9];struct Cyc_Position_Segment;struct Cyc_List_List*
Cyc_Position_strings_of_segments(struct Cyc_List_List*);struct Cyc_Position_Error{
struct _dyneither_ptr source;struct Cyc_Position_Segment*seg;void*kind;struct
_dyneither_ptr desc;};extern char Cyc_Position_Nocontext[14];extern int Cyc_Position_use_gcc_style_location;
struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct
_union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{
int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct
_union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_Loc_n Loc_n;};union Cyc_Absyn_Nmspace
Cyc_Absyn_Loc_n;union Cyc_Absyn_Nmspace Cyc_Absyn_Rel_n(struct Cyc_List_List*);
union Cyc_Absyn_Nmspace Cyc_Absyn_Abs_n(struct Cyc_List_List*);struct _tuple0{union
Cyc_Absyn_Nmspace f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Tqual{int
print_const;int q_volatile;int q_restrict;int real_const;struct Cyc_Position_Segment*
loc;};struct _union_Constraint_Eq_constr{int tag;void*val;};struct
_union_Constraint_Forward_constr{int tag;union Cyc_Absyn_Constraint*val;};struct
_union_Constraint_No_constr{int tag;int val;};union Cyc_Absyn_Constraint{struct
_union_Constraint_Eq_constr Eq_constr;struct _union_Constraint_Forward_constr
Forward_constr;struct _union_Constraint_No_constr No_constr;};struct Cyc_Absyn_Eq_kb_struct{
int tag;void*f1;};struct Cyc_Absyn_Unknown_kb_struct{int tag;struct Cyc_Core_Opt*f1;
};struct Cyc_Absyn_Less_kb_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;};struct
Cyc_Absyn_Tvar{struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_Upper_b_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_PtrLoc{struct Cyc_Position_Segment*
ptr_loc;struct Cyc_Position_Segment*rgn_loc;struct Cyc_Position_Segment*zt_loc;};
struct Cyc_Absyn_PtrAtts{void*rgn;union Cyc_Absyn_Constraint*nullable;union Cyc_Absyn_Constraint*
bounds;union Cyc_Absyn_Constraint*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};
struct Cyc_Absyn_PtrInfo{void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts
ptr_atts;};struct Cyc_Absyn_Numelts_ptrqual_struct{int tag;struct Cyc_Absyn_Exp*f1;
};struct Cyc_Absyn_Region_ptrqual_struct{int tag;void*f1;};struct Cyc_Absyn_VarargInfo{
struct Cyc_Core_Opt*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{
struct Cyc_List_List*tvars;struct Cyc_Core_Opt*effect;void*ret_typ;struct Cyc_List_List*
args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*
rgn_po;struct Cyc_List_List*attributes;};struct Cyc_Absyn_UnknownDatatypeInfo{
struct _tuple0*name;int is_extensible;};struct _union_DatatypeInfoU_UnknownDatatype{
int tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct
_union_DatatypeInfoU_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};
union Cyc_Absyn_DatatypeInfoU{struct _union_DatatypeInfoU_UnknownDatatype
UnknownDatatype;struct _union_DatatypeInfoU_KnownDatatype KnownDatatype;};struct
Cyc_Absyn_DatatypeInfo{union Cyc_Absyn_DatatypeInfoU datatype_info;struct Cyc_List_List*
targs;struct Cyc_Core_Opt*rgn;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{struct
_tuple0*datatype_name;struct _tuple0*field_name;int is_extensible;};struct
_union_DatatypeFieldInfoU_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo
val;};struct _tuple1{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*
f2;};struct _union_DatatypeFieldInfoU_KnownDatatypefield{int tag;struct _tuple1 val;
};union Cyc_Absyn_DatatypeFieldInfoU{struct
_union_DatatypeFieldInfoU_UnknownDatatypefield UnknownDatatypefield;struct
_union_DatatypeFieldInfoU_KnownDatatypefield KnownDatatypefield;};struct Cyc_Absyn_DatatypeFieldInfo{
union Cyc_Absyn_DatatypeFieldInfoU field_info;struct Cyc_List_List*targs;};struct
_tuple2{void*f1;struct _tuple0*f2;struct Cyc_Core_Opt*f3;};struct
_union_AggrInfoU_UnknownAggr{int tag;struct _tuple2 val;};struct
_union_AggrInfoU_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfoU{
struct _union_AggrInfoU_UnknownAggr UnknownAggr;struct _union_AggrInfoU_KnownAggr
KnownAggr;};struct Cyc_Absyn_AggrInfo{union Cyc_Absyn_AggrInfoU aggr_info;struct Cyc_List_List*
targs;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct
Cyc_Absyn_Exp*num_elts;union Cyc_Absyn_Constraint*zero_term;struct Cyc_Position_Segment*
zt_loc;};struct Cyc_Absyn_Evar_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Core_Opt*
f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_struct{int tag;struct Cyc_Absyn_Tvar*
f1;};struct Cyc_Absyn_DatatypeType_struct{int tag;struct Cyc_Absyn_DatatypeInfo f1;}
;struct Cyc_Absyn_DatatypeFieldType_struct{int tag;struct Cyc_Absyn_DatatypeFieldInfo
f1;};struct Cyc_Absyn_PointerType_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};
struct Cyc_Absyn_IntType_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_DoubleType_struct{
int tag;int f1;};struct Cyc_Absyn_ArrayType_struct{int tag;struct Cyc_Absyn_ArrayInfo
f1;};struct Cyc_Absyn_FnType_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AggrType_struct{int tag;struct Cyc_Absyn_AggrInfo
f1;};struct Cyc_Absyn_AnonAggrType_struct{int tag;void*f1;struct Cyc_List_List*f2;}
;struct Cyc_Absyn_EnumType_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*
f2;};struct Cyc_Absyn_AnonEnumType_struct{int tag;struct Cyc_List_List*f1;};struct
Cyc_Absyn_RgnHandleType_struct{int tag;void*f1;};struct Cyc_Absyn_DynRgnType_struct{
int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_struct{int tag;struct _tuple0*
f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void**f4;};struct Cyc_Absyn_ValueofType_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TagType_struct{int tag;void*f1;};
struct Cyc_Absyn_AccessEff_struct{int tag;void*f1;};struct Cyc_Absyn_JoinEff_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnsEff_struct{int tag;void*f1;};
struct Cyc_Absyn_NoTypes_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*
f2;};struct Cyc_Absyn_WithTypes_struct{int tag;struct Cyc_List_List*f1;int f2;struct
Cyc_Absyn_VarargInfo*f3;struct Cyc_Core_Opt*f4;struct Cyc_List_List*f5;};struct Cyc_Absyn_Regparm_att_struct{
int tag;int f1;};struct Cyc_Absyn_Aligned_att_struct{int tag;int f1;};struct Cyc_Absyn_Section_att_struct{
int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Format_att_struct{int tag;void*f1;
int f2;int f3;};struct Cyc_Absyn_Initializes_att_struct{int tag;int f1;};struct Cyc_Absyn_Mode_att_struct{
int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Carray_mod_struct{int tag;union
Cyc_Absyn_Constraint*f1;struct Cyc_Position_Segment*f2;};struct Cyc_Absyn_ConstArray_mod_struct{
int tag;struct Cyc_Absyn_Exp*f1;union Cyc_Absyn_Constraint*f2;struct Cyc_Position_Segment*
f3;};struct Cyc_Absyn_Pointer_mod_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct
Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_struct{int tag;void*f1;};struct
Cyc_Absyn_TypeParams_mod_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*
f2;int f3;};struct Cyc_Absyn_Attributes_mod_struct{int tag;struct Cyc_Position_Segment*
f1;struct Cyc_List_List*f2;};struct _union_Cnst_Null_c{int tag;int val;};struct
_tuple3{void*f1;char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple3 val;};
struct _tuple4{void*f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple4
val;};struct _tuple5{void*f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple5
val;};struct _tuple6{void*f1;long long f2;};struct _union_Cnst_LongLong_c{int tag;
struct _tuple6 val;};struct _union_Cnst_Float_c{int tag;struct _dyneither_ptr val;};
struct _union_Cnst_String_c{int tag;struct _dyneither_ptr val;};union Cyc_Absyn_Cnst{
struct _union_Cnst_Null_c Null_c;struct _union_Cnst_Char_c Char_c;struct
_union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct
_union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct
_union_Cnst_String_c String_c;};struct Cyc_Absyn_VarargCallInfo{int num_varargs;
struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_struct{
int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_struct{int tag;
unsigned int f1;};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;
void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;};struct Cyc_Absyn_Const_e_struct{
int tag;union Cyc_Absyn_Cnst f1;};struct Cyc_Absyn_Var_e_struct{int tag;struct _tuple0*
f1;void*f2;};struct Cyc_Absyn_UnknownId_e_struct{int tag;struct _tuple0*f1;};struct
Cyc_Absyn_Primop_e_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};
struct Cyc_Absyn_Increment_e_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;};
struct Cyc_Absyn_Conditional_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*
f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnknownCall_e_struct{int tag;struct
Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_FnCall_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;
};struct Cyc_Absyn_Throw_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoInstantiate_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_struct{
int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;void*f4;};struct Cyc_Absyn_Address_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_struct{int tag;void*f1;};
struct Cyc_Absyn_Sizeofexp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_struct{
int tag;void*f1;void*f2;};struct Cyc_Absyn_Gentyp_e_struct{int tag;struct Cyc_List_List*
f1;void*f2;};struct Cyc_Absyn_Deref_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_AggrMember_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
_dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_struct{int tag;struct
Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_struct{
int tag;struct Cyc_List_List*f1;};struct _tuple7{struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Tqual
f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_struct{int tag;struct _tuple7*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_struct{int tag;struct Cyc_List_List*f1;
};struct Cyc_Absyn_Comprehension_e_struct{int tag;struct Cyc_Absyn_Vardecl*f1;
struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_Aggregate_e_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*
f4;};struct Cyc_Absyn_AnonStruct_e_struct{int tag;void*f1;struct Cyc_List_List*f2;}
;struct Cyc_Absyn_Datatype_e_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*
f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_struct{int tag;
struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;struct Cyc_Absyn_Enumfield*f3;};
struct Cyc_Absyn_AnonEnum_e_struct{int tag;struct _tuple0*f1;void*f2;struct Cyc_Absyn_Enumfield*
f3;};struct Cyc_Absyn_Malloc_e_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};
struct Cyc_Absyn_Swap_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*
f2;};struct Cyc_Absyn_UnresolvedMem_e_struct{int tag;struct Cyc_Core_Opt*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_struct{int tag;struct Cyc_Absyn_Stmt*
f1;};struct Cyc_Absyn_Tagcheck_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
_dyneither_ptr*f2;};struct Cyc_Absyn_Valueof_e_struct{int tag;void*f1;};struct Cyc_Absyn_Exp{
struct Cyc_Core_Opt*topt;void*r;struct Cyc_Position_Segment*loc;void*annot;};
struct Cyc_Absyn_Exp_s_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct
_tuple8{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_struct{
int tag;struct _tuple8 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Continue_s_struct{int tag;struct
Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Goto_s_struct{int tag;struct _dyneither_ptr*f1;
struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_For_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct _tuple8 f2;struct _tuple8 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Fallthru_s_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_struct{
int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_struct{
int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple8 f2;};struct Cyc_Absyn_TryCatch_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ResetRegion_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Stmt{void*r;struct Cyc_Position_Segment*
loc;struct Cyc_List_List*non_local_preds;int try_depth;void*annot;};struct Cyc_Absyn_Var_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_Reference_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_struct{
int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_struct{
int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_struct{int tag;
struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_struct{int tag;struct Cyc_Absyn_AggrInfo*
f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_struct{
int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct
Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Int_p_struct{int tag;void*f1;int f2;};
struct Cyc_Absyn_Char_p_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_struct{int
tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Enum_p_struct{int tag;struct Cyc_Absyn_Enumdecl*
f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_struct{int tag;void*
f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_struct{int tag;
struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_struct{int tag;struct _tuple0*f1;
struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_Pat{void*r;struct Cyc_Core_Opt*topt;struct Cyc_Position_Segment*
loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*
pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;struct Cyc_Position_Segment*
loc;};struct Cyc_Absyn_Global_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Funname_b_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Local_b_struct{int tag;struct
Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_struct{int tag;struct Cyc_Absyn_Vardecl*
f1;};struct Cyc_Absyn_Vardecl{void*sc;struct _tuple0*name;struct Cyc_Absyn_Tqual tq;
void*type;struct Cyc_Absyn_Exp*initializer;struct Cyc_Core_Opt*rgn;struct Cyc_List_List*
attributes;int escapes;};struct Cyc_Absyn_Fndecl{void*sc;int is_inline;struct
_tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*effect;void*ret_type;
struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;
struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;struct Cyc_Core_Opt*
cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;
struct Cyc_List_List*attributes;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*
name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*
attributes;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct
Cyc_List_List*rgn_po;struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{
void*kind;void*sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*
impl;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Datatypefield{struct
_tuple0*name;struct Cyc_List_List*typs;struct Cyc_Position_Segment*loc;void*sc;};
struct Cyc_Absyn_Datatypedecl{void*sc;struct _tuple0*name;struct Cyc_List_List*tvs;
struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct
_tuple0*name;struct Cyc_Absyn_Exp*tag;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Enumdecl{
void*sc;struct _tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{
struct _tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
kind;struct Cyc_Core_Opt*defn;struct Cyc_List_List*atts;};struct Cyc_Absyn_Var_d_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Let_d_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*
f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Letv_d_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_Region_d_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*
f2;int f3;struct Cyc_Absyn_Exp*f4;};struct Cyc_Absyn_Alias_d_struct{int tag;struct
Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Tvar*f2;struct Cyc_Absyn_Vardecl*f3;};struct Cyc_Absyn_Aggr_d_struct{
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Datatype_d_struct{int tag;
struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_Enum_d_struct{int tag;struct Cyc_Absyn_Enumdecl*
f1;};struct Cyc_Absyn_Typedef_d_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};
struct Cyc_Absyn_Namespace_d_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Using_d_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_ExternC_d_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Decl{void*
r;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_ArrayElement_struct{int tag;
struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_struct{int tag;struct
_dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[15];int Cyc_Absyn_qvar_cmp(
struct _tuple0*,struct _tuple0*);struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(struct
Cyc_Position_Segment*);union Cyc_Absyn_Constraint*Cyc_Absyn_empty_conref();void*
Cyc_Absyn_conref_val(union Cyc_Absyn_Constraint*x);void*Cyc_Absyn_wildtyp(struct
Cyc_Core_Opt*);extern void*Cyc_Absyn_sint_typ;void*Cyc_Absyn_string_typ(void*rgn);
void*Cyc_Absyn_dyneither_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,union Cyc_Absyn_Constraint*
zero_term);struct Cyc___cycFILE;extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_Cstdio___abstractFILE;
struct Cyc_String_pa_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Int_pa_struct{
int tag;unsigned long f1;};struct Cyc_Double_pa_struct{int tag;double f1;};struct Cyc_LongDouble_pa_struct{
int tag;long double f1;};struct Cyc_ShortPtr_pa_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_struct{
int tag;unsigned long*f1;};struct _dyneither_ptr Cyc_aprintf(struct _dyneither_ptr,
struct _dyneither_ptr);int Cyc_fprintf(struct Cyc___cycFILE*,struct _dyneither_ptr,
struct _dyneither_ptr);struct Cyc_ShortPtr_sa_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_struct{
int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_struct{
int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_struct{int tag;struct
_dyneither_ptr f1;};struct Cyc_DoublePtr_sa_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_struct{
int tag;float*f1;};struct Cyc_CharPtr_sa_struct{int tag;struct _dyneither_ptr f1;};
int Cyc_printf(struct _dyneither_ptr,struct _dyneither_ptr);extern char Cyc_FileCloseError[
19];extern char Cyc_FileOpenError[18];struct Cyc_FileOpenError_struct{char*tag;
struct _dyneither_ptr f1;};struct Cyc_PP_Ppstate;struct Cyc_PP_Out;struct Cyc_PP_Doc;
struct Cyc_Absynpp_Params{int expand_typedefs: 1;int qvar_to_Cids: 1;int
add_cyc_prefix: 1;int to_VC: 1;int decls_first: 1;int rewrite_temp_tvars: 1;int
print_all_tvars: 1;int print_all_kinds: 1;int print_all_effects: 1;int
print_using_stmts: 1;int print_externC_stmts: 1;int print_full_evars: 1;int
print_zeroterm: 1;int generate_line_directives: 1;int use_curr_namespace: 1;struct Cyc_List_List*
curr_namespace;};struct _dyneither_ptr Cyc_Absynpp_exp2string(struct Cyc_Absyn_Exp*);
struct _dyneither_ptr Cyc_Absynpp_qvar2string(struct _tuple0*);struct Cyc_Iter_Iter{
void*env;int(*next)(void*env,void*dest);};int Cyc_Iter_next(struct Cyc_Iter_Iter,
void*);struct Cyc_Set_Set;extern char Cyc_Set_Absent[11];struct Cyc_Dict_T;struct Cyc_Dict_Dict{
int(*rel)(void*,void*);struct _RegionHandle*r;struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[
12];extern char Cyc_Dict_Absent[11];struct Cyc_Dict_Dict Cyc_Dict_empty(int(*cmp)(
void*,void*));int Cyc_Dict_member(struct Cyc_Dict_Dict d,void*k);struct Cyc_Dict_Dict
Cyc_Dict_insert(struct Cyc_Dict_Dict d,void*k,void*v);void*Cyc_Dict_lookup(struct
Cyc_Dict_Dict d,void*k);void**Cyc_Dict_lookup_opt(struct Cyc_Dict_Dict d,void*k);
struct _tuple9{void*f1;void*f2;};struct _tuple9*Cyc_Dict_rchoose(struct
_RegionHandle*r,struct Cyc_Dict_Dict d);struct _tuple9*Cyc_Dict_rchoose(struct
_RegionHandle*,struct Cyc_Dict_Dict d);struct Cyc_RgnOrder_RgnPO;struct Cyc_RgnOrder_RgnPO*
Cyc_RgnOrder_initial_fn_po(struct _RegionHandle*,struct Cyc_List_List*tvs,struct
Cyc_List_List*po,void*effect,struct Cyc_Absyn_Tvar*fst_rgn,struct Cyc_Position_Segment*);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(struct
_RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn,struct Cyc_Position_Segment*
loc);struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(struct _RegionHandle*,
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*rgn,int resetable,int opened);int
Cyc_RgnOrder_is_region_resetable(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*
r);int Cyc_RgnOrder_effect_outlives(struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);
int Cyc_RgnOrder_satisfies_constraints(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_List_List*
constraints,void*default_bound,int do_pin);int Cyc_RgnOrder_eff_outlives_eff(
struct Cyc_RgnOrder_RgnPO*po,void*eff1,void*eff2);void Cyc_RgnOrder_print_region_po(
struct Cyc_RgnOrder_RgnPO*po);struct Cyc_Tcenv_CList{void*hd;struct Cyc_Tcenv_CList*
tl;};struct Cyc_Tcenv_VarRes_struct{int tag;void*f1;};struct Cyc_Tcenv_AggrRes_struct{
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_DatatypeRes_struct{int tag;
struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct Cyc_Tcenv_EnumRes_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_struct{
int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_Genv{struct
_RegionHandle*grgn;struct Cyc_Set_Set*namespaces;struct Cyc_Dict_Dict aggrdecls;
struct Cyc_Dict_Dict datatypedecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict
typedefs;struct Cyc_Dict_Dict ordinaries;struct Cyc_List_List*availables;};struct
Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Stmt_j_struct{int tag;struct Cyc_Absyn_Stmt*f1;};
struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict ae;struct Cyc_Tcenv_Fenv*
le;int allow_valueof;};void*Cyc_Tcutil_impos(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap);struct Cyc_Tcexp_TestEnv{struct _tuple9*eq;int isTrue;};struct Cyc_Tcexp_TestEnv
Cyc_Tcexp_tcTest(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e,struct
_dyneither_ptr msg_part);extern char Cyc_Tcdecl_Incompatible[17];struct Cyc_Tcdecl_Xdatatypefielddecl{
struct Cyc_Absyn_Datatypedecl*base;struct Cyc_Absyn_Datatypefield*field;};struct
Cyc_Tcgenrep_RepInfo;struct Cyc_Position_Segment{int start;int end;};struct Cyc_Port_Edit{
struct Cyc_Position_Segment*loc;struct _dyneither_ptr old_string;struct
_dyneither_ptr new_string;};int Cyc_Port_cmp_edit(struct Cyc_Port_Edit*e1,struct Cyc_Port_Edit*
e2);int Cyc_Port_cmp_edit(struct Cyc_Port_Edit*e1,struct Cyc_Port_Edit*e2){if((
unsigned int)e1->loc  && (unsigned int)e2->loc)return((struct Cyc_Position_Segment*)
_check_null(e1->loc))->start - ((struct Cyc_Position_Segment*)_check_null(e2->loc))->start;
else{if(e1->loc == e2->loc)return 0;else{if((unsigned int)e1->loc)return 1;else{
return - 1;}}}}struct Cyc_Port_Cvar{int id;void**eq;struct Cyc_List_List*uppers;
struct Cyc_List_List*lowers;};struct Cyc_Port_Cfield{void*qual;struct
_dyneither_ptr*f;void*type;};struct Cyc_Port_RgnVar_ct_struct{int tag;struct
_dyneither_ptr*f1;};struct Cyc_Port_Ptr_ct_struct{int tag;void*f1;void*f2;void*f3;
void*f4;void*f5;};struct Cyc_Port_Array_ct_struct{int tag;void*f1;void*f2;void*f3;
};struct _tuple10{struct Cyc_Absyn_Aggrdecl*f1;struct Cyc_List_List*f2;};struct Cyc_Port_KnownAggr_ct_struct{
int tag;struct _tuple10*f1;};struct Cyc_Port_UnknownAggr_ct_struct{int tag;struct Cyc_List_List*
f1;void**f2;};struct Cyc_Port_Fn_ct_struct{int tag;void*f1;struct Cyc_List_List*f2;
};struct Cyc_Port_Var_ct_struct{int tag;struct Cyc_Port_Cvar*f1;};static struct
_dyneither_ptr Cyc_Port_ctypes2string(int deep,struct Cyc_List_List*ts);static
struct _dyneither_ptr Cyc_Port_cfields2string(int deep,struct Cyc_List_List*ts);
static struct _dyneither_ptr Cyc_Port_ctype2string(int deep,void*t);static struct
_dyneither_ptr Cyc_Port_ctype2string(int deep,void*t){void*_tmp0=t;struct
_dyneither_ptr*_tmp1;void*_tmp2;void*_tmp3;void*_tmp4;void*_tmp5;void*_tmp6;void*
_tmp7;void*_tmp8;void*_tmp9;struct _tuple10*_tmpA;struct _tuple10 _tmpB;struct Cyc_Absyn_Aggrdecl*
_tmpC;struct Cyc_List_List*_tmpD;struct Cyc_List_List*_tmpE;void**_tmpF;void*
_tmp10;struct Cyc_List_List*_tmp11;void*_tmp12;struct Cyc_List_List*_tmp13;struct
Cyc_Port_Cvar*_tmp14;_LL1: if((int)_tmp0 != 0)goto _LL3;_LL2: {const char*_tmp3EC;
return(_tmp3EC="const",_tag_dyneither(_tmp3EC,sizeof(char),6));}_LL3: if((int)
_tmp0 != 1)goto _LL5;_LL4: {const char*_tmp3ED;return(_tmp3ED="notconst",
_tag_dyneither(_tmp3ED,sizeof(char),9));}_LL5: if((int)_tmp0 != 2)goto _LL7;_LL6: {
const char*_tmp3EE;return(_tmp3EE="thin",_tag_dyneither(_tmp3EE,sizeof(char),5));}
_LL7: if((int)_tmp0 != 3)goto _LL9;_LL8: {const char*_tmp3EF;return(_tmp3EF="fat",
_tag_dyneither(_tmp3EF,sizeof(char),4));}_LL9: if((int)_tmp0 != 4)goto _LLB;_LLA: {
const char*_tmp3F0;return(_tmp3F0="void",_tag_dyneither(_tmp3F0,sizeof(char),5));}
_LLB: if((int)_tmp0 != 5)goto _LLD;_LLC: {const char*_tmp3F1;return(_tmp3F1="zero",
_tag_dyneither(_tmp3F1,sizeof(char),5));}_LLD: if((int)_tmp0 != 6)goto _LLF;_LLE: {
const char*_tmp3F2;return(_tmp3F2="arith",_tag_dyneither(_tmp3F2,sizeof(char),6));}
_LLF: if((int)_tmp0 != 7)goto _LL11;_LL10: {const char*_tmp3F3;return(_tmp3F3="heap",
_tag_dyneither(_tmp3F3,sizeof(char),5));}_LL11: if((int)_tmp0 != 8)goto _LL13;_LL12: {
const char*_tmp3F4;return(_tmp3F4="ZT",_tag_dyneither(_tmp3F4,sizeof(char),3));}
_LL13: if((int)_tmp0 != 9)goto _LL15;_LL14: {const char*_tmp3F5;return(_tmp3F5="NZT",
_tag_dyneither(_tmp3F5,sizeof(char),4));}_LL15: if(_tmp0 <= (void*)10)goto _LL17;
if(*((int*)_tmp0)!= 0)goto _LL17;_tmp1=((struct Cyc_Port_RgnVar_ct_struct*)_tmp0)->f1;
_LL16: {const char*_tmp3F9;void*_tmp3F8[1];struct Cyc_String_pa_struct _tmp3F7;
return(struct _dyneither_ptr)((_tmp3F7.tag=0,((_tmp3F7.f1=(struct _dyneither_ptr)((
struct _dyneither_ptr)*_tmp1),((_tmp3F8[0]=& _tmp3F7,Cyc_aprintf(((_tmp3F9="%s",
_tag_dyneither(_tmp3F9,sizeof(char),3))),_tag_dyneither(_tmp3F8,sizeof(void*),1))))))));}
_LL17: if(_tmp0 <= (void*)10)goto _LL19;if(*((int*)_tmp0)!= 1)goto _LL19;_tmp2=(void*)((
struct Cyc_Port_Ptr_ct_struct*)_tmp0)->f1;_tmp3=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp0)->f2;_tmp4=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp0)->f3;_tmp5=(void*)((
struct Cyc_Port_Ptr_ct_struct*)_tmp0)->f4;_tmp6=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp0)->f5;_LL18: {const char*_tmp401;void*_tmp400[5];struct Cyc_String_pa_struct
_tmp3FF;struct Cyc_String_pa_struct _tmp3FE;struct Cyc_String_pa_struct _tmp3FD;
struct Cyc_String_pa_struct _tmp3FC;struct Cyc_String_pa_struct _tmp3FB;return(
struct _dyneither_ptr)((_tmp3FB.tag=0,((_tmp3FB.f1=(struct _dyneither_ptr)((struct
_dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp6)),((_tmp3FC.tag=0,((_tmp3FC.f1=(
struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp5)),((
_tmp3FD.tag=0,((_tmp3FD.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(
deep,_tmp4)),((_tmp3FE.tag=0,((_tmp3FE.f1=(struct _dyneither_ptr)((struct
_dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp3)),((_tmp3FF.tag=0,((_tmp3FF.f1=(
struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp2)),((
_tmp400[0]=& _tmp3FF,((_tmp400[1]=& _tmp3FE,((_tmp400[2]=& _tmp3FD,((_tmp400[3]=&
_tmp3FC,((_tmp400[4]=& _tmp3FB,Cyc_aprintf(((_tmp401="ptr(%s,%s,%s,%s,%s)",
_tag_dyneither(_tmp401,sizeof(char),20))),_tag_dyneither(_tmp400,sizeof(void*),5))))))))))))))))))))))))))))))));}
_LL19: if(_tmp0 <= (void*)10)goto _LL1B;if(*((int*)_tmp0)!= 2)goto _LL1B;_tmp7=(void*)((
struct Cyc_Port_Array_ct_struct*)_tmp0)->f1;_tmp8=(void*)((struct Cyc_Port_Array_ct_struct*)
_tmp0)->f2;_tmp9=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp0)->f3;_LL1A: {
const char*_tmp407;void*_tmp406[3];struct Cyc_String_pa_struct _tmp405;struct Cyc_String_pa_struct
_tmp404;struct Cyc_String_pa_struct _tmp403;return(struct _dyneither_ptr)((_tmp403.tag=
0,((_tmp403.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(
deep,_tmp9)),((_tmp404.tag=0,((_tmp404.f1=(struct _dyneither_ptr)((struct
_dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp8)),((_tmp405.tag=0,((_tmp405.f1=(
struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp7)),((
_tmp406[0]=& _tmp405,((_tmp406[1]=& _tmp404,((_tmp406[2]=& _tmp403,Cyc_aprintf(((
_tmp407="array(%s,%s,%s)",_tag_dyneither(_tmp407,sizeof(char),16))),
_tag_dyneither(_tmp406,sizeof(void*),3))))))))))))))))))));}_LL1B: if(_tmp0 <= (
void*)10)goto _LL1D;if(*((int*)_tmp0)!= 3)goto _LL1D;_tmpA=((struct Cyc_Port_KnownAggr_ct_struct*)
_tmp0)->f1;_tmpB=*_tmpA;_tmpC=_tmpB.f1;_tmpD=_tmpB.f2;_LL1C: {const char*_tmp409;
const char*_tmp408;struct _dyneither_ptr s=_tmpC->kind == (void*)0?(_tmp409="struct",
_tag_dyneither(_tmp409,sizeof(char),7)):((_tmp408="union",_tag_dyneither(_tmp408,
sizeof(char),6)));if(!deep){const char*_tmp40E;void*_tmp40D[2];struct Cyc_String_pa_struct
_tmp40C;struct Cyc_String_pa_struct _tmp40B;return(struct _dyneither_ptr)((_tmp40B.tag=
0,((_tmp40B.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(
_tmpC->name)),((_tmp40C.tag=0,((_tmp40C.f1=(struct _dyneither_ptr)((struct
_dyneither_ptr)s),((_tmp40D[0]=& _tmp40C,((_tmp40D[1]=& _tmp40B,Cyc_aprintf(((
_tmp40E="%s %s",_tag_dyneither(_tmp40E,sizeof(char),6))),_tag_dyneither(_tmp40D,
sizeof(void*),2))))))))))))));}else{const char*_tmp414;void*_tmp413[3];struct Cyc_String_pa_struct
_tmp412;struct Cyc_String_pa_struct _tmp411;struct Cyc_String_pa_struct _tmp410;
return(struct _dyneither_ptr)((_tmp410.tag=0,((_tmp410.f1=(struct _dyneither_ptr)((
struct _dyneither_ptr)Cyc_Port_cfields2string(0,_tmpD)),((_tmp411.tag=0,((_tmp411.f1=(
struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmpC->name)),((
_tmp412.tag=0,((_tmp412.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s),((
_tmp413[0]=& _tmp412,((_tmp413[1]=& _tmp411,((_tmp413[2]=& _tmp410,Cyc_aprintf(((
_tmp414="%s %s {%s}",_tag_dyneither(_tmp414,sizeof(char),11))),_tag_dyneither(
_tmp413,sizeof(void*),3))))))))))))))))))));}}_LL1D: if(_tmp0 <= (void*)10)goto
_LL1F;if(*((int*)_tmp0)!= 4)goto _LL1F;_tmpE=((struct Cyc_Port_UnknownAggr_ct_struct*)
_tmp0)->f1;_tmpF=((struct Cyc_Port_UnknownAggr_ct_struct*)_tmp0)->f2;if(_tmpF == 0)
goto _LL1F;_tmp10=*_tmpF;_LL1E: return Cyc_Port_ctype2string(deep,_tmp10);_LL1F: if(
_tmp0 <= (void*)10)goto _LL21;if(*((int*)_tmp0)!= 4)goto _LL21;_tmp11=((struct Cyc_Port_UnknownAggr_ct_struct*)
_tmp0)->f1;_LL20: {const char*_tmp418;void*_tmp417[1];struct Cyc_String_pa_struct
_tmp416;return(struct _dyneither_ptr)((_tmp416.tag=0,((_tmp416.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_cfields2string(deep,_tmp11)),((
_tmp417[0]=& _tmp416,Cyc_aprintf(((_tmp418="aggr {%s}",_tag_dyneither(_tmp418,
sizeof(char),10))),_tag_dyneither(_tmp417,sizeof(void*),1))))))));}_LL21: if(
_tmp0 <= (void*)10)goto _LL23;if(*((int*)_tmp0)!= 5)goto _LL23;_tmp12=(void*)((
struct Cyc_Port_Fn_ct_struct*)_tmp0)->f1;_tmp13=((struct Cyc_Port_Fn_ct_struct*)
_tmp0)->f2;_LL22: {const char*_tmp41D;void*_tmp41C[2];struct Cyc_String_pa_struct
_tmp41B;struct Cyc_String_pa_struct _tmp41A;return(struct _dyneither_ptr)((_tmp41A.tag=
0,((_tmp41A.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(
deep,_tmp12)),((_tmp41B.tag=0,((_tmp41B.f1=(struct _dyneither_ptr)((struct
_dyneither_ptr)Cyc_Port_ctypes2string(deep,_tmp13)),((_tmp41C[0]=& _tmp41B,((
_tmp41C[1]=& _tmp41A,Cyc_aprintf(((_tmp41D="fn(%s)->%s",_tag_dyneither(_tmp41D,
sizeof(char),11))),_tag_dyneither(_tmp41C,sizeof(void*),2))))))))))))));}_LL23:
if(_tmp0 <= (void*)10)goto _LL0;if(*((int*)_tmp0)!= 6)goto _LL0;_tmp14=((struct Cyc_Port_Var_ct_struct*)
_tmp0)->f1;_LL24: if((unsigned int)_tmp14->eq)return Cyc_Port_ctype2string(deep,*((
void**)_check_null(_tmp14->eq)));else{if(!deep  || _tmp14->uppers == 0  && _tmp14->lowers
== 0){const char*_tmp421;void*_tmp420[1];struct Cyc_Int_pa_struct _tmp41F;return(
struct _dyneither_ptr)((_tmp41F.tag=1,((_tmp41F.f1=(unsigned long)_tmp14->id,((
_tmp420[0]=& _tmp41F,Cyc_aprintf(((_tmp421="var(%d)",_tag_dyneither(_tmp421,
sizeof(char),8))),_tag_dyneither(_tmp420,sizeof(void*),1))))))));}else{const char*
_tmp427;void*_tmp426[3];struct Cyc_String_pa_struct _tmp425;struct Cyc_Int_pa_struct
_tmp424;struct Cyc_String_pa_struct _tmp423;return(struct _dyneither_ptr)((_tmp423.tag=
0,((_tmp423.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctypes2string(
0,_tmp14->uppers)),((_tmp424.tag=1,((_tmp424.f1=(unsigned long)_tmp14->id,((
_tmp425.tag=0,((_tmp425.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctypes2string(
0,_tmp14->lowers)),((_tmp426[0]=& _tmp425,((_tmp426[1]=& _tmp424,((_tmp426[2]=&
_tmp423,Cyc_aprintf(((_tmp427="var([%s]<=%d<=[%s])",_tag_dyneither(_tmp427,
sizeof(char),20))),_tag_dyneither(_tmp426,sizeof(void*),3))))))))))))))))))));}}
_LL0:;}static struct _dyneither_ptr*Cyc_Port_ctype2stringptr(int deep,void*t);
static struct _dyneither_ptr*Cyc_Port_ctype2stringptr(int deep,void*t){struct
_dyneither_ptr*_tmp428;return(_tmp428=_cycalloc(sizeof(*_tmp428)),((_tmp428[0]=
Cyc_Port_ctype2string(deep,t),_tmp428)));}struct Cyc_List_List*Cyc_Port_sep(
struct _dyneither_ptr s,struct Cyc_List_List*xs);struct Cyc_List_List*Cyc_Port_sep(
struct _dyneither_ptr s,struct Cyc_List_List*xs){struct _dyneither_ptr*_tmp429;
struct _dyneither_ptr*_tmp49=(_tmp429=_cycalloc(sizeof(*_tmp429)),((_tmp429[0]=s,
_tmp429)));if(xs == 0)return xs;{struct Cyc_List_List*_tmp4A=xs;struct Cyc_List_List*
_tmp4B=xs->tl;for(0;_tmp4B != 0;(_tmp4A=_tmp4B,_tmp4B=_tmp4B->tl)){struct Cyc_List_List*
_tmp42A;((struct Cyc_List_List*)_check_null(_tmp4A))->tl=((_tmp42A=_cycalloc(
sizeof(*_tmp42A)),((_tmp42A->hd=_tmp49,((_tmp42A->tl=_tmp4B,_tmp42A))))));}
return xs;}}static struct _dyneither_ptr*Cyc_Port_cfield2stringptr(int deep,struct
Cyc_Port_Cfield*f);static struct _dyneither_ptr*Cyc_Port_cfield2stringptr(int deep,
struct Cyc_Port_Cfield*f){const char*_tmp430;void*_tmp42F[3];struct Cyc_String_pa_struct
_tmp42E;struct Cyc_String_pa_struct _tmp42D;struct Cyc_String_pa_struct _tmp42C;
struct _dyneither_ptr s=(struct _dyneither_ptr)((_tmp42C.tag=0,((_tmp42C.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,f->type)),((
_tmp42D.tag=0,((_tmp42D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f->f),((
_tmp42E.tag=0,((_tmp42E.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(
deep,f->qual)),((_tmp42F[0]=& _tmp42E,((_tmp42F[1]=& _tmp42D,((_tmp42F[2]=& _tmp42C,
Cyc_aprintf(((_tmp430="%s %s: %s",_tag_dyneither(_tmp430,sizeof(char),10))),
_tag_dyneither(_tmp42F,sizeof(void*),3))))))))))))))))))));struct _dyneither_ptr*
_tmp431;return(_tmp431=_cycalloc(sizeof(*_tmp431)),((_tmp431[0]=s,_tmp431)));}
static struct _dyneither_ptr Cyc_Port_ctypes2string(int deep,struct Cyc_List_List*ts);
static struct _dyneither_ptr Cyc_Port_ctypes2string(int deep,struct Cyc_List_List*ts){
const char*_tmp432;return(struct _dyneither_ptr)Cyc_strconcat_l(Cyc_Port_sep(((
_tmp432=",",_tag_dyneither(_tmp432,sizeof(char),2))),((struct Cyc_List_List*(*)(
struct _dyneither_ptr*(*f)(int,void*),int env,struct Cyc_List_List*x))Cyc_List_map_c)(
Cyc_Port_ctype2stringptr,deep,ts)));}static struct _dyneither_ptr Cyc_Port_cfields2string(
int deep,struct Cyc_List_List*fs);static struct _dyneither_ptr Cyc_Port_cfields2string(
int deep,struct Cyc_List_List*fs){const char*_tmp433;return(struct _dyneither_ptr)
Cyc_strconcat_l(Cyc_Port_sep(((_tmp433=";",_tag_dyneither(_tmp433,sizeof(char),2))),((
struct Cyc_List_List*(*)(struct _dyneither_ptr*(*f)(int,struct Cyc_Port_Cfield*),
int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_cfield2stringptr,deep,fs)));}
static void*Cyc_Port_notconst_ct();static void*Cyc_Port_notconst_ct(){return(void*)
1;}static void*Cyc_Port_const_ct();static void*Cyc_Port_const_ct(){return(void*)0;}
static void*Cyc_Port_thin_ct();static void*Cyc_Port_thin_ct(){return(void*)2;}
static void*Cyc_Port_fat_ct();static void*Cyc_Port_fat_ct(){return(void*)3;}static
void*Cyc_Port_void_ct();static void*Cyc_Port_void_ct(){return(void*)4;}static void*
Cyc_Port_zero_ct();static void*Cyc_Port_zero_ct(){return(void*)5;}static void*Cyc_Port_arith_ct();
static void*Cyc_Port_arith_ct(){return(void*)6;}static void*Cyc_Port_heap_ct();
static void*Cyc_Port_heap_ct(){return(void*)7;}static void*Cyc_Port_zterm_ct();
static void*Cyc_Port_zterm_ct(){return(void*)8;}static void*Cyc_Port_nozterm_ct();
static void*Cyc_Port_nozterm_ct(){return(void*)9;}static void*Cyc_Port_rgnvar_ct(
struct _dyneither_ptr*n);static void*Cyc_Port_rgnvar_ct(struct _dyneither_ptr*n){
struct Cyc_Port_RgnVar_ct_struct _tmp436;struct Cyc_Port_RgnVar_ct_struct*_tmp435;
return(void*)((_tmp435=_cycalloc(sizeof(*_tmp435)),((_tmp435[0]=((_tmp436.tag=0,((
_tmp436.f1=n,_tmp436)))),_tmp435))));}static void*Cyc_Port_unknown_aggr_ct(struct
Cyc_List_List*fs);static void*Cyc_Port_unknown_aggr_ct(struct Cyc_List_List*fs){
struct Cyc_Port_UnknownAggr_ct_struct _tmp439;struct Cyc_Port_UnknownAggr_ct_struct*
_tmp438;return(void*)((_tmp438=_cycalloc(sizeof(*_tmp438)),((_tmp438[0]=((
_tmp439.tag=4,((_tmp439.f1=fs,((_tmp439.f2=0,_tmp439)))))),_tmp438))));}static
void*Cyc_Port_known_aggr_ct(struct _tuple10*p);static void*Cyc_Port_known_aggr_ct(
struct _tuple10*p){struct Cyc_Port_KnownAggr_ct_struct _tmp43C;struct Cyc_Port_KnownAggr_ct_struct*
_tmp43B;return(void*)((_tmp43B=_cycalloc(sizeof(*_tmp43B)),((_tmp43B[0]=((
_tmp43C.tag=3,((_tmp43C.f1=p,_tmp43C)))),_tmp43B))));}static void*Cyc_Port_ptr_ct(
void*elt,void*qual,void*ptr_kind,void*r,void*zt);static void*Cyc_Port_ptr_ct(void*
elt,void*qual,void*ptr_kind,void*r,void*zt){struct Cyc_Port_Ptr_ct_struct _tmp43F;
struct Cyc_Port_Ptr_ct_struct*_tmp43E;return(void*)((_tmp43E=_cycalloc(sizeof(*
_tmp43E)),((_tmp43E[0]=((_tmp43F.tag=1,((_tmp43F.f1=(void*)elt,((_tmp43F.f2=(
void*)qual,((_tmp43F.f3=(void*)ptr_kind,((_tmp43F.f4=(void*)r,((_tmp43F.f5=(void*)
zt,_tmp43F)))))))))))),_tmp43E))));}static void*Cyc_Port_array_ct(void*elt,void*
qual,void*zt);static void*Cyc_Port_array_ct(void*elt,void*qual,void*zt){struct Cyc_Port_Array_ct_struct
_tmp442;struct Cyc_Port_Array_ct_struct*_tmp441;return(void*)((_tmp441=_cycalloc(
sizeof(*_tmp441)),((_tmp441[0]=((_tmp442.tag=2,((_tmp442.f1=(void*)elt,((_tmp442.f2=(
void*)qual,((_tmp442.f3=(void*)zt,_tmp442)))))))),_tmp441))));}static void*Cyc_Port_fn_ct(
void*return_type,struct Cyc_List_List*args);static void*Cyc_Port_fn_ct(void*
return_type,struct Cyc_List_List*args){struct Cyc_Port_Fn_ct_struct _tmp445;struct
Cyc_Port_Fn_ct_struct*_tmp444;return(void*)((_tmp444=_cycalloc(sizeof(*_tmp444)),((
_tmp444[0]=((_tmp445.tag=5,((_tmp445.f1=(void*)return_type,((_tmp445.f2=args,
_tmp445)))))),_tmp444))));}static void*Cyc_Port_var();static void*Cyc_Port_var(){
static int counter=0;struct Cyc_Port_Var_ct_struct _tmp44B;struct Cyc_Port_Cvar*
_tmp44A;struct Cyc_Port_Var_ct_struct*_tmp449;return(void*)((_tmp449=_cycalloc(
sizeof(*_tmp449)),((_tmp449[0]=((_tmp44B.tag=6,((_tmp44B.f1=((_tmp44A=_cycalloc(
sizeof(*_tmp44A)),((_tmp44A->id=counter ++,((_tmp44A->eq=0,((_tmp44A->uppers=0,((
_tmp44A->lowers=0,_tmp44A)))))))))),_tmp44B)))),_tmp449))));}static void*Cyc_Port_new_var(
void*x);static void*Cyc_Port_new_var(void*x){return Cyc_Port_var();}static struct
_dyneither_ptr*Cyc_Port_new_region_var();static struct _dyneither_ptr*Cyc_Port_new_region_var(){
static int counter=0;const char*_tmp44F;void*_tmp44E[1];struct Cyc_Int_pa_struct
_tmp44D;struct _dyneither_ptr s=(struct _dyneither_ptr)((_tmp44D.tag=1,((_tmp44D.f1=(
unsigned long)counter ++,((_tmp44E[0]=& _tmp44D,Cyc_aprintf(((_tmp44F="`R%d",
_tag_dyneither(_tmp44F,sizeof(char),5))),_tag_dyneither(_tmp44E,sizeof(void*),1))))))));
struct _dyneither_ptr*_tmp450;return(_tmp450=_cycalloc(sizeof(*_tmp450)),((
_tmp450[0]=s,_tmp450)));}static int Cyc_Port_unifies(void*t1,void*t2);static void*
Cyc_Port_compress_ct(void*t);static void*Cyc_Port_compress_ct(void*t){void*_tmp69=
t;struct Cyc_Port_Cvar*_tmp6A;struct Cyc_Port_Cvar _tmp6B;void**_tmp6C;void***
_tmp6D;struct Cyc_List_List*_tmp6E;struct Cyc_List_List*_tmp6F;void**_tmp70;_LL26:
if(_tmp69 <= (void*)10)goto _LL2A;if(*((int*)_tmp69)!= 6)goto _LL28;_tmp6A=((struct
Cyc_Port_Var_ct_struct*)_tmp69)->f1;_tmp6B=*_tmp6A;_tmp6C=_tmp6B.eq;_tmp6D=(void***)&(*((
struct Cyc_Port_Var_ct_struct*)_tmp69)->f1).eq;_tmp6E=_tmp6B.uppers;_tmp6F=_tmp6B.lowers;
_LL27: {void**_tmp71=*_tmp6D;if((unsigned int)_tmp71){void*r=Cyc_Port_compress_ct(*
_tmp71);if(*_tmp71 != r){void**_tmp451;*_tmp6D=((_tmp451=_cycalloc(sizeof(*
_tmp451)),((_tmp451[0]=r,_tmp451))));}return r;}for(0;_tmp6F != 0;_tmp6F=_tmp6F->tl){
void*_tmp73=(void*)_tmp6F->hd;_LL2D: if((int)_tmp73 != 0)goto _LL2F;_LL2E: goto _LL30;
_LL2F: if((int)_tmp73 != 9)goto _LL31;_LL30: goto _LL32;_LL31: if((int)_tmp73 != 4)goto
_LL33;_LL32: goto _LL34;_LL33: if(_tmp73 <= (void*)10)goto _LL37;if(*((int*)_tmp73)!= 
3)goto _LL35;_LL34: goto _LL36;_LL35: if(*((int*)_tmp73)!= 5)goto _LL37;_LL36:{void**
_tmp452;*_tmp6D=((_tmp452=_cycalloc(sizeof(*_tmp452)),((_tmp452[0]=(void*)_tmp6F->hd,
_tmp452))));}return(void*)_tmp6F->hd;_LL37:;_LL38: goto _LL2C;_LL2C:;}for(0;_tmp6E
!= 0;_tmp6E=_tmp6E->tl){void*_tmp75=(void*)_tmp6E->hd;_LL3A: if((int)_tmp75 != 1)
goto _LL3C;_LL3B: goto _LL3D;_LL3C: if((int)_tmp75 != 8)goto _LL3E;_LL3D: goto _LL3F;
_LL3E: if((int)_tmp75 != 5)goto _LL40;_LL3F: goto _LL41;_LL40: if(_tmp75 <= (void*)10)
goto _LL44;if(*((int*)_tmp75)!= 3)goto _LL42;_LL41: goto _LL43;_LL42: if(*((int*)
_tmp75)!= 5)goto _LL44;_LL43:{void**_tmp453;*_tmp6D=((_tmp453=_cycalloc(sizeof(*
_tmp453)),((_tmp453[0]=(void*)_tmp6E->hd,_tmp453))));}return(void*)_tmp6E->hd;
_LL44:;_LL45: goto _LL39;_LL39:;}return t;}_LL28: if(*((int*)_tmp69)!= 4)goto _LL2A;
_tmp70=((struct Cyc_Port_UnknownAggr_ct_struct*)_tmp69)->f2;_LL29: if((
unsigned int)_tmp70)return Cyc_Port_compress_ct(*_tmp70);else{return t;}_LL2A:;
_LL2B: return t;_LL25:;}static void*Cyc_Port_inst(struct Cyc_Dict_Dict*instenv,void*
t);static void*Cyc_Port_inst(struct Cyc_Dict_Dict*instenv,void*t){void*_tmp77=Cyc_Port_compress_ct(
t);struct _dyneither_ptr*_tmp78;void*_tmp79;void*_tmp7A;void*_tmp7B;void*_tmp7C;
void*_tmp7D;void*_tmp7E;void*_tmp7F;void*_tmp80;void*_tmp81;struct Cyc_List_List*
_tmp82;_LL47: if((int)_tmp77 != 0)goto _LL49;_LL48: goto _LL4A;_LL49: if((int)_tmp77 != 
1)goto _LL4B;_LL4A: goto _LL4C;_LL4B: if((int)_tmp77 != 2)goto _LL4D;_LL4C: goto _LL4E;
_LL4D: if((int)_tmp77 != 3)goto _LL4F;_LL4E: goto _LL50;_LL4F: if((int)_tmp77 != 4)goto
_LL51;_LL50: goto _LL52;_LL51: if((int)_tmp77 != 5)goto _LL53;_LL52: goto _LL54;_LL53:
if((int)_tmp77 != 6)goto _LL55;_LL54: goto _LL56;_LL55: if((int)_tmp77 != 8)goto _LL57;
_LL56: goto _LL58;_LL57: if((int)_tmp77 != 9)goto _LL59;_LL58: goto _LL5A;_LL59: if(
_tmp77 <= (void*)10)goto _LL5F;if(*((int*)_tmp77)!= 4)goto _LL5B;_LL5A: goto _LL5C;
_LL5B: if(*((int*)_tmp77)!= 3)goto _LL5D;_LL5C: goto _LL5E;_LL5D: if(*((int*)_tmp77)
!= 6)goto _LL5F;_LL5E: goto _LL60;_LL5F: if((int)_tmp77 != 7)goto _LL61;_LL60: return t;
_LL61: if(_tmp77 <= (void*)10)goto _LL63;if(*((int*)_tmp77)!= 0)goto _LL63;_tmp78=((
struct Cyc_Port_RgnVar_ct_struct*)_tmp77)->f1;_LL62: if(!((int(*)(struct Cyc_Dict_Dict
d,struct _dyneither_ptr*k))Cyc_Dict_member)(*instenv,_tmp78))*instenv=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _dyneither_ptr*k,void*v))Cyc_Dict_insert)(*instenv,
_tmp78,Cyc_Port_var());return((void*(*)(struct Cyc_Dict_Dict d,struct
_dyneither_ptr*k))Cyc_Dict_lookup)(*instenv,_tmp78);_LL63: if(_tmp77 <= (void*)10)
goto _LL65;if(*((int*)_tmp77)!= 1)goto _LL65;_tmp79=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp77)->f1;_tmp7A=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp77)->f2;_tmp7B=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp77)->f3;_tmp7C=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp77)->f4;_tmp7D=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp77)->f5;_LL64: {
void*_tmp85;void*_tmp86;struct _tuple9 _tmp454;struct _tuple9 _tmp84=(_tmp454.f1=Cyc_Port_inst(
instenv,_tmp79),((_tmp454.f2=Cyc_Port_inst(instenv,_tmp7C),_tmp454)));_tmp85=
_tmp84.f1;_tmp86=_tmp84.f2;if(_tmp85 == _tmp79  && _tmp86 == _tmp7C)return t;{struct
Cyc_Port_Ptr_ct_struct _tmp457;struct Cyc_Port_Ptr_ct_struct*_tmp456;return(void*)((
_tmp456=_cycalloc(sizeof(*_tmp456)),((_tmp456[0]=((_tmp457.tag=1,((_tmp457.f1=(
void*)_tmp85,((_tmp457.f2=(void*)_tmp7A,((_tmp457.f3=(void*)_tmp7B,((_tmp457.f4=(
void*)_tmp86,((_tmp457.f5=(void*)_tmp7D,_tmp457)))))))))))),_tmp456))));}}_LL65:
if(_tmp77 <= (void*)10)goto _LL67;if(*((int*)_tmp77)!= 2)goto _LL67;_tmp7E=(void*)((
struct Cyc_Port_Array_ct_struct*)_tmp77)->f1;_tmp7F=(void*)((struct Cyc_Port_Array_ct_struct*)
_tmp77)->f2;_tmp80=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp77)->f3;_LL66: {
void*_tmp89=Cyc_Port_inst(instenv,_tmp7E);if(_tmp89 == _tmp7E)return t;{struct Cyc_Port_Array_ct_struct
_tmp45A;struct Cyc_Port_Array_ct_struct*_tmp459;return(void*)((_tmp459=_cycalloc(
sizeof(*_tmp459)),((_tmp459[0]=((_tmp45A.tag=2,((_tmp45A.f1=(void*)_tmp89,((
_tmp45A.f2=(void*)_tmp7F,((_tmp45A.f3=(void*)_tmp80,_tmp45A)))))))),_tmp459))));}}
_LL67: if(_tmp77 <= (void*)10)goto _LL46;if(*((int*)_tmp77)!= 5)goto _LL46;_tmp81=(
void*)((struct Cyc_Port_Fn_ct_struct*)_tmp77)->f1;_tmp82=((struct Cyc_Port_Fn_ct_struct*)
_tmp77)->f2;_LL68: {struct Cyc_Port_Fn_ct_struct _tmp45D;struct Cyc_Port_Fn_ct_struct*
_tmp45C;return(void*)((_tmp45C=_cycalloc(sizeof(*_tmp45C)),((_tmp45C[0]=((
_tmp45D.tag=5,((_tmp45D.f1=(void*)Cyc_Port_inst(instenv,_tmp81),((_tmp45D.f2=((
struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Dict_Dict*,void*),struct Cyc_Dict_Dict*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_inst,instenv,_tmp82),_tmp45D)))))),
_tmp45C))));}_LL46:;}void*Cyc_Port_instantiate(void*t);void*Cyc_Port_instantiate(
void*t){struct Cyc_Dict_Dict*_tmp45E;return Cyc_Port_inst(((_tmp45E=_cycalloc(
sizeof(*_tmp45E)),((_tmp45E[0]=((struct Cyc_Dict_Dict(*)(int(*cmp)(struct
_dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Dict_empty)(Cyc_strptrcmp),_tmp45E)))),
t);}struct Cyc_List_List*Cyc_Port_filter_self(void*t,struct Cyc_List_List*ts);
struct Cyc_List_List*Cyc_Port_filter_self(void*t,struct Cyc_List_List*ts){int found=
0;{struct Cyc_List_List*_tmp8F=ts;for(0;(unsigned int)_tmp8F;_tmp8F=_tmp8F->tl){
void*_tmp90=Cyc_Port_compress_ct((void*)_tmp8F->hd);if(t == _tmp90)found=1;}}if(!
found)return ts;{struct Cyc_List_List*res=0;for(0;ts != 0;ts=ts->tl){if(t == Cyc_Port_compress_ct((
void*)ts->hd))continue;{struct Cyc_List_List*_tmp45F;res=((_tmp45F=_cycalloc(
sizeof(*_tmp45F)),((_tmp45F->hd=(void*)((void*)ts->hd),((_tmp45F->tl=res,_tmp45F))))));}}
return res;}}void Cyc_Port_generalize(int is_rgn,void*t);void Cyc_Port_generalize(
int is_rgn,void*t){t=Cyc_Port_compress_ct(t);{void*_tmp92=t;struct Cyc_Port_Cvar*
_tmp93;void*_tmp94;void*_tmp95;void*_tmp96;void*_tmp97;void*_tmp98;void*_tmp99;
void*_tmp9A;void*_tmp9B;void*_tmp9C;struct Cyc_List_List*_tmp9D;_LL6A: if(_tmp92 <= (
void*)10)goto _LL6C;if(*((int*)_tmp92)!= 6)goto _LL6C;_tmp93=((struct Cyc_Port_Var_ct_struct*)
_tmp92)->f1;_LL6B: _tmp93->uppers=Cyc_Port_filter_self(t,_tmp93->uppers);_tmp93->lowers=
Cyc_Port_filter_self(t,_tmp93->lowers);if(is_rgn){if(_tmp93->uppers == 0  && 
_tmp93->lowers == 0){Cyc_Port_unifies(t,Cyc_Port_rgnvar_ct(Cyc_Port_new_region_var()));
return;}if((unsigned int)_tmp93->uppers){Cyc_Port_unifies(t,(void*)((struct Cyc_List_List*)
_check_null(_tmp93->uppers))->hd);Cyc_Port_generalize(1,t);}else{Cyc_Port_unifies(
t,(void*)((struct Cyc_List_List*)_check_null(_tmp93->lowers))->hd);Cyc_Port_generalize(
1,t);}}return;_LL6C: if((int)_tmp92 != 0)goto _LL6E;_LL6D: goto _LL6F;_LL6E: if((int)
_tmp92 != 1)goto _LL70;_LL6F: goto _LL71;_LL70: if((int)_tmp92 != 2)goto _LL72;_LL71:
goto _LL73;_LL72: if((int)_tmp92 != 3)goto _LL74;_LL73: goto _LL75;_LL74: if((int)
_tmp92 != 4)goto _LL76;_LL75: goto _LL77;_LL76: if((int)_tmp92 != 5)goto _LL78;_LL77:
goto _LL79;_LL78: if((int)_tmp92 != 6)goto _LL7A;_LL79: goto _LL7B;_LL7A: if(_tmp92 <= (
void*)10)goto _LL80;if(*((int*)_tmp92)!= 4)goto _LL7C;_LL7B: goto _LL7D;_LL7C: if(*((
int*)_tmp92)!= 3)goto _LL7E;_LL7D: goto _LL7F;_LL7E: if(*((int*)_tmp92)!= 0)goto
_LL80;_LL7F: goto _LL81;_LL80: if((int)_tmp92 != 9)goto _LL82;_LL81: goto _LL83;_LL82:
if((int)_tmp92 != 8)goto _LL84;_LL83: goto _LL85;_LL84: if((int)_tmp92 != 7)goto _LL86;
_LL85: return;_LL86: if(_tmp92 <= (void*)10)goto _LL88;if(*((int*)_tmp92)!= 1)goto
_LL88;_tmp94=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp92)->f1;_tmp95=(void*)((
struct Cyc_Port_Ptr_ct_struct*)_tmp92)->f2;_tmp96=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp92)->f3;_tmp97=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp92)->f4;_tmp98=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp92)->f5;_LL87: Cyc_Port_generalize(0,
_tmp94);Cyc_Port_generalize(1,_tmp97);goto _LL69;_LL88: if(_tmp92 <= (void*)10)goto
_LL8A;if(*((int*)_tmp92)!= 2)goto _LL8A;_tmp99=(void*)((struct Cyc_Port_Array_ct_struct*)
_tmp92)->f1;_tmp9A=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp92)->f2;_tmp9B=(
void*)((struct Cyc_Port_Array_ct_struct*)_tmp92)->f3;_LL89: Cyc_Port_generalize(0,
_tmp99);Cyc_Port_generalize(0,_tmp9A);goto _LL69;_LL8A: if(_tmp92 <= (void*)10)goto
_LL69;if(*((int*)_tmp92)!= 5)goto _LL69;_tmp9C=(void*)((struct Cyc_Port_Fn_ct_struct*)
_tmp92)->f1;_tmp9D=((struct Cyc_Port_Fn_ct_struct*)_tmp92)->f2;_LL8B: Cyc_Port_generalize(
0,_tmp9C);((void(*)(void(*f)(int,void*),int env,struct Cyc_List_List*x))Cyc_List_iter_c)(
Cyc_Port_generalize,0,_tmp9D);goto _LL69;_LL69:;}}static int Cyc_Port_occurs(void*v,
void*t);static int Cyc_Port_occurs(void*v,void*t){t=Cyc_Port_compress_ct(t);if(t == 
v)return 1;{void*_tmp9E=t;void*_tmp9F;void*_tmpA0;void*_tmpA1;void*_tmpA2;void*
_tmpA3;void*_tmpA4;void*_tmpA5;void*_tmpA6;void*_tmpA7;struct Cyc_List_List*
_tmpA8;struct _tuple10*_tmpA9;struct _tuple10 _tmpAA;struct Cyc_List_List*_tmpAB;
struct Cyc_List_List*_tmpAC;_LL8D: if(_tmp9E <= (void*)10)goto _LL97;if(*((int*)
_tmp9E)!= 1)goto _LL8F;_tmp9F=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp9E)->f1;
_tmpA0=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp9E)->f2;_tmpA1=(void*)((struct
Cyc_Port_Ptr_ct_struct*)_tmp9E)->f3;_tmpA2=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp9E)->f4;_tmpA3=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp9E)->f5;_LL8E:
return(((Cyc_Port_occurs(v,_tmp9F) || Cyc_Port_occurs(v,_tmpA0)) || Cyc_Port_occurs(
v,_tmpA1)) || Cyc_Port_occurs(v,_tmpA2)) || Cyc_Port_occurs(v,_tmpA3);_LL8F: if(*((
int*)_tmp9E)!= 2)goto _LL91;_tmpA4=(void*)((struct Cyc_Port_Array_ct_struct*)
_tmp9E)->f1;_tmpA5=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp9E)->f2;_tmpA6=(
void*)((struct Cyc_Port_Array_ct_struct*)_tmp9E)->f3;_LL90: return(Cyc_Port_occurs(
v,_tmpA4) || Cyc_Port_occurs(v,_tmpA5)) || Cyc_Port_occurs(v,_tmpA6);_LL91: if(*((
int*)_tmp9E)!= 5)goto _LL93;_tmpA7=(void*)((struct Cyc_Port_Fn_ct_struct*)_tmp9E)->f1;
_tmpA8=((struct Cyc_Port_Fn_ct_struct*)_tmp9E)->f2;_LL92: if(Cyc_Port_occurs(v,
_tmpA7))return 1;for(0;(unsigned int)_tmpA8;_tmpA8=_tmpA8->tl){if(Cyc_Port_occurs(
v,(void*)_tmpA8->hd))return 1;}return 0;_LL93: if(*((int*)_tmp9E)!= 3)goto _LL95;
_tmpA9=((struct Cyc_Port_KnownAggr_ct_struct*)_tmp9E)->f1;_tmpAA=*_tmpA9;_tmpAB=
_tmpAA.f2;_LL94: return 0;_LL95: if(*((int*)_tmp9E)!= 4)goto _LL97;_tmpAC=((struct
Cyc_Port_UnknownAggr_ct_struct*)_tmp9E)->f1;_LL96: for(0;(unsigned int)_tmpAC;
_tmpAC=_tmpAC->tl){if(Cyc_Port_occurs(v,((struct Cyc_Port_Cfield*)_tmpAC->hd)->qual)
 || Cyc_Port_occurs(v,((struct Cyc_Port_Cfield*)_tmpAC->hd)->type))return 1;}
return 0;_LL97:;_LL98: return 0;_LL8C:;}}char Cyc_Port_Unify_ct[13]="\000\000\000\000Unify_ct\000";
static int Cyc_Port_leq(void*t1,void*t2);static void Cyc_Port_unify_cts(struct Cyc_List_List*
t1,struct Cyc_List_List*t2);static struct Cyc_List_List*Cyc_Port_merge_fields(
struct Cyc_List_List*fs1,struct Cyc_List_List*fs2,int allow_subset);static void Cyc_Port_unify_ct(
void*t1,void*t2);static void Cyc_Port_unify_ct(void*t1,void*t2){t1=Cyc_Port_compress_ct(
t1);t2=Cyc_Port_compress_ct(t2);if(t1 == t2)return;{struct _tuple9 _tmp460;struct
_tuple9 _tmpAE=(_tmp460.f1=t1,((_tmp460.f2=t2,_tmp460)));void*_tmpAF;struct Cyc_Port_Cvar*
_tmpB0;void*_tmpB1;struct Cyc_Port_Cvar*_tmpB2;void*_tmpB3;void*_tmpB4;void*
_tmpB5;void*_tmpB6;void*_tmpB7;void*_tmpB8;void*_tmpB9;void*_tmpBA;void*_tmpBB;
void*_tmpBC;void*_tmpBD;void*_tmpBE;void*_tmpBF;struct _dyneither_ptr*_tmpC0;
struct _dyneither_ptr _tmpC1;void*_tmpC2;struct _dyneither_ptr*_tmpC3;struct
_dyneither_ptr _tmpC4;void*_tmpC5;void*_tmpC6;void*_tmpC7;void*_tmpC8;void*_tmpC9;
void*_tmpCA;void*_tmpCB;void*_tmpCC;void*_tmpCD;void*_tmpCE;struct Cyc_List_List*
_tmpCF;void*_tmpD0;void*_tmpD1;struct Cyc_List_List*_tmpD2;void*_tmpD3;struct
_tuple10*_tmpD4;void*_tmpD5;struct _tuple10*_tmpD6;void*_tmpD7;struct Cyc_List_List*
_tmpD8;void**_tmpD9;void***_tmpDA;void*_tmpDB;struct Cyc_List_List*_tmpDC;void**
_tmpDD;void***_tmpDE;void*_tmpDF;struct Cyc_List_List*_tmpE0;void**_tmpE1;void***
_tmpE2;void*_tmpE3;struct _tuple10*_tmpE4;struct _tuple10 _tmpE5;struct Cyc_Absyn_Aggrdecl*
_tmpE6;struct Cyc_List_List*_tmpE7;void*_tmpE8;struct _tuple10*_tmpE9;struct
_tuple10 _tmpEA;struct Cyc_Absyn_Aggrdecl*_tmpEB;struct Cyc_List_List*_tmpEC;void*
_tmpED;struct Cyc_List_List*_tmpEE;void**_tmpEF;void***_tmpF0;_LL9A: _tmpAF=_tmpAE.f1;
if(_tmpAF <= (void*)10)goto _LL9C;if(*((int*)_tmpAF)!= 6)goto _LL9C;_tmpB0=((struct
Cyc_Port_Var_ct_struct*)_tmpAF)->f1;_LL9B: if(!Cyc_Port_occurs(t1,t2)){{struct Cyc_List_List*
_tmpF1=Cyc_Port_filter_self(t1,_tmpB0->uppers);for(0;(unsigned int)_tmpF1;_tmpF1=
_tmpF1->tl){if(!Cyc_Port_leq(t2,(void*)_tmpF1->hd))(int)_throw((void*)Cyc_Port_Unify_ct);}}{
struct Cyc_List_List*_tmpF2=Cyc_Port_filter_self(t1,_tmpB0->lowers);for(0;(
unsigned int)_tmpF2;_tmpF2=_tmpF2->tl){if(!Cyc_Port_leq((void*)_tmpF2->hd,t2))(
int)_throw((void*)Cyc_Port_Unify_ct);}}{void**_tmp461;_tmpB0->eq=((_tmp461=
_cycalloc(sizeof(*_tmp461)),((_tmp461[0]=t2,_tmp461))));}return;}else{(int)
_throw((void*)Cyc_Port_Unify_ct);}_LL9C: _tmpB1=_tmpAE.f2;if(_tmpB1 <= (void*)10)
goto _LL9E;if(*((int*)_tmpB1)!= 6)goto _LL9E;_tmpB2=((struct Cyc_Port_Var_ct_struct*)
_tmpB1)->f1;_LL9D: Cyc_Port_unify_ct(t2,t1);return;_LL9E: _tmpB3=_tmpAE.f1;if(
_tmpB3 <= (void*)10)goto _LLA0;if(*((int*)_tmpB3)!= 1)goto _LLA0;_tmpB4=(void*)((
struct Cyc_Port_Ptr_ct_struct*)_tmpB3)->f1;_tmpB5=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmpB3)->f2;_tmpB6=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmpB3)->f3;_tmpB7=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmpB3)->f4;_tmpB8=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmpB3)->f5;_tmpB9=_tmpAE.f2;if(_tmpB9 <= (void*)10)goto _LLA0;if(*((int*)_tmpB9)
!= 1)goto _LLA0;_tmpBA=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmpB9)->f1;_tmpBB=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmpB9)->f2;_tmpBC=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmpB9)->f3;_tmpBD=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmpB9)->f4;_tmpBE=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmpB9)->f5;_LL9F: Cyc_Port_unify_ct(_tmpB4,
_tmpBA);Cyc_Port_unify_ct(_tmpB5,_tmpBB);Cyc_Port_unify_ct(_tmpB6,_tmpBC);Cyc_Port_unify_ct(
_tmpB7,_tmpBD);Cyc_Port_unify_ct(_tmpB8,_tmpBE);return;_LLA0: _tmpBF=_tmpAE.f1;
if(_tmpBF <= (void*)10)goto _LLA2;if(*((int*)_tmpBF)!= 0)goto _LLA2;_tmpC0=((struct
Cyc_Port_RgnVar_ct_struct*)_tmpBF)->f1;_tmpC1=*_tmpC0;_tmpC2=_tmpAE.f2;if(_tmpC2
<= (void*)10)goto _LLA2;if(*((int*)_tmpC2)!= 0)goto _LLA2;_tmpC3=((struct Cyc_Port_RgnVar_ct_struct*)
_tmpC2)->f1;_tmpC4=*_tmpC3;_LLA1: if(Cyc_strcmp((struct _dyneither_ptr)_tmpC1,(
struct _dyneither_ptr)_tmpC4)!= 0)(int)_throw((void*)Cyc_Port_Unify_ct);return;
_LLA2: _tmpC5=_tmpAE.f1;if(_tmpC5 <= (void*)10)goto _LLA4;if(*((int*)_tmpC5)!= 2)
goto _LLA4;_tmpC6=(void*)((struct Cyc_Port_Array_ct_struct*)_tmpC5)->f1;_tmpC7=(
void*)((struct Cyc_Port_Array_ct_struct*)_tmpC5)->f2;_tmpC8=(void*)((struct Cyc_Port_Array_ct_struct*)
_tmpC5)->f3;_tmpC9=_tmpAE.f2;if(_tmpC9 <= (void*)10)goto _LLA4;if(*((int*)_tmpC9)
!= 2)goto _LLA4;_tmpCA=(void*)((struct Cyc_Port_Array_ct_struct*)_tmpC9)->f1;
_tmpCB=(void*)((struct Cyc_Port_Array_ct_struct*)_tmpC9)->f2;_tmpCC=(void*)((
struct Cyc_Port_Array_ct_struct*)_tmpC9)->f3;_LLA3: Cyc_Port_unify_ct(_tmpC6,
_tmpCA);Cyc_Port_unify_ct(_tmpC7,_tmpCB);Cyc_Port_unify_ct(_tmpC8,_tmpCC);
return;_LLA4: _tmpCD=_tmpAE.f1;if(_tmpCD <= (void*)10)goto _LLA6;if(*((int*)_tmpCD)
!= 5)goto _LLA6;_tmpCE=(void*)((struct Cyc_Port_Fn_ct_struct*)_tmpCD)->f1;_tmpCF=((
struct Cyc_Port_Fn_ct_struct*)_tmpCD)->f2;_tmpD0=_tmpAE.f2;if(_tmpD0 <= (void*)10)
goto _LLA6;if(*((int*)_tmpD0)!= 5)goto _LLA6;_tmpD1=(void*)((struct Cyc_Port_Fn_ct_struct*)
_tmpD0)->f1;_tmpD2=((struct Cyc_Port_Fn_ct_struct*)_tmpD0)->f2;_LLA5: Cyc_Port_unify_ct(
_tmpCE,_tmpD1);Cyc_Port_unify_cts(_tmpCF,_tmpD2);return;_LLA6: _tmpD3=_tmpAE.f1;
if(_tmpD3 <= (void*)10)goto _LLA8;if(*((int*)_tmpD3)!= 3)goto _LLA8;_tmpD4=((struct
Cyc_Port_KnownAggr_ct_struct*)_tmpD3)->f1;_tmpD5=_tmpAE.f2;if(_tmpD5 <= (void*)10)
goto _LLA8;if(*((int*)_tmpD5)!= 3)goto _LLA8;_tmpD6=((struct Cyc_Port_KnownAggr_ct_struct*)
_tmpD5)->f1;_LLA7: if(_tmpD4 == _tmpD6)return;else{(int)_throw((void*)Cyc_Port_Unify_ct);}
_LLA8: _tmpD7=_tmpAE.f1;if(_tmpD7 <= (void*)10)goto _LLAA;if(*((int*)_tmpD7)!= 4)
goto _LLAA;_tmpD8=((struct Cyc_Port_UnknownAggr_ct_struct*)_tmpD7)->f1;_tmpD9=((
struct Cyc_Port_UnknownAggr_ct_struct*)_tmpD7)->f2;_tmpDA=(void***)&((struct Cyc_Port_UnknownAggr_ct_struct*)
_tmpD7)->f2;_tmpDB=_tmpAE.f2;if(_tmpDB <= (void*)10)goto _LLAA;if(*((int*)_tmpDB)
!= 4)goto _LLAA;_tmpDC=((struct Cyc_Port_UnknownAggr_ct_struct*)_tmpDB)->f1;_tmpDD=((
struct Cyc_Port_UnknownAggr_ct_struct*)_tmpDB)->f2;_tmpDE=(void***)&((struct Cyc_Port_UnknownAggr_ct_struct*)
_tmpDB)->f2;_LLA9: {void*_tmpF4=Cyc_Port_unknown_aggr_ct(Cyc_Port_merge_fields(
_tmpD8,_tmpDC,1));{void**_tmp462;*_tmpDA=(*_tmpDE=((_tmp462=_cycalloc(sizeof(*
_tmp462)),((_tmp462[0]=_tmpF4,_tmp462)))));}return;}_LLAA: _tmpDF=_tmpAE.f1;if(
_tmpDF <= (void*)10)goto _LLAC;if(*((int*)_tmpDF)!= 4)goto _LLAC;_tmpE0=((struct Cyc_Port_UnknownAggr_ct_struct*)
_tmpDF)->f1;_tmpE1=((struct Cyc_Port_UnknownAggr_ct_struct*)_tmpDF)->f2;_tmpE2=(
void***)&((struct Cyc_Port_UnknownAggr_ct_struct*)_tmpDF)->f2;_tmpE3=_tmpAE.f2;
if(_tmpE3 <= (void*)10)goto _LLAC;if(*((int*)_tmpE3)!= 3)goto _LLAC;_tmpE4=((struct
Cyc_Port_KnownAggr_ct_struct*)_tmpE3)->f1;_tmpE5=*_tmpE4;_tmpE6=_tmpE5.f1;_tmpE7=
_tmpE5.f2;_LLAB: Cyc_Port_merge_fields(_tmpE7,_tmpE0,0);{void**_tmp463;*_tmpE2=((
_tmp463=_cycalloc(sizeof(*_tmp463)),((_tmp463[0]=t2,_tmp463))));}return;_LLAC:
_tmpE8=_tmpAE.f1;if(_tmpE8 <= (void*)10)goto _LLAE;if(*((int*)_tmpE8)!= 3)goto
_LLAE;_tmpE9=((struct Cyc_Port_KnownAggr_ct_struct*)_tmpE8)->f1;_tmpEA=*_tmpE9;
_tmpEB=_tmpEA.f1;_tmpEC=_tmpEA.f2;_tmpED=_tmpAE.f2;if(_tmpED <= (void*)10)goto
_LLAE;if(*((int*)_tmpED)!= 4)goto _LLAE;_tmpEE=((struct Cyc_Port_UnknownAggr_ct_struct*)
_tmpED)->f1;_tmpEF=((struct Cyc_Port_UnknownAggr_ct_struct*)_tmpED)->f2;_tmpF0=(
void***)&((struct Cyc_Port_UnknownAggr_ct_struct*)_tmpED)->f2;_LLAD: Cyc_Port_merge_fields(
_tmpEC,_tmpEE,0);{void**_tmp464;*_tmpF0=((_tmp464=_cycalloc(sizeof(*_tmp464)),((
_tmp464[0]=t1,_tmp464))));}return;_LLAE:;_LLAF:(int)_throw((void*)Cyc_Port_Unify_ct);
_LL99:;}}static void Cyc_Port_unify_cts(struct Cyc_List_List*t1,struct Cyc_List_List*
t2);static void Cyc_Port_unify_cts(struct Cyc_List_List*t1,struct Cyc_List_List*t2){
for(0;t1 != 0  && t2 != 0;(t1=t1->tl,t2=t2->tl)){Cyc_Port_unify_ct((void*)t1->hd,(
void*)t2->hd);}if(t1 != 0  || t2 != 0)(int)_throw((void*)Cyc_Port_Unify_ct);}static
struct Cyc_List_List*Cyc_Port_merge_fields(struct Cyc_List_List*fs1,struct Cyc_List_List*
fs2,int allow_f1_subset_f2);static struct Cyc_List_List*Cyc_Port_merge_fields(
struct Cyc_List_List*fs1,struct Cyc_List_List*fs2,int allow_f1_subset_f2){struct Cyc_List_List*
common=0;{struct Cyc_List_List*_tmpF8=fs2;for(0;(unsigned int)_tmpF8;_tmpF8=
_tmpF8->tl){struct Cyc_Port_Cfield*_tmpF9=(struct Cyc_Port_Cfield*)_tmpF8->hd;int
found=0;{struct Cyc_List_List*_tmpFA=fs1;for(0;(unsigned int)_tmpFA;_tmpFA=_tmpFA->tl){
struct Cyc_Port_Cfield*_tmpFB=(struct Cyc_Port_Cfield*)_tmpFA->hd;if(Cyc_strptrcmp(
_tmpFB->f,_tmpF9->f)== 0){{struct Cyc_List_List*_tmp465;common=((_tmp465=
_cycalloc(sizeof(*_tmp465)),((_tmp465->hd=_tmpFB,((_tmp465->tl=common,_tmp465))))));}
Cyc_Port_unify_ct(_tmpFB->qual,_tmpF9->qual);Cyc_Port_unify_ct(_tmpFB->type,
_tmpF9->type);found=1;break;}}}if(!found){if(allow_f1_subset_f2){struct Cyc_List_List*
_tmp466;common=((_tmp466=_cycalloc(sizeof(*_tmp466)),((_tmp466->hd=_tmpF9,((
_tmp466->tl=common,_tmp466))))));}else{(int)_throw((void*)Cyc_Port_Unify_ct);}}}}{
struct Cyc_List_List*_tmpFE=fs1;for(0;(unsigned int)_tmpFE;_tmpFE=_tmpFE->tl){
struct Cyc_Port_Cfield*_tmpFF=(struct Cyc_Port_Cfield*)_tmpFE->hd;int found=0;{
struct Cyc_List_List*_tmp100=fs2;for(0;(unsigned int)_tmp100;_tmp100=_tmp100->tl){
struct Cyc_Port_Cfield*_tmp101=(struct Cyc_Port_Cfield*)_tmp100->hd;if(Cyc_strptrcmp(
_tmpFF->f,_tmp101->f))found=1;}}if(!found){struct Cyc_List_List*_tmp467;common=((
_tmp467=_cycalloc(sizeof(*_tmp467)),((_tmp467->hd=_tmpFF,((_tmp467->tl=common,
_tmp467))))));}}}return common;}static int Cyc_Port_unifies(void*t1,void*t2);static
int Cyc_Port_unifies(void*t1,void*t2){{struct _handler_cons _tmp103;_push_handler(&
_tmp103);{int _tmp105=0;if(setjmp(_tmp103.handler))_tmp105=1;if(!_tmp105){Cyc_Port_unify_ct(
t1,t2);;_pop_handler();}else{void*_tmp104=(void*)_exn_thrown;void*_tmp107=
_tmp104;_LLB1: if(_tmp107 != Cyc_Port_Unify_ct)goto _LLB3;_LLB2: return 0;_LLB3:;
_LLB4:(void)_throw(_tmp107);_LLB0:;}}}return 1;}struct _tuple11{void*f1;void*f2;
void*f3;void*f4;void*f5;};static struct Cyc_List_List*Cyc_Port_insert_upper(void*v,
void*t,struct Cyc_List_List**uppers);static struct Cyc_List_List*Cyc_Port_insert_upper(
void*v,void*t,struct Cyc_List_List**uppers){t=Cyc_Port_compress_ct(t);{void*
_tmp108=t;_LLB6: if((int)_tmp108 != 1)goto _LLB8;_LLB7: goto _LLB9;_LLB8: if((int)
_tmp108 != 8)goto _LLBA;_LLB9: goto _LLBB;_LLBA: if((int)_tmp108 != 5)goto _LLBC;_LLBB:
goto _LLBD;_LLBC: if((int)_tmp108 != 2)goto _LLBE;_LLBD: goto _LLBF;_LLBE: if((int)
_tmp108 != 3)goto _LLC0;_LLBF: goto _LLC1;_LLC0: if(_tmp108 <= (void*)10)goto _LLC6;if(*((
int*)_tmp108)!= 2)goto _LLC2;_LLC1: goto _LLC3;_LLC2: if(*((int*)_tmp108)!= 3)goto
_LLC4;_LLC3: goto _LLC5;_LLC4: if(*((int*)_tmp108)!= 5)goto _LLC6;_LLC5: goto _LLC7;
_LLC6: if((int)_tmp108 != 7)goto _LLC8;_LLC7:*uppers=0;Cyc_Port_unifies(v,t);return*
uppers;_LLC8: if((int)_tmp108 != 4)goto _LLCA;_LLC9: goto _LLCB;_LLCA: if((int)_tmp108
!= 0)goto _LLCC;_LLCB: goto _LLCD;_LLCC: if((int)_tmp108 != 9)goto _LLCE;_LLCD: return*
uppers;_LLCE:;_LLCF: goto _LLB5;_LLB5:;}{struct Cyc_List_List*_tmp109=*uppers;for(0;(
unsigned int)_tmp109;_tmp109=_tmp109->tl){void*_tmp10A=Cyc_Port_compress_ct((
void*)_tmp109->hd);if(t == _tmp10A)return*uppers;{struct _tuple9 _tmp468;struct
_tuple9 _tmp10C=(_tmp468.f1=t,((_tmp468.f2=_tmp10A,_tmp468)));void*_tmp10D;void*
_tmp10E;void*_tmp10F;void*_tmp110;void*_tmp111;void*_tmp112;void*_tmp113;void*
_tmp114;void*_tmp115;void*_tmp116;void*_tmp117;void*_tmp118;void*_tmp119;void*
_tmp11A;void*_tmp11B;void*_tmp11C;void*_tmp11D;void*_tmp11E;_LLD1: _tmp10D=
_tmp10C.f1;if((int)_tmp10D != 6)goto _LLD3;_tmp10E=_tmp10C.f2;if(_tmp10E <= (void*)
10)goto _LLD3;if(*((int*)_tmp10E)!= 1)goto _LLD3;_LLD2: goto _LLD4;_LLD3: _tmp10F=
_tmp10C.f1;if((int)_tmp10F != 6)goto _LLD5;_tmp110=_tmp10C.f2;if((int)_tmp110 != 5)
goto _LLD5;_LLD4: goto _LLD6;_LLD5: _tmp111=_tmp10C.f1;if((int)_tmp111 != 6)goto _LLD7;
_tmp112=_tmp10C.f2;if(_tmp112 <= (void*)10)goto _LLD7;if(*((int*)_tmp112)!= 2)goto
_LLD7;_LLD6: return*uppers;_LLD7: _tmp113=_tmp10C.f1;if(_tmp113 <= (void*)10)goto
_LLD9;if(*((int*)_tmp113)!= 1)goto _LLD9;_tmp114=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp113)->f1;_tmp115=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp113)->f2;_tmp116=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp113)->f3;_tmp117=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp113)->f4;_tmp118=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp113)->f5;_tmp119=
_tmp10C.f2;if(_tmp119 <= (void*)10)goto _LLD9;if(*((int*)_tmp119)!= 1)goto _LLD9;
_tmp11A=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp119)->f1;_tmp11B=(void*)((
struct Cyc_Port_Ptr_ct_struct*)_tmp119)->f2;_tmp11C=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp119)->f3;_tmp11D=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp119)->f4;_tmp11E=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp119)->f5;_LLD8: {void*_tmp121;void*
_tmp122;void*_tmp123;void*_tmp124;void*_tmp125;struct _tuple11 _tmp469;struct
_tuple11 _tmp120=(_tmp469.f1=Cyc_Port_var(),((_tmp469.f2=Cyc_Port_var(),((_tmp469.f3=
Cyc_Port_var(),((_tmp469.f4=Cyc_Port_var(),((_tmp469.f5=Cyc_Port_var(),_tmp469)))))))));
_tmp121=_tmp120.f1;_tmp122=_tmp120.f2;_tmp123=_tmp120.f3;_tmp124=_tmp120.f4;
_tmp125=_tmp120.f5;{struct Cyc_Port_Ptr_ct_struct _tmp46C;struct Cyc_Port_Ptr_ct_struct*
_tmp46B;struct Cyc_Port_Ptr_ct_struct*_tmp126=(_tmp46B=_cycalloc(sizeof(*_tmp46B)),((
_tmp46B[0]=((_tmp46C.tag=1,((_tmp46C.f1=(void*)_tmp121,((_tmp46C.f2=(void*)
_tmp122,((_tmp46C.f3=(void*)_tmp123,((_tmp46C.f4=(void*)_tmp124,((_tmp46C.f5=(
void*)_tmp125,_tmp46C)))))))))))),_tmp46B)));Cyc_Port_leq(_tmp121,_tmp114);Cyc_Port_leq(
_tmp121,_tmp11A);Cyc_Port_leq(_tmp122,_tmp115);Cyc_Port_leq(_tmp122,_tmp11B);Cyc_Port_leq(
_tmp123,_tmp116);Cyc_Port_leq(_tmp123,_tmp11B);Cyc_Port_leq(_tmp124,_tmp117);Cyc_Port_leq(
_tmp124,_tmp11D);Cyc_Port_leq(_tmp125,_tmp118);Cyc_Port_leq(_tmp125,_tmp11E);
_tmp109->hd=(void*)((void*)_tmp126);return*uppers;}}_LLD9:;_LLDA: goto _LLD0;_LLD0:;}}}{
struct Cyc_List_List*_tmp46D;return(_tmp46D=_cycalloc(sizeof(*_tmp46D)),((_tmp46D->hd=(
void*)t,((_tmp46D->tl=*uppers,_tmp46D)))));}}static struct Cyc_List_List*Cyc_Port_insert_lower(
void*v,void*t,struct Cyc_List_List**lowers);static struct Cyc_List_List*Cyc_Port_insert_lower(
void*v,void*t,struct Cyc_List_List**lowers){t=Cyc_Port_compress_ct(t);{void*
_tmp12A=t;_LLDC: if((int)_tmp12A != 0)goto _LLDE;_LLDD: goto _LLDF;_LLDE: if((int)
_tmp12A != 8)goto _LLE0;_LLDF: goto _LLE1;_LLE0: if((int)_tmp12A != 2)goto _LLE2;_LLE1:
goto _LLE3;_LLE2: if((int)_tmp12A != 3)goto _LLE4;_LLE3: goto _LLE5;_LLE4: if((int)
_tmp12A != 4)goto _LLE6;_LLE5: goto _LLE7;_LLE6: if(_tmp12A <= (void*)10)goto _LLEC;if(*((
int*)_tmp12A)!= 3)goto _LLE8;_LLE7: goto _LLE9;_LLE8: if(*((int*)_tmp12A)!= 5)goto
_LLEA;_LLE9: goto _LLEB;_LLEA: if(*((int*)_tmp12A)!= 0)goto _LLEC;_LLEB:*lowers=0;
Cyc_Port_unifies(v,t);return*lowers;_LLEC: if((int)_tmp12A != 7)goto _LLEE;_LLED:
goto _LLEF;_LLEE: if((int)_tmp12A != 1)goto _LLF0;_LLEF: goto _LLF1;_LLF0: if((int)
_tmp12A != 9)goto _LLF2;_LLF1: return*lowers;_LLF2:;_LLF3: goto _LLDB;_LLDB:;}{struct
Cyc_List_List*_tmp12B=*lowers;for(0;(unsigned int)_tmp12B;_tmp12B=_tmp12B->tl){
void*_tmp12C=Cyc_Port_compress_ct((void*)_tmp12B->hd);if(t == _tmp12C)return*
lowers;{struct _tuple9 _tmp46E;struct _tuple9 _tmp12E=(_tmp46E.f1=t,((_tmp46E.f2=
_tmp12C,_tmp46E)));void*_tmp12F;void*_tmp130;void*_tmp131;void*_tmp132;void*
_tmp133;void*_tmp134;void*_tmp135;void*_tmp136;void*_tmp137;void*_tmp138;void*
_tmp139;void*_tmp13A;void*_tmp13B;void*_tmp13C;void*_tmp13D;void*_tmp13E;void*
_tmp13F;void*_tmp140;void*_tmp141;void*_tmp142;void*_tmp143;_LLF5: _tmp12F=
_tmp12E.f2;if((int)_tmp12F != 4)goto _LLF7;_LLF6: goto _LLF8;_LLF7: _tmp130=_tmp12E.f1;
if((int)_tmp130 != 5)goto _LLF9;_tmp131=_tmp12E.f2;if((int)_tmp131 != 6)goto _LLF9;
_LLF8: goto _LLFA;_LLF9: _tmp132=_tmp12E.f1;if((int)_tmp132 != 5)goto _LLFB;_tmp133=
_tmp12E.f2;if(_tmp133 <= (void*)10)goto _LLFB;if(*((int*)_tmp133)!= 1)goto _LLFB;
_LLFA: goto _LLFC;_LLFB: _tmp134=_tmp12E.f1;if(_tmp134 <= (void*)10)goto _LLFD;if(*((
int*)_tmp134)!= 1)goto _LLFD;_tmp135=_tmp12E.f2;if((int)_tmp135 != 6)goto _LLFD;
_LLFC: goto _LLFE;_LLFD: _tmp136=_tmp12E.f1;if(_tmp136 <= (void*)10)goto _LLFF;if(*((
int*)_tmp136)!= 2)goto _LLFF;_tmp137=_tmp12E.f2;if((int)_tmp137 != 6)goto _LLFF;
_LLFE: return*lowers;_LLFF: _tmp138=_tmp12E.f1;if(_tmp138 <= (void*)10)goto _LL101;
if(*((int*)_tmp138)!= 1)goto _LL101;_tmp139=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp138)->f1;_tmp13A=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp138)->f2;_tmp13B=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp138)->f3;_tmp13C=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp138)->f4;_tmp13D=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp138)->f5;_tmp13E=
_tmp12E.f2;if(_tmp13E <= (void*)10)goto _LL101;if(*((int*)_tmp13E)!= 1)goto _LL101;
_tmp13F=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp13E)->f1;_tmp140=(void*)((
struct Cyc_Port_Ptr_ct_struct*)_tmp13E)->f2;_tmp141=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp13E)->f3;_tmp142=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp13E)->f4;_tmp143=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp13E)->f5;_LL100: {void*_tmp146;void*
_tmp147;void*_tmp148;void*_tmp149;void*_tmp14A;struct _tuple11 _tmp46F;struct
_tuple11 _tmp145=(_tmp46F.f1=Cyc_Port_var(),((_tmp46F.f2=Cyc_Port_var(),((_tmp46F.f3=
Cyc_Port_var(),((_tmp46F.f4=Cyc_Port_var(),((_tmp46F.f5=Cyc_Port_var(),_tmp46F)))))))));
_tmp146=_tmp145.f1;_tmp147=_tmp145.f2;_tmp148=_tmp145.f3;_tmp149=_tmp145.f4;
_tmp14A=_tmp145.f5;{struct Cyc_Port_Ptr_ct_struct _tmp472;struct Cyc_Port_Ptr_ct_struct*
_tmp471;struct Cyc_Port_Ptr_ct_struct*_tmp14B=(_tmp471=_cycalloc(sizeof(*_tmp471)),((
_tmp471[0]=((_tmp472.tag=1,((_tmp472.f1=(void*)_tmp146,((_tmp472.f2=(void*)
_tmp147,((_tmp472.f3=(void*)_tmp148,((_tmp472.f4=(void*)_tmp149,((_tmp472.f5=(
void*)_tmp14A,_tmp472)))))))))))),_tmp471)));Cyc_Port_leq(_tmp139,_tmp146);Cyc_Port_leq(
_tmp13F,_tmp146);Cyc_Port_leq(_tmp13A,_tmp147);Cyc_Port_leq(_tmp140,_tmp147);Cyc_Port_leq(
_tmp13B,_tmp148);Cyc_Port_leq(_tmp140,_tmp148);Cyc_Port_leq(_tmp13C,_tmp149);Cyc_Port_leq(
_tmp142,_tmp149);Cyc_Port_leq(_tmp13D,_tmp14A);Cyc_Port_leq(_tmp143,_tmp14A);
_tmp12B->hd=(void*)((void*)_tmp14B);return*lowers;}}_LL101:;_LL102: goto _LLF4;
_LLF4:;}}}{struct Cyc_List_List*_tmp473;return(_tmp473=_cycalloc(sizeof(*_tmp473)),((
_tmp473->hd=(void*)t,((_tmp473->tl=*lowers,_tmp473)))));}}static int Cyc_Port_leq(
void*t1,void*t2);static int Cyc_Port_leq(void*t1,void*t2){if(t1 == t2)return 1;t1=
Cyc_Port_compress_ct(t1);t2=Cyc_Port_compress_ct(t2);{struct _tuple9 _tmp474;
struct _tuple9 _tmp150=(_tmp474.f1=t1,((_tmp474.f2=t2,_tmp474)));void*_tmp151;void*
_tmp152;struct _dyneither_ptr*_tmp153;struct _dyneither_ptr _tmp154;void*_tmp155;
struct _dyneither_ptr*_tmp156;struct _dyneither_ptr _tmp157;void*_tmp158;struct
_dyneither_ptr*_tmp159;struct _dyneither_ptr _tmp15A;void*_tmp15B;void*_tmp15C;
void*_tmp15D;void*_tmp15E;void*_tmp15F;void*_tmp160;void*_tmp161;void*_tmp162;
void*_tmp163;void*_tmp164;void*_tmp165;void*_tmp166;void*_tmp167;void*_tmp168;
void*_tmp169;void*_tmp16A;void*_tmp16B;void*_tmp16C;void*_tmp16D;void*_tmp16E;
void*_tmp16F;void*_tmp170;void*_tmp171;void*_tmp172;void*_tmp173;void*_tmp174;
void*_tmp175;void*_tmp176;void*_tmp177;void*_tmp178;void*_tmp179;void*_tmp17A;
void*_tmp17B;void*_tmp17C;void*_tmp17D;void*_tmp17E;void*_tmp17F;void*_tmp180;
void*_tmp181;void*_tmp182;void*_tmp183;void*_tmp184;void*_tmp185;void*_tmp186;
void*_tmp187;void*_tmp188;void*_tmp189;void*_tmp18A;void*_tmp18B;void*_tmp18C;
void*_tmp18D;void*_tmp18E;void*_tmp18F;void*_tmp190;void*_tmp191;void*_tmp192;
void*_tmp193;void*_tmp194;struct Cyc_Port_Cvar*_tmp195;void*_tmp196;struct Cyc_Port_Cvar*
_tmp197;void*_tmp198;struct Cyc_Port_Cvar*_tmp199;void*_tmp19A;struct Cyc_Port_Cvar*
_tmp19B;_LL104: _tmp151=_tmp150.f1;if((int)_tmp151 != 7)goto _LL106;_LL105: return 1;
_LL106: _tmp152=_tmp150.f1;if(_tmp152 <= (void*)10)goto _LL108;if(*((int*)_tmp152)
!= 0)goto _LL108;_tmp153=((struct Cyc_Port_RgnVar_ct_struct*)_tmp152)->f1;_tmp154=*
_tmp153;_tmp155=_tmp150.f2;if(_tmp155 <= (void*)10)goto _LL108;if(*((int*)_tmp155)
!= 0)goto _LL108;_tmp156=((struct Cyc_Port_RgnVar_ct_struct*)_tmp155)->f1;_tmp157=*
_tmp156;_LL107: return Cyc_strcmp((struct _dyneither_ptr)_tmp154,(struct
_dyneither_ptr)_tmp157)== 0;_LL108: _tmp158=_tmp150.f1;if(_tmp158 <= (void*)10)
goto _LL10A;if(*((int*)_tmp158)!= 0)goto _LL10A;_tmp159=((struct Cyc_Port_RgnVar_ct_struct*)
_tmp158)->f1;_tmp15A=*_tmp159;_tmp15B=_tmp150.f2;if((int)_tmp15B != 7)goto _LL10A;
_LL109: return 0;_LL10A: _tmp15C=_tmp150.f1;if((int)_tmp15C != 1)goto _LL10C;_tmp15D=
_tmp150.f2;if((int)_tmp15D != 0)goto _LL10C;_LL10B: return 1;_LL10C: _tmp15E=_tmp150.f1;
if((int)_tmp15E != 0)goto _LL10E;_tmp15F=_tmp150.f2;if((int)_tmp15F != 1)goto _LL10E;
_LL10D: return 0;_LL10E: _tmp160=_tmp150.f1;if((int)_tmp160 != 9)goto _LL110;_tmp161=
_tmp150.f2;if((int)_tmp161 != 8)goto _LL110;_LL10F: return 0;_LL110: _tmp162=_tmp150.f1;
if((int)_tmp162 != 8)goto _LL112;_tmp163=_tmp150.f2;if((int)_tmp163 != 9)goto _LL112;
_LL111: return 1;_LL112: _tmp164=_tmp150.f1;if(_tmp164 <= (void*)10)goto _LL114;if(*((
int*)_tmp164)!= 6)goto _LL114;_tmp165=_tmp150.f2;if((int)_tmp165 != 0)goto _LL114;
_LL113: return 1;_LL114: _tmp166=_tmp150.f1;if(_tmp166 <= (void*)10)goto _LL116;if(*((
int*)_tmp166)!= 6)goto _LL116;_tmp167=_tmp150.f2;if((int)_tmp167 != 4)goto _LL116;
_LL115: return 1;_LL116: _tmp168=_tmp150.f1;if((int)_tmp168 != 4)goto _LL118;_LL117:
return 0;_LL118: _tmp169=_tmp150.f1;if((int)_tmp169 != 5)goto _LL11A;_tmp16A=_tmp150.f2;
if((int)_tmp16A != 6)goto _LL11A;_LL119: return 1;_LL11A: _tmp16B=_tmp150.f1;if((int)
_tmp16B != 5)goto _LL11C;_tmp16C=_tmp150.f2;if(_tmp16C <= (void*)10)goto _LL11C;if(*((
int*)_tmp16C)!= 1)goto _LL11C;_LL11B: return 1;_LL11C: _tmp16D=_tmp150.f1;if((int)
_tmp16D != 5)goto _LL11E;_tmp16E=_tmp150.f2;if((int)_tmp16E != 4)goto _LL11E;_LL11D:
return 1;_LL11E: _tmp16F=_tmp150.f1;if(_tmp16F <= (void*)10)goto _LL120;if(*((int*)
_tmp16F)!= 1)goto _LL120;_tmp170=_tmp150.f2;if((int)_tmp170 != 6)goto _LL120;_LL11F:
return 1;_LL120: _tmp171=_tmp150.f1;if(_tmp171 <= (void*)10)goto _LL122;if(*((int*)
_tmp171)!= 1)goto _LL122;_tmp172=_tmp150.f2;if((int)_tmp172 != 4)goto _LL122;_LL121:
return 1;_LL122: _tmp173=_tmp150.f1;if(_tmp173 <= (void*)10)goto _LL124;if(*((int*)
_tmp173)!= 2)goto _LL124;_tmp174=_tmp150.f2;if((int)_tmp174 != 6)goto _LL124;_LL123:
return 1;_LL124: _tmp175=_tmp150.f1;if(_tmp175 <= (void*)10)goto _LL126;if(*((int*)
_tmp175)!= 2)goto _LL126;_tmp176=_tmp150.f2;if((int)_tmp176 != 4)goto _LL126;_LL125:
return 1;_LL126: _tmp177=_tmp150.f1;if(_tmp177 <= (void*)10)goto _LL128;if(*((int*)
_tmp177)!= 1)goto _LL128;_tmp178=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp177)->f1;
_tmp179=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp177)->f2;_tmp17A=(void*)((
struct Cyc_Port_Ptr_ct_struct*)_tmp177)->f3;_tmp17B=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp177)->f4;_tmp17C=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp177)->f5;_tmp17D=
_tmp150.f2;if(_tmp17D <= (void*)10)goto _LL128;if(*((int*)_tmp17D)!= 1)goto _LL128;
_tmp17E=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp17D)->f1;_tmp17F=(void*)((
struct Cyc_Port_Ptr_ct_struct*)_tmp17D)->f2;_tmp180=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp17D)->f3;_tmp181=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp17D)->f4;_tmp182=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp17D)->f5;_LL127: return(((Cyc_Port_leq(
_tmp178,_tmp17E) && Cyc_Port_leq(_tmp179,_tmp17F)) && Cyc_Port_unifies(_tmp17A,
_tmp180)) && Cyc_Port_leq(_tmp17B,_tmp181)) && Cyc_Port_leq(_tmp17C,_tmp182);
_LL128: _tmp183=_tmp150.f1;if(_tmp183 <= (void*)10)goto _LL12A;if(*((int*)_tmp183)
!= 2)goto _LL12A;_tmp184=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp183)->f1;
_tmp185=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp183)->f2;_tmp186=(void*)((
struct Cyc_Port_Array_ct_struct*)_tmp183)->f3;_tmp187=_tmp150.f2;if(_tmp187 <= (
void*)10)goto _LL12A;if(*((int*)_tmp187)!= 2)goto _LL12A;_tmp188=(void*)((struct
Cyc_Port_Array_ct_struct*)_tmp187)->f1;_tmp189=(void*)((struct Cyc_Port_Array_ct_struct*)
_tmp187)->f2;_tmp18A=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp187)->f3;
_LL129: return(Cyc_Port_leq(_tmp184,_tmp188) && Cyc_Port_leq(_tmp185,_tmp189))
 && Cyc_Port_leq(_tmp186,_tmp18A);_LL12A: _tmp18B=_tmp150.f1;if(_tmp18B <= (void*)
10)goto _LL12C;if(*((int*)_tmp18B)!= 2)goto _LL12C;_tmp18C=(void*)((struct Cyc_Port_Array_ct_struct*)
_tmp18B)->f1;_tmp18D=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp18B)->f2;
_tmp18E=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp18B)->f3;_tmp18F=_tmp150.f2;
if(_tmp18F <= (void*)10)goto _LL12C;if(*((int*)_tmp18F)!= 1)goto _LL12C;_tmp190=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp18F)->f1;_tmp191=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp18F)->f2;_tmp192=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp18F)->f3;_tmp193=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp18F)->f5;_LL12B: return((Cyc_Port_leq(
_tmp18C,_tmp190) && Cyc_Port_leq(_tmp18D,_tmp191)) && Cyc_Port_unifies((void*)3,
_tmp192)) && Cyc_Port_leq(_tmp18E,_tmp193);_LL12C: _tmp194=_tmp150.f1;if(_tmp194
<= (void*)10)goto _LL12E;if(*((int*)_tmp194)!= 6)goto _LL12E;_tmp195=((struct Cyc_Port_Var_ct_struct*)
_tmp194)->f1;_tmp196=_tmp150.f2;if(_tmp196 <= (void*)10)goto _LL12E;if(*((int*)
_tmp196)!= 6)goto _LL12E;_tmp197=((struct Cyc_Port_Var_ct_struct*)_tmp196)->f1;
_LL12D: _tmp195->uppers=Cyc_Port_filter_self(t1,_tmp195->uppers);_tmp197->lowers=
Cyc_Port_filter_self(t2,_tmp197->lowers);_tmp195->uppers=Cyc_Port_insert_upper(
t1,t2,& _tmp195->uppers);_tmp197->lowers=Cyc_Port_insert_lower(t2,t1,& _tmp197->lowers);
return 1;_LL12E: _tmp198=_tmp150.f1;if(_tmp198 <= (void*)10)goto _LL130;if(*((int*)
_tmp198)!= 6)goto _LL130;_tmp199=((struct Cyc_Port_Var_ct_struct*)_tmp198)->f1;
_LL12F: _tmp199->uppers=Cyc_Port_filter_self(t1,_tmp199->uppers);_tmp199->uppers=
Cyc_Port_insert_upper(t1,t2,& _tmp199->uppers);return 1;_LL130: _tmp19A=_tmp150.f2;
if(_tmp19A <= (void*)10)goto _LL132;if(*((int*)_tmp19A)!= 6)goto _LL132;_tmp19B=((
struct Cyc_Port_Var_ct_struct*)_tmp19A)->f1;_LL131: _tmp19B->lowers=Cyc_Port_filter_self(
t2,_tmp19B->lowers);_tmp19B->lowers=Cyc_Port_insert_lower(t2,t1,& _tmp19B->lowers);
return 1;_LL132:;_LL133: return Cyc_Port_unifies(t1,t2);_LL103:;}}struct Cyc_Port_GlobalCenv{
struct Cyc_Dict_Dict typedef_dict;struct Cyc_Dict_Dict struct_dict;struct Cyc_Dict_Dict
union_dict;void*return_type;struct Cyc_List_List*qualifier_edits;struct Cyc_List_List*
pointer_edits;struct Cyc_List_List*zeroterm_edits;struct Cyc_List_List*edits;int
porting;};struct Cyc_Port_Cenv{struct Cyc_Port_GlobalCenv*gcenv;struct Cyc_Dict_Dict
var_dict;void*cpos;};static struct Cyc_Port_Cenv*Cyc_Port_empty_cenv();static
struct Cyc_Port_Cenv*Cyc_Port_empty_cenv(){struct Cyc_Port_GlobalCenv*_tmp475;
struct Cyc_Port_GlobalCenv*g=(_tmp475=_cycalloc(sizeof(*_tmp475)),((_tmp475->typedef_dict=((
struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(
Cyc_Absyn_qvar_cmp),((_tmp475->struct_dict=((struct Cyc_Dict_Dict(*)(int(*cmp)(
struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),((_tmp475->union_dict=((
struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(
Cyc_Absyn_qvar_cmp),((_tmp475->qualifier_edits=0,((_tmp475->pointer_edits=0,((
_tmp475->zeroterm_edits=0,((_tmp475->edits=0,((_tmp475->porting=0,((_tmp475->return_type=
Cyc_Port_void_ct(),_tmp475)))))))))))))))))));struct Cyc_Port_Cenv*_tmp476;return(
_tmp476=_cycalloc(sizeof(*_tmp476)),((_tmp476->gcenv=g,((_tmp476->cpos=(void*)3,((
_tmp476->var_dict=((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct
_tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),_tmp476)))))));}static int Cyc_Port_in_fn_arg(
struct Cyc_Port_Cenv*env);static int Cyc_Port_in_fn_arg(struct Cyc_Port_Cenv*env){
return env->cpos == (void*)1;}static int Cyc_Port_in_fn_res(struct Cyc_Port_Cenv*env);
static int Cyc_Port_in_fn_res(struct Cyc_Port_Cenv*env){return env->cpos == (void*)0;}
static int Cyc_Port_in_toplevel(struct Cyc_Port_Cenv*env);static int Cyc_Port_in_toplevel(
struct Cyc_Port_Cenv*env){return env->cpos == (void*)3;}static void*Cyc_Port_lookup_return_type(
struct Cyc_Port_Cenv*env);static void*Cyc_Port_lookup_return_type(struct Cyc_Port_Cenv*
env){return(env->gcenv)->return_type;}static void*Cyc_Port_lookup_typedef(struct
Cyc_Port_Cenv*env,struct _tuple0*n);static void*Cyc_Port_lookup_typedef(struct Cyc_Port_Cenv*
env,struct _tuple0*n){struct _handler_cons _tmp19E;_push_handler(& _tmp19E);{int
_tmp1A0=0;if(setjmp(_tmp19E.handler))_tmp1A0=1;if(!_tmp1A0){{void*_tmp1A2;struct
_tuple9 _tmp1A1=*((struct _tuple9*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)((
env->gcenv)->typedef_dict,n);_tmp1A2=_tmp1A1.f1;{void*_tmp1A3=_tmp1A2;
_npop_handler(0);return _tmp1A3;}};_pop_handler();}else{void*_tmp19F=(void*)
_exn_thrown;void*_tmp1A5=_tmp19F;_LL135: if(_tmp1A5 != Cyc_Dict_Absent)goto _LL137;
_LL136: return Cyc_Absyn_sint_typ;_LL137:;_LL138:(void)_throw(_tmp1A5);_LL134:;}}}
static void*Cyc_Port_lookup_typedef_ctype(struct Cyc_Port_Cenv*env,struct _tuple0*n);
static void*Cyc_Port_lookup_typedef_ctype(struct Cyc_Port_Cenv*env,struct _tuple0*n){
struct _handler_cons _tmp1A6;_push_handler(& _tmp1A6);{int _tmp1A8=0;if(setjmp(
_tmp1A6.handler))_tmp1A8=1;if(!_tmp1A8){{void*_tmp1AA;struct _tuple9 _tmp1A9=*((
struct _tuple9*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)((env->gcenv)->typedef_dict,
n);_tmp1AA=_tmp1A9.f2;{void*_tmp1AB=_tmp1AA;_npop_handler(0);return _tmp1AB;}};
_pop_handler();}else{void*_tmp1A7=(void*)_exn_thrown;void*_tmp1AD=_tmp1A7;_LL13A:
if(_tmp1AD != Cyc_Dict_Absent)goto _LL13C;_LL13B: return Cyc_Port_var();_LL13C:;
_LL13D:(void)_throw(_tmp1AD);_LL139:;}}}static struct _tuple10*Cyc_Port_lookup_struct_decl(
struct Cyc_Port_Cenv*env,struct _tuple0*n);static struct _tuple10*Cyc_Port_lookup_struct_decl(
struct Cyc_Port_Cenv*env,struct _tuple0*n){struct _tuple10**_tmp1AE=((struct
_tuple10**(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup_opt)((env->gcenv)->struct_dict,
n);if((unsigned int)_tmp1AE)return*_tmp1AE;else{struct Cyc_Absyn_Aggrdecl*_tmp477;
struct Cyc_Absyn_Aggrdecl*_tmp1AF=(_tmp477=_cycalloc(sizeof(*_tmp477)),((_tmp477->kind=(
void*)0,((_tmp477->sc=(void*)2,((_tmp477->name=n,((_tmp477->tvs=0,((_tmp477->impl=
0,((_tmp477->attributes=0,_tmp477)))))))))))));struct _tuple10*_tmp478;struct
_tuple10*p=(_tmp478=_cycalloc(sizeof(*_tmp478)),((_tmp478->f1=_tmp1AF,((_tmp478->f2=
0,_tmp478)))));(env->gcenv)->struct_dict=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict
d,struct _tuple0*k,struct _tuple10*v))Cyc_Dict_insert)((env->gcenv)->struct_dict,n,
p);return p;}}static struct _tuple10*Cyc_Port_lookup_union_decl(struct Cyc_Port_Cenv*
env,struct _tuple0*n);static struct _tuple10*Cyc_Port_lookup_union_decl(struct Cyc_Port_Cenv*
env,struct _tuple0*n){struct _tuple10**_tmp1B2=((struct _tuple10**(*)(struct Cyc_Dict_Dict
d,struct _tuple0*k))Cyc_Dict_lookup_opt)((env->gcenv)->union_dict,n);if((
unsigned int)_tmp1B2)return*_tmp1B2;else{struct Cyc_Absyn_Aggrdecl*_tmp479;struct
Cyc_Absyn_Aggrdecl*_tmp1B3=(_tmp479=_cycalloc(sizeof(*_tmp479)),((_tmp479->kind=(
void*)1,((_tmp479->sc=(void*)2,((_tmp479->name=n,((_tmp479->tvs=0,((_tmp479->impl=
0,((_tmp479->attributes=0,_tmp479)))))))))))));struct _tuple10*_tmp47A;struct
_tuple10*p=(_tmp47A=_cycalloc(sizeof(*_tmp47A)),((_tmp47A->f1=_tmp1B3,((_tmp47A->f2=
0,_tmp47A)))));(env->gcenv)->union_dict=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict
d,struct _tuple0*k,struct _tuple10*v))Cyc_Dict_insert)((env->gcenv)->union_dict,n,
p);return p;}}struct _tuple12{void*f1;struct _tuple9*f2;};static struct _tuple9 Cyc_Port_lookup_var(
struct Cyc_Port_Cenv*env,struct _tuple0*x);static struct _tuple9 Cyc_Port_lookup_var(
struct Cyc_Port_Cenv*env,struct _tuple0*x){struct _handler_cons _tmp1B6;
_push_handler(& _tmp1B6);{int _tmp1B8=0;if(setjmp(_tmp1B6.handler))_tmp1B8=1;if(!
_tmp1B8){{struct _tuple9*_tmp1BA;struct _tuple12 _tmp1B9=*((struct _tuple12*(*)(
struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);_tmp1BA=
_tmp1B9.f2;{struct _tuple9 _tmp1BB=*_tmp1BA;_npop_handler(0);return _tmp1BB;}};
_pop_handler();}else{void*_tmp1B7=(void*)_exn_thrown;void*_tmp1BD=_tmp1B7;_LL13F:
if(_tmp1BD != Cyc_Dict_Absent)goto _LL141;_LL140: {struct _tuple9 _tmp47B;return(
_tmp47B.f1=Cyc_Port_var(),((_tmp47B.f2=Cyc_Port_var(),_tmp47B)));}_LL141:;_LL142:(
void)_throw(_tmp1BD);_LL13E:;}}}static struct _tuple12*Cyc_Port_lookup_full_var(
struct Cyc_Port_Cenv*env,struct _tuple0*x);static struct _tuple12*Cyc_Port_lookup_full_var(
struct Cyc_Port_Cenv*env,struct _tuple0*x){return((struct _tuple12*(*)(struct Cyc_Dict_Dict
d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);}static int Cyc_Port_declared_var(
struct Cyc_Port_Cenv*env,struct _tuple0*x);static int Cyc_Port_declared_var(struct
Cyc_Port_Cenv*env,struct _tuple0*x){return((int(*)(struct Cyc_Dict_Dict d,struct
_tuple0*k))Cyc_Dict_member)(env->var_dict,x);}static int Cyc_Port_isfn(struct Cyc_Port_Cenv*
env,struct _tuple0*x);static int Cyc_Port_isfn(struct Cyc_Port_Cenv*env,struct
_tuple0*x){struct _handler_cons _tmp1BF;_push_handler(& _tmp1BF);{int _tmp1C1=0;if(
setjmp(_tmp1BF.handler))_tmp1C1=1;if(!_tmp1C1){{void*_tmp1C3;struct _tuple12
_tmp1C2=*((struct _tuple12*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(
env->var_dict,x);_tmp1C3=_tmp1C2.f1;LOOP: {void*_tmp1C4=_tmp1C3;struct _tuple0*
_tmp1C5;_LL144: if(_tmp1C4 <= (void*)4)goto _LL148;if(*((int*)_tmp1C4)!= 16)goto
_LL146;_tmp1C5=((struct Cyc_Absyn_TypedefType_struct*)_tmp1C4)->f1;_LL145: _tmp1C3=
Cyc_Port_lookup_typedef(env,_tmp1C5);goto LOOP;_LL146: if(*((int*)_tmp1C4)!= 8)
goto _LL148;_LL147: {int _tmp1C6=1;_npop_handler(0);return _tmp1C6;}_LL148:;_LL149: {
int _tmp1C7=0;_npop_handler(0);return _tmp1C7;}_LL143:;}};_pop_handler();}else{
void*_tmp1C0=(void*)_exn_thrown;void*_tmp1C9=_tmp1C0;_LL14B: if(_tmp1C9 != Cyc_Dict_Absent)
goto _LL14D;_LL14C: return 0;_LL14D:;_LL14E:(void)_throw(_tmp1C9);_LL14A:;}}}static
int Cyc_Port_isarray(struct Cyc_Port_Cenv*env,struct _tuple0*x);static int Cyc_Port_isarray(
struct Cyc_Port_Cenv*env,struct _tuple0*x){struct _handler_cons _tmp1CA;
_push_handler(& _tmp1CA);{int _tmp1CC=0;if(setjmp(_tmp1CA.handler))_tmp1CC=1;if(!
_tmp1CC){{void*_tmp1CE;struct _tuple12 _tmp1CD=*((struct _tuple12*(*)(struct Cyc_Dict_Dict
d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);_tmp1CE=_tmp1CD.f1;LOOP: {
void*_tmp1CF=_tmp1CE;struct _tuple0*_tmp1D0;_LL150: if(_tmp1CF <= (void*)4)goto
_LL154;if(*((int*)_tmp1CF)!= 16)goto _LL152;_tmp1D0=((struct Cyc_Absyn_TypedefType_struct*)
_tmp1CF)->f1;_LL151: _tmp1CE=Cyc_Port_lookup_typedef(env,_tmp1D0);goto LOOP;_LL152:
if(*((int*)_tmp1CF)!= 7)goto _LL154;_LL153: {int _tmp1D1=1;_npop_handler(0);return
_tmp1D1;}_LL154:;_LL155: {int _tmp1D2=0;_npop_handler(0);return _tmp1D2;}_LL14F:;}};
_pop_handler();}else{void*_tmp1CB=(void*)_exn_thrown;void*_tmp1D4=_tmp1CB;_LL157:
if(_tmp1D4 != Cyc_Dict_Absent)goto _LL159;_LL158: return 0;_LL159:;_LL15A:(void)
_throw(_tmp1D4);_LL156:;}}}static struct Cyc_Port_Cenv*Cyc_Port_set_cpos(struct Cyc_Port_Cenv*
env,void*cpos);static struct Cyc_Port_Cenv*Cyc_Port_set_cpos(struct Cyc_Port_Cenv*
env,void*cpos){struct Cyc_Port_Cenv*_tmp47C;return(_tmp47C=_cycalloc(sizeof(*
_tmp47C)),((_tmp47C->gcenv=env->gcenv,((_tmp47C->var_dict=env->var_dict,((
_tmp47C->cpos=cpos,_tmp47C)))))));}static void Cyc_Port_add_return_type(struct Cyc_Port_Cenv*
env,void*t);static void Cyc_Port_add_return_type(struct Cyc_Port_Cenv*env,void*t){(
env->gcenv)->return_type=t;}static struct Cyc_Port_Cenv*Cyc_Port_add_var(struct Cyc_Port_Cenv*
env,struct _tuple0*x,void*t,void*qual,void*ctype);static struct Cyc_Port_Cenv*Cyc_Port_add_var(
struct Cyc_Port_Cenv*env,struct _tuple0*x,void*t,void*qual,void*ctype){struct
_tuple12*_tmp482;struct _tuple9*_tmp481;struct Cyc_Port_Cenv*_tmp480;return(
_tmp480=_cycalloc(sizeof(*_tmp480)),((_tmp480->gcenv=env->gcenv,((_tmp480->var_dict=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple12*v))
Cyc_Dict_insert)(env->var_dict,x,((_tmp482=_cycalloc(sizeof(*_tmp482)),((_tmp482->f1=
t,((_tmp482->f2=((_tmp481=_cycalloc(sizeof(*_tmp481)),((_tmp481->f1=qual,((
_tmp481->f2=ctype,_tmp481)))))),_tmp482))))))),((_tmp480->cpos=env->cpos,_tmp480)))))));}
static void Cyc_Port_add_typedef(struct Cyc_Port_Cenv*env,struct _tuple0*n,void*t,
void*ct);static void Cyc_Port_add_typedef(struct Cyc_Port_Cenv*env,struct _tuple0*n,
void*t,void*ct){struct _tuple9*_tmp483;(env->gcenv)->typedef_dict=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple9*v))Cyc_Dict_insert)((env->gcenv)->typedef_dict,
n,((_tmp483=_cycalloc(sizeof(*_tmp483)),((_tmp483->f1=t,((_tmp483->f2=ct,_tmp483)))))));}
struct _tuple13{void*f1;void*f2;struct Cyc_Position_Segment*f3;};static void Cyc_Port_register_const_cvar(
struct Cyc_Port_Cenv*env,void*new_qual,void*orig_qual,struct Cyc_Position_Segment*
loc);static void Cyc_Port_register_const_cvar(struct Cyc_Port_Cenv*env,void*
new_qual,void*orig_qual,struct Cyc_Position_Segment*loc){struct _tuple13*_tmp486;
struct Cyc_List_List*_tmp485;(env->gcenv)->qualifier_edits=((_tmp485=_cycalloc(
sizeof(*_tmp485)),((_tmp485->hd=((_tmp486=_cycalloc(sizeof(*_tmp486)),((_tmp486->f1=
new_qual,((_tmp486->f2=orig_qual,((_tmp486->f3=loc,_tmp486)))))))),((_tmp485->tl=(
env->gcenv)->qualifier_edits,_tmp485))))));}static void Cyc_Port_register_ptr_cvars(
struct Cyc_Port_Cenv*env,void*new_ptr,void*orig_ptr,struct Cyc_Position_Segment*
loc);static void Cyc_Port_register_ptr_cvars(struct Cyc_Port_Cenv*env,void*new_ptr,
void*orig_ptr,struct Cyc_Position_Segment*loc){struct _tuple13*_tmp489;struct Cyc_List_List*
_tmp488;(env->gcenv)->pointer_edits=((_tmp488=_cycalloc(sizeof(*_tmp488)),((
_tmp488->hd=((_tmp489=_cycalloc(sizeof(*_tmp489)),((_tmp489->f1=new_ptr,((
_tmp489->f2=orig_ptr,((_tmp489->f3=loc,_tmp489)))))))),((_tmp488->tl=(env->gcenv)->pointer_edits,
_tmp488))))));}static void Cyc_Port_register_zeroterm_cvars(struct Cyc_Port_Cenv*
env,void*new_zt,void*orig_zt,struct Cyc_Position_Segment*loc);static void Cyc_Port_register_zeroterm_cvars(
struct Cyc_Port_Cenv*env,void*new_zt,void*orig_zt,struct Cyc_Position_Segment*loc){
struct _tuple13*_tmp48C;struct Cyc_List_List*_tmp48B;(env->gcenv)->zeroterm_edits=((
_tmp48B=_cycalloc(sizeof(*_tmp48B)),((_tmp48B->hd=((_tmp48C=_cycalloc(sizeof(*
_tmp48C)),((_tmp48C->f1=new_zt,((_tmp48C->f2=orig_zt,((_tmp48C->f3=loc,_tmp48C)))))))),((
_tmp48B->tl=(env->gcenv)->zeroterm_edits,_tmp48B))))));}static void*Cyc_Port_main_type();
static void*Cyc_Port_main_type(){struct _tuple7*_tmp48D;struct _tuple7*arg1=(
_tmp48D=_cycalloc(sizeof(*_tmp48D)),((_tmp48D->f1=0,((_tmp48D->f2=Cyc_Absyn_empty_tqual(
0),((_tmp48D->f3=Cyc_Absyn_sint_typ,_tmp48D)))))));struct _tuple7*_tmp48E;struct
_tuple7*arg2=(_tmp48E=_cycalloc(sizeof(*_tmp48E)),((_tmp48E->f1=0,((_tmp48E->f2=
Cyc_Absyn_empty_tqual(0),((_tmp48E->f3=Cyc_Absyn_dyneither_typ(Cyc_Absyn_string_typ(
Cyc_Absyn_wildtyp(0)),Cyc_Absyn_wildtyp(0),Cyc_Absyn_empty_tqual(0),((union Cyc_Absyn_Constraint*(*)())
Cyc_Absyn_empty_conref)()),_tmp48E)))))));struct Cyc_Absyn_FnType_struct _tmp498;
struct _tuple7*_tmp497[2];struct Cyc_Absyn_FnInfo _tmp496;struct Cyc_Absyn_FnType_struct*
_tmp495;return(void*)((_tmp495=_cycalloc(sizeof(*_tmp495)),((_tmp495[0]=((
_tmp498.tag=8,((_tmp498.f1=((_tmp496.tvars=0,((_tmp496.effect=0,((_tmp496.ret_typ=
Cyc_Absyn_sint_typ,((_tmp496.args=((_tmp497[1]=arg2,((_tmp497[0]=arg1,((struct
Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp497,
sizeof(struct _tuple7*),2)))))),((_tmp496.c_varargs=0,((_tmp496.cyc_varargs=0,((
_tmp496.rgn_po=0,((_tmp496.attributes=0,_tmp496)))))))))))))))),_tmp498)))),
_tmp495))));}static void*Cyc_Port_type_to_ctype(struct Cyc_Port_Cenv*env,void*t);
static struct Cyc_Port_Cenv*Cyc_Port_initial_cenv();static struct Cyc_Port_Cenv*Cyc_Port_initial_cenv(){
struct Cyc_Port_Cenv*_tmp1E6=Cyc_Port_empty_cenv();{struct _tuple0*_tmp499;_tmp1E6=
Cyc_Port_add_var(_tmp1E6,((_tmp499=_cycalloc(sizeof(*_tmp499)),((_tmp499->f1=Cyc_Absyn_Loc_n,((
_tmp499->f2=_init_dyneither_ptr(_cycalloc(sizeof(struct _dyneither_ptr)),"main",
sizeof(char),5),_tmp499)))))),Cyc_Port_main_type(),Cyc_Port_const_ct(),Cyc_Port_type_to_ctype(
_tmp1E6,Cyc_Port_main_type()));}return _tmp1E6;}static void Cyc_Port_region_counts(
struct Cyc_Dict_Dict*cts,void*t);static void Cyc_Port_region_counts(struct Cyc_Dict_Dict*
cts,void*t){void*_tmp1E9=Cyc_Port_compress_ct(t);struct _dyneither_ptr*_tmp1EA;
void*_tmp1EB;void*_tmp1EC;void*_tmp1ED;void*_tmp1EE;void*_tmp1EF;void*_tmp1F0;
void*_tmp1F1;void*_tmp1F2;void*_tmp1F3;struct Cyc_List_List*_tmp1F4;_LL15C: if((
int)_tmp1E9 != 0)goto _LL15E;_LL15D: goto _LL15F;_LL15E: if((int)_tmp1E9 != 1)goto
_LL160;_LL15F: goto _LL161;_LL160: if((int)_tmp1E9 != 2)goto _LL162;_LL161: goto _LL163;
_LL162: if((int)_tmp1E9 != 3)goto _LL164;_LL163: goto _LL165;_LL164: if((int)_tmp1E9 != 
4)goto _LL166;_LL165: goto _LL167;_LL166: if((int)_tmp1E9 != 5)goto _LL168;_LL167: goto
_LL169;_LL168: if((int)_tmp1E9 != 6)goto _LL16A;_LL169: goto _LL16B;_LL16A: if(_tmp1E9
<= (void*)10)goto _LL170;if(*((int*)_tmp1E9)!= 4)goto _LL16C;_LL16B: goto _LL16D;
_LL16C: if(*((int*)_tmp1E9)!= 3)goto _LL16E;_LL16D: goto _LL16F;_LL16E: if(*((int*)
_tmp1E9)!= 6)goto _LL170;_LL16F: goto _LL171;_LL170: if((int)_tmp1E9 != 8)goto _LL172;
_LL171: goto _LL173;_LL172: if((int)_tmp1E9 != 9)goto _LL174;_LL173: goto _LL175;_LL174:
if((int)_tmp1E9 != 7)goto _LL176;_LL175: return;_LL176: if(_tmp1E9 <= (void*)10)goto
_LL178;if(*((int*)_tmp1E9)!= 0)goto _LL178;_tmp1EA=((struct Cyc_Port_RgnVar_ct_struct*)
_tmp1E9)->f1;_LL177: if(!((int(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))
Cyc_Dict_member)(*cts,_tmp1EA)){int*_tmp49A;*cts=((struct Cyc_Dict_Dict(*)(struct
Cyc_Dict_Dict d,struct _dyneither_ptr*k,int*v))Cyc_Dict_insert)(*cts,_tmp1EA,(int*)((
_tmp49A=_cycalloc_atomic(sizeof(*_tmp49A)),((_tmp49A[0]=0,_tmp49A)))));}{int*
_tmp1F6=((int*(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup)(*
cts,_tmp1EA);*_tmp1F6=*_tmp1F6 + 1;return;}_LL178: if(_tmp1E9 <= (void*)10)goto
_LL17A;if(*((int*)_tmp1E9)!= 1)goto _LL17A;_tmp1EB=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp1E9)->f1;_tmp1EC=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp1E9)->f2;_tmp1ED=(
void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp1E9)->f3;_tmp1EE=(void*)((struct Cyc_Port_Ptr_ct_struct*)
_tmp1E9)->f4;_tmp1EF=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp1E9)->f5;_LL179:
Cyc_Port_region_counts(cts,_tmp1EB);Cyc_Port_region_counts(cts,_tmp1EE);return;
_LL17A: if(_tmp1E9 <= (void*)10)goto _LL17C;if(*((int*)_tmp1E9)!= 2)goto _LL17C;
_tmp1F0=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp1E9)->f1;_tmp1F1=(void*)((
struct Cyc_Port_Array_ct_struct*)_tmp1E9)->f2;_tmp1F2=(void*)((struct Cyc_Port_Array_ct_struct*)
_tmp1E9)->f3;_LL17B: Cyc_Port_region_counts(cts,_tmp1F0);return;_LL17C: if(_tmp1E9
<= (void*)10)goto _LL15B;if(*((int*)_tmp1E9)!= 5)goto _LL15B;_tmp1F3=(void*)((
struct Cyc_Port_Fn_ct_struct*)_tmp1E9)->f1;_tmp1F4=((struct Cyc_Port_Fn_ct_struct*)
_tmp1E9)->f2;_LL17D: Cyc_Port_region_counts(cts,_tmp1F3);for(0;(unsigned int)
_tmp1F4;_tmp1F4=_tmp1F4->tl){Cyc_Port_region_counts(cts,(void*)_tmp1F4->hd);}
return;_LL15B:;}static void Cyc_Port_register_rgns(struct Cyc_Port_Cenv*env,struct
Cyc_Dict_Dict counts,int fn_res,void*t,void*c);static void Cyc_Port_register_rgns(
struct Cyc_Port_Cenv*env,struct Cyc_Dict_Dict counts,int fn_res,void*t,void*c){c=Cyc_Port_compress_ct(
c);{struct _tuple9 _tmp49B;struct _tuple9 _tmp1F8=(_tmp49B.f1=t,((_tmp49B.f2=c,
_tmp49B)));void*_tmp1F9;struct Cyc_Absyn_PtrInfo _tmp1FA;void*_tmp1FB;struct Cyc_Absyn_PtrAtts
_tmp1FC;void*_tmp1FD;struct Cyc_Absyn_PtrLoc*_tmp1FE;void*_tmp1FF;void*_tmp200;
void*_tmp201;void*_tmp202;struct Cyc_Absyn_ArrayInfo _tmp203;void*_tmp204;void*
_tmp205;void*_tmp206;void*_tmp207;struct Cyc_Absyn_FnInfo _tmp208;void*_tmp209;
struct Cyc_List_List*_tmp20A;void*_tmp20B;void*_tmp20C;struct Cyc_List_List*
_tmp20D;_LL17F: _tmp1F9=_tmp1F8.f1;if(_tmp1F9 <= (void*)4)goto _LL181;if(*((int*)
_tmp1F9)!= 4)goto _LL181;_tmp1FA=((struct Cyc_Absyn_PointerType_struct*)_tmp1F9)->f1;
_tmp1FB=_tmp1FA.elt_typ;_tmp1FC=_tmp1FA.ptr_atts;_tmp1FD=_tmp1FC.rgn;_tmp1FE=
_tmp1FC.ptrloc;_tmp1FF=_tmp1F8.f2;if(_tmp1FF <= (void*)10)goto _LL181;if(*((int*)
_tmp1FF)!= 1)goto _LL181;_tmp200=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp1FF)->f1;
_tmp201=(void*)((struct Cyc_Port_Ptr_ct_struct*)_tmp1FF)->f4;_LL180: Cyc_Port_register_rgns(
env,counts,fn_res,_tmp1FB,_tmp200);{struct Cyc_Position_Segment*_tmp20E=(
unsigned int)_tmp1FE?_tmp1FE->rgn_loc: 0;_tmp201=Cyc_Port_compress_ct(_tmp201);{
struct _tuple9 _tmp49C;struct _tuple9 _tmp210=(_tmp49C.f1=_tmp1FD,((_tmp49C.f2=
_tmp201,_tmp49C)));void*_tmp211;void*_tmp212;void*_tmp213;void*_tmp214;struct
_dyneither_ptr*_tmp215;_LL188: _tmp211=_tmp210.f1;if(_tmp211 <= (void*)4)goto
_LL18A;if(*((int*)_tmp211)!= 0)goto _LL18A;_tmp212=_tmp210.f2;if((int)_tmp212 != 7)
goto _LL18A;if(!(!fn_res))goto _LL18A;_LL189:{struct Cyc_Port_Edit*_tmp4A5;const
char*_tmp4A4;const char*_tmp4A3;struct Cyc_List_List*_tmp4A2;(env->gcenv)->edits=((
_tmp4A2=_cycalloc(sizeof(*_tmp4A2)),((_tmp4A2->hd=((_tmp4A5=_cycalloc(sizeof(*
_tmp4A5)),((_tmp4A5->loc=_tmp20E,((_tmp4A5->old_string=((_tmp4A3="`H ",
_tag_dyneither(_tmp4A3,sizeof(char),4))),((_tmp4A5->new_string=((_tmp4A4="",
_tag_dyneither(_tmp4A4,sizeof(char),1))),_tmp4A5)))))))),((_tmp4A2->tl=(env->gcenv)->edits,
_tmp4A2))))));}goto _LL187;_LL18A: _tmp213=_tmp210.f1;if(_tmp213 <= (void*)4)goto
_LL18C;if(*((int*)_tmp213)!= 0)goto _LL18C;_tmp214=_tmp210.f2;if(_tmp214 <= (void*)
10)goto _LL18C;if(*((int*)_tmp214)!= 0)goto _LL18C;_tmp215=((struct Cyc_Port_RgnVar_ct_struct*)
_tmp214)->f1;_LL18B: {int _tmp21A=*((int*(*)(struct Cyc_Dict_Dict d,struct
_dyneither_ptr*k))Cyc_Dict_lookup)(counts,_tmp215);if(_tmp21A > 1){struct Cyc_Port_Edit*
_tmp4B5;const char*_tmp4B4;const char*_tmp4B3;void*_tmp4B2[1];struct Cyc_String_pa_struct
_tmp4B1;struct Cyc_List_List*_tmp4B0;(env->gcenv)->edits=((_tmp4B0=_cycalloc(
sizeof(*_tmp4B0)),((_tmp4B0->hd=((_tmp4B5=_cycalloc(sizeof(*_tmp4B5)),((_tmp4B5->loc=
_tmp20E,((_tmp4B5->old_string=(struct _dyneither_ptr)((_tmp4B1.tag=0,((_tmp4B1.f1=(
struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp215),((_tmp4B2[0]=& _tmp4B1,Cyc_aprintf(((
_tmp4B3="%s ",_tag_dyneither(_tmp4B3,sizeof(char),4))),_tag_dyneither(_tmp4B2,
sizeof(void*),1)))))))),((_tmp4B5->new_string=((_tmp4B4="",_tag_dyneither(
_tmp4B4,sizeof(char),1))),_tmp4B5)))))))),((_tmp4B0->tl=(env->gcenv)->edits,
_tmp4B0))))));}goto _LL187;}_LL18C:;_LL18D: goto _LL187;_LL187:;}goto _LL17E;}_LL181:
_tmp202=_tmp1F8.f1;if(_tmp202 <= (void*)4)goto _LL183;if(*((int*)_tmp202)!= 7)goto
_LL183;_tmp203=((struct Cyc_Absyn_ArrayType_struct*)_tmp202)->f1;_tmp204=_tmp203.elt_type;
_tmp205=_tmp1F8.f2;if(_tmp205 <= (void*)10)goto _LL183;if(*((int*)_tmp205)!= 2)
goto _LL183;_tmp206=(void*)((struct Cyc_Port_Array_ct_struct*)_tmp205)->f1;_LL182:
Cyc_Port_register_rgns(env,counts,fn_res,_tmp204,_tmp206);goto _LL17E;_LL183:
_tmp207=_tmp1F8.f1;if(_tmp207 <= (void*)4)goto _LL185;if(*((int*)_tmp207)!= 8)goto
_LL185;_tmp208=((struct Cyc_Absyn_FnType_struct*)_tmp207)->f1;_tmp209=_tmp208.ret_typ;
_tmp20A=_tmp208.args;_tmp20B=_tmp1F8.f2;if(_tmp20B <= (void*)10)goto _LL185;if(*((
int*)_tmp20B)!= 5)goto _LL185;_tmp20C=(void*)((struct Cyc_Port_Fn_ct_struct*)
_tmp20B)->f1;_tmp20D=((struct Cyc_Port_Fn_ct_struct*)_tmp20B)->f2;_LL184: Cyc_Port_register_rgns(
env,counts,1,_tmp209,_tmp20C);for(0;_tmp20A != 0  && _tmp20D != 0;(_tmp20A=_tmp20A->tl,
_tmp20D=_tmp20D->tl)){void*_tmp222;struct _tuple7 _tmp221=*((struct _tuple7*)
_tmp20A->hd);_tmp222=_tmp221.f3;{void*_tmp223=(void*)_tmp20D->hd;Cyc_Port_register_rgns(
env,counts,0,_tmp222,_tmp223);}}goto _LL17E;_LL185:;_LL186: goto _LL17E;_LL17E:;}}
static void*Cyc_Port_gen_exp(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Exp*e);
static void Cyc_Port_gen_stmt(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Stmt*s);
static struct Cyc_Port_Cenv*Cyc_Port_gen_localdecl(struct Cyc_Port_Cenv*env,struct
Cyc_Absyn_Decl*d);static int Cyc_Port_is_char(struct Cyc_Port_Cenv*env,void*t);
static int Cyc_Port_is_char(struct Cyc_Port_Cenv*env,void*t){void*_tmp224=t;struct
_tuple0*_tmp225;void*_tmp226;_LL18F: if(_tmp224 <= (void*)4)goto _LL193;if(*((int*)
_tmp224)!= 16)goto _LL191;_tmp225=((struct Cyc_Absyn_TypedefType_struct*)_tmp224)->f1;
_LL190:(*_tmp225).f1=Cyc_Absyn_Loc_n;return Cyc_Port_is_char(env,Cyc_Port_lookup_typedef(
env,_tmp225));_LL191: if(*((int*)_tmp224)!= 5)goto _LL193;_tmp226=(void*)((struct
Cyc_Absyn_IntType_struct*)_tmp224)->f2;_LL192: return 1;_LL193:;_LL194: return 0;
_LL18E:;}static void*Cyc_Port_type_to_ctype(struct Cyc_Port_Cenv*env,void*t);
static void*Cyc_Port_type_to_ctype(struct Cyc_Port_Cenv*env,void*t){void*_tmp227=t;
struct _tuple0*_tmp228;struct Cyc_Absyn_PtrInfo _tmp229;void*_tmp22A;struct Cyc_Absyn_Tqual
_tmp22B;struct Cyc_Absyn_PtrAtts _tmp22C;void*_tmp22D;union Cyc_Absyn_Constraint*
_tmp22E;union Cyc_Absyn_Constraint*_tmp22F;union Cyc_Absyn_Constraint*_tmp230;
struct Cyc_Absyn_PtrLoc*_tmp231;struct Cyc_Absyn_ArrayInfo _tmp232;void*_tmp233;
struct Cyc_Absyn_Tqual _tmp234;union Cyc_Absyn_Constraint*_tmp235;struct Cyc_Position_Segment*
_tmp236;struct Cyc_Absyn_FnInfo _tmp237;void*_tmp238;struct Cyc_List_List*_tmp239;
struct Cyc_Absyn_AggrInfo _tmp23A;union Cyc_Absyn_AggrInfoU _tmp23B;struct Cyc_List_List*
_tmp23C;_LL196: if(_tmp227 <= (void*)4)goto _LL198;if(*((int*)_tmp227)!= 16)goto
_LL198;_tmp228=((struct Cyc_Absyn_TypedefType_struct*)_tmp227)->f1;_LL197:(*
_tmp228).f1=Cyc_Absyn_Loc_n;return Cyc_Port_lookup_typedef_ctype(env,_tmp228);
_LL198: if((int)_tmp227 != 0)goto _LL19A;_LL199: return Cyc_Port_void_ct();_LL19A: if(
_tmp227 <= (void*)4)goto _LL19E;if(*((int*)_tmp227)!= 4)goto _LL19C;_tmp229=((
struct Cyc_Absyn_PointerType_struct*)_tmp227)->f1;_tmp22A=_tmp229.elt_typ;_tmp22B=
_tmp229.elt_tq;_tmp22C=_tmp229.ptr_atts;_tmp22D=_tmp22C.rgn;_tmp22E=_tmp22C.nullable;
_tmp22F=_tmp22C.bounds;_tmp230=_tmp22C.zero_term;_tmp231=_tmp22C.ptrloc;_LL19B: {
struct Cyc_Position_Segment*_tmp23D=(unsigned int)_tmp231?_tmp231->ptr_loc: 0;
struct Cyc_Position_Segment*_tmp23E=(unsigned int)_tmp231?_tmp231->rgn_loc: 0;
struct Cyc_Position_Segment*_tmp23F=(unsigned int)_tmp231?_tmp231->zt_loc: 0;void*
_tmp240=Cyc_Port_type_to_ctype(env,_tmp22A);void*new_rgn;{void*_tmp241=_tmp22D;
struct Cyc_Absyn_Tvar*_tmp242;struct Cyc_Absyn_Tvar _tmp243;struct _dyneither_ptr*
_tmp244;_LL1AF: if((int)_tmp241 != 2)goto _LL1B1;_LL1B0: new_rgn=Cyc_Port_heap_ct();
goto _LL1AE;_LL1B1: if(_tmp241 <= (void*)4)goto _LL1B5;if(*((int*)_tmp241)!= 1)goto
_LL1B3;_tmp242=((struct Cyc_Absyn_VarType_struct*)_tmp241)->f1;_tmp243=*_tmp242;
_tmp244=_tmp243.name;_LL1B2: new_rgn=Cyc_Port_rgnvar_ct(_tmp244);goto _LL1AE;
_LL1B3: if(*((int*)_tmp241)!= 0)goto _LL1B5;_LL1B4: if(Cyc_Port_in_fn_arg(env))
new_rgn=Cyc_Port_rgnvar_ct(Cyc_Port_new_region_var());else{if(Cyc_Port_in_fn_res(
env) || Cyc_Port_in_toplevel(env))new_rgn=Cyc_Port_heap_ct();else{new_rgn=Cyc_Port_var();}}
goto _LL1AE;_LL1B5:;_LL1B6: new_rgn=Cyc_Port_heap_ct();goto _LL1AE;_LL1AE:;}{int
_tmp245=Cyc_Absyn_conref_val(_tmp22F)== (void*)0;int orig_zt;{union Cyc_Absyn_Constraint
_tmp246=*_tmp230;int _tmp247;_LL1B8: if((_tmp246.No_constr).tag != 3)goto _LL1BA;
_tmp247=(int)(_tmp246.No_constr).val;_LL1B9: orig_zt=Cyc_Port_is_char(env,_tmp22A);
goto _LL1B7;_LL1BA:;_LL1BB: orig_zt=((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(
_tmp230);goto _LL1B7;_LL1B7:;}if((env->gcenv)->porting){void*_tmp248=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,_tmp248,_tmp22B.print_const?Cyc_Port_const_ct():
Cyc_Port_notconst_ct(),_tmp22B.loc);if(_tmp22B.print_const)Cyc_Port_unify_ct(
_tmp248,Cyc_Port_const_ct());{void*_tmp249=Cyc_Port_var();Cyc_Port_register_ptr_cvars(
env,_tmp249,_tmp245?Cyc_Port_fat_ct(): Cyc_Port_thin_ct(),_tmp23D);if(_tmp245)Cyc_Port_unify_ct(
_tmp249,Cyc_Port_fat_ct());{void*_tmp24A=Cyc_Port_var();Cyc_Port_register_zeroterm_cvars(
env,_tmp24A,orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct(),_tmp23F);{union Cyc_Absyn_Constraint
_tmp24B=*_tmp230;int _tmp24C;_LL1BD: if((_tmp24B.No_constr).tag != 3)goto _LL1BF;
_tmp24C=(int)(_tmp24B.No_constr).val;_LL1BE: goto _LL1BC;_LL1BF:;_LL1C0: Cyc_Port_unify_ct(
_tmp24A,((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp230)?Cyc_Port_zterm_ct():
Cyc_Port_nozterm_ct());goto _LL1BC;_LL1BC:;}return Cyc_Port_ptr_ct(_tmp240,_tmp248,
_tmp249,new_rgn,_tmp24A);}}}else{return Cyc_Port_ptr_ct(_tmp240,_tmp22B.print_const?
Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),_tmp245?Cyc_Port_fat_ct(): Cyc_Port_thin_ct(),
new_rgn,orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());}}}_LL19C: if(*((int*)
_tmp227)!= 5)goto _LL19E;_LL19D: goto _LL19F;_LL19E: if((int)_tmp227 != 1)goto _LL1A0;
_LL19F: goto _LL1A1;_LL1A0: if(_tmp227 <= (void*)4)goto _LL1AC;if(*((int*)_tmp227)!= 
6)goto _LL1A2;_LL1A1: return Cyc_Port_arith_ct();_LL1A2: if(*((int*)_tmp227)!= 7)
goto _LL1A4;_tmp232=((struct Cyc_Absyn_ArrayType_struct*)_tmp227)->f1;_tmp233=
_tmp232.elt_type;_tmp234=_tmp232.tq;_tmp235=_tmp232.zero_term;_tmp236=_tmp232.zt_loc;
_LL1A3: {void*_tmp24D=Cyc_Port_type_to_ctype(env,_tmp233);int orig_zt;{union Cyc_Absyn_Constraint
_tmp24E=*_tmp235;int _tmp24F;_LL1C2: if((_tmp24E.No_constr).tag != 3)goto _LL1C4;
_tmp24F=(int)(_tmp24E.No_constr).val;_LL1C3: orig_zt=0;goto _LL1C1;_LL1C4:;_LL1C5:
orig_zt=((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp235);
goto _LL1C1;_LL1C1:;}if((env->gcenv)->porting){void*_tmp250=Cyc_Port_var();Cyc_Port_register_const_cvar(
env,_tmp250,_tmp234.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),
_tmp234.loc);{void*_tmp251=Cyc_Port_var();Cyc_Port_register_zeroterm_cvars(env,
_tmp251,orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct(),_tmp236);{union Cyc_Absyn_Constraint
_tmp252=*_tmp235;int _tmp253;_LL1C7: if((_tmp252.No_constr).tag != 3)goto _LL1C9;
_tmp253=(int)(_tmp252.No_constr).val;_LL1C8: goto _LL1C6;_LL1C9:;_LL1CA: Cyc_Port_unify_ct(
_tmp251,((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp235)?Cyc_Port_zterm_ct():
Cyc_Port_nozterm_ct());goto _LL1C6;_LL1C6:;}return Cyc_Port_array_ct(_tmp24D,
_tmp250,_tmp251);}}else{return Cyc_Port_array_ct(_tmp24D,_tmp234.print_const?Cyc_Port_const_ct():
Cyc_Port_notconst_ct(),orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());}}
_LL1A4: if(*((int*)_tmp227)!= 8)goto _LL1A6;_tmp237=((struct Cyc_Absyn_FnType_struct*)
_tmp227)->f1;_tmp238=_tmp237.ret_typ;_tmp239=_tmp237.args;_LL1A5: {void*_tmp254=
Cyc_Port_type_to_ctype(Cyc_Port_set_cpos(env,(void*)0),_tmp238);struct Cyc_Port_Cenv*
_tmp255=Cyc_Port_set_cpos(env,(void*)1);struct Cyc_List_List*cargs=0;for(0;
_tmp239 != 0;_tmp239=_tmp239->tl){struct Cyc_Absyn_Tqual _tmp257;void*_tmp258;
struct _tuple7 _tmp256=*((struct _tuple7*)_tmp239->hd);_tmp257=_tmp256.f2;_tmp258=
_tmp256.f3;{struct Cyc_List_List*_tmp4B6;cargs=((_tmp4B6=_cycalloc(sizeof(*
_tmp4B6)),((_tmp4B6->hd=(void*)Cyc_Port_type_to_ctype(_tmp255,_tmp258),((_tmp4B6->tl=
cargs,_tmp4B6))))));}}return Cyc_Port_fn_ct(_tmp254,Cyc_List_imp_rev(cargs));}
_LL1A6: if(*((int*)_tmp227)!= 10)goto _LL1A8;_tmp23A=((struct Cyc_Absyn_AggrType_struct*)
_tmp227)->f1;_tmp23B=_tmp23A.aggr_info;_LL1A7: {union Cyc_Absyn_AggrInfoU _tmp25A=
_tmp23B;struct _tuple2 _tmp25B;void*_tmp25C;struct _tuple0*_tmp25D;struct _tuple2
_tmp25E;void*_tmp25F;struct _tuple0*_tmp260;struct Cyc_Absyn_Aggrdecl**_tmp261;
_LL1CC: if((_tmp25A.UnknownAggr).tag != 1)goto _LL1CE;_tmp25B=(struct _tuple2)(
_tmp25A.UnknownAggr).val;_tmp25C=_tmp25B.f1;if((int)_tmp25C != 0)goto _LL1CE;
_tmp25D=_tmp25B.f2;_LL1CD:(*_tmp25D).f1=Cyc_Absyn_Loc_n;{struct _tuple10*_tmp262=
Cyc_Port_lookup_struct_decl(env,_tmp25D);return Cyc_Port_known_aggr_ct(_tmp262);}
_LL1CE: if((_tmp25A.UnknownAggr).tag != 1)goto _LL1D0;_tmp25E=(struct _tuple2)(
_tmp25A.UnknownAggr).val;_tmp25F=_tmp25E.f1;if((int)_tmp25F != 1)goto _LL1D0;
_tmp260=_tmp25E.f2;_LL1CF:(*_tmp260).f1=Cyc_Absyn_Loc_n;{struct _tuple10*_tmp263=
Cyc_Port_lookup_union_decl(env,_tmp260);return Cyc_Port_known_aggr_ct(_tmp263);}
_LL1D0: if((_tmp25A.KnownAggr).tag != 2)goto _LL1CB;_tmp261=(struct Cyc_Absyn_Aggrdecl**)(
_tmp25A.KnownAggr).val;_LL1D1: {struct Cyc_Absyn_Aggrdecl*_tmp264=*_tmp261;void*
_tmp265=_tmp264->kind;_LL1D3: if((int)_tmp265 != 0)goto _LL1D5;_LL1D4: {struct
_tuple10*_tmp266=Cyc_Port_lookup_struct_decl(env,_tmp264->name);return Cyc_Port_known_aggr_ct(
_tmp266);}_LL1D5: if((int)_tmp265 != 1)goto _LL1D2;_LL1D6: {struct _tuple10*_tmp267=
Cyc_Port_lookup_union_decl(env,_tmp264->name);return Cyc_Port_known_aggr_ct(
_tmp267);}_LL1D2:;}_LL1CB:;}_LL1A8: if(*((int*)_tmp227)!= 12)goto _LL1AA;_LL1A9:
return Cyc_Port_arith_ct();_LL1AA: if(*((int*)_tmp227)!= 13)goto _LL1AC;_tmp23C=((
struct Cyc_Absyn_AnonEnumType_struct*)_tmp227)->f1;_LL1AB: for(0;(unsigned int)
_tmp23C;_tmp23C=_tmp23C->tl){(*((struct Cyc_Absyn_Enumfield*)_tmp23C->hd)->name).f1=
Cyc_Absyn_Loc_n;env=Cyc_Port_add_var(env,((struct Cyc_Absyn_Enumfield*)_tmp23C->hd)->name,
Cyc_Absyn_sint_typ,Cyc_Port_const_ct(),Cyc_Port_arith_ct());}return Cyc_Port_arith_ct();
_LL1AC:;_LL1AD: return Cyc_Port_arith_ct();_LL195:;}static void*Cyc_Port_gen_primop(
struct Cyc_Port_Cenv*env,void*p,struct Cyc_List_List*args);static void*Cyc_Port_gen_primop(
struct Cyc_Port_Cenv*env,void*p,struct Cyc_List_List*args){void*_tmp268=Cyc_Port_compress_ct((
void*)((struct Cyc_List_List*)_check_null(args))->hd);struct Cyc_List_List*_tmp269=
args->tl;void*_tmp26A=p;_LL1D8: if((int)_tmp26A != 0)goto _LL1DA;_LL1D9: {void*
_tmp26B=Cyc_Port_compress_ct((void*)((struct Cyc_List_List*)_check_null(_tmp269))->hd);
if(Cyc_Port_leq(_tmp268,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),
Cyc_Port_var(),Cyc_Port_var()))){Cyc_Port_leq(_tmp26B,Cyc_Port_arith_ct());
return _tmp268;}else{if(Cyc_Port_leq(_tmp26B,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),
Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()))){Cyc_Port_leq(_tmp268,Cyc_Port_arith_ct());
return _tmp26B;}else{Cyc_Port_leq(_tmp268,Cyc_Port_arith_ct());Cyc_Port_leq(
_tmp26B,Cyc_Port_arith_ct());return Cyc_Port_arith_ct();}}}_LL1DA: if((int)_tmp26A
!= 2)goto _LL1DC;_LL1DB: if(_tmp269 == 0){Cyc_Port_leq(_tmp268,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();}else{void*_tmp26C=Cyc_Port_compress_ct((void*)_tmp269->hd);
if(Cyc_Port_leq(_tmp268,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),
Cyc_Port_var(),Cyc_Port_var()))){if(Cyc_Port_leq(_tmp26C,Cyc_Port_ptr_ct(Cyc_Port_var(),
Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var())))return Cyc_Port_arith_ct();
else{Cyc_Port_leq(_tmp26C,Cyc_Port_arith_ct());return _tmp268;}}else{Cyc_Port_leq(
_tmp268,Cyc_Port_arith_ct());Cyc_Port_leq(_tmp268,Cyc_Port_arith_ct());return Cyc_Port_arith_ct();}}
_LL1DC: if((int)_tmp26A != 1)goto _LL1DE;_LL1DD: goto _LL1DF;_LL1DE: if((int)_tmp26A != 
3)goto _LL1E0;_LL1DF: goto _LL1E1;_LL1E0: if((int)_tmp26A != 4)goto _LL1E2;_LL1E1: goto
_LL1E3;_LL1E2: if((int)_tmp26A != 5)goto _LL1E4;_LL1E3: goto _LL1E5;_LL1E4: if((int)
_tmp26A != 6)goto _LL1E6;_LL1E5: goto _LL1E7;_LL1E6: if((int)_tmp26A != 8)goto _LL1E8;
_LL1E7: goto _LL1E9;_LL1E8: if((int)_tmp26A != 7)goto _LL1EA;_LL1E9: goto _LL1EB;_LL1EA:
if((int)_tmp26A != 10)goto _LL1EC;_LL1EB: goto _LL1ED;_LL1EC: if((int)_tmp26A != 9)
goto _LL1EE;_LL1ED: goto _LL1EF;_LL1EE: if((int)_tmp26A != 13)goto _LL1F0;_LL1EF: goto
_LL1F1;_LL1F0: if((int)_tmp26A != 14)goto _LL1F2;_LL1F1: goto _LL1F3;_LL1F2: if((int)
_tmp26A != 15)goto _LL1F4;_LL1F3: goto _LL1F5;_LL1F4: if((int)_tmp26A != 16)goto _LL1F6;
_LL1F5: goto _LL1F7;_LL1F6: if((int)_tmp26A != 17)goto _LL1F8;_LL1F7: goto _LL1F9;
_LL1F8: if((int)_tmp26A != 18)goto _LL1FA;_LL1F9: Cyc_Port_leq(_tmp268,Cyc_Port_arith_ct());
Cyc_Port_leq((void*)((struct Cyc_List_List*)_check_null(_tmp269))->hd,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();_LL1FA: if((int)_tmp26A != 11)goto _LL1FC;_LL1FB: goto
_LL1FD;_LL1FC: if((int)_tmp26A != 12)goto _LL1FE;_LL1FD: Cyc_Port_leq(_tmp268,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();_LL1FE:;_LL1FF: {const char*_tmp4B9;void*_tmp4B8;(
_tmp4B8=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp4B9=".size primop",_tag_dyneither(_tmp4B9,sizeof(char),13))),_tag_dyneither(
_tmp4B8,sizeof(void*),0)));}_LL1D7:;}static struct _tuple9 Cyc_Port_gen_lhs(struct
Cyc_Port_Cenv*env,struct Cyc_Absyn_Exp*e);static struct _tuple9 Cyc_Port_gen_lhs(
struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Exp*e){void*_tmp26F=e->r;struct _tuple0*
_tmp270;struct _tuple0*_tmp271;struct Cyc_Absyn_Exp*_tmp272;struct Cyc_Absyn_Exp*
_tmp273;struct Cyc_Absyn_Exp*_tmp274;struct Cyc_Absyn_Exp*_tmp275;struct
_dyneither_ptr*_tmp276;struct Cyc_Absyn_Exp*_tmp277;struct _dyneither_ptr*_tmp278;
_LL201: if(*((int*)_tmp26F)!= 1)goto _LL203;_tmp270=((struct Cyc_Absyn_Var_e_struct*)
_tmp26F)->f1;_LL202: _tmp271=_tmp270;goto _LL204;_LL203: if(*((int*)_tmp26F)!= 2)
goto _LL205;_tmp271=((struct Cyc_Absyn_UnknownId_e_struct*)_tmp26F)->f1;_LL204:(*
_tmp271).f1=Cyc_Absyn_Loc_n;return Cyc_Port_lookup_var(env,_tmp271);_LL205: if(*((
int*)_tmp26F)!= 25)goto _LL207;_tmp272=((struct Cyc_Absyn_Subscript_e_struct*)
_tmp26F)->f1;_tmp273=((struct Cyc_Absyn_Subscript_e_struct*)_tmp26F)->f2;_LL206: {
void*_tmp279=Cyc_Port_var();void*_tmp27A=Cyc_Port_var();void*_tmp27B=Cyc_Port_gen_exp(
env,_tmp272);Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp273),Cyc_Port_arith_ct());{
void*_tmp27C=Cyc_Port_ptr_ct(_tmp279,_tmp27A,Cyc_Port_fat_ct(),Cyc_Port_var(),
Cyc_Port_var());Cyc_Port_leq(_tmp27B,_tmp27C);{struct _tuple9 _tmp4BA;return(
_tmp4BA.f1=_tmp27A,((_tmp4BA.f2=_tmp279,_tmp4BA)));}}}_LL207: if(*((int*)_tmp26F)
!= 22)goto _LL209;_tmp274=((struct Cyc_Absyn_Deref_e_struct*)_tmp26F)->f1;_LL208: {
void*_tmp27E=Cyc_Port_var();void*_tmp27F=Cyc_Port_var();void*_tmp280=Cyc_Port_ptr_ct(
_tmp27E,_tmp27F,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var());Cyc_Port_leq(Cyc_Port_gen_exp(
env,_tmp274),_tmp280);{struct _tuple9 _tmp4BB;return(_tmp4BB.f1=_tmp27F,((_tmp4BB.f2=
_tmp27E,_tmp4BB)));}}_LL209: if(*((int*)_tmp26F)!= 23)goto _LL20B;_tmp275=((struct
Cyc_Absyn_AggrMember_e_struct*)_tmp26F)->f1;_tmp276=((struct Cyc_Absyn_AggrMember_e_struct*)
_tmp26F)->f2;_LL20A: {void*_tmp282=Cyc_Port_var();void*_tmp283=Cyc_Port_var();
void*_tmp284=Cyc_Port_gen_exp(env,_tmp275);{struct Cyc_Port_Cfield*_tmp4BE;struct
Cyc_Port_Cfield*_tmp4BD[1];Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp275),Cyc_Port_unknown_aggr_ct(((
_tmp4BD[0]=((_tmp4BE=_cycalloc(sizeof(*_tmp4BE)),((_tmp4BE->qual=_tmp283,((
_tmp4BE->f=_tmp276,((_tmp4BE->type=_tmp282,_tmp4BE)))))))),((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp4BD,sizeof(struct Cyc_Port_Cfield*),
1))))));}{struct _tuple9 _tmp4BF;return(_tmp4BF.f1=_tmp283,((_tmp4BF.f2=_tmp282,
_tmp4BF)));}}_LL20B: if(*((int*)_tmp26F)!= 24)goto _LL20D;_tmp277=((struct Cyc_Absyn_AggrArrow_e_struct*)
_tmp26F)->f1;_tmp278=((struct Cyc_Absyn_AggrArrow_e_struct*)_tmp26F)->f2;_LL20C: {
void*_tmp288=Cyc_Port_var();void*_tmp289=Cyc_Port_var();void*_tmp28A=Cyc_Port_gen_exp(
env,_tmp277);{struct Cyc_Port_Cfield*_tmp4C2;struct Cyc_Port_Cfield*_tmp4C1[1];Cyc_Port_leq(
Cyc_Port_gen_exp(env,_tmp277),Cyc_Port_ptr_ct(Cyc_Port_unknown_aggr_ct(((_tmp4C1[
0]=((_tmp4C2=_cycalloc(sizeof(*_tmp4C2)),((_tmp4C2->qual=_tmp289,((_tmp4C2->f=
_tmp278,((_tmp4C2->type=_tmp288,_tmp4C2)))))))),((struct Cyc_List_List*(*)(struct
_dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp4C1,sizeof(struct Cyc_Port_Cfield*),
1))))),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var()));}{struct
_tuple9 _tmp4C3;return(_tmp4C3.f1=_tmp289,((_tmp4C3.f2=_tmp288,_tmp4C3)));}}
_LL20D:;_LL20E: {struct Cyc_String_pa_struct _tmp4CB;void*_tmp4CA[1];const char*
_tmp4C9;void*_tmp4C8;(_tmp4C8=0,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Tcutil_impos)((struct _dyneither_ptr)((_tmp4CB.tag=0,((
_tmp4CB.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(e)),((
_tmp4CA[0]=& _tmp4CB,Cyc_aprintf(((_tmp4C9="gen_lhs: %s",_tag_dyneither(_tmp4C9,
sizeof(char),12))),_tag_dyneither(_tmp4CA,sizeof(void*),1)))))))),_tag_dyneither(
_tmp4C8,sizeof(void*),0)));}_LL200:;}static void*Cyc_Port_gen_exp(struct Cyc_Port_Cenv*
env,struct Cyc_Absyn_Exp*e);static void*Cyc_Port_gen_exp(struct Cyc_Port_Cenv*env,
struct Cyc_Absyn_Exp*e){void*_tmp292=e->r;union Cyc_Absyn_Cnst _tmp293;struct
_tuple3 _tmp294;union Cyc_Absyn_Cnst _tmp295;struct _tuple4 _tmp296;union Cyc_Absyn_Cnst
_tmp297;struct _tuple6 _tmp298;union Cyc_Absyn_Cnst _tmp299;struct _dyneither_ptr
_tmp29A;union Cyc_Absyn_Cnst _tmp29B;struct _tuple5 _tmp29C;int _tmp29D;union Cyc_Absyn_Cnst
_tmp29E;struct _tuple5 _tmp29F;union Cyc_Absyn_Cnst _tmp2A0;struct _dyneither_ptr
_tmp2A1;union Cyc_Absyn_Cnst _tmp2A2;int _tmp2A3;struct _tuple0*_tmp2A4;struct
_tuple0*_tmp2A5;void*_tmp2A6;struct Cyc_List_List*_tmp2A7;struct Cyc_Absyn_Exp*
_tmp2A8;struct Cyc_Core_Opt*_tmp2A9;struct Cyc_Absyn_Exp*_tmp2AA;struct Cyc_Absyn_Exp*
_tmp2AB;void*_tmp2AC;struct Cyc_Absyn_Exp*_tmp2AD;struct Cyc_Absyn_Exp*_tmp2AE;
struct Cyc_Absyn_Exp*_tmp2AF;struct Cyc_Absyn_Exp*_tmp2B0;struct Cyc_Absyn_Exp*
_tmp2B1;struct Cyc_Absyn_Exp*_tmp2B2;struct Cyc_Absyn_Exp*_tmp2B3;struct Cyc_Absyn_Exp*
_tmp2B4;struct Cyc_Absyn_Exp*_tmp2B5;struct Cyc_Absyn_Exp*_tmp2B6;struct Cyc_List_List*
_tmp2B7;struct Cyc_Absyn_Exp*_tmp2B8;struct Cyc_List_List*_tmp2B9;struct Cyc_Absyn_Exp*
_tmp2BA;void*_tmp2BB;struct Cyc_Absyn_Exp*_tmp2BC;struct Cyc_Absyn_Exp*_tmp2BD;
struct Cyc_Absyn_Exp*_tmp2BE;struct Cyc_Absyn_Exp*_tmp2BF;struct Cyc_Absyn_Exp*
_tmp2C0;struct Cyc_Absyn_Exp*_tmp2C1;struct _dyneither_ptr*_tmp2C2;struct Cyc_Absyn_Exp*
_tmp2C3;struct _dyneither_ptr*_tmp2C4;struct Cyc_Absyn_MallocInfo _tmp2C5;void**
_tmp2C6;struct Cyc_Absyn_Exp*_tmp2C7;struct Cyc_Absyn_Exp*_tmp2C8;struct Cyc_Absyn_Exp*
_tmp2C9;struct Cyc_Absyn_Stmt*_tmp2CA;_LL210: if(*((int*)_tmp292)!= 0)goto _LL212;
_tmp293=((struct Cyc_Absyn_Const_e_struct*)_tmp292)->f1;if((_tmp293.Char_c).tag != 
2)goto _LL212;_tmp294=(struct _tuple3)(_tmp293.Char_c).val;_LL211: goto _LL213;
_LL212: if(*((int*)_tmp292)!= 0)goto _LL214;_tmp295=((struct Cyc_Absyn_Const_e_struct*)
_tmp292)->f1;if((_tmp295.Short_c).tag != 3)goto _LL214;_tmp296=(struct _tuple4)(
_tmp295.Short_c).val;_LL213: goto _LL215;_LL214: if(*((int*)_tmp292)!= 0)goto _LL216;
_tmp297=((struct Cyc_Absyn_Const_e_struct*)_tmp292)->f1;if((_tmp297.LongLong_c).tag
!= 5)goto _LL216;_tmp298=(struct _tuple6)(_tmp297.LongLong_c).val;_LL215: goto
_LL217;_LL216: if(*((int*)_tmp292)!= 18)goto _LL218;_LL217: goto _LL219;_LL218: if(*((
int*)_tmp292)!= 19)goto _LL21A;_LL219: goto _LL21B;_LL21A: if(*((int*)_tmp292)!= 20)
goto _LL21C;_LL21B: goto _LL21D;_LL21C: if(*((int*)_tmp292)!= 33)goto _LL21E;_LL21D:
goto _LL21F;_LL21E: if(*((int*)_tmp292)!= 34)goto _LL220;_LL21F: goto _LL221;_LL220:
if(*((int*)_tmp292)!= 0)goto _LL222;_tmp299=((struct Cyc_Absyn_Const_e_struct*)
_tmp292)->f1;if((_tmp299.Float_c).tag != 6)goto _LL222;_tmp29A=(struct
_dyneither_ptr)(_tmp299.Float_c).val;_LL221: return Cyc_Port_arith_ct();_LL222: if(*((
int*)_tmp292)!= 0)goto _LL224;_tmp29B=((struct Cyc_Absyn_Const_e_struct*)_tmp292)->f1;
if((_tmp29B.Int_c).tag != 4)goto _LL224;_tmp29C=(struct _tuple5)(_tmp29B.Int_c).val;
_tmp29D=_tmp29C.f2;if(_tmp29D != 0)goto _LL224;_LL223: return Cyc_Port_zero_ct();
_LL224: if(*((int*)_tmp292)!= 0)goto _LL226;_tmp29E=((struct Cyc_Absyn_Const_e_struct*)
_tmp292)->f1;if((_tmp29E.Int_c).tag != 4)goto _LL226;_tmp29F=(struct _tuple5)(
_tmp29E.Int_c).val;_LL225: return Cyc_Port_arith_ct();_LL226: if(*((int*)_tmp292)!= 
0)goto _LL228;_tmp2A0=((struct Cyc_Absyn_Const_e_struct*)_tmp292)->f1;if((_tmp2A0.String_c).tag
!= 7)goto _LL228;_tmp2A1=(struct _dyneither_ptr)(_tmp2A0.String_c).val;_LL227:
return Cyc_Port_ptr_ct(Cyc_Port_arith_ct(),Cyc_Port_const_ct(),Cyc_Port_fat_ct(),
Cyc_Port_heap_ct(),Cyc_Port_zterm_ct());_LL228: if(*((int*)_tmp292)!= 0)goto
_LL22A;_tmp2A2=((struct Cyc_Absyn_Const_e_struct*)_tmp292)->f1;if((_tmp2A2.Null_c).tag
!= 1)goto _LL22A;_tmp2A3=(int)(_tmp2A2.Null_c).val;_LL229: return Cyc_Port_zero_ct();
_LL22A: if(*((int*)_tmp292)!= 1)goto _LL22C;_tmp2A4=((struct Cyc_Absyn_Var_e_struct*)
_tmp292)->f1;_LL22B: _tmp2A5=_tmp2A4;goto _LL22D;_LL22C: if(*((int*)_tmp292)!= 2)
goto _LL22E;_tmp2A5=((struct Cyc_Absyn_UnknownId_e_struct*)_tmp292)->f1;_LL22D:(*
_tmp2A5).f1=Cyc_Absyn_Loc_n;{void*_tmp2CC;struct _tuple9 _tmp2CB=Cyc_Port_lookup_var(
env,_tmp2A5);_tmp2CC=_tmp2CB.f2;if(Cyc_Port_isfn(env,_tmp2A5)){_tmp2CC=Cyc_Port_instantiate(
_tmp2CC);return Cyc_Port_ptr_ct(_tmp2CC,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_heap_ct(),
Cyc_Port_nozterm_ct());}else{if(Cyc_Port_isarray(env,_tmp2A5)){void*_tmp2CD=Cyc_Port_var();
void*_tmp2CE=Cyc_Port_var();void*_tmp2CF=Cyc_Port_var();void*_tmp2D0=Cyc_Port_array_ct(
_tmp2CD,_tmp2CE,_tmp2CF);Cyc_Port_unifies(_tmp2CC,_tmp2D0);{void*_tmp2D1=Cyc_Port_ptr_ct(
_tmp2CD,_tmp2CE,Cyc_Port_fat_ct(),Cyc_Port_var(),_tmp2CF);return _tmp2D1;}}else{
return _tmp2CC;}}}_LL22E: if(*((int*)_tmp292)!= 3)goto _LL230;_tmp2A6=(void*)((
struct Cyc_Absyn_Primop_e_struct*)_tmp292)->f1;_tmp2A7=((struct Cyc_Absyn_Primop_e_struct*)
_tmp292)->f2;_LL22F: return Cyc_Port_gen_primop(env,_tmp2A6,((struct Cyc_List_List*(*)(
void*(*f)(struct Cyc_Port_Cenv*,struct Cyc_Absyn_Exp*),struct Cyc_Port_Cenv*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_gen_exp,env,_tmp2A7));_LL230: if(*((
int*)_tmp292)!= 4)goto _LL232;_tmp2A8=((struct Cyc_Absyn_AssignOp_e_struct*)
_tmp292)->f1;_tmp2A9=((struct Cyc_Absyn_AssignOp_e_struct*)_tmp292)->f2;_tmp2AA=((
struct Cyc_Absyn_AssignOp_e_struct*)_tmp292)->f3;_LL231: {void*_tmp2D3;void*
_tmp2D4;struct _tuple9 _tmp2D2=Cyc_Port_gen_lhs(env,_tmp2A8);_tmp2D3=_tmp2D2.f1;
_tmp2D4=_tmp2D2.f2;Cyc_Port_unifies(_tmp2D3,Cyc_Port_notconst_ct());{void*
_tmp2D5=Cyc_Port_gen_exp(env,_tmp2AA);if((unsigned int)_tmp2A9){struct Cyc_List_List
_tmp4CC;struct Cyc_List_List x2=(_tmp4CC.hd=(void*)_tmp2D5,((_tmp4CC.tl=0,_tmp4CC)));
struct Cyc_List_List _tmp4CD;struct Cyc_List_List x1=(_tmp4CD.hd=(void*)_tmp2D4,((
_tmp4CD.tl=(struct Cyc_List_List*)& x2,_tmp4CD)));void*_tmp2D6=Cyc_Port_gen_primop(
env,(void*)_tmp2A9->v,(struct Cyc_List_List*)& x1);Cyc_Port_leq(_tmp2D6,_tmp2D4);
return _tmp2D4;}else{Cyc_Port_leq(_tmp2D5,_tmp2D4);return _tmp2D4;}}}_LL232: if(*((
int*)_tmp292)!= 5)goto _LL234;_tmp2AB=((struct Cyc_Absyn_Increment_e_struct*)
_tmp292)->f1;_tmp2AC=(void*)((struct Cyc_Absyn_Increment_e_struct*)_tmp292)->f2;
_LL233: {void*_tmp2DA;void*_tmp2DB;struct _tuple9 _tmp2D9=Cyc_Port_gen_lhs(env,
_tmp2AB);_tmp2DA=_tmp2D9.f1;_tmp2DB=_tmp2D9.f2;Cyc_Port_unifies(_tmp2DA,Cyc_Port_notconst_ct());
Cyc_Port_leq(_tmp2DB,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),
Cyc_Port_var(),Cyc_Port_var()));Cyc_Port_leq(_tmp2DB,Cyc_Port_arith_ct());return
_tmp2DB;}_LL234: if(*((int*)_tmp292)!= 6)goto _LL236;_tmp2AD=((struct Cyc_Absyn_Conditional_e_struct*)
_tmp292)->f1;_tmp2AE=((struct Cyc_Absyn_Conditional_e_struct*)_tmp292)->f2;
_tmp2AF=((struct Cyc_Absyn_Conditional_e_struct*)_tmp292)->f3;_LL235: {void*
_tmp2DC=Cyc_Port_var();Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2AD),Cyc_Port_arith_ct());
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2AE),_tmp2DC);Cyc_Port_leq(Cyc_Port_gen_exp(
env,_tmp2AF),_tmp2DC);return _tmp2DC;}_LL236: if(*((int*)_tmp292)!= 7)goto _LL238;
_tmp2B0=((struct Cyc_Absyn_And_e_struct*)_tmp292)->f1;_tmp2B1=((struct Cyc_Absyn_And_e_struct*)
_tmp292)->f2;_LL237: _tmp2B2=_tmp2B0;_tmp2B3=_tmp2B1;goto _LL239;_LL238: if(*((int*)
_tmp292)!= 8)goto _LL23A;_tmp2B2=((struct Cyc_Absyn_Or_e_struct*)_tmp292)->f1;
_tmp2B3=((struct Cyc_Absyn_Or_e_struct*)_tmp292)->f2;_LL239: Cyc_Port_leq(Cyc_Port_gen_exp(
env,_tmp2B2),Cyc_Port_arith_ct());Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2B3),Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();_LL23A: if(*((int*)_tmp292)!= 9)goto _LL23C;_tmp2B4=((
struct Cyc_Absyn_SeqExp_e_struct*)_tmp292)->f1;_tmp2B5=((struct Cyc_Absyn_SeqExp_e_struct*)
_tmp292)->f2;_LL23B: Cyc_Port_gen_exp(env,_tmp2B4);return Cyc_Port_gen_exp(env,
_tmp2B5);_LL23C: if(*((int*)_tmp292)!= 10)goto _LL23E;_tmp2B6=((struct Cyc_Absyn_UnknownCall_e_struct*)
_tmp292)->f1;_tmp2B7=((struct Cyc_Absyn_UnknownCall_e_struct*)_tmp292)->f2;_LL23D:
_tmp2B8=_tmp2B6;_tmp2B9=_tmp2B7;goto _LL23F;_LL23E: if(*((int*)_tmp292)!= 11)goto
_LL240;_tmp2B8=((struct Cyc_Absyn_FnCall_e_struct*)_tmp292)->f1;_tmp2B9=((struct
Cyc_Absyn_FnCall_e_struct*)_tmp292)->f2;_LL23F: {void*_tmp2DD=Cyc_Port_var();
void*_tmp2DE=Cyc_Port_gen_exp(env,_tmp2B8);struct Cyc_List_List*_tmp2DF=((struct
Cyc_List_List*(*)(void*(*f)(struct Cyc_Port_Cenv*,struct Cyc_Absyn_Exp*),struct Cyc_Port_Cenv*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_gen_exp,env,_tmp2B9);struct
Cyc_List_List*_tmp2E0=Cyc_List_map(Cyc_Port_new_var,_tmp2DF);Cyc_Port_unifies(
_tmp2DE,Cyc_Port_ptr_ct(Cyc_Port_fn_ct(_tmp2DD,_tmp2E0),Cyc_Port_var(),Cyc_Port_var(),
Cyc_Port_var(),Cyc_Port_nozterm_ct()));for(0;_tmp2DF != 0;(_tmp2DF=_tmp2DF->tl,
_tmp2E0=_tmp2E0->tl)){Cyc_Port_leq((void*)_tmp2DF->hd,(void*)((struct Cyc_List_List*)
_check_null(_tmp2E0))->hd);}return _tmp2DD;}_LL240: if(*((int*)_tmp292)!= 12)goto
_LL242;_LL241: {const char*_tmp4D0;void*_tmp4CF;(_tmp4CF=0,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4D0="throw",
_tag_dyneither(_tmp4D0,sizeof(char),6))),_tag_dyneither(_tmp4CF,sizeof(void*),0)));}
_LL242: if(*((int*)_tmp292)!= 13)goto _LL244;_tmp2BA=((struct Cyc_Absyn_NoInstantiate_e_struct*)
_tmp292)->f1;_LL243: return Cyc_Port_gen_exp(env,_tmp2BA);_LL244: if(*((int*)
_tmp292)!= 14)goto _LL246;_LL245: {const char*_tmp4D3;void*_tmp4D2;(_tmp4D2=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp4D3="instantiate",_tag_dyneither(_tmp4D3,sizeof(char),12))),_tag_dyneither(
_tmp4D2,sizeof(void*),0)));}_LL246: if(*((int*)_tmp292)!= 15)goto _LL248;_tmp2BB=(
void*)((struct Cyc_Absyn_Cast_e_struct*)_tmp292)->f1;_tmp2BC=((struct Cyc_Absyn_Cast_e_struct*)
_tmp292)->f2;_LL247: {void*_tmp2E5=Cyc_Port_type_to_ctype(env,_tmp2BB);void*
_tmp2E6=Cyc_Port_gen_exp(env,_tmp2BC);Cyc_Port_leq(_tmp2E5,_tmp2E6);return
_tmp2E6;}_LL248: if(*((int*)_tmp292)!= 16)goto _LL24A;_tmp2BD=((struct Cyc_Absyn_Address_e_struct*)
_tmp292)->f1;_LL249: {void*_tmp2E8;void*_tmp2E9;struct _tuple9 _tmp2E7=Cyc_Port_gen_lhs(
env,_tmp2BD);_tmp2E8=_tmp2E7.f1;_tmp2E9=_tmp2E7.f2;return Cyc_Port_ptr_ct(_tmp2E9,
_tmp2E8,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var());}_LL24A: if(*((int*)_tmp292)
!= 25)goto _LL24C;_tmp2BE=((struct Cyc_Absyn_Subscript_e_struct*)_tmp292)->f1;
_tmp2BF=((struct Cyc_Absyn_Subscript_e_struct*)_tmp292)->f2;_LL24B: {void*_tmp2EA=
Cyc_Port_var();Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2BF),Cyc_Port_arith_ct());{
void*_tmp2EB=Cyc_Port_gen_exp(env,_tmp2BE);Cyc_Port_leq(_tmp2EB,Cyc_Port_ptr_ct(
_tmp2EA,Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()));return
_tmp2EA;}}_LL24C: if(*((int*)_tmp292)!= 22)goto _LL24E;_tmp2C0=((struct Cyc_Absyn_Deref_e_struct*)
_tmp292)->f1;_LL24D: {void*_tmp2EC=Cyc_Port_var();Cyc_Port_leq(Cyc_Port_gen_exp(
env,_tmp2C0),Cyc_Port_ptr_ct(_tmp2EC,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),
Cyc_Port_var()));return _tmp2EC;}_LL24E: if(*((int*)_tmp292)!= 23)goto _LL250;
_tmp2C1=((struct Cyc_Absyn_AggrMember_e_struct*)_tmp292)->f1;_tmp2C2=((struct Cyc_Absyn_AggrMember_e_struct*)
_tmp292)->f2;_LL24F: {void*_tmp2ED=Cyc_Port_var();void*_tmp2EE=Cyc_Port_gen_exp(
env,_tmp2C1);{struct Cyc_Port_Cfield*_tmp4D6;struct Cyc_Port_Cfield*_tmp4D5[1];Cyc_Port_leq(
Cyc_Port_gen_exp(env,_tmp2C1),Cyc_Port_unknown_aggr_ct(((_tmp4D5[0]=((_tmp4D6=
_cycalloc(sizeof(*_tmp4D6)),((_tmp4D6->qual=Cyc_Port_var(),((_tmp4D6->f=_tmp2C2,((
_tmp4D6->type=_tmp2ED,_tmp4D6)))))))),((struct Cyc_List_List*(*)(struct
_dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp4D5,sizeof(struct Cyc_Port_Cfield*),
1))))));}return _tmp2ED;}_LL250: if(*((int*)_tmp292)!= 24)goto _LL252;_tmp2C3=((
struct Cyc_Absyn_AggrArrow_e_struct*)_tmp292)->f1;_tmp2C4=((struct Cyc_Absyn_AggrArrow_e_struct*)
_tmp292)->f2;_LL251: {void*_tmp2F1=Cyc_Port_var();void*_tmp2F2=Cyc_Port_gen_exp(
env,_tmp2C3);{struct Cyc_Port_Cfield*_tmp4D9;struct Cyc_Port_Cfield*_tmp4D8[1];Cyc_Port_leq(
Cyc_Port_gen_exp(env,_tmp2C3),Cyc_Port_ptr_ct(Cyc_Port_unknown_aggr_ct(((_tmp4D8[
0]=((_tmp4D9=_cycalloc(sizeof(*_tmp4D9)),((_tmp4D9->qual=Cyc_Port_var(),((
_tmp4D9->f=_tmp2C4,((_tmp4D9->type=_tmp2F1,_tmp4D9)))))))),((struct Cyc_List_List*(*)(
struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp4D8,sizeof(struct Cyc_Port_Cfield*),
1))))),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var()));}return
_tmp2F1;}_LL252: if(*((int*)_tmp292)!= 35)goto _LL254;_tmp2C5=((struct Cyc_Absyn_Malloc_e_struct*)
_tmp292)->f1;_tmp2C6=_tmp2C5.elt_type;_tmp2C7=_tmp2C5.num_elts;_LL253: Cyc_Port_leq(
Cyc_Port_gen_exp(env,_tmp2C7),Cyc_Port_arith_ct());{void*_tmp2F5=(unsigned int)
_tmp2C6?Cyc_Port_type_to_ctype(env,*_tmp2C6): Cyc_Port_var();return Cyc_Port_ptr_ct(
_tmp2F5,Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_heap_ct(),Cyc_Port_var());}
_LL254: if(*((int*)_tmp292)!= 36)goto _LL256;_tmp2C8=((struct Cyc_Absyn_Swap_e_struct*)
_tmp292)->f1;_tmp2C9=((struct Cyc_Absyn_Swap_e_struct*)_tmp292)->f2;_LL255: {
const char*_tmp4DC;void*_tmp4DB;(_tmp4DB=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4DC="Swap_e",_tag_dyneither(
_tmp4DC,sizeof(char),7))),_tag_dyneither(_tmp4DB,sizeof(void*),0)));}_LL256: if(*((
int*)_tmp292)!= 21)goto _LL258;_LL257: {const char*_tmp4DF;void*_tmp4DE;(_tmp4DE=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp4DF="Gen",_tag_dyneither(_tmp4DF,sizeof(char),4))),_tag_dyneither(_tmp4DE,
sizeof(void*),0)));}_LL258: if(*((int*)_tmp292)!= 17)goto _LL25A;_LL259: {const
char*_tmp4E2;void*_tmp4E1;(_tmp4E1=0,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4E2="New_e",_tag_dyneither(_tmp4E2,
sizeof(char),6))),_tag_dyneither(_tmp4E1,sizeof(void*),0)));}_LL25A: if(*((int*)
_tmp292)!= 26)goto _LL25C;_LL25B: {const char*_tmp4E5;void*_tmp4E4;(_tmp4E4=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp4E5="Tuple_e",_tag_dyneither(_tmp4E5,sizeof(char),8))),_tag_dyneither(
_tmp4E4,sizeof(void*),0)));}_LL25C: if(*((int*)_tmp292)!= 27)goto _LL25E;_LL25D: {
const char*_tmp4E8;void*_tmp4E7;(_tmp4E7=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4E8="CompoundLit_e",
_tag_dyneither(_tmp4E8,sizeof(char),14))),_tag_dyneither(_tmp4E7,sizeof(void*),0)));}
_LL25E: if(*((int*)_tmp292)!= 28)goto _LL260;_LL25F: {const char*_tmp4EB;void*
_tmp4EA;(_tmp4EA=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp4EB="Array_e",_tag_dyneither(_tmp4EB,sizeof(char),8))),_tag_dyneither(
_tmp4EA,sizeof(void*),0)));}_LL260: if(*((int*)_tmp292)!= 29)goto _LL262;_LL261: {
const char*_tmp4EE;void*_tmp4ED;(_tmp4ED=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4EE="Comprehension_e",
_tag_dyneither(_tmp4EE,sizeof(char),16))),_tag_dyneither(_tmp4ED,sizeof(void*),0)));}
_LL262: if(*((int*)_tmp292)!= 30)goto _LL264;_LL263: {const char*_tmp4F1;void*
_tmp4F0;(_tmp4F0=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp4F1="Aggregate_e",_tag_dyneither(_tmp4F1,sizeof(char),12))),_tag_dyneither(
_tmp4F0,sizeof(void*),0)));}_LL264: if(*((int*)_tmp292)!= 31)goto _LL266;_LL265: {
const char*_tmp4F4;void*_tmp4F3;(_tmp4F3=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4F4="AnonStruct_e",
_tag_dyneither(_tmp4F4,sizeof(char),13))),_tag_dyneither(_tmp4F3,sizeof(void*),0)));}
_LL266: if(*((int*)_tmp292)!= 32)goto _LL268;_LL267: {const char*_tmp4F7;void*
_tmp4F6;(_tmp4F6=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp4F7="Datatype_e",_tag_dyneither(_tmp4F7,sizeof(char),11))),_tag_dyneither(
_tmp4F6,sizeof(void*),0)));}_LL268: if(*((int*)_tmp292)!= 37)goto _LL26A;_LL269: {
const char*_tmp4FA;void*_tmp4F9;(_tmp4F9=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4FA="UnresolvedMem_e",
_tag_dyneither(_tmp4FA,sizeof(char),16))),_tag_dyneither(_tmp4F9,sizeof(void*),0)));}
_LL26A: if(*((int*)_tmp292)!= 38)goto _LL26C;_tmp2CA=((struct Cyc_Absyn_StmtExp_e_struct*)
_tmp292)->f1;_LL26B: {const char*_tmp4FD;void*_tmp4FC;(_tmp4FC=0,((int(*)(struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4FD="StmtExp_e",
_tag_dyneither(_tmp4FD,sizeof(char),10))),_tag_dyneither(_tmp4FC,sizeof(void*),0)));}
_LL26C: if(*((int*)_tmp292)!= 40)goto _LL26E;_LL26D: {const char*_tmp500;void*
_tmp4FF;(_tmp4FF=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp500="Valueof_e",_tag_dyneither(_tmp500,sizeof(char),10))),_tag_dyneither(
_tmp4FF,sizeof(void*),0)));}_LL26E: if(*((int*)_tmp292)!= 39)goto _LL20F;_LL26F: {
const char*_tmp503;void*_tmp502;(_tmp502=0,((int(*)(struct _dyneither_ptr fmt,
struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp503="Tagcheck_e",_tag_dyneither(
_tmp503,sizeof(char),11))),_tag_dyneither(_tmp502,sizeof(void*),0)));}_LL20F:;}
static void Cyc_Port_gen_stmt(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Stmt*s);
static void Cyc_Port_gen_stmt(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Stmt*s){void*
_tmp312=s->r;struct Cyc_Absyn_Exp*_tmp313;struct Cyc_Absyn_Stmt*_tmp314;struct Cyc_Absyn_Stmt*
_tmp315;struct Cyc_Absyn_Exp*_tmp316;struct Cyc_Absyn_Exp*_tmp317;struct Cyc_Absyn_Stmt*
_tmp318;struct Cyc_Absyn_Stmt*_tmp319;struct _tuple8 _tmp31A;struct Cyc_Absyn_Exp*
_tmp31B;struct Cyc_Absyn_Stmt*_tmp31C;struct Cyc_Absyn_Exp*_tmp31D;struct _tuple8
_tmp31E;struct Cyc_Absyn_Exp*_tmp31F;struct _tuple8 _tmp320;struct Cyc_Absyn_Exp*
_tmp321;struct Cyc_Absyn_Stmt*_tmp322;struct Cyc_Absyn_Exp*_tmp323;struct Cyc_List_List*
_tmp324;struct Cyc_Absyn_Decl*_tmp325;struct Cyc_Absyn_Stmt*_tmp326;struct Cyc_Absyn_Stmt*
_tmp327;struct Cyc_Absyn_Stmt*_tmp328;struct _tuple8 _tmp329;struct Cyc_Absyn_Exp*
_tmp32A;_LL271: if((int)_tmp312 != 0)goto _LL273;_LL272: return;_LL273: if(_tmp312 <= (
void*)1)goto _LL275;if(*((int*)_tmp312)!= 0)goto _LL275;_tmp313=((struct Cyc_Absyn_Exp_s_struct*)
_tmp312)->f1;_LL274: Cyc_Port_gen_exp(env,_tmp313);return;_LL275: if(_tmp312 <= (
void*)1)goto _LL277;if(*((int*)_tmp312)!= 1)goto _LL277;_tmp314=((struct Cyc_Absyn_Seq_s_struct*)
_tmp312)->f1;_tmp315=((struct Cyc_Absyn_Seq_s_struct*)_tmp312)->f2;_LL276: Cyc_Port_gen_stmt(
env,_tmp314);Cyc_Port_gen_stmt(env,_tmp315);return;_LL277: if(_tmp312 <= (void*)1)
goto _LL279;if(*((int*)_tmp312)!= 2)goto _LL279;_tmp316=((struct Cyc_Absyn_Return_s_struct*)
_tmp312)->f1;_LL278: {void*_tmp32B=Cyc_Port_lookup_return_type(env);void*_tmp32C=(
unsigned int)_tmp316?Cyc_Port_gen_exp(env,(struct Cyc_Absyn_Exp*)_tmp316): Cyc_Port_void_ct();
Cyc_Port_leq(_tmp32C,_tmp32B);return;}_LL279: if(_tmp312 <= (void*)1)goto _LL27B;
if(*((int*)_tmp312)!= 3)goto _LL27B;_tmp317=((struct Cyc_Absyn_IfThenElse_s_struct*)
_tmp312)->f1;_tmp318=((struct Cyc_Absyn_IfThenElse_s_struct*)_tmp312)->f2;_tmp319=((
struct Cyc_Absyn_IfThenElse_s_struct*)_tmp312)->f3;_LL27A: Cyc_Port_leq(Cyc_Port_gen_exp(
env,_tmp317),Cyc_Port_arith_ct());Cyc_Port_gen_stmt(env,_tmp318);Cyc_Port_gen_stmt(
env,_tmp319);return;_LL27B: if(_tmp312 <= (void*)1)goto _LL27D;if(*((int*)_tmp312)
!= 4)goto _LL27D;_tmp31A=((struct Cyc_Absyn_While_s_struct*)_tmp312)->f1;_tmp31B=
_tmp31A.f1;_tmp31C=((struct Cyc_Absyn_While_s_struct*)_tmp312)->f2;_LL27C: Cyc_Port_leq(
Cyc_Port_gen_exp(env,_tmp31B),Cyc_Port_arith_ct());Cyc_Port_gen_stmt(env,_tmp31C);
return;_LL27D: if(_tmp312 <= (void*)1)goto _LL27F;if(*((int*)_tmp312)!= 5)goto
_LL27F;_LL27E: goto _LL280;_LL27F: if(_tmp312 <= (void*)1)goto _LL281;if(*((int*)
_tmp312)!= 6)goto _LL281;_LL280: goto _LL282;_LL281: if(_tmp312 <= (void*)1)goto
_LL283;if(*((int*)_tmp312)!= 7)goto _LL283;_LL282: return;_LL283: if(_tmp312 <= (
void*)1)goto _LL285;if(*((int*)_tmp312)!= 8)goto _LL285;_tmp31D=((struct Cyc_Absyn_For_s_struct*)
_tmp312)->f1;_tmp31E=((struct Cyc_Absyn_For_s_struct*)_tmp312)->f2;_tmp31F=
_tmp31E.f1;_tmp320=((struct Cyc_Absyn_For_s_struct*)_tmp312)->f3;_tmp321=_tmp320.f1;
_tmp322=((struct Cyc_Absyn_For_s_struct*)_tmp312)->f4;_LL284: Cyc_Port_gen_exp(env,
_tmp31D);Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp31F),Cyc_Port_arith_ct());Cyc_Port_gen_exp(
env,_tmp321);Cyc_Port_gen_stmt(env,_tmp322);return;_LL285: if(_tmp312 <= (void*)1)
goto _LL287;if(*((int*)_tmp312)!= 9)goto _LL287;_tmp323=((struct Cyc_Absyn_Switch_s_struct*)
_tmp312)->f1;_tmp324=((struct Cyc_Absyn_Switch_s_struct*)_tmp312)->f2;_LL286: Cyc_Port_leq(
Cyc_Port_gen_exp(env,_tmp323),Cyc_Port_arith_ct());for(0;(unsigned int)_tmp324;
_tmp324=_tmp324->tl){Cyc_Port_gen_stmt(env,((struct Cyc_Absyn_Switch_clause*)
_tmp324->hd)->body);}return;_LL287: if(_tmp312 <= (void*)1)goto _LL289;if(*((int*)
_tmp312)!= 10)goto _LL289;_LL288: {const char*_tmp506;void*_tmp505;(_tmp505=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp506="fallthru",_tag_dyneither(_tmp506,sizeof(char),9))),_tag_dyneither(
_tmp505,sizeof(void*),0)));}_LL289: if(_tmp312 <= (void*)1)goto _LL28B;if(*((int*)
_tmp312)!= 11)goto _LL28B;_tmp325=((struct Cyc_Absyn_Decl_s_struct*)_tmp312)->f1;
_tmp326=((struct Cyc_Absyn_Decl_s_struct*)_tmp312)->f2;_LL28A: env=Cyc_Port_gen_localdecl(
env,_tmp325);Cyc_Port_gen_stmt(env,_tmp326);return;_LL28B: if(_tmp312 <= (void*)1)
goto _LL28D;if(*((int*)_tmp312)!= 12)goto _LL28D;_tmp327=((struct Cyc_Absyn_Label_s_struct*)
_tmp312)->f2;_LL28C: Cyc_Port_gen_stmt(env,_tmp327);return;_LL28D: if(_tmp312 <= (
void*)1)goto _LL28F;if(*((int*)_tmp312)!= 13)goto _LL28F;_tmp328=((struct Cyc_Absyn_Do_s_struct*)
_tmp312)->f1;_tmp329=((struct Cyc_Absyn_Do_s_struct*)_tmp312)->f2;_tmp32A=_tmp329.f1;
_LL28E: Cyc_Port_gen_stmt(env,_tmp328);Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp32A),
Cyc_Port_arith_ct());return;_LL28F: if(_tmp312 <= (void*)1)goto _LL291;if(*((int*)
_tmp312)!= 14)goto _LL291;_LL290: {const char*_tmp509;void*_tmp508;(_tmp508=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp509="try/catch",_tag_dyneither(_tmp509,sizeof(char),10))),_tag_dyneither(
_tmp508,sizeof(void*),0)));}_LL291: if(_tmp312 <= (void*)1)goto _LL270;if(*((int*)
_tmp312)!= 15)goto _LL270;_LL292: {const char*_tmp50C;void*_tmp50B;(_tmp50B=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp50C="reset region",_tag_dyneither(_tmp50C,sizeof(char),13))),_tag_dyneither(
_tmp50B,sizeof(void*),0)));}_LL270:;}struct _tuple14{struct Cyc_List_List*f1;
struct Cyc_Absyn_Exp*f2;};static void*Cyc_Port_gen_initializer(struct Cyc_Port_Cenv*
env,void*t,struct Cyc_Absyn_Exp*e);static void*Cyc_Port_gen_initializer(struct Cyc_Port_Cenv*
env,void*t,struct Cyc_Absyn_Exp*e){void*_tmp333=e->r;struct Cyc_List_List*_tmp334;
struct Cyc_List_List*_tmp335;struct Cyc_List_List*_tmp336;union Cyc_Absyn_Cnst
_tmp337;struct _dyneither_ptr _tmp338;_LL294: if(*((int*)_tmp333)!= 37)goto _LL296;
_tmp334=((struct Cyc_Absyn_UnresolvedMem_e_struct*)_tmp333)->f2;_LL295: _tmp335=
_tmp334;goto _LL297;_LL296: if(*((int*)_tmp333)!= 28)goto _LL298;_tmp335=((struct
Cyc_Absyn_Array_e_struct*)_tmp333)->f1;_LL297: _tmp336=_tmp335;goto _LL299;_LL298:
if(*((int*)_tmp333)!= 27)goto _LL29A;_tmp336=((struct Cyc_Absyn_CompoundLit_e_struct*)
_tmp333)->f2;_LL299: LOOP: {void*_tmp339=t;struct _tuple0*_tmp33A;struct Cyc_Absyn_ArrayInfo
_tmp33B;void*_tmp33C;union Cyc_Absyn_Constraint*_tmp33D;struct Cyc_Position_Segment*
_tmp33E;struct Cyc_Absyn_AggrInfo _tmp33F;union Cyc_Absyn_AggrInfoU _tmp340;struct
_tuple2 _tmp341;void*_tmp342;struct _tuple0*_tmp343;struct Cyc_Absyn_AggrInfo
_tmp344;union Cyc_Absyn_AggrInfoU _tmp345;struct Cyc_Absyn_Aggrdecl**_tmp346;struct
Cyc_Absyn_Aggrdecl*_tmp347;_LL29F: if(_tmp339 <= (void*)4)goto _LL2A7;if(*((int*)
_tmp339)!= 16)goto _LL2A1;_tmp33A=((struct Cyc_Absyn_TypedefType_struct*)_tmp339)->f1;
_LL2A0:(*_tmp33A).f1=Cyc_Absyn_Loc_n;t=Cyc_Port_lookup_typedef(env,_tmp33A);goto
LOOP;_LL2A1: if(*((int*)_tmp339)!= 7)goto _LL2A3;_tmp33B=((struct Cyc_Absyn_ArrayType_struct*)
_tmp339)->f1;_tmp33C=_tmp33B.elt_type;_tmp33D=_tmp33B.zero_term;_tmp33E=_tmp33B.zt_loc;
_LL2A2: {void*_tmp348=Cyc_Port_var();void*_tmp349=Cyc_Port_var();void*_tmp34A=
Cyc_Port_var();void*_tmp34B=Cyc_Port_type_to_ctype(env,_tmp33C);for(0;_tmp336 != 
0;_tmp336=_tmp336->tl){struct Cyc_List_List*_tmp34D;struct Cyc_Absyn_Exp*_tmp34E;
struct _tuple14 _tmp34C=*((struct _tuple14*)_tmp336->hd);_tmp34D=_tmp34C.f1;_tmp34E=
_tmp34C.f2;if((unsigned int)_tmp34D){const char*_tmp50F;void*_tmp50E;(_tmp50E=0,((
int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp50F="designators in initializers",_tag_dyneither(_tmp50F,sizeof(char),28))),
_tag_dyneither(_tmp50E,sizeof(void*),0)));}{void*_tmp351=Cyc_Port_gen_initializer(
env,_tmp33C,_tmp34E);Cyc_Port_leq(_tmp351,_tmp348);}}return Cyc_Port_array_ct(
_tmp348,_tmp349,_tmp34A);}_LL2A3: if(*((int*)_tmp339)!= 10)goto _LL2A5;_tmp33F=((
struct Cyc_Absyn_AggrType_struct*)_tmp339)->f1;_tmp340=_tmp33F.aggr_info;if((
_tmp340.UnknownAggr).tag != 1)goto _LL2A5;_tmp341=(struct _tuple2)(_tmp340.UnknownAggr).val;
_tmp342=_tmp341.f1;if((int)_tmp342 != 0)goto _LL2A5;_tmp343=_tmp341.f2;_LL2A4:(*
_tmp343).f1=Cyc_Absyn_Loc_n;{struct Cyc_Absyn_Aggrdecl*_tmp353;struct _tuple10
_tmp352=*Cyc_Port_lookup_struct_decl(env,_tmp343);_tmp353=_tmp352.f1;if((struct
Cyc_Absyn_Aggrdecl*)_tmp353 == 0){const char*_tmp512;void*_tmp511;(_tmp511=0,((int(*)(
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp512="struct is not yet defined",
_tag_dyneither(_tmp512,sizeof(char),26))),_tag_dyneither(_tmp511,sizeof(void*),0)));}
_tmp347=_tmp353;goto _LL2A6;}_LL2A5: if(*((int*)_tmp339)!= 10)goto _LL2A7;_tmp344=((
struct Cyc_Absyn_AggrType_struct*)_tmp339)->f1;_tmp345=_tmp344.aggr_info;if((
_tmp345.KnownAggr).tag != 2)goto _LL2A7;_tmp346=(struct Cyc_Absyn_Aggrdecl**)(
_tmp345.KnownAggr).val;_tmp347=*_tmp346;_LL2A6: {struct Cyc_List_List*_tmp356=((
struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp347->impl))->fields;for(0;_tmp336
!= 0;_tmp336=_tmp336->tl){struct Cyc_List_List*_tmp358;struct Cyc_Absyn_Exp*
_tmp359;struct _tuple14 _tmp357=*((struct _tuple14*)_tmp336->hd);_tmp358=_tmp357.f1;
_tmp359=_tmp357.f2;if((unsigned int)_tmp358){const char*_tmp515;void*_tmp514;(
_tmp514=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp515="designators in initializers",_tag_dyneither(_tmp515,sizeof(char),28))),
_tag_dyneither(_tmp514,sizeof(void*),0)));}{struct Cyc_Absyn_Aggrfield*_tmp35C=(
struct Cyc_Absyn_Aggrfield*)((struct Cyc_List_List*)_check_null(_tmp356))->hd;
_tmp356=_tmp356->tl;{void*_tmp35D=Cyc_Port_gen_initializer(env,_tmp35C->type,
_tmp359);;}}}return Cyc_Port_type_to_ctype(env,t);}_LL2A7:;_LL2A8: {const char*
_tmp518;void*_tmp517;(_tmp517=0,((int(*)(struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp518="bad type for aggregate initializer",
_tag_dyneither(_tmp518,sizeof(char),35))),_tag_dyneither(_tmp517,sizeof(void*),0)));}
_LL29E:;}_LL29A: if(*((int*)_tmp333)!= 0)goto _LL29C;_tmp337=((struct Cyc_Absyn_Const_e_struct*)
_tmp333)->f1;if((_tmp337.String_c).tag != 7)goto _LL29C;_tmp338=(struct
_dyneither_ptr)(_tmp337.String_c).val;_LL29B: LOOP2: {void*_tmp360=t;struct
_tuple0*_tmp361;_LL2AA: if(_tmp360 <= (void*)4)goto _LL2AE;if(*((int*)_tmp360)!= 16)
goto _LL2AC;_tmp361=((struct Cyc_Absyn_TypedefType_struct*)_tmp360)->f1;_LL2AB:(*
_tmp361).f1=Cyc_Absyn_Loc_n;t=Cyc_Port_lookup_typedef(env,_tmp361);goto LOOP2;
_LL2AC: if(*((int*)_tmp360)!= 7)goto _LL2AE;_LL2AD: return Cyc_Port_array_ct(Cyc_Port_arith_ct(),
Cyc_Port_const_ct(),Cyc_Port_zterm_ct());_LL2AE:;_LL2AF: return Cyc_Port_gen_exp(
env,e);_LL2A9:;}_LL29C:;_LL29D: return Cyc_Port_gen_exp(env,e);_LL293:;}static
struct Cyc_Port_Cenv*Cyc_Port_gen_localdecl(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Decl*
d);static struct Cyc_Port_Cenv*Cyc_Port_gen_localdecl(struct Cyc_Port_Cenv*env,
struct Cyc_Absyn_Decl*d){void*_tmp362=d->r;struct Cyc_Absyn_Vardecl*_tmp363;_LL2B1:
if(_tmp362 <= (void*)2)goto _LL2B3;if(*((int*)_tmp362)!= 0)goto _LL2B3;_tmp363=((
struct Cyc_Absyn_Var_d_struct*)_tmp362)->f1;_LL2B2: {void*_tmp364=Cyc_Port_var();
void*q;if((env->gcenv)->porting){q=Cyc_Port_var();Cyc_Port_register_const_cvar(
env,q,(_tmp363->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),(
_tmp363->tq).loc);}else{q=(_tmp363->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}(*
_tmp363->name).f1=Cyc_Absyn_Loc_n;env=Cyc_Port_add_var(env,_tmp363->name,_tmp363->type,
q,_tmp364);Cyc_Port_unifies(_tmp364,Cyc_Port_type_to_ctype(env,_tmp363->type));
if((unsigned int)_tmp363->initializer){struct Cyc_Absyn_Exp*e=(struct Cyc_Absyn_Exp*)
_check_null(_tmp363->initializer);void*t2=Cyc_Port_gen_initializer(env,_tmp363->type,
e);Cyc_Port_leq(t2,_tmp364);}return env;}_LL2B3:;_LL2B4: {const char*_tmp51B;void*
_tmp51A;(_tmp51A=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp51B="non-local decl that isn't a variable",_tag_dyneither(_tmp51B,sizeof(
char),37))),_tag_dyneither(_tmp51A,sizeof(void*),0)));}_LL2B0:;}struct _tuple15{
struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};static struct _tuple7*
Cyc_Port_make_targ(struct _tuple15*arg);static struct _tuple7*Cyc_Port_make_targ(
struct _tuple15*arg){struct _dyneither_ptr*_tmp368;struct Cyc_Absyn_Tqual _tmp369;
void*_tmp36A;struct _tuple15 _tmp367=*arg;_tmp368=_tmp367.f1;_tmp369=_tmp367.f2;
_tmp36A=_tmp367.f3;{struct _tuple7*_tmp51C;return(_tmp51C=_cycalloc(sizeof(*
_tmp51C)),((_tmp51C->f1=0,((_tmp51C->f2=_tmp369,((_tmp51C->f3=_tmp36A,_tmp51C)))))));}}
static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecls(struct Cyc_Port_Cenv*env,struct
Cyc_List_List*ds);struct _tuple16{struct _dyneither_ptr*f1;void*f2;void*f3;void*f4;
};static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecl(struct Cyc_Port_Cenv*env,struct
Cyc_Absyn_Decl*d);static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecl(struct Cyc_Port_Cenv*
env,struct Cyc_Absyn_Decl*d){void*_tmp36C=d->r;struct Cyc_Absyn_Vardecl*_tmp36D;
struct Cyc_Absyn_Fndecl*_tmp36E;struct Cyc_Absyn_Typedefdecl*_tmp36F;struct Cyc_Absyn_Aggrdecl*
_tmp370;struct Cyc_Absyn_Enumdecl*_tmp371;_LL2B6: if((int)_tmp36C != 0)goto _LL2B8;
_LL2B7:(env->gcenv)->porting=1;return env;_LL2B8: if((int)_tmp36C != 1)goto _LL2BA;
_LL2B9:(env->gcenv)->porting=0;return env;_LL2BA: if(_tmp36C <= (void*)2)goto _LL2C4;
if(*((int*)_tmp36C)!= 0)goto _LL2BC;_tmp36D=((struct Cyc_Absyn_Var_d_struct*)
_tmp36C)->f1;_LL2BB:(*_tmp36D->name).f1=Cyc_Absyn_Loc_n;if(Cyc_Port_declared_var(
env,_tmp36D->name)){void*_tmp373;void*_tmp374;struct _tuple9 _tmp372=Cyc_Port_lookup_var(
env,_tmp36D->name);_tmp373=_tmp372.f1;_tmp374=_tmp372.f2;{void*q2;if((env->gcenv)->porting
 && !Cyc_Port_isfn(env,_tmp36D->name)){q2=Cyc_Port_var();Cyc_Port_register_const_cvar(
env,q2,(_tmp36D->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),(
_tmp36D->tq).loc);}else{q2=(_tmp36D->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}
Cyc_Port_unifies(_tmp373,q2);Cyc_Port_unifies(_tmp374,Cyc_Port_type_to_ctype(env,
_tmp36D->type));if((unsigned int)_tmp36D->initializer){struct Cyc_Absyn_Exp*e=(
struct Cyc_Absyn_Exp*)_check_null(_tmp36D->initializer);Cyc_Port_leq(Cyc_Port_gen_initializer(
env,_tmp36D->type,e),_tmp374);}return env;}}else{return Cyc_Port_gen_localdecl(env,
d);}_LL2BC: if(*((int*)_tmp36C)!= 1)goto _LL2BE;_tmp36E=((struct Cyc_Absyn_Fn_d_struct*)
_tmp36C)->f1;_LL2BD:(*_tmp36E->name).f1=Cyc_Absyn_Loc_n;{struct _tuple12*
predeclared=0;if(Cyc_Port_declared_var(env,_tmp36E->name))predeclared=(struct
_tuple12*)Cyc_Port_lookup_full_var(env,_tmp36E->name);{void*_tmp375=_tmp36E->ret_type;
struct Cyc_List_List*_tmp376=_tmp36E->args;struct Cyc_List_List*_tmp377=((struct
Cyc_List_List*(*)(struct _tuple7*(*f)(struct _tuple15*),struct Cyc_List_List*x))Cyc_List_map)(
Cyc_Port_make_targ,_tmp376);struct Cyc_Absyn_FnType_struct _tmp522;struct Cyc_Absyn_FnInfo
_tmp521;struct Cyc_Absyn_FnType_struct*_tmp520;struct Cyc_Absyn_FnType_struct*
_tmp378=(_tmp520=_cycalloc(sizeof(*_tmp520)),((_tmp520[0]=((_tmp522.tag=8,((
_tmp522.f1=((_tmp521.tvars=0,((_tmp521.effect=0,((_tmp521.ret_typ=_tmp375,((
_tmp521.args=_tmp377,((_tmp521.c_varargs=0,((_tmp521.cyc_varargs=0,((_tmp521.rgn_po=
0,((_tmp521.attributes=0,_tmp521)))))))))))))))),_tmp522)))),_tmp520)));struct
Cyc_Port_Cenv*_tmp379=Cyc_Port_set_cpos(env,(void*)2);void*_tmp37A=Cyc_Port_type_to_ctype(
_tmp379,_tmp375);struct Cyc_List_List*c_args=0;struct Cyc_List_List*c_arg_types=0;{
struct Cyc_List_List*_tmp37B=_tmp376;for(0;(unsigned int)_tmp37B;_tmp37B=_tmp37B->tl){
struct _dyneither_ptr*_tmp37D;struct Cyc_Absyn_Tqual _tmp37E;void*_tmp37F;struct
_tuple15 _tmp37C=*((struct _tuple15*)_tmp37B->hd);_tmp37D=_tmp37C.f1;_tmp37E=
_tmp37C.f2;_tmp37F=_tmp37C.f3;{void*_tmp380=Cyc_Port_type_to_ctype(_tmp379,
_tmp37F);void*tqv;if((env->gcenv)->porting){tqv=Cyc_Port_var();Cyc_Port_register_const_cvar(
env,tqv,_tmp37E.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),_tmp37E.loc);}
else{tqv=_tmp37E.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}{struct
_tuple16*_tmp525;struct Cyc_List_List*_tmp524;c_args=((_tmp524=_cycalloc(sizeof(*
_tmp524)),((_tmp524->hd=((_tmp525=_cycalloc(sizeof(*_tmp525)),((_tmp525->f1=
_tmp37D,((_tmp525->f2=_tmp37F,((_tmp525->f3=tqv,((_tmp525->f4=_tmp380,_tmp525)))))))))),((
_tmp524->tl=c_args,_tmp524))))));}{struct Cyc_List_List*_tmp526;c_arg_types=((
_tmp526=_cycalloc(sizeof(*_tmp526)),((_tmp526->hd=(void*)_tmp380,((_tmp526->tl=
c_arg_types,_tmp526))))));}}}}c_args=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_imp_rev)(c_args);c_arg_types=Cyc_List_imp_rev(c_arg_types);{void*
_tmp384=Cyc_Port_fn_ct(_tmp37A,c_arg_types);(*_tmp36E->name).f1=Cyc_Absyn_Loc_n;
_tmp379=Cyc_Port_add_var(_tmp379,_tmp36E->name,(void*)_tmp378,Cyc_Port_const_ct(),
_tmp384);Cyc_Port_add_return_type(_tmp379,_tmp37A);{struct Cyc_List_List*_tmp385=
c_args;for(0;(unsigned int)_tmp385;_tmp385=_tmp385->tl){struct _dyneither_ptr*
_tmp387;void*_tmp388;void*_tmp389;void*_tmp38A;struct _tuple16 _tmp386=*((struct
_tuple16*)_tmp385->hd);_tmp387=_tmp386.f1;_tmp388=_tmp386.f2;_tmp389=_tmp386.f3;
_tmp38A=_tmp386.f4;{struct _tuple0*_tmp527;_tmp379=Cyc_Port_add_var(_tmp379,((
_tmp527=_cycalloc(sizeof(*_tmp527)),((_tmp527->f1=Cyc_Absyn_Loc_n,((_tmp527->f2=
_tmp387,_tmp527)))))),_tmp388,_tmp389,_tmp38A);}}}Cyc_Port_gen_stmt(_tmp379,
_tmp36E->body);Cyc_Port_generalize(0,_tmp384);{struct Cyc_Dict_Dict counts=((
struct Cyc_Dict_Dict(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))
Cyc_Dict_empty)(Cyc_strptrcmp);Cyc_Port_region_counts(& counts,_tmp384);Cyc_Port_register_rgns(
env,counts,1,(void*)_tmp378,_tmp384);env=Cyc_Port_add_var(_tmp379,_tmp36E->name,(
void*)_tmp378,Cyc_Port_const_ct(),_tmp384);if((unsigned int)predeclared){void*
_tmp38D;struct _tuple9*_tmp38E;struct _tuple9 _tmp38F;void*_tmp390;void*_tmp391;
struct _tuple12 _tmp38C=*predeclared;_tmp38D=_tmp38C.f1;_tmp38E=_tmp38C.f2;_tmp38F=*
_tmp38E;_tmp390=_tmp38F.f1;_tmp391=_tmp38F.f2;Cyc_Port_unifies(_tmp390,Cyc_Port_const_ct());
Cyc_Port_unifies(_tmp391,Cyc_Port_instantiate(_tmp384));Cyc_Port_register_rgns(
env,counts,1,_tmp38D,_tmp384);}return env;}}}}_LL2BE: if(*((int*)_tmp36C)!= 9)goto
_LL2C0;_tmp36F=((struct Cyc_Absyn_Typedef_d_struct*)_tmp36C)->f1;_LL2BF: {void*
_tmp395=(void*)((struct Cyc_Core_Opt*)_check_null(_tmp36F->defn))->v;void*_tmp396=
Cyc_Port_type_to_ctype(env,_tmp395);(*_tmp36F->name).f1=Cyc_Absyn_Loc_n;Cyc_Port_add_typedef(
env,_tmp36F->name,_tmp395,_tmp396);return env;}_LL2C0: if(*((int*)_tmp36C)!= 6)
goto _LL2C2;_tmp370=((struct Cyc_Absyn_Aggr_d_struct*)_tmp36C)->f1;_LL2C1: {struct
_tuple0*_tmp397=_tmp370->name;(*_tmp370->name).f1=Cyc_Absyn_Loc_n;{struct
_tuple10*previous;struct Cyc_List_List*curr=0;{void*_tmp398=_tmp370->kind;_LL2C7:
if((int)_tmp398 != 0)goto _LL2C9;_LL2C8: previous=Cyc_Port_lookup_struct_decl(env,
_tmp397);goto _LL2C6;_LL2C9: if((int)_tmp398 != 1)goto _LL2C6;_LL2CA: previous=Cyc_Port_lookup_union_decl(
env,_tmp397);goto _LL2C6;_LL2C6:;}if((unsigned int)_tmp370->impl){struct Cyc_List_List*
cf=(*previous).f2;{struct Cyc_List_List*_tmp399=((struct Cyc_Absyn_AggrdeclImpl*)
_check_null(_tmp370->impl))->fields;for(0;(unsigned int)_tmp399;_tmp399=_tmp399->tl){
struct Cyc_Absyn_Aggrfield*_tmp39A=(struct Cyc_Absyn_Aggrfield*)_tmp399->hd;void*
qv;if((env->gcenv)->porting){qv=Cyc_Port_var();Cyc_Port_register_const_cvar(env,
qv,(_tmp39A->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),(_tmp39A->tq).loc);}
else{qv=(_tmp39A->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}{
void*_tmp39B=Cyc_Port_type_to_ctype(env,_tmp39A->type);if(cf != 0){void*_tmp39D;
void*_tmp39E;struct Cyc_Port_Cfield _tmp39C=*((struct Cyc_Port_Cfield*)cf->hd);
_tmp39D=_tmp39C.qual;_tmp39E=_tmp39C.type;cf=cf->tl;Cyc_Port_unifies(qv,_tmp39D);
Cyc_Port_unifies(_tmp39B,_tmp39E);}{struct Cyc_Port_Cfield*_tmp52A;struct Cyc_List_List*
_tmp529;curr=((_tmp529=_cycalloc(sizeof(*_tmp529)),((_tmp529->hd=((_tmp52A=
_cycalloc(sizeof(*_tmp52A)),((_tmp52A->qual=qv,((_tmp52A->f=_tmp39A->name,((
_tmp52A->type=_tmp39B,_tmp52A)))))))),((_tmp529->tl=curr,_tmp529))))));}}}}curr=((
struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(curr);if((*
previous).f2 == 0)(*previous).f2=curr;}return env;}}_LL2C2: if(*((int*)_tmp36C)!= 8)
goto _LL2C4;_tmp371=((struct Cyc_Absyn_Enum_d_struct*)_tmp36C)->f1;_LL2C3:(*
_tmp371->name).f1=Cyc_Absyn_Loc_n;if((unsigned int)_tmp371->fields){struct Cyc_List_List*
_tmp3A1=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp371->fields))->v;
for(0;(unsigned int)_tmp3A1;_tmp3A1=_tmp3A1->tl){(*((struct Cyc_Absyn_Enumfield*)
_tmp3A1->hd)->name).f1=Cyc_Absyn_Loc_n;{struct Cyc_Absyn_EnumType_struct _tmp52D;
struct Cyc_Absyn_EnumType_struct*_tmp52C;env=Cyc_Port_add_var(env,((struct Cyc_Absyn_Enumfield*)
_tmp3A1->hd)->name,(void*)((_tmp52C=_cycalloc(sizeof(*_tmp52C)),((_tmp52C[0]=((
_tmp52D.tag=12,((_tmp52D.f1=_tmp371->name,((_tmp52D.f2=(struct Cyc_Absyn_Enumdecl*)
_tmp371,_tmp52D)))))),_tmp52C)))),Cyc_Port_const_ct(),Cyc_Port_arith_ct());}}}
return env;_LL2C4:;_LL2C5: if((env->gcenv)->porting){const char*_tmp530;void*
_tmp52F;(_tmp52F=0,Cyc_fprintf(Cyc_stderr,((_tmp530="Warning: Cyclone declaration found in code to be ported -- skipping.",
_tag_dyneither(_tmp530,sizeof(char),69))),_tag_dyneither(_tmp52F,sizeof(void*),0)));}
return env;_LL2B5:;}static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecls(struct Cyc_Port_Cenv*
env,struct Cyc_List_List*ds);static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecls(
struct Cyc_Port_Cenv*env,struct Cyc_List_List*ds){for(0;(unsigned int)ds;ds=ds->tl){
env=Cyc_Port_gen_topdecl(env,(struct Cyc_Absyn_Decl*)ds->hd);}return env;}static
struct Cyc_List_List*Cyc_Port_gen_edits(struct Cyc_List_List*ds);static struct Cyc_List_List*
Cyc_Port_gen_edits(struct Cyc_List_List*ds){struct Cyc_Port_Cenv*env=Cyc_Port_gen_topdecls(
Cyc_Port_initial_cenv(),ds);struct Cyc_List_List*_tmp3A6=(env->gcenv)->pointer_edits;
struct Cyc_List_List*_tmp3A7=(env->gcenv)->qualifier_edits;struct Cyc_List_List*
_tmp3A8=(env->gcenv)->zeroterm_edits;struct Cyc_List_List*_tmp3A9=(env->gcenv)->edits;{
struct Cyc_List_List*_tmp3AA=_tmp3A6;for(0;(unsigned int)_tmp3AA;_tmp3AA=_tmp3AA->tl){
void*_tmp3AC;void*_tmp3AD;struct Cyc_Position_Segment*_tmp3AE;struct _tuple13
_tmp3AB=*((struct _tuple13*)_tmp3AA->hd);_tmp3AC=_tmp3AB.f1;_tmp3AD=_tmp3AB.f2;
_tmp3AE=_tmp3AB.f3;Cyc_Port_unifies(_tmp3AC,_tmp3AD);}}{struct Cyc_List_List*
_tmp3AF=_tmp3A7;for(0;(unsigned int)_tmp3AF;_tmp3AF=_tmp3AF->tl){void*_tmp3B1;
void*_tmp3B2;struct Cyc_Position_Segment*_tmp3B3;struct _tuple13 _tmp3B0=*((struct
_tuple13*)_tmp3AF->hd);_tmp3B1=_tmp3B0.f1;_tmp3B2=_tmp3B0.f2;_tmp3B3=_tmp3B0.f3;
Cyc_Port_unifies(_tmp3B1,_tmp3B2);}}{struct Cyc_List_List*_tmp3B4=_tmp3A8;for(0;(
unsigned int)_tmp3B4;_tmp3B4=_tmp3B4->tl){void*_tmp3B6;void*_tmp3B7;struct Cyc_Position_Segment*
_tmp3B8;struct _tuple13 _tmp3B5=*((struct _tuple13*)_tmp3B4->hd);_tmp3B6=_tmp3B5.f1;
_tmp3B7=_tmp3B5.f2;_tmp3B8=_tmp3B5.f3;Cyc_Port_unifies(_tmp3B6,_tmp3B7);}}for(0;(
unsigned int)_tmp3A6;_tmp3A6=_tmp3A6->tl){void*_tmp3BA;void*_tmp3BB;struct Cyc_Position_Segment*
_tmp3BC;struct _tuple13 _tmp3B9=*((struct _tuple13*)_tmp3A6->hd);_tmp3BA=_tmp3B9.f1;
_tmp3BB=_tmp3B9.f2;_tmp3BC=_tmp3B9.f3;if(!Cyc_Port_unifies(_tmp3BA,_tmp3BB) && (
unsigned int)_tmp3BC){void*_tmp3BD=Cyc_Port_compress_ct(_tmp3BB);_LL2CC: if((int)
_tmp3BD != 2)goto _LL2CE;_LL2CD:{struct Cyc_Port_Edit*_tmp539;const char*_tmp538;
const char*_tmp537;struct Cyc_List_List*_tmp536;_tmp3A9=((_tmp536=_cycalloc(
sizeof(*_tmp536)),((_tmp536->hd=((_tmp539=_cycalloc(sizeof(*_tmp539)),((_tmp539->loc=
_tmp3BC,((_tmp539->old_string=((_tmp537="?",_tag_dyneither(_tmp537,sizeof(char),
2))),((_tmp539->new_string=((_tmp538="*",_tag_dyneither(_tmp538,sizeof(char),2))),
_tmp539)))))))),((_tmp536->tl=_tmp3A9,_tmp536))))));}goto _LL2CB;_LL2CE: if((int)
_tmp3BD != 3)goto _LL2D0;_LL2CF:{struct Cyc_Port_Edit*_tmp542;const char*_tmp541;
const char*_tmp540;struct Cyc_List_List*_tmp53F;_tmp3A9=((_tmp53F=_cycalloc(
sizeof(*_tmp53F)),((_tmp53F->hd=((_tmp542=_cycalloc(sizeof(*_tmp542)),((_tmp542->loc=
_tmp3BC,((_tmp542->old_string=((_tmp540="*",_tag_dyneither(_tmp540,sizeof(char),
2))),((_tmp542->new_string=((_tmp541="?",_tag_dyneither(_tmp541,sizeof(char),2))),
_tmp542)))))))),((_tmp53F->tl=_tmp3A9,_tmp53F))))));}goto _LL2CB;_LL2D0:;_LL2D1:
goto _LL2CB;_LL2CB:;}}for(0;(unsigned int)_tmp3A7;_tmp3A7=_tmp3A7->tl){void*
_tmp3C7;void*_tmp3C8;struct Cyc_Position_Segment*_tmp3C9;struct _tuple13 _tmp3C6=*((
struct _tuple13*)_tmp3A7->hd);_tmp3C7=_tmp3C6.f1;_tmp3C8=_tmp3C6.f2;_tmp3C9=
_tmp3C6.f3;if(!Cyc_Port_unifies(_tmp3C7,_tmp3C8) && (unsigned int)_tmp3C9){void*
_tmp3CA=Cyc_Port_compress_ct(_tmp3C8);_LL2D3: if((int)_tmp3CA != 1)goto _LL2D5;
_LL2D4:{struct Cyc_Port_Edit*_tmp54B;const char*_tmp54A;const char*_tmp549;struct
Cyc_List_List*_tmp548;_tmp3A9=((_tmp548=_cycalloc(sizeof(*_tmp548)),((_tmp548->hd=((
_tmp54B=_cycalloc(sizeof(*_tmp54B)),((_tmp54B->loc=_tmp3C9,((_tmp54B->old_string=((
_tmp549="const ",_tag_dyneither(_tmp549,sizeof(char),7))),((_tmp54B->new_string=((
_tmp54A="",_tag_dyneither(_tmp54A,sizeof(char),1))),_tmp54B)))))))),((_tmp548->tl=
_tmp3A9,_tmp548))))));}goto _LL2D2;_LL2D5: if((int)_tmp3CA != 0)goto _LL2D7;_LL2D6:{
struct Cyc_Port_Edit*_tmp554;const char*_tmp553;const char*_tmp552;struct Cyc_List_List*
_tmp551;_tmp3A9=((_tmp551=_cycalloc(sizeof(*_tmp551)),((_tmp551->hd=((_tmp554=
_cycalloc(sizeof(*_tmp554)),((_tmp554->loc=_tmp3C9,((_tmp554->old_string=((
_tmp552="",_tag_dyneither(_tmp552,sizeof(char),1))),((_tmp554->new_string=((
_tmp553="const ",_tag_dyneither(_tmp553,sizeof(char),7))),_tmp554)))))))),((
_tmp551->tl=_tmp3A9,_tmp551))))));}goto _LL2D2;_LL2D7:;_LL2D8: goto _LL2D2;_LL2D2:;}}
for(0;(unsigned int)_tmp3A8;_tmp3A8=_tmp3A8->tl){void*_tmp3D4;void*_tmp3D5;
struct Cyc_Position_Segment*_tmp3D6;struct _tuple13 _tmp3D3=*((struct _tuple13*)
_tmp3A8->hd);_tmp3D4=_tmp3D3.f1;_tmp3D5=_tmp3D3.f2;_tmp3D6=_tmp3D3.f3;if(!Cyc_Port_unifies(
_tmp3D4,_tmp3D5) && (unsigned int)_tmp3D6){void*_tmp3D7=Cyc_Port_compress_ct(
_tmp3D5);_LL2DA: if((int)_tmp3D7 != 8)goto _LL2DC;_LL2DB:{struct Cyc_Port_Edit*
_tmp55D;const char*_tmp55C;const char*_tmp55B;struct Cyc_List_List*_tmp55A;_tmp3A9=((
_tmp55A=_cycalloc(sizeof(*_tmp55A)),((_tmp55A->hd=((_tmp55D=_cycalloc(sizeof(*
_tmp55D)),((_tmp55D->loc=_tmp3D6,((_tmp55D->old_string=((_tmp55B="NOZEROTERM ",
_tag_dyneither(_tmp55B,sizeof(char),12))),((_tmp55D->new_string=((_tmp55C="",
_tag_dyneither(_tmp55C,sizeof(char),1))),_tmp55D)))))))),((_tmp55A->tl=_tmp3A9,
_tmp55A))))));}goto _LL2D9;_LL2DC: if((int)_tmp3D7 != 9)goto _LL2DE;_LL2DD:{struct
Cyc_Port_Edit*_tmp566;const char*_tmp565;const char*_tmp564;struct Cyc_List_List*
_tmp563;_tmp3A9=((_tmp563=_cycalloc(sizeof(*_tmp563)),((_tmp563->hd=((_tmp566=
_cycalloc(sizeof(*_tmp566)),((_tmp566->loc=_tmp3D6,((_tmp566->old_string=((
_tmp564="ZEROTERM ",_tag_dyneither(_tmp564,sizeof(char),10))),((_tmp566->new_string=((
_tmp565="",_tag_dyneither(_tmp565,sizeof(char),1))),_tmp566)))))))),((_tmp563->tl=
_tmp3A9,_tmp563))))));}goto _LL2D9;_LL2DE:;_LL2DF: goto _LL2D9;_LL2D9:;}}_tmp3A9=((
struct Cyc_List_List*(*)(int(*cmp)(struct Cyc_Port_Edit*,struct Cyc_Port_Edit*),
struct Cyc_List_List*x))Cyc_List_merge_sort)(Cyc_Port_cmp_edit,_tmp3A9);while((
unsigned int)_tmp3A9  && ((struct Cyc_Position_Segment*)_check_null(((struct Cyc_Port_Edit*)
_tmp3A9->hd)->loc))->start == 0){_tmp3A9=_tmp3A9->tl;}return _tmp3A9;}static struct
Cyc_Position_Segment*Cyc_Port_get_seg(struct Cyc_Port_Edit*e);static struct Cyc_Position_Segment*
Cyc_Port_get_seg(struct Cyc_Port_Edit*e){return e->loc;}void Cyc_Port_port(struct
Cyc_List_List*ds);void Cyc_Port_port(struct Cyc_List_List*ds){struct Cyc_List_List*
_tmp3E0=Cyc_Port_gen_edits(ds);struct Cyc_List_List*_tmp3E1=((struct Cyc_List_List*(*)(
struct Cyc_Position_Segment*(*f)(struct Cyc_Port_Edit*),struct Cyc_List_List*x))Cyc_List_map)(
Cyc_Port_get_seg,_tmp3E0);Cyc_Position_use_gcc_style_location=0;{struct Cyc_List_List*
_tmp3E2=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_Position_strings_of_segments(
_tmp3E1));for(0;(unsigned int)_tmp3E0;(_tmp3E0=_tmp3E0->tl,_tmp3E2=_tmp3E2->tl)){
struct Cyc_Position_Segment*_tmp3E4;struct _dyneither_ptr _tmp3E5;struct
_dyneither_ptr _tmp3E6;struct Cyc_Port_Edit _tmp3E3=*((struct Cyc_Port_Edit*)_tmp3E0->hd);
_tmp3E4=_tmp3E3.loc;_tmp3E5=_tmp3E3.old_string;_tmp3E6=_tmp3E3.new_string;{
struct _dyneither_ptr sloc=(struct _dyneither_ptr)*((struct _dyneither_ptr*)((struct
Cyc_List_List*)_check_null(_tmp3E2))->hd);const char*_tmp56C;void*_tmp56B[3];
struct Cyc_String_pa_struct _tmp56A;struct Cyc_String_pa_struct _tmp569;struct Cyc_String_pa_struct
_tmp568;(_tmp568.tag=0,((_tmp568.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
_tmp3E6),((_tmp569.tag=0,((_tmp569.f1=(struct _dyneither_ptr)((struct
_dyneither_ptr)_tmp3E5),((_tmp56A.tag=0,((_tmp56A.f1=(struct _dyneither_ptr)((
struct _dyneither_ptr)sloc),((_tmp56B[0]=& _tmp56A,((_tmp56B[1]=& _tmp569,((_tmp56B[
2]=& _tmp568,Cyc_printf(((_tmp56C="%s: insert `%s' for `%s'\n",_tag_dyneither(
_tmp56C,sizeof(char),26))),_tag_dyneither(_tmp56B,sizeof(void*),3)))))))))))))))))));}}}}
