#include <setjmp.h>
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
extern int _throw_match();
extern int _throw(void* e);
#endif

extern struct _xtunion_struct *_exn_thrown;

/* Built-in Exceptions */
struct Cyc_Null_Exception_struct { char *tag; };
struct Cyc_Array_bounds_struct { char *tag; };
struct Cyc_Match_Exception_struct { char *tag; };
struct Cyc_Bad_alloc_struct { char *tag; };
extern char Cyc_Null_Exception[];
extern char Cyc_Array_bounds[];
extern char Cyc_Match_Exception[];
extern char Cyc_Bad_alloc[];

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
_zero_arr_inplace_plus_char(char **x, int orig_i) {
  *x = _zero_arr_plus_char(*x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_short(short **x, int orig_i) {
  *x = _zero_arr_plus_short(*x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_int(int **x, int orig_i) {
  *x = _zero_arr_plus_int(*x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_float(float **x, int orig_i) {
  *x = _zero_arr_plus_float(*x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_double(double **x, int orig_i) {
  *x = _zero_arr_plus_double(*x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_longdouble(long double **x, int orig_i) {
  *x = _zero_arr_plus_longdouble(*x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_voidstar(void ***x, int orig_i) {
  *x = _zero_arr_plus_voidstar(*x,1,orig_i);
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
_zero_arr_inplace_plus_post_char(char **x, int orig_i){
  char * _zap_res = *x;
  *x = _zero_arr_plus_char(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE short *
_zero_arr_inplace_plus_post_short(short **x, int orig_i){
  short * _zap_res = *x;
  *x = _zero_arr_plus_short(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE int *
_zero_arr_inplace_plus_post_int(int **x, int orig_i){
  int * _zap_res = *x;
  *x = _zero_arr_plus_int(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE float *
_zero_arr_inplace_plus_post_float(float **x, int orig_i){
  float * _zap_res = *x;
  *x = _zero_arr_plus_float(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE double *
_zero_arr_inplace_plus_post_double(double **x, int orig_i){
  double * _zap_res = *x;
  *x = _zero_arr_plus_double(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE long double *
_zero_arr_inplace_plus_post_longdouble(long double **x, int orig_i){
  long double * _zap_res = *x;
  *x = _zero_arr_plus_longdouble(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE void **
_zero_arr_inplace_plus_post_voidstar(void ***x, int orig_i){
  void ** _zap_res = *x;
  *x = _zero_arr_plus_voidstar(_zap_res,1,orig_i);
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
extern void* _profile_GC_malloc(int,const char *file,const char *func,
                                int lineno);
extern void* _profile_GC_malloc_atomic(int,const char *file,
                                       const char *func,int lineno);
extern void* _profile_region_malloc(struct _RegionHandle *, unsigned,
                                    const char *file,
                                    const char *func,
                                    int lineno);
extern struct _RegionHandle _profile_new_region(const char *rgn_name,
						const char *file,
						const char *func,
                                                int lineno);
extern void _profile_free_region(struct _RegionHandle *,
				 const char *file,
                                 const char *func,
                                 int lineno);
#  if !defined(RUNTIME_CYC)
#define _new_region(n) _profile_new_region(n,__FILE__,__FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__,__FUNCTION__,__LINE__)
#define _region_malloc(rh,n) _profile_region_malloc(rh,n,__FILE__,__FUNCTION__,__LINE__)
#  endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__,__FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__,__FUNCTION__,__LINE__)
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
 typedef char*Cyc_Cstring;typedef char*Cyc_CstringNN;typedef struct _dyneither_ptr Cyc_string_t;
typedef struct _dyneither_ptr Cyc_mstring_t;typedef struct _dyneither_ptr*Cyc_stringptr_t;
typedef struct _dyneither_ptr*Cyc_mstringptr_t;typedef char*Cyc_Cbuffer_t;typedef
char*Cyc_CbufferNN_t;typedef struct _dyneither_ptr Cyc_buffer_t;typedef struct
_dyneither_ptr Cyc_mbuffer_t;typedef int Cyc_bool;struct Cyc_Core_NewRegion{struct
_DynRegionHandle*dynregion;};typedef unsigned long Cyc_size_t;typedef
unsigned short Cyc_mode_t;struct Cyc___cycFILE;typedef struct Cyc___cycFILE Cyc_FILE;
extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_String_pa_struct{int tag;struct
_dyneither_ptr f1;};struct Cyc_Int_pa_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_struct{
int tag;double f1;};struct Cyc_LongDouble_pa_struct{int tag;long double f1;};struct
Cyc_ShortPtr_pa_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_struct{int tag;
unsigned long*f1;};typedef void*Cyc_parg_t;struct _dyneither_ptr Cyc_aprintf(struct
_dyneither_ptr,struct _dyneither_ptr);int Cyc_fprintf(struct Cyc___cycFILE*,struct
_dyneither_ptr,struct _dyneither_ptr);struct Cyc_ShortPtr_sa_struct{int tag;short*
f1;};struct Cyc_UShortPtr_sa_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_struct{
int tag;int*f1;};struct Cyc_UIntPtr_sa_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_struct{
int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_struct{int tag;double*f1;};
struct Cyc_FloatPtr_sa_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_struct{int
tag;struct _dyneither_ptr f1;};typedef void*Cyc_sarg_t;struct _dyneither_ptr Cyc_vrprintf(
struct _RegionHandle*,struct _dyneither_ptr,struct _dyneither_ptr);extern char Cyc_FileCloseError[
15];struct Cyc_FileCloseError_struct{char*tag;};extern char Cyc_FileOpenError[14];
struct Cyc_FileOpenError_struct{char*tag;struct _dyneither_ptr f1;};typedef
unsigned int Cyc_Core_sizeof_t;struct Cyc_Core_Opt{void*v;};typedef struct Cyc_Core_Opt*
Cyc_Core_opt_t;extern char Cyc_Core_Invalid_argument[17];struct Cyc_Core_Invalid_argument_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[8];struct Cyc_Core_Failure_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[11];struct Cyc_Core_Impossible_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[10];struct Cyc_Core_Not_found_struct{
char*tag;};extern char Cyc_Core_Unreachable[12];struct Cyc_Core_Unreachable_struct{
char*tag;struct _dyneither_ptr f1;};extern struct _RegionHandle*Cyc_Core_heap_region;
extern struct _RegionHandle*Cyc_Core_unique_region;extern char Cyc_Core_Open_Region[
12];struct Cyc_Core_Open_Region_struct{char*tag;};extern char Cyc_Core_Free_Region[
12];struct Cyc_Core_Free_Region_struct{char*tag;};inline static void* arrcast(
struct _dyneither_ptr dyn,unsigned int bd,unsigned int sz){if(bd >> 20  || sz >> 12)
return 0;{unsigned char*ptrbd=dyn.curr + bd * sz;if(((ptrbd < dyn.curr  || dyn.curr == 
0) || dyn.curr < dyn.base) || ptrbd > dyn.last_plus_one)return 0;return dyn.curr;};}
extern char Cyc_Lexing_Error[6];struct Cyc_Lexing_Error_struct{char*tag;struct
_dyneither_ptr f1;};struct Cyc_Lexing_lexbuf{void(*refill_buff)(struct Cyc_Lexing_lexbuf*);
void*refill_state;struct _dyneither_ptr lex_buffer;int lex_buffer_len;int
lex_abs_pos;int lex_start_pos;int lex_curr_pos;int lex_last_pos;int lex_last_action;
int lex_eof_reached;};typedef struct Cyc_Lexing_lexbuf*Cyc_Lexing_Lexbuf;struct Cyc_Lexing_function_lexbuf_state{
int(*read_fun)(struct _dyneither_ptr,int,void*);void*read_fun_state;};typedef
struct Cyc_Lexing_function_lexbuf_state*Cyc_Lexing_Function_lexbuf_state;struct
Cyc_Lexing_lex_tables{struct _dyneither_ptr lex_base;struct _dyneither_ptr
lex_backtrk;struct _dyneither_ptr lex_default;struct _dyneither_ptr lex_trans;struct
_dyneither_ptr lex_check;};typedef struct Cyc_Lexing_lex_tables*Cyc_Lexing_LexTables;
struct Cyc_Lexing_lexbuf*Cyc_Lexing_from_file(struct Cyc___cycFILE*);struct Cyc_List_List{
void*hd;struct Cyc_List_List*tl;};typedef struct Cyc_List_List*Cyc_List_list_t;
typedef struct Cyc_List_List*Cyc_List_List_t;struct Cyc_List_List*Cyc_List_list(
struct _dyneither_ptr);int Cyc_List_length(struct Cyc_List_List*x);struct Cyc_List_List*
Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_map_c(
void*(*f)(void*,void*),void*env,struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[
14];struct Cyc_List_List_mismatch_struct{char*tag;};void Cyc_List_iter_c(void(*f)(
void*,void*),void*env,struct Cyc_List_List*x);void*Cyc_List_fold_right(void*(*f)(
void*,void*),struct Cyc_List_List*x,void*accum);struct Cyc_List_List*Cyc_List_rev(
struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*
x);struct Cyc_List_List*Cyc_List_append(struct Cyc_List_List*x,struct Cyc_List_List*
y);struct Cyc_List_List*Cyc_List_imp_append(struct Cyc_List_List*x,struct Cyc_List_List*
y);extern char Cyc_List_Nth[4];struct Cyc_List_Nth_struct{char*tag;};struct Cyc_List_List*
Cyc_List_rzip(struct _RegionHandle*r1,struct _RegionHandle*r2,struct Cyc_List_List*
x,struct Cyc_List_List*y);unsigned long Cyc_strlen(struct _dyneither_ptr s);int Cyc_strcmp(
struct _dyneither_ptr s1,struct _dyneither_ptr s2);int Cyc_strptrcmp(struct
_dyneither_ptr*s1,struct _dyneither_ptr*s2);int Cyc_zstrcmp(struct _dyneither_ptr,
struct _dyneither_ptr);int Cyc_zstrptrcmp(struct _dyneither_ptr*,struct
_dyneither_ptr*);struct _dyneither_ptr Cyc_strcat(struct _dyneither_ptr dest,struct
_dyneither_ptr src);struct _dyneither_ptr Cyc_strcpy(struct _dyneither_ptr dest,
struct _dyneither_ptr src);struct _dyneither_ptr Cyc_substring(struct _dyneither_ptr,
int ofs,unsigned long n);struct Cyc_Lineno_Pos{struct _dyneither_ptr logical_file;
struct _dyneither_ptr line;int line_no;int col;};typedef struct Cyc_Lineno_Pos*Cyc_Lineno_pos_t;
extern char Cyc_Position_Exit[5];struct Cyc_Position_Exit_struct{char*tag;};typedef
unsigned int Cyc_Position_seg_t;struct _dyneither_ptr Cyc_Position_string_of_segment(
unsigned int);struct Cyc_Position_Lex_struct{int tag;};struct Cyc_Position_Parse_struct{
int tag;};struct Cyc_Position_Elab_struct{int tag;};typedef void*Cyc_Position_error_kind_t;
struct Cyc_Position_Error{struct _dyneither_ptr source;unsigned int seg;void*kind;
struct _dyneither_ptr desc;};typedef struct Cyc_Position_Error*Cyc_Position_error_t;
struct Cyc_Position_Error*Cyc_Position_mk_err_parse(unsigned int,struct
_dyneither_ptr);extern char Cyc_Position_Nocontext[10];struct Cyc_Position_Nocontext_struct{
char*tag;};void Cyc_Position_post_error(struct Cyc_Position_Error*);typedef struct
_dyneither_ptr*Cyc_Absyn_field_name_t;typedef struct _dyneither_ptr*Cyc_Absyn_var_t;
typedef struct _dyneither_ptr*Cyc_Absyn_tvarname_t;typedef struct _dyneither_ptr*Cyc_Absyn_var_opt_t;
struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct
_union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{
int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};
union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct
_union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n
Loc_n;};typedef union Cyc_Absyn_Nmspace Cyc_Absyn_nmspace_t;union Cyc_Absyn_Nmspace
Cyc_Absyn_Loc_n;union Cyc_Absyn_Nmspace Cyc_Absyn_Rel_n(struct Cyc_List_List*);
union Cyc_Absyn_Nmspace Cyc_Absyn_Abs_n(struct Cyc_List_List*ns,int C_scope);struct
_tuple0{union Cyc_Absyn_Nmspace f1;struct _dyneither_ptr*f2;};typedef struct _tuple0*
Cyc_Absyn_qvar_t;typedef struct _tuple0*Cyc_Absyn_qvar_opt_t;typedef struct _tuple0*
Cyc_Absyn_typedef_name_t;typedef struct _tuple0*Cyc_Absyn_typedef_name_opt_t;
typedef enum Cyc_Absyn_Scope Cyc_Absyn_scope_t;typedef struct Cyc_Absyn_Tqual Cyc_Absyn_tqual_t;
typedef enum Cyc_Absyn_Size_of Cyc_Absyn_size_of_t;typedef struct Cyc_Absyn_Kind*Cyc_Absyn_kind_t;
typedef void*Cyc_Absyn_kindbound_t;typedef struct Cyc_Absyn_Tvar*Cyc_Absyn_tvar_t;
typedef enum Cyc_Absyn_Sign Cyc_Absyn_sign_t;typedef enum Cyc_Absyn_AggrKind Cyc_Absyn_aggr_kind_t;
typedef void*Cyc_Absyn_bounds_t;typedef struct Cyc_Absyn_PtrAtts Cyc_Absyn_ptr_atts_t;
typedef struct Cyc_Absyn_PtrInfo Cyc_Absyn_ptr_info_t;typedef struct Cyc_Absyn_VarargInfo
Cyc_Absyn_vararg_info_t;typedef struct Cyc_Absyn_FnInfo Cyc_Absyn_fn_info_t;typedef
struct Cyc_Absyn_DatatypeInfo Cyc_Absyn_datatype_info_t;typedef struct Cyc_Absyn_DatatypeFieldInfo
Cyc_Absyn_datatype_field_info_t;typedef struct Cyc_Absyn_AggrInfo Cyc_Absyn_aggr_info_t;
typedef struct Cyc_Absyn_ArrayInfo Cyc_Absyn_array_info_t;typedef void*Cyc_Absyn_type_t;
typedef void*Cyc_Absyn_rgntype_t;typedef void*Cyc_Absyn_type_opt_t;typedef union Cyc_Absyn_Cnst
Cyc_Absyn_cnst_t;typedef enum Cyc_Absyn_Primop Cyc_Absyn_primop_t;typedef enum Cyc_Absyn_Incrementor
Cyc_Absyn_incrementor_t;typedef struct Cyc_Absyn_VarargCallInfo Cyc_Absyn_vararg_call_info_t;
typedef void*Cyc_Absyn_raw_exp_t;typedef struct Cyc_Absyn_Exp*Cyc_Absyn_exp_t;
typedef struct Cyc_Absyn_Exp*Cyc_Absyn_exp_opt_t;typedef void*Cyc_Absyn_raw_stmt_t;
typedef struct Cyc_Absyn_Stmt*Cyc_Absyn_stmt_t;typedef struct Cyc_Absyn_Stmt*Cyc_Absyn_stmt_opt_t;
typedef void*Cyc_Absyn_raw_pat_t;typedef struct Cyc_Absyn_Pat*Cyc_Absyn_pat_t;
typedef void*Cyc_Absyn_binding_t;typedef struct Cyc_Absyn_Switch_clause*Cyc_Absyn_switch_clause_t;
typedef struct Cyc_Absyn_Fndecl*Cyc_Absyn_fndecl_t;typedef struct Cyc_Absyn_Aggrdecl*
Cyc_Absyn_aggrdecl_t;typedef struct Cyc_Absyn_Datatypefield*Cyc_Absyn_datatypefield_t;
typedef struct Cyc_Absyn_Datatypedecl*Cyc_Absyn_datatypedecl_t;typedef struct Cyc_Absyn_Typedefdecl*
Cyc_Absyn_typedefdecl_t;typedef struct Cyc_Absyn_Enumfield*Cyc_Absyn_enumfield_t;
typedef struct Cyc_Absyn_Enumdecl*Cyc_Absyn_enumdecl_t;typedef struct Cyc_Absyn_Vardecl*
Cyc_Absyn_vardecl_t;typedef void*Cyc_Absyn_raw_decl_t;typedef struct Cyc_Absyn_Decl*
Cyc_Absyn_decl_t;typedef void*Cyc_Absyn_designator_t;typedef void*Cyc_Absyn_absyn_annot_t;
typedef void*Cyc_Absyn_attribute_t;typedef struct Cyc_List_List*Cyc_Absyn_attributes_t;
typedef struct Cyc_Absyn_Aggrfield*Cyc_Absyn_aggrfield_t;typedef void*Cyc_Absyn_offsetof_field_t;
typedef struct Cyc_Absyn_MallocInfo Cyc_Absyn_malloc_info_t;typedef enum Cyc_Absyn_Coercion
Cyc_Absyn_coercion_t;typedef struct Cyc_Absyn_PtrLoc*Cyc_Absyn_ptrloc_t;enum Cyc_Absyn_Scope{
Cyc_Absyn_Static  = 0,Cyc_Absyn_Abstract  = 1,Cyc_Absyn_Public  = 2,Cyc_Absyn_Extern
 = 3,Cyc_Absyn_ExternC  = 4,Cyc_Absyn_Register  = 5};struct Cyc_Absyn_Tqual{int
print_const;int q_volatile;int q_restrict;int real_const;unsigned int loc;};enum Cyc_Absyn_Size_of{
Cyc_Absyn_Char_sz  = 0,Cyc_Absyn_Short_sz  = 1,Cyc_Absyn_Int_sz  = 2,Cyc_Absyn_Long_sz
 = 3,Cyc_Absyn_LongLong_sz  = 4};enum Cyc_Absyn_AliasQual{Cyc_Absyn_Aliasable  = 0,
Cyc_Absyn_Unique  = 1,Cyc_Absyn_Top  = 2};enum Cyc_Absyn_KindQual{Cyc_Absyn_AnyKind
 = 0,Cyc_Absyn_MemKind  = 1,Cyc_Absyn_BoxKind  = 2,Cyc_Absyn_RgnKind  = 3,Cyc_Absyn_EffKind
 = 4,Cyc_Absyn_IntKind  = 5};struct Cyc_Absyn_Kind{enum Cyc_Absyn_KindQual kind;
enum Cyc_Absyn_AliasQual aliasqual;};enum Cyc_Absyn_Sign{Cyc_Absyn_Signed  = 0,Cyc_Absyn_Unsigned
 = 1,Cyc_Absyn_None  = 2};enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA  = 0,Cyc_Absyn_UnionA
 = 1};struct _union_Constraint_Eq_constr{int tag;void*val;};struct
_union_Constraint_Forward_constr{int tag;union Cyc_Absyn_Constraint*val;};struct
_union_Constraint_No_constr{int tag;int val;};union Cyc_Absyn_Constraint{struct
_union_Constraint_Eq_constr Eq_constr;struct _union_Constraint_Forward_constr
Forward_constr;struct _union_Constraint_No_constr No_constr;};typedef union Cyc_Absyn_Constraint*
Cyc_Absyn_conref_t;struct Cyc_Absyn_Eq_kb_struct{int tag;struct Cyc_Absyn_Kind*f1;};
struct Cyc_Absyn_Unknown_kb_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_struct{
int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{
struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_DynEither_b_struct{
int tag;};struct Cyc_Absyn_Upper_b_struct{int tag;struct Cyc_Absyn_Exp*f1;};extern
struct Cyc_Absyn_DynEither_b_struct Cyc_Absyn_DynEither_b_val;struct Cyc_Absyn_PtrLoc{
unsigned int ptr_loc;unsigned int rgn_loc;unsigned int zt_loc;};struct Cyc_Absyn_PtrAtts{
void*rgn;union Cyc_Absyn_Constraint*nullable;union Cyc_Absyn_Constraint*bounds;
union Cyc_Absyn_Constraint*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{
void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};
struct Cyc_Absyn_Numelts_ptrqual_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Region_ptrqual_struct{
int tag;void*f1;};struct Cyc_Absyn_Thin_ptrqual_struct{int tag;};struct Cyc_Absyn_Fat_ptrqual_struct{
int tag;};struct Cyc_Absyn_Zeroterm_ptrqual_struct{int tag;};struct Cyc_Absyn_Nozeroterm_ptrqual_struct{
int tag;};struct Cyc_Absyn_Notnull_ptrqual_struct{int tag;};struct Cyc_Absyn_Nullable_ptrqual_struct{
int tag;};typedef void*Cyc_Absyn_pointer_qual_t;typedef struct Cyc_List_List*Cyc_Absyn_pointer_quals_t;
struct Cyc_Absyn_VarargInfo{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;
void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*
effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_typ;struct Cyc_List_List*args;int
c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;
struct Cyc_List_List*attributes;};struct Cyc_Absyn_UnknownDatatypeInfo{struct
_tuple0*name;int is_extensible;};struct _union_DatatypeInfoU_UnknownDatatype{int
tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct
_union_DatatypeInfoU_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};
union Cyc_Absyn_DatatypeInfoU{struct _union_DatatypeInfoU_UnknownDatatype
UnknownDatatype;struct _union_DatatypeInfoU_KnownDatatype KnownDatatype;};union Cyc_Absyn_DatatypeInfoU
Cyc_Absyn_UnknownDatatype(struct Cyc_Absyn_UnknownDatatypeInfo);struct Cyc_Absyn_DatatypeInfo{
union Cyc_Absyn_DatatypeInfoU datatype_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{
struct _tuple0*datatype_name;struct _tuple0*field_name;int is_extensible;};struct
_union_DatatypeFieldInfoU_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo
val;};struct _tuple1{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*
f2;};struct _union_DatatypeFieldInfoU_KnownDatatypefield{int tag;struct _tuple1 val;
};union Cyc_Absyn_DatatypeFieldInfoU{struct
_union_DatatypeFieldInfoU_UnknownDatatypefield UnknownDatatypefield;struct
_union_DatatypeFieldInfoU_KnownDatatypefield KnownDatatypefield;};union Cyc_Absyn_DatatypeFieldInfoU
Cyc_Absyn_UnknownDatatypefield(struct Cyc_Absyn_UnknownDatatypeFieldInfo);struct
Cyc_Absyn_DatatypeFieldInfo{union Cyc_Absyn_DatatypeFieldInfoU field_info;struct
Cyc_List_List*targs;};struct _tuple2{enum Cyc_Absyn_AggrKind f1;struct _tuple0*f2;
struct Cyc_Core_Opt*f3;};struct _union_AggrInfoU_UnknownAggr{int tag;struct _tuple2
val;};struct _union_AggrInfoU_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};
union Cyc_Absyn_AggrInfoU{struct _union_AggrInfoU_UnknownAggr UnknownAggr;struct
_union_AggrInfoU_KnownAggr KnownAggr;};union Cyc_Absyn_AggrInfoU Cyc_Absyn_UnknownAggr(
enum Cyc_Absyn_AggrKind,struct _tuple0*,struct Cyc_Core_Opt*);struct Cyc_Absyn_AggrInfo{
union Cyc_Absyn_AggrInfoU aggr_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_ArrayInfo{
void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;union Cyc_Absyn_Constraint*
zero_term;unsigned int zt_loc;};struct Cyc_Absyn_Aggr_td_struct{int tag;struct Cyc_Absyn_Aggrdecl*
f1;};struct Cyc_Absyn_Enum_td_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct
Cyc_Absyn_Datatype_td_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};typedef
void*Cyc_Absyn_raw_type_decl_t;struct Cyc_Absyn_TypeDecl{void*r;unsigned int loc;};
typedef struct Cyc_Absyn_TypeDecl*Cyc_Absyn_type_decl_t;struct Cyc_Absyn_VoidType_struct{
int tag;};struct Cyc_Absyn_Evar_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;
struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_struct{int tag;struct Cyc_Absyn_Tvar*
f1;};struct Cyc_Absyn_DatatypeType_struct{int tag;struct Cyc_Absyn_DatatypeInfo f1;};
struct Cyc_Absyn_DatatypeFieldType_struct{int tag;struct Cyc_Absyn_DatatypeFieldInfo
f1;};struct Cyc_Absyn_PointerType_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};
struct Cyc_Absyn_IntType_struct{int tag;enum Cyc_Absyn_Sign f1;enum Cyc_Absyn_Size_of
f2;};struct Cyc_Absyn_FloatType_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_struct{
int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_struct{int tag;struct
Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_AggrType_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct
Cyc_Absyn_AnonAggrType_struct{int tag;enum Cyc_Absyn_AggrKind f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_EnumType_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*
f2;};struct Cyc_Absyn_AnonEnumType_struct{int tag;struct Cyc_List_List*f1;};struct
Cyc_Absyn_RgnHandleType_struct{int tag;void*f1;};struct Cyc_Absyn_DynRgnType_struct{
int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_struct{int tag;struct _tuple0*
f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TagType_struct{int tag;void*f1;};
struct Cyc_Absyn_HeapRgn_struct{int tag;};struct Cyc_Absyn_UniqueRgn_struct{int tag;
};struct Cyc_Absyn_RefCntRgn_struct{int tag;};struct Cyc_Absyn_AccessEff_struct{int
tag;void*f1;};struct Cyc_Absyn_JoinEff_struct{int tag;struct Cyc_List_List*f1;};
struct Cyc_Absyn_RgnsEff_struct{int tag;void*f1;};struct Cyc_Absyn_TypeDeclType_struct{
int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};extern struct Cyc_Absyn_HeapRgn_struct
Cyc_Absyn_HeapRgn_val;extern struct Cyc_Absyn_UniqueRgn_struct Cyc_Absyn_UniqueRgn_val;
extern struct Cyc_Absyn_RefCntRgn_struct Cyc_Absyn_RefCntRgn_val;extern struct Cyc_Absyn_VoidType_struct
Cyc_Absyn_VoidType_val;struct Cyc_Absyn_NoTypes_struct{int tag;struct Cyc_List_List*
f1;unsigned int f2;};struct Cyc_Absyn_WithTypes_struct{int tag;struct Cyc_List_List*
f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;};typedef
void*Cyc_Absyn_funcparams_t;enum Cyc_Absyn_Format_Type{Cyc_Absyn_Printf_ft  = 0,
Cyc_Absyn_Scanf_ft  = 1};struct Cyc_Absyn_Regparm_att_struct{int tag;int f1;};struct
Cyc_Absyn_Stdcall_att_struct{int tag;};struct Cyc_Absyn_Cdecl_att_struct{int tag;};
struct Cyc_Absyn_Fastcall_att_struct{int tag;};struct Cyc_Absyn_Noreturn_att_struct{
int tag;};struct Cyc_Absyn_Const_att_struct{int tag;};struct Cyc_Absyn_Aligned_att_struct{
int tag;int f1;};struct Cyc_Absyn_Packed_att_struct{int tag;};struct Cyc_Absyn_Section_att_struct{
int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Nocommon_att_struct{int tag;};
struct Cyc_Absyn_Shared_att_struct{int tag;};struct Cyc_Absyn_Unused_att_struct{int
tag;};struct Cyc_Absyn_Weak_att_struct{int tag;};struct Cyc_Absyn_Dllimport_att_struct{
int tag;};struct Cyc_Absyn_Dllexport_att_struct{int tag;};struct Cyc_Absyn_No_instrument_function_att_struct{
int tag;};struct Cyc_Absyn_Constructor_att_struct{int tag;};struct Cyc_Absyn_Destructor_att_struct{
int tag;};struct Cyc_Absyn_No_check_memory_usage_att_struct{int tag;};struct Cyc_Absyn_Format_att_struct{
int tag;enum Cyc_Absyn_Format_Type f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_struct{
int tag;int f1;};struct Cyc_Absyn_Noliveunique_att_struct{int tag;int f1;};struct Cyc_Absyn_Pure_att_struct{
int tag;};struct Cyc_Absyn_Mode_att_struct{int tag;struct _dyneither_ptr f1;};extern
struct Cyc_Absyn_Stdcall_att_struct Cyc_Absyn_Stdcall_att_val;extern struct Cyc_Absyn_Cdecl_att_struct
Cyc_Absyn_Cdecl_att_val;extern struct Cyc_Absyn_Fastcall_att_struct Cyc_Absyn_Fastcall_att_val;
extern struct Cyc_Absyn_Noreturn_att_struct Cyc_Absyn_Noreturn_att_val;extern struct
Cyc_Absyn_Const_att_struct Cyc_Absyn_Const_att_val;extern struct Cyc_Absyn_Packed_att_struct
Cyc_Absyn_Packed_att_val;extern struct Cyc_Absyn_Shared_att_struct Cyc_Absyn_Shared_att_val;
extern struct Cyc_Absyn_Unused_att_struct Cyc_Absyn_Unused_att_val;extern struct Cyc_Absyn_Weak_att_struct
Cyc_Absyn_Weak_att_val;extern struct Cyc_Absyn_Dllimport_att_struct Cyc_Absyn_Dllimport_att_val;
extern struct Cyc_Absyn_Dllexport_att_struct Cyc_Absyn_Dllexport_att_val;extern
struct Cyc_Absyn_No_instrument_function_att_struct Cyc_Absyn_No_instrument_function_att_val;
extern struct Cyc_Absyn_Constructor_att_struct Cyc_Absyn_Constructor_att_val;extern
struct Cyc_Absyn_Destructor_att_struct Cyc_Absyn_Destructor_att_val;extern struct
Cyc_Absyn_No_check_memory_usage_att_struct Cyc_Absyn_No_check_memory_usage_att_val;
extern struct Cyc_Absyn_Pure_att_struct Cyc_Absyn_Pure_att_val;struct Cyc_Absyn_Carray_mod_struct{
int tag;union Cyc_Absyn_Constraint*f1;unsigned int f2;};struct Cyc_Absyn_ConstArray_mod_struct{
int tag;struct Cyc_Absyn_Exp*f1;union Cyc_Absyn_Constraint*f2;unsigned int f3;};
struct Cyc_Absyn_Pointer_mod_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual
f2;};struct Cyc_Absyn_Function_mod_struct{int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_struct{
int tag;struct Cyc_List_List*f1;unsigned int f2;int f3;};struct Cyc_Absyn_Attributes_mod_struct{
int tag;unsigned int f1;struct Cyc_List_List*f2;};typedef void*Cyc_Absyn_type_modifier_t;
struct _union_Cnst_Null_c{int tag;int val;};struct _tuple3{enum Cyc_Absyn_Sign f1;
char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple3 val;};struct
_union_Cnst_Wchar_c{int tag;struct _dyneither_ptr val;};struct _tuple4{enum Cyc_Absyn_Sign
f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple4 val;};struct _tuple5{
enum Cyc_Absyn_Sign f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple5 val;};
struct _tuple6{enum Cyc_Absyn_Sign f1;long long f2;};struct _union_Cnst_LongLong_c{
int tag;struct _tuple6 val;};struct _tuple7{struct _dyneither_ptr f1;int f2;};struct
_union_Cnst_Float_c{int tag;struct _tuple7 val;};struct _union_Cnst_String_c{int tag;
struct _dyneither_ptr val;};struct _union_Cnst_Wstring_c{int tag;struct
_dyneither_ptr val;};union Cyc_Absyn_Cnst{struct _union_Cnst_Null_c Null_c;struct
_union_Cnst_Char_c Char_c;struct _union_Cnst_Wchar_c Wchar_c;struct
_union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct
_union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct
_union_Cnst_String_c String_c;struct _union_Cnst_Wstring_c Wstring_c;};enum Cyc_Absyn_Primop{
Cyc_Absyn_Plus  = 0,Cyc_Absyn_Times  = 1,Cyc_Absyn_Minus  = 2,Cyc_Absyn_Div  = 3,Cyc_Absyn_Mod
 = 4,Cyc_Absyn_Eq  = 5,Cyc_Absyn_Neq  = 6,Cyc_Absyn_Gt  = 7,Cyc_Absyn_Lt  = 8,Cyc_Absyn_Gte
 = 9,Cyc_Absyn_Lte  = 10,Cyc_Absyn_Not  = 11,Cyc_Absyn_Bitnot  = 12,Cyc_Absyn_Bitand
 = 13,Cyc_Absyn_Bitor  = 14,Cyc_Absyn_Bitxor  = 15,Cyc_Absyn_Bitlshift  = 16,Cyc_Absyn_Bitlrshift
 = 17,Cyc_Absyn_Bitarshift  = 18,Cyc_Absyn_Numelts  = 19};enum Cyc_Absyn_Incrementor{
Cyc_Absyn_PreInc  = 0,Cyc_Absyn_PostInc  = 1,Cyc_Absyn_PreDec  = 2,Cyc_Absyn_PostDec
 = 3};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*
injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_struct{
int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_struct{int tag;
unsigned int f1;};enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion  = 0,Cyc_Absyn_No_coercion
 = 1,Cyc_Absyn_NonNull_to_Null  = 2,Cyc_Absyn_Other_coercion  = 3};struct Cyc_Absyn_MallocInfo{
int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;
int fat_result;};struct Cyc_Absyn_Const_e_struct{int tag;union Cyc_Absyn_Cnst f1;};
struct Cyc_Absyn_Var_e_struct{int tag;struct _tuple0*f1;void*f2;};struct Cyc_Absyn_Primop_e_struct{
int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};
struct Cyc_Absyn_Increment_e_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor
f2;};struct Cyc_Absyn_Conditional_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_struct{int
tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*
f3;int f4;};struct Cyc_Absyn_Throw_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct
Cyc_Absyn_NoInstantiate_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_struct{
int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct
Cyc_Absyn_Address_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_struct{
int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_Offsetof_e_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_Deref_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_struct{
int tag;struct Cyc_List_List*f1;};struct _tuple8{struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual
f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_struct{int tag;struct _tuple8*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_struct{int tag;struct Cyc_List_List*f1;
};struct Cyc_Absyn_Comprehension_e_struct{int tag;struct Cyc_Absyn_Vardecl*f1;
struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_Aggregate_e_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*
f4;};struct Cyc_Absyn_AnonStruct_e_struct{int tag;void*f1;struct Cyc_List_List*f2;};
struct Cyc_Absyn_Datatype_e_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*
f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_struct{int tag;
struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;struct Cyc_Absyn_Enumfield*f3;};
struct Cyc_Absyn_AnonEnum_e_struct{int tag;struct _tuple0*f1;void*f2;struct Cyc_Absyn_Enumfield*
f3;};struct Cyc_Absyn_Malloc_e_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};
struct Cyc_Absyn_Swap_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*
f2;};struct Cyc_Absyn_UnresolvedMem_e_struct{int tag;struct Cyc_Core_Opt*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_struct{int tag;struct Cyc_Absyn_Stmt*
f1;};struct Cyc_Absyn_Tagcheck_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
_dyneither_ptr*f2;};struct Cyc_Absyn_Valueof_e_struct{int tag;void*f1;};struct Cyc_Absyn_Asm_e_struct{
int tag;int f1;struct _dyneither_ptr f2;};struct Cyc_Absyn_Exp{void*topt;void*r;
unsigned int loc;void*annot;};struct Cyc_Absyn_Skip_s_struct{int tag;};struct Cyc_Absyn_Exp_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_IfThenElse_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*
f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple9{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_While_s_struct{int tag;struct _tuple9 f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_Break_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Continue_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Goto_s_struct{int tag;struct
_dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_For_s_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct _tuple9 f2;struct _tuple9 f3;struct Cyc_Absyn_Stmt*f4;};
struct Cyc_Absyn_Switch_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Fallthru_s_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**
f2;};struct Cyc_Absyn_Decl_s_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_Label_s_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_Do_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple9 f2;
};struct Cyc_Absyn_TryCatch_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_ResetRegion_s_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct
Cyc_Absyn_Stmt{void*r;unsigned int loc;struct Cyc_List_List*non_local_preds;int
try_depth;void*annot;};struct Cyc_Absyn_Wild_p_struct{int tag;};struct Cyc_Absyn_Var_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_Reference_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_struct{
int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_struct{
int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_struct{int tag;
struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_struct{int tag;struct Cyc_Absyn_AggrInfo*
f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_struct{
int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct
Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Null_p_struct{int tag;};struct Cyc_Absyn_Int_p_struct{
int tag;enum Cyc_Absyn_Sign f1;int f2;};struct Cyc_Absyn_Char_p_struct{int tag;char f1;
};struct Cyc_Absyn_Float_p_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Absyn_Enum_p_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_struct{
int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_struct{
int tag;struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_struct{int tag;struct
_tuple0*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_struct{int tag;
struct Cyc_Absyn_Exp*f1;};extern struct Cyc_Absyn_Wild_p_struct Cyc_Absyn_Wild_p_val;
extern struct Cyc_Absyn_Null_p_struct Cyc_Absyn_Null_p_val;struct Cyc_Absyn_Pat{void*
r;void*topt;unsigned int loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*
pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*
body;unsigned int loc;};struct Cyc_Absyn_Unresolved_b_struct{int tag;};struct Cyc_Absyn_Global_b_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_struct{int tag;
struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_struct{int tag;struct Cyc_Absyn_Vardecl*
f1;};struct Cyc_Absyn_Local_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Pat_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{
enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_Absyn_Tqual tq;void*type;
struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int
escapes;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct
_tuple0*name;struct Cyc_List_List*tvs;void*effect;struct Cyc_Absyn_Tqual ret_tqual;
void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*
cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;void*cached_typ;
struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;struct Cyc_List_List*
attributes;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual
tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;};struct
Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*rgn_po;
struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind
kind;enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*
impl;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Datatypefield{struct
_tuple0*name;struct Cyc_List_List*typs;unsigned int loc;enum Cyc_Absyn_Scope sc;};
struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*
tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{
struct _tuple0*name;struct Cyc_Absyn_Exp*tag;unsigned int loc;};struct Cyc_Absyn_Enumdecl{
enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{
struct _tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
kind;void*defn;struct Cyc_List_List*atts;};struct Cyc_Absyn_Var_d_struct{int tag;
struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_struct{int tag;struct Cyc_Absyn_Fndecl*
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
int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Porton_d_struct{
int tag;};struct Cyc_Absyn_Portoff_d_struct{int tag;};extern struct Cyc_Absyn_Porton_d_struct
Cyc_Absyn_Porton_d_val;extern struct Cyc_Absyn_Portoff_d_struct Cyc_Absyn_Portoff_d_val;
struct Cyc_Absyn_Decl{void*r;unsigned int loc;};struct Cyc_Absyn_ArrayElement_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_struct{int tag;struct
_dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[11];struct Cyc_Absyn_EmptyAnnot_struct{
char*tag;};int Cyc_Absyn_is_qvar_qualified(struct _tuple0*);struct Cyc_Absyn_Tqual
Cyc_Absyn_combine_tqual(struct Cyc_Absyn_Tqual x,struct Cyc_Absyn_Tqual y);struct Cyc_Absyn_Tqual
Cyc_Absyn_empty_tqual(unsigned int);union Cyc_Absyn_Constraint*Cyc_Absyn_new_conref(
void*x);union Cyc_Absyn_Constraint*Cyc_Absyn_empty_conref();extern union Cyc_Absyn_Constraint*
Cyc_Absyn_true_conref;extern union Cyc_Absyn_Constraint*Cyc_Absyn_false_conref;
extern union Cyc_Absyn_Constraint*Cyc_Absyn_bounds_one_conref;extern union Cyc_Absyn_Constraint*
Cyc_Absyn_bounds_dyneither_conref;void*Cyc_Absyn_compress_kb(void*);void*Cyc_Absyn_new_evar(
struct Cyc_Core_Opt*k,struct Cyc_Core_Opt*tenv);void*Cyc_Absyn_wildtyp(struct Cyc_Core_Opt*);
void*Cyc_Absyn_int_typ(enum Cyc_Absyn_Sign,enum Cyc_Absyn_Size_of);extern void*
Cyc_Absyn_char_typ;extern void*Cyc_Absyn_uint_typ;extern void*Cyc_Absyn_sint_typ;
void*Cyc_Absyn_float_typ(int);void*Cyc_Absyn_starb_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual
tq,void*b,union Cyc_Absyn_Constraint*zero_term);void*Cyc_Absyn_array_typ(void*
elt_type,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Exp*num_elts,union Cyc_Absyn_Constraint*
zero_term,unsigned int ztloc);struct Cyc_Absyn_Exp*Cyc_Absyn_new_exp(void*,
unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_New_exp(struct Cyc_Absyn_Exp*
rgn_handle,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_null_exp(
unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_true_exp(unsigned int);struct Cyc_Absyn_Exp*
Cyc_Absyn_false_exp(unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_int_exp(enum Cyc_Absyn_Sign,
int,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(unsigned int,
unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_char_exp(char c,unsigned int);struct
Cyc_Absyn_Exp*Cyc_Absyn_wchar_exp(struct _dyneither_ptr s,unsigned int);struct Cyc_Absyn_Exp*
Cyc_Absyn_float_exp(struct _dyneither_ptr f,int i,unsigned int);struct Cyc_Absyn_Exp*
Cyc_Absyn_string_exp(struct _dyneither_ptr s,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_wstring_exp(
struct _dyneither_ptr s,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_unknownid_exp(
struct _tuple0*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_primop_exp(enum Cyc_Absyn_Primop,
struct Cyc_List_List*es,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_prim1_exp(
enum Cyc_Absyn_Primop,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_prim2_exp(
enum Cyc_Absyn_Primop,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_swap_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_eq_exp(struct Cyc_Absyn_Exp*,struct
Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_neq_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_gt_exp(struct
Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_lt_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_gte_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_lte_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_assignop_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Core_Opt*,struct Cyc_Absyn_Exp*,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_post_inc_exp(struct Cyc_Absyn_Exp*,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_post_dec_exp(struct Cyc_Absyn_Exp*,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_pre_inc_exp(struct Cyc_Absyn_Exp*,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_pre_dec_exp(struct Cyc_Absyn_Exp*,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_conditional_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_and_exp(struct
Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_or_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_seq_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_unknowncall_exp(
struct Cyc_Absyn_Exp*,struct Cyc_List_List*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_throw_exp(
struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_noinstantiate_exp(
struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_instantiate_exp(
struct Cyc_Absyn_Exp*,struct Cyc_List_List*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_cast_exp(
void*,struct Cyc_Absyn_Exp*,int user_cast,enum Cyc_Absyn_Coercion,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_address_exp(struct Cyc_Absyn_Exp*,unsigned int);
struct Cyc_Absyn_Exp*Cyc_Absyn_sizeoftyp_exp(void*t,unsigned int);struct Cyc_Absyn_Exp*
Cyc_Absyn_sizeofexp_exp(struct Cyc_Absyn_Exp*e,unsigned int);struct Cyc_Absyn_Exp*
Cyc_Absyn_offsetof_exp(void*,void*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_deref_exp(
struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_aggrmember_exp(
struct Cyc_Absyn_Exp*,struct _dyneither_ptr*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_aggrarrow_exp(
struct Cyc_Absyn_Exp*,struct _dyneither_ptr*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_subscript_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_tuple_exp(
struct Cyc_List_List*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_stmt_exp(struct
Cyc_Absyn_Stmt*,unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_valueof_exp(void*,
unsigned int);struct Cyc_Absyn_Exp*Cyc_Absyn_asm_exp(int volatile_kw,struct
_dyneither_ptr body,unsigned int);struct Cyc_Absyn_Stmt*Cyc_Absyn_new_stmt(void*s,
unsigned int loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_skip_stmt(unsigned int loc);
struct Cyc_Absyn_Stmt*Cyc_Absyn_exp_stmt(struct Cyc_Absyn_Exp*e,unsigned int loc);
struct Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmt(struct Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*
s2,unsigned int loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_return_stmt(struct Cyc_Absyn_Exp*
e,unsigned int loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_ifthenelse_stmt(struct Cyc_Absyn_Exp*
e,struct Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*s2,unsigned int loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_while_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Stmt*s,unsigned int
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_break_stmt(unsigned int loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_continue_stmt(unsigned int loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_for_stmt(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Absyn_Exp*e3,struct Cyc_Absyn_Stmt*
s,unsigned int loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_switch_stmt(struct Cyc_Absyn_Exp*
e,struct Cyc_List_List*,unsigned int loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_fallthru_stmt(
struct Cyc_List_List*el,unsigned int loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_do_stmt(
struct Cyc_Absyn_Stmt*s,struct Cyc_Absyn_Exp*e,unsigned int loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_goto_stmt(struct _dyneither_ptr*lab,unsigned int loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_trycatch_stmt(struct Cyc_Absyn_Stmt*,struct Cyc_List_List*,unsigned int);
struct Cyc_Absyn_Pat*Cyc_Absyn_new_pat(void*p,unsigned int s);struct Cyc_Absyn_Pat*
Cyc_Absyn_exp_pat(struct Cyc_Absyn_Exp*);struct Cyc_Absyn_Decl*Cyc_Absyn_new_decl(
void*r,unsigned int loc);struct Cyc_Absyn_Decl*Cyc_Absyn_let_decl(struct Cyc_Absyn_Pat*
p,struct Cyc_Absyn_Exp*e,unsigned int loc);struct Cyc_Absyn_Decl*Cyc_Absyn_letv_decl(
struct Cyc_List_List*,unsigned int loc);struct Cyc_Absyn_Decl*Cyc_Absyn_region_decl(
struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Vardecl*,int,struct Cyc_Absyn_Exp*,
unsigned int);struct Cyc_Absyn_Decl*Cyc_Absyn_alias_decl(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Vardecl*,unsigned int);struct Cyc_Absyn_Vardecl*
Cyc_Absyn_new_vardecl(struct _tuple0*x,void*t,struct Cyc_Absyn_Exp*init);struct Cyc_Absyn_AggrdeclImpl*
Cyc_Absyn_aggrdecl_impl(struct Cyc_List_List*exists,struct Cyc_List_List*po,struct
Cyc_List_List*fs,int tagged);struct Cyc_Absyn_TypeDecl*Cyc_Absyn_aggr_tdecl(enum 
Cyc_Absyn_AggrKind k,enum Cyc_Absyn_Scope s,struct _tuple0*n,struct Cyc_List_List*ts,
struct Cyc_Absyn_AggrdeclImpl*i,struct Cyc_List_List*atts,unsigned int loc);struct
Cyc_Absyn_Decl*Cyc_Absyn_datatype_decl(enum Cyc_Absyn_Scope s,struct _tuple0*n,
struct Cyc_List_List*ts,struct Cyc_Core_Opt*fs,int is_extensible,unsigned int loc);
struct Cyc_Absyn_TypeDecl*Cyc_Absyn_datatype_tdecl(enum Cyc_Absyn_Scope s,struct
_tuple0*n,struct Cyc_List_List*ts,struct Cyc_Core_Opt*fs,int is_extensible,
unsigned int loc);void*Cyc_Absyn_function_typ(struct Cyc_List_List*tvs,void*
eff_typ,struct Cyc_Absyn_Tqual ret_tqual,void*ret_typ,struct Cyc_List_List*args,int
c_varargs,struct Cyc_Absyn_VarargInfo*cyc_varargs,struct Cyc_List_List*rgn_po,
struct Cyc_List_List*atts);int Cyc_Absyn_fntype_att(void*a);extern int Cyc_Absyn_porting_c_code;
struct Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};typedef struct Cyc_Iter_Iter
Cyc_Iter_iter_t;int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Set_Set;
typedef struct Cyc_Set_Set*Cyc_Set_set_t;extern char Cyc_Set_Absent[7];struct Cyc_Set_Absent_struct{
char*tag;};struct Cyc_Dict_T;typedef const struct Cyc_Dict_T*Cyc_Dict_tree;struct Cyc_Dict_Dict{
int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};typedef
struct Cyc_Dict_Dict Cyc_Dict_dict_t;extern char Cyc_Dict_Present[8];struct Cyc_Dict_Present_struct{
char*tag;};extern char Cyc_Dict_Absent[7];struct Cyc_Dict_Absent_struct{char*tag;};
struct Cyc_RgnOrder_RgnPO;typedef struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_rgn_po_t;
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_initial_fn_po(struct _RegionHandle*,struct
Cyc_List_List*tvs,struct Cyc_List_List*po,void*effect,struct Cyc_Absyn_Tvar*
fst_rgn,unsigned int);struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(
struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn,unsigned int
loc);struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(struct _RegionHandle*,
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*rgn,int resetable,int opened);int
Cyc_RgnOrder_is_region_resetable(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*
r);int Cyc_RgnOrder_effect_outlives(struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);
int Cyc_RgnOrder_satisfies_constraints(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_List_List*
constraints,void*default_bound,int do_pin);int Cyc_RgnOrder_eff_outlives_eff(
struct Cyc_RgnOrder_RgnPO*po,void*eff1,void*eff2);void Cyc_RgnOrder_print_region_po(
struct Cyc_RgnOrder_RgnPO*po);struct Cyc_Tcenv_CList{void*hd;const struct Cyc_Tcenv_CList*
tl;};typedef const struct Cyc_Tcenv_CList*Cyc_Tcenv_mclist_t;typedef const struct Cyc_Tcenv_CList*
const Cyc_Tcenv_clist_t;struct Cyc_Tcenv_VarRes_struct{int tag;void*f1;};struct Cyc_Tcenv_AggrRes_struct{
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_DatatypeRes_struct{int tag;
struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct Cyc_Tcenv_EnumRes_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_struct{
int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};typedef void*Cyc_Tcenv_resolved_t;
struct Cyc_Tcenv_Genv{struct _RegionHandle*grgn;struct Cyc_Set_Set*namespaces;
struct Cyc_Dict_Dict aggrdecls;struct Cyc_Dict_Dict datatypedecls;struct Cyc_Dict_Dict
enumdecls;struct Cyc_Dict_Dict typedefs;struct Cyc_Dict_Dict ordinaries;struct Cyc_List_List*
availables;};typedef struct Cyc_Tcenv_Genv*Cyc_Tcenv_genv_t;struct Cyc_Tcenv_Fenv;
typedef struct Cyc_Tcenv_Fenv*Cyc_Tcenv_fenv_t;struct Cyc_Tcenv_NotLoop_j_struct{
int tag;};struct Cyc_Tcenv_CaseEnd_j_struct{int tag;};struct Cyc_Tcenv_FnEnd_j_struct{
int tag;};struct Cyc_Tcenv_Stmt_j_struct{int tag;struct Cyc_Absyn_Stmt*f1;};typedef
void*Cyc_Tcenv_jumpee_t;struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict
ae;struct Cyc_Tcenv_Fenv*le;int allow_valueof;};typedef struct Cyc_Tcenv_Tenv*Cyc_Tcenv_tenv_t;
void Cyc_Tcutil_warn(unsigned int,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
void*Cyc_Tcutil_copy_type(void*t);void*Cyc_Tcutil_compress(void*t);extern struct
Cyc_Absyn_Kind Cyc_Tcutil_rk;extern struct Cyc_Absyn_Kind Cyc_Tcutil_ak;extern struct
Cyc_Absyn_Kind Cyc_Tcutil_bk;extern struct Cyc_Absyn_Kind Cyc_Tcutil_mk;extern struct
Cyc_Absyn_Kind Cyc_Tcutil_ek;extern struct Cyc_Absyn_Kind Cyc_Tcutil_ik;extern struct
Cyc_Absyn_Kind Cyc_Tcutil_trk;extern struct Cyc_Absyn_Kind Cyc_Tcutil_tak;extern
struct Cyc_Absyn_Kind Cyc_Tcutil_tbk;extern struct Cyc_Absyn_Kind Cyc_Tcutil_tmk;
extern struct Cyc_Absyn_Kind Cyc_Tcutil_urk;extern struct Cyc_Absyn_Kind Cyc_Tcutil_uak;
extern struct Cyc_Absyn_Kind Cyc_Tcutil_ubk;extern struct Cyc_Absyn_Kind Cyc_Tcutil_umk;
extern struct Cyc_Core_Opt Cyc_Tcutil_rko;extern struct Cyc_Core_Opt Cyc_Tcutil_bko;
extern struct Cyc_Core_Opt Cyc_Tcutil_iko;extern struct Cyc_Core_Opt Cyc_Tcutil_trko;
struct Cyc_Core_Opt*Cyc_Tcutil_kind_to_opt(struct Cyc_Absyn_Kind*k);void*Cyc_Tcutil_kind_to_bound(
struct Cyc_Absyn_Kind*k);struct Cyc_Absyn_Tvar*Cyc_Tcutil_new_tvar(void*k);void Cyc_Tcutil_check_no_qual(
unsigned int loc,void*t);struct Cyc_PP_Ppstate;typedef struct Cyc_PP_Ppstate*Cyc_PP_ppstate_t;
struct Cyc_PP_Out;typedef struct Cyc_PP_Out*Cyc_PP_out_t;struct Cyc_PP_Doc;typedef
struct Cyc_PP_Doc*Cyc_PP_doc_t;struct Cyc_Absynpp_Params{int expand_typedefs;int
qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int
print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int
print_externC_stmts;int print_full_evars;int print_zeroterm;int
generate_line_directives;int use_curr_namespace;struct Cyc_List_List*
curr_namespace;};void Cyc_Lex_register_typedef(struct _tuple0*s);void Cyc_Lex_enter_namespace(
struct _dyneither_ptr*);void Cyc_Lex_leave_namespace();void Cyc_Lex_enter_using(
struct _tuple0*);void Cyc_Lex_leave_using();struct _tuple10{struct _tuple10*tl;};
struct Cyc_Parse_FlatList{struct _tuple10*tl;};typedef struct _tuple10*Cyc_Parse_flat_list_t;
struct _tuple10*Cyc_Parse_flat_imp_rev(struct _tuple10*x){if(x == 0)return x;else{
struct _tuple10*first=x;struct _tuple10*second=x->tl;x->tl=0;while(second != 0){
struct _tuple10*temp=second->tl;second->tl=first;first=second;second=temp;}return
first;}}int Cyc_Parse_no_register=0;struct Cyc_Parse_Type_specifier{int Signed_spec:
1;int Unsigned_spec: 1;int Short_spec: 1;int Long_spec: 1;int Long_Long_spec: 1;int
Valid_type_spec: 1;void*Type_spec;unsigned int loc;};typedef struct Cyc_Parse_Type_specifier
Cyc_Parse_type_specifier_t;enum Cyc_Parse_Storage_class{Cyc_Parse_Typedef_sc  = 0,
Cyc_Parse_Extern_sc  = 1,Cyc_Parse_ExternC_sc  = 2,Cyc_Parse_Static_sc  = 3,Cyc_Parse_Auto_sc
 = 4,Cyc_Parse_Register_sc  = 5,Cyc_Parse_Abstract_sc  = 6};typedef enum Cyc_Parse_Storage_class
Cyc_Parse_storage_class_t;struct Cyc_Parse_Declaration_spec{enum Cyc_Parse_Storage_class*
sc;struct Cyc_Absyn_Tqual tq;struct Cyc_Parse_Type_specifier type_specs;int is_inline;
struct Cyc_List_List*attributes;};typedef struct Cyc_Parse_Declaration_spec Cyc_Parse_decl_spec_t;
struct Cyc_Parse_Declarator{struct _tuple0*id;struct Cyc_List_List*tms;};typedef
struct Cyc_Parse_Declarator Cyc_Parse_declarator_t;struct _tuple11{struct _tuple11*
tl;struct Cyc_Parse_Declarator hd;};typedef struct _tuple11*Cyc_Parse_declarators_t;
struct _tuple12{struct Cyc_Parse_Declarator f1;struct Cyc_Absyn_Exp*f2;};struct
_tuple13{struct _tuple13*tl;struct _tuple12 hd;};typedef struct _tuple13*Cyc_Parse_declarator_list_t;
static void Cyc_Parse_decl_split(struct _RegionHandle*r,struct _tuple13*ds,struct
_tuple11**decls,struct Cyc_List_List**es){struct _tuple11*declarators=0;struct Cyc_List_List*
exprs=0;for(0;ds != 0;ds=ds->tl){struct Cyc_Parse_Declarator _tmp1;struct Cyc_Absyn_Exp*
_tmp2;struct _tuple12 _tmp0=ds->hd;_tmp1=_tmp0.f1;_tmp2=_tmp0.f2;{struct _tuple11*
_tmp84A;declarators=((_tmp84A=_region_malloc(r,sizeof(*_tmp84A)),((_tmp84A->tl=
declarators,((_tmp84A->hd=_tmp1,_tmp84A))))));}{struct Cyc_List_List*_tmp84B;
exprs=((_tmp84B=_region_malloc(r,sizeof(*_tmp84B)),((_tmp84B->hd=_tmp2,((_tmp84B->tl=
exprs,_tmp84B))))));};}*es=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(
exprs);*decls=((struct _tuple11*(*)(struct _tuple11*x))Cyc_Parse_flat_imp_rev)(
declarators);}struct Cyc_Parse_Abstractdeclarator{struct Cyc_List_List*tms;};
typedef struct Cyc_Parse_Abstractdeclarator Cyc_Parse_abstractdeclarator_t;static
void*Cyc_Parse_collapse_type_specifiers(struct Cyc_Parse_Type_specifier ts,
unsigned int loc);struct _tuple14{struct Cyc_Absyn_Tqual f1;void*f2;struct Cyc_List_List*
f3;struct Cyc_List_List*f4;};static struct _tuple14 Cyc_Parse_apply_tms(struct Cyc_Absyn_Tqual,
void*,struct Cyc_List_List*,struct Cyc_List_List*);static struct Cyc_List_List*Cyc_Parse_parse_result=
0;static void Cyc_Parse_err(struct _dyneither_ptr msg,unsigned int sg){Cyc_Position_post_error(
Cyc_Position_mk_err_parse(sg,msg));}static void*Cyc_Parse_abort(unsigned int sg,
struct _dyneither_ptr fmt,struct _dyneither_ptr ap){Cyc_Parse_err((struct
_dyneither_ptr)Cyc_vrprintf(Cyc_Core_heap_region,fmt,ap),sg);{struct Cyc_Position_Exit_struct
_tmp84E;struct Cyc_Position_Exit_struct*_tmp84D;(int)_throw((void*)((_tmp84D=
_cycalloc_atomic(sizeof(*_tmp84D)),((_tmp84D[0]=((_tmp84E.tag=Cyc_Position_Exit,
_tmp84E)),_tmp84D)))));};}static void Cyc_Parse_unimp(struct _dyneither_ptr msg,
unsigned int sg){{const char*_tmp853;void*_tmp852[2];struct Cyc_String_pa_struct
_tmp851;struct Cyc_String_pa_struct _tmp850;(_tmp850.tag=0,((_tmp850.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)msg),((_tmp851.tag=0,((_tmp851.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Position_string_of_segment(sg)),((
_tmp852[0]=& _tmp851,((_tmp852[1]=& _tmp850,Cyc_fprintf(Cyc_stderr,((_tmp853="%s: Warning: Cyclone does not yet support %s\n",
_tag_dyneither(_tmp853,sizeof(char),46))),_tag_dyneither(_tmp852,sizeof(void*),2)))))))))))));}
return;}static void*Cyc_Parse_type_name_to_type(struct _tuple8*tqt,unsigned int loc){
struct _tuple8 _tmpC;struct Cyc_Absyn_Tqual _tmpD;void*_tmpE;struct _tuple8*_tmpB=tqt;
_tmpC=*_tmpB;_tmpD=_tmpC.f2;_tmpE=_tmpC.f3;if((_tmpD.print_const  || _tmpD.q_volatile)
 || _tmpD.q_restrict){if(_tmpD.loc != 0)loc=_tmpD.loc;{const char*_tmp856;void*
_tmp855;(_tmp855=0,Cyc_Tcutil_warn(loc,((_tmp856="qualifier on type is ignored",
_tag_dyneither(_tmp856,sizeof(char),29))),_tag_dyneither(_tmp855,sizeof(void*),0)));};}
return _tmpE;}struct _tuple15{union Cyc_Absyn_Constraint*f1;union Cyc_Absyn_Constraint*
f2;union Cyc_Absyn_Constraint*f3;void*f4;};static struct _tuple15 Cyc_Parse_collapse_pointer_quals(
unsigned int loc,union Cyc_Absyn_Constraint*nullable,union Cyc_Absyn_Constraint*
bound,void*rgn,struct Cyc_List_List*pqs){union Cyc_Absyn_Constraint*zeroterm=((
union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)();for(0;pqs != 0;pqs=pqs->tl){
void*_tmp11=(void*)pqs->hd;struct Cyc_Absyn_Exp*_tmp19;void*_tmp1B;_LL1: {struct
Cyc_Absyn_Zeroterm_ptrqual_struct*_tmp12=(struct Cyc_Absyn_Zeroterm_ptrqual_struct*)
_tmp11;if(_tmp12->tag != 4)goto _LL3;}_LL2: zeroterm=Cyc_Absyn_true_conref;goto _LL0;
_LL3: {struct Cyc_Absyn_Nozeroterm_ptrqual_struct*_tmp13=(struct Cyc_Absyn_Nozeroterm_ptrqual_struct*)
_tmp11;if(_tmp13->tag != 5)goto _LL5;}_LL4: zeroterm=Cyc_Absyn_false_conref;goto
_LL0;_LL5: {struct Cyc_Absyn_Nullable_ptrqual_struct*_tmp14=(struct Cyc_Absyn_Nullable_ptrqual_struct*)
_tmp11;if(_tmp14->tag != 7)goto _LL7;}_LL6: nullable=Cyc_Absyn_true_conref;goto _LL0;
_LL7: {struct Cyc_Absyn_Notnull_ptrqual_struct*_tmp15=(struct Cyc_Absyn_Notnull_ptrqual_struct*)
_tmp11;if(_tmp15->tag != 6)goto _LL9;}_LL8: nullable=Cyc_Absyn_false_conref;goto
_LL0;_LL9: {struct Cyc_Absyn_Fat_ptrqual_struct*_tmp16=(struct Cyc_Absyn_Fat_ptrqual_struct*)
_tmp11;if(_tmp16->tag != 3)goto _LLB;}_LLA: bound=Cyc_Absyn_bounds_dyneither_conref;
goto _LL0;_LLB: {struct Cyc_Absyn_Thin_ptrqual_struct*_tmp17=(struct Cyc_Absyn_Thin_ptrqual_struct*)
_tmp11;if(_tmp17->tag != 2)goto _LLD;}_LLC: bound=Cyc_Absyn_bounds_one_conref;goto
_LL0;_LLD: {struct Cyc_Absyn_Numelts_ptrqual_struct*_tmp18=(struct Cyc_Absyn_Numelts_ptrqual_struct*)
_tmp11;if(_tmp18->tag != 0)goto _LLF;else{_tmp19=_tmp18->f1;}}_LLE:{struct Cyc_Absyn_Upper_b_struct
_tmp859;struct Cyc_Absyn_Upper_b_struct*_tmp858;bound=((union Cyc_Absyn_Constraint*(*)(
void*x))Cyc_Absyn_new_conref)((void*)((_tmp858=_cycalloc(sizeof(*_tmp858)),((
_tmp858[0]=((_tmp859.tag=1,((_tmp859.f1=_tmp19,_tmp859)))),_tmp858)))));}goto
_LL0;_LLF: {struct Cyc_Absyn_Region_ptrqual_struct*_tmp1A=(struct Cyc_Absyn_Region_ptrqual_struct*)
_tmp11;if(_tmp1A->tag != 1)goto _LL0;else{_tmp1B=(void*)_tmp1A->f1;}}_LL10: rgn=
_tmp1B;goto _LL0;_LL0:;}{struct _tuple15 _tmp85A;return(_tmp85A.f1=nullable,((
_tmp85A.f2=bound,((_tmp85A.f3=zeroterm,((_tmp85A.f4=rgn,_tmp85A)))))));};}struct
_tuple0*Cyc_Parse_gensym_enum(){static int enum_counter=0;struct _dyneither_ptr*
_tmp867;const char*_tmp866;void*_tmp865[1];struct Cyc_Int_pa_struct _tmp864;struct
_tuple0*_tmp863;return(_tmp863=_cycalloc(sizeof(*_tmp863)),((_tmp863->f1=Cyc_Absyn_Rel_n(
0),((_tmp863->f2=((_tmp867=_cycalloc(sizeof(*_tmp867)),((_tmp867[0]=(struct
_dyneither_ptr)((_tmp864.tag=1,((_tmp864.f1=(unsigned long)enum_counter ++,((
_tmp865[0]=& _tmp864,Cyc_aprintf(((_tmp866="__anonymous_enum_%d__",_tag_dyneither(
_tmp866,sizeof(char),22))),_tag_dyneither(_tmp865,sizeof(void*),1)))))))),
_tmp867)))),_tmp863)))));}struct _tuple16{struct _tuple0*f1;struct Cyc_Absyn_Tqual
f2;void*f3;struct Cyc_List_List*f4;struct Cyc_List_List*f5;};struct _tuple17{struct
_tuple16*f1;struct Cyc_Absyn_Exp*f2;};static struct Cyc_Absyn_Aggrfield*Cyc_Parse_make_aggr_field(
unsigned int loc,struct _tuple17*field_info){struct _tuple17 _tmp25;struct _tuple16*
_tmp26;struct _tuple16 _tmp27;struct _tuple0*_tmp28;struct Cyc_Absyn_Tqual _tmp29;
void*_tmp2A;struct Cyc_List_List*_tmp2B;struct Cyc_List_List*_tmp2C;struct Cyc_Absyn_Exp*
_tmp2D;struct _tuple17*_tmp24=field_info;_tmp25=*_tmp24;_tmp26=_tmp25.f1;_tmp27=*
_tmp26;_tmp28=_tmp27.f1;_tmp29=_tmp27.f2;_tmp2A=_tmp27.f3;_tmp2B=_tmp27.f4;
_tmp2C=_tmp27.f5;_tmp2D=_tmp25.f2;if(_tmp2B != 0){const char*_tmp868;Cyc_Parse_err(((
_tmp868="bad type params in struct field",_tag_dyneither(_tmp868,sizeof(char),32))),
loc);}if(Cyc_Absyn_is_qvar_qualified(_tmp28)){const char*_tmp869;Cyc_Parse_err(((
_tmp869="struct or union field cannot be qualified with a namespace",
_tag_dyneither(_tmp869,sizeof(char),59))),loc);}{struct Cyc_Absyn_Aggrfield*
_tmp86A;return(_tmp86A=_cycalloc(sizeof(*_tmp86A)),((_tmp86A->name=(*_tmp28).f2,((
_tmp86A->tq=_tmp29,((_tmp86A->type=_tmp2A,((_tmp86A->width=_tmp2D,((_tmp86A->attributes=
_tmp2C,_tmp86A)))))))))));};}static struct Cyc_Parse_Type_specifier Cyc_Parse_empty_spec(
unsigned int loc){struct Cyc_Parse_Type_specifier _tmp86B;return(_tmp86B.Signed_spec=
0,((_tmp86B.Unsigned_spec=0,((_tmp86B.Short_spec=0,((_tmp86B.Long_spec=0,((
_tmp86B.Long_Long_spec=0,((_tmp86B.Valid_type_spec=0,((_tmp86B.Type_spec=Cyc_Absyn_sint_typ,((
_tmp86B.loc=loc,_tmp86B)))))))))))))));}static struct Cyc_Parse_Type_specifier Cyc_Parse_type_spec(
void*t,unsigned int loc){struct Cyc_Parse_Type_specifier _tmp32=Cyc_Parse_empty_spec(
loc);_tmp32.Type_spec=t;_tmp32.Valid_type_spec=1;return _tmp32;}static struct Cyc_Parse_Type_specifier
Cyc_Parse_signed_spec(unsigned int loc){struct Cyc_Parse_Type_specifier _tmp33=Cyc_Parse_empty_spec(
loc);_tmp33.Signed_spec=1;return _tmp33;}static struct Cyc_Parse_Type_specifier Cyc_Parse_unsigned_spec(
unsigned int loc){struct Cyc_Parse_Type_specifier _tmp34=Cyc_Parse_empty_spec(loc);
_tmp34.Unsigned_spec=1;return _tmp34;}static struct Cyc_Parse_Type_specifier Cyc_Parse_short_spec(
unsigned int loc){struct Cyc_Parse_Type_specifier _tmp35=Cyc_Parse_empty_spec(loc);
_tmp35.Short_spec=1;return _tmp35;}static struct Cyc_Parse_Type_specifier Cyc_Parse_long_spec(
unsigned int loc){struct Cyc_Parse_Type_specifier _tmp36=Cyc_Parse_empty_spec(loc);
_tmp36.Long_spec=1;return _tmp36;}static void*Cyc_Parse_array2ptr(void*t,int
argposn){void*_tmp37=t;struct Cyc_Absyn_ArrayInfo _tmp39;void*_tmp3A;struct Cyc_Absyn_Tqual
_tmp3B;struct Cyc_Absyn_Exp*_tmp3C;union Cyc_Absyn_Constraint*_tmp3D;unsigned int
_tmp3E;_LL12: {struct Cyc_Absyn_ArrayType_struct*_tmp38=(struct Cyc_Absyn_ArrayType_struct*)
_tmp37;if(_tmp38->tag != 8)goto _LL14;else{_tmp39=_tmp38->f1;_tmp3A=_tmp39.elt_type;
_tmp3B=_tmp39.tq;_tmp3C=_tmp39.num_elts;_tmp3D=_tmp39.zero_term;_tmp3E=_tmp39.zt_loc;}}
_LL13: {struct Cyc_Absyn_Upper_b_struct _tmp86E;struct Cyc_Absyn_Upper_b_struct*
_tmp86D;return Cyc_Absyn_starb_typ(_tmp3A,argposn?Cyc_Absyn_new_evar((struct Cyc_Core_Opt*)&
Cyc_Tcutil_rko,0):(void*)& Cyc_Absyn_HeapRgn_val,_tmp3B,_tmp3C == 0?(void*)& Cyc_Absyn_DynEither_b_val:(
void*)((void*)((_tmp86D=_cycalloc(sizeof(*_tmp86D)),((_tmp86D[0]=((_tmp86E.tag=1,((
_tmp86E.f1=(struct Cyc_Absyn_Exp*)_tmp3C,_tmp86E)))),_tmp86D))))),_tmp3D);}_LL14:;
_LL15: return t;_LL11:;}struct _tuple18{struct _dyneither_ptr*f1;void*f2;};static
struct Cyc_List_List*Cyc_Parse_get_arg_tags(struct Cyc_List_List*x){struct Cyc_List_List*
_tmp41=0;for(0;x != 0;x=x->tl){struct _tuple8*_tmp42=(struct _tuple8*)x->hd;struct
_tuple8 _tmp43;struct _dyneither_ptr*_tmp44;void*_tmp45;void*_tmp47;struct _tuple8
_tmp48;struct _dyneither_ptr*_tmp49;struct _dyneither_ptr _tmp4A;void*_tmp4B;void*
_tmp4D;void*_tmp4F;void**_tmp50;_LL17: _tmp43=*_tmp42;_tmp44=_tmp43.f1;_tmp45=
_tmp43.f3;{struct Cyc_Absyn_TagType_struct*_tmp46=(struct Cyc_Absyn_TagType_struct*)
_tmp45;if(_tmp46->tag != 19)goto _LL19;else{_tmp47=(void*)_tmp46->f1;}};if(!(
_tmp44 != 0))goto _LL19;_LL18:{void*_tmp51=_tmp47;void*_tmp53;void**_tmp54;_LL1E: {
struct Cyc_Absyn_Evar_struct*_tmp52=(struct Cyc_Absyn_Evar_struct*)_tmp51;if(
_tmp52->tag != 1)goto _LL20;else{_tmp53=(void**)& _tmp52->f2;_tmp54=(void**)((void**)&
_tmp52->f2);}}_LL1F: {struct Cyc_String_pa_struct _tmp876;void*_tmp875[1];const
char*_tmp874;struct _dyneither_ptr*_tmp873;struct _dyneither_ptr*nm=(_tmp873=
_cycalloc(sizeof(*_tmp873)),((_tmp873[0]=(struct _dyneither_ptr)((_tmp876.tag=0,((
_tmp876.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*((struct _dyneither_ptr*)
_tmp44)),((_tmp875[0]=& _tmp876,Cyc_aprintf(((_tmp874="`%s",_tag_dyneither(
_tmp874,sizeof(char),4))),_tag_dyneither(_tmp875,sizeof(void*),1)))))))),_tmp873)));{
struct Cyc_Absyn_VarType_struct _tmp885;struct Cyc_Absyn_Eq_kb_struct*_tmp884;
struct Cyc_Absyn_Eq_kb_struct _tmp883;struct Cyc_Absyn_Tvar*_tmp882;struct Cyc_Absyn_VarType_struct*
_tmp881;*_tmp54=(void*)((_tmp881=_cycalloc(sizeof(*_tmp881)),((_tmp881[0]=((
_tmp885.tag=2,((_tmp885.f1=((_tmp882=_cycalloc(sizeof(*_tmp882)),((_tmp882->name=
nm,((_tmp882->identity=- 1,((_tmp882->kind=(void*)((_tmp884=_cycalloc(sizeof(*
_tmp884)),((_tmp884[0]=((_tmp883.tag=0,((_tmp883.f1=& Cyc_Tcutil_ik,_tmp883)))),
_tmp884)))),_tmp882)))))))),_tmp885)))),_tmp881))));}goto _LL1D;}_LL20:;_LL21:
goto _LL1D;_LL1D:;}{struct _tuple18*_tmp888;struct Cyc_List_List*_tmp887;_tmp41=((
_tmp887=_cycalloc(sizeof(*_tmp887)),((_tmp887->hd=((_tmp888=_cycalloc(sizeof(*
_tmp888)),((_tmp888->f1=(struct _dyneither_ptr*)_tmp44,((_tmp888->f2=_tmp47,
_tmp888)))))),((_tmp887->tl=_tmp41,_tmp887))))));}goto _LL16;_LL19: _tmp48=*_tmp42;
_tmp49=_tmp48.f1;if(_tmp49 == 0)goto _LL1B;_tmp4A=*_tmp49;_tmp4B=_tmp48.f3;{struct
Cyc_Absyn_RgnHandleType_struct*_tmp4C=(struct Cyc_Absyn_RgnHandleType_struct*)
_tmp4B;if(_tmp4C->tag != 15)goto _LL1B;else{_tmp4D=(void*)_tmp4C->f1;{struct Cyc_Absyn_Evar_struct*
_tmp4E=(struct Cyc_Absyn_Evar_struct*)_tmp4D;if(_tmp4E->tag != 1)goto _LL1B;else{
_tmp4F=(void**)& _tmp4E->f2;_tmp50=(void**)((void**)& _tmp4E->f2);}};}};_LL1A: {
struct Cyc_String_pa_struct _tmp890;void*_tmp88F[1];const char*_tmp88E;struct
_dyneither_ptr*_tmp88D;struct _dyneither_ptr*nm=(_tmp88D=_cycalloc(sizeof(*
_tmp88D)),((_tmp88D[0]=(struct _dyneither_ptr)((_tmp890.tag=0,((_tmp890.f1=(
struct _dyneither_ptr)((struct _dyneither_ptr)_tmp4A),((_tmp88F[0]=& _tmp890,Cyc_aprintf(((
_tmp88E="`%s",_tag_dyneither(_tmp88E,sizeof(char),4))),_tag_dyneither(_tmp88F,
sizeof(void*),1)))))))),_tmp88D)));{struct Cyc_Absyn_VarType_struct _tmp89F;struct
Cyc_Absyn_Eq_kb_struct*_tmp89E;struct Cyc_Absyn_Eq_kb_struct _tmp89D;struct Cyc_Absyn_Tvar*
_tmp89C;struct Cyc_Absyn_VarType_struct*_tmp89B;*_tmp50=(void*)((_tmp89B=
_cycalloc(sizeof(*_tmp89B)),((_tmp89B[0]=((_tmp89F.tag=2,((_tmp89F.f1=((_tmp89C=
_cycalloc(sizeof(*_tmp89C)),((_tmp89C->name=nm,((_tmp89C->identity=- 1,((_tmp89C->kind=(
void*)((_tmp89E=_cycalloc(sizeof(*_tmp89E)),((_tmp89E[0]=((_tmp89D.tag=0,((
_tmp89D.f1=& Cyc_Tcutil_rk,_tmp89D)))),_tmp89E)))),_tmp89C)))))))),_tmp89F)))),
_tmp89B))));}goto _LL16;}_LL1B:;_LL1C: goto _LL16;_LL16:;}return _tmp41;}static
struct Cyc_List_List*Cyc_Parse_get_argrfield_tags(struct Cyc_List_List*x){struct
Cyc_List_List*_tmp69=0;for(0;x != 0;x=x->tl){void*_tmp6A=((struct Cyc_Absyn_Aggrfield*)
x->hd)->type;void*_tmp6C;_LL23: {struct Cyc_Absyn_TagType_struct*_tmp6B=(struct
Cyc_Absyn_TagType_struct*)_tmp6A;if(_tmp6B->tag != 19)goto _LL25;else{_tmp6C=(void*)
_tmp6B->f1;}}_LL24:{struct _tuple18*_tmp8A2;struct Cyc_List_List*_tmp8A1;_tmp69=((
_tmp8A1=_cycalloc(sizeof(*_tmp8A1)),((_tmp8A1->hd=((_tmp8A2=_cycalloc(sizeof(*
_tmp8A2)),((_tmp8A2->f1=((struct Cyc_Absyn_Aggrfield*)x->hd)->name,((_tmp8A2->f2=
_tmp6C,_tmp8A2)))))),((_tmp8A1->tl=_tmp69,_tmp8A1))))));}goto _LL22;_LL25:;_LL26:
goto _LL22;_LL22:;}return _tmp69;}static struct Cyc_Absyn_Exp*Cyc_Parse_substitute_tags_exp(
struct Cyc_List_List*tags,struct Cyc_Absyn_Exp*e){{void*_tmp6F=e->r;struct _tuple0*
_tmp71;struct _tuple0 _tmp72;union Cyc_Absyn_Nmspace _tmp73;struct Cyc_List_List*
_tmp74;struct _dyneither_ptr*_tmp75;_LL28: {struct Cyc_Absyn_Var_e_struct*_tmp70=(
struct Cyc_Absyn_Var_e_struct*)_tmp6F;if(_tmp70->tag != 1)goto _LL2A;else{_tmp71=
_tmp70->f1;_tmp72=*_tmp71;_tmp73=_tmp72.f1;if((_tmp73.Rel_n).tag != 1)goto _LL2A;
_tmp74=(struct Cyc_List_List*)(_tmp73.Rel_n).val;if(_tmp74 != 0)goto _LL2A;_tmp75=
_tmp72.f2;}}_LL29:{struct Cyc_List_List*_tmp76=tags;for(0;_tmp76 != 0;_tmp76=
_tmp76->tl){struct _tuple18 _tmp78;struct _dyneither_ptr*_tmp79;void*_tmp7A;struct
_tuple18*_tmp77=(struct _tuple18*)_tmp76->hd;_tmp78=*_tmp77;_tmp79=_tmp78.f1;
_tmp7A=_tmp78.f2;if(Cyc_strptrcmp(_tmp79,_tmp75)== 0){struct Cyc_Absyn_Valueof_e_struct
_tmp8A5;struct Cyc_Absyn_Valueof_e_struct*_tmp8A4;return Cyc_Absyn_new_exp((void*)((
_tmp8A4=_cycalloc(sizeof(*_tmp8A4)),((_tmp8A4[0]=((_tmp8A5.tag=37,((_tmp8A5.f1=(
void*)Cyc_Tcutil_copy_type(_tmp7A),_tmp8A5)))),_tmp8A4)))),e->loc);}}}goto _LL27;
_LL2A:;_LL2B: goto _LL27;_LL27:;}return e;}static void*Cyc_Parse_substitute_tags(
struct Cyc_List_List*tags,void*t){{void*_tmp7D=t;struct Cyc_Absyn_ArrayInfo _tmp7F;
void*_tmp80;struct Cyc_Absyn_Tqual _tmp81;struct Cyc_Absyn_Exp*_tmp82;union Cyc_Absyn_Constraint*
_tmp83;unsigned int _tmp84;struct Cyc_Absyn_PtrInfo _tmp86;void*_tmp87;struct Cyc_Absyn_Tqual
_tmp88;struct Cyc_Absyn_PtrAtts _tmp89;void*_tmp8A;union Cyc_Absyn_Constraint*
_tmp8B;union Cyc_Absyn_Constraint*_tmp8C;union Cyc_Absyn_Constraint*_tmp8D;struct
Cyc_Absyn_PtrLoc*_tmp8E;_LL2D: {struct Cyc_Absyn_ArrayType_struct*_tmp7E=(struct
Cyc_Absyn_ArrayType_struct*)_tmp7D;if(_tmp7E->tag != 8)goto _LL2F;else{_tmp7F=
_tmp7E->f1;_tmp80=_tmp7F.elt_type;_tmp81=_tmp7F.tq;_tmp82=_tmp7F.num_elts;_tmp83=
_tmp7F.zero_term;_tmp84=_tmp7F.zt_loc;}}_LL2E: {struct Cyc_Absyn_Exp*nelts2=
_tmp82;if(_tmp82 != 0)nelts2=(struct Cyc_Absyn_Exp*)Cyc_Parse_substitute_tags_exp(
tags,(struct Cyc_Absyn_Exp*)_tmp82);{void*_tmp8F=Cyc_Parse_substitute_tags(tags,
_tmp80);if(_tmp82 != nelts2  || _tmp80 != _tmp8F){struct Cyc_Absyn_ArrayType_struct
_tmp8AB;struct Cyc_Absyn_ArrayInfo _tmp8AA;struct Cyc_Absyn_ArrayType_struct*
_tmp8A9;return(void*)((_tmp8A9=_cycalloc(sizeof(*_tmp8A9)),((_tmp8A9[0]=((
_tmp8AB.tag=8,((_tmp8AB.f1=((_tmp8AA.elt_type=_tmp8F,((_tmp8AA.tq=_tmp81,((
_tmp8AA.num_elts=nelts2,((_tmp8AA.zero_term=_tmp83,((_tmp8AA.zt_loc=_tmp84,
_tmp8AA)))))))))),_tmp8AB)))),_tmp8A9))));}goto _LL2C;};}_LL2F: {struct Cyc_Absyn_PointerType_struct*
_tmp85=(struct Cyc_Absyn_PointerType_struct*)_tmp7D;if(_tmp85->tag != 5)goto _LL31;
else{_tmp86=_tmp85->f1;_tmp87=_tmp86.elt_typ;_tmp88=_tmp86.elt_tq;_tmp89=_tmp86.ptr_atts;
_tmp8A=_tmp89.rgn;_tmp8B=_tmp89.nullable;_tmp8C=_tmp89.bounds;_tmp8D=_tmp89.zero_term;
_tmp8E=_tmp89.ptrloc;}}_LL30: {void*_tmp93=Cyc_Parse_substitute_tags(tags,_tmp87);
union Cyc_Absyn_Constraint*_tmp94=_tmp8C;{union Cyc_Absyn_Constraint _tmp95=*_tmp8C;
void*_tmp96;struct Cyc_Absyn_Exp*_tmp98;_LL34: if((_tmp95.Eq_constr).tag != 1)goto
_LL36;_tmp96=(void*)(_tmp95.Eq_constr).val;{struct Cyc_Absyn_Upper_b_struct*
_tmp97=(struct Cyc_Absyn_Upper_b_struct*)_tmp96;if(_tmp97->tag != 1)goto _LL36;
else{_tmp98=_tmp97->f1;}};_LL35: _tmp98=Cyc_Parse_substitute_tags_exp(tags,_tmp98);{
struct Cyc_Absyn_Upper_b_struct*_tmp8B1;struct Cyc_Absyn_Upper_b_struct _tmp8B0;
union Cyc_Absyn_Constraint*_tmp8AF;_tmp94=((_tmp8AF=_cycalloc(sizeof(*_tmp8AF)),(((
_tmp8AF->Eq_constr).val=(void*)((_tmp8B1=_cycalloc(sizeof(*_tmp8B1)),((_tmp8B1[0]=((
_tmp8B0.tag=1,((_tmp8B0.f1=_tmp98,_tmp8B0)))),_tmp8B1)))),(((_tmp8AF->Eq_constr).tag=
1,_tmp8AF))))));}goto _LL33;_LL36:;_LL37: goto _LL33;_LL33:;}if(_tmp93 != _tmp87  || 
_tmp94 != _tmp8C){struct Cyc_Absyn_PointerType_struct _tmp8BB;struct Cyc_Absyn_PtrAtts
_tmp8BA;struct Cyc_Absyn_PtrInfo _tmp8B9;struct Cyc_Absyn_PointerType_struct*
_tmp8B8;return(void*)((_tmp8B8=_cycalloc(sizeof(*_tmp8B8)),((_tmp8B8[0]=((
_tmp8BB.tag=5,((_tmp8BB.f1=((_tmp8B9.elt_typ=_tmp93,((_tmp8B9.elt_tq=_tmp88,((
_tmp8B9.ptr_atts=((_tmp8BA.rgn=_tmp8A,((_tmp8BA.nullable=_tmp8B,((_tmp8BA.bounds=
_tmp94,((_tmp8BA.zero_term=_tmp8D,((_tmp8BA.ptrloc=_tmp8E,_tmp8BA)))))))))),
_tmp8B9)))))),_tmp8BB)))),_tmp8B8))));}goto _LL2C;}_LL31:;_LL32: goto _LL2C;_LL2C:;}
return t;}static void Cyc_Parse_substitute_aggrfield_tags(struct Cyc_List_List*tags,
struct Cyc_Absyn_Aggrfield*x){x->type=Cyc_Parse_substitute_tags(tags,x->type);}
struct _tuple19{struct Cyc_Absyn_Tqual f1;void*f2;};static struct _tuple19*Cyc_Parse_get_tqual_typ(
unsigned int loc,struct _tuple8*t){struct _tuple19*_tmp8BC;return(_tmp8BC=_cycalloc(
sizeof(*_tmp8BC)),((_tmp8BC->f1=(*t).f2,((_tmp8BC->f2=(*t).f3,_tmp8BC)))));}
static struct Cyc_Absyn_Vardecl*Cyc_Parse_decl2vardecl(struct Cyc_Absyn_Decl*d){
void*_tmpA1=d->r;struct Cyc_Absyn_Vardecl*_tmpA3;_LL39: {struct Cyc_Absyn_Var_d_struct*
_tmpA2=(struct Cyc_Absyn_Var_d_struct*)_tmpA1;if(_tmpA2->tag != 0)goto _LL3B;else{
_tmpA3=_tmpA2->f1;}}_LL3A: return _tmpA3;_LL3B:;_LL3C: {const char*_tmp8BF;void*
_tmp8BE;(_tmp8BE=0,((int(*)(unsigned int sg,struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Parse_abort)(d->loc,((_tmp8BF="bad declaration in `forarray' statement",
_tag_dyneither(_tmp8BF,sizeof(char),40))),_tag_dyneither(_tmp8BE,sizeof(void*),0)));}
_LL38:;}static int Cyc_Parse_is_typeparam(void*tm){void*_tmpA6=tm;_LL3E: {struct
Cyc_Absyn_TypeParams_mod_struct*_tmpA7=(struct Cyc_Absyn_TypeParams_mod_struct*)
_tmpA6;if(_tmpA7->tag != 4)goto _LL40;}_LL3F: return 1;_LL40:;_LL41: return 0;_LL3D:;}
static void*Cyc_Parse_id2type(struct _dyneither_ptr s,void*k){const char*_tmp8C0;if(
Cyc_zstrcmp((struct _dyneither_ptr)s,((_tmp8C0="`H",_tag_dyneither(_tmp8C0,
sizeof(char),3))))== 0)return(void*)& Cyc_Absyn_HeapRgn_val;else{const char*
_tmp8C1;if(Cyc_zstrcmp((struct _dyneither_ptr)s,((_tmp8C1="`U",_tag_dyneither(
_tmp8C1,sizeof(char),3))))== 0)return(void*)& Cyc_Absyn_UniqueRgn_val;else{const
char*_tmp8C2;if(Cyc_zstrcmp((struct _dyneither_ptr)s,((_tmp8C2="`RC",
_tag_dyneither(_tmp8C2,sizeof(char),4))))== 0)return(void*)& Cyc_Absyn_RefCntRgn_val;
else{struct Cyc_Absyn_VarType_struct _tmp8CC;struct _dyneither_ptr*_tmp8CB;struct
Cyc_Absyn_Tvar*_tmp8CA;struct Cyc_Absyn_VarType_struct*_tmp8C9;return(void*)((
_tmp8C9=_cycalloc(sizeof(*_tmp8C9)),((_tmp8C9[0]=((_tmp8CC.tag=2,((_tmp8CC.f1=((
_tmp8CA=_cycalloc(sizeof(*_tmp8CA)),((_tmp8CA->name=((_tmp8CB=_cycalloc(sizeof(*
_tmp8CB)),((_tmp8CB[0]=s,_tmp8CB)))),((_tmp8CA->identity=- 1,((_tmp8CA->kind=k,
_tmp8CA)))))))),_tmp8CC)))),_tmp8C9))));}}}}static struct Cyc_Absyn_Tvar*Cyc_Parse_copy_tvar(
struct Cyc_Absyn_Tvar*t){void*k;{void*_tmpAF=Cyc_Absyn_compress_kb(t->kind);
struct Cyc_Absyn_Kind*_tmpB2;_LL43: {struct Cyc_Absyn_Unknown_kb_struct*_tmpB0=(
struct Cyc_Absyn_Unknown_kb_struct*)_tmpAF;if(_tmpB0->tag != 1)goto _LL45;}_LL44:{
struct Cyc_Absyn_Unknown_kb_struct _tmp8CF;struct Cyc_Absyn_Unknown_kb_struct*
_tmp8CE;k=(void*)((_tmp8CE=_cycalloc(sizeof(*_tmp8CE)),((_tmp8CE[0]=((_tmp8CF.tag=
1,((_tmp8CF.f1=0,_tmp8CF)))),_tmp8CE))));}goto _LL42;_LL45: {struct Cyc_Absyn_Less_kb_struct*
_tmpB1=(struct Cyc_Absyn_Less_kb_struct*)_tmpAF;if(_tmpB1->tag != 2)goto _LL47;
else{_tmpB2=_tmpB1->f2;}}_LL46:{struct Cyc_Absyn_Less_kb_struct _tmp8D2;struct Cyc_Absyn_Less_kb_struct*
_tmp8D1;k=(void*)((_tmp8D1=_cycalloc(sizeof(*_tmp8D1)),((_tmp8D1[0]=((_tmp8D2.tag=
2,((_tmp8D2.f1=0,((_tmp8D2.f2=_tmpB2,_tmp8D2)))))),_tmp8D1))));}goto _LL42;_LL47:;
_LL48: k=_tmpAF;goto _LL42;_LL42:;}{struct Cyc_Absyn_Tvar*_tmp8D3;return(_tmp8D3=
_cycalloc(sizeof(*_tmp8D3)),((_tmp8D3->name=t->name,((_tmp8D3->identity=- 1,((
_tmp8D3->kind=k,_tmp8D3)))))));};}static struct Cyc_Absyn_Tvar*Cyc_Parse_typ2tvar(
unsigned int loc,void*t){void*_tmpB8=t;struct Cyc_Absyn_Tvar*_tmpBA;_LL4A: {struct
Cyc_Absyn_VarType_struct*_tmpB9=(struct Cyc_Absyn_VarType_struct*)_tmpB8;if(
_tmpB9->tag != 2)goto _LL4C;else{_tmpBA=_tmpB9->f1;}}_LL4B: return _tmpBA;_LL4C:;
_LL4D: {const char*_tmp8D6;void*_tmp8D5;(_tmp8D5=0,((int(*)(unsigned int sg,struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Parse_abort)(loc,((_tmp8D6="expecting a list of type variables, not types",
_tag_dyneither(_tmp8D6,sizeof(char),46))),_tag_dyneither(_tmp8D5,sizeof(void*),0)));}
_LL49:;}static void*Cyc_Parse_tvar2typ(struct Cyc_Absyn_Tvar*pr){struct Cyc_Absyn_VarType_struct
_tmp8D9;struct Cyc_Absyn_VarType_struct*_tmp8D8;return(void*)((_tmp8D8=_cycalloc(
sizeof(*_tmp8D8)),((_tmp8D8[0]=((_tmp8D9.tag=2,((_tmp8D9.f1=pr,_tmp8D9)))),
_tmp8D8))));}static void Cyc_Parse_set_vartyp_kind(void*t,struct Cyc_Absyn_Kind*k,
int leq){void*_tmpBF=Cyc_Tcutil_compress(t);struct Cyc_Absyn_Tvar*_tmpC1;struct Cyc_Absyn_Tvar
_tmpC2;void*_tmpC3;void**_tmpC4;_LL4F: {struct Cyc_Absyn_VarType_struct*_tmpC0=(
struct Cyc_Absyn_VarType_struct*)_tmpBF;if(_tmpC0->tag != 2)goto _LL51;else{_tmpC1=
_tmpC0->f1;_tmpC2=*_tmpC1;_tmpC3=_tmpC2.kind;_tmpC4=(void**)&(*_tmpC0->f1).kind;}}
_LL50: {void*_tmpC5=Cyc_Absyn_compress_kb(*_tmpC4);_LL54: {struct Cyc_Absyn_Unknown_kb_struct*
_tmpC6=(struct Cyc_Absyn_Unknown_kb_struct*)_tmpC5;if(_tmpC6->tag != 1)goto _LL56;}
_LL55: if(!leq)*_tmpC4=Cyc_Tcutil_kind_to_bound(k);else{struct Cyc_Absyn_Less_kb_struct
_tmp8DC;struct Cyc_Absyn_Less_kb_struct*_tmp8DB;*_tmpC4=(void*)((_tmp8DB=
_cycalloc(sizeof(*_tmp8DB)),((_tmp8DB[0]=((_tmp8DC.tag=2,((_tmp8DC.f1=0,((
_tmp8DC.f2=k,_tmp8DC)))))),_tmp8DB))));}return;_LL56:;_LL57: return;_LL53:;}_LL51:;
_LL52: return;_LL4E:;}static struct Cyc_List_List*Cyc_Parse_oldstyle2newstyle(
struct _RegionHandle*yy,struct Cyc_List_List*tms,struct Cyc_List_List*tds,
unsigned int loc){if(tds == 0)return tms;if(tms == 0)return 0;{void*_tmpC9=(void*)tms->hd;
void*_tmpCB;_LL59: {struct Cyc_Absyn_Function_mod_struct*_tmpCA=(struct Cyc_Absyn_Function_mod_struct*)
_tmpC9;if(_tmpCA->tag != 3)goto _LL5B;else{_tmpCB=(void*)_tmpCA->f1;}}_LL5A: if(tms->tl
== 0  || Cyc_Parse_is_typeparam((void*)((struct Cyc_List_List*)_check_null(tms->tl))->hd)
 && ((struct Cyc_List_List*)_check_null(tms->tl))->tl == 0){void*_tmpCC=_tmpCB;
struct Cyc_List_List*_tmpCF;_LL5E: {struct Cyc_Absyn_WithTypes_struct*_tmpCD=(
struct Cyc_Absyn_WithTypes_struct*)_tmpCC;if(_tmpCD->tag != 1)goto _LL60;}_LL5F:{
const char*_tmp8DF;void*_tmp8DE;(_tmp8DE=0,Cyc_Tcutil_warn(loc,((_tmp8DF="function declaration with both new- and old-style parameter declarations; ignoring old-style",
_tag_dyneither(_tmp8DF,sizeof(char),93))),_tag_dyneither(_tmp8DE,sizeof(void*),0)));}
return tms;_LL60: {struct Cyc_Absyn_NoTypes_struct*_tmpCE=(struct Cyc_Absyn_NoTypes_struct*)
_tmpCC;if(_tmpCE->tag != 0)goto _LL5D;else{_tmpCF=_tmpCE->f1;}}_LL61: if(((int(*)(
struct Cyc_List_List*x))Cyc_List_length)(_tmpCF)!= ((int(*)(struct Cyc_List_List*x))
Cyc_List_length)(tds)){const char*_tmp8E2;void*_tmp8E1;(_tmp8E1=0,((int(*)(
unsigned int sg,struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Parse_abort)(
loc,((_tmp8E2="wrong number of parameter declarations in old-style function declaration",
_tag_dyneither(_tmp8E2,sizeof(char),73))),_tag_dyneither(_tmp8E1,sizeof(void*),0)));}{
struct Cyc_List_List*rev_new_params=0;for(0;_tmpCF != 0;_tmpCF=_tmpCF->tl){struct
Cyc_List_List*_tmpD4=tds;for(0;_tmpD4 != 0;_tmpD4=_tmpD4->tl){struct Cyc_Absyn_Decl*
_tmpD5=(struct Cyc_Absyn_Decl*)_tmpD4->hd;void*_tmpD6=_tmpD5->r;struct Cyc_Absyn_Vardecl*
_tmpD8;_LL63: {struct Cyc_Absyn_Var_d_struct*_tmpD7=(struct Cyc_Absyn_Var_d_struct*)
_tmpD6;if(_tmpD7->tag != 0)goto _LL65;else{_tmpD8=_tmpD7->f1;}}_LL64: if(Cyc_zstrptrcmp((*
_tmpD8->name).f2,(struct _dyneither_ptr*)_tmpCF->hd)!= 0)continue;if(_tmpD8->initializer
!= 0){const char*_tmp8E5;void*_tmp8E4;(_tmp8E4=0,((int(*)(unsigned int sg,struct
_dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Parse_abort)(_tmpD5->loc,((_tmp8E5="initializer found in parameter declaration",
_tag_dyneither(_tmp8E5,sizeof(char),43))),_tag_dyneither(_tmp8E4,sizeof(void*),0)));}
if(Cyc_Absyn_is_qvar_qualified(_tmpD8->name)){const char*_tmp8E8;void*_tmp8E7;(
_tmp8E7=0,((int(*)(unsigned int sg,struct _dyneither_ptr fmt,struct _dyneither_ptr ap))
Cyc_Parse_abort)(_tmpD5->loc,((_tmp8E8="namespaces forbidden in parameter declarations",
_tag_dyneither(_tmp8E8,sizeof(char),47))),_tag_dyneither(_tmp8E7,sizeof(void*),0)));}{
struct _tuple8*_tmp8EB;struct Cyc_List_List*_tmp8EA;rev_new_params=((_tmp8EA=
_cycalloc(sizeof(*_tmp8EA)),((_tmp8EA->hd=((_tmp8EB=_cycalloc(sizeof(*_tmp8EB)),((
_tmp8EB->f1=(struct _dyneither_ptr*)(*_tmpD8->name).f2,((_tmp8EB->f2=_tmpD8->tq,((
_tmp8EB->f3=_tmpD8->type,_tmp8EB)))))))),((_tmp8EA->tl=rev_new_params,_tmp8EA))))));}
goto L;_LL65:;_LL66: {const char*_tmp8EE;void*_tmp8ED;(_tmp8ED=0,((int(*)(
unsigned int sg,struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Parse_abort)(
_tmpD5->loc,((_tmp8EE="nonvariable declaration in parameter type",_tag_dyneither(
_tmp8EE,sizeof(char),42))),_tag_dyneither(_tmp8ED,sizeof(void*),0)));}_LL62:;}L:
if(_tmpD4 == 0){const char*_tmp8F2;void*_tmp8F1[1];struct Cyc_String_pa_struct
_tmp8F0;(_tmp8F0.tag=0,((_tmp8F0.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*((
struct _dyneither_ptr*)_tmpCF->hd)),((_tmp8F1[0]=& _tmp8F0,((int(*)(unsigned int sg,
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Parse_abort)(loc,((_tmp8F2="%s is not given a type",
_tag_dyneither(_tmp8F2,sizeof(char),23))),_tag_dyneither(_tmp8F1,sizeof(void*),1)))))));}}{
struct Cyc_Absyn_Function_mod_struct*_tmp901;struct Cyc_Absyn_WithTypes_struct*
_tmp900;struct Cyc_Absyn_WithTypes_struct _tmp8FF;struct Cyc_Absyn_Function_mod_struct
_tmp8FE;struct Cyc_List_List*_tmp8FD;return(_tmp8FD=_region_malloc(yy,sizeof(*
_tmp8FD)),((_tmp8FD->hd=(void*)((_tmp901=_region_malloc(yy,sizeof(*_tmp901)),((
_tmp901[0]=((_tmp8FE.tag=3,((_tmp8FE.f1=(void*)((void*)((_tmp900=_region_malloc(
yy,sizeof(*_tmp900)),((_tmp900[0]=((_tmp8FF.tag=1,((_tmp8FF.f1=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(rev_new_params),((_tmp8FF.f2=0,((
_tmp8FF.f3=0,((_tmp8FF.f4=(void*)0,((_tmp8FF.f5=0,_tmp8FF)))))))))))),_tmp900))))),
_tmp8FE)))),_tmp901)))),((_tmp8FD->tl=0,_tmp8FD)))));};};_LL5D:;}goto _LL5C;_LL5B:;
_LL5C: {struct Cyc_List_List*_tmp902;return(_tmp902=_region_malloc(yy,sizeof(*
_tmp902)),((_tmp902->hd=(void*)tms->hd,((_tmp902->tl=Cyc_Parse_oldstyle2newstyle(
yy,tms->tl,tds,loc),_tmp902)))));}_LL58:;};}static struct Cyc_Absyn_Fndecl*Cyc_Parse_make_function(
struct _RegionHandle*yy,struct Cyc_Parse_Declaration_spec*dso,struct Cyc_Parse_Declarator
d,struct Cyc_List_List*tds,struct Cyc_Absyn_Stmt*body,unsigned int loc){if(tds != 0){
struct Cyc_Parse_Declarator _tmp903;d=((_tmp903.id=d.id,((_tmp903.tms=Cyc_Parse_oldstyle2newstyle(
yy,d.tms,tds,loc),_tmp903))));}{enum Cyc_Absyn_Scope sc=Cyc_Absyn_Public;struct
Cyc_Parse_Type_specifier tss=Cyc_Parse_empty_spec(loc);struct Cyc_Absyn_Tqual tq=
Cyc_Absyn_empty_tqual(0);int is_inline=0;struct Cyc_List_List*atts=0;if(dso != 0){
tss=dso->type_specs;tq=dso->tq;is_inline=dso->is_inline;atts=dso->attributes;if(
dso->sc != 0)switch(*((enum Cyc_Parse_Storage_class*)_check_null(dso->sc))){case
Cyc_Parse_Extern_sc: _LL67: sc=Cyc_Absyn_Extern;break;case Cyc_Parse_Static_sc:
_LL68: sc=Cyc_Absyn_Static;break;default: _LL69:{const char*_tmp904;Cyc_Parse_err(((
_tmp904="bad storage class on function",_tag_dyneither(_tmp904,sizeof(char),30))),
loc);}break;}}{void*_tmpEC=Cyc_Parse_collapse_type_specifiers(tss,loc);struct Cyc_Absyn_Tqual
_tmpEE;void*_tmpEF;struct Cyc_List_List*_tmpF0;struct Cyc_List_List*_tmpF1;struct
_tuple14 _tmpED=Cyc_Parse_apply_tms(tq,_tmpEC,atts,d.tms);_tmpEE=_tmpED.f1;_tmpEF=
_tmpED.f2;_tmpF0=_tmpED.f3;_tmpF1=_tmpED.f4;if(_tmpF0 != 0){const char*_tmp907;
void*_tmp906;(_tmp906=0,Cyc_Tcutil_warn(loc,((_tmp907="bad type params, ignoring",
_tag_dyneither(_tmp907,sizeof(char),26))),_tag_dyneither(_tmp906,sizeof(void*),0)));}{
void*_tmpF4=_tmpEF;struct Cyc_Absyn_FnInfo _tmpF6;struct Cyc_List_List*_tmpF7;void*
_tmpF8;struct Cyc_Absyn_Tqual _tmpF9;void*_tmpFA;struct Cyc_List_List*_tmpFB;int
_tmpFC;struct Cyc_Absyn_VarargInfo*_tmpFD;struct Cyc_List_List*_tmpFE;struct Cyc_List_List*
_tmpFF;_LL6C: {struct Cyc_Absyn_FnType_struct*_tmpF5=(struct Cyc_Absyn_FnType_struct*)
_tmpF4;if(_tmpF5->tag != 9)goto _LL6E;else{_tmpF6=_tmpF5->f1;_tmpF7=_tmpF6.tvars;
_tmpF8=_tmpF6.effect;_tmpF9=_tmpF6.ret_tqual;_tmpFA=_tmpF6.ret_typ;_tmpFB=_tmpF6.args;
_tmpFC=_tmpF6.c_varargs;_tmpFD=_tmpF6.cyc_varargs;_tmpFE=_tmpF6.rgn_po;_tmpFF=
_tmpF6.attributes;}}_LL6D: {struct Cyc_List_List*_tmp100=0;{struct Cyc_List_List*
_tmp101=_tmpFB;for(0;_tmp101 != 0;_tmp101=_tmp101->tl){struct _tuple8 _tmp103;
struct _dyneither_ptr*_tmp104;struct Cyc_Absyn_Tqual _tmp105;void*_tmp106;struct
_tuple8*_tmp102=(struct _tuple8*)_tmp101->hd;_tmp103=*_tmp102;_tmp104=_tmp103.f1;
_tmp105=_tmp103.f2;_tmp106=_tmp103.f3;if(_tmp104 == 0){{const char*_tmp908;Cyc_Parse_err(((
_tmp908="missing argument variable in function prototype",_tag_dyneither(_tmp908,
sizeof(char),48))),loc);}{struct _tuple8*_tmp90B;struct Cyc_List_List*_tmp90A;
_tmp100=((_tmp90A=_cycalloc(sizeof(*_tmp90A)),((_tmp90A->hd=((_tmp90B=_cycalloc(
sizeof(*_tmp90B)),((_tmp90B->f1=_init_dyneither_ptr(_cycalloc(sizeof(struct
_dyneither_ptr)),"?",sizeof(char),2),((_tmp90B->f2=_tmp105,((_tmp90B->f3=_tmp106,
_tmp90B)))))))),((_tmp90A->tl=_tmp100,_tmp90A))))));};}else{struct _tuple8*
_tmp90E;struct Cyc_List_List*_tmp90D;_tmp100=((_tmp90D=_cycalloc(sizeof(*_tmp90D)),((
_tmp90D->hd=((_tmp90E=_cycalloc(sizeof(*_tmp90E)),((_tmp90E->f1=(struct
_dyneither_ptr*)_tmp104,((_tmp90E->f2=_tmp105,((_tmp90E->f3=_tmp106,_tmp90E)))))))),((
_tmp90D->tl=_tmp100,_tmp90D))))));}}}{struct Cyc_Absyn_Fndecl*_tmp90F;return(
_tmp90F=_cycalloc(sizeof(*_tmp90F)),((_tmp90F->sc=sc,((_tmp90F->name=d.id,((
_tmp90F->tvs=_tmpF7,((_tmp90F->is_inline=is_inline,((_tmp90F->effect=_tmpF8,((
_tmp90F->ret_tqual=_tmpF9,((_tmp90F->ret_type=_tmpFA,((_tmp90F->args=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(_tmp100),((_tmp90F->c_varargs=_tmpFC,((
_tmp90F->cyc_varargs=_tmpFD,((_tmp90F->rgn_po=_tmpFE,((_tmp90F->body=body,((
_tmp90F->cached_typ=0,((_tmp90F->param_vardecls=0,((_tmp90F->fn_vardecl=0,((
_tmp90F->attributes=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*
y))Cyc_List_append)(_tmpFF,_tmpF1),_tmp90F)))))))))))))))))))))))))))))))));};}
_LL6E:;_LL6F: {const char*_tmp912;void*_tmp911;(_tmp911=0,((int(*)(unsigned int sg,
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Parse_abort)(loc,((_tmp912="declarator is not a function prototype",
_tag_dyneither(_tmp912,sizeof(char),39))),_tag_dyneither(_tmp911,sizeof(void*),0)));}
_LL6B:;};};};}static char _tmp110[52]="at most one type may appear within a type specifier";
static struct _dyneither_ptr Cyc_Parse_msg1={_tmp110,_tmp110,_tmp110 + 52};static
char _tmp111[63]="const or volatile may appear only once within a type specifier";
static struct _dyneither_ptr Cyc_Parse_msg2={_tmp111,_tmp111,_tmp111 + 63};static
char _tmp112[50]="type specifier includes more than one declaration";static struct
_dyneither_ptr Cyc_Parse_msg3={_tmp112,_tmp112,_tmp112 + 50};static char _tmp113[60]="sign specifier may appear only once within a type specifier";
static struct _dyneither_ptr Cyc_Parse_msg4={_tmp113,_tmp113,_tmp113 + 60};static
struct Cyc_Parse_Type_specifier Cyc_Parse_combine_specifiers(unsigned int loc,
struct Cyc_Parse_Type_specifier s1,struct Cyc_Parse_Type_specifier s2){if(s1.Signed_spec
 && s2.Signed_spec){void*_tmp913;(_tmp913=0,Cyc_Tcutil_warn(loc,Cyc_Parse_msg4,
_tag_dyneither(_tmp913,sizeof(void*),0)));}s1.Signed_spec |=s2.Signed_spec;if(s1.Unsigned_spec
 && s2.Unsigned_spec){void*_tmp914;(_tmp914=0,Cyc_Tcutil_warn(loc,Cyc_Parse_msg4,
_tag_dyneither(_tmp914,sizeof(void*),0)));}s1.Unsigned_spec |=s2.Unsigned_spec;
if(s1.Short_spec  && s2.Short_spec){void*_tmp915;(_tmp915=0,Cyc_Tcutil_warn(loc,
Cyc_Parse_msg4,_tag_dyneither(_tmp915,sizeof(void*),0)));}s1.Short_spec |=s2.Short_spec;
if((s1.Long_Long_spec  && s2.Long_Long_spec  || s1.Long_Long_spec  && s2.Long_spec)
 || s2.Long_Long_spec  && s1.Long_spec){void*_tmp916;(_tmp916=0,Cyc_Tcutil_warn(
loc,Cyc_Parse_msg4,_tag_dyneither(_tmp916,sizeof(void*),0)));}s1.Long_Long_spec=(
s1.Long_Long_spec  || s2.Long_Long_spec) || s1.Long_spec  && s2.Long_spec;s1.Long_spec=
!s1.Long_Long_spec  && (s1.Long_spec  || s2.Long_spec);if(s1.Valid_type_spec  && s2.Valid_type_spec)
Cyc_Parse_err(Cyc_Parse_msg1,loc);else{if(s2.Valid_type_spec){s1.Type_spec=s2.Type_spec;
s1.Valid_type_spec=1;}}return s1;}static void*Cyc_Parse_collapse_type_specifiers(
struct Cyc_Parse_Type_specifier ts,unsigned int loc){int seen_type=ts.Valid_type_spec;
int seen_sign=ts.Signed_spec  || ts.Unsigned_spec;int seen_size=(ts.Short_spec  || 
ts.Long_spec) || ts.Long_Long_spec;void*t=seen_type?ts.Type_spec:(void*)& Cyc_Absyn_VoidType_val;
enum Cyc_Absyn_Size_of sz=Cyc_Absyn_Int_sz;enum Cyc_Absyn_Sign sgn=Cyc_Absyn_Signed;
if(ts.Signed_spec  && ts.Unsigned_spec)Cyc_Parse_err(Cyc_Parse_msg4,loc);if(ts.Unsigned_spec)
sgn=Cyc_Absyn_Unsigned;if(ts.Short_spec  && (ts.Long_spec  || ts.Long_Long_spec)
 || ts.Long_spec  && ts.Long_Long_spec)Cyc_Parse_err(Cyc_Parse_msg4,loc);if(ts.Short_spec)
sz=Cyc_Absyn_Short_sz;if(ts.Long_spec)sz=Cyc_Absyn_Long_sz;if(ts.Long_Long_spec)
sz=Cyc_Absyn_LongLong_sz;if(!seen_type){if(!seen_sign  && !seen_size){const char*
_tmp919;void*_tmp918;(_tmp918=0,Cyc_Tcutil_warn(loc,((_tmp919="missing type within specifier",
_tag_dyneither(_tmp919,sizeof(char),30))),_tag_dyneither(_tmp918,sizeof(void*),0)));}
t=Cyc_Absyn_int_typ(sgn,sz);}else{if(seen_sign){void*_tmp11A=t;enum Cyc_Absyn_Sign
_tmp11C;enum Cyc_Absyn_Size_of _tmp11D;_LL71: {struct Cyc_Absyn_IntType_struct*
_tmp11B=(struct Cyc_Absyn_IntType_struct*)_tmp11A;if(_tmp11B->tag != 6)goto _LL73;
else{_tmp11C=_tmp11B->f1;_tmp11D=_tmp11B->f2;}}_LL72: if(_tmp11C != sgn)t=Cyc_Absyn_int_typ(
sgn,_tmp11D);goto _LL70;_LL73:;_LL74:{const char*_tmp91A;Cyc_Parse_err(((_tmp91A="sign specification on non-integral type",
_tag_dyneither(_tmp91A,sizeof(char),40))),loc);}goto _LL70;_LL70:;}if(seen_size){
void*_tmp11F=t;enum Cyc_Absyn_Sign _tmp121;enum Cyc_Absyn_Size_of _tmp122;_LL76: {
struct Cyc_Absyn_IntType_struct*_tmp120=(struct Cyc_Absyn_IntType_struct*)_tmp11F;
if(_tmp120->tag != 6)goto _LL78;else{_tmp121=_tmp120->f1;_tmp122=_tmp120->f2;}}
_LL77: if(_tmp122 != sz)t=Cyc_Absyn_int_typ(_tmp121,sz);goto _LL75;_LL78: {struct
Cyc_Absyn_FloatType_struct*_tmp123=(struct Cyc_Absyn_FloatType_struct*)_tmp11F;
if(_tmp123->tag != 7)goto _LL7A;}_LL79: t=Cyc_Absyn_float_typ(2);goto _LL75;_LL7A:;
_LL7B:{const char*_tmp91B;Cyc_Parse_err(((_tmp91B="size qualifier on non-integral type",
_tag_dyneither(_tmp91B,sizeof(char),36))),loc);}goto _LL75;_LL75:;}}return t;}
static struct Cyc_List_List*Cyc_Parse_apply_tmss(struct _RegionHandle*r,struct Cyc_Absyn_Tqual
tq,void*t,struct _tuple11*ds,struct Cyc_List_List*shared_atts){if(ds == 0)return 0;{
struct Cyc_Parse_Declarator d=ds->hd;struct _tuple0*_tmp125=d.id;struct Cyc_Absyn_Tqual
_tmp127;void*_tmp128;struct Cyc_List_List*_tmp129;struct Cyc_List_List*_tmp12A;
struct _tuple14 _tmp126=Cyc_Parse_apply_tms(tq,t,shared_atts,d.tms);_tmp127=
_tmp126.f1;_tmp128=_tmp126.f2;_tmp129=_tmp126.f3;_tmp12A=_tmp126.f4;if(ds->tl == 
0){struct _tuple16*_tmp91E;struct Cyc_List_List*_tmp91D;return(_tmp91D=
_region_malloc(r,sizeof(*_tmp91D)),((_tmp91D->hd=((_tmp91E=_region_malloc(r,
sizeof(*_tmp91E)),((_tmp91E->f1=_tmp125,((_tmp91E->f2=_tmp127,((_tmp91E->f3=
_tmp128,((_tmp91E->f4=_tmp129,((_tmp91E->f5=_tmp12A,_tmp91E)))))))))))),((
_tmp91D->tl=0,_tmp91D)))));}else{struct _tuple16*_tmp921;struct Cyc_List_List*
_tmp920;return(_tmp920=_region_malloc(r,sizeof(*_tmp920)),((_tmp920->hd=((
_tmp921=_region_malloc(r,sizeof(*_tmp921)),((_tmp921->f1=_tmp125,((_tmp921->f2=
_tmp127,((_tmp921->f3=_tmp128,((_tmp921->f4=_tmp129,((_tmp921->f5=_tmp12A,
_tmp921)))))))))))),((_tmp920->tl=Cyc_Parse_apply_tmss(r,tq,Cyc_Tcutil_copy_type(
t),ds->tl,shared_atts),_tmp920)))));}};}static struct _tuple14 Cyc_Parse_apply_tms(
struct Cyc_Absyn_Tqual tq,void*t,struct Cyc_List_List*atts,struct Cyc_List_List*tms){
if(tms == 0){struct _tuple14 _tmp922;return(_tmp922.f1=tq,((_tmp922.f2=t,((_tmp922.f3=
0,((_tmp922.f4=atts,_tmp922)))))));}{void*_tmp130=(void*)tms->hd;union Cyc_Absyn_Constraint*
_tmp132;unsigned int _tmp133;struct Cyc_Absyn_Exp*_tmp135;union Cyc_Absyn_Constraint*
_tmp136;unsigned int _tmp137;void*_tmp139;struct Cyc_List_List*_tmp13B;
unsigned int _tmp13C;struct Cyc_Absyn_PtrAtts _tmp13E;struct Cyc_Absyn_Tqual _tmp13F;
unsigned int _tmp141;struct Cyc_List_List*_tmp142;_LL7D: {struct Cyc_Absyn_Carray_mod_struct*
_tmp131=(struct Cyc_Absyn_Carray_mod_struct*)_tmp130;if(_tmp131->tag != 0)goto
_LL7F;else{_tmp132=_tmp131->f1;_tmp133=_tmp131->f2;}}_LL7E: return Cyc_Parse_apply_tms(
Cyc_Absyn_empty_tqual(0),Cyc_Absyn_array_typ(t,tq,0,_tmp132,_tmp133),atts,tms->tl);
_LL7F: {struct Cyc_Absyn_ConstArray_mod_struct*_tmp134=(struct Cyc_Absyn_ConstArray_mod_struct*)
_tmp130;if(_tmp134->tag != 1)goto _LL81;else{_tmp135=_tmp134->f1;_tmp136=_tmp134->f2;
_tmp137=_tmp134->f3;}}_LL80: return Cyc_Parse_apply_tms(Cyc_Absyn_empty_tqual(0),
Cyc_Absyn_array_typ(t,tq,(struct Cyc_Absyn_Exp*)_tmp135,_tmp136,_tmp137),atts,tms->tl);
_LL81: {struct Cyc_Absyn_Function_mod_struct*_tmp138=(struct Cyc_Absyn_Function_mod_struct*)
_tmp130;if(_tmp138->tag != 3)goto _LL83;else{_tmp139=(void*)_tmp138->f1;}}_LL82: {
void*_tmp143=_tmp139;struct Cyc_List_List*_tmp145;int _tmp146;struct Cyc_Absyn_VarargInfo*
_tmp147;void*_tmp148;struct Cyc_List_List*_tmp149;unsigned int _tmp14B;_LL8A: {
struct Cyc_Absyn_WithTypes_struct*_tmp144=(struct Cyc_Absyn_WithTypes_struct*)
_tmp143;if(_tmp144->tag != 1)goto _LL8C;else{_tmp145=_tmp144->f1;_tmp146=_tmp144->f2;
_tmp147=_tmp144->f3;_tmp148=(void*)_tmp144->f4;_tmp149=_tmp144->f5;}}_LL8B: {
struct Cyc_List_List*typvars=0;struct Cyc_List_List*fn_atts=0;struct Cyc_List_List*
new_atts=0;{struct Cyc_List_List*as=atts;for(0;as != 0;as=as->tl){if(Cyc_Absyn_fntype_att((
void*)as->hd)){struct Cyc_List_List*_tmp923;fn_atts=((_tmp923=_cycalloc(sizeof(*
_tmp923)),((_tmp923->hd=(void*)as->hd,((_tmp923->tl=fn_atts,_tmp923))))));}else{
struct Cyc_List_List*_tmp924;new_atts=((_tmp924=_cycalloc(sizeof(*_tmp924)),((
_tmp924->hd=(void*)as->hd,((_tmp924->tl=new_atts,_tmp924))))));}}}if(tms->tl != 0){
void*_tmp14E=(void*)((struct Cyc_List_List*)_check_null(tms->tl))->hd;struct Cyc_List_List*
_tmp150;_LL8F: {struct Cyc_Absyn_TypeParams_mod_struct*_tmp14F=(struct Cyc_Absyn_TypeParams_mod_struct*)
_tmp14E;if(_tmp14F->tag != 4)goto _LL91;else{_tmp150=_tmp14F->f1;}}_LL90: typvars=
_tmp150;tms=tms->tl;goto _LL8E;_LL91:;_LL92: goto _LL8E;_LL8E:;}if(((((!_tmp146  && 
_tmp147 == 0) && _tmp145 != 0) && _tmp145->tl == 0) && (*((struct _tuple8*)_tmp145->hd)).f1
== 0) && (*((struct _tuple8*)_tmp145->hd)).f3 == (void*)& Cyc_Absyn_VoidType_val)
_tmp145=0;{struct Cyc_List_List*_tmp151=Cyc_Parse_get_arg_tags(_tmp145);if(
_tmp151 != 0)t=Cyc_Parse_substitute_tags(_tmp151,t);t=Cyc_Parse_array2ptr(t,0);{
struct Cyc_List_List*_tmp152=_tmp145;for(0;_tmp152 != 0;_tmp152=_tmp152->tl){
struct _tuple8 _tmp154;struct _dyneither_ptr*_tmp155;struct Cyc_Absyn_Tqual _tmp156;
void*_tmp157;void**_tmp158;struct _tuple8*_tmp153=(struct _tuple8*)_tmp152->hd;
_tmp154=*_tmp153;_tmp155=_tmp154.f1;_tmp156=_tmp154.f2;_tmp157=_tmp154.f3;
_tmp158=(void**)&(*_tmp153).f3;if(_tmp151 != 0)*_tmp158=Cyc_Parse_substitute_tags(
_tmp151,*_tmp158);*_tmp158=Cyc_Parse_array2ptr(*_tmp158,1);}}return Cyc_Parse_apply_tms(
Cyc_Absyn_empty_tqual(tq.loc),Cyc_Absyn_function_typ(typvars,_tmp148,tq,t,
_tmp145,_tmp146,_tmp147,_tmp149,fn_atts),new_atts,((struct Cyc_List_List*)
_check_null(tms))->tl);};}_LL8C: {struct Cyc_Absyn_NoTypes_struct*_tmp14A=(struct
Cyc_Absyn_NoTypes_struct*)_tmp143;if(_tmp14A->tag != 0)goto _LL89;else{_tmp14B=
_tmp14A->f2;}}_LL8D: {const char*_tmp927;void*_tmp926;(_tmp926=0,((int(*)(
unsigned int sg,struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Parse_abort)(
_tmp14B,((_tmp927="function declaration without parameter types",_tag_dyneither(
_tmp927,sizeof(char),45))),_tag_dyneither(_tmp926,sizeof(void*),0)));}_LL89:;}
_LL83: {struct Cyc_Absyn_TypeParams_mod_struct*_tmp13A=(struct Cyc_Absyn_TypeParams_mod_struct*)
_tmp130;if(_tmp13A->tag != 4)goto _LL85;else{_tmp13B=_tmp13A->f1;_tmp13C=_tmp13A->f2;}}
_LL84: if(tms->tl == 0){struct _tuple14 _tmp928;return(_tmp928.f1=tq,((_tmp928.f2=t,((
_tmp928.f3=_tmp13B,((_tmp928.f4=atts,_tmp928)))))));}{const char*_tmp92B;void*
_tmp92A;(_tmp92A=0,((int(*)(unsigned int sg,struct _dyneither_ptr fmt,struct
_dyneither_ptr ap))Cyc_Parse_abort)(_tmp13C,((_tmp92B="type parameters must appear before function arguments in declarator",
_tag_dyneither(_tmp92B,sizeof(char),68))),_tag_dyneither(_tmp92A,sizeof(void*),0)));};
_LL85: {struct Cyc_Absyn_Pointer_mod_struct*_tmp13D=(struct Cyc_Absyn_Pointer_mod_struct*)
_tmp130;if(_tmp13D->tag != 2)goto _LL87;else{_tmp13E=_tmp13D->f1;_tmp13F=_tmp13D->f2;}}
_LL86: {struct Cyc_Absyn_PointerType_struct _tmp931;struct Cyc_Absyn_PtrInfo _tmp930;
struct Cyc_Absyn_PointerType_struct*_tmp92F;return Cyc_Parse_apply_tms(_tmp13F,(
void*)((_tmp92F=_cycalloc(sizeof(*_tmp92F)),((_tmp92F[0]=((_tmp931.tag=5,((
_tmp931.f1=((_tmp930.elt_typ=t,((_tmp930.elt_tq=tq,((_tmp930.ptr_atts=_tmp13E,
_tmp930)))))),_tmp931)))),_tmp92F)))),atts,tms->tl);}_LL87: {struct Cyc_Absyn_Attributes_mod_struct*
_tmp140=(struct Cyc_Absyn_Attributes_mod_struct*)_tmp130;if(_tmp140->tag != 5)goto
_LL7C;else{_tmp141=_tmp140->f1;_tmp142=_tmp140->f2;}}_LL88: return Cyc_Parse_apply_tms(
tq,t,((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(
atts,_tmp142),tms->tl);_LL7C:;};}void*Cyc_Parse_speclist2typ(struct Cyc_Parse_Type_specifier
tss,unsigned int loc){return Cyc_Parse_collapse_type_specifiers(tss,loc);}static
struct Cyc_Absyn_Decl*Cyc_Parse_v_typ_to_typedef(unsigned int loc,struct _tuple16*t){
struct _tuple16 _tmp162;struct _tuple0*_tmp163;struct Cyc_Absyn_Tqual _tmp164;void*
_tmp165;struct Cyc_List_List*_tmp166;struct Cyc_List_List*_tmp167;struct _tuple16*
_tmp161=t;_tmp162=*_tmp161;_tmp163=_tmp162.f1;_tmp164=_tmp162.f2;_tmp165=_tmp162.f3;
_tmp166=_tmp162.f4;_tmp167=_tmp162.f5;Cyc_Lex_register_typedef(_tmp163);{struct
Cyc_Core_Opt*kind;void*type;{void*_tmp168=_tmp165;struct Cyc_Core_Opt*_tmp16A;
_LL94: {struct Cyc_Absyn_Evar_struct*_tmp169=(struct Cyc_Absyn_Evar_struct*)
_tmp168;if(_tmp169->tag != 1)goto _LL96;else{_tmp16A=_tmp169->f1;}}_LL95: type=0;
if(_tmp16A == 0)kind=(struct Cyc_Core_Opt*)& Cyc_Tcutil_bko;else{kind=_tmp16A;}goto
_LL93;_LL96:;_LL97: kind=0;type=(void*)_tmp165;goto _LL93;_LL93:;}{struct Cyc_Absyn_Typedef_d_struct
_tmp937;struct Cyc_Absyn_Typedefdecl*_tmp936;struct Cyc_Absyn_Typedef_d_struct*
_tmp935;return Cyc_Absyn_new_decl((void*)((_tmp935=_cycalloc(sizeof(*_tmp935)),((
_tmp935[0]=((_tmp937.tag=9,((_tmp937.f1=((_tmp936=_cycalloc(sizeof(*_tmp936)),((
_tmp936->name=_tmp163,((_tmp936->tvs=_tmp166,((_tmp936->kind=kind,((_tmp936->defn=
type,((_tmp936->atts=_tmp167,((_tmp936->tq=_tmp164,_tmp936)))))))))))))),_tmp937)))),
_tmp935)))),loc);};};}static struct Cyc_Absyn_Stmt*Cyc_Parse_flatten_decl(struct
Cyc_Absyn_Decl*d,struct Cyc_Absyn_Stmt*s){struct Cyc_Absyn_Decl_s_struct _tmp93A;
struct Cyc_Absyn_Decl_s_struct*_tmp939;return Cyc_Absyn_new_stmt((void*)((_tmp939=
_cycalloc(sizeof(*_tmp939)),((_tmp939[0]=((_tmp93A.tag=12,((_tmp93A.f1=d,((
_tmp93A.f2=s,_tmp93A)))))),_tmp939)))),d->loc);}static struct Cyc_Absyn_Stmt*Cyc_Parse_flatten_declarations(
struct Cyc_List_List*ds,struct Cyc_Absyn_Stmt*s){return((struct Cyc_Absyn_Stmt*(*)(
struct Cyc_Absyn_Stmt*(*f)(struct Cyc_Absyn_Decl*,struct Cyc_Absyn_Stmt*),struct Cyc_List_List*
x,struct Cyc_Absyn_Stmt*accum))Cyc_List_fold_right)(Cyc_Parse_flatten_decl,ds,s);}
static struct Cyc_List_List*Cyc_Parse_make_declarations(struct Cyc_Parse_Declaration_spec
ds,struct _tuple13*ids,unsigned int tqual_loc,unsigned int loc){struct _RegionHandle
_tmp170=_new_region("mkrgn");struct _RegionHandle*mkrgn=& _tmp170;_push_region(
mkrgn);{struct Cyc_Absyn_Tqual _tmp172;struct Cyc_Parse_Type_specifier _tmp173;
struct Cyc_List_List*_tmp174;struct Cyc_Parse_Declaration_spec _tmp171=ds;_tmp172=
_tmp171.tq;_tmp173=_tmp171.type_specs;_tmp174=_tmp171.attributes;if(_tmp172.loc
== 0)_tmp172.loc=tqual_loc;if(ds.is_inline){const char*_tmp93B;Cyc_Parse_err(((
_tmp93B="inline is allowed only on function definitions",_tag_dyneither(_tmp93B,
sizeof(char),47))),loc);}{enum Cyc_Absyn_Scope s=Cyc_Absyn_Public;int istypedef=0;
if(ds.sc != 0)switch(*ds.sc){case Cyc_Parse_Typedef_sc: _LL98: istypedef=1;break;
case Cyc_Parse_Extern_sc: _LL99: s=Cyc_Absyn_Extern;break;case Cyc_Parse_ExternC_sc:
_LL9A: s=Cyc_Absyn_ExternC;break;case Cyc_Parse_Static_sc: _LL9B: s=Cyc_Absyn_Static;
break;case Cyc_Parse_Auto_sc: _LL9C: s=Cyc_Absyn_Public;break;case Cyc_Parse_Register_sc:
_LL9D: if(Cyc_Parse_no_register)s=Cyc_Absyn_Public;else{s=Cyc_Absyn_Register;}
break;case Cyc_Parse_Abstract_sc: _LL9E: s=Cyc_Absyn_Abstract;break;}{struct
_tuple11*declarators=0;struct Cyc_List_List*exprs=0;Cyc_Parse_decl_split(mkrgn,
ids,& declarators,& exprs);{int exps_empty=1;{struct Cyc_List_List*es=exprs;for(0;es
!= 0;es=es->tl){if((struct Cyc_Absyn_Exp*)es->hd != 0){exps_empty=0;break;}}}{void*
_tmp176=Cyc_Parse_collapse_type_specifiers(_tmp173,loc);if(declarators == 0){void*
_tmp177=_tmp176;struct Cyc_Absyn_TypeDecl*_tmp179;struct Cyc_Absyn_TypeDecl _tmp17A;
void*_tmp17B;struct Cyc_Absyn_Aggrdecl*_tmp17D;struct Cyc_Absyn_TypeDecl*_tmp17F;
struct Cyc_Absyn_TypeDecl _tmp180;void*_tmp181;struct Cyc_Absyn_Enumdecl*_tmp183;
struct Cyc_Absyn_TypeDecl*_tmp185;struct Cyc_Absyn_TypeDecl _tmp186;void*_tmp187;
struct Cyc_Absyn_Datatypedecl*_tmp189;struct Cyc_Absyn_AggrInfo _tmp18B;union Cyc_Absyn_AggrInfoU
_tmp18C;struct _tuple2 _tmp18D;enum Cyc_Absyn_AggrKind _tmp18E;struct _tuple0*
_tmp18F;struct Cyc_List_List*_tmp190;struct Cyc_Absyn_DatatypeInfo _tmp192;union Cyc_Absyn_DatatypeInfoU
_tmp193;struct Cyc_Absyn_Datatypedecl**_tmp194;struct Cyc_Absyn_DatatypeInfo
_tmp196;union Cyc_Absyn_DatatypeInfoU _tmp197;struct Cyc_Absyn_UnknownDatatypeInfo
_tmp198;struct _tuple0*_tmp199;int _tmp19A;struct Cyc_List_List*_tmp19B;struct
_tuple0*_tmp19D;struct Cyc_List_List*_tmp19F;_LLA1: {struct Cyc_Absyn_TypeDeclType_struct*
_tmp178=(struct Cyc_Absyn_TypeDeclType_struct*)_tmp177;if(_tmp178->tag != 26)goto
_LLA3;else{_tmp179=_tmp178->f1;_tmp17A=*_tmp179;_tmp17B=_tmp17A.r;{struct Cyc_Absyn_Aggr_td_struct*
_tmp17C=(struct Cyc_Absyn_Aggr_td_struct*)_tmp17B;if(_tmp17C->tag != 0)goto _LLA3;
else{_tmp17D=_tmp17C->f1;}};}}_LLA2: _tmp17D->attributes=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp17D->attributes,
_tmp174);{struct Cyc_Absyn_Aggr_d_struct*_tmp941;struct Cyc_Absyn_Aggr_d_struct
_tmp940;struct Cyc_List_List*_tmp93F;struct Cyc_List_List*_tmp1A3=(_tmp93F=
_cycalloc(sizeof(*_tmp93F)),((_tmp93F->hd=Cyc_Absyn_new_decl((void*)((_tmp941=
_cycalloc(sizeof(*_tmp941)),((_tmp941[0]=((_tmp940.tag=6,((_tmp940.f1=_tmp17D,
_tmp940)))),_tmp941)))),loc),((_tmp93F->tl=0,_tmp93F)))));_npop_handler(0);
return _tmp1A3;};_LLA3: {struct Cyc_Absyn_TypeDeclType_struct*_tmp17E=(struct Cyc_Absyn_TypeDeclType_struct*)
_tmp177;if(_tmp17E->tag != 26)goto _LLA5;else{_tmp17F=_tmp17E->f1;_tmp180=*_tmp17F;
_tmp181=_tmp180.r;{struct Cyc_Absyn_Enum_td_struct*_tmp182=(struct Cyc_Absyn_Enum_td_struct*)
_tmp181;if(_tmp182->tag != 1)goto _LLA5;else{_tmp183=_tmp182->f1;}};}}_LLA4: if(
_tmp174 != 0){const char*_tmp942;Cyc_Parse_err(((_tmp942="attributes on enum not supported",
_tag_dyneither(_tmp942,sizeof(char),33))),loc);}{struct Cyc_Absyn_Enum_d_struct*
_tmp948;struct Cyc_Absyn_Enum_d_struct _tmp947;struct Cyc_List_List*_tmp946;struct
Cyc_List_List*_tmp1A8=(_tmp946=_cycalloc(sizeof(*_tmp946)),((_tmp946->hd=Cyc_Absyn_new_decl((
void*)((_tmp948=_cycalloc(sizeof(*_tmp948)),((_tmp948[0]=((_tmp947.tag=8,((
_tmp947.f1=_tmp183,_tmp947)))),_tmp948)))),loc),((_tmp946->tl=0,_tmp946)))));
_npop_handler(0);return _tmp1A8;};_LLA5: {struct Cyc_Absyn_TypeDeclType_struct*
_tmp184=(struct Cyc_Absyn_TypeDeclType_struct*)_tmp177;if(_tmp184->tag != 26)goto
_LLA7;else{_tmp185=_tmp184->f1;_tmp186=*_tmp185;_tmp187=_tmp186.r;{struct Cyc_Absyn_Datatype_td_struct*
_tmp188=(struct Cyc_Absyn_Datatype_td_struct*)_tmp187;if(_tmp188->tag != 2)goto
_LLA7;else{_tmp189=_tmp188->f1;}};}}_LLA6: if(_tmp174 != 0){const char*_tmp949;Cyc_Parse_err(((
_tmp949="attributes on datatypes not supported",_tag_dyneither(_tmp949,sizeof(
char),38))),loc);}{struct Cyc_Absyn_Datatype_d_struct*_tmp94F;struct Cyc_Absyn_Datatype_d_struct
_tmp94E;struct Cyc_List_List*_tmp94D;struct Cyc_List_List*_tmp1AD=(_tmp94D=
_cycalloc(sizeof(*_tmp94D)),((_tmp94D->hd=Cyc_Absyn_new_decl((void*)((_tmp94F=
_cycalloc(sizeof(*_tmp94F)),((_tmp94F[0]=((_tmp94E.tag=7,((_tmp94E.f1=_tmp189,
_tmp94E)))),_tmp94F)))),loc),((_tmp94D->tl=0,_tmp94D)))));_npop_handler(0);
return _tmp1AD;};_LLA7: {struct Cyc_Absyn_AggrType_struct*_tmp18A=(struct Cyc_Absyn_AggrType_struct*)
_tmp177;if(_tmp18A->tag != 11)goto _LLA9;else{_tmp18B=_tmp18A->f1;_tmp18C=_tmp18B.aggr_info;
if((_tmp18C.UnknownAggr).tag != 1)goto _LLA9;_tmp18D=(struct _tuple2)(_tmp18C.UnknownAggr).val;
_tmp18E=_tmp18D.f1;_tmp18F=_tmp18D.f2;_tmp190=_tmp18B.targs;}}_LLA8: {struct Cyc_List_List*
_tmp1AE=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*f)(unsigned int,void*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,loc,
_tmp190);struct Cyc_Absyn_Aggrdecl*_tmp950;struct Cyc_Absyn_Aggrdecl*_tmp1AF=(
_tmp950=_cycalloc(sizeof(*_tmp950)),((_tmp950->kind=_tmp18E,((_tmp950->sc=s,((
_tmp950->name=_tmp18F,((_tmp950->tvs=_tmp1AE,((_tmp950->impl=0,((_tmp950->attributes=
0,_tmp950)))))))))))));if(_tmp174 != 0){const char*_tmp951;Cyc_Parse_err(((_tmp951="bad attributes on type declaration",
_tag_dyneither(_tmp951,sizeof(char),35))),loc);}{struct Cyc_Absyn_Aggr_d_struct*
_tmp957;struct Cyc_Absyn_Aggr_d_struct _tmp956;struct Cyc_List_List*_tmp955;struct
Cyc_List_List*_tmp1B4=(_tmp955=_cycalloc(sizeof(*_tmp955)),((_tmp955->hd=Cyc_Absyn_new_decl((
void*)((_tmp957=_cycalloc(sizeof(*_tmp957)),((_tmp957[0]=((_tmp956.tag=6,((
_tmp956.f1=_tmp1AF,_tmp956)))),_tmp957)))),loc),((_tmp955->tl=0,_tmp955)))));
_npop_handler(0);return _tmp1B4;};}_LLA9: {struct Cyc_Absyn_DatatypeType_struct*
_tmp191=(struct Cyc_Absyn_DatatypeType_struct*)_tmp177;if(_tmp191->tag != 3)goto
_LLAB;else{_tmp192=_tmp191->f1;_tmp193=_tmp192.datatype_info;if((_tmp193.KnownDatatype).tag
!= 2)goto _LLAB;_tmp194=(struct Cyc_Absyn_Datatypedecl**)(_tmp193.KnownDatatype).val;}}
_LLAA: if(_tmp174 != 0){const char*_tmp958;Cyc_Parse_err(((_tmp958="bad attributes on datatype",
_tag_dyneither(_tmp958,sizeof(char),27))),loc);}{struct Cyc_Absyn_Datatype_d_struct*
_tmp95E;struct Cyc_Absyn_Datatype_d_struct _tmp95D;struct Cyc_List_List*_tmp95C;
struct Cyc_List_List*_tmp1BA=(_tmp95C=_cycalloc(sizeof(*_tmp95C)),((_tmp95C->hd=
Cyc_Absyn_new_decl((void*)((_tmp95E=_cycalloc(sizeof(*_tmp95E)),((_tmp95E[0]=((
_tmp95D.tag=7,((_tmp95D.f1=*_tmp194,_tmp95D)))),_tmp95E)))),loc),((_tmp95C->tl=0,
_tmp95C)))));_npop_handler(0);return _tmp1BA;};_LLAB: {struct Cyc_Absyn_DatatypeType_struct*
_tmp195=(struct Cyc_Absyn_DatatypeType_struct*)_tmp177;if(_tmp195->tag != 3)goto
_LLAD;else{_tmp196=_tmp195->f1;_tmp197=_tmp196.datatype_info;if((_tmp197.UnknownDatatype).tag
!= 1)goto _LLAD;_tmp198=(struct Cyc_Absyn_UnknownDatatypeInfo)(_tmp197.UnknownDatatype).val;
_tmp199=_tmp198.name;_tmp19A=_tmp198.is_extensible;_tmp19B=_tmp196.targs;}}_LLAC: {
struct Cyc_List_List*_tmp1BB=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*f)(
unsigned int,void*),unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,
loc,_tmp19B);struct Cyc_Absyn_Decl*_tmp1BC=Cyc_Absyn_datatype_decl(s,_tmp199,
_tmp1BB,0,_tmp19A,loc);if(_tmp174 != 0){const char*_tmp95F;Cyc_Parse_err(((_tmp95F="bad attributes on datatype",
_tag_dyneither(_tmp95F,sizeof(char),27))),loc);}{struct Cyc_List_List*_tmp960;
struct Cyc_List_List*_tmp1BF=(_tmp960=_cycalloc(sizeof(*_tmp960)),((_tmp960->hd=
_tmp1BC,((_tmp960->tl=0,_tmp960)))));_npop_handler(0);return _tmp1BF;};}_LLAD: {
struct Cyc_Absyn_EnumType_struct*_tmp19C=(struct Cyc_Absyn_EnumType_struct*)
_tmp177;if(_tmp19C->tag != 13)goto _LLAF;else{_tmp19D=_tmp19C->f1;}}_LLAE: {struct
Cyc_Absyn_Enumdecl*_tmp961;struct Cyc_Absyn_Enumdecl*_tmp1C0=(_tmp961=_cycalloc(
sizeof(*_tmp961)),((_tmp961->sc=s,((_tmp961->name=_tmp19D,((_tmp961->fields=0,
_tmp961)))))));if(_tmp174 != 0){const char*_tmp962;Cyc_Parse_err(((_tmp962="bad attributes on enum",
_tag_dyneither(_tmp962,sizeof(char),23))),loc);}{struct Cyc_Absyn_Decl*_tmp96C;
struct Cyc_Absyn_Enum_d_struct _tmp96B;struct Cyc_Absyn_Enum_d_struct*_tmp96A;
struct Cyc_List_List*_tmp969;struct Cyc_List_List*_tmp1C6=(_tmp969=_cycalloc(
sizeof(*_tmp969)),((_tmp969->hd=((_tmp96C=_cycalloc(sizeof(*_tmp96C)),((_tmp96C->r=(
void*)((_tmp96A=_cycalloc(sizeof(*_tmp96A)),((_tmp96A[0]=((_tmp96B.tag=8,((
_tmp96B.f1=_tmp1C0,_tmp96B)))),_tmp96A)))),((_tmp96C->loc=loc,_tmp96C)))))),((
_tmp969->tl=0,_tmp969)))));_npop_handler(0);return _tmp1C6;};}_LLAF: {struct Cyc_Absyn_AnonEnumType_struct*
_tmp19E=(struct Cyc_Absyn_AnonEnumType_struct*)_tmp177;if(_tmp19E->tag != 14)goto
_LLB1;else{_tmp19F=_tmp19E->f1;}}_LLB0: {struct Cyc_Core_Opt*_tmp96F;struct Cyc_Absyn_Enumdecl*
_tmp96E;struct Cyc_Absyn_Enumdecl*_tmp1C8=(_tmp96E=_cycalloc(sizeof(*_tmp96E)),((
_tmp96E->sc=s,((_tmp96E->name=Cyc_Parse_gensym_enum(),((_tmp96E->fields=((
_tmp96F=_cycalloc(sizeof(*_tmp96F)),((_tmp96F->v=_tmp19F,_tmp96F)))),_tmp96E)))))));
if(_tmp174 != 0){const char*_tmp970;Cyc_Parse_err(((_tmp970="bad attributes on enum",
_tag_dyneither(_tmp970,sizeof(char),23))),loc);}{struct Cyc_Absyn_Decl*_tmp97A;
struct Cyc_Absyn_Enum_d_struct _tmp979;struct Cyc_Absyn_Enum_d_struct*_tmp978;
struct Cyc_List_List*_tmp977;struct Cyc_List_List*_tmp1CE=(_tmp977=_cycalloc(
sizeof(*_tmp977)),((_tmp977->hd=((_tmp97A=_cycalloc(sizeof(*_tmp97A)),((_tmp97A->r=(
void*)((_tmp978=_cycalloc(sizeof(*_tmp978)),((_tmp978[0]=((_tmp979.tag=8,((
_tmp979.f1=_tmp1C8,_tmp979)))),_tmp978)))),((_tmp97A->loc=loc,_tmp97A)))))),((
_tmp977->tl=0,_tmp977)))));_npop_handler(0);return _tmp1CE;};}_LLB1:;_LLB2:{const
char*_tmp97B;Cyc_Parse_err(((_tmp97B="missing declarator",_tag_dyneither(_tmp97B,
sizeof(char),19))),loc);}{struct Cyc_List_List*_tmp1D2=0;_npop_handler(0);return
_tmp1D2;};_LLA0:;}else{struct Cyc_List_List*_tmp1D3=Cyc_Parse_apply_tmss(mkrgn,
_tmp172,_tmp176,declarators,_tmp174);if(istypedef){if(!exps_empty){const char*
_tmp97C;Cyc_Parse_err(((_tmp97C="initializer in typedef declaration",
_tag_dyneither(_tmp97C,sizeof(char),35))),loc);}{struct Cyc_List_List*decls=((
struct Cyc_List_List*(*)(struct Cyc_Absyn_Decl*(*f)(unsigned int,struct _tuple16*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_v_typ_to_typedef,
loc,_tmp1D3);struct Cyc_List_List*_tmp1D5=decls;_npop_handler(0);return _tmp1D5;};}
else{struct Cyc_List_List*decls=0;{struct Cyc_List_List*_tmp1D6=_tmp1D3;for(0;
_tmp1D6 != 0;(_tmp1D6=_tmp1D6->tl,exprs=((struct Cyc_List_List*)_check_null(exprs))->tl)){
struct _tuple16 _tmp1D8;struct _tuple0*_tmp1D9;struct Cyc_Absyn_Tqual _tmp1DA;void*
_tmp1DB;struct Cyc_List_List*_tmp1DC;struct Cyc_List_List*_tmp1DD;struct _tuple16*
_tmp1D7=(struct _tuple16*)_tmp1D6->hd;_tmp1D8=*_tmp1D7;_tmp1D9=_tmp1D8.f1;_tmp1DA=
_tmp1D8.f2;_tmp1DB=_tmp1D8.f3;_tmp1DC=_tmp1D8.f4;_tmp1DD=_tmp1D8.f5;if(_tmp1DC != 
0){const char*_tmp97F;void*_tmp97E;(_tmp97E=0,Cyc_Tcutil_warn(loc,((_tmp97F="bad type params, ignoring",
_tag_dyneither(_tmp97F,sizeof(char),26))),_tag_dyneither(_tmp97E,sizeof(void*),0)));}
if(exprs == 0){const char*_tmp982;void*_tmp981;(_tmp981=0,((int(*)(unsigned int sg,
struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Parse_abort)(loc,((_tmp982="unexpected NULL in parse!",
_tag_dyneither(_tmp982,sizeof(char),26))),_tag_dyneither(_tmp981,sizeof(void*),0)));}{
struct Cyc_Absyn_Exp*_tmp1E2=(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)
_check_null(exprs))->hd;struct Cyc_Absyn_Vardecl*_tmp1E3=Cyc_Absyn_new_vardecl(
_tmp1D9,_tmp1DB,_tmp1E2);_tmp1E3->tq=_tmp1DA;_tmp1E3->sc=s;_tmp1E3->attributes=
_tmp1DD;{struct Cyc_Absyn_Var_d_struct*_tmp988;struct Cyc_Absyn_Var_d_struct
_tmp987;struct Cyc_Absyn_Decl*_tmp986;struct Cyc_Absyn_Decl*_tmp1E4=(_tmp986=
_cycalloc(sizeof(*_tmp986)),((_tmp986->r=(void*)((_tmp988=_cycalloc(sizeof(*
_tmp988)),((_tmp988[0]=((_tmp987.tag=0,((_tmp987.f1=_tmp1E3,_tmp987)))),_tmp988)))),((
_tmp986->loc=loc,_tmp986)))));struct Cyc_List_List*_tmp989;decls=((_tmp989=
_cycalloc(sizeof(*_tmp989)),((_tmp989->hd=_tmp1E4,((_tmp989->tl=decls,_tmp989))))));};};}}{
struct Cyc_List_List*_tmp1E9=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(
decls);_npop_handler(0);return _tmp1E9;};}}};};};};};_pop_region(mkrgn);}static
struct Cyc_Absyn_Kind*Cyc_Parse_id_to_kind(struct _dyneither_ptr s,unsigned int loc){
if(Cyc_strlen((struct _dyneither_ptr)s)== 1  || Cyc_strlen((struct _dyneither_ptr)s)
== 2)switch(*((const char*)_check_dyneither_subscript(s,sizeof(char),0))){case 'A':
_LLB3: return& Cyc_Tcutil_ak;case 'M': _LLB4: return& Cyc_Tcutil_mk;case 'B': _LLB5:
return& Cyc_Tcutil_bk;case 'R': _LLB6: return& Cyc_Tcutil_rk;case 'E': _LLB7: return& Cyc_Tcutil_ek;
case 'I': _LLB8: return& Cyc_Tcutil_ik;case 'U': _LLB9: switch(*((const char*)
_check_dyneither_subscript(s,sizeof(char),1))){case 'R': _LLBB: return& Cyc_Tcutil_urk;
case 'A': _LLBC: return& Cyc_Tcutil_uak;case 'M': _LLBD: return& Cyc_Tcutil_umk;case 'B':
_LLBE: return& Cyc_Tcutil_ubk;default: _LLBF: break;}break;case 'T': _LLBA: switch(*((
const char*)_check_dyneither_subscript(s,sizeof(char),1))){case 'R': _LLC2: return&
Cyc_Tcutil_trk;case 'A': _LLC3: return& Cyc_Tcutil_tak;case 'M': _LLC4: return& Cyc_Tcutil_tmk;
case 'B': _LLC5: return& Cyc_Tcutil_tbk;default: _LLC6: break;}break;default: _LLC1:
break;}{const char*_tmp98E;void*_tmp98D[2];struct Cyc_String_pa_struct _tmp98C;
struct Cyc_Int_pa_struct _tmp98B;Cyc_Parse_err((struct _dyneither_ptr)((_tmp98B.tag=
1,((_tmp98B.f1=(unsigned long)((int)Cyc_strlen((struct _dyneither_ptr)s)),((
_tmp98C.tag=0,((_tmp98C.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s),((
_tmp98D[0]=& _tmp98C,((_tmp98D[1]=& _tmp98B,Cyc_aprintf(((_tmp98E="bad kind: %s; strlen=%d",
_tag_dyneither(_tmp98E,sizeof(char),24))),_tag_dyneither(_tmp98D,sizeof(void*),2)))))))))))))),
loc);}return& Cyc_Tcutil_bk;}static struct Cyc_Absyn_Exp*Cyc_Parse_pat2exp(struct
Cyc_Absyn_Pat*p){void*_tmp1EE=p->r;struct _tuple0*_tmp1F0;struct Cyc_Absyn_Vardecl*
_tmp1F2;struct Cyc_Absyn_Pat*_tmp1F3;struct Cyc_Absyn_Pat _tmp1F4;void*_tmp1F5;
struct Cyc_Absyn_Pat*_tmp1F8;enum Cyc_Absyn_Sign _tmp1FB;int _tmp1FC;char _tmp1FE;
struct _dyneither_ptr _tmp200;int _tmp201;struct _tuple0*_tmp203;struct Cyc_List_List*
_tmp204;int _tmp205;struct Cyc_Absyn_Exp*_tmp207;_LLCA: {struct Cyc_Absyn_UnknownId_p_struct*
_tmp1EF=(struct Cyc_Absyn_UnknownId_p_struct*)_tmp1EE;if(_tmp1EF->tag != 14)goto
_LLCC;else{_tmp1F0=_tmp1EF->f1;}}_LLCB: return Cyc_Absyn_unknownid_exp(_tmp1F0,p->loc);
_LLCC: {struct Cyc_Absyn_Reference_p_struct*_tmp1F1=(struct Cyc_Absyn_Reference_p_struct*)
_tmp1EE;if(_tmp1F1->tag != 2)goto _LLCE;else{_tmp1F2=_tmp1F1->f1;_tmp1F3=_tmp1F1->f2;
_tmp1F4=*_tmp1F3;_tmp1F5=_tmp1F4.r;{struct Cyc_Absyn_Wild_p_struct*_tmp1F6=(
struct Cyc_Absyn_Wild_p_struct*)_tmp1F5;if(_tmp1F6->tag != 0)goto _LLCE;};}}_LLCD:
return Cyc_Absyn_deref_exp(Cyc_Absyn_unknownid_exp(_tmp1F2->name,p->loc),p->loc);
_LLCE: {struct Cyc_Absyn_Pointer_p_struct*_tmp1F7=(struct Cyc_Absyn_Pointer_p_struct*)
_tmp1EE;if(_tmp1F7->tag != 5)goto _LLD0;else{_tmp1F8=_tmp1F7->f1;}}_LLCF: return Cyc_Absyn_address_exp(
Cyc_Parse_pat2exp(_tmp1F8),p->loc);_LLD0: {struct Cyc_Absyn_Null_p_struct*_tmp1F9=(
struct Cyc_Absyn_Null_p_struct*)_tmp1EE;if(_tmp1F9->tag != 8)goto _LLD2;}_LLD1:
return Cyc_Absyn_null_exp(p->loc);_LLD2: {struct Cyc_Absyn_Int_p_struct*_tmp1FA=(
struct Cyc_Absyn_Int_p_struct*)_tmp1EE;if(_tmp1FA->tag != 9)goto _LLD4;else{_tmp1FB=
_tmp1FA->f1;_tmp1FC=_tmp1FA->f2;}}_LLD3: return Cyc_Absyn_int_exp(_tmp1FB,_tmp1FC,
p->loc);_LLD4: {struct Cyc_Absyn_Char_p_struct*_tmp1FD=(struct Cyc_Absyn_Char_p_struct*)
_tmp1EE;if(_tmp1FD->tag != 10)goto _LLD6;else{_tmp1FE=_tmp1FD->f1;}}_LLD5: return
Cyc_Absyn_char_exp(_tmp1FE,p->loc);_LLD6: {struct Cyc_Absyn_Float_p_struct*
_tmp1FF=(struct Cyc_Absyn_Float_p_struct*)_tmp1EE;if(_tmp1FF->tag != 11)goto _LLD8;
else{_tmp200=_tmp1FF->f1;_tmp201=_tmp1FF->f2;}}_LLD7: return Cyc_Absyn_float_exp(
_tmp200,_tmp201,p->loc);_LLD8: {struct Cyc_Absyn_UnknownCall_p_struct*_tmp202=(
struct Cyc_Absyn_UnknownCall_p_struct*)_tmp1EE;if(_tmp202->tag != 15)goto _LLDA;
else{_tmp203=_tmp202->f1;_tmp204=_tmp202->f2;_tmp205=_tmp202->f3;if(_tmp205 != 0)
goto _LLDA;}}_LLD9: {struct Cyc_Absyn_Exp*e1=Cyc_Absyn_unknownid_exp(_tmp203,p->loc);
struct Cyc_List_List*es=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct
Cyc_Absyn_Pat*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Parse_pat2exp,_tmp204);
return Cyc_Absyn_unknowncall_exp(e1,es,p->loc);}_LLDA: {struct Cyc_Absyn_Exp_p_struct*
_tmp206=(struct Cyc_Absyn_Exp_p_struct*)_tmp1EE;if(_tmp206->tag != 16)goto _LLDC;
else{_tmp207=_tmp206->f1;}}_LLDB: return _tmp207;_LLDC:;_LLDD:{const char*_tmp98F;
Cyc_Parse_err(((_tmp98F="cannot mix patterns and expressions in case",
_tag_dyneither(_tmp98F,sizeof(char),44))),p->loc);}return Cyc_Absyn_null_exp(p->loc);
_LLC9:;}struct _union_YYSTYPE_Int_tok{int tag;struct _tuple5 val;};struct
_union_YYSTYPE_Char_tok{int tag;char val;};struct _union_YYSTYPE_String_tok{int tag;
struct _dyneither_ptr val;};struct _union_YYSTYPE_Stringopt_tok{int tag;struct Cyc_Core_Opt*
val;};struct _union_YYSTYPE_QualId_tok{int tag;struct _tuple0*val;};struct _tuple20{
int f1;struct _dyneither_ptr f2;};struct _union_YYSTYPE_Asm_tok{int tag;struct
_tuple20 val;};struct _tuple21{unsigned int f1;union Cyc_Absyn_Constraint*f2;union
Cyc_Absyn_Constraint*f3;};struct _union_YYSTYPE_YY1{int tag;struct _tuple21*val;};
struct _union_YYSTYPE_YY2{int tag;union Cyc_Absyn_Constraint*val;};struct
_union_YYSTYPE_YY3{int tag;struct Cyc_Absyn_Exp*val;};struct _union_YYSTYPE_YY4{int
tag;struct Cyc_Absyn_Exp*val;};struct _union_YYSTYPE_YY5{int tag;struct Cyc_List_List*
val;};struct _union_YYSTYPE_YY6{int tag;struct Cyc_List_List*val;};struct
_union_YYSTYPE_YY7{int tag;enum Cyc_Absyn_Primop val;};struct _union_YYSTYPE_YY8{
int tag;struct Cyc_Core_Opt*val;};struct _union_YYSTYPE_YY9{int tag;struct Cyc_Absyn_Stmt*
val;};struct _union_YYSTYPE_YY10{int tag;struct Cyc_List_List*val;};struct
_union_YYSTYPE_YY11{int tag;struct Cyc_Absyn_Pat*val;};struct _tuple22{struct Cyc_List_List*
f1;int f2;};struct _union_YYSTYPE_YY12{int tag;struct _tuple22*val;};struct
_union_YYSTYPE_YY13{int tag;struct Cyc_List_List*val;};struct _tuple23{struct Cyc_List_List*
f1;struct Cyc_Absyn_Pat*f2;};struct _union_YYSTYPE_YY14{int tag;struct _tuple23*val;
};struct _union_YYSTYPE_YY15{int tag;struct Cyc_List_List*val;};struct
_union_YYSTYPE_YY16{int tag;struct _tuple22*val;};struct _union_YYSTYPE_YY17{int tag;
struct Cyc_Absyn_Fndecl*val;};struct _union_YYSTYPE_YY18{int tag;struct Cyc_List_List*
val;};struct _union_YYSTYPE_YY19{int tag;struct Cyc_Parse_Declaration_spec val;};
struct _union_YYSTYPE_YY20{int tag;struct _tuple12 val;};struct _union_YYSTYPE_YY21{
int tag;struct _tuple13*val;};struct _union_YYSTYPE_YY22{int tag;enum Cyc_Parse_Storage_class*
val;};struct _union_YYSTYPE_YY23{int tag;struct Cyc_Parse_Type_specifier val;};
struct _union_YYSTYPE_YY24{int tag;enum Cyc_Absyn_AggrKind val;};struct
_union_YYSTYPE_YY25{int tag;struct Cyc_Absyn_Tqual val;};struct _union_YYSTYPE_YY26{
int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY27{int tag;struct Cyc_List_List*
val;};struct _union_YYSTYPE_YY28{int tag;struct Cyc_List_List*val;};struct
_union_YYSTYPE_YY29{int tag;struct Cyc_Parse_Declarator val;};struct
_union_YYSTYPE_YY30{int tag;struct Cyc_Parse_Abstractdeclarator val;};struct
_union_YYSTYPE_YY31{int tag;int val;};struct _union_YYSTYPE_YY32{int tag;enum Cyc_Absyn_Scope
val;};struct _union_YYSTYPE_YY33{int tag;struct Cyc_Absyn_Datatypefield*val;};
struct _union_YYSTYPE_YY34{int tag;struct Cyc_List_List*val;};struct _tuple24{struct
Cyc_Absyn_Tqual f1;struct Cyc_Parse_Type_specifier f2;struct Cyc_List_List*f3;};
struct _union_YYSTYPE_YY35{int tag;struct _tuple24 val;};struct _union_YYSTYPE_YY36{
int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY37{int tag;struct _tuple8*
val;};struct _union_YYSTYPE_YY38{int tag;struct Cyc_List_List*val;};struct _tuple25{
struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*
f5;};struct _union_YYSTYPE_YY39{int tag;struct _tuple25*val;};struct
_union_YYSTYPE_YY40{int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY41{
int tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY42{int tag;void*val;};
struct _union_YYSTYPE_YY43{int tag;struct Cyc_Absyn_Kind*val;};struct
_union_YYSTYPE_YY44{int tag;void*val;};struct _union_YYSTYPE_YY45{int tag;struct Cyc_List_List*
val;};struct _union_YYSTYPE_YY46{int tag;void*val;};struct _union_YYSTYPE_YY47{int
tag;struct Cyc_Absyn_Enumfield*val;};struct _union_YYSTYPE_YY48{int tag;struct Cyc_List_List*
val;};struct _union_YYSTYPE_YY49{int tag;void*val;};struct _union_YYSTYPE_YY50{int
tag;struct Cyc_List_List*val;};struct _union_YYSTYPE_YY51{int tag;union Cyc_Absyn_Constraint*
val;};struct _union_YYSTYPE_YY52{int tag;struct Cyc_List_List*val;};struct
_union_YYSTYPE_YY53{int tag;void*val;};struct _union_YYSTYPE_YY54{int tag;struct Cyc_List_List*
val;};struct _union_YYSTYPE_YYINITIALSVAL{int tag;int val;};union Cyc_YYSTYPE{struct
_union_YYSTYPE_Int_tok Int_tok;struct _union_YYSTYPE_Char_tok Char_tok;struct
_union_YYSTYPE_String_tok String_tok;struct _union_YYSTYPE_Stringopt_tok
Stringopt_tok;struct _union_YYSTYPE_QualId_tok QualId_tok;struct
_union_YYSTYPE_Asm_tok Asm_tok;struct _union_YYSTYPE_YY1 YY1;struct
_union_YYSTYPE_YY2 YY2;struct _union_YYSTYPE_YY3 YY3;struct _union_YYSTYPE_YY4 YY4;
struct _union_YYSTYPE_YY5 YY5;struct _union_YYSTYPE_YY6 YY6;struct _union_YYSTYPE_YY7
YY7;struct _union_YYSTYPE_YY8 YY8;struct _union_YYSTYPE_YY9 YY9;struct
_union_YYSTYPE_YY10 YY10;struct _union_YYSTYPE_YY11 YY11;struct _union_YYSTYPE_YY12
YY12;struct _union_YYSTYPE_YY13 YY13;struct _union_YYSTYPE_YY14 YY14;struct
_union_YYSTYPE_YY15 YY15;struct _union_YYSTYPE_YY16 YY16;struct _union_YYSTYPE_YY17
YY17;struct _union_YYSTYPE_YY18 YY18;struct _union_YYSTYPE_YY19 YY19;struct
_union_YYSTYPE_YY20 YY20;struct _union_YYSTYPE_YY21 YY21;struct _union_YYSTYPE_YY22
YY22;struct _union_YYSTYPE_YY23 YY23;struct _union_YYSTYPE_YY24 YY24;struct
_union_YYSTYPE_YY25 YY25;struct _union_YYSTYPE_YY26 YY26;struct _union_YYSTYPE_YY27
YY27;struct _union_YYSTYPE_YY28 YY28;struct _union_YYSTYPE_YY29 YY29;struct
_union_YYSTYPE_YY30 YY30;struct _union_YYSTYPE_YY31 YY31;struct _union_YYSTYPE_YY32
YY32;struct _union_YYSTYPE_YY33 YY33;struct _union_YYSTYPE_YY34 YY34;struct
_union_YYSTYPE_YY35 YY35;struct _union_YYSTYPE_YY36 YY36;struct _union_YYSTYPE_YY37
YY37;struct _union_YYSTYPE_YY38 YY38;struct _union_YYSTYPE_YY39 YY39;struct
_union_YYSTYPE_YY40 YY40;struct _union_YYSTYPE_YY41 YY41;struct _union_YYSTYPE_YY42
YY42;struct _union_YYSTYPE_YY43 YY43;struct _union_YYSTYPE_YY44 YY44;struct
_union_YYSTYPE_YY45 YY45;struct _union_YYSTYPE_YY46 YY46;struct _union_YYSTYPE_YY47
YY47;struct _union_YYSTYPE_YY48 YY48;struct _union_YYSTYPE_YY49 YY49;struct
_union_YYSTYPE_YY50 YY50;struct _union_YYSTYPE_YY51 YY51;struct _union_YYSTYPE_YY52
YY52;struct _union_YYSTYPE_YY53 YY53;struct _union_YYSTYPE_YY54 YY54;struct
_union_YYSTYPE_YYINITIALSVAL YYINITIALSVAL;};static char _tmp20A[14]="$(sign_t,int)";
static struct Cyc_Core_Failure_struct Cyc_yyfail_Int_tok={Cyc_Core_Failure,{_tmp20A,
_tmp20A,_tmp20A + 14}};struct _tuple5 Cyc_yyget_Int_tok(union Cyc_YYSTYPE yy1){struct
_tuple5 yyzzz;{union Cyc_YYSTYPE _tmp20B=yy1;struct _tuple5 _tmp20C;_LLDF: if((_tmp20B.Int_tok).tag
!= 1)goto _LLE1;_tmp20C=(struct _tuple5)(_tmp20B.Int_tok).val;_LLE0: yyzzz=_tmp20C;
goto _LLDE;_LLE1:;_LLE2:(int)_throw((void*)& Cyc_yyfail_Int_tok);_LLDE:;}return
yyzzz;}union Cyc_YYSTYPE Cyc_Int_tok(struct _tuple5 yy1){union Cyc_YYSTYPE _tmp990;
return((_tmp990.Int_tok).val=yy1,(((_tmp990.Int_tok).tag=1,_tmp990)));}static
char _tmp20F[5]="char";static struct Cyc_Core_Failure_struct Cyc_yyfail_Char_tok={
Cyc_Core_Failure,{_tmp20F,_tmp20F,_tmp20F + 5}};char Cyc_yyget_Char_tok(union Cyc_YYSTYPE
yy1){char yyzzz;{union Cyc_YYSTYPE _tmp210=yy1;char _tmp211;_LLE4: if((_tmp210.Char_tok).tag
!= 2)goto _LLE6;_tmp211=(char)(_tmp210.Char_tok).val;_LLE5: yyzzz=_tmp211;goto
_LLE3;_LLE6:;_LLE7:(int)_throw((void*)& Cyc_yyfail_Char_tok);_LLE3:;}return yyzzz;}
union Cyc_YYSTYPE Cyc_Char_tok(char yy1){union Cyc_YYSTYPE _tmp991;return((_tmp991.Char_tok).val=
yy1,(((_tmp991.Char_tok).tag=2,_tmp991)));}static char _tmp214[13]="string_t<`H>";
static struct Cyc_Core_Failure_struct Cyc_yyfail_String_tok={Cyc_Core_Failure,{
_tmp214,_tmp214,_tmp214 + 13}};struct _dyneither_ptr Cyc_yyget_String_tok(union Cyc_YYSTYPE
yy1){struct _dyneither_ptr yyzzz;{union Cyc_YYSTYPE _tmp215=yy1;struct _dyneither_ptr
_tmp216;_LLE9: if((_tmp215.String_tok).tag != 3)goto _LLEB;_tmp216=(struct
_dyneither_ptr)(_tmp215.String_tok).val;_LLEA: yyzzz=_tmp216;goto _LLE8;_LLEB:;
_LLEC:(int)_throw((void*)& Cyc_yyfail_String_tok);_LLE8:;}return yyzzz;}union Cyc_YYSTYPE
Cyc_String_tok(struct _dyneither_ptr yy1){union Cyc_YYSTYPE _tmp992;return((_tmp992.String_tok).val=
yy1,(((_tmp992.String_tok).tag=3,_tmp992)));}static char _tmp219[56]="$(Position::seg_t,conref_t<bool>,conref_t<bounds_t>)@`H";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY1={Cyc_Core_Failure,{_tmp219,
_tmp219,_tmp219 + 56}};struct _tuple21*Cyc_yyget_YY1(union Cyc_YYSTYPE yy1){struct
_tuple21*yyzzz;{union Cyc_YYSTYPE _tmp21A=yy1;struct _tuple21*_tmp21B;_LLEE: if((
_tmp21A.YY1).tag != 7)goto _LLF0;_tmp21B=(struct _tuple21*)(_tmp21A.YY1).val;_LLEF:
yyzzz=_tmp21B;goto _LLED;_LLF0:;_LLF1:(int)_throw((void*)& Cyc_yyfail_YY1);_LLED:;}
return yyzzz;}union Cyc_YYSTYPE Cyc_YY1(struct _tuple21*yy1){union Cyc_YYSTYPE _tmp993;
return((_tmp993.YY1).val=yy1,(((_tmp993.YY1).tag=7,_tmp993)));}static char _tmp21E[
19]="conref_t<bounds_t>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY2={Cyc_Core_Failure,{
_tmp21E,_tmp21E,_tmp21E + 19}};union Cyc_Absyn_Constraint*Cyc_yyget_YY2(union Cyc_YYSTYPE
yy1){union Cyc_Absyn_Constraint*yyzzz;{union Cyc_YYSTYPE _tmp21F=yy1;union Cyc_Absyn_Constraint*
_tmp220;_LLF3: if((_tmp21F.YY2).tag != 8)goto _LLF5;_tmp220=(union Cyc_Absyn_Constraint*)(
_tmp21F.YY2).val;_LLF4: yyzzz=_tmp220;goto _LLF2;_LLF5:;_LLF6:(int)_throw((void*)&
Cyc_yyfail_YY2);_LLF2:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY2(union Cyc_Absyn_Constraint*
yy1){union Cyc_YYSTYPE _tmp994;return((_tmp994.YY2).val=yy1,(((_tmp994.YY2).tag=8,
_tmp994)));}static char _tmp223[6]="exp_t";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY3={
Cyc_Core_Failure,{_tmp223,_tmp223,_tmp223 + 6}};struct Cyc_Absyn_Exp*Cyc_yyget_YY3(
union Cyc_YYSTYPE yy1){struct Cyc_Absyn_Exp*yyzzz;{union Cyc_YYSTYPE _tmp224=yy1;
struct Cyc_Absyn_Exp*_tmp225;_LLF8: if((_tmp224.YY3).tag != 9)goto _LLFA;_tmp225=(
struct Cyc_Absyn_Exp*)(_tmp224.YY3).val;_LLF9: yyzzz=_tmp225;goto _LLF7;_LLFA:;
_LLFB:(int)_throw((void*)& Cyc_yyfail_YY3);_LLF7:;}return yyzzz;}union Cyc_YYSTYPE
Cyc_YY3(struct Cyc_Absyn_Exp*yy1){union Cyc_YYSTYPE _tmp995;return((_tmp995.YY3).val=
yy1,(((_tmp995.YY3).tag=9,_tmp995)));}static char _tmp228[10]="exp_opt_t";static
struct Cyc_Core_Failure_struct Cyc_yyfail_YY4={Cyc_Core_Failure,{_tmp228,_tmp228,
_tmp228 + 10}};struct Cyc_Absyn_Exp*Cyc_yyget_YY4(union Cyc_YYSTYPE yy1){struct Cyc_Absyn_Exp*
yyzzz;{union Cyc_YYSTYPE _tmp229=yy1;struct Cyc_Absyn_Exp*_tmp22A;_LLFD: if((_tmp229.YY4).tag
!= 10)goto _LLFF;_tmp22A=(struct Cyc_Absyn_Exp*)(_tmp229.YY4).val;_LLFE: yyzzz=
_tmp22A;goto _LLFC;_LLFF:;_LL100:(int)_throw((void*)& Cyc_yyfail_YY4);_LLFC:;}
return yyzzz;}union Cyc_YYSTYPE Cyc_YY4(struct Cyc_Absyn_Exp*yy1){union Cyc_YYSTYPE
_tmp996;return((_tmp996.YY4).val=yy1,(((_tmp996.YY4).tag=10,_tmp996)));}static
char _tmp22D[17]="list_t<exp_t,`H>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY5={
Cyc_Core_Failure,{_tmp22D,_tmp22D,_tmp22D + 17}};struct Cyc_List_List*Cyc_yyget_YY5(
union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp22E=yy1;
struct Cyc_List_List*_tmp22F;_LL102: if((_tmp22E.YY5).tag != 11)goto _LL104;_tmp22F=(
struct Cyc_List_List*)(_tmp22E.YY5).val;_LL103: yyzzz=_tmp22F;goto _LL101;_LL104:;
_LL105:(int)_throw((void*)& Cyc_yyfail_YY5);_LL101:;}return yyzzz;}union Cyc_YYSTYPE
Cyc_YY5(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp997;return((_tmp997.YY5).val=
yy1,(((_tmp997.YY5).tag=11,_tmp997)));}static char _tmp232[47]="list_t<$(list_t<designator_t,`H>,exp_t)@`H,`H>";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY6={Cyc_Core_Failure,{_tmp232,
_tmp232,_tmp232 + 47}};struct Cyc_List_List*Cyc_yyget_YY6(union Cyc_YYSTYPE yy1){
struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp233=yy1;struct Cyc_List_List*
_tmp234;_LL107: if((_tmp233.YY6).tag != 12)goto _LL109;_tmp234=(struct Cyc_List_List*)(
_tmp233.YY6).val;_LL108: yyzzz=_tmp234;goto _LL106;_LL109:;_LL10A:(int)_throw((
void*)& Cyc_yyfail_YY6);_LL106:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY6(struct Cyc_List_List*
yy1){union Cyc_YYSTYPE _tmp998;return((_tmp998.YY6).val=yy1,(((_tmp998.YY6).tag=12,
_tmp998)));}static char _tmp237[9]="primop_t";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY7={Cyc_Core_Failure,{_tmp237,_tmp237,_tmp237 + 9}};enum Cyc_Absyn_Primop
Cyc_yyget_YY7(union Cyc_YYSTYPE yy1){enum Cyc_Absyn_Primop yyzzz;{union Cyc_YYSTYPE
_tmp238=yy1;enum Cyc_Absyn_Primop _tmp239;_LL10C: if((_tmp238.YY7).tag != 13)goto
_LL10E;_tmp239=(enum Cyc_Absyn_Primop)(_tmp238.YY7).val;_LL10D: yyzzz=_tmp239;
goto _LL10B;_LL10E:;_LL10F:(int)_throw((void*)& Cyc_yyfail_YY7);_LL10B:;}return
yyzzz;}union Cyc_YYSTYPE Cyc_YY7(enum Cyc_Absyn_Primop yy1){union Cyc_YYSTYPE _tmp999;
return((_tmp999.YY7).val=yy1,(((_tmp999.YY7).tag=13,_tmp999)));}static char
_tmp23C[19]="opt_t<primop_t,`H>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY8={
Cyc_Core_Failure,{_tmp23C,_tmp23C,_tmp23C + 19}};struct Cyc_Core_Opt*Cyc_yyget_YY8(
union Cyc_YYSTYPE yy1){struct Cyc_Core_Opt*yyzzz;{union Cyc_YYSTYPE _tmp23D=yy1;
struct Cyc_Core_Opt*_tmp23E;_LL111: if((_tmp23D.YY8).tag != 14)goto _LL113;_tmp23E=(
struct Cyc_Core_Opt*)(_tmp23D.YY8).val;_LL112: yyzzz=_tmp23E;goto _LL110;_LL113:;
_LL114:(int)_throw((void*)& Cyc_yyfail_YY8);_LL110:;}return yyzzz;}union Cyc_YYSTYPE
Cyc_YY8(struct Cyc_Core_Opt*yy1){union Cyc_YYSTYPE _tmp99A;return((_tmp99A.YY8).val=
yy1,(((_tmp99A.YY8).tag=14,_tmp99A)));}static char _tmp241[7]="qvar_t";static
struct Cyc_Core_Failure_struct Cyc_yyfail_QualId_tok={Cyc_Core_Failure,{_tmp241,
_tmp241,_tmp241 + 7}};struct _tuple0*Cyc_yyget_QualId_tok(union Cyc_YYSTYPE yy1){
struct _tuple0*yyzzz;{union Cyc_YYSTYPE _tmp242=yy1;struct _tuple0*_tmp243;_LL116:
if((_tmp242.QualId_tok).tag != 5)goto _LL118;_tmp243=(struct _tuple0*)(_tmp242.QualId_tok).val;
_LL117: yyzzz=_tmp243;goto _LL115;_LL118:;_LL119:(int)_throw((void*)& Cyc_yyfail_QualId_tok);
_LL115:;}return yyzzz;}union Cyc_YYSTYPE Cyc_QualId_tok(struct _tuple0*yy1){union Cyc_YYSTYPE
_tmp99B;return((_tmp99B.QualId_tok).val=yy1,(((_tmp99B.QualId_tok).tag=5,_tmp99B)));}
static char _tmp246[7]="stmt_t";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY9={
Cyc_Core_Failure,{_tmp246,_tmp246,_tmp246 + 7}};struct Cyc_Absyn_Stmt*Cyc_yyget_YY9(
union Cyc_YYSTYPE yy1){struct Cyc_Absyn_Stmt*yyzzz;{union Cyc_YYSTYPE _tmp247=yy1;
struct Cyc_Absyn_Stmt*_tmp248;_LL11B: if((_tmp247.YY9).tag != 15)goto _LL11D;_tmp248=(
struct Cyc_Absyn_Stmt*)(_tmp247.YY9).val;_LL11C: yyzzz=_tmp248;goto _LL11A;_LL11D:;
_LL11E:(int)_throw((void*)& Cyc_yyfail_YY9);_LL11A:;}return yyzzz;}union Cyc_YYSTYPE
Cyc_YY9(struct Cyc_Absyn_Stmt*yy1){union Cyc_YYSTYPE _tmp99C;return((_tmp99C.YY9).val=
yy1,(((_tmp99C.YY9).tag=15,_tmp99C)));}static char _tmp24B[27]="list_t<switch_clause_t,`H>";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY10={Cyc_Core_Failure,{_tmp24B,
_tmp24B,_tmp24B + 27}};struct Cyc_List_List*Cyc_yyget_YY10(union Cyc_YYSTYPE yy1){
struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp24C=yy1;struct Cyc_List_List*
_tmp24D;_LL120: if((_tmp24C.YY10).tag != 16)goto _LL122;_tmp24D=(struct Cyc_List_List*)(
_tmp24C.YY10).val;_LL121: yyzzz=_tmp24D;goto _LL11F;_LL122:;_LL123:(int)_throw((
void*)& Cyc_yyfail_YY10);_LL11F:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY10(struct Cyc_List_List*
yy1){union Cyc_YYSTYPE _tmp99D;return((_tmp99D.YY10).val=yy1,(((_tmp99D.YY10).tag=
16,_tmp99D)));}static char _tmp250[6]="pat_t";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY11={Cyc_Core_Failure,{_tmp250,_tmp250,_tmp250 + 6}};struct Cyc_Absyn_Pat*
Cyc_yyget_YY11(union Cyc_YYSTYPE yy1){struct Cyc_Absyn_Pat*yyzzz;{union Cyc_YYSTYPE
_tmp251=yy1;struct Cyc_Absyn_Pat*_tmp252;_LL125: if((_tmp251.YY11).tag != 17)goto
_LL127;_tmp252=(struct Cyc_Absyn_Pat*)(_tmp251.YY11).val;_LL126: yyzzz=_tmp252;
goto _LL124;_LL127:;_LL128:(int)_throw((void*)& Cyc_yyfail_YY11);_LL124:;}return
yyzzz;}union Cyc_YYSTYPE Cyc_YY11(struct Cyc_Absyn_Pat*yy1){union Cyc_YYSTYPE _tmp99E;
return((_tmp99E.YY11).val=yy1,(((_tmp99E.YY11).tag=17,_tmp99E)));}static char
_tmp255[28]="$(list_t<pat_t,`H>,bool)@`H";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY12={
Cyc_Core_Failure,{_tmp255,_tmp255,_tmp255 + 28}};struct _tuple22*Cyc_yyget_YY12(
union Cyc_YYSTYPE yy1){struct _tuple22*yyzzz;{union Cyc_YYSTYPE _tmp256=yy1;struct
_tuple22*_tmp257;_LL12A: if((_tmp256.YY12).tag != 18)goto _LL12C;_tmp257=(struct
_tuple22*)(_tmp256.YY12).val;_LL12B: yyzzz=_tmp257;goto _LL129;_LL12C:;_LL12D:(int)
_throw((void*)& Cyc_yyfail_YY12);_LL129:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY12(
struct _tuple22*yy1){union Cyc_YYSTYPE _tmp99F;return((_tmp99F.YY12).val=yy1,(((
_tmp99F.YY12).tag=18,_tmp99F)));}static char _tmp25A[17]="list_t<pat_t,`H>";static
struct Cyc_Core_Failure_struct Cyc_yyfail_YY13={Cyc_Core_Failure,{_tmp25A,_tmp25A,
_tmp25A + 17}};struct Cyc_List_List*Cyc_yyget_YY13(union Cyc_YYSTYPE yy1){struct Cyc_List_List*
yyzzz;{union Cyc_YYSTYPE _tmp25B=yy1;struct Cyc_List_List*_tmp25C;_LL12F: if((
_tmp25B.YY13).tag != 19)goto _LL131;_tmp25C=(struct Cyc_List_List*)(_tmp25B.YY13).val;
_LL130: yyzzz=_tmp25C;goto _LL12E;_LL131:;_LL132:(int)_throw((void*)& Cyc_yyfail_YY13);
_LL12E:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY13(struct Cyc_List_List*yy1){union Cyc_YYSTYPE
_tmp9A0;return((_tmp9A0.YY13).val=yy1,(((_tmp9A0.YY13).tag=19,_tmp9A0)));}static
char _tmp25F[36]="$(list_t<designator_t,`H>,pat_t)@`H";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY14={Cyc_Core_Failure,{_tmp25F,_tmp25F,_tmp25F + 36}};struct _tuple23*
Cyc_yyget_YY14(union Cyc_YYSTYPE yy1){struct _tuple23*yyzzz;{union Cyc_YYSTYPE
_tmp260=yy1;struct _tuple23*_tmp261;_LL134: if((_tmp260.YY14).tag != 20)goto _LL136;
_tmp261=(struct _tuple23*)(_tmp260.YY14).val;_LL135: yyzzz=_tmp261;goto _LL133;
_LL136:;_LL137:(int)_throw((void*)& Cyc_yyfail_YY14);_LL133:;}return yyzzz;}union
Cyc_YYSTYPE Cyc_YY14(struct _tuple23*yy1){union Cyc_YYSTYPE _tmp9A1;return((_tmp9A1.YY14).val=
yy1,(((_tmp9A1.YY14).tag=20,_tmp9A1)));}static char _tmp264[47]="list_t<$(list_t<designator_t,`H>,pat_t)@`H,`H>";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY15={Cyc_Core_Failure,{_tmp264,
_tmp264,_tmp264 + 47}};struct Cyc_List_List*Cyc_yyget_YY15(union Cyc_YYSTYPE yy1){
struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp265=yy1;struct Cyc_List_List*
_tmp266;_LL139: if((_tmp265.YY15).tag != 21)goto _LL13B;_tmp266=(struct Cyc_List_List*)(
_tmp265.YY15).val;_LL13A: yyzzz=_tmp266;goto _LL138;_LL13B:;_LL13C:(int)_throw((
void*)& Cyc_yyfail_YY15);_LL138:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY15(struct Cyc_List_List*
yy1){union Cyc_YYSTYPE _tmp9A2;return((_tmp9A2.YY15).val=yy1,(((_tmp9A2.YY15).tag=
21,_tmp9A2)));}static char _tmp269[58]="$(list_t<$(list_t<designator_t,`H>,pat_t)@`H,`H>,bool)@`H";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY16={Cyc_Core_Failure,{_tmp269,
_tmp269,_tmp269 + 58}};struct _tuple22*Cyc_yyget_YY16(union Cyc_YYSTYPE yy1){struct
_tuple22*yyzzz;{union Cyc_YYSTYPE _tmp26A=yy1;struct _tuple22*_tmp26B;_LL13E: if((
_tmp26A.YY16).tag != 22)goto _LL140;_tmp26B=(struct _tuple22*)(_tmp26A.YY16).val;
_LL13F: yyzzz=_tmp26B;goto _LL13D;_LL140:;_LL141:(int)_throw((void*)& Cyc_yyfail_YY16);
_LL13D:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY16(struct _tuple22*yy1){union Cyc_YYSTYPE
_tmp9A3;return((_tmp9A3.YY16).val=yy1,(((_tmp9A3.YY16).tag=22,_tmp9A3)));}static
char _tmp26E[9]="fndecl_t";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY17={
Cyc_Core_Failure,{_tmp26E,_tmp26E,_tmp26E + 9}};struct Cyc_Absyn_Fndecl*Cyc_yyget_YY17(
union Cyc_YYSTYPE yy1){struct Cyc_Absyn_Fndecl*yyzzz;{union Cyc_YYSTYPE _tmp26F=yy1;
struct Cyc_Absyn_Fndecl*_tmp270;_LL143: if((_tmp26F.YY17).tag != 23)goto _LL145;
_tmp270=(struct Cyc_Absyn_Fndecl*)(_tmp26F.YY17).val;_LL144: yyzzz=_tmp270;goto
_LL142;_LL145:;_LL146:(int)_throw((void*)& Cyc_yyfail_YY17);_LL142:;}return yyzzz;}
union Cyc_YYSTYPE Cyc_YY17(struct Cyc_Absyn_Fndecl*yy1){union Cyc_YYSTYPE _tmp9A4;
return((_tmp9A4.YY17).val=yy1,(((_tmp9A4.YY17).tag=23,_tmp9A4)));}static char
_tmp273[18]="list_t<decl_t,`H>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY18={
Cyc_Core_Failure,{_tmp273,_tmp273,_tmp273 + 18}};struct Cyc_List_List*Cyc_yyget_YY18(
union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp274=yy1;
struct Cyc_List_List*_tmp275;_LL148: if((_tmp274.YY18).tag != 24)goto _LL14A;_tmp275=(
struct Cyc_List_List*)(_tmp274.YY18).val;_LL149: yyzzz=_tmp275;goto _LL147;_LL14A:;
_LL14B:(int)_throw((void*)& Cyc_yyfail_YY18);_LL147:;}return yyzzz;}union Cyc_YYSTYPE
Cyc_YY18(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp9A5;return((_tmp9A5.YY18).val=
yy1,(((_tmp9A5.YY18).tag=24,_tmp9A5)));}static char _tmp278[12]="decl_spec_t";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY19={Cyc_Core_Failure,{_tmp278,
_tmp278,_tmp278 + 12}};struct Cyc_Parse_Declaration_spec Cyc_yyget_YY19(union Cyc_YYSTYPE
yy1){struct Cyc_Parse_Declaration_spec yyzzz;{union Cyc_YYSTYPE _tmp279=yy1;struct
Cyc_Parse_Declaration_spec _tmp27A;_LL14D: if((_tmp279.YY19).tag != 25)goto _LL14F;
_tmp27A=(struct Cyc_Parse_Declaration_spec)(_tmp279.YY19).val;_LL14E: yyzzz=
_tmp27A;goto _LL14C;_LL14F:;_LL150:(int)_throw((void*)& Cyc_yyfail_YY19);_LL14C:;}
return yyzzz;}union Cyc_YYSTYPE Cyc_YY19(struct Cyc_Parse_Declaration_spec yy1){union
Cyc_YYSTYPE _tmp9A6;return((_tmp9A6.YY19).val=yy1,(((_tmp9A6.YY19).tag=25,_tmp9A6)));}
static char _tmp27D[31]="$(declarator_t<`yy>,exp_opt_t)";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY20={Cyc_Core_Failure,{_tmp27D,_tmp27D,_tmp27D + 31}};struct _tuple12
Cyc_yyget_YY20(union Cyc_YYSTYPE yy1){struct _tuple12 yyzzz;{union Cyc_YYSTYPE _tmp27E=
yy1;struct _tuple12 _tmp27F;_LL152: if((_tmp27E.YY20).tag != 26)goto _LL154;_tmp27F=(
struct _tuple12)(_tmp27E.YY20).val;_LL153: yyzzz=_tmp27F;goto _LL151;_LL154:;_LL155:(
int)_throw((void*)& Cyc_yyfail_YY20);_LL151:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY20(
struct _tuple12 yy1){union Cyc_YYSTYPE _tmp9A7;return((_tmp9A7.YY20).val=yy1,(((
_tmp9A7.YY20).tag=26,_tmp9A7)));}static char _tmp282[23]="declarator_list_t<`yy>";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY21={Cyc_Core_Failure,{_tmp282,
_tmp282,_tmp282 + 23}};struct _tuple13*Cyc_yyget_YY21(union Cyc_YYSTYPE yy1){struct
_tuple13*yyzzz;{union Cyc_YYSTYPE _tmp283=yy1;struct _tuple13*_tmp284;_LL157: if((
_tmp283.YY21).tag != 27)goto _LL159;_tmp284=(struct _tuple13*)(_tmp283.YY21).val;
_LL158: yyzzz=_tmp284;goto _LL156;_LL159:;_LL15A:(int)_throw((void*)& Cyc_yyfail_YY21);
_LL156:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY21(struct _tuple13*yy1){union Cyc_YYSTYPE
_tmp9A8;return((_tmp9A8.YY21).val=yy1,(((_tmp9A8.YY21).tag=27,_tmp9A8)));}static
char _tmp287[19]="storage_class_t@`H";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY22={
Cyc_Core_Failure,{_tmp287,_tmp287,_tmp287 + 19}};enum Cyc_Parse_Storage_class*Cyc_yyget_YY22(
union Cyc_YYSTYPE yy1){enum Cyc_Parse_Storage_class*yyzzz;{union Cyc_YYSTYPE _tmp288=
yy1;enum Cyc_Parse_Storage_class*_tmp289;_LL15C: if((_tmp288.YY22).tag != 28)goto
_LL15E;_tmp289=(enum Cyc_Parse_Storage_class*)(_tmp288.YY22).val;_LL15D: yyzzz=
_tmp289;goto _LL15B;_LL15E:;_LL15F:(int)_throw((void*)& Cyc_yyfail_YY22);_LL15B:;}
return yyzzz;}union Cyc_YYSTYPE Cyc_YY22(enum Cyc_Parse_Storage_class*yy1){union Cyc_YYSTYPE
_tmp9A9;return((_tmp9A9.YY22).val=yy1,(((_tmp9A9.YY22).tag=28,_tmp9A9)));}static
char _tmp28C[17]="type_specifier_t";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY23={
Cyc_Core_Failure,{_tmp28C,_tmp28C,_tmp28C + 17}};struct Cyc_Parse_Type_specifier
Cyc_yyget_YY23(union Cyc_YYSTYPE yy1){struct Cyc_Parse_Type_specifier yyzzz;{union
Cyc_YYSTYPE _tmp28D=yy1;struct Cyc_Parse_Type_specifier _tmp28E;_LL161: if((_tmp28D.YY23).tag
!= 29)goto _LL163;_tmp28E=(struct Cyc_Parse_Type_specifier)(_tmp28D.YY23).val;
_LL162: yyzzz=_tmp28E;goto _LL160;_LL163:;_LL164:(int)_throw((void*)& Cyc_yyfail_YY23);
_LL160:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY23(struct Cyc_Parse_Type_specifier yy1){
union Cyc_YYSTYPE _tmp9AA;return((_tmp9AA.YY23).val=yy1,(((_tmp9AA.YY23).tag=29,
_tmp9AA)));}static char _tmp291[12]="aggr_kind_t";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY24={Cyc_Core_Failure,{_tmp291,_tmp291,_tmp291 + 12}};enum Cyc_Absyn_AggrKind
Cyc_yyget_YY24(union Cyc_YYSTYPE yy1){enum Cyc_Absyn_AggrKind yyzzz;{union Cyc_YYSTYPE
_tmp292=yy1;enum Cyc_Absyn_AggrKind _tmp293;_LL166: if((_tmp292.YY24).tag != 30)
goto _LL168;_tmp293=(enum Cyc_Absyn_AggrKind)(_tmp292.YY24).val;_LL167: yyzzz=
_tmp293;goto _LL165;_LL168:;_LL169:(int)_throw((void*)& Cyc_yyfail_YY24);_LL165:;}
return yyzzz;}union Cyc_YYSTYPE Cyc_YY24(enum Cyc_Absyn_AggrKind yy1){union Cyc_YYSTYPE
_tmp9AB;return((_tmp9AB.YY24).val=yy1,(((_tmp9AB.YY24).tag=30,_tmp9AB)));}static
char _tmp296[8]="tqual_t";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY25={Cyc_Core_Failure,{
_tmp296,_tmp296,_tmp296 + 8}};struct Cyc_Absyn_Tqual Cyc_yyget_YY25(union Cyc_YYSTYPE
yy1){struct Cyc_Absyn_Tqual yyzzz;{union Cyc_YYSTYPE _tmp297=yy1;struct Cyc_Absyn_Tqual
_tmp298;_LL16B: if((_tmp297.YY25).tag != 31)goto _LL16D;_tmp298=(struct Cyc_Absyn_Tqual)(
_tmp297.YY25).val;_LL16C: yyzzz=_tmp298;goto _LL16A;_LL16D:;_LL16E:(int)_throw((
void*)& Cyc_yyfail_YY25);_LL16A:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY25(struct Cyc_Absyn_Tqual
yy1){union Cyc_YYSTYPE _tmp9AC;return((_tmp9AC.YY25).val=yy1,(((_tmp9AC.YY25).tag=
31,_tmp9AC)));}static char _tmp29B[23]="list_t<aggrfield_t,`H>";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY26={Cyc_Core_Failure,{_tmp29B,_tmp29B,_tmp29B + 23}};struct Cyc_List_List*
Cyc_yyget_YY26(union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE
_tmp29C=yy1;struct Cyc_List_List*_tmp29D;_LL170: if((_tmp29C.YY26).tag != 32)goto
_LL172;_tmp29D=(struct Cyc_List_List*)(_tmp29C.YY26).val;_LL171: yyzzz=_tmp29D;
goto _LL16F;_LL172:;_LL173:(int)_throw((void*)& Cyc_yyfail_YY26);_LL16F:;}return
yyzzz;}union Cyc_YYSTYPE Cyc_YY26(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp9AD;
return((_tmp9AD.YY26).val=yy1,(((_tmp9AD.YY26).tag=32,_tmp9AD)));}static char
_tmp2A0[34]="list_t<list_t<aggrfield_t,`H>,`H>";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY27={Cyc_Core_Failure,{_tmp2A0,_tmp2A0,_tmp2A0 + 34}};struct Cyc_List_List*
Cyc_yyget_YY27(union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE
_tmp2A1=yy1;struct Cyc_List_List*_tmp2A2;_LL175: if((_tmp2A1.YY27).tag != 33)goto
_LL177;_tmp2A2=(struct Cyc_List_List*)(_tmp2A1.YY27).val;_LL176: yyzzz=_tmp2A2;
goto _LL174;_LL177:;_LL178:(int)_throw((void*)& Cyc_yyfail_YY27);_LL174:;}return
yyzzz;}union Cyc_YYSTYPE Cyc_YY27(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp9AE;
return((_tmp9AE.YY27).val=yy1,(((_tmp9AE.YY27).tag=33,_tmp9AE)));}static char
_tmp2A5[33]="list_t<type_modifier_t<`yy>,`yy>";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY28={Cyc_Core_Failure,{_tmp2A5,_tmp2A5,_tmp2A5 + 33}};struct Cyc_List_List*
Cyc_yyget_YY28(union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE
_tmp2A6=yy1;struct Cyc_List_List*_tmp2A7;_LL17A: if((_tmp2A6.YY28).tag != 34)goto
_LL17C;_tmp2A7=(struct Cyc_List_List*)(_tmp2A6.YY28).val;_LL17B: yyzzz=_tmp2A7;
goto _LL179;_LL17C:;_LL17D:(int)_throw((void*)& Cyc_yyfail_YY28);_LL179:;}return
yyzzz;}union Cyc_YYSTYPE Cyc_YY28(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp9AF;
return((_tmp9AF.YY28).val=yy1,(((_tmp9AF.YY28).tag=34,_tmp9AF)));}static char
_tmp2AA[18]="declarator_t<`yy>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY29={
Cyc_Core_Failure,{_tmp2AA,_tmp2AA,_tmp2AA + 18}};struct Cyc_Parse_Declarator Cyc_yyget_YY29(
union Cyc_YYSTYPE yy1){struct Cyc_Parse_Declarator yyzzz;{union Cyc_YYSTYPE _tmp2AB=
yy1;struct Cyc_Parse_Declarator _tmp2AC;_LL17F: if((_tmp2AB.YY29).tag != 35)goto
_LL181;_tmp2AC=(struct Cyc_Parse_Declarator)(_tmp2AB.YY29).val;_LL180: yyzzz=
_tmp2AC;goto _LL17E;_LL181:;_LL182:(int)_throw((void*)& Cyc_yyfail_YY29);_LL17E:;}
return yyzzz;}union Cyc_YYSTYPE Cyc_YY29(struct Cyc_Parse_Declarator yy1){union Cyc_YYSTYPE
_tmp9B0;return((_tmp9B0.YY29).val=yy1,(((_tmp9B0.YY29).tag=35,_tmp9B0)));}static
char _tmp2AF[26]="abstractdeclarator_t<`yy>";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY30={Cyc_Core_Failure,{_tmp2AF,_tmp2AF,_tmp2AF + 26}};struct Cyc_Parse_Abstractdeclarator
Cyc_yyget_YY30(union Cyc_YYSTYPE yy1){struct Cyc_Parse_Abstractdeclarator yyzzz;{
union Cyc_YYSTYPE _tmp2B0=yy1;struct Cyc_Parse_Abstractdeclarator _tmp2B1;_LL184: if((
_tmp2B0.YY30).tag != 36)goto _LL186;_tmp2B1=(struct Cyc_Parse_Abstractdeclarator)(
_tmp2B0.YY30).val;_LL185: yyzzz=_tmp2B1;goto _LL183;_LL186:;_LL187:(int)_throw((
void*)& Cyc_yyfail_YY30);_LL183:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY30(struct Cyc_Parse_Abstractdeclarator
yy1){union Cyc_YYSTYPE _tmp9B1;return((_tmp9B1.YY30).val=yy1,(((_tmp9B1.YY30).tag=
36,_tmp9B1)));}static char _tmp2B4[5]="bool";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY31={Cyc_Core_Failure,{_tmp2B4,_tmp2B4,_tmp2B4 + 5}};int Cyc_yyget_YY31(
union Cyc_YYSTYPE yy1){int yyzzz;{union Cyc_YYSTYPE _tmp2B5=yy1;int _tmp2B6;_LL189: if((
_tmp2B5.YY31).tag != 37)goto _LL18B;_tmp2B6=(int)(_tmp2B5.YY31).val;_LL18A: yyzzz=
_tmp2B6;goto _LL188;_LL18B:;_LL18C:(int)_throw((void*)& Cyc_yyfail_YY31);_LL188:;}
return yyzzz;}union Cyc_YYSTYPE Cyc_YY31(int yy1){union Cyc_YYSTYPE _tmp9B2;return((
_tmp9B2.YY31).val=yy1,(((_tmp9B2.YY31).tag=37,_tmp9B2)));}static char _tmp2B9[8]="scope_t";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY32={Cyc_Core_Failure,{_tmp2B9,
_tmp2B9,_tmp2B9 + 8}};enum Cyc_Absyn_Scope Cyc_yyget_YY32(union Cyc_YYSTYPE yy1){
enum Cyc_Absyn_Scope yyzzz;{union Cyc_YYSTYPE _tmp2BA=yy1;enum Cyc_Absyn_Scope
_tmp2BB;_LL18E: if((_tmp2BA.YY32).tag != 38)goto _LL190;_tmp2BB=(enum Cyc_Absyn_Scope)(
_tmp2BA.YY32).val;_LL18F: yyzzz=_tmp2BB;goto _LL18D;_LL190:;_LL191:(int)_throw((
void*)& Cyc_yyfail_YY32);_LL18D:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY32(enum Cyc_Absyn_Scope
yy1){union Cyc_YYSTYPE _tmp9B3;return((_tmp9B3.YY32).val=yy1,(((_tmp9B3.YY32).tag=
38,_tmp9B3)));}static char _tmp2BE[16]="datatypefield_t";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY33={Cyc_Core_Failure,{_tmp2BE,_tmp2BE,_tmp2BE + 16}};struct Cyc_Absyn_Datatypefield*
Cyc_yyget_YY33(union Cyc_YYSTYPE yy1){struct Cyc_Absyn_Datatypefield*yyzzz;{union
Cyc_YYSTYPE _tmp2BF=yy1;struct Cyc_Absyn_Datatypefield*_tmp2C0;_LL193: if((_tmp2BF.YY33).tag
!= 39)goto _LL195;_tmp2C0=(struct Cyc_Absyn_Datatypefield*)(_tmp2BF.YY33).val;
_LL194: yyzzz=_tmp2C0;goto _LL192;_LL195:;_LL196:(int)_throw((void*)& Cyc_yyfail_YY33);
_LL192:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY33(struct Cyc_Absyn_Datatypefield*yy1){
union Cyc_YYSTYPE _tmp9B4;return((_tmp9B4.YY33).val=yy1,(((_tmp9B4.YY33).tag=39,
_tmp9B4)));}static char _tmp2C3[27]="list_t<datatypefield_t,`H>";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY34={Cyc_Core_Failure,{_tmp2C3,_tmp2C3,_tmp2C3 + 27}};struct Cyc_List_List*
Cyc_yyget_YY34(union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE
_tmp2C4=yy1;struct Cyc_List_List*_tmp2C5;_LL198: if((_tmp2C4.YY34).tag != 40)goto
_LL19A;_tmp2C5=(struct Cyc_List_List*)(_tmp2C4.YY34).val;_LL199: yyzzz=_tmp2C5;
goto _LL197;_LL19A:;_LL19B:(int)_throw((void*)& Cyc_yyfail_YY34);_LL197:;}return
yyzzz;}union Cyc_YYSTYPE Cyc_YY34(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp9B5;
return((_tmp9B5.YY34).val=yy1,(((_tmp9B5.YY34).tag=40,_tmp9B5)));}static char
_tmp2C8[41]="$(tqual_t,type_specifier_t,attributes_t)";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY35={Cyc_Core_Failure,{_tmp2C8,_tmp2C8,_tmp2C8 + 41}};struct _tuple24
Cyc_yyget_YY35(union Cyc_YYSTYPE yy1){struct _tuple24 yyzzz;{union Cyc_YYSTYPE _tmp2C9=
yy1;struct _tuple24 _tmp2CA;_LL19D: if((_tmp2C9.YY35).tag != 41)goto _LL19F;_tmp2CA=(
struct _tuple24)(_tmp2C9.YY35).val;_LL19E: yyzzz=_tmp2CA;goto _LL19C;_LL19F:;_LL1A0:(
int)_throw((void*)& Cyc_yyfail_YY35);_LL19C:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY35(
struct _tuple24 yy1){union Cyc_YYSTYPE _tmp9B6;return((_tmp9B6.YY35).val=yy1,(((
_tmp9B6.YY35).tag=41,_tmp9B6)));}static char _tmp2CD[17]="list_t<var_t,`H>";static
struct Cyc_Core_Failure_struct Cyc_yyfail_YY36={Cyc_Core_Failure,{_tmp2CD,_tmp2CD,
_tmp2CD + 17}};struct Cyc_List_List*Cyc_yyget_YY36(union Cyc_YYSTYPE yy1){struct Cyc_List_List*
yyzzz;{union Cyc_YYSTYPE _tmp2CE=yy1;struct Cyc_List_List*_tmp2CF;_LL1A2: if((
_tmp2CE.YY36).tag != 42)goto _LL1A4;_tmp2CF=(struct Cyc_List_List*)(_tmp2CE.YY36).val;
_LL1A3: yyzzz=_tmp2CF;goto _LL1A1;_LL1A4:;_LL1A5:(int)_throw((void*)& Cyc_yyfail_YY36);
_LL1A1:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY36(struct Cyc_List_List*yy1){union Cyc_YYSTYPE
_tmp9B7;return((_tmp9B7.YY36).val=yy1,(((_tmp9B7.YY36).tag=42,_tmp9B7)));}static
char _tmp2D2[31]="$(var_opt_t,tqual_t,type_t)@`H";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY37={Cyc_Core_Failure,{_tmp2D2,_tmp2D2,_tmp2D2 + 31}};struct _tuple8*
Cyc_yyget_YY37(union Cyc_YYSTYPE yy1){struct _tuple8*yyzzz;{union Cyc_YYSTYPE _tmp2D3=
yy1;struct _tuple8*_tmp2D4;_LL1A7: if((_tmp2D3.YY37).tag != 43)goto _LL1A9;_tmp2D4=(
struct _tuple8*)(_tmp2D3.YY37).val;_LL1A8: yyzzz=_tmp2D4;goto _LL1A6;_LL1A9:;_LL1AA:(
int)_throw((void*)& Cyc_yyfail_YY37);_LL1A6:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY37(
struct _tuple8*yy1){union Cyc_YYSTYPE _tmp9B8;return((_tmp9B8.YY37).val=yy1,(((
_tmp9B8.YY37).tag=43,_tmp9B8)));}static char _tmp2D7[42]="list_t<$(var_opt_t,tqual_t,type_t)@`H,`H>";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY38={Cyc_Core_Failure,{_tmp2D7,
_tmp2D7,_tmp2D7 + 42}};struct Cyc_List_List*Cyc_yyget_YY38(union Cyc_YYSTYPE yy1){
struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp2D8=yy1;struct Cyc_List_List*
_tmp2D9;_LL1AC: if((_tmp2D8.YY38).tag != 44)goto _LL1AE;_tmp2D9=(struct Cyc_List_List*)(
_tmp2D8.YY38).val;_LL1AD: yyzzz=_tmp2D9;goto _LL1AB;_LL1AE:;_LL1AF:(int)_throw((
void*)& Cyc_yyfail_YY38);_LL1AB:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY38(struct Cyc_List_List*
yy1){union Cyc_YYSTYPE _tmp9B9;return((_tmp9B9.YY38).val=yy1,(((_tmp9B9.YY38).tag=
44,_tmp9B9)));}static char _tmp2DC[115]="$(list_t<$(var_opt_t,tqual_t,type_t)@`H,`H>, bool,vararg_info_t *`H,type_opt_t, list_t<$(type_t,type_t)@`H,`H>)@`H";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY39={Cyc_Core_Failure,{_tmp2DC,
_tmp2DC,_tmp2DC + 115}};struct _tuple25*Cyc_yyget_YY39(union Cyc_YYSTYPE yy1){struct
_tuple25*yyzzz;{union Cyc_YYSTYPE _tmp2DD=yy1;struct _tuple25*_tmp2DE;_LL1B1: if((
_tmp2DD.YY39).tag != 45)goto _LL1B3;_tmp2DE=(struct _tuple25*)(_tmp2DD.YY39).val;
_LL1B2: yyzzz=_tmp2DE;goto _LL1B0;_LL1B3:;_LL1B4:(int)_throw((void*)& Cyc_yyfail_YY39);
_LL1B0:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY39(struct _tuple25*yy1){union Cyc_YYSTYPE
_tmp9BA;return((_tmp9BA.YY39).val=yy1,(((_tmp9BA.YY39).tag=45,_tmp9BA)));}static
char _tmp2E1[18]="list_t<type_t,`H>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY40={
Cyc_Core_Failure,{_tmp2E1,_tmp2E1,_tmp2E1 + 18}};struct Cyc_List_List*Cyc_yyget_YY40(
union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp2E2=yy1;
struct Cyc_List_List*_tmp2E3;_LL1B6: if((_tmp2E2.YY40).tag != 46)goto _LL1B8;_tmp2E3=(
struct Cyc_List_List*)(_tmp2E2.YY40).val;_LL1B7: yyzzz=_tmp2E3;goto _LL1B5;_LL1B8:;
_LL1B9:(int)_throw((void*)& Cyc_yyfail_YY40);_LL1B5:;}return yyzzz;}union Cyc_YYSTYPE
Cyc_YY40(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp9BB;return((_tmp9BB.YY40).val=
yy1,(((_tmp9BB.YY40).tag=46,_tmp9BB)));}static char _tmp2E6[24]="list_t<designator_t,`H>";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY41={Cyc_Core_Failure,{_tmp2E6,
_tmp2E6,_tmp2E6 + 24}};struct Cyc_List_List*Cyc_yyget_YY41(union Cyc_YYSTYPE yy1){
struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp2E7=yy1;struct Cyc_List_List*
_tmp2E8;_LL1BB: if((_tmp2E7.YY41).tag != 47)goto _LL1BD;_tmp2E8=(struct Cyc_List_List*)(
_tmp2E7.YY41).val;_LL1BC: yyzzz=_tmp2E8;goto _LL1BA;_LL1BD:;_LL1BE:(int)_throw((
void*)& Cyc_yyfail_YY41);_LL1BA:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY41(struct Cyc_List_List*
yy1){union Cyc_YYSTYPE _tmp9BC;return((_tmp9BC.YY41).val=yy1,(((_tmp9BC.YY41).tag=
47,_tmp9BC)));}static char _tmp2EB[13]="designator_t";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY42={Cyc_Core_Failure,{_tmp2EB,_tmp2EB,_tmp2EB + 13}};void*Cyc_yyget_YY42(
union Cyc_YYSTYPE yy1){void*yyzzz;{union Cyc_YYSTYPE _tmp2EC=yy1;void*_tmp2ED;_LL1C0:
if((_tmp2EC.YY42).tag != 48)goto _LL1C2;_tmp2ED=(void*)(_tmp2EC.YY42).val;_LL1C1:
yyzzz=_tmp2ED;goto _LL1BF;_LL1C2:;_LL1C3:(int)_throw((void*)& Cyc_yyfail_YY42);
_LL1BF:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY42(void*yy1){union Cyc_YYSTYPE _tmp9BD;
return((_tmp9BD.YY42).val=yy1,(((_tmp9BD.YY42).tag=48,_tmp9BD)));}static char
_tmp2F0[7]="kind_t";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY43={Cyc_Core_Failure,{
_tmp2F0,_tmp2F0,_tmp2F0 + 7}};struct Cyc_Absyn_Kind*Cyc_yyget_YY43(union Cyc_YYSTYPE
yy1){struct Cyc_Absyn_Kind*yyzzz;{union Cyc_YYSTYPE _tmp2F1=yy1;struct Cyc_Absyn_Kind*
_tmp2F2;_LL1C5: if((_tmp2F1.YY43).tag != 49)goto _LL1C7;_tmp2F2=(struct Cyc_Absyn_Kind*)(
_tmp2F1.YY43).val;_LL1C6: yyzzz=_tmp2F2;goto _LL1C4;_LL1C7:;_LL1C8:(int)_throw((
void*)& Cyc_yyfail_YY43);_LL1C4:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY43(struct Cyc_Absyn_Kind*
yy1){union Cyc_YYSTYPE _tmp9BE;return((_tmp9BE.YY43).val=yy1,(((_tmp9BE.YY43).tag=
49,_tmp9BE)));}static char _tmp2F5[7]="type_t";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY44={Cyc_Core_Failure,{_tmp2F5,_tmp2F5,_tmp2F5 + 7}};void*Cyc_yyget_YY44(
union Cyc_YYSTYPE yy1){void*yyzzz;{union Cyc_YYSTYPE _tmp2F6=yy1;void*_tmp2F7;_LL1CA:
if((_tmp2F6.YY44).tag != 50)goto _LL1CC;_tmp2F7=(void*)(_tmp2F6.YY44).val;_LL1CB:
yyzzz=_tmp2F7;goto _LL1C9;_LL1CC:;_LL1CD:(int)_throw((void*)& Cyc_yyfail_YY44);
_LL1C9:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY44(void*yy1){union Cyc_YYSTYPE _tmp9BF;
return((_tmp9BF.YY44).val=yy1,(((_tmp9BF.YY44).tag=50,_tmp9BF)));}static char
_tmp2FA[23]="list_t<attribute_t,`H>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY45={
Cyc_Core_Failure,{_tmp2FA,_tmp2FA,_tmp2FA + 23}};struct Cyc_List_List*Cyc_yyget_YY45(
union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp2FB=yy1;
struct Cyc_List_List*_tmp2FC;_LL1CF: if((_tmp2FB.YY45).tag != 51)goto _LL1D1;_tmp2FC=(
struct Cyc_List_List*)(_tmp2FB.YY45).val;_LL1D0: yyzzz=_tmp2FC;goto _LL1CE;_LL1D1:;
_LL1D2:(int)_throw((void*)& Cyc_yyfail_YY45);_LL1CE:;}return yyzzz;}union Cyc_YYSTYPE
Cyc_YY45(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp9C0;return((_tmp9C0.YY45).val=
yy1,(((_tmp9C0.YY45).tag=51,_tmp9C0)));}static char _tmp2FF[12]="attribute_t";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY46={Cyc_Core_Failure,{_tmp2FF,
_tmp2FF,_tmp2FF + 12}};void*Cyc_yyget_YY46(union Cyc_YYSTYPE yy1){void*yyzzz;{union
Cyc_YYSTYPE _tmp300=yy1;void*_tmp301;_LL1D4: if((_tmp300.YY46).tag != 52)goto _LL1D6;
_tmp301=(void*)(_tmp300.YY46).val;_LL1D5: yyzzz=_tmp301;goto _LL1D3;_LL1D6:;_LL1D7:(
int)_throw((void*)& Cyc_yyfail_YY46);_LL1D3:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY46(
void*yy1){union Cyc_YYSTYPE _tmp9C1;return((_tmp9C1.YY46).val=yy1,(((_tmp9C1.YY46).tag=
52,_tmp9C1)));}static char _tmp304[12]="enumfield_t";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY47={Cyc_Core_Failure,{_tmp304,_tmp304,_tmp304 + 12}};struct Cyc_Absyn_Enumfield*
Cyc_yyget_YY47(union Cyc_YYSTYPE yy1){struct Cyc_Absyn_Enumfield*yyzzz;{union Cyc_YYSTYPE
_tmp305=yy1;struct Cyc_Absyn_Enumfield*_tmp306;_LL1D9: if((_tmp305.YY47).tag != 53)
goto _LL1DB;_tmp306=(struct Cyc_Absyn_Enumfield*)(_tmp305.YY47).val;_LL1DA: yyzzz=
_tmp306;goto _LL1D8;_LL1DB:;_LL1DC:(int)_throw((void*)& Cyc_yyfail_YY47);_LL1D8:;}
return yyzzz;}union Cyc_YYSTYPE Cyc_YY47(struct Cyc_Absyn_Enumfield*yy1){union Cyc_YYSTYPE
_tmp9C2;return((_tmp9C2.YY47).val=yy1,(((_tmp9C2.YY47).tag=53,_tmp9C2)));}static
char _tmp309[23]="list_t<enumfield_t,`H>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY48={
Cyc_Core_Failure,{_tmp309,_tmp309,_tmp309 + 23}};struct Cyc_List_List*Cyc_yyget_YY48(
union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp30A=yy1;
struct Cyc_List_List*_tmp30B;_LL1DE: if((_tmp30A.YY48).tag != 54)goto _LL1E0;_tmp30B=(
struct Cyc_List_List*)(_tmp30A.YY48).val;_LL1DF: yyzzz=_tmp30B;goto _LL1DD;_LL1E0:;
_LL1E1:(int)_throw((void*)& Cyc_yyfail_YY48);_LL1DD:;}return yyzzz;}union Cyc_YYSTYPE
Cyc_YY48(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp9C3;return((_tmp9C3.YY48).val=
yy1,(((_tmp9C3.YY48).tag=54,_tmp9C3)));}static char _tmp30E[11]="type_opt_t";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY49={Cyc_Core_Failure,{_tmp30E,
_tmp30E,_tmp30E + 11}};void*Cyc_yyget_YY49(union Cyc_YYSTYPE yy1){void*yyzzz;{union
Cyc_YYSTYPE _tmp30F=yy1;void*_tmp310;_LL1E3: if((_tmp30F.YY49).tag != 55)goto _LL1E5;
_tmp310=(void*)(_tmp30F.YY49).val;_LL1E4: yyzzz=_tmp310;goto _LL1E2;_LL1E5:;_LL1E6:(
int)_throw((void*)& Cyc_yyfail_YY49);_LL1E2:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY49(
void*yy1){union Cyc_YYSTYPE _tmp9C4;return((_tmp9C4.YY49).val=yy1,(((_tmp9C4.YY49).tag=
55,_tmp9C4)));}static char _tmp313[31]="list_t<$(type_t,type_t)@`H,`H>";static
struct Cyc_Core_Failure_struct Cyc_yyfail_YY50={Cyc_Core_Failure,{_tmp313,_tmp313,
_tmp313 + 31}};struct Cyc_List_List*Cyc_yyget_YY50(union Cyc_YYSTYPE yy1){struct Cyc_List_List*
yyzzz;{union Cyc_YYSTYPE _tmp314=yy1;struct Cyc_List_List*_tmp315;_LL1E8: if((
_tmp314.YY50).tag != 56)goto _LL1EA;_tmp315=(struct Cyc_List_List*)(_tmp314.YY50).val;
_LL1E9: yyzzz=_tmp315;goto _LL1E7;_LL1EA:;_LL1EB:(int)_throw((void*)& Cyc_yyfail_YY50);
_LL1E7:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY50(struct Cyc_List_List*yy1){union Cyc_YYSTYPE
_tmp9C5;return((_tmp9C5.YY50).val=yy1,(((_tmp9C5.YY50).tag=56,_tmp9C5)));}static
char _tmp318[15]="conref_t<bool>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY51={
Cyc_Core_Failure,{_tmp318,_tmp318,_tmp318 + 15}};union Cyc_Absyn_Constraint*Cyc_yyget_YY51(
union Cyc_YYSTYPE yy1){union Cyc_Absyn_Constraint*yyzzz;{union Cyc_YYSTYPE _tmp319=
yy1;union Cyc_Absyn_Constraint*_tmp31A;_LL1ED: if((_tmp319.YY51).tag != 57)goto
_LL1EF;_tmp31A=(union Cyc_Absyn_Constraint*)(_tmp319.YY51).val;_LL1EE: yyzzz=
_tmp31A;goto _LL1EC;_LL1EF:;_LL1F0:(int)_throw((void*)& Cyc_yyfail_YY51);_LL1EC:;}
return yyzzz;}union Cyc_YYSTYPE Cyc_YY51(union Cyc_Absyn_Constraint*yy1){union Cyc_YYSTYPE
_tmp9C6;return((_tmp9C6.YY51).val=yy1,(((_tmp9C6.YY51).tag=57,_tmp9C6)));}static
char _tmp31D[45]="list_t<$(Position::seg_t,qvar_t,bool)@`H,`H>";static struct Cyc_Core_Failure_struct
Cyc_yyfail_YY52={Cyc_Core_Failure,{_tmp31D,_tmp31D,_tmp31D + 45}};struct Cyc_List_List*
Cyc_yyget_YY52(union Cyc_YYSTYPE yy1){struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE
_tmp31E=yy1;struct Cyc_List_List*_tmp31F;_LL1F2: if((_tmp31E.YY52).tag != 58)goto
_LL1F4;_tmp31F=(struct Cyc_List_List*)(_tmp31E.YY52).val;_LL1F3: yyzzz=_tmp31F;
goto _LL1F1;_LL1F4:;_LL1F5:(int)_throw((void*)& Cyc_yyfail_YY52);_LL1F1:;}return
yyzzz;}union Cyc_YYSTYPE Cyc_YY52(struct Cyc_List_List*yy1){union Cyc_YYSTYPE _tmp9C7;
return((_tmp9C7.YY52).val=yy1,(((_tmp9C7.YY52).tag=58,_tmp9C7)));}static char
_tmp322[20]="pointer_qual_t<`yy>";static struct Cyc_Core_Failure_struct Cyc_yyfail_YY53={
Cyc_Core_Failure,{_tmp322,_tmp322,_tmp322 + 20}};void*Cyc_yyget_YY53(union Cyc_YYSTYPE
yy1){void*yyzzz;{union Cyc_YYSTYPE _tmp323=yy1;void*_tmp324;_LL1F7: if((_tmp323.YY53).tag
!= 59)goto _LL1F9;_tmp324=(void*)(_tmp323.YY53).val;_LL1F8: yyzzz=_tmp324;goto
_LL1F6;_LL1F9:;_LL1FA:(int)_throw((void*)& Cyc_yyfail_YY53);_LL1F6:;}return yyzzz;}
union Cyc_YYSTYPE Cyc_YY53(void*yy1){union Cyc_YYSTYPE _tmp9C8;return((_tmp9C8.YY53).val=
yy1,(((_tmp9C8.YY53).tag=59,_tmp9C8)));}static char _tmp327[21]="pointer_quals_t<`yy>";
static struct Cyc_Core_Failure_struct Cyc_yyfail_YY54={Cyc_Core_Failure,{_tmp327,
_tmp327,_tmp327 + 21}};struct Cyc_List_List*Cyc_yyget_YY54(union Cyc_YYSTYPE yy1){
struct Cyc_List_List*yyzzz;{union Cyc_YYSTYPE _tmp328=yy1;struct Cyc_List_List*
_tmp329;_LL1FC: if((_tmp328.YY54).tag != 60)goto _LL1FE;_tmp329=(struct Cyc_List_List*)(
_tmp328.YY54).val;_LL1FD: yyzzz=_tmp329;goto _LL1FB;_LL1FE:;_LL1FF:(int)_throw((
void*)& Cyc_yyfail_YY54);_LL1FB:;}return yyzzz;}union Cyc_YYSTYPE Cyc_YY54(struct Cyc_List_List*
yy1){union Cyc_YYSTYPE _tmp9C9;return((_tmp9C9.YY54).val=yy1,(((_tmp9C9.YY54).tag=
60,_tmp9C9)));}static char _tmp32C[21]="$(bool,string_t<`H>)";static struct Cyc_Core_Failure_struct
Cyc_yyfail_Asm_tok={Cyc_Core_Failure,{_tmp32C,_tmp32C,_tmp32C + 21}};struct
_tuple20 Cyc_yyget_Asm_tok(union Cyc_YYSTYPE yy1){struct _tuple20 yyzzz;{union Cyc_YYSTYPE
_tmp32D=yy1;struct _tuple20 _tmp32E;_LL201: if((_tmp32D.Asm_tok).tag != 6)goto _LL203;
_tmp32E=(struct _tuple20)(_tmp32D.Asm_tok).val;_LL202: yyzzz=_tmp32E;goto _LL200;
_LL203:;_LL204:(int)_throw((void*)& Cyc_yyfail_Asm_tok);_LL200:;}return yyzzz;}
union Cyc_YYSTYPE Cyc_Asm_tok(struct _tuple20 yy1){union Cyc_YYSTYPE _tmp9CA;return((
_tmp9CA.Asm_tok).val=yy1,(((_tmp9CA.Asm_tok).tag=6,_tmp9CA)));}struct Cyc_Yyltype{
int timestamp;int first_line;int first_column;int last_line;int last_column;};typedef
struct Cyc_Yyltype Cyc_yyltype;struct Cyc_Yyltype Cyc_yynewloc(){struct Cyc_Yyltype
_tmp9CB;return(_tmp9CB.timestamp=0,((_tmp9CB.first_line=0,((_tmp9CB.first_column=
0,((_tmp9CB.last_line=0,((_tmp9CB.last_column=0,_tmp9CB)))))))));}struct Cyc_Yyltype
Cyc_yylloc={0,0,0,0,0};static short Cyc_yytranslate[374]={0,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,145,2,2,129,143,140,2,126,127,134,137,
122,141,131,142,2,2,2,2,2,2,2,2,2,2,130,119,124,123,125,136,135,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,132,2,133,139,128,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,120,138,121,144,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,3,4,5,6,7,8,9,
10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,
37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,
91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
113,114,115,116,117,118};static char _tmp331[2]="$";static char _tmp332[6]="error";
static char _tmp333[12]="$undefined.";static char _tmp334[5]="AUTO";static char
_tmp335[9]="REGISTER";static char _tmp336[7]="STATIC";static char _tmp337[7]="EXTERN";
static char _tmp338[8]="TYPEDEF";static char _tmp339[5]="VOID";static char _tmp33A[5]="CHAR";
static char _tmp33B[6]="SHORT";static char _tmp33C[4]="INT";static char _tmp33D[5]="LONG";
static char _tmp33E[6]="FLOAT";static char _tmp33F[7]="DOUBLE";static char _tmp340[7]="SIGNED";
static char _tmp341[9]="UNSIGNED";static char _tmp342[6]="CONST";static char _tmp343[9]="VOLATILE";
static char _tmp344[9]="RESTRICT";static char _tmp345[7]="STRUCT";static char _tmp346[
6]="UNION";static char _tmp347[5]="CASE";static char _tmp348[8]="DEFAULT";static char
_tmp349[7]="INLINE";static char _tmp34A[7]="SIZEOF";static char _tmp34B[9]="OFFSETOF";
static char _tmp34C[3]="IF";static char _tmp34D[5]="ELSE";static char _tmp34E[7]="SWITCH";
static char _tmp34F[6]="WHILE";static char _tmp350[3]="DO";static char _tmp351[4]="FOR";
static char _tmp352[5]="GOTO";static char _tmp353[9]="CONTINUE";static char _tmp354[6]="BREAK";
static char _tmp355[7]="RETURN";static char _tmp356[5]="ENUM";static char _tmp357[8]="NULL_kw";
static char _tmp358[4]="LET";static char _tmp359[6]="THROW";static char _tmp35A[4]="TRY";
static char _tmp35B[6]="CATCH";static char _tmp35C[7]="EXPORT";static char _tmp35D[4]="NEW";
static char _tmp35E[9]="ABSTRACT";static char _tmp35F[9]="FALLTHRU";static char
_tmp360[6]="USING";static char _tmp361[10]="NAMESPACE";static char _tmp362[9]="DATATYPE";
static char _tmp363[8]="XTUNION";static char _tmp364[7]="TUNION";static char _tmp365[7]="MALLOC";
static char _tmp366[8]="RMALLOC";static char _tmp367[7]="CALLOC";static char _tmp368[8]="RCALLOC";
static char _tmp369[5]="SWAP";static char _tmp36A[9]="REGION_T";static char _tmp36B[6]="TAG_T";
static char _tmp36C[7]="REGION";static char _tmp36D[5]="RNEW";static char _tmp36E[8]="REGIONS";
static char _tmp36F[13]="RESET_REGION";static char _tmp370[16]="NOZEROTERM_QUAL";
static char _tmp371[14]="ZEROTERM_QUAL";static char _tmp372[12]="REGION_QUAL";static
char _tmp373[7]="PORTON";static char _tmp374[8]="PORTOFF";static char _tmp375[12]="DYNREGION_T";
static char _tmp376[6]="ALIAS";static char _tmp377[8]="NUMELTS";static char _tmp378[8]="VALUEOF";
static char _tmp379[10]="VALUEOF_T";static char _tmp37A[9]="TAGCHECK";static char
_tmp37B[13]="NUMELTS_QUAL";static char _tmp37C[10]="THIN_QUAL";static char _tmp37D[9]="FAT_QUAL";
static char _tmp37E[13]="NOTNULL_QUAL";static char _tmp37F[14]="NULLABLE_QUAL";
static char _tmp380[12]="TAGGED_QUAL";static char _tmp381[16]="EXTENSIBLE_QUAL";
static char _tmp382[15]="RESETABLE_QUAL";static char _tmp383[7]="PTR_OP";static char
_tmp384[7]="INC_OP";static char _tmp385[7]="DEC_OP";static char _tmp386[8]="LEFT_OP";
static char _tmp387[9]="RIGHT_OP";static char _tmp388[6]="LE_OP";static char _tmp389[6]="GE_OP";
static char _tmp38A[6]="EQ_OP";static char _tmp38B[6]="NE_OP";static char _tmp38C[7]="AND_OP";
static char _tmp38D[6]="OR_OP";static char _tmp38E[11]="MUL_ASSIGN";static char
_tmp38F[11]="DIV_ASSIGN";static char _tmp390[11]="MOD_ASSIGN";static char _tmp391[11]="ADD_ASSIGN";
static char _tmp392[11]="SUB_ASSIGN";static char _tmp393[12]="LEFT_ASSIGN";static
char _tmp394[13]="RIGHT_ASSIGN";static char _tmp395[11]="AND_ASSIGN";static char
_tmp396[11]="XOR_ASSIGN";static char _tmp397[10]="OR_ASSIGN";static char _tmp398[9]="ELLIPSIS";
static char _tmp399[11]="LEFT_RIGHT";static char _tmp39A[12]="COLON_COLON";static
char _tmp39B[11]="IDENTIFIER";static char _tmp39C[17]="INTEGER_CONSTANT";static char
_tmp39D[7]="STRING";static char _tmp39E[8]="WSTRING";static char _tmp39F[19]="CHARACTER_CONSTANT";
static char _tmp3A0[20]="WCHARACTER_CONSTANT";static char _tmp3A1[18]="FLOATING_CONSTANT";
static char _tmp3A2[9]="TYPE_VAR";static char _tmp3A3[13]="TYPEDEF_NAME";static char
_tmp3A4[16]="QUAL_IDENTIFIER";static char _tmp3A5[18]="QUAL_TYPEDEF_NAME";static
char _tmp3A6[10]="ATTRIBUTE";static char _tmp3A7[4]="ASM";static char _tmp3A8[4]="';'";
static char _tmp3A9[4]="'{'";static char _tmp3AA[4]="'}'";static char _tmp3AB[4]="','";
static char _tmp3AC[4]="'='";static char _tmp3AD[4]="'<'";static char _tmp3AE[4]="'>'";
static char _tmp3AF[4]="'('";static char _tmp3B0[4]="')'";static char _tmp3B1[4]="'_'";
static char _tmp3B2[4]="'$'";static char _tmp3B3[4]="':'";static char _tmp3B4[4]="'.'";
static char _tmp3B5[4]="'['";static char _tmp3B6[4]="']'";static char _tmp3B7[4]="'*'";
static char _tmp3B8[4]="'@'";static char _tmp3B9[4]="'?'";static char _tmp3BA[4]="'+'";
static char _tmp3BB[4]="'|'";static char _tmp3BC[4]="'^'";static char _tmp3BD[4]="'&'";
static char _tmp3BE[4]="'-'";static char _tmp3BF[4]="'/'";static char _tmp3C0[4]="'%'";
static char _tmp3C1[4]="'~'";static char _tmp3C2[4]="'!'";static char _tmp3C3[5]="prog";
static char _tmp3C4[17]="translation_unit";static char _tmp3C5[12]="export_list";
static char _tmp3C6[19]="export_list_values";static char _tmp3C7[21]="external_declaration";
static char _tmp3C8[20]="function_definition";static char _tmp3C9[21]="function_definition2";
static char _tmp3CA[13]="using_action";static char _tmp3CB[15]="unusing_action";
static char _tmp3CC[17]="namespace_action";static char _tmp3CD[19]="unnamespace_action";
static char _tmp3CE[12]="declaration";static char _tmp3CF[19]="resetable_qual_opt";
static char _tmp3D0[17]="declaration_list";static char _tmp3D1[23]="declaration_specifiers";
static char _tmp3D2[24]="storage_class_specifier";static char _tmp3D3[15]="attributes_opt";
static char _tmp3D4[11]="attributes";static char _tmp3D5[15]="attribute_list";static
char _tmp3D6[10]="attribute";static char _tmp3D7[15]="type_specifier";static char
_tmp3D8[25]="type_specifier_notypedef";static char _tmp3D9[5]="kind";static char
_tmp3DA[15]="type_qualifier";static char _tmp3DB[15]="enum_specifier";static char
_tmp3DC[11]="enum_field";static char _tmp3DD[22]="enum_declaration_list";static
char _tmp3DE[26]="struct_or_union_specifier";static char _tmp3DF[16]="type_params_opt";
static char _tmp3E0[16]="struct_or_union";static char _tmp3E1[24]="struct_declaration_list";
static char _tmp3E2[25]="struct_declaration_list0";static char _tmp3E3[21]="init_declarator_list";
static char _tmp3E4[22]="init_declarator_list0";static char _tmp3E5[16]="init_declarator";
static char _tmp3E6[19]="struct_declaration";static char _tmp3E7[25]="specifier_qualifier_list";
static char _tmp3E8[35]="notypedef_specifier_qualifier_list";static char _tmp3E9[23]="struct_declarator_list";
static char _tmp3EA[24]="struct_declarator_list0";static char _tmp3EB[18]="struct_declarator";
static char _tmp3EC[19]="datatype_specifier";static char _tmp3ED[14]="qual_datatype";
static char _tmp3EE[19]="datatypefield_list";static char _tmp3EF[20]="datatypefield_scope";
static char _tmp3F0[14]="datatypefield";static char _tmp3F1[11]="declarator";static
char _tmp3F2[23]="declarator_withtypedef";static char _tmp3F3[18]="direct_declarator";
static char _tmp3F4[30]="direct_declarator_withtypedef";static char _tmp3F5[8]="pointer";
static char _tmp3F6[12]="one_pointer";static char _tmp3F7[14]="pointer_quals";static
char _tmp3F8[13]="pointer_qual";static char _tmp3F9[23]="pointer_null_and_bound";
static char _tmp3FA[14]="pointer_bound";static char _tmp3FB[18]="zeroterm_qual_opt";
static char _tmp3FC[8]="rgn_opt";static char _tmp3FD[11]="tqual_list";static char
_tmp3FE[20]="parameter_type_list";static char _tmp3FF[9]="type_var";static char
_tmp400[16]="optional_effect";static char _tmp401[19]="optional_rgn_order";static
char _tmp402[10]="rgn_order";static char _tmp403[16]="optional_inject";static char
_tmp404[11]="effect_set";static char _tmp405[14]="atomic_effect";static char _tmp406[
11]="region_set";static char _tmp407[15]="parameter_list";static char _tmp408[22]="parameter_declaration";
static char _tmp409[16]="identifier_list";static char _tmp40A[17]="identifier_list0";
static char _tmp40B[12]="initializer";static char _tmp40C[18]="array_initializer";
static char _tmp40D[17]="initializer_list";static char _tmp40E[12]="designation";
static char _tmp40F[16]="designator_list";static char _tmp410[11]="designator";
static char _tmp411[10]="type_name";static char _tmp412[14]="any_type_name";static
char _tmp413[15]="type_name_list";static char _tmp414[20]="abstract_declarator";
static char _tmp415[27]="direct_abstract_declarator";static char _tmp416[10]="statement";
static char _tmp417[13]="open_exp_opt";static char _tmp418[18]="labeled_statement";
static char _tmp419[21]="expression_statement";static char _tmp41A[19]="compound_statement";
static char _tmp41B[16]="block_item_list";static char _tmp41C[20]="selection_statement";
static char _tmp41D[15]="switch_clauses";static char _tmp41E[20]="iteration_statement";
static char _tmp41F[15]="jump_statement";static char _tmp420[12]="exp_pattern";
static char _tmp421[20]="conditional_pattern";static char _tmp422[19]="logical_or_pattern";
static char _tmp423[20]="logical_and_pattern";static char _tmp424[21]="inclusive_or_pattern";
static char _tmp425[21]="exclusive_or_pattern";static char _tmp426[12]="and_pattern";
static char _tmp427[17]="equality_pattern";static char _tmp428[19]="relational_pattern";
static char _tmp429[14]="shift_pattern";static char _tmp42A[17]="additive_pattern";
static char _tmp42B[23]="multiplicative_pattern";static char _tmp42C[13]="cast_pattern";
static char _tmp42D[14]="unary_pattern";static char _tmp42E[16]="postfix_pattern";
static char _tmp42F[16]="primary_pattern";static char _tmp430[8]="pattern";static
char _tmp431[19]="tuple_pattern_list";static char _tmp432[20]="tuple_pattern_list0";
static char _tmp433[14]="field_pattern";static char _tmp434[19]="field_pattern_list";
static char _tmp435[20]="field_pattern_list0";static char _tmp436[11]="expression";
static char _tmp437[22]="assignment_expression";static char _tmp438[20]="assignment_operator";
static char _tmp439[23]="conditional_expression";static char _tmp43A[20]="constant_expression";
static char _tmp43B[22]="logical_or_expression";static char _tmp43C[23]="logical_and_expression";
static char _tmp43D[24]="inclusive_or_expression";static char _tmp43E[24]="exclusive_or_expression";
static char _tmp43F[15]="and_expression";static char _tmp440[20]="equality_expression";
static char _tmp441[22]="relational_expression";static char _tmp442[17]="shift_expression";
static char _tmp443[20]="additive_expression";static char _tmp444[26]="multiplicative_expression";
static char _tmp445[16]="cast_expression";static char _tmp446[17]="unary_expression";
static char _tmp447[15]="unary_operator";static char _tmp448[19]="postfix_expression";
static char _tmp449[19]="primary_expression";static char _tmp44A[25]="argument_expression_list";
static char _tmp44B[26]="argument_expression_list0";static char _tmp44C[9]="constant";
static char _tmp44D[20]="qual_opt_identifier";static char _tmp44E[17]="qual_opt_typedef";
static char _tmp44F[18]="struct_union_name";static char _tmp450[11]="field_name";
static char _tmp451[12]="right_angle";static struct _dyneither_ptr Cyc_yytname[289]={{
_tmp331,_tmp331,_tmp331 + 2},{_tmp332,_tmp332,_tmp332 + 6},{_tmp333,_tmp333,
_tmp333 + 12},{_tmp334,_tmp334,_tmp334 + 5},{_tmp335,_tmp335,_tmp335 + 9},{_tmp336,
_tmp336,_tmp336 + 7},{_tmp337,_tmp337,_tmp337 + 7},{_tmp338,_tmp338,_tmp338 + 8},{
_tmp339,_tmp339,_tmp339 + 5},{_tmp33A,_tmp33A,_tmp33A + 5},{_tmp33B,_tmp33B,
_tmp33B + 6},{_tmp33C,_tmp33C,_tmp33C + 4},{_tmp33D,_tmp33D,_tmp33D + 5},{_tmp33E,
_tmp33E,_tmp33E + 6},{_tmp33F,_tmp33F,_tmp33F + 7},{_tmp340,_tmp340,_tmp340 + 7},{
_tmp341,_tmp341,_tmp341 + 9},{_tmp342,_tmp342,_tmp342 + 6},{_tmp343,_tmp343,
_tmp343 + 9},{_tmp344,_tmp344,_tmp344 + 9},{_tmp345,_tmp345,_tmp345 + 7},{_tmp346,
_tmp346,_tmp346 + 6},{_tmp347,_tmp347,_tmp347 + 5},{_tmp348,_tmp348,_tmp348 + 8},{
_tmp349,_tmp349,_tmp349 + 7},{_tmp34A,_tmp34A,_tmp34A + 7},{_tmp34B,_tmp34B,
_tmp34B + 9},{_tmp34C,_tmp34C,_tmp34C + 3},{_tmp34D,_tmp34D,_tmp34D + 5},{_tmp34E,
_tmp34E,_tmp34E + 7},{_tmp34F,_tmp34F,_tmp34F + 6},{_tmp350,_tmp350,_tmp350 + 3},{
_tmp351,_tmp351,_tmp351 + 4},{_tmp352,_tmp352,_tmp352 + 5},{_tmp353,_tmp353,
_tmp353 + 9},{_tmp354,_tmp354,_tmp354 + 6},{_tmp355,_tmp355,_tmp355 + 7},{_tmp356,
_tmp356,_tmp356 + 5},{_tmp357,_tmp357,_tmp357 + 8},{_tmp358,_tmp358,_tmp358 + 4},{
_tmp359,_tmp359,_tmp359 + 6},{_tmp35A,_tmp35A,_tmp35A + 4},{_tmp35B,_tmp35B,
_tmp35B + 6},{_tmp35C,_tmp35C,_tmp35C + 7},{_tmp35D,_tmp35D,_tmp35D + 4},{_tmp35E,
_tmp35E,_tmp35E + 9},{_tmp35F,_tmp35F,_tmp35F + 9},{_tmp360,_tmp360,_tmp360 + 6},{
_tmp361,_tmp361,_tmp361 + 10},{_tmp362,_tmp362,_tmp362 + 9},{_tmp363,_tmp363,
_tmp363 + 8},{_tmp364,_tmp364,_tmp364 + 7},{_tmp365,_tmp365,_tmp365 + 7},{_tmp366,
_tmp366,_tmp366 + 8},{_tmp367,_tmp367,_tmp367 + 7},{_tmp368,_tmp368,_tmp368 + 8},{
_tmp369,_tmp369,_tmp369 + 5},{_tmp36A,_tmp36A,_tmp36A + 9},{_tmp36B,_tmp36B,
_tmp36B + 6},{_tmp36C,_tmp36C,_tmp36C + 7},{_tmp36D,_tmp36D,_tmp36D + 5},{_tmp36E,
_tmp36E,_tmp36E + 8},{_tmp36F,_tmp36F,_tmp36F + 13},{_tmp370,_tmp370,_tmp370 + 16},{
_tmp371,_tmp371,_tmp371 + 14},{_tmp372,_tmp372,_tmp372 + 12},{_tmp373,_tmp373,
_tmp373 + 7},{_tmp374,_tmp374,_tmp374 + 8},{_tmp375,_tmp375,_tmp375 + 12},{_tmp376,
_tmp376,_tmp376 + 6},{_tmp377,_tmp377,_tmp377 + 8},{_tmp378,_tmp378,_tmp378 + 8},{
_tmp379,_tmp379,_tmp379 + 10},{_tmp37A,_tmp37A,_tmp37A + 9},{_tmp37B,_tmp37B,
_tmp37B + 13},{_tmp37C,_tmp37C,_tmp37C + 10},{_tmp37D,_tmp37D,_tmp37D + 9},{_tmp37E,
_tmp37E,_tmp37E + 13},{_tmp37F,_tmp37F,_tmp37F + 14},{_tmp380,_tmp380,_tmp380 + 12},{
_tmp381,_tmp381,_tmp381 + 16},{_tmp382,_tmp382,_tmp382 + 15},{_tmp383,_tmp383,
_tmp383 + 7},{_tmp384,_tmp384,_tmp384 + 7},{_tmp385,_tmp385,_tmp385 + 7},{_tmp386,
_tmp386,_tmp386 + 8},{_tmp387,_tmp387,_tmp387 + 9},{_tmp388,_tmp388,_tmp388 + 6},{
_tmp389,_tmp389,_tmp389 + 6},{_tmp38A,_tmp38A,_tmp38A + 6},{_tmp38B,_tmp38B,
_tmp38B + 6},{_tmp38C,_tmp38C,_tmp38C + 7},{_tmp38D,_tmp38D,_tmp38D + 6},{_tmp38E,
_tmp38E,_tmp38E + 11},{_tmp38F,_tmp38F,_tmp38F + 11},{_tmp390,_tmp390,_tmp390 + 11},{
_tmp391,_tmp391,_tmp391 + 11},{_tmp392,_tmp392,_tmp392 + 11},{_tmp393,_tmp393,
_tmp393 + 12},{_tmp394,_tmp394,_tmp394 + 13},{_tmp395,_tmp395,_tmp395 + 11},{
_tmp396,_tmp396,_tmp396 + 11},{_tmp397,_tmp397,_tmp397 + 10},{_tmp398,_tmp398,
_tmp398 + 9},{_tmp399,_tmp399,_tmp399 + 11},{_tmp39A,_tmp39A,_tmp39A + 12},{_tmp39B,
_tmp39B,_tmp39B + 11},{_tmp39C,_tmp39C,_tmp39C + 17},{_tmp39D,_tmp39D,_tmp39D + 7},{
_tmp39E,_tmp39E,_tmp39E + 8},{_tmp39F,_tmp39F,_tmp39F + 19},{_tmp3A0,_tmp3A0,
_tmp3A0 + 20},{_tmp3A1,_tmp3A1,_tmp3A1 + 18},{_tmp3A2,_tmp3A2,_tmp3A2 + 9},{_tmp3A3,
_tmp3A3,_tmp3A3 + 13},{_tmp3A4,_tmp3A4,_tmp3A4 + 16},{_tmp3A5,_tmp3A5,_tmp3A5 + 18},{
_tmp3A6,_tmp3A6,_tmp3A6 + 10},{_tmp3A7,_tmp3A7,_tmp3A7 + 4},{_tmp3A8,_tmp3A8,
_tmp3A8 + 4},{_tmp3A9,_tmp3A9,_tmp3A9 + 4},{_tmp3AA,_tmp3AA,_tmp3AA + 4},{_tmp3AB,
_tmp3AB,_tmp3AB + 4},{_tmp3AC,_tmp3AC,_tmp3AC + 4},{_tmp3AD,_tmp3AD,_tmp3AD + 4},{
_tmp3AE,_tmp3AE,_tmp3AE + 4},{_tmp3AF,_tmp3AF,_tmp3AF + 4},{_tmp3B0,_tmp3B0,
_tmp3B0 + 4},{_tmp3B1,_tmp3B1,_tmp3B1 + 4},{_tmp3B2,_tmp3B2,_tmp3B2 + 4},{_tmp3B3,
_tmp3B3,_tmp3B3 + 4},{_tmp3B4,_tmp3B4,_tmp3B4 + 4},{_tmp3B5,_tmp3B5,_tmp3B5 + 4},{
_tmp3B6,_tmp3B6,_tmp3B6 + 4},{_tmp3B7,_tmp3B7,_tmp3B7 + 4},{_tmp3B8,_tmp3B8,
_tmp3B8 + 4},{_tmp3B9,_tmp3B9,_tmp3B9 + 4},{_tmp3BA,_tmp3BA,_tmp3BA + 4},{_tmp3BB,
_tmp3BB,_tmp3BB + 4},{_tmp3BC,_tmp3BC,_tmp3BC + 4},{_tmp3BD,_tmp3BD,_tmp3BD + 4},{
_tmp3BE,_tmp3BE,_tmp3BE + 4},{_tmp3BF,_tmp3BF,_tmp3BF + 4},{_tmp3C0,_tmp3C0,
_tmp3C0 + 4},{_tmp3C1,_tmp3C1,_tmp3C1 + 4},{_tmp3C2,_tmp3C2,_tmp3C2 + 4},{_tmp3C3,
_tmp3C3,_tmp3C3 + 5},{_tmp3C4,_tmp3C4,_tmp3C4 + 17},{_tmp3C5,_tmp3C5,_tmp3C5 + 12},{
_tmp3C6,_tmp3C6,_tmp3C6 + 19},{_tmp3C7,_tmp3C7,_tmp3C7 + 21},{_tmp3C8,_tmp3C8,
_tmp3C8 + 20},{_tmp3C9,_tmp3C9,_tmp3C9 + 21},{_tmp3CA,_tmp3CA,_tmp3CA + 13},{
_tmp3CB,_tmp3CB,_tmp3CB + 15},{_tmp3CC,_tmp3CC,_tmp3CC + 17},{_tmp3CD,_tmp3CD,
_tmp3CD + 19},{_tmp3CE,_tmp3CE,_tmp3CE + 12},{_tmp3CF,_tmp3CF,_tmp3CF + 19},{
_tmp3D0,_tmp3D0,_tmp3D0 + 17},{_tmp3D1,_tmp3D1,_tmp3D1 + 23},{_tmp3D2,_tmp3D2,
_tmp3D2 + 24},{_tmp3D3,_tmp3D3,_tmp3D3 + 15},{_tmp3D4,_tmp3D4,_tmp3D4 + 11},{
_tmp3D5,_tmp3D5,_tmp3D5 + 15},{_tmp3D6,_tmp3D6,_tmp3D6 + 10},{_tmp3D7,_tmp3D7,
_tmp3D7 + 15},{_tmp3D8,_tmp3D8,_tmp3D8 + 25},{_tmp3D9,_tmp3D9,_tmp3D9 + 5},{_tmp3DA,
_tmp3DA,_tmp3DA + 15},{_tmp3DB,_tmp3DB,_tmp3DB + 15},{_tmp3DC,_tmp3DC,_tmp3DC + 11},{
_tmp3DD,_tmp3DD,_tmp3DD + 22},{_tmp3DE,_tmp3DE,_tmp3DE + 26},{_tmp3DF,_tmp3DF,
_tmp3DF + 16},{_tmp3E0,_tmp3E0,_tmp3E0 + 16},{_tmp3E1,_tmp3E1,_tmp3E1 + 24},{
_tmp3E2,_tmp3E2,_tmp3E2 + 25},{_tmp3E3,_tmp3E3,_tmp3E3 + 21},{_tmp3E4,_tmp3E4,
_tmp3E4 + 22},{_tmp3E5,_tmp3E5,_tmp3E5 + 16},{_tmp3E6,_tmp3E6,_tmp3E6 + 19},{
_tmp3E7,_tmp3E7,_tmp3E7 + 25},{_tmp3E8,_tmp3E8,_tmp3E8 + 35},{_tmp3E9,_tmp3E9,
_tmp3E9 + 23},{_tmp3EA,_tmp3EA,_tmp3EA + 24},{_tmp3EB,_tmp3EB,_tmp3EB + 18},{
_tmp3EC,_tmp3EC,_tmp3EC + 19},{_tmp3ED,_tmp3ED,_tmp3ED + 14},{_tmp3EE,_tmp3EE,
_tmp3EE + 19},{_tmp3EF,_tmp3EF,_tmp3EF + 20},{_tmp3F0,_tmp3F0,_tmp3F0 + 14},{
_tmp3F1,_tmp3F1,_tmp3F1 + 11},{_tmp3F2,_tmp3F2,_tmp3F2 + 23},{_tmp3F3,_tmp3F3,
_tmp3F3 + 18},{_tmp3F4,_tmp3F4,_tmp3F4 + 30},{_tmp3F5,_tmp3F5,_tmp3F5 + 8},{_tmp3F6,
_tmp3F6,_tmp3F6 + 12},{_tmp3F7,_tmp3F7,_tmp3F7 + 14},{_tmp3F8,_tmp3F8,_tmp3F8 + 13},{
_tmp3F9,_tmp3F9,_tmp3F9 + 23},{_tmp3FA,_tmp3FA,_tmp3FA + 14},{_tmp3FB,_tmp3FB,
_tmp3FB + 18},{_tmp3FC,_tmp3FC,_tmp3FC + 8},{_tmp3FD,_tmp3FD,_tmp3FD + 11},{_tmp3FE,
_tmp3FE,_tmp3FE + 20},{_tmp3FF,_tmp3FF,_tmp3FF + 9},{_tmp400,_tmp400,_tmp400 + 16},{
_tmp401,_tmp401,_tmp401 + 19},{_tmp402,_tmp402,_tmp402 + 10},{_tmp403,_tmp403,
_tmp403 + 16},{_tmp404,_tmp404,_tmp404 + 11},{_tmp405,_tmp405,_tmp405 + 14},{
_tmp406,_tmp406,_tmp406 + 11},{_tmp407,_tmp407,_tmp407 + 15},{_tmp408,_tmp408,
_tmp408 + 22},{_tmp409,_tmp409,_tmp409 + 16},{_tmp40A,_tmp40A,_tmp40A + 17},{
_tmp40B,_tmp40B,_tmp40B + 12},{_tmp40C,_tmp40C,_tmp40C + 18},{_tmp40D,_tmp40D,
_tmp40D + 17},{_tmp40E,_tmp40E,_tmp40E + 12},{_tmp40F,_tmp40F,_tmp40F + 16},{
_tmp410,_tmp410,_tmp410 + 11},{_tmp411,_tmp411,_tmp411 + 10},{_tmp412,_tmp412,
_tmp412 + 14},{_tmp413,_tmp413,_tmp413 + 15},{_tmp414,_tmp414,_tmp414 + 20},{
_tmp415,_tmp415,_tmp415 + 27},{_tmp416,_tmp416,_tmp416 + 10},{_tmp417,_tmp417,
_tmp417 + 13},{_tmp418,_tmp418,_tmp418 + 18},{_tmp419,_tmp419,_tmp419 + 21},{
_tmp41A,_tmp41A,_tmp41A + 19},{_tmp41B,_tmp41B,_tmp41B + 16},{_tmp41C,_tmp41C,
_tmp41C + 20},{_tmp41D,_tmp41D,_tmp41D + 15},{_tmp41E,_tmp41E,_tmp41E + 20},{
_tmp41F,_tmp41F,_tmp41F + 15},{_tmp420,_tmp420,_tmp420 + 12},{_tmp421,_tmp421,
_tmp421 + 20},{_tmp422,_tmp422,_tmp422 + 19},{_tmp423,_tmp423,_tmp423 + 20},{
_tmp424,_tmp424,_tmp424 + 21},{_tmp425,_tmp425,_tmp425 + 21},{_tmp426,_tmp426,
_tmp426 + 12},{_tmp427,_tmp427,_tmp427 + 17},{_tmp428,_tmp428,_tmp428 + 19},{
_tmp429,_tmp429,_tmp429 + 14},{_tmp42A,_tmp42A,_tmp42A + 17},{_tmp42B,_tmp42B,
_tmp42B + 23},{_tmp42C,_tmp42C,_tmp42C + 13},{_tmp42D,_tmp42D,_tmp42D + 14},{
_tmp42E,_tmp42E,_tmp42E + 16},{_tmp42F,_tmp42F,_tmp42F + 16},{_tmp430,_tmp430,
_tmp430 + 8},{_tmp431,_tmp431,_tmp431 + 19},{_tmp432,_tmp432,_tmp432 + 20},{_tmp433,
_tmp433,_tmp433 + 14},{_tmp434,_tmp434,_tmp434 + 19},{_tmp435,_tmp435,_tmp435 + 20},{
_tmp436,_tmp436,_tmp436 + 11},{_tmp437,_tmp437,_tmp437 + 22},{_tmp438,_tmp438,
_tmp438 + 20},{_tmp439,_tmp439,_tmp439 + 23},{_tmp43A,_tmp43A,_tmp43A + 20},{
_tmp43B,_tmp43B,_tmp43B + 22},{_tmp43C,_tmp43C,_tmp43C + 23},{_tmp43D,_tmp43D,
_tmp43D + 24},{_tmp43E,_tmp43E,_tmp43E + 24},{_tmp43F,_tmp43F,_tmp43F + 15},{
_tmp440,_tmp440,_tmp440 + 20},{_tmp441,_tmp441,_tmp441 + 22},{_tmp442,_tmp442,
_tmp442 + 17},{_tmp443,_tmp443,_tmp443 + 20},{_tmp444,_tmp444,_tmp444 + 26},{
_tmp445,_tmp445,_tmp445 + 16},{_tmp446,_tmp446,_tmp446 + 17},{_tmp447,_tmp447,
_tmp447 + 15},{_tmp448,_tmp448,_tmp448 + 19},{_tmp449,_tmp449,_tmp449 + 19},{
_tmp44A,_tmp44A,_tmp44A + 25},{_tmp44B,_tmp44B,_tmp44B + 26},{_tmp44C,_tmp44C,
_tmp44C + 9},{_tmp44D,_tmp44D,_tmp44D + 20},{_tmp44E,_tmp44E,_tmp44E + 17},{_tmp44F,
_tmp44F,_tmp44F + 18},{_tmp450,_tmp450,_tmp450 + 11},{_tmp451,_tmp451,_tmp451 + 12}};
static short Cyc_yyr1[515]={0,146,147,147,147,147,147,147,147,147,147,147,148,148,
149,149,149,150,150,150,151,151,151,151,152,152,153,154,155,156,157,157,157,157,
157,157,157,158,158,159,159,160,160,160,160,160,160,160,160,160,160,161,161,161,
161,161,161,161,162,162,163,164,164,165,165,165,165,165,165,166,166,167,167,167,
167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,167,
168,169,169,169,170,170,170,171,171,172,172,172,173,173,173,173,173,174,174,175,
175,176,176,177,177,178,179,179,180,180,181,182,182,182,182,182,182,183,183,183,
183,183,183,184,185,185,186,186,186,186,187,187,187,188,188,189,189,189,189,190,
190,190,191,191,192,192,193,193,194,194,194,194,194,194,194,194,194,194,195,195,
195,195,195,195,195,195,195,195,195,196,196,197,198,198,199,199,199,199,199,199,
199,199,200,200,200,201,201,202,202,202,203,203,203,204,204,205,205,205,205,206,
206,207,207,208,208,209,209,210,210,211,211,212,212,212,212,213,213,214,214,215,
215,215,216,217,217,218,218,219,219,219,219,220,220,220,220,221,222,222,223,223,
224,224,225,225,225,225,225,226,226,227,227,227,228,228,228,228,228,228,228,228,
228,228,228,229,229,229,229,229,229,229,230,230,231,232,232,233,233,234,234,234,
234,234,234,235,235,235,235,235,235,236,236,236,236,236,236,237,237,237,237,237,
237,237,237,237,237,237,237,237,237,238,238,238,238,238,238,238,238,239,240,240,
241,241,242,242,243,243,244,244,245,245,246,246,246,247,247,247,247,247,248,248,
248,249,249,249,250,250,250,250,251,251,252,252,252,252,252,252,252,253,254,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,256,256,256,257,257,258,
258,259,259,259,260,260,261,261,262,262,262,263,263,263,263,263,263,263,263,263,
263,263,264,264,264,264,264,264,264,265,266,266,267,267,268,268,269,269,270,270,
271,271,271,272,272,272,272,272,273,273,273,274,274,274,275,275,275,275,276,276,
277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,
278,278,278,279,279,279,279,279,279,279,279,279,279,280,280,280,280,280,280,280,
280,280,280,281,282,282,283,283,283,283,283,284,284,285,285,286,286,287,287,288,
288};static short Cyc_yyr2[515]={0,1,2,3,5,3,5,6,7,3,3,0,4,3,1,2,3,1,1,1,2,3,3,4,3,
4,2,1,2,1,2,3,5,3,7,5,8,0,1,1,2,1,2,1,2,1,2,1,2,1,2,1,1,1,1,2,1,1,0,1,6,1,3,1,1,
4,4,4,8,1,2,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,4,4,1,4,6,4,1,4,1,1,1,1,5,2,4,1,3,1,2,
3,4,9,8,4,3,0,3,1,1,0,1,1,2,1,1,3,1,3,3,1,2,1,2,1,2,1,2,1,2,1,2,1,1,3,1,2,0,3,6,
3,5,1,2,1,2,3,3,0,1,1,2,5,1,2,1,2,1,3,4,4,5,4,5,4,4,2,1,1,3,4,4,5,4,5,4,4,2,1,2,
5,0,2,4,4,1,1,1,1,1,1,2,2,1,0,3,0,1,1,0,1,1,0,2,3,5,5,7,1,3,0,2,0,2,3,5,0,1,1,3,
2,3,4,1,1,3,1,3,2,1,2,1,1,3,1,1,2,3,4,8,1,2,3,4,2,1,2,3,2,1,2,1,2,3,4,3,1,3,1,1,
2,3,3,4,4,5,4,3,5,4,4,2,1,1,1,1,1,1,5,0,5,3,1,2,2,3,1,2,1,2,1,2,5,7,7,5,8,6,0,3,
4,5,6,7,5,7,6,7,7,8,7,8,8,9,6,7,7,8,3,2,2,2,3,2,4,5,1,1,5,1,3,1,3,1,3,1,3,1,3,1,
3,3,1,3,3,3,3,1,3,3,1,3,3,1,3,3,3,1,4,1,2,2,4,2,6,6,1,1,1,3,1,1,3,4,4,5,4,2,2,2,
4,4,4,1,3,1,1,3,1,2,1,3,1,1,3,1,3,1,3,3,1,1,1,1,1,1,1,1,1,1,1,1,5,2,2,2,5,5,1,1,
3,1,3,1,3,1,3,1,3,1,3,3,1,3,3,3,3,1,3,3,1,3,3,1,3,3,3,1,4,1,2,2,2,2,2,2,4,2,6,6,
4,6,9,11,4,6,6,4,1,1,1,1,1,4,3,4,3,3,2,2,6,7,1,1,1,1,3,2,5,4,5,5,1,1,3,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1};static short Cyc_yydefact[1030]={0,19,51,52,53,54,56,71,74,75,
76,77,78,79,80,81,95,96,97,113,114,47,0,0,57,0,0,147,88,92,0,0,0,0,0,0,0,38,505,
213,507,506,508,0,0,72,0,199,199,198,1,0,17,0,0,18,0,0,41,49,43,69,45,82,83,0,84,
0,37,158,0,183,186,85,162,111,55,54,48,0,99,504,0,505,500,501,502,503,111,0,373,
0,0,0,0,236,0,375,376,26,28,0,0,0,0,0,0,0,0,148,0,0,0,0,0,0,0,196,197,2,0,0,0,0,
0,30,0,119,120,122,42,50,44,46,115,509,510,111,111,37,39,37,0,20,0,215,0,171,159,
184,193,192,0,0,190,191,194,195,204,186,0,70,0,55,103,0,101,0,505,383,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,489,490,473,0,0,0,0,0,476,474,475,0,400,402,416,424,426,
428,430,432,434,437,442,445,448,452,0,454,477,488,487,0,384,382,33,0,0,111,0,0,0,
129,125,127,254,256,0,0,9,10,0,0,0,111,511,512,214,94,0,0,163,73,234,0,231,0,3,0,
5,0,284,0,31,0,0,37,21,0,116,117,142,110,0,145,0,0,0,0,0,0,0,0,0,0,0,0,505,287,
289,0,295,291,0,293,277,278,279,0,280,281,282,0,40,22,122,261,0,221,237,0,0,217,
215,0,201,0,0,0,206,58,205,187,0,0,104,100,0,0,377,0,0,397,0,0,0,0,250,393,398,0,
395,0,462,0,418,452,0,419,420,0,0,0,0,0,0,0,0,0,455,456,37,0,0,0,458,459,457,0,
374,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,406,407,408,409,410,411,412,413,414,
415,405,0,460,0,483,484,0,0,0,492,0,111,390,391,0,388,0,238,0,0,0,0,257,229,0,
130,135,131,133,126,128,215,0,263,255,264,514,513,0,87,91,0,89,0,93,109,64,63,0,
61,164,215,233,160,263,235,172,173,0,86,200,27,0,29,0,0,0,0,121,123,240,239,23,
106,118,0,0,0,137,138,140,0,111,111,153,0,0,0,0,0,0,37,0,324,325,326,0,0,328,0,0,
0,0,296,292,122,294,290,288,0,170,222,0,0,0,228,216,223,167,0,0,0,217,169,203,
202,165,201,0,0,207,59,112,0,105,423,102,98,386,387,253,0,394,249,251,381,0,0,0,
0,241,245,0,0,0,0,0,0,0,0,0,0,0,0,0,491,498,0,497,401,425,0,427,429,431,433,435,
436,440,441,438,439,443,444,446,447,449,450,451,404,403,482,479,0,481,0,0,0,378,
0,385,32,0,379,0,0,258,136,132,134,0,217,0,201,0,265,0,215,0,276,260,0,0,111,0,0,
0,129,0,111,0,215,0,182,161,232,4,6,0,35,0,0,141,124,142,0,217,146,155,154,0,0,
149,0,0,0,303,0,0,0,0,0,323,327,0,0,0,0,286,37,24,262,215,0,225,0,0,218,0,168,
221,209,166,189,188,207,185,0,7,0,252,396,399,461,0,0,242,0,246,465,0,0,0,0,469,
472,0,0,0,0,0,453,494,0,0,480,478,0,0,389,392,380,259,230,272,0,266,267,201,0,0,
217,201,0,90,0,217,0,0,0,60,62,0,174,0,0,217,0,201,0,0,0,139,143,115,144,156,153,
153,0,0,0,0,0,0,0,0,0,0,0,0,0,303,329,0,0,25,217,0,226,224,0,217,0,208,0,8,0,0,0,
243,247,0,0,0,0,421,422,482,481,496,0,499,417,493,495,0,271,269,275,274,0,268,
201,0,115,0,67,65,66,175,181,178,0,180,176,201,0,34,0,0,152,151,297,303,0,0,0,0,
0,0,331,332,334,336,338,340,342,344,347,352,355,358,362,364,371,372,0,37,300,309,
0,0,0,0,0,0,0,0,0,0,330,283,211,227,219,210,215,13,0,14,464,463,0,248,466,0,0,
471,470,485,0,273,270,36,0,0,179,177,285,108,0,0,0,303,0,368,0,0,365,37,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,366,304,0,311,0,0,0,319,0,0,0,0,0,0,302,0,217,12,
15,0,0,0,0,486,107,0,157,298,299,0,0,0,0,303,305,335,0,337,339,341,343,345,346,
350,351,348,349,353,354,356,357,359,360,361,0,310,312,313,0,321,320,0,315,0,0,0,
220,212,16,0,0,0,0,301,367,0,363,306,0,37,314,322,316,317,0,244,467,0,68,0,0,333,
303,307,318,0,370,369,308,468,0,0,0};static short Cyc_yydefgoto[143]={1027,50,685,
875,51,52,287,53,458,54,460,55,56,141,57,58,526,228,444,445,229,61,243,230,63,
164,165,64,161,65,264,265,126,127,128,266,231,425,473,474,475,66,67,646,647,648,
68,476,69,449,70,71,158,159,72,117,522,316,682,606,73,607,516,673,508,512,513,
420,309,251,94,95,547,466,548,331,332,333,232,302,303,608,431,290,462,291,292,
293,294,295,760,296,297,836,837,838,839,840,841,842,843,844,845,846,847,848,849,
850,851,334,410,411,335,336,337,298,199,397,200,532,201,202,203,204,205,206,207,
208,209,210,211,212,213,214,215,563,564,216,217,75,876,244,435};static short Cyc_yypact[
1030]={2748,- -32768,- -32768,- -32768,- -32768,116,- -32768,- -32768,- -32768,- -32768,
- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,
- -32768,3328,206,3101,- -32768,32,- 49,- -32768,- 5,136,145,165,162,189,191,340,279,
- -32768,- -32768,243,- -32768,- -32768,- -32768,250,487,285,286,312,312,- -32768,-
-32768,2611,- -32768,454,536,- -32768,295,732,3328,3328,3328,- -32768,3328,- -32768,
- -32768,252,- -32768,32,3247,381,37,684,651,- -32768,- -32768,333,349,359,- -32768,
32,371,- -32768,4365,228,- -32768,- -32768,- -32768,- -32768,333,5752,- -32768,367,313,
4365,383,389,425,- -32768,72,- -32768,- -32768,3474,3474,2611,2611,3474,429,5752,
337,- -32768,79,434,565,459,79,3693,5752,- -32768,- -32768,- -32768,2611,2884,2611,
2884,73,- -32768,469,474,- -32768,3165,- -32768,- -32768,- -32768,- -32768,3693,-
-32768,- -32768,333,139,1623,- -32768,3247,732,- -32768,3474,3401,4122,- -32768,381,
- -32768,- -32768,- -32768,484,486,- -32768,- -32768,- -32768,- -32768,- 28,651,3474,-
-32768,2884,- -32768,496,510,504,32,99,- -32768,4365,- 15,958,5957,551,5752,5801,
592,598,608,632,659,661,663,681,6006,6006,- -32768,- -32768,- -32768,2190,685,6055,
6055,6055,- -32768,- -32768,- -32768,299,- -32768,- -32768,- 6,641,545,682,631,606,130,
658,234,123,- -32768,1171,6055,193,- 24,- -32768,712,6196,735,- -32768,- -32768,740,
5752,333,6196,727,220,3693,1096,3693,605,- -32768,- 20,- 20,- -32768,- -32768,50,744,
323,333,- -32768,- -32768,- -32768,- -32768,71,730,- -32768,- -32768,647,373,- -32768,
753,- -32768,777,- -32768,784,768,797,- -32768,565,4220,3247,- -32768,791,3693,-
-32768,709,801,32,806,803,75,805,1324,816,811,809,824,4318,1324,218,820,818,-
-32768,- -32768,823,1766,1766,732,1766,- -32768,- -32768,- -32768,829,- -32768,-
-32768,- -32768,121,- -32768,- -32768,832,815,74,848,- -32768,11,836,826,284,837,701,
828,3474,5752,- -32768,842,- -32768,- -32768,74,852,32,- -32768,5752,854,- -32768,833,
851,- -32768,79,5752,4365,260,- -32768,- -32768,- -32768,856,857,2190,- -32768,3693,-
-32768,- -32768,3730,- -32768,886,5752,5752,5752,5752,5752,5752,3693,859,2190,-
-32768,- -32768,1909,853,377,5752,- -32768,- -32768,- -32768,5752,- -32768,6055,5752,
6055,6055,6055,6055,6055,6055,6055,6055,6055,6055,6055,6055,6055,6055,6055,6055,
6055,5752,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,-
-32768,- -32768,- -32768,5752,- -32768,79,- -32768,- -32768,4413,79,5752,- -32768,868,
333,- -32768,- -32768,867,861,4365,- -32768,311,958,871,3474,- -32768,873,860,-
-32768,1096,1096,1096,- -32768,- -32768,2958,4508,157,- -32768,500,- -32768,- -32768,
11,- -32768,- -32768,3474,- -32768,894,- -32768,881,- -32768,876,877,883,- -32768,1173,
- -32768,535,654,- -32768,- -32768,- -32768,3693,- -32768,- -32768,- -32768,2611,-
-32768,2611,897,895,887,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,
773,5752,898,891,- -32768,888,122,333,333,831,5752,5752,890,900,5752,991,2052,904,
- -32768,- -32768,- -32768,390,982,- -32768,4606,5752,1324,2328,- -32768,- -32768,3165,
- -32768,- -32768,- -32768,3474,- -32768,- -32768,3693,899,267,- -32768,- -32768,889,-
-32768,11,903,3620,826,- -32768,- -32768,- -32768,- -32768,701,34,905,863,- -32768,-
-32768,2474,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,901,- -32768,
- -32768,- -32768,- -32768,2994,906,909,929,- -32768,- -32768,728,4220,910,914,917,
919,393,915,920,562,921,923,5879,- -32768,- -32768,924,928,- -32768,641,272,545,682,
631,606,130,130,658,658,658,658,234,234,123,123,- -32768,- -32768,- -32768,- -32768,
- -32768,- -32768,- -32768,926,- -32768,- 26,3474,4024,- -32768,6235,- -32768,- -32768,
933,- -32768,98,943,- -32768,- -32768,- -32768,- -32768,930,826,931,701,927,500,3474,
3547,4701,- -32768,- -32768,- 20,936,333,770,940,71,3031,944,333,3474,3401,4799,-
-32768,535,- -32768,- -32768,- -32768,946,- -32768,968,893,- -32768,- -32768,709,5752,
826,- -32768,- -32768,- -32768,954,32,506,399,419,5752,841,436,950,4897,4992,520,-
-32768,- -32768,957,960,953,478,- -32768,3247,- -32768,815,962,3474,- -32768,967,11,
- -32768,966,- -32768,175,- -32768,- -32768,- -32768,- -32768,863,- -32768,963,- -32768,
2611,- -32768,- -32768,- -32768,973,208,970,- -32768,3828,- -32768,- -32768,5752,1070,
5752,5801,- -32768,- -32768,79,79,973,969,4024,- -32768,- -32768,5752,5752,- -32768,-
-32768,74,750,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,972,- -32768,-
-32768,701,74,974,826,701,985,- -32768,5752,826,516,975,976,- -32768,- -32768,992,-
-32768,74,993,826,995,701,990,5752,1006,- -32768,- -32768,3693,- -32768,1000,48,831,
1324,1007,1001,6160,999,1009,1324,5752,5087,547,5182,557,5277,841,- -32768,1012,
1015,- -32768,826,112,- -32768,- -32768,1022,826,3693,- -32768,249,- -32768,1010,1011,
5752,- -32768,- -32768,4220,1013,1016,1017,- -32768,886,1014,1019,- -32768,762,-
-32768,- -32768,- -32768,- -32768,4024,- -32768,- -32768,- -32768,- -32768,1020,- -32768,
701,588,3693,1036,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,1021,- -32768,-
-32768,701,527,- -32768,1028,3693,- -32768,- -32768,1122,841,1032,6104,1029,2328,
6055,1026,- -32768,- 3,- -32768,1066,1023,756,825,134,834,268,347,- -32768,- -32768,-
-32768,- -32768,1067,6055,1909,- -32768,- -32768,531,1324,541,5372,1324,575,5467,
5562,589,1038,- -32768,- -32768,- -32768,- -32768,1040,- -32768,962,- -32768,1039,656,
- -32768,- -32768,324,- -32768,- -32768,3693,1138,- -32768,- -32768,- -32768,3926,-
-32768,- -32768,- -32768,1044,1045,- -32768,- -32768,- -32768,- -32768,620,1324,1048,
841,2190,- -32768,3693,1043,- -32768,1480,6055,5752,6055,6055,6055,6055,6055,6055,
6055,6055,6055,6055,6055,6055,6055,6055,6055,6055,6055,5752,- -32768,- -32768,1047,
- -32768,1324,1324,645,- -32768,1324,1324,695,1324,706,5657,- -32768,11,826,- -32768,
- -32768,337,5752,1050,1046,- -32768,- -32768,1064,- -32768,- -32768,- -32768,1053,
1051,1057,6055,841,- -32768,641,334,545,682,682,606,130,130,658,658,658,658,234,
234,123,123,- -32768,- -32768,- -32768,356,- -32768,- -32768,- -32768,1324,- -32768,-
-32768,1324,- -32768,1324,1324,707,- -32768,- -32768,- -32768,802,1071,3693,1072,-
-32768,973,293,- -32768,- -32768,5752,1480,- -32768,- -32768,- -32768,- -32768,1324,-
-32768,- -32768,1073,- -32768,1075,1076,- -32768,841,- -32768,- -32768,1077,- -32768,-
-32768,- -32768,- -32768,1197,1206,- -32768};static short Cyc_yypgoto[143]={- -32768,
176,- -32768,261,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- 46,-
-32768,- 84,16,- -32768,- -32768,0,586,- -32768,91,- 177,1097,5,- -32768,- -32768,- 90,-
-32768,194,1177,- 695,- -32768,- -32768,- -32768,956,952,697,480,- -32768,- -32768,578,
- -32768,- -32768,187,- -32768,- -32768,283,- 162,1149,- 371,69,- -32768,1061,- -32768,-
-32768,1175,- 87,- -32768,540,- 99,- 133,- 143,- 334,287,550,556,- 418,- 469,- 114,- 317,-
95,- -32768,- 258,- 149,- 532,- 319,- -32768,902,- 184,- 94,- 45,20,- 240,55,- -32768,-
-32768,- -32768,- 59,- 255,- -32768,- 650,- -32768,- -32768,- -32768,- -32768,- -32768,-
-32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,- -32768,-
-32768,- -32768,- -32768,8,1008,- -32768,690,821,- -32768,- 8,587,- -32768,- 165,- 296,-
140,- 327,- 348,- 346,865,- 341,- 344,- 362,- 336,- 332,- 175,422,479,896,- -32768,- 331,-
-32768,179,415,- 61,27,- 289,- 51};static short Cyc_yytable[6376]={59,250,308,465,136,
62,358,233,234,143,341,237,574,575,576,577,616,361,362,363,568,59,140,569,549,
317,62,344,572,573,571,96,499,500,537,502,345,78,398,566,536,671,578,579,112,262,
307,136,580,581,310,59,423,644,645,825,62,100,59,59,59,714,59,62,62,62,432,62,59,
147,263,589,509,62,130,131,132,324,133,630,405,198,300,140,142,39,366,448,442,
907,169,60,137,288,419,299,364,674,326,239,315,220,559,59,59,433,630,712,62,62,
587,406,60,327,590,319,891,434,866,101,59,59,59,59,39,62,62,62,62,59,367,510,719,
908,62,240,432,631,38,59,149,59,60,38,62,142,62,41,147,60,60,60,41,60,543,289,
544,142,531,60,432,679,59,44,662,531,518,62,556,- 150,558,434,437,511,797,433,638,
443,325,257,899,38,359,436,677,241,438,434,453,611,41,668,224,242,60,60,505,258,
225,433,631,482,97,468,483,170,453,582,583,584,611,60,60,60,60,414,299,374,375,
524,60,915,916,171,76,718,409,119,38,422,60,530,60,409,424,434,40,41,42,870,504,
288,288,364,288,423,423,423,471,434,956,430,506,60,376,377,961,382,917,918,102,
97,59,160,103,383,384,62,528,451,269,492,97,721,549,399,400,401,142,235,236,507,
172,427,104,624,105,59,59,428,59,694,62,62,306,62,253,254,255,256,429,511,- 215,
289,289,- 215,289,531,750,624,1004,288,38,106,241,783,527,107,450,402,757,41,242,
600,403,404,79,113,109,486,359,268,270,39,170,493,477,494,320,538,129,418,554,
617,495,749,359,- 237,110,97,- 237,97,171,60,124,38,1020,59,38,567,19,20,62,40,41,
42,40,41,42,1025,874,380,134,289,788,381,111,419,60,60,39,60,511,539,250,707,625,
549,670,453,114,329,330,807,364,246,591,97,811,241,1016,784,710,306,97,921,517,
242,819,922,453,667,115,794,795,74,453,665,415,219,596,364,422,422,422,301,365,
424,424,424,597,615,116,364,441,787,678,80,98,869,99,656,666,38,872,364,364,623,
60,629,440,40,41,42,947,140,364,160,59,74,59,738,873,62,1005,62,74,163,419,162,
727,637,74,649,650,738,531,653,364,657,135,923,138,788,742,74,1006,59,663,924,
925,167,62,218,166,454,429,98,43,364,455,59,221,142,561,144,62,145,98,659,97,222,
364,146,726,364,450,142,74,74,699,364,723,135,674,773,755,74,741,59,880,681,743,
778,62,74,74,74,74,511,477,364,238,301,787,799,756,713,223,60,792,60,664,970,971,
972,973,74,364,793,245,964,625,761,965,966,730,725,788,968,969,967,501,120,121,
774,453,74,60,453,962,740,166,974,975,98,247,98,259,976,977,97,60,38,97,339,260,
342,342,928,364,593,41,716,43,771,772,355,356,994,313,615,314,44,342,342,342,43,
321,299,60,47,48,49,612,753,613,323,754,787,629,322,614,98,632,342,633,804,812,
767,98,808,364,813,702,400,401,764,766,364,904,960,43,364,895,122,123,821,929,
626,905,627,800,364,452,59,860,628,931,364,62,38,642,643,805,74,863,340,927,364,
41,142,452,369,479,59,681,484,402,817,62,44,477,703,404,372,373,364,948,47,48,49,
935,252,74,684,477,890,940,477,364,364,897,1017,150,151,152,957,346,958,136,97,
889,810,347,153,154,155,156,157,943,427,368,312,348,894,166,428,823,47,48,49,454,
378,379,342,98,953,978,979,980,1019,342,38,857,859,60,862,349,865,38,40,41,42,
520,521,852,364,40,41,42,371,985,447,97,945,60,879,946,428,447,47,48,49,1003,350,
428,351,342,352,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,342,
342,353,288,511,829,360,249,732,1014,38,856,364,47,48,49,370,988,40,41,42,198,98,
364,364,98,267,407,990,1011,471,644,645,38,472,1018,412,249,47,48,49,413,41,467,
692,693,125,933,417,59,937,939,446,44,62,288,782,452,758,759,452,47,48,49,439,
289,801,802,74,456,74,733,734,735,38,16,17,18,886,887,136,452,40,41,42,43,461,
452,359,342,911,912,81,457,471,963,525,603,604,605,459,59,47,48,49,463,62,469,
930,913,914,934,488,981,919,920,478,289,1012,364,421,480,426,489,481,467,485,992,
550,551,552,553,97,555,996,827,828,487,490,74,60,496,562,497,498,503,565,434,954,
507,261,515,98,534,43,288,523,267,514,519,38,84,187,188,85,86,87,585,529,41,533,
535,541,366,542,560,602,342,595,586,354,983,984,191,562,986,987,592,989,594,601,
81,60,599,38,618,619,620,634,621,622,59,40,41,42,98,62,636,640,635,610,651,639,
641,471,652,654,289,658,660,669,672,47,48,49,675,690,680,689,686,691,696,695,452,
697,1007,698,700,1008,705,1009,1010,701,704,82,709,708,452,711,717,452,39,720,
722,731,724,328,752,342,168,84,1021,736,85,86,87,739,746,41,747,751,762,768,88,
769,770,306,562,781,89,562,90,91,775,329,330,777,92,706,785,790,796,60,93,803,74,
806,814,815,7,8,9,10,11,12,13,14,15,16,17,18,19,20,809,816,818,342,820,822,249,
824,826,830,831,854,855,867,342,22,868,871,467,877,878,883,881,884,882,892,249,
27,885,888,893,896,898,249,900,28,29,903,906,909,926,941,944,910,942,949,32,951,
982,952,34,955,959,999,998,98,1000,35,36,997,1001,1002,467,7,8,9,10,11,12,13,14,
15,16,17,18,19,20,249,135,1028,1013,1015,1022,729,1023,1024,1026,249,1029,995,
737,39,22,248,108,43,249,745,464,470,748,148,318,780,27,118,45,46,779,385,776,
993,28,29,688,416,540,570,598,853,562,0,0,32,0,0,0,34,0,0,0,557,0,0,35,36,902,0,
0,342,0,0,0,0,0,0,386,387,388,389,390,391,392,393,394,395,0,342,304,0,0,38,467,0,
0,789,0,791,39,40,41,42,43,0,306,467,396,0,798,0,0,447,0,45,46,0,0,428,0,47,48,
49,249,0,0,0,0,0,0,0,0,0,421,0,0,0,249,0,0,0,0,342,0,342,342,342,342,342,342,342,
342,342,342,342,342,342,342,342,342,342,0,173,174,271,0,272,273,274,275,276,277,
278,279,135,81,0,175,280,0,0,176,0,281,0,0,0,0,467,177,178,179,180,0,342,0,0,181,
0,282,0,0,467,0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,342,0,0,283,84,187,188,85,86,87,0,0,41,0,0,189,284,139,0,0,267,
0,0,190,0,0,191,0,0,0,0,192,0,0,193,0,0,194,195,0,0,196,197,0,0,0,0,467,0,249,0,
0,0,0,0,0,2,3,4,77,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,758,759,21,173,174,
271,267,272,273,274,275,276,277,278,279,22,81,23,175,280,0,249,176,24,281,0,0,27,
0,0,177,178,179,180,0,28,29,0,181,0,282,0,0,0,0,0,32,33,182,183,34,184,0,0,0,0,0,
35,36,37,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,283,84,187,188,85,
86,87,39,40,41,42,43,189,284,139,- 303,0,0,0,0,190,0,45,286,0,0,0,0,192,0,0,193,0,
0,194,195,0,0,196,197,2,3,4,77,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,21,
173,174,271,0,272,273,274,275,276,277,278,279,22,81,23,175,280,0,0,176,24,281,0,
0,27,0,0,177,178,179,180,0,28,29,0,181,0,282,0,0,0,0,0,32,33,182,183,34,184,0,0,
0,0,0,35,36,37,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,283,84,187,
188,85,86,87,39,40,41,42,43,189,284,139,285,0,0,0,0,190,0,45,286,0,0,0,0,192,0,0,
193,0,0,194,195,0,0,196,197,2,3,4,77,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,
0,21,173,174,271,0,272,273,274,275,276,277,278,279,22,81,23,175,280,0,0,176,24,
281,0,0,27,0,0,177,178,179,180,0,28,29,- 37,181,0,282,0,0,0,0,0,32,33,182,183,34,
184,0,0,0,0,0,35,36,37,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,283,
84,187,188,85,86,87,39,40,41,42,43,189,284,139,0,0,0,0,0,190,0,45,286,0,0,0,0,
192,0,0,193,0,0,194,195,0,0,196,197,2,3,4,77,6,7,8,9,10,11,12,13,14,15,16,17,18,
19,20,0,0,21,173,174,271,0,272,273,274,275,276,277,278,279,22,81,23,175,280,0,0,
176,24,281,0,0,27,0,0,177,178,179,180,0,28,29,0,181,0,282,0,0,0,0,0,32,33,182,
183,34,184,0,0,0,0,0,35,36,37,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,283,84,187,188,85,86,87,39,40,41,42,43,189,284,139,0,0,0,0,0,190,0,45,286,0,0,
0,0,192,0,0,193,0,0,194,195,0,0,196,197,2,3,4,77,6,7,8,9,10,11,12,13,14,15,16,17,
18,19,20,0,0,21,173,174,0,0,0,0,0,0,0,0,0,0,22,81,23,175,0,0,0,176,24,0,0,0,27,0,
0,177,178,179,180,0,28,29,0,181,0,0,0,0,0,0,0,32,33,182,183,34,184,0,0,0,0,0,35,
36,37,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,
39,40,41,42,43,189,655,0,0,0,0,0,0,190,0,45,286,0,0,0,0,192,0,0,193,0,0,194,195,
0,0,196,197,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,0,173,174,0,0,0,0,0,0,0,0,
0,0,22,81,0,175,0,0,0,176,0,0,0,0,27,0,0,177,178,179,180,0,28,29,0,181,0,0,0,0,0,
0,0,32,0,182,183,34,184,0,0,0,0,0,35,36,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,38,84,187,188,85,86,87,39,40,41,42,43,189,0,357,0,0,0,0,0,190,0,45,
286,0,0,0,0,192,0,0,193,0,0,194,195,0,0,196,197,7,8,9,10,11,12,13,14,15,16,17,18,
19,20,0,0,0,173,174,0,0,0,0,0,0,0,0,0,0,22,81,0,175,0,0,0,176,0,0,0,0,27,0,0,177,
178,179,180,0,28,29,0,181,0,0,0,0,0,0,0,32,0,182,183,34,184,0,0,0,0,0,35,36,0,0,
185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,39,40,
41,42,43,189,0,0,0,0,0,0,0,190,0,45,286,0,0,0,0,192,0,0,193,0,0,194,195,0,0,196,
197,- 11,1,0,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,21,0,0,0,0,0,0,
0,0,0,0,0,0,22,0,23,0,0,0,683,0,24,0,25,26,27,0,0,0,0,0,0,0,28,29,- 37,0,0,0,0,0,
0,30,31,32,33,0,0,34,0,0,0,0,0,0,35,36,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,38,0,0,0,0,0,0,39,40,41,42,43,0,0,0,- 11,0,0,0,0,44,0,45,46,0,0,0,0,47,
48,49,- 11,1,0,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,21,0,0,0,0,0,
0,0,0,0,0,0,0,22,0,23,0,0,0,0,0,24,0,25,26,27,0,0,0,0,0,0,0,28,29,- 37,0,0,0,0,0,
0,30,31,32,33,0,0,34,0,0,0,0,0,0,35,36,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,38,0,0,0,0,0,0,39,40,41,42,43,0,0,0,- 11,0,0,0,0,44,0,45,46,0,0,0,0,47,
48,49,- 11,1,0,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,21,0,0,0,0,0,
0,0,0,0,0,0,0,22,0,23,0,0,0,0,0,24,0,25,26,27,0,0,0,0,0,0,0,28,29,- 37,0,0,0,0,0,
0,30,31,32,33,0,0,34,0,0,0,0,0,0,35,36,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,38,0,0,0,0,0,0,39,40,41,42,43,0,0,0,0,0,0,0,0,44,0,45,46,0,0,0,0,47,48,
49,1,0,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,21,0,0,0,0,0,0,0,0,0,
0,0,0,22,0,23,0,0,0,0,0,24,0,25,26,27,0,0,0,0,0,0,0,28,29,- 37,0,0,0,0,0,0,30,31,
32,33,0,0,34,0,0,0,0,0,0,35,36,37,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,0,0,
0,0,0,0,0,0,38,0,0,0,0,22,0,39,40,41,42,43,0,0,0,- 11,0,27,0,0,44,0,45,46,0,28,29,
0,47,48,49,0,0,0,0,0,32,0,0,0,34,0,81,0,0,0,0,35,36,7,8,9,10,11,12,13,14,15,16,
17,18,19,20,0,0,0,0,0,0,0,0,304,0,0,0,0,0,0,22,0,0,39,40,0,42,43,0,306,0,0,27,0,
0,0,427,82,45,46,28,29,428,0,47,48,49,0,0,687,0,32,168,84,0,34,85,86,87,0,0,41,
35,36,0,0,88,0,0,0,0,0,89,0,90,91,0,329,330,0,92,0,0,0,0,0,93,0,0,38,0,81,0,0,0,
0,39,40,41,42,43,0,0,0,0,0,0,0,0,471,0,45,46,0,0,0,0,47,48,49,2,3,4,77,6,7,8,9,
10,11,12,13,14,15,16,17,18,19,20,0,0,21,0,0,82,0,0,0,0,0,0,0,0,0,22,0,23,0,0,83,
84,0,24,85,86,87,27,0,41,0,0,0,0,88,28,29,- 37,0,0,89,0,90,91,0,0,32,33,92,0,34,0,
0,0,93,0,0,35,36,37,0,0,0,2,3,4,77,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,
21,0,0,0,0,0,0,39,40,0,42,43,0,22,139,23,0,261,0,0,0,24,45,46,0,27,0,0,0,0,0,0,0,
28,29,0,0,0,0,0,0,0,0,0,32,33,0,0,34,0,0,0,0,0,0,35,36,37,0,0,2,3,4,77,6,7,8,9,
10,11,12,13,14,15,16,17,18,19,20,0,0,21,0,0,0,0,0,0,0,39,40,0,42,43,22,0,139,0,0,
0,0,0,24,0,45,46,27,0,0,0,0,0,0,0,28,29,0,0,0,0,0,0,0,0,0,32,0,0,0,34,0,0,0,0,0,
0,35,36,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,
0,0,39,40,0,42,43,0,0,0,0,27,0,0,0,0,0,45,46,28,29,0,0,0,0,0,0,0,0,0,32,0,0,0,34,
0,0,0,0,0,0,35,36,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,0,0,0,0,0,0,304,0,0,
305,0,0,0,22,0,0,39,40,0,42,43,0,306,0,0,27,0,0,0,0,0,45,46,28,29,0,0,226,0,0,0,
0,0,0,32,0,0,0,34,0,0,0,0,0,0,35,36,7,8,9,10,11,12,13,14,15,16,17,18,19,20,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,22,0,0,39,40,0,42,43,0,0,227,0,27,0,0,0,0,0,45,46,28,29,
0,0,0,0,0,0,0,0,0,32,0,0,0,34,0,0,0,0,0,0,35,36,7,8,9,10,11,12,13,14,15,16,17,18,
19,20,0,0,0,0,0,0,0,0,304,0,0,0,0,0,0,22,0,0,39,40,0,42,43,0,306,0,0,27,0,0,0,0,
0,45,46,28,29,0,0,0,0,0,0,0,0,0,32,0,0,0,34,0,0,0,0,0,0,35,36,7,8,9,10,11,12,13,
14,15,16,17,18,19,20,0,0,0,0,0,0,0,0,676,0,0,0,0,0,0,22,0,0,39,40,0,42,43,0,0,0,
0,27,0,0,0,0,0,45,46,28,29,0,0,0,173,174,0,0,0,0,32,545,0,0,34,0,0,81,0,175,0,35,
36,176,0,0,0,0,0,0,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,
0,0,39,40,0,42,43,0,0,185,186,0,0,0,0,0,0,45,46,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,
187,188,85,86,87,0,0,41,0,0,189,0,343,546,0,173,174,0,190,0,0,191,0,329,330,0,
192,0,81,193,175,0,194,195,176,0,196,197,0,0,0,0,177,178,179,180,0,0,0,0,181,0,0,
0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,0,189,0,343,786,0,173,174,0,190,
0,0,191,0,329,330,0,192,0,81,193,175,0,194,195,176,0,196,197,0,0,0,0,177,178,179,
180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,0,189,0,343,
950,0,173,174,0,190,0,0,191,0,329,330,0,192,0,81,193,175,0,194,195,176,0,196,197,
0,0,0,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,0,
0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,
0,41,0,0,189,0,343,0,0,173,174,0,190,0,0,191,0,329,330,0,192,0,81,193,175,0,194,
195,176,0,196,197,0,0,0,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,
0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,
187,188,85,86,87,0,0,41,0,0,189,0,0,0,0,173,174,0,190,0,0,191,0,0,0,311,192,0,81,
193,175,0,194,195,176,0,196,197,0,0,0,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,
0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,0,189,0,343,0,0,173,174,0,190,0,0,191,0,
0,0,0,192,0,81,193,175,0,194,195,176,0,196,197,0,0,0,0,177,178,179,180,0,0,0,0,
181,0,0,0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,81,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,0,189,491,173,174,0,0,0,
0,190,0,0,191,0,0,0,81,192,175,0,193,82,176,194,195,0,0,196,197,0,177,178,179,
180,0,0,168,84,181,0,85,86,87,0,0,41,0,0,182,183,88,184,0,0,0,0,89,0,90,91,0,185,
186,0,92,0,0,0,0,0,93,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,
0,189,0,173,174,0,0,0,0,190,588,0,191,0,0,0,81,192,175,0,193,0,176,194,195,0,0,
196,197,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,
0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,
0,0,41,0,0,189,0,0,0,0,173,174,0,190,0,0,191,0,0,0,609,192,0,81,193,175,0,194,
195,176,0,196,197,0,0,0,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,
0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,
187,188,85,86,87,0,0,41,0,0,189,0,173,174,0,0,0,0,190,661,0,191,0,0,0,81,192,175,
0,193,0,176,194,195,0,0,196,197,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,
182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,38,84,187,188,85,86,87,0,0,41,0,0,189,0,0,0,0,173,174,0,190,0,0,191,0,0,0,728,
192,0,81,193,175,0,194,195,176,0,196,197,0,0,0,0,177,178,179,180,0,0,0,0,181,0,0,
0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,0,189,0,0,0,0,173,174,0,190,0,0,
191,0,0,0,744,192,0,81,193,175,0,194,195,176,0,196,197,0,0,0,0,177,178,179,180,0,
0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,0,189,763,173,174,
0,0,0,0,190,0,0,191,0,0,0,81,192,175,0,193,0,176,194,195,0,0,196,197,0,177,178,
179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,0,189,
765,173,174,0,0,0,0,190,0,0,191,0,0,0,81,192,175,0,193,0,176,194,195,0,0,196,197,
0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,
185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,
0,0,189,0,173,174,0,0,0,0,190,858,0,191,0,0,0,81,192,175,0,193,0,176,194,195,0,0,
196,197,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,
0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,
0,0,41,0,0,189,0,173,174,0,0,0,0,190,861,0,191,0,0,0,81,192,175,0,193,0,176,194,
195,0,0,196,197,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,0,
0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,187,188,
85,86,87,0,0,41,0,0,189,864,173,174,0,0,0,0,190,0,0,191,0,0,0,81,192,175,0,193,0,
176,194,195,0,0,196,197,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,182,183,
0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,84,
187,188,85,86,87,0,0,41,0,0,189,0,173,174,0,0,0,0,190,932,0,191,0,0,0,81,192,175,
0,193,0,176,194,195,0,0,196,197,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,0,0,0,0,
182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,38,84,187,188,85,86,87,0,0,41,0,0,189,0,173,174,0,0,0,0,190,936,0,191,0,0,0,81,
192,175,0,193,0,176,194,195,0,0,196,197,0,177,178,179,180,0,0,0,0,181,0,0,0,0,0,
0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,0,189,0,173,174,0,0,0,0,190,938,0,191,
0,0,0,81,192,175,0,193,0,176,194,195,0,0,196,197,0,177,178,179,180,0,0,0,0,181,0,
0,0,0,0,0,0,0,0,182,183,0,184,0,0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,38,84,187,188,85,86,87,0,0,41,0,0,189,0,173,174,0,0,0,0,190,
991,0,191,0,0,0,81,192,175,0,193,0,176,194,195,0,0,196,197,0,177,178,179,180,0,0,
0,0,181,0,0,0,0,0,0,0,0,0,182,183,0,184,173,174,0,0,0,0,0,0,0,185,186,0,0,81,0,0,
0,0,0,0,0,0,0,0,0,0,0,177,178,179,180,0,38,84,187,188,85,86,87,0,0,41,0,0,189,
182,183,0,184,0,0,0,190,0,0,191,0,0,185,186,192,0,0,193,0,0,194,195,0,0,196,197,
0,0,0,0,0,0,173,174,0,38,84,187,188,85,86,87,0,0,41,81,0,189,0,343,0,0,0,0,0,190,
0,0,191,177,178,179,180,192,0,0,193,0,0,194,195,0,0,196,197,0,0,182,183,0,184,0,
0,0,0,0,0,0,0,0,185,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,173,174,0,38,84,187,
188,85,86,87,0,0,41,81,0,189,0,706,0,0,0,0,0,190,0,0,191,177,178,179,180,192,0,0,
193,0,0,194,195,0,0,196,197,0,0,182,183,0,184,173,174,0,0,0,0,0,0,0,185,186,0,0,
81,0,0,0,0,0,0,0,0,0,0,0,0,0,177,178,179,180,0,38,84,187,188,85,86,87,0,0,41,0,0,
189,182,183,0,184,173,174,0,338,0,0,191,0,0,185,186,192,0,81,193,0,0,194,195,0,0,
196,197,0,0,0,0,177,178,179,180,0,38,84,187,188,85,86,87,0,0,41,0,0,189,182,183,
0,184,173,174,0,354,0,0,191,0,0,185,186,192,0,81,193,0,0,194,195,0,0,196,197,0,0,
0,0,177,178,179,180,0,38,84,187,188,85,86,87,0,0,41,0,0,189,182,183,0,184,0,0,0,
190,0,0,191,832,833,185,186,192,0,0,193,0,0,194,195,0,81,196,197,0,0,0,0,0,0,0,0,
0,38,84,187,188,85,86,87,0,0,41,0,0,189,0,0,0,0,0,0,0,901,0,0,191,81,0,0,0,192,0,
0,193,0,0,194,195,0,0,196,197,0,82,0,0,0,0,0,0,0,0,0,0,0,0,0,0,168,84,0,0,85,86,
87,81,0,41,0,0,0,0,88,0,0,0,0,0,834,82,90,91,0,0,0,0,92,0,0,835,0,408,93,195,168,
84,196,197,85,86,87,0,0,41,0,0,0,0,88,0,0,0,0,0,89,0,90,91,82,0,0,0,92,0,0,0,0,0,
93,0,715,0,0,168,84,0,0,85,86,87,0,0,41,0,0,0,0,88,0,0,0,0,0,89,0,90,91,0,0,0,0,
92,0,0,0,0,0,93};static short Cyc_yycheck[6376]={0,115,145,261,65,0,190,101,102,68,
175,105,374,375,376,377,434,192,193,194,368,21,68,369,343,158,21,176,372,373,371,
23,287,288,330,290,176,21,213,366,329,510,378,379,44,129,145,108,380,381,145,51,
229,5,6,750,51,106,58,59,60,593,62,58,59,60,86,62,68,69,129,402,61,68,58,59,60,
167,62,450,104,89,141,129,68,113,92,249,17,92,82,0,65,139,227,141,122,515,113,
107,128,93,357,103,104,125,477,133,103,104,399,135,21,128,403,160,811,137,768,
124,120,121,122,123,113,120,121,122,123,129,136,120,601,136,129,108,86,454,106,
139,71,141,51,106,139,129,141,115,148,58,59,60,115,62,338,139,340,141,323,68,86,
127,162,126,495,330,309,162,352,121,354,137,122,306,706,125,472,106,170,106,830,
106,190,234,518,106,237,137,249,429,115,508,120,114,103,104,122,124,126,125,517,
126,23,262,129,106,267,382,383,384,450,120,121,122,123,223,262,87,88,313,129,87,
88,124,108,127,218,51,106,229,139,321,141,225,229,137,114,115,116,127,119,287,
288,122,290,422,423,424,126,137,900,231,303,162,124,125,906,134,124,125,124,82,
262,124,119,142,143,262,319,249,131,279,93,607,593,82,83,84,262,103,104,106,88,
126,119,447,124,287,288,132,290,549,287,288,119,290,120,121,122,123,231,434,127,
287,288,130,290,472,642,471,960,357,106,124,106,107,316,126,249,126,651,115,114,
417,131,132,120,44,49,274,338,137,138,113,106,280,267,119,162,331,57,121,350,437,
126,641,354,119,105,170,122,172,124,262,59,106,1006,357,106,367,20,21,357,114,
115,116,114,115,116,1019,121,137,120,357,693,141,126,510,287,288,113,290,515,123,
498,560,447,706,121,450,105,131,132,727,122,112,404,218,732,106,107,690,130,119,
225,137,122,114,742,141,471,505,126,702,703,0,477,501,224,106,412,122,422,423,
424,142,127,422,423,424,119,431,120,122,240,693,523,22,23,773,25,487,501,106,778,
122,122,447,357,449,127,114,115,116,130,501,122,124,458,44,460,623,779,458,130,
460,51,108,601,120,613,471,57,481,482,637,641,485,122,487,65,134,67,802,627,70,
130,487,496,142,143,120,487,126,79,122,427,82,117,122,127,501,119,487,127,124,
501,126,93,119,331,122,122,132,613,122,447,501,103,104,127,122,609,108,942,668,
127,112,627,529,788,526,627,676,529,120,121,122,123,672,471,122,113,260,802,710,
127,592,123,458,699,460,497,915,916,917,918,142,122,699,126,909,623,127,910,911,
617,612,887,913,914,912,289,119,120,669,637,162,487,640,907,626,167,919,920,170,
127,172,119,921,922,412,501,106,415,173,122,175,176,854,122,407,115,595,117,127,
665,185,186,943,126,611,126,126,192,193,194,117,122,665,529,134,135,136,124,119,
126,123,122,887,630,121,132,218,458,213,460,724,122,119,225,728,122,127,82,83,84,
655,656,122,834,906,117,122,127,119,120,744,127,124,835,126,713,122,249,665,119,
132,127,122,665,106,478,479,725,260,119,126,853,122,115,665,267,138,269,685,681,
272,126,740,685,126,623,131,132,89,90,122,882,134,135,136,127,116,289,529,637,
119,119,640,122,122,826,1002,63,64,65,901,126,903,781,542,809,731,126,74,75,76,
77,78,873,126,91,146,126,822,321,132,746,134,135,136,122,85,86,323,331,127,923,
924,925,1006,330,106,762,763,665,765,126,767,106,114,115,116,63,64,758,122,114,
115,116,140,127,126,595,119,685,785,122,132,126,134,135,136,959,126,132,126,366,
126,368,369,370,371,372,373,374,375,376,377,378,379,380,381,382,383,384,126,854,
942,755,126,115,619,998,106,761,122,134,135,136,139,127,114,115,116,834,412,122,
122,415,134,120,127,127,126,5,6,106,130,1005,106,145,134,135,136,106,115,261,121,
122,119,860,126,854,863,864,127,126,854,906,685,447,22,23,450,134,135,136,125,
854,121,122,458,121,460,106,107,108,106,17,18,19,121,122,946,471,114,115,116,117,
123,477,901,472,139,140,38,121,126,908,314,422,423,424,121,906,134,135,136,113,
906,121,858,89,90,861,106,926,85,86,120,906,121,122,228,120,230,119,126,343,126,
940,346,347,348,349,758,351,947,753,754,126,119,529,854,126,360,130,126,121,364,
137,898,106,123,130,542,125,117,1006,133,265,127,127,106,107,108,109,110,111,112,
385,121,115,121,125,121,92,122,127,121,560,122,397,126,931,932,129,402,935,936,
124,938,127,122,38,906,127,106,106,120,126,106,127,122,1006,114,115,116,595,1006,
125,122,119,428,126,119,130,126,120,30,1006,119,42,126,137,134,135,136,127,122,
127,127,133,106,122,127,623,122,985,122,127,988,121,990,991,127,127,91,122,127,
637,127,121,640,113,127,127,123,133,103,647,641,106,107,1011,127,110,111,112,127,
126,115,106,121,126,120,120,119,127,119,495,120,126,498,128,129,121,131,132,125,
134,120,124,25,127,1006,140,127,685,127,127,127,8,9,10,11,12,13,14,15,16,17,18,
19,20,21,133,127,127,699,127,133,427,119,126,120,127,130,121,119,710,37,119,113,
549,127,127,122,127,127,126,107,447,49,127,127,127,121,28,454,120,57,58,126,130,
91,91,121,121,138,122,25,68,121,119,122,72,121,127,107,126,758,121,79,80,127,127,
122,593,8,9,10,11,12,13,14,15,16,17,18,19,20,21,498,781,0,127,127,127,614,127,
127,127,508,0,946,622,113,37,114,35,117,517,628,260,265,640,70,159,681,49,48,128,
129,676,56,672,942,57,58,542,225,332,370,415,758,651,- 1,- 1,68,- 1,- 1,- 1,72,- 1,- 1,
- 1,353,- 1,- 1,79,80,832,- 1,- 1,835,- 1,- 1,- 1,- 1,- 1,- 1,93,94,95,96,97,98,99,100,101,
102,- 1,853,103,- 1,- 1,106,693,- 1,- 1,696,- 1,698,113,114,115,116,117,- 1,119,706,123,
- 1,709,- 1,- 1,126,- 1,128,129,- 1,- 1,132,- 1,134,135,136,613,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,623,- 1,- 1,- 1,627,- 1,- 1,- 1,- 1,907,- 1,909,910,911,912,913,914,915,916,917,918,
919,920,921,922,923,924,925,- 1,25,26,27,- 1,29,30,31,32,33,34,35,36,946,38,- 1,40,
41,- 1,- 1,44,- 1,46,- 1,- 1,- 1,- 1,788,52,53,54,55,- 1,959,- 1,- 1,60,- 1,62,- 1,- 1,802,- 1,
- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,1005,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,-
1,118,119,120,- 1,- 1,750,- 1,- 1,126,- 1,- 1,129,- 1,- 1,- 1,- 1,134,- 1,- 1,137,- 1,- 1,140,
141,- 1,- 1,144,145,- 1,- 1,- 1,- 1,887,- 1,779,- 1,- 1,- 1,- 1,- 1,- 1,3,4,5,6,7,8,9,10,11,
12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,811,29,30,31,32,33,34,35,36,37,
38,39,40,41,- 1,826,44,45,46,- 1,- 1,49,- 1,- 1,52,53,54,55,- 1,57,58,- 1,60,- 1,62,- 1,-
1,- 1,- 1,- 1,68,69,70,71,72,73,- 1,- 1,- 1,- 1,- 1,79,80,81,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,113,114,
115,116,117,118,119,120,121,- 1,- 1,- 1,- 1,126,- 1,128,129,- 1,- 1,- 1,- 1,134,- 1,- 1,137,
- 1,- 1,140,141,- 1,- 1,144,145,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,- 1,
- 1,24,25,26,27,- 1,29,30,31,32,33,34,35,36,37,38,39,40,41,- 1,- 1,44,45,46,- 1,- 1,49,
- 1,- 1,52,53,54,55,- 1,57,58,- 1,60,- 1,62,- 1,- 1,- 1,- 1,- 1,68,69,70,71,72,73,- 1,- 1,- 1,
- 1,- 1,79,80,81,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,- 1,- 1,- 1,-
1,126,- 1,128,129,- 1,- 1,- 1,- 1,134,- 1,- 1,137,- 1,- 1,140,141,- 1,- 1,144,145,3,4,5,6,7,
8,9,10,11,12,13,14,15,16,17,18,19,20,21,- 1,- 1,24,25,26,27,- 1,29,30,31,32,33,34,
35,36,37,38,39,40,41,- 1,- 1,44,45,46,- 1,- 1,49,- 1,- 1,52,53,54,55,- 1,57,58,59,60,- 1,
62,- 1,- 1,- 1,- 1,- 1,68,69,70,71,72,73,- 1,- 1,- 1,- 1,- 1,79,80,81,- 1,83,84,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,
113,114,115,116,117,118,119,120,- 1,- 1,- 1,- 1,- 1,126,- 1,128,129,- 1,- 1,- 1,- 1,134,- 1,
- 1,137,- 1,- 1,140,141,- 1,- 1,144,145,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
20,21,- 1,- 1,24,25,26,27,- 1,29,30,31,32,33,34,35,36,37,38,39,40,41,- 1,- 1,44,45,46,
- 1,- 1,49,- 1,- 1,52,53,54,55,- 1,57,58,- 1,60,- 1,62,- 1,- 1,- 1,- 1,- 1,68,69,70,71,72,73,
- 1,- 1,- 1,- 1,- 1,79,80,81,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,- 1,- 1,
- 1,- 1,- 1,126,- 1,128,129,- 1,- 1,- 1,- 1,134,- 1,- 1,137,- 1,- 1,140,141,- 1,- 1,144,145,3,
4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,- 1,- 1,24,25,26,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,37,38,39,40,- 1,- 1,- 1,44,45,- 1,- 1,- 1,49,- 1,- 1,52,53,54,55,- 1,57,58,- 1,
60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,68,69,70,71,72,73,- 1,- 1,- 1,- 1,- 1,79,80,81,- 1,83,84,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,
112,113,114,115,116,117,118,119,- 1,- 1,- 1,- 1,- 1,- 1,126,- 1,128,129,- 1,- 1,- 1,- 1,134,
- 1,- 1,137,- 1,- 1,140,141,- 1,- 1,144,145,8,9,10,11,12,13,14,15,16,17,18,19,20,21,- 1,
- 1,- 1,25,26,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,37,38,- 1,40,- 1,- 1,- 1,44,- 1,- 1,- 1,- 1,49,
- 1,- 1,52,53,54,55,- 1,57,58,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,68,- 1,70,71,72,73,- 1,- 1,- 1,
- 1,- 1,79,80,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,106,107,108,109,110,111,112,113,114,115,116,117,118,- 1,120,- 1,- 1,- 1,- 1,- 1,
126,- 1,128,129,- 1,- 1,- 1,- 1,134,- 1,- 1,137,- 1,- 1,140,141,- 1,- 1,144,145,8,9,10,11,
12,13,14,15,16,17,18,19,20,21,- 1,- 1,- 1,25,26,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,37,38,
- 1,40,- 1,- 1,- 1,44,- 1,- 1,- 1,- 1,49,- 1,- 1,52,53,54,55,- 1,57,58,- 1,60,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,68,- 1,70,71,72,73,- 1,- 1,- 1,- 1,- 1,79,80,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,113,114,115,
116,117,118,- 1,- 1,- 1,- 1,- 1,- 1,- 1,126,- 1,128,129,- 1,- 1,- 1,- 1,134,- 1,- 1,137,- 1,- 1,
140,141,- 1,- 1,144,145,0,1,- 1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,-
1,- 1,24,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,37,- 1,39,- 1,- 1,- 1,43,- 1,45,- 1,47,48,
49,- 1,- 1,- 1,- 1,- 1,- 1,- 1,57,58,59,- 1,- 1,- 1,- 1,- 1,- 1,66,67,68,69,- 1,- 1,72,- 1,- 1,- 1,
- 1,- 1,- 1,79,80,81,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,106,- 1,- 1,- 1,- 1,- 1,- 1,113,114,115,116,117,- 1,- 1,- 1,121,- 1,- 1,- 1,- 1,126,
- 1,128,129,- 1,- 1,- 1,- 1,134,135,136,0,1,- 1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,
18,19,20,21,- 1,- 1,24,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,37,- 1,39,- 1,- 1,- 1,- 1,- 1,
45,- 1,47,48,49,- 1,- 1,- 1,- 1,- 1,- 1,- 1,57,58,59,- 1,- 1,- 1,- 1,- 1,- 1,66,67,68,69,- 1,- 1,
72,- 1,- 1,- 1,- 1,- 1,- 1,79,80,81,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,- 1,- 1,- 1,- 1,- 1,- 1,113,114,115,116,117,- 1,- 1,- 1,121,- 1,-
1,- 1,- 1,126,- 1,128,129,- 1,- 1,- 1,- 1,134,135,136,0,1,- 1,3,4,5,6,7,8,9,10,11,12,13,
14,15,16,17,18,19,20,21,- 1,- 1,24,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,37,- 1,39,- 1,
- 1,- 1,- 1,- 1,45,- 1,47,48,49,- 1,- 1,- 1,- 1,- 1,- 1,- 1,57,58,59,- 1,- 1,- 1,- 1,- 1,- 1,66,67,
68,69,- 1,- 1,72,- 1,- 1,- 1,- 1,- 1,- 1,79,80,81,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,- 1,- 1,- 1,- 1,- 1,- 1,113,114,115,116,117,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,126,- 1,128,129,- 1,- 1,- 1,- 1,134,135,136,1,- 1,3,4,5,6,7,8,9,10,
11,12,13,14,15,16,17,18,19,20,21,- 1,- 1,24,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,37,
- 1,39,- 1,- 1,- 1,- 1,- 1,45,- 1,47,48,49,- 1,- 1,- 1,- 1,- 1,- 1,- 1,57,58,59,- 1,- 1,- 1,- 1,- 1,
- 1,66,67,68,69,- 1,- 1,72,- 1,- 1,- 1,- 1,- 1,- 1,79,80,81,8,9,10,11,12,13,14,15,16,17,
18,19,20,21,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,- 1,- 1,- 1,- 1,37,- 1,113,114,115,116,
117,- 1,- 1,- 1,121,- 1,49,- 1,- 1,126,- 1,128,129,- 1,57,58,- 1,134,135,136,- 1,- 1,- 1,- 1,
- 1,68,- 1,- 1,- 1,72,- 1,38,- 1,- 1,- 1,- 1,79,80,8,9,10,11,12,13,14,15,16,17,18,19,20,
21,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,103,- 1,- 1,- 1,- 1,- 1,- 1,37,- 1,- 1,113,114,- 1,116,117,- 1,
119,- 1,- 1,49,- 1,- 1,- 1,126,91,128,129,57,58,132,- 1,134,135,136,- 1,- 1,103,- 1,68,
106,107,- 1,72,110,111,112,- 1,- 1,115,79,80,- 1,- 1,120,- 1,- 1,- 1,- 1,- 1,126,- 1,128,
129,- 1,131,132,- 1,134,- 1,- 1,- 1,- 1,- 1,140,- 1,- 1,106,- 1,38,- 1,- 1,- 1,- 1,113,114,115,
116,117,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,126,- 1,128,129,- 1,- 1,- 1,- 1,134,135,136,3,4,5,6,7,
8,9,10,11,12,13,14,15,16,17,18,19,20,21,- 1,- 1,24,- 1,- 1,91,- 1,- 1,- 1,- 1,- 1,- 1,- 1,-
1,- 1,37,- 1,39,- 1,- 1,106,107,- 1,45,110,111,112,49,- 1,115,- 1,- 1,- 1,- 1,120,57,58,59,
- 1,- 1,126,- 1,128,129,- 1,- 1,68,69,134,- 1,72,- 1,- 1,- 1,140,- 1,- 1,79,80,81,- 1,- 1,- 1,
3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,- 1,- 1,24,- 1,- 1,- 1,- 1,- 1,- 1,113,
114,- 1,116,117,- 1,37,120,39,- 1,123,- 1,- 1,- 1,45,128,129,- 1,49,- 1,- 1,- 1,- 1,- 1,- 1,-
1,57,58,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,68,69,- 1,- 1,72,- 1,- 1,- 1,- 1,- 1,- 1,79,80,81,- 1,
- 1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,- 1,- 1,24,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,113,114,- 1,116,117,37,- 1,120,- 1,- 1,- 1,- 1,- 1,45,- 1,128,129,49,- 1,- 1,- 1,- 1,- 1,-
1,- 1,57,58,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,68,- 1,- 1,- 1,72,- 1,- 1,- 1,- 1,- 1,- 1,79,80,8,9,
10,11,12,13,14,15,16,17,18,19,20,21,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
37,- 1,- 1,113,114,- 1,116,117,- 1,- 1,- 1,- 1,49,- 1,- 1,- 1,- 1,- 1,128,129,57,58,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,68,- 1,- 1,- 1,72,- 1,- 1,- 1,- 1,- 1,- 1,79,80,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,103,- 1,- 1,106,- 1,- 1,- 1,37,- 1,- 1,113,
114,- 1,116,117,- 1,119,- 1,- 1,49,- 1,- 1,- 1,- 1,- 1,128,129,57,58,- 1,- 1,61,- 1,- 1,- 1,- 1,
- 1,- 1,68,- 1,- 1,- 1,72,- 1,- 1,- 1,- 1,- 1,- 1,79,80,8,9,10,11,12,13,14,15,16,17,18,19,
20,21,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,37,- 1,- 1,113,114,- 1,116,117,-
1,- 1,120,- 1,49,- 1,- 1,- 1,- 1,- 1,128,129,57,58,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,68,- 1,- 1,
- 1,72,- 1,- 1,- 1,- 1,- 1,- 1,79,80,8,9,10,11,12,13,14,15,16,17,18,19,20,21,- 1,- 1,- 1,-
1,- 1,- 1,- 1,- 1,103,- 1,- 1,- 1,- 1,- 1,- 1,37,- 1,- 1,113,114,- 1,116,117,- 1,119,- 1,- 1,49,
- 1,- 1,- 1,- 1,- 1,128,129,57,58,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,68,- 1,- 1,- 1,72,- 1,- 1,- 1,
- 1,- 1,- 1,79,80,8,9,10,11,12,13,14,15,16,17,18,19,20,21,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
103,- 1,- 1,- 1,- 1,- 1,- 1,37,- 1,- 1,113,114,- 1,116,117,- 1,- 1,- 1,- 1,49,- 1,- 1,- 1,- 1,- 1,
128,129,57,58,- 1,- 1,- 1,25,26,- 1,- 1,- 1,- 1,68,32,- 1,- 1,72,- 1,- 1,38,- 1,40,- 1,79,80,
44,- 1,- 1,- 1,- 1,- 1,- 1,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,
71,- 1,73,- 1,- 1,113,114,- 1,116,117,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,128,129,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,
120,121,- 1,25,26,- 1,126,- 1,- 1,129,- 1,131,132,- 1,134,- 1,38,137,40,- 1,140,141,44,-
1,144,145,- 1,- 1,- 1,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,
71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,120,
121,- 1,25,26,- 1,126,- 1,- 1,129,- 1,131,132,- 1,134,- 1,38,137,40,- 1,140,141,44,- 1,
144,145,- 1,- 1,- 1,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,
- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,120,121,
- 1,25,26,- 1,126,- 1,- 1,129,- 1,131,132,- 1,134,- 1,38,137,40,- 1,140,141,44,- 1,144,
145,- 1,- 1,- 1,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,
73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,120,- 1,- 1,
25,26,- 1,126,- 1,- 1,129,- 1,131,132,- 1,134,- 1,38,137,40,- 1,140,141,44,- 1,144,145,-
1,- 1,- 1,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,- 1,- 1,- 1,25,26,- 1,
126,- 1,- 1,129,- 1,- 1,- 1,133,134,- 1,38,137,40,- 1,140,141,44,- 1,144,145,- 1,- 1,- 1,- 1,
52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,120,- 1,- 1,25,26,- 1,126,- 1,- 1,
129,- 1,- 1,- 1,- 1,134,- 1,38,137,40,- 1,140,141,44,- 1,144,145,- 1,- 1,- 1,- 1,52,53,54,
55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,83,84,38,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,
108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,119,25,26,- 1,- 1,- 1,- 1,126,- 1,- 1,129,- 1,-
1,- 1,38,134,40,- 1,137,91,44,140,141,- 1,- 1,144,145,- 1,52,53,54,55,- 1,- 1,106,107,
60,- 1,110,111,112,- 1,- 1,115,- 1,- 1,70,71,120,73,- 1,- 1,- 1,- 1,126,- 1,128,129,- 1,83,
84,- 1,134,- 1,- 1,- 1,- 1,- 1,140,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,
109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,25,26,- 1,- 1,- 1,- 1,126,127,- 1,129,- 1,- 1,- 1,
38,134,40,- 1,137,- 1,44,140,141,- 1,- 1,144,145,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,-
1,115,- 1,- 1,118,- 1,- 1,- 1,- 1,25,26,- 1,126,- 1,- 1,129,- 1,- 1,- 1,133,134,- 1,38,137,40,
- 1,140,141,44,- 1,144,145,- 1,- 1,- 1,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,-
1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,
- 1,118,- 1,25,26,- 1,- 1,- 1,- 1,126,127,- 1,129,- 1,- 1,- 1,38,134,40,- 1,137,- 1,44,140,
141,- 1,- 1,144,145,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,
- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,- 1,- 1,- 1,
25,26,- 1,126,- 1,- 1,129,- 1,- 1,- 1,133,134,- 1,38,137,40,- 1,140,141,44,- 1,144,145,- 1,
- 1,- 1,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,- 1,- 1,- 1,25,26,- 1,
126,- 1,- 1,129,- 1,- 1,- 1,133,134,- 1,38,137,40,- 1,140,141,44,- 1,144,145,- 1,- 1,- 1,- 1,
52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,119,25,26,- 1,- 1,- 1,- 1,126,- 1,- 1,
129,- 1,- 1,- 1,38,134,40,- 1,137,- 1,44,140,141,- 1,- 1,144,145,- 1,52,53,54,55,- 1,- 1,-
1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,
110,111,112,- 1,- 1,115,- 1,- 1,118,119,25,26,- 1,- 1,- 1,- 1,126,- 1,- 1,129,- 1,- 1,- 1,38,
134,40,- 1,137,- 1,44,140,141,- 1,- 1,144,145,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,
115,- 1,- 1,118,- 1,25,26,- 1,- 1,- 1,- 1,126,127,- 1,129,- 1,- 1,- 1,38,134,40,- 1,137,- 1,
44,140,141,- 1,- 1,144,145,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,-
1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,
25,26,- 1,- 1,- 1,- 1,126,127,- 1,129,- 1,- 1,- 1,38,134,40,- 1,137,- 1,44,140,141,- 1,- 1,
144,145,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,119,25,26,- 1,- 1,- 1,-
1,126,- 1,- 1,129,- 1,- 1,- 1,38,134,40,- 1,137,- 1,44,140,141,- 1,- 1,144,145,- 1,52,53,
54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,
107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,25,26,- 1,- 1,- 1,- 1,126,127,- 1,129,
- 1,- 1,- 1,38,134,40,- 1,137,- 1,44,140,141,- 1,- 1,144,145,- 1,52,53,54,55,- 1,- 1,- 1,- 1,
60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,
112,- 1,- 1,115,- 1,- 1,118,- 1,25,26,- 1,- 1,- 1,- 1,126,127,- 1,129,- 1,- 1,- 1,38,134,40,-
1,137,- 1,44,140,141,- 1,- 1,144,145,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,-
1,- 1,- 1,- 1,70,71,- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,
- 1,118,- 1,25,26,- 1,- 1,- 1,- 1,126,127,- 1,129,- 1,- 1,- 1,38,134,40,- 1,137,- 1,44,140,
141,- 1,- 1,144,145,- 1,52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,
- 1,73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,- 1,25,26,- 1,
- 1,- 1,- 1,126,127,- 1,129,- 1,- 1,- 1,38,134,40,- 1,137,- 1,44,140,141,- 1,- 1,144,145,- 1,
52,53,54,55,- 1,- 1,- 1,- 1,60,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,70,71,- 1,73,25,26,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,83,84,- 1,- 1,38,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,52,53,54,55,- 1,
106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,70,71,- 1,73,- 1,- 1,- 1,126,- 1,- 1,
129,- 1,- 1,83,84,134,- 1,- 1,137,- 1,- 1,140,141,- 1,- 1,144,145,- 1,- 1,- 1,- 1,- 1,- 1,25,
26,- 1,106,107,108,109,110,111,112,- 1,- 1,115,38,- 1,118,- 1,120,- 1,- 1,- 1,- 1,- 1,126,
- 1,- 1,129,52,53,54,55,134,- 1,- 1,137,- 1,- 1,140,141,- 1,- 1,144,145,- 1,- 1,70,71,- 1,
73,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,25,26,- 1,106,107,108,109,110,111,112,- 1,- 1,115,38,- 1,118,- 1,120,- 1,- 1,-
1,- 1,- 1,126,- 1,- 1,129,52,53,54,55,134,- 1,- 1,137,- 1,- 1,140,141,- 1,- 1,144,145,- 1,-
1,70,71,- 1,73,25,26,- 1,- 1,- 1,- 1,- 1,- 1,- 1,83,84,- 1,- 1,38,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,
- 1,- 1,- 1,- 1,- 1,52,53,54,55,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,118,70,
71,- 1,73,25,26,- 1,126,- 1,- 1,129,- 1,- 1,83,84,134,- 1,38,137,- 1,- 1,140,141,- 1,- 1,
144,145,- 1,- 1,- 1,- 1,52,53,54,55,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,- 1,
118,70,71,- 1,73,25,26,- 1,126,- 1,- 1,129,- 1,- 1,83,84,134,- 1,38,137,- 1,- 1,140,141,-
1,- 1,144,145,- 1,- 1,- 1,- 1,52,53,54,55,- 1,106,107,108,109,110,111,112,- 1,- 1,115,- 1,
- 1,118,70,71,- 1,73,- 1,- 1,- 1,126,- 1,- 1,129,25,26,83,84,134,- 1,- 1,137,- 1,- 1,140,
141,- 1,38,144,145,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,108,109,110,111,112,- 1,- 1,
115,- 1,- 1,118,- 1,- 1,- 1,- 1,- 1,- 1,- 1,126,- 1,- 1,129,38,- 1,- 1,- 1,134,- 1,- 1,137,- 1,- 1,
140,141,- 1,- 1,144,145,- 1,91,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,106,107,- 1,
- 1,110,111,112,38,- 1,115,- 1,- 1,- 1,- 1,120,- 1,- 1,- 1,- 1,- 1,126,91,128,129,- 1,- 1,- 1,
- 1,134,- 1,- 1,137,- 1,103,140,141,106,107,144,145,110,111,112,- 1,- 1,115,- 1,- 1,- 1,-
1,120,- 1,- 1,- 1,- 1,- 1,126,- 1,128,129,91,- 1,- 1,- 1,134,- 1,- 1,- 1,- 1,- 1,140,- 1,103,- 1,
- 1,106,107,- 1,- 1,110,111,112,- 1,- 1,115,- 1,- 1,- 1,- 1,120,- 1,- 1,- 1,- 1,- 1,126,- 1,128,
129,- 1,- 1,- 1,- 1,134,- 1,- 1,- 1,- 1,- 1,140};char Cyc_Yystack_overflow[17]="Yystack_overflow";
struct Cyc_Yystack_overflow_struct{char*tag;int f1;};struct Cyc_Yystack_overflow_struct
Cyc_Yystack_overflow_val={Cyc_Yystack_overflow,0};void Cyc_yyerror(struct
_dyneither_ptr);int Cyc_yylex(struct Cyc_Lexing_lexbuf*,union Cyc_YYSTYPE*
yylval_ptr);struct _tuple26{unsigned int f1;struct _tuple0*f2;int f3;};struct
_tuple27{struct _dyneither_ptr f1;void*f2;};static char _tmp502[8]="stdcall";static
char _tmp503[6]="cdecl";static char _tmp504[9]="fastcall";static char _tmp505[9]="noreturn";
static char _tmp506[6]="const";static char _tmp507[8]="aligned";static char _tmp508[7]="packed";
static char _tmp509[7]="shared";static char _tmp50A[7]="unused";static char _tmp50B[5]="weak";
static char _tmp50C[10]="dllimport";static char _tmp50D[10]="dllexport";static char
_tmp50E[23]="no_instrument_function";static char _tmp50F[12]="constructor";static
char _tmp510[11]="destructor";static char _tmp511[22]="no_check_memory_usage";
static char _tmp512[5]="pure";struct _tuple28{void*f1;void*f2;};struct _tuple29{
struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};int Cyc_yyparse(struct
_RegionHandle*yyr,struct Cyc_Lexing_lexbuf*yylex_buf);static void _tmp9D3(union Cyc_YYSTYPE*
yylval,unsigned int*_tmp9D2,unsigned int*_tmp9D1,union Cyc_YYSTYPE**_tmp9D0){for(*
_tmp9D2=0;*_tmp9D2 < *_tmp9D1;(*_tmp9D2)++){(*_tmp9D0)[*_tmp9D2]=*yylval;}}static
void _tmp9D8(unsigned int*_tmp9D7,unsigned int*_tmp9D6,struct Cyc_Yyltype**_tmp9D5){
for(*_tmp9D7=0;*_tmp9D7 < *_tmp9D6;(*_tmp9D7)++){(*_tmp9D5)[*_tmp9D7]=Cyc_yynewloc();}}
static void _tmp9DF(int*yyssp_offset,struct _dyneither_ptr*yyss,unsigned int*
_tmp9DE,unsigned int*_tmp9DD,short**_tmp9DB){for(*_tmp9DE=0;*_tmp9DE < *_tmp9DD;(*
_tmp9DE)++){(*_tmp9DB)[*_tmp9DE]=*_tmp9DE <= *yyssp_offset?*((short*)
_check_dyneither_subscript(*yyss,sizeof(short),(int)*_tmp9DE)): 0;}}static void
_tmp9E5(struct _dyneither_ptr*yyvs,int*yyssp_offset,union Cyc_YYSTYPE*yylval,
unsigned int*_tmp9E4,unsigned int*_tmp9E3,union Cyc_YYSTYPE**_tmp9E1){for(*
_tmp9E4=0;*_tmp9E4 < *_tmp9E3;(*_tmp9E4)++){(*_tmp9E1)[*_tmp9E4]=*_tmp9E4 <= *
yyssp_offset?*((union Cyc_YYSTYPE*)_check_dyneither_subscript(*yyvs,sizeof(union
Cyc_YYSTYPE),(int)*_tmp9E4)):*yylval;}}static void _tmp9EB(int*yyssp_offset,struct
_dyneither_ptr*yyls,unsigned int*_tmp9EA,unsigned int*_tmp9E9,struct Cyc_Yyltype**
_tmp9E7){for(*_tmp9EA=0;*_tmp9EA < *_tmp9E9;(*_tmp9EA)++){(*_tmp9E7)[*_tmp9EA]=*
_tmp9EA <= *yyssp_offset?*((struct Cyc_Yyltype*)_check_dyneither_subscript(*yyls,
sizeof(struct Cyc_Yyltype),(int)*_tmp9EA)): Cyc_yynewloc();}}static void _tmpEB1(
unsigned int*_tmpEB0,unsigned int*_tmpEAF,char**_tmpEAD){for(*_tmpEB0=0;*_tmpEB0
< *_tmpEAF;(*_tmpEB0)++){(*_tmpEAD)[*_tmpEB0]='\000';}}int Cyc_yyparse(struct
_RegionHandle*yyr,struct Cyc_Lexing_lexbuf*yylex_buf){struct _RegionHandle _tmp453=
_new_region("yyregion");struct _RegionHandle*yyregion=& _tmp453;_push_region(
yyregion);{int yystate;int yyn=0;int yyerrstatus;int yychar1=0;int yychar;union Cyc_YYSTYPE
_tmp9CC;union Cyc_YYSTYPE yylval=((_tmp9CC.YYINITIALSVAL).val=0,(((_tmp9CC.YYINITIALSVAL).tag=
61,_tmp9CC)));int yynerrs;struct Cyc_Yyltype yylloc;int yyssp_offset;short*_tmp9CE;
unsigned int _tmp9CD;struct _dyneither_ptr yyss=(_tmp9CD=200,((_tmp9CE=
_region_calloc(yyregion,sizeof(short),_tmp9CD),_tag_dyneither(_tmp9CE,sizeof(
short),_tmp9CD))));int yyvsp_offset;unsigned int _tmp9D2;unsigned int _tmp9D1;union
Cyc_YYSTYPE*_tmp9D0;unsigned int _tmp9CF;struct _dyneither_ptr yyvs=_tag_dyneither(((
_tmp9CF=(unsigned int)200,((_tmp9D0=(union Cyc_YYSTYPE*)_region_malloc(yyregion,
_check_times(sizeof(union Cyc_YYSTYPE),_tmp9CF)),((((_tmp9D1=_tmp9CF,_tmp9D3(&
yylval,& _tmp9D2,& _tmp9D1,& _tmp9D0))),_tmp9D0)))))),sizeof(union Cyc_YYSTYPE),(
unsigned int)200);int yylsp_offset;unsigned int _tmp9D7;unsigned int _tmp9D6;struct
Cyc_Yyltype*_tmp9D5;unsigned int _tmp9D4;struct _dyneither_ptr yyls=_tag_dyneither(((
_tmp9D4=(unsigned int)200,((_tmp9D5=(struct Cyc_Yyltype*)_region_malloc(yyregion,
_check_times(sizeof(struct Cyc_Yyltype),_tmp9D4)),((((_tmp9D6=_tmp9D4,_tmp9D8(&
_tmp9D7,& _tmp9D6,& _tmp9D5))),_tmp9D5)))))),sizeof(struct Cyc_Yyltype),(
unsigned int)200);int yystacksize=200;union Cyc_YYSTYPE yyval=yylval;int yylen;
yystate=0;yyerrstatus=0;yynerrs=0;yychar=- 2;yyssp_offset=- 1;yyvsp_offset=0;
yylsp_offset=0;yynewstate:*((short*)_check_dyneither_subscript(yyss,sizeof(short),
++ yyssp_offset))=(short)yystate;if(yyssp_offset >= yystacksize - 1){if(yystacksize
>= 10000){{const char*_tmp9D9;Cyc_yyerror(((_tmp9D9="parser stack overflow",
_tag_dyneither(_tmp9D9,sizeof(char),22))));}(int)_throw((void*)& Cyc_Yystack_overflow_val);}
yystacksize *=2;if(yystacksize > 10000)yystacksize=10000;{unsigned int _tmp9DE;
unsigned int _tmp9DD;struct _dyneither_ptr _tmp9DC;short*_tmp9DB;unsigned int
_tmp9DA;struct _dyneither_ptr yyss1=(_tmp9DA=(unsigned int)yystacksize,((_tmp9DB=(
short*)_region_malloc(yyregion,_check_times(sizeof(short),_tmp9DA)),((_tmp9DC=
_tag_dyneither(_tmp9DB,sizeof(short),_tmp9DA),((((_tmp9DD=_tmp9DA,_tmp9DF(&
yyssp_offset,& yyss,& _tmp9DE,& _tmp9DD,& _tmp9DB))),_tmp9DC)))))));unsigned int
_tmp9E4;unsigned int _tmp9E3;struct _dyneither_ptr _tmp9E2;union Cyc_YYSTYPE*_tmp9E1;
unsigned int _tmp9E0;struct _dyneither_ptr yyvs1=(_tmp9E0=(unsigned int)yystacksize,((
_tmp9E1=(union Cyc_YYSTYPE*)_region_malloc(yyregion,_check_times(sizeof(union Cyc_YYSTYPE),
_tmp9E0)),((_tmp9E2=_tag_dyneither(_tmp9E1,sizeof(union Cyc_YYSTYPE),_tmp9E0),((((
_tmp9E3=_tmp9E0,_tmp9E5(& yyvs,& yyssp_offset,& yylval,& _tmp9E4,& _tmp9E3,& _tmp9E1))),
_tmp9E2)))))));unsigned int _tmp9EA;unsigned int _tmp9E9;struct _dyneither_ptr
_tmp9E8;struct Cyc_Yyltype*_tmp9E7;unsigned int _tmp9E6;struct _dyneither_ptr yyls1=(
_tmp9E6=(unsigned int)yystacksize,((_tmp9E7=(struct Cyc_Yyltype*)_region_malloc(
yyregion,_check_times(sizeof(struct Cyc_Yyltype),_tmp9E6)),((_tmp9E8=
_tag_dyneither(_tmp9E7,sizeof(struct Cyc_Yyltype),_tmp9E6),((((_tmp9E9=_tmp9E6,
_tmp9EB(& yyssp_offset,& yyls,& _tmp9EA,& _tmp9E9,& _tmp9E7))),_tmp9E8)))))));yyss=
yyss1;yyvs=yyvs1;yyls=yyls1;};}goto yybackup;yybackup: yyn=(int)Cyc_yypact[
_check_known_subscript_notnull(1030,yystate)];if(yyn == - 32768)goto yydefault;if(
yychar == - 2)yychar=Cyc_yylex(yylex_buf,& yylval);if(yychar <= 0){yychar1=0;yychar=
0;}else{yychar1=yychar > 0  && yychar <= 373?(int)Cyc_yytranslate[
_check_known_subscript_notnull(374,yychar)]: 289;}yyn +=yychar1;if((yyn < 0  || yyn > 
6375) || Cyc_yycheck[_check_known_subscript_notnull(6376,yyn)]!= yychar1)goto
yydefault;yyn=(int)Cyc_yytable[_check_known_subscript_notnull(6376,yyn)];if(yyn < 
0){if(yyn == - 32768)goto yyerrlab;yyn=- yyn;goto yyreduce;}else{if(yyn == 0)goto
yyerrlab;}if(yyn == 1029){int _tmp461=0;_npop_handler(0);return _tmp461;}if(yychar
!= 0)yychar=- 2;*((union Cyc_YYSTYPE*)_check_dyneither_subscript(yyvs,sizeof(union
Cyc_YYSTYPE),++ yyvsp_offset))=yylval;*((struct Cyc_Yyltype*)
_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),++ yylsp_offset))=
yylloc;if(yyerrstatus != 0)-- yyerrstatus;yystate=yyn;goto yynewstate;yydefault: yyn=(
int)Cyc_yydefact[_check_known_subscript_notnull(1030,yystate)];if(yyn == 0)goto
yyerrlab;yyreduce: yylen=(int)Cyc_yyr2[_check_known_subscript_notnull(515,yyn)];
if(yylen > 0)yyval=*((union Cyc_YYSTYPE*)_check_dyneither_subscript(yyvs,sizeof(
union Cyc_YYSTYPE),(yyvsp_offset + 1)- yylen));switch(yyn){case 1: _LL205: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
yyyyvsp[0];Cyc_Parse_parse_result=Cyc_yyget_YY18(yyyyvsp[0]);break;}case 2: _LL206: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_List_List*x;struct Cyc_List_List*
y;{struct _handler_cons _tmp462;_push_handler(& _tmp462);{int _tmp464=0;if(setjmp(
_tmp462.handler))_tmp464=1;if(!_tmp464){x=Cyc_yyget_YY18(yyyyvsp[0]);;
_pop_handler();}else{void*_tmp463=(void*)_exn_thrown;void*_tmp466=_tmp463;_LL209: {
struct Cyc_Core_Failure_struct*_tmp467=(struct Cyc_Core_Failure_struct*)_tmp466;
if(_tmp467->tag != Cyc_Core_Failure)goto _LL20B;}_LL20A: x=0;goto _LL208;_LL20B:;
_LL20C:(void)_throw(_tmp466);_LL208:;}};}{struct _handler_cons _tmp468;
_push_handler(& _tmp468);{int _tmp46A=0;if(setjmp(_tmp468.handler))_tmp46A=1;if(!
_tmp46A){y=Cyc_yyget_YY18(yyyyvsp[1]);;_pop_handler();}else{void*_tmp469=(void*)
_exn_thrown;void*_tmp46C=_tmp469;_LL20E: {struct Cyc_Core_Failure_struct*_tmp46D=(
struct Cyc_Core_Failure_struct*)_tmp46C;if(_tmp46D->tag != Cyc_Core_Failure)goto
_LL210;}_LL20F: y=0;goto _LL20D;_LL210:;_LL211:(void)_throw(_tmp46C);_LL20D:;}};}
yyval=Cyc_YY18(((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*
y))Cyc_List_imp_append)(x,y));break;}case 3: _LL207: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_Decl*_tmp9F5;
struct Cyc_Absyn_Using_d_struct _tmp9F4;struct Cyc_Absyn_Using_d_struct*_tmp9F3;
struct Cyc_List_List*_tmp9F2;yyval=Cyc_YY18(((_tmp9F2=_cycalloc(sizeof(*_tmp9F2)),((
_tmp9F2->hd=((_tmp9F5=_cycalloc(sizeof(*_tmp9F5)),((_tmp9F5->r=(void*)((_tmp9F3=
_cycalloc(sizeof(*_tmp9F3)),((_tmp9F3[0]=((_tmp9F4.tag=11,((_tmp9F4.f1=Cyc_yyget_QualId_tok(
yyyyvsp[0]),((_tmp9F4.f2=Cyc_yyget_YY18(yyyyvsp[2]),_tmp9F4)))))),_tmp9F3)))),((
_tmp9F5->loc=(unsigned int)(yyyylsp[0]).first_line,_tmp9F5)))))),((_tmp9F2->tl=0,
_tmp9F2)))))));}Cyc_Lex_leave_using();break;}case 4: _LL212: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct
Cyc_Absyn_Decl*_tmp9FF;struct Cyc_Absyn_Using_d_struct _tmp9FE;struct Cyc_Absyn_Using_d_struct*
_tmp9FD;struct Cyc_List_List*_tmp9FC;yyval=Cyc_YY18(((_tmp9FC=_cycalloc(sizeof(*
_tmp9FC)),((_tmp9FC->hd=((_tmp9FF=_cycalloc(sizeof(*_tmp9FF)),((_tmp9FF->r=(void*)((
_tmp9FD=_cycalloc(sizeof(*_tmp9FD)),((_tmp9FD[0]=((_tmp9FE.tag=11,((_tmp9FE.f1=
Cyc_yyget_QualId_tok(yyyyvsp[0]),((_tmp9FE.f2=Cyc_yyget_YY18(yyyyvsp[2]),_tmp9FE)))))),
_tmp9FD)))),((_tmp9FF->loc=(unsigned int)(yyyylsp[0]).first_line,_tmp9FF)))))),((
_tmp9FC->tl=Cyc_yyget_YY18(yyyyvsp[4]),_tmp9FC)))))));}break;}case 5: _LL213: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_Decl*_tmpA0E;
struct Cyc_Absyn_Namespace_d_struct _tmpA0D;struct _dyneither_ptr*_tmpA0C;struct Cyc_Absyn_Namespace_d_struct*
_tmpA0B;struct Cyc_List_List*_tmpA0A;yyval=Cyc_YY18(((_tmpA0A=_cycalloc(sizeof(*
_tmpA0A)),((_tmpA0A->hd=((_tmpA0E=_cycalloc(sizeof(*_tmpA0E)),((_tmpA0E->r=(void*)((
_tmpA0B=_cycalloc(sizeof(*_tmpA0B)),((_tmpA0B[0]=((_tmpA0D.tag=10,((_tmpA0D.f1=((
_tmpA0C=_cycalloc(sizeof(*_tmpA0C)),((_tmpA0C[0]=Cyc_yyget_String_tok(yyyyvsp[0]),
_tmpA0C)))),((_tmpA0D.f2=Cyc_yyget_YY18(yyyyvsp[2]),_tmpA0D)))))),_tmpA0B)))),((
_tmpA0E->loc=(unsigned int)(yyyylsp[0]).first_line,_tmpA0E)))))),((_tmpA0A->tl=0,
_tmpA0A)))))));}Cyc_Lex_leave_namespace();break;}case 6: _LL214: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct
_dyneither_ptr nspace;struct Cyc_List_List*x;struct Cyc_List_List*y;{struct
_handler_cons _tmp47B;_push_handler(& _tmp47B);{int _tmp47D=0;if(setjmp(_tmp47B.handler))
_tmp47D=1;if(!_tmp47D){nspace=Cyc_yyget_String_tok(yyyyvsp[0]);;_pop_handler();}
else{void*_tmp47C=(void*)_exn_thrown;void*_tmp47F=_tmp47C;_LL217: {struct Cyc_Core_Failure_struct*
_tmp480=(struct Cyc_Core_Failure_struct*)_tmp47F;if(_tmp480->tag != Cyc_Core_Failure)
goto _LL219;}_LL218:{const char*_tmpA0F;nspace=((_tmpA0F="",_tag_dyneither(_tmpA0F,
sizeof(char),1)));}goto _LL216;_LL219:;_LL21A:(void)_throw(_tmp47F);_LL216:;}};}{
struct _handler_cons _tmp482;_push_handler(& _tmp482);{int _tmp484=0;if(setjmp(
_tmp482.handler))_tmp484=1;if(!_tmp484){x=Cyc_yyget_YY18(yyyyvsp[2]);;
_pop_handler();}else{void*_tmp483=(void*)_exn_thrown;void*_tmp486=_tmp483;_LL21C: {
struct Cyc_Core_Failure_struct*_tmp487=(struct Cyc_Core_Failure_struct*)_tmp486;
if(_tmp487->tag != Cyc_Core_Failure)goto _LL21E;}_LL21D: x=0;goto _LL21B;_LL21E:;
_LL21F:(void)_throw(_tmp486);_LL21B:;}};}{struct _handler_cons _tmp488;
_push_handler(& _tmp488);{int _tmp48A=0;if(setjmp(_tmp488.handler))_tmp48A=1;if(!
_tmp48A){y=Cyc_yyget_YY18(yyyyvsp[4]);;_pop_handler();}else{void*_tmp489=(void*)
_exn_thrown;void*_tmp48C=_tmp489;_LL221: {struct Cyc_Core_Failure_struct*_tmp48D=(
struct Cyc_Core_Failure_struct*)_tmp48C;if(_tmp48D->tag != Cyc_Core_Failure)goto
_LL223;}_LL222: y=0;goto _LL220;_LL223:;_LL224:(void)_throw(_tmp48C);_LL220:;}};}{
struct Cyc_Absyn_Decl*_tmpA1E;struct Cyc_Absyn_Namespace_d_struct _tmpA1D;struct
_dyneither_ptr*_tmpA1C;struct Cyc_Absyn_Namespace_d_struct*_tmpA1B;struct Cyc_List_List*
_tmpA1A;yyval=Cyc_YY18(((_tmpA1A=_cycalloc(sizeof(*_tmpA1A)),((_tmpA1A->hd=((
_tmpA1E=_cycalloc(sizeof(*_tmpA1E)),((_tmpA1E->r=(void*)((_tmpA1B=_cycalloc(
sizeof(*_tmpA1B)),((_tmpA1B[0]=((_tmpA1D.tag=10,((_tmpA1D.f1=((_tmpA1C=_cycalloc(
sizeof(*_tmpA1C)),((_tmpA1C[0]=nspace,_tmpA1C)))),((_tmpA1D.f2=x,_tmpA1D)))))),
_tmpA1B)))),((_tmpA1E->loc=(unsigned int)(yyyylsp[0]).first_line,_tmpA1E)))))),((
_tmpA1A->tl=y,_tmpA1A)))))));}break;}case 7: _LL215: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 5),sizeof(union Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);struct _dyneither_ptr _tmp493=Cyc_yyget_String_tok(
yyyyvsp[1]);{const char*_tmpA1F;if(Cyc_strcmp((struct _dyneither_ptr)_tmp493,((
_tmpA1F="C",_tag_dyneither(_tmpA1F,sizeof(char),2))))== 0){struct Cyc_Absyn_Decl*
_tmpA29;struct Cyc_Absyn_ExternC_d_struct _tmpA28;struct Cyc_Absyn_ExternC_d_struct*
_tmpA27;struct Cyc_List_List*_tmpA26;yyval=Cyc_YY18(((_tmpA26=_cycalloc(sizeof(*
_tmpA26)),((_tmpA26->hd=((_tmpA29=_cycalloc(sizeof(*_tmpA29)),((_tmpA29->r=(void*)((
_tmpA27=_cycalloc(sizeof(*_tmpA27)),((_tmpA27[0]=((_tmpA28.tag=12,((_tmpA28.f1=
Cyc_yyget_YY18(yyyyvsp[3]),_tmpA28)))),_tmpA27)))),((_tmpA29->loc=(unsigned int)(
yyyylsp[0]).first_line,_tmpA29)))))),((_tmpA26->tl=Cyc_yyget_YY18(yyyyvsp[5]),
_tmpA26)))))));}else{{const char*_tmpA2A;if(Cyc_strcmp((struct _dyneither_ptr)
_tmp493,((_tmpA2A="C include",_tag_dyneither(_tmpA2A,sizeof(char),10))))!= 0){
const char*_tmpA2B;Cyc_Parse_err(((_tmpA2B="expecting \"C\" or \"C include\"",
_tag_dyneither(_tmpA2B,sizeof(char),29))),(unsigned int)(yyyylsp[0]).first_line);}}{
struct Cyc_Absyn_Decl*_tmpA35;struct Cyc_Absyn_ExternCinclude_d_struct _tmpA34;
struct Cyc_Absyn_ExternCinclude_d_struct*_tmpA33;struct Cyc_List_List*_tmpA32;
yyval=Cyc_YY18(((_tmpA32=_cycalloc(sizeof(*_tmpA32)),((_tmpA32->hd=((_tmpA35=
_cycalloc(sizeof(*_tmpA35)),((_tmpA35->r=(void*)((_tmpA33=_cycalloc(sizeof(*
_tmpA33)),((_tmpA33[0]=((_tmpA34.tag=13,((_tmpA34.f1=Cyc_yyget_YY18(yyyyvsp[3]),((
_tmpA34.f2=0,_tmpA34)))))),_tmpA33)))),((_tmpA35->loc=(unsigned int)(yyyylsp[0]).first_line,
_tmpA35)))))),((_tmpA32->tl=Cyc_yyget_YY18(yyyyvsp[5]),_tmpA32)))))));};}}break;}
case 8: _LL225: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 6),sizeof(union
Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);{const char*_tmpA36;if(Cyc_strcmp((
struct _dyneither_ptr)Cyc_yyget_String_tok(yyyyvsp[1]),((_tmpA36="C include",
_tag_dyneither(_tmpA36,sizeof(char),10))))!= 0){const char*_tmpA37;Cyc_Parse_err(((
_tmpA37="expecting \"C include\"",_tag_dyneither(_tmpA37,sizeof(char),22))),(
unsigned int)(yyyylsp[0]).first_line);}}{struct Cyc_List_List*exs=Cyc_yyget_YY52(
yyyyvsp[5]);{struct Cyc_Absyn_Decl*_tmpA41;struct Cyc_Absyn_ExternCinclude_d_struct
_tmpA40;struct Cyc_Absyn_ExternCinclude_d_struct*_tmpA3F;struct Cyc_List_List*
_tmpA3E;yyval=Cyc_YY18(((_tmpA3E=_cycalloc(sizeof(*_tmpA3E)),((_tmpA3E->hd=((
_tmpA41=_cycalloc(sizeof(*_tmpA41)),((_tmpA41->r=(void*)((_tmpA3F=_cycalloc(
sizeof(*_tmpA3F)),((_tmpA3F[0]=((_tmpA40.tag=13,((_tmpA40.f1=Cyc_yyget_YY18(
yyyyvsp[3]),((_tmpA40.f2=exs,_tmpA40)))))),_tmpA3F)))),((_tmpA41->loc=(
unsigned int)(yyyylsp[0]).first_line,_tmpA41)))))),((_tmpA3E->tl=Cyc_yyget_YY18(
yyyyvsp[6]),_tmpA3E)))))));}break;};}case 9: _LL226: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_Decl*_tmpA44;
struct Cyc_List_List*_tmpA43;yyval=Cyc_YY18(((_tmpA43=_cycalloc(sizeof(*_tmpA43)),((
_tmpA43->hd=((_tmpA44=_cycalloc(sizeof(*_tmpA44)),((_tmpA44->r=(void*)& Cyc_Absyn_Porton_d_val,((
_tmpA44->loc=(unsigned int)(yyyylsp[0]).first_line,_tmpA44)))))),((_tmpA43->tl=
Cyc_yyget_YY18(yyyyvsp[2]),_tmpA43)))))));}break;}case 10: _LL227: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct
Cyc_Absyn_Decl*_tmpA47;struct Cyc_List_List*_tmpA46;yyval=Cyc_YY18(((_tmpA46=
_cycalloc(sizeof(*_tmpA46)),((_tmpA46->hd=((_tmpA47=_cycalloc(sizeof(*_tmpA47)),((
_tmpA47->r=(void*)& Cyc_Absyn_Portoff_d_val,((_tmpA47->loc=(unsigned int)(yyyylsp[
0]).first_line,_tmpA47)))))),((_tmpA46->tl=Cyc_yyget_YY18(yyyyvsp[2]),_tmpA46)))))));}
break;}case 11: _LL228: yyval=Cyc_YY18(0);break;case 12: _LL229: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);yyval=
yyyyvsp[2];break;}case 13: _LL22A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY52(0);break;}case 14:
_LL22B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple26*_tmpA4A;struct Cyc_List_List*
_tmpA49;yyval=Cyc_YY52(((_tmpA49=_cycalloc(sizeof(*_tmpA49)),((_tmpA49->hd=((
_tmpA4A=_cycalloc(sizeof(*_tmpA4A)),((_tmpA4A->f1=(unsigned int)(yyyylsp[0]).first_line,((
_tmpA4A->f2=Cyc_yyget_QualId_tok(yyyyvsp[0]),((_tmpA4A->f3=0,_tmpA4A)))))))),((
_tmpA49->tl=0,_tmpA49)))))));}break;}case 15: _LL22C: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct _tuple26*_tmpA4D;struct Cyc_List_List*
_tmpA4C;yyval=Cyc_YY52(((_tmpA4C=_cycalloc(sizeof(*_tmpA4C)),((_tmpA4C->hd=((
_tmpA4D=_cycalloc(sizeof(*_tmpA4D)),((_tmpA4D->f1=(unsigned int)(yyyylsp[0]).first_line,((
_tmpA4D->f2=Cyc_yyget_QualId_tok(yyyyvsp[0]),((_tmpA4D->f3=0,_tmpA4D)))))))),((
_tmpA4C->tl=0,_tmpA4C)))))));}break;}case 16: _LL22D: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _tuple26*_tmpA50;struct Cyc_List_List*
_tmpA4F;yyval=Cyc_YY52(((_tmpA4F=_cycalloc(sizeof(*_tmpA4F)),((_tmpA4F->hd=((
_tmpA50=_cycalloc(sizeof(*_tmpA50)),((_tmpA50->f1=(unsigned int)(yyyylsp[0]).first_line,((
_tmpA50->f2=Cyc_yyget_QualId_tok(yyyyvsp[0]),((_tmpA50->f3=0,_tmpA50)))))))),((
_tmpA4F->tl=Cyc_yyget_YY52(yyyyvsp[2]),_tmpA4F)))))));}break;}case 17: _LL22E: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Fn_d_struct*
_tmpA56;struct Cyc_Absyn_Fn_d_struct _tmpA55;struct Cyc_List_List*_tmpA54;yyval=Cyc_YY18(((
_tmpA54=_cycalloc(sizeof(*_tmpA54)),((_tmpA54->hd=Cyc_Absyn_new_decl((void*)((
_tmpA56=_cycalloc(sizeof(*_tmpA56)),((_tmpA56[0]=((_tmpA55.tag=1,((_tmpA55.f1=
Cyc_yyget_YY17(yyyyvsp[0]),_tmpA55)))),_tmpA56)))),(unsigned int)(yyyylsp[0]).first_line),((
_tmpA54->tl=0,_tmpA54)))))));}break;}case 18: _LL22F: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 19:
_LL230: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY18(0);break;}case 20:
_LL231: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY17(Cyc_Parse_make_function(
yyr,0,Cyc_yyget_YY29(yyyyvsp[0]),0,Cyc_yyget_YY9(yyyyvsp[1]),(unsigned int)(
yyyylsp[0]).first_line));break;}case 21: _LL232: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);struct Cyc_Parse_Declaration_spec
_tmp4B2=Cyc_yyget_YY19(yyyyvsp[0]);yyval=Cyc_YY17(Cyc_Parse_make_function(yyr,(
struct Cyc_Parse_Declaration_spec*)& _tmp4B2,Cyc_yyget_YY29(yyyyvsp[1]),0,Cyc_yyget_YY9(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));break;}case 22: _LL233: {union
Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(
yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);
struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(
struct Cyc_Yyltype),3);yyval=Cyc_YY17(Cyc_Parse_make_function(yyr,0,Cyc_yyget_YY29(
yyyyvsp[0]),Cyc_yyget_YY18(yyyyvsp[1]),Cyc_yyget_YY9(yyyyvsp[2]),(unsigned int)(
yyyylsp[0]).first_line));break;}case 23: _LL234: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_Parse_Declaration_spec
_tmp4B3=Cyc_yyget_YY19(yyyyvsp[0]);yyval=Cyc_YY17(Cyc_Parse_make_function(yyr,(
struct Cyc_Parse_Declaration_spec*)& _tmp4B3,Cyc_yyget_YY29(yyyyvsp[1]),Cyc_yyget_YY18(
yyyyvsp[2]),Cyc_yyget_YY9(yyyyvsp[3]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 24: _LL235: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);struct Cyc_Parse_Declaration_spec
_tmp4B4=Cyc_yyget_YY19(yyyyvsp[0]);yyval=Cyc_YY17(Cyc_Parse_make_function(yyr,(
struct Cyc_Parse_Declaration_spec*)& _tmp4B4,Cyc_yyget_YY29(yyyyvsp[1]),0,Cyc_yyget_YY9(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));break;}case 25: _LL236: {union
Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(
yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);
struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset - 3),sizeof(
struct Cyc_Yyltype),4);struct Cyc_Parse_Declaration_spec _tmp4B5=Cyc_yyget_YY19(
yyyyvsp[0]);yyval=Cyc_YY17(Cyc_Parse_make_function(yyr,(struct Cyc_Parse_Declaration_spec*)&
_tmp4B5,Cyc_yyget_YY29(yyyyvsp[1]),Cyc_yyget_YY18(yyyyvsp[2]),Cyc_yyget_YY9(
yyyyvsp[3]),(unsigned int)(yyyylsp[0]).first_line));break;}case 26: _LL237: {union
Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(
yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);
struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(
struct Cyc_Yyltype),2);Cyc_Lex_enter_using(Cyc_yyget_QualId_tok(yyyyvsp[1]));
yyval=yyyyvsp[1];break;}case 27: _LL238: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);Cyc_Lex_leave_using();break;}case
28: _LL239: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct _dyneither_ptr*_tmpA57;Cyc_Lex_enter_namespace(((
_tmpA57=_cycalloc(sizeof(*_tmpA57)),((_tmpA57[0]=Cyc_yyget_String_tok(yyyyvsp[1]),
_tmpA57)))));}yyval=yyyyvsp[1];break;}case 29: _LL23A: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);Cyc_Lex_leave_namespace();break;}
case 30: _LL23B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);int _tmp4B7=(yyyylsp[0]).first_line;
yyval=Cyc_YY18(Cyc_Parse_make_declarations(Cyc_yyget_YY19(yyyyvsp[0]),0,(
unsigned int)_tmp4B7,(unsigned int)_tmp4B7));break;}case 31: _LL23C: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=
Cyc_YY18(Cyc_Parse_make_declarations(Cyc_yyget_YY19(yyyyvsp[0]),Cyc_yyget_YY21(
yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line,(unsigned int)(yyyylsp[0]).first_line));
break;}case 32: _LL23D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct Cyc_List_List*_tmpA58;yyval=
Cyc_YY18(((_tmpA58=_cycalloc(sizeof(*_tmpA58)),((_tmpA58->hd=Cyc_Absyn_let_decl(
Cyc_yyget_YY11(yyyyvsp[1]),Cyc_yyget_YY3(yyyyvsp[3]),(unsigned int)(yyyylsp[0]).first_line),((
_tmpA58->tl=0,_tmpA58)))))));}break;}case 33: _LL23E: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);struct Cyc_List_List*_tmp4B9=0;{
struct Cyc_List_List*_tmp4BA=Cyc_yyget_YY36(yyyyvsp[1]);for(0;_tmp4BA != 0;_tmp4BA=
_tmp4BA->tl){struct _dyneither_ptr*_tmp4BB=(struct _dyneither_ptr*)_tmp4BA->hd;
struct _tuple0*_tmpA59;struct _tuple0*qv=(_tmpA59=_cycalloc(sizeof(*_tmpA59)),((
_tmpA59->f1=Cyc_Absyn_Rel_n(0),((_tmpA59->f2=_tmp4BB,_tmpA59)))));struct Cyc_Absyn_Vardecl*
_tmp4BC=Cyc_Absyn_new_vardecl(qv,Cyc_Absyn_wildtyp(0),0);struct Cyc_List_List*
_tmpA5A;_tmp4B9=((_tmpA5A=_cycalloc(sizeof(*_tmpA5A)),((_tmpA5A->hd=_tmp4BC,((
_tmpA5A->tl=_tmp4B9,_tmpA5A))))));}}_tmp4B9=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_imp_rev)(_tmp4B9);{struct Cyc_List_List*_tmpA5B;yyval=Cyc_YY18(((
_tmpA5B=_cycalloc(sizeof(*_tmpA5B)),((_tmpA5B->hd=Cyc_Absyn_letv_decl(_tmp4B9,(
unsigned int)(yyyylsp[0]).first_line),((_tmpA5B->tl=0,_tmpA5B)))))));}break;}
case 34: _LL23F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 6),sizeof(union
Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);struct _dyneither_ptr _tmp4C0=Cyc_yyget_String_tok(
yyyyvsp[3]);{const char*_tmpA5C;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp4C0,((
_tmpA5C="`H",_tag_dyneither(_tmpA5C,sizeof(char),3))))== 0){const char*_tmpA5F;
void*_tmpA5E;Cyc_Parse_err((struct _dyneither_ptr)((_tmpA5E=0,Cyc_aprintf(((
_tmpA5F="bad occurrence of heap region",_tag_dyneither(_tmpA5F,sizeof(char),30))),
_tag_dyneither(_tmpA5E,sizeof(void*),0)))),(unsigned int)(yyyylsp[3]).first_line);}}{
const char*_tmpA60;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp4C0,((_tmpA60="`U",
_tag_dyneither(_tmpA60,sizeof(char),3))))== 0){const char*_tmpA63;void*_tmpA62;
Cyc_Parse_err((struct _dyneither_ptr)((_tmpA62=0,Cyc_aprintf(((_tmpA63="bad occurrence of unique region",
_tag_dyneither(_tmpA63,sizeof(char),32))),_tag_dyneither(_tmpA62,sizeof(void*),0)))),(
unsigned int)(yyyylsp[3]).first_line);}}{struct _dyneither_ptr*_tmpA66;struct Cyc_Absyn_Tvar*
_tmpA65;struct Cyc_Absyn_Tvar*tv=(_tmpA65=_cycalloc(sizeof(*_tmpA65)),((_tmpA65->name=((
_tmpA66=_cycalloc(sizeof(*_tmpA66)),((_tmpA66[0]=_tmp4C0,_tmpA66)))),((_tmpA65->identity=
- 1,((_tmpA65->kind=Cyc_Tcutil_kind_to_bound(& Cyc_Tcutil_rk),_tmpA65)))))));
struct Cyc_Absyn_VarType_struct _tmpA69;struct Cyc_Absyn_VarType_struct*_tmpA68;
void*t=(void*)((_tmpA68=_cycalloc(sizeof(*_tmpA68)),((_tmpA68[0]=((_tmpA69.tag=2,((
_tmpA69.f1=tv,_tmpA69)))),_tmpA68))));struct Cyc_Absyn_RgnHandleType_struct
_tmpA6F;struct Cyc_Absyn_RgnHandleType_struct*_tmpA6E;struct _dyneither_ptr*
_tmpA6C;struct _tuple0*_tmpA6B;struct Cyc_Absyn_Vardecl*vd=Cyc_Absyn_new_vardecl(((
_tmpA6B=_cycalloc(sizeof(*_tmpA6B)),((_tmpA6B->f1=Cyc_Absyn_Loc_n,((_tmpA6B->f2=((
_tmpA6C=_cycalloc(sizeof(*_tmpA6C)),((_tmpA6C[0]=Cyc_yyget_String_tok(yyyyvsp[5]),
_tmpA6C)))),_tmpA6B)))))),(void*)((_tmpA6E=_cycalloc(sizeof(*_tmpA6E)),((_tmpA6E[
0]=((_tmpA6F.tag=15,((_tmpA6F.f1=(void*)t,_tmpA6F)))),_tmpA6E)))),0);{struct Cyc_List_List*
_tmpA70;yyval=Cyc_YY18(((_tmpA70=_cycalloc(sizeof(*_tmpA70)),((_tmpA70->hd=Cyc_Absyn_region_decl(
tv,vd,Cyc_yyget_YY31(yyyyvsp[0]),0,(unsigned int)(yyyylsp[0]).first_line),((
_tmpA70->tl=0,_tmpA70)))))));}break;};}case 35: _LL240: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);int _tmp4D0=Cyc_yyget_YY31(yyyyvsp[
0]);struct _dyneither_ptr _tmp4D1=Cyc_yyget_String_tok(yyyyvsp[2]);struct Cyc_Absyn_Exp*
_tmp4D2=Cyc_yyget_YY4(yyyyvsp[3]);{const char*_tmpA71;if(Cyc_zstrcmp((struct
_dyneither_ptr)_tmp4D1,((_tmpA71="H",_tag_dyneither(_tmpA71,sizeof(char),2))))== 
0){const char*_tmpA72;Cyc_Parse_err(((_tmpA72="bad occurrence of heap region `H",
_tag_dyneither(_tmpA72,sizeof(char),33))),(unsigned int)(yyyylsp[2]).first_line);}}{
const char*_tmpA73;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp4D1,((_tmpA73="U",
_tag_dyneither(_tmpA73,sizeof(char),2))))== 0){const char*_tmpA74;Cyc_Parse_err(((
_tmpA74="bad occurrence of unique region `U",_tag_dyneither(_tmpA74,sizeof(char),
35))),(unsigned int)(yyyylsp[2]).first_line);}}if(_tmp4D0  && _tmp4D2 != 0){const
char*_tmpA75;Cyc_Parse_err(((_tmpA75="open regions cannot be @resetable",
_tag_dyneither(_tmpA75,sizeof(char),34))),(unsigned int)(yyyylsp[0]).first_line);}{
struct _dyneither_ptr*_tmpA82;const char*_tmpA81;void*_tmpA80[1];struct Cyc_String_pa_struct
_tmpA7F;struct Cyc_Absyn_Tvar*_tmpA7E;struct Cyc_Absyn_Tvar*tv=(_tmpA7E=_cycalloc(
sizeof(*_tmpA7E)),((_tmpA7E->name=((_tmpA82=_cycalloc(sizeof(*_tmpA82)),((
_tmpA82[0]=(struct _dyneither_ptr)((_tmpA7F.tag=0,((_tmpA7F.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)_tmp4D1),((_tmpA80[0]=& _tmpA7F,Cyc_aprintf(((
_tmpA81="`%s",_tag_dyneither(_tmpA81,sizeof(char),4))),_tag_dyneither(_tmpA80,
sizeof(void*),1)))))))),_tmpA82)))),((_tmpA7E->identity=- 1,((_tmpA7E->kind=Cyc_Tcutil_kind_to_bound(&
Cyc_Tcutil_rk),_tmpA7E)))))));struct Cyc_Absyn_VarType_struct _tmpA85;struct Cyc_Absyn_VarType_struct*
_tmpA84;void*t=(void*)((_tmpA84=_cycalloc(sizeof(*_tmpA84)),((_tmpA84[0]=((
_tmpA85.tag=2,((_tmpA85.f1=tv,_tmpA85)))),_tmpA84))));struct Cyc_Absyn_RgnHandleType_struct
_tmpA8B;struct Cyc_Absyn_RgnHandleType_struct*_tmpA8A;struct _dyneither_ptr*
_tmpA88;struct _tuple0*_tmpA87;struct Cyc_Absyn_Vardecl*vd=Cyc_Absyn_new_vardecl(((
_tmpA87=_cycalloc(sizeof(*_tmpA87)),((_tmpA87->f1=Cyc_Absyn_Loc_n,((_tmpA87->f2=((
_tmpA88=_cycalloc(sizeof(*_tmpA88)),((_tmpA88[0]=_tmp4D1,_tmpA88)))),_tmpA87)))))),(
void*)((_tmpA8A=_cycalloc(sizeof(*_tmpA8A)),((_tmpA8A[0]=((_tmpA8B.tag=15,((
_tmpA8B.f1=(void*)t,_tmpA8B)))),_tmpA8A)))),0);{struct Cyc_List_List*_tmpA8C;
yyval=Cyc_YY18(((_tmpA8C=_cycalloc(sizeof(*_tmpA8C)),((_tmpA8C->hd=Cyc_Absyn_region_decl(
tv,vd,_tmp4D0,_tmp4D2,(unsigned int)(yyyylsp[0]).first_line),((_tmpA8C->tl=0,
_tmpA8C)))))));}break;};}case 36: _LL241: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 7),sizeof(union Cyc_YYSTYPE),8);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 7),sizeof(struct Cyc_Yyltype),8);struct Cyc_Absyn_Eq_kb_struct*
_tmpA94;struct Cyc_Absyn_Eq_kb_struct _tmpA93;struct _dyneither_ptr*_tmpA92;struct
Cyc_Absyn_Tvar*_tmpA91;struct Cyc_Absyn_Tvar*tv=(_tmpA91=_cycalloc(sizeof(*
_tmpA91)),((_tmpA91->name=((_tmpA92=_cycalloc(sizeof(*_tmpA92)),((_tmpA92[0]=Cyc_yyget_String_tok(
yyyyvsp[2]),_tmpA92)))),((_tmpA91->identity=- 1,((_tmpA91->kind=(void*)((_tmpA94=
_cycalloc(sizeof(*_tmpA94)),((_tmpA94[0]=((_tmpA93.tag=0,((_tmpA93.f1=& Cyc_Tcutil_rk,
_tmpA93)))),_tmpA94)))),_tmpA91)))))));struct _dyneither_ptr*_tmpA97;struct
_tuple0*_tmpA96;struct Cyc_Absyn_Vardecl*vd=Cyc_Absyn_new_vardecl(((_tmpA96=
_cycalloc(sizeof(*_tmpA96)),((_tmpA96->f1=Cyc_Absyn_Loc_n,((_tmpA96->f2=((
_tmpA97=_cycalloc(sizeof(*_tmpA97)),((_tmpA97[0]=Cyc_yyget_String_tok(yyyyvsp[4]),
_tmpA97)))),_tmpA96)))))),(void*)& Cyc_Absyn_VoidType_val,0);{struct Cyc_List_List*
_tmpA98;yyval=Cyc_YY18(((_tmpA98=_cycalloc(sizeof(*_tmpA98)),((_tmpA98->hd=Cyc_Absyn_alias_decl(
Cyc_yyget_YY3(yyyyvsp[6]),tv,vd,(unsigned int)(yyyylsp[0]).first_line),((_tmpA98->tl=
0,_tmpA98)))))));}break;}case 37: _LL242: yyval=Cyc_YY31(0);break;case 38: _LL243: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY31(1);break;}case 39:
_LL244: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 40:
_LL245: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY18(((struct Cyc_List_List*(*)(
struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(Cyc_yyget_YY18(
yyyyvsp[0]),Cyc_yyget_YY18(yyyyvsp[1])));break;}case 41: _LL246: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct
Cyc_Parse_Declaration_spec _tmpA99;yyval=Cyc_YY19(((_tmpA99.sc=(enum Cyc_Parse_Storage_class*)
Cyc_yyget_YY22(yyyyvsp[0]),((_tmpA99.tq=Cyc_Absyn_empty_tqual((unsigned int)(
yyyylsp[0]).first_line),((_tmpA99.type_specs=Cyc_Parse_empty_spec(0),((_tmpA99.is_inline=
0,((_tmpA99.attributes=0,_tmpA99)))))))))));}break;}case 42: _LL247: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct
Cyc_Parse_Declaration_spec _tmp4EC=Cyc_yyget_YY19(yyyyvsp[1]);if(_tmp4EC.sc != 0){
const char*_tmpA9C;void*_tmpA9B;(_tmpA9B=0,Cyc_Tcutil_warn((unsigned int)(yyyylsp[
0]).first_line,((_tmpA9C="Only one storage class is allowed in a declaration",
_tag_dyneither(_tmpA9C,sizeof(char),51))),_tag_dyneither(_tmpA9B,sizeof(void*),0)));}{
struct Cyc_Parse_Declaration_spec _tmpA9D;yyval=Cyc_YY19(((_tmpA9D.sc=(enum Cyc_Parse_Storage_class*)
Cyc_yyget_YY22(yyyyvsp[0]),((_tmpA9D.tq=_tmp4EC.tq,((_tmpA9D.type_specs=_tmp4EC.type_specs,((
_tmpA9D.is_inline=_tmp4EC.is_inline,((_tmpA9D.attributes=_tmp4EC.attributes,
_tmpA9D)))))))))));}break;}case 43: _LL248: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Parse_Declaration_spec
_tmpA9E;yyval=Cyc_YY19(((_tmpA9E.sc=0,((_tmpA9E.tq=Cyc_Absyn_empty_tqual((
unsigned int)(yyyylsp[0]).first_line),((_tmpA9E.type_specs=Cyc_yyget_YY23(
yyyyvsp[0]),((_tmpA9E.is_inline=0,((_tmpA9E.attributes=0,_tmpA9E)))))))))));}
break;}case 44: _LL249: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_Parse_Declaration_spec
_tmp4F1=Cyc_yyget_YY19(yyyyvsp[1]);{struct Cyc_Parse_Declaration_spec _tmpA9F;
yyval=Cyc_YY19(((_tmpA9F.sc=_tmp4F1.sc,((_tmpA9F.tq=_tmp4F1.tq,((_tmpA9F.type_specs=
Cyc_Parse_combine_specifiers((unsigned int)(yyyylsp[0]).first_line,_tmp4F1.type_specs,
Cyc_yyget_YY23(yyyyvsp[0])),((_tmpA9F.is_inline=_tmp4F1.is_inline,((_tmpA9F.attributes=
_tmp4F1.attributes,_tmpA9F)))))))))));}break;}case 45: _LL24A: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct
Cyc_Parse_Declaration_spec _tmpAA0;yyval=Cyc_YY19(((_tmpAA0.sc=0,((_tmpAA0.tq=Cyc_yyget_YY25(
yyyyvsp[0]),((_tmpAA0.type_specs=Cyc_Parse_empty_spec(0),((_tmpAA0.is_inline=0,((
_tmpAA0.attributes=0,_tmpAA0)))))))))));}break;}case 46: _LL24B: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct
Cyc_Parse_Declaration_spec _tmp4F4=Cyc_yyget_YY19(yyyyvsp[1]);{struct Cyc_Parse_Declaration_spec
_tmpAA1;yyval=Cyc_YY19(((_tmpAA1.sc=_tmp4F4.sc,((_tmpAA1.tq=Cyc_Absyn_combine_tqual(
Cyc_yyget_YY25(yyyyvsp[0]),_tmp4F4.tq),((_tmpAA1.type_specs=_tmp4F4.type_specs,((
_tmpAA1.is_inline=_tmp4F4.is_inline,((_tmpAA1.attributes=_tmp4F4.attributes,
_tmpAA1)))))))))));}break;}case 47: _LL24C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Parse_Declaration_spec
_tmpAA2;yyval=Cyc_YY19(((_tmpAA2.sc=0,((_tmpAA2.tq=Cyc_Absyn_empty_tqual((
unsigned int)(yyyylsp[0]).first_line),((_tmpAA2.type_specs=Cyc_Parse_empty_spec(
0),((_tmpAA2.is_inline=1,((_tmpAA2.attributes=0,_tmpAA2)))))))))));}break;}case
48: _LL24D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_Parse_Declaration_spec
_tmp4F7=Cyc_yyget_YY19(yyyyvsp[1]);{struct Cyc_Parse_Declaration_spec _tmpAA3;
yyval=Cyc_YY19(((_tmpAA3.sc=_tmp4F7.sc,((_tmpAA3.tq=_tmp4F7.tq,((_tmpAA3.type_specs=
_tmp4F7.type_specs,((_tmpAA3.is_inline=1,((_tmpAA3.attributes=_tmp4F7.attributes,
_tmpAA3)))))))))));}break;}case 49: _LL24E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Parse_Declaration_spec
_tmpAA4;yyval=Cyc_YY19(((_tmpAA4.sc=0,((_tmpAA4.tq=Cyc_Absyn_empty_tqual((
unsigned int)(yyyylsp[0]).first_line),((_tmpAA4.type_specs=Cyc_Parse_empty_spec(
0),((_tmpAA4.is_inline=0,((_tmpAA4.attributes=Cyc_yyget_YY45(yyyyvsp[0]),_tmpAA4)))))))))));}
break;}case 50: _LL24F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_Parse_Declaration_spec
_tmp4FA=Cyc_yyget_YY19(yyyyvsp[1]);{struct Cyc_Parse_Declaration_spec _tmpAA5;
yyval=Cyc_YY19(((_tmpAA5.sc=_tmp4FA.sc,((_tmpAA5.tq=_tmp4FA.tq,((_tmpAA5.type_specs=
_tmp4FA.type_specs,((_tmpAA5.is_inline=_tmp4FA.is_inline,((_tmpAA5.attributes=((
struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(
Cyc_yyget_YY45(yyyyvsp[0]),_tmp4FA.attributes),_tmpAA5)))))))))));}break;}case 51:
_LL250: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);static enum Cyc_Parse_Storage_class
auto_sc=Cyc_Parse_Auto_sc;yyval=Cyc_YY22(& auto_sc);break;}case 52: _LL251: {union
Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(
yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);
struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(
struct Cyc_Yyltype),1);static enum Cyc_Parse_Storage_class register_sc=Cyc_Parse_Register_sc;
yyval=Cyc_YY22(& register_sc);break;}case 53: _LL252: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);static enum Cyc_Parse_Storage_class
static_sc=Cyc_Parse_Static_sc;yyval=Cyc_YY22(& static_sc);break;}case 54: _LL253: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);static enum Cyc_Parse_Storage_class
extern_sc=Cyc_Parse_Extern_sc;yyval=Cyc_YY22(& extern_sc);break;}case 55: _LL254: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);static enum Cyc_Parse_Storage_class
externC_sc=Cyc_Parse_ExternC_sc;{const char*_tmpAA6;if(Cyc_strcmp((struct
_dyneither_ptr)Cyc_yyget_String_tok(yyyyvsp[1]),((_tmpAA6="C",_tag_dyneither(
_tmpAA6,sizeof(char),2))))!= 0){const char*_tmpAA7;Cyc_Parse_err(((_tmpAA7="only extern or extern \"C\" is allowed",
_tag_dyneither(_tmpAA7,sizeof(char),37))),(unsigned int)(yyyylsp[0]).first_line);}}
yyval=Cyc_YY22(& externC_sc);break;}case 56: _LL255: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);static enum Cyc_Parse_Storage_class
typedef_sc=Cyc_Parse_Typedef_sc;yyval=Cyc_YY22(& typedef_sc);break;}case 57: _LL256: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);static enum Cyc_Parse_Storage_class
abstract_sc=Cyc_Parse_Abstract_sc;yyval=Cyc_YY22(& abstract_sc);break;}case 58:
_LL257: yyval=Cyc_YY45(0);break;case 59: _LL258: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 60:
_LL259: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 5),sizeof(union
Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);yyval=yyyyvsp[3];break;}case 61:
_LL25A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_List_List*_tmpAA8;yyval=
Cyc_YY45(((_tmpAA8=_cycalloc(sizeof(*_tmpAA8)),((_tmpAA8->hd=Cyc_yyget_YY46(
yyyyvsp[0]),((_tmpAA8->tl=0,_tmpAA8)))))));}break;}case 62: _LL25B: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct
Cyc_List_List*_tmpAA9;yyval=Cyc_YY45(((_tmpAA9=_cycalloc(sizeof(*_tmpAA9)),((
_tmpAA9->hd=Cyc_yyget_YY46(yyyyvsp[0]),((_tmpAA9->tl=Cyc_yyget_YY45(yyyyvsp[2]),
_tmpAA9)))))));}break;}case 63: _LL25C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);static struct Cyc_Absyn_Aligned_att_struct
att_aligned={6,- 1};static struct _tuple27 att_map[17]={{{_tmp502,_tmp502,_tmp502 + 8},(
void*)& Cyc_Absyn_Stdcall_att_val},{{_tmp503,_tmp503,_tmp503 + 6},(void*)& Cyc_Absyn_Cdecl_att_val},{{
_tmp504,_tmp504,_tmp504 + 9},(void*)& Cyc_Absyn_Fastcall_att_val},{{_tmp505,
_tmp505,_tmp505 + 9},(void*)& Cyc_Absyn_Noreturn_att_val},{{_tmp506,_tmp506,
_tmp506 + 6},(void*)& Cyc_Absyn_Const_att_val},{{_tmp507,_tmp507,_tmp507 + 8},(void*)&
att_aligned},{{_tmp508,_tmp508,_tmp508 + 7},(void*)& Cyc_Absyn_Packed_att_val},{{
_tmp509,_tmp509,_tmp509 + 7},(void*)& Cyc_Absyn_Shared_att_val},{{_tmp50A,_tmp50A,
_tmp50A + 7},(void*)& Cyc_Absyn_Unused_att_val},{{_tmp50B,_tmp50B,_tmp50B + 5},(
void*)& Cyc_Absyn_Weak_att_val},{{_tmp50C,_tmp50C,_tmp50C + 10},(void*)& Cyc_Absyn_Dllimport_att_val},{{
_tmp50D,_tmp50D,_tmp50D + 10},(void*)& Cyc_Absyn_Dllexport_att_val},{{_tmp50E,
_tmp50E,_tmp50E + 23},(void*)& Cyc_Absyn_No_instrument_function_att_val},{{_tmp50F,
_tmp50F,_tmp50F + 12},(void*)& Cyc_Absyn_Constructor_att_val},{{_tmp510,_tmp510,
_tmp510 + 11},(void*)& Cyc_Absyn_Destructor_att_val},{{_tmp511,_tmp511,_tmp511 + 22},(
void*)& Cyc_Absyn_No_check_memory_usage_att_val},{{_tmp512,_tmp512,_tmp512 + 5},(
void*)& Cyc_Absyn_Pure_att_val}};struct _dyneither_ptr _tmp500=Cyc_yyget_String_tok(
yyyyvsp[0]);if((((_get_dyneither_size(_tmp500,sizeof(char))> 4  && *((const char*)
_check_dyneither_subscript(_tmp500,sizeof(char),0))== '_') && *((const char*)
_check_dyneither_subscript(_tmp500,sizeof(char),1))== '_') && *((const char*)
_check_dyneither_subscript(_tmp500,sizeof(char),(int)(_get_dyneither_size(
_tmp500,sizeof(char))- 2)))== '_') && *((const char*)_check_dyneither_subscript(
_tmp500,sizeof(char),(int)(_get_dyneither_size(_tmp500,sizeof(char))- 3)))== '_')
_tmp500=(struct _dyneither_ptr)Cyc_substring((struct _dyneither_ptr)_tmp500,2,
_get_dyneither_size(_tmp500,sizeof(char))- 5);{int i=0;for(0;i < 17;++ i){if(Cyc_strcmp((
struct _dyneither_ptr)_tmp500,(struct _dyneither_ptr)(att_map[i]).f1)== 0){yyval=
Cyc_YY46((att_map[i]).f2);break;}}if(i == 17){{const char*_tmpAAA;Cyc_Parse_err(((
_tmpAAA="unrecognized attribute",_tag_dyneither(_tmpAAA,sizeof(char),23))),(
unsigned int)(yyyylsp[0]).first_line);}yyval=Cyc_YY46((void*)& Cyc_Absyn_Cdecl_att_val);}
break;};}case 64: _LL25D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY46((void*)& Cyc_Absyn_Const_att_val);
break;}case 65: _LL25E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct _dyneither_ptr _tmp514=Cyc_yyget_String_tok(
yyyyvsp[0]);int _tmp516;struct _tuple5 _tmp515=Cyc_yyget_Int_tok(yyyyvsp[2]);
_tmp516=_tmp515.f2;{void*a;{const char*_tmpAAC;const char*_tmpAAB;if(Cyc_zstrcmp((
struct _dyneither_ptr)_tmp514,((_tmpAAC="regparm",_tag_dyneither(_tmpAAC,sizeof(
char),8))))== 0  || Cyc_zstrcmp((struct _dyneither_ptr)_tmp514,((_tmpAAB="__regparm__",
_tag_dyneither(_tmpAAB,sizeof(char),12))))== 0){if(_tmp516 < 0  || _tmp516 > 3){
const char*_tmpAAD;Cyc_Parse_err(((_tmpAAD="regparm requires value between 0 and 3",
_tag_dyneither(_tmpAAD,sizeof(char),39))),(unsigned int)(yyyylsp[2]).first_line);}{
struct Cyc_Absyn_Regparm_att_struct _tmpAB0;struct Cyc_Absyn_Regparm_att_struct*
_tmpAAF;a=(void*)((_tmpAAF=_cycalloc_atomic(sizeof(*_tmpAAF)),((_tmpAAF[0]=((
_tmpAB0.tag=0,((_tmpAB0.f1=_tmp516,_tmpAB0)))),_tmpAAF))));};}else{const char*
_tmpAB2;const char*_tmpAB1;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp514,((_tmpAB2="aligned",
_tag_dyneither(_tmpAB2,sizeof(char),8))))== 0  || Cyc_zstrcmp((struct
_dyneither_ptr)_tmp514,((_tmpAB1="__aligned__",_tag_dyneither(_tmpAB1,sizeof(
char),12))))== 0){if(_tmp516 < 0){const char*_tmpAB3;Cyc_Parse_err(((_tmpAB3="aligned requires positive power of two",
_tag_dyneither(_tmpAB3,sizeof(char),39))),(unsigned int)(yyyylsp[2]).first_line);}{
unsigned int j=(unsigned int)_tmp516;for(0;(j & 1)== 0;j=j >> 1){;}j=j >> 1;if(j != 0){
const char*_tmpAB4;Cyc_Parse_err(((_tmpAB4="aligned requires positive power of two",
_tag_dyneither(_tmpAB4,sizeof(char),39))),(unsigned int)(yyyylsp[2]).first_line);}{
struct Cyc_Absyn_Aligned_att_struct _tmpAB7;struct Cyc_Absyn_Aligned_att_struct*
_tmpAB6;a=(void*)((_tmpAB6=_cycalloc_atomic(sizeof(*_tmpAB6)),((_tmpAB6[0]=((
_tmpAB7.tag=6,((_tmpAB7.f1=_tmp516,_tmpAB7)))),_tmpAB6))));};};}else{const char*
_tmpAB9;const char*_tmpAB8;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp514,((_tmpAB9="initializes",
_tag_dyneither(_tmpAB9,sizeof(char),12))))== 0  || Cyc_zstrcmp((struct
_dyneither_ptr)_tmp514,((_tmpAB8="__initializes__",_tag_dyneither(_tmpAB8,
sizeof(char),16))))== 0){struct Cyc_Absyn_Initializes_att_struct _tmpABC;struct Cyc_Absyn_Initializes_att_struct*
_tmpABB;a=(void*)((_tmpABB=_cycalloc_atomic(sizeof(*_tmpABB)),((_tmpABB[0]=((
_tmpABC.tag=20,((_tmpABC.f1=_tmp516,_tmpABC)))),_tmpABB))));}else{const char*
_tmpABE;const char*_tmpABD;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp514,((_tmpABE="noliveunique",
_tag_dyneither(_tmpABE,sizeof(char),13))))== 0  || Cyc_zstrcmp((struct
_dyneither_ptr)_tmp514,((_tmpABD="__noliveunique__",_tag_dyneither(_tmpABD,
sizeof(char),17))))== 0){struct Cyc_Absyn_Noliveunique_att_struct _tmpAC1;struct
Cyc_Absyn_Noliveunique_att_struct*_tmpAC0;a=(void*)((_tmpAC0=_cycalloc_atomic(
sizeof(*_tmpAC0)),((_tmpAC0[0]=((_tmpAC1.tag=21,((_tmpAC1.f1=_tmp516,_tmpAC1)))),
_tmpAC0))));}else{{const char*_tmpAC2;Cyc_Parse_err(((_tmpAC2="unrecognized attribute",
_tag_dyneither(_tmpAC2,sizeof(char),23))),(unsigned int)(yyyylsp[0]).first_line);}
a=(void*)& Cyc_Absyn_Cdecl_att_val;}}}}}yyval=Cyc_YY46(a);break;};}case 66: _LL25F: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct _dyneither_ptr _tmp52B=Cyc_yyget_String_tok(
yyyyvsp[0]);struct _dyneither_ptr _tmp52C=Cyc_yyget_String_tok(yyyyvsp[2]);void*a;{
const char*_tmpAC4;const char*_tmpAC3;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp52B,((
_tmpAC4="section",_tag_dyneither(_tmpAC4,sizeof(char),8))))== 0  || Cyc_zstrcmp((
struct _dyneither_ptr)_tmp52B,((_tmpAC3="__section__",_tag_dyneither(_tmpAC3,
sizeof(char),12))))== 0){struct Cyc_Absyn_Section_att_struct _tmpAC7;struct Cyc_Absyn_Section_att_struct*
_tmpAC6;a=(void*)((_tmpAC6=_cycalloc(sizeof(*_tmpAC6)),((_tmpAC6[0]=((_tmpAC7.tag=
8,((_tmpAC7.f1=_tmp52C,_tmpAC7)))),_tmpAC6))));}else{{const char*_tmpAC8;Cyc_Parse_err(((
_tmpAC8="unrecognized attribute",_tag_dyneither(_tmpAC8,sizeof(char),23))),(
unsigned int)(yyyylsp[0]).first_line);}a=(void*)& Cyc_Absyn_Cdecl_att_val;}}yyval=
Cyc_YY46(a);break;}case 67: _LL260: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct _dyneither_ptr _tmp532=Cyc_yyget_String_tok(
yyyyvsp[0]);struct _dyneither_ptr _tmp533=Cyc_yyget_String_tok(yyyyvsp[2]);void*a;{
const char*_tmpAC9;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp532,((_tmpAC9="__mode__",
_tag_dyneither(_tmpAC9,sizeof(char),9))))== 0){struct Cyc_Absyn_Mode_att_struct
_tmpACC;struct Cyc_Absyn_Mode_att_struct*_tmpACB;a=(void*)((_tmpACB=_cycalloc(
sizeof(*_tmpACB)),((_tmpACB[0]=((_tmpACC.tag=23,((_tmpACC.f1=_tmp533,_tmpACC)))),
_tmpACB))));}else{{const char*_tmpACD;Cyc_Parse_err(((_tmpACD="unrecognized attribute",
_tag_dyneither(_tmpACD,sizeof(char),23))),(unsigned int)(yyyylsp[0]).first_line);}
a=(void*)& Cyc_Absyn_Cdecl_att_val;}}yyval=Cyc_YY46(a);break;}case 68: _LL261: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 7),sizeof(union
Cyc_YYSTYPE),8);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 7),sizeof(struct Cyc_Yyltype),8);struct _dyneither_ptr _tmp538=Cyc_yyget_String_tok(
yyyyvsp[0]);struct _dyneither_ptr _tmp539=Cyc_yyget_String_tok(yyyyvsp[2]);int
_tmp53B;struct _tuple5 _tmp53A=Cyc_yyget_Int_tok(yyyyvsp[4]);_tmp53B=_tmp53A.f2;{
int _tmp53D;struct _tuple5 _tmp53C=Cyc_yyget_Int_tok(yyyyvsp[6]);_tmp53D=_tmp53C.f2;{
void*a=(void*)& Cyc_Absyn_Cdecl_att_val;{const char*_tmpACF;const char*_tmpACE;if(
Cyc_zstrcmp((struct _dyneither_ptr)_tmp538,((_tmpACF="format",_tag_dyneither(
_tmpACF,sizeof(char),7))))== 0  || Cyc_zstrcmp((struct _dyneither_ptr)_tmp538,((
_tmpACE="__format__",_tag_dyneither(_tmpACE,sizeof(char),11))))== 0){const char*
_tmpAD0;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp539,((_tmpAD0="printf",
_tag_dyneither(_tmpAD0,sizeof(char),7))))== 0){struct Cyc_Absyn_Format_att_struct
_tmpAD3;struct Cyc_Absyn_Format_att_struct*_tmpAD2;a=(void*)((_tmpAD2=
_cycalloc_atomic(sizeof(*_tmpAD2)),((_tmpAD2[0]=((_tmpAD3.tag=19,((_tmpAD3.f1=
Cyc_Absyn_Printf_ft,((_tmpAD3.f2=_tmp53B,((_tmpAD3.f3=_tmp53D,_tmpAD3)))))))),
_tmpAD2))));}else{const char*_tmpAD4;if(Cyc_zstrcmp((struct _dyneither_ptr)_tmp539,((
_tmpAD4="scanf",_tag_dyneither(_tmpAD4,sizeof(char),6))))== 0){struct Cyc_Absyn_Format_att_struct
_tmpAD7;struct Cyc_Absyn_Format_att_struct*_tmpAD6;a=(void*)((_tmpAD6=
_cycalloc_atomic(sizeof(*_tmpAD6)),((_tmpAD6[0]=((_tmpAD7.tag=19,((_tmpAD7.f1=
Cyc_Absyn_Scanf_ft,((_tmpAD7.f2=_tmp53B,((_tmpAD7.f3=_tmp53D,_tmpAD7)))))))),
_tmpAD6))));}else{const char*_tmpAD8;Cyc_Parse_err(((_tmpAD8="unrecognized format type",
_tag_dyneither(_tmpAD8,sizeof(char),25))),(unsigned int)(yyyylsp[2]).first_line);}}}
else{const char*_tmpAD9;Cyc_Parse_err(((_tmpAD9="unrecognized attribute",
_tag_dyneither(_tmpAD9,sizeof(char),23))),(unsigned int)(yyyylsp[0]).first_line);}}
yyval=Cyc_YY46(a);break;};};}case 69: _LL262: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 70:
_LL263: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Absyn_TypedefType_struct
_tmpADC;struct Cyc_Absyn_TypedefType_struct*_tmpADB;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpADB=_cycalloc(sizeof(*_tmpADB)),((_tmpADB[0]=((_tmpADC.tag=17,((
_tmpADC.f1=Cyc_yyget_QualId_tok(yyyyvsp[0]),((_tmpADC.f2=Cyc_yyget_YY40(yyyyvsp[
1]),((_tmpADC.f3=0,((_tmpADC.f4=(void*)0,_tmpADC)))))))))),_tmpADB)))),(
unsigned int)(yyyylsp[0]).first_line));}break;}case 71: _LL264: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
Cyc_YY23(Cyc_Parse_type_spec((void*)& Cyc_Absyn_VoidType_val,(unsigned int)(
yyyylsp[0]).first_line));break;}case 72: _LL265: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY23(Cyc_Parse_type_spec(
Cyc_Absyn_new_evar(0,0),(unsigned int)(yyyylsp[0]).first_line));break;}case 73:
_LL266: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY23(Cyc_Parse_type_spec(
Cyc_Absyn_new_evar(Cyc_Tcutil_kind_to_opt(Cyc_yyget_YY43(yyyyvsp[2])),0),(
unsigned int)(yyyylsp[0]).first_line));break;}case 74: _LL267: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
Cyc_YY23(Cyc_Parse_type_spec(Cyc_Absyn_char_typ,(unsigned int)(yyyylsp[0]).first_line));
break;}case 75: _LL268: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY23(Cyc_Parse_short_spec((
unsigned int)(yyyylsp[0]).first_line));break;}case 76: _LL269: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
Cyc_YY23(Cyc_Parse_type_spec(Cyc_Absyn_sint_typ,(unsigned int)(yyyylsp[0]).first_line));
break;}case 77: _LL26A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY23(Cyc_Parse_long_spec((
unsigned int)(yyyylsp[0]).first_line));break;}case 78: _LL26B: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
Cyc_YY23(Cyc_Parse_type_spec(Cyc_Absyn_float_typ(0),(unsigned int)(yyyylsp[0]).first_line));
break;}case 79: _LL26C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY23(Cyc_Parse_type_spec(
Cyc_Absyn_float_typ(1),(unsigned int)(yyyylsp[0]).first_line));break;}case 80:
_LL26D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY23(Cyc_Parse_signed_spec((
unsigned int)(yyyylsp[0]).first_line));break;}case 81: _LL26E: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
Cyc_YY23(Cyc_Parse_unsigned_spec((unsigned int)(yyyylsp[0]).first_line));break;}
case 82: _LL26F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 83:
_LL270: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 84:
_LL271: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 85:
_LL272: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY23(Cyc_Parse_type_spec(
Cyc_yyget_YY44(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}case 86:
_LL273: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_TupleType_struct
_tmpADF;struct Cyc_Absyn_TupleType_struct*_tmpADE;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpADE=_cycalloc(sizeof(*_tmpADE)),((_tmpADE[0]=((_tmpADF.tag=10,((
_tmpADF.f1=((struct Cyc_List_List*(*)(struct _tuple19*(*f)(unsigned int,struct
_tuple8*),unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_get_tqual_typ,(
unsigned int)(yyyylsp[2]).first_line,((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_imp_rev)(Cyc_yyget_YY38(yyyyvsp[2]))),_tmpADF)))),_tmpADE)))),(
unsigned int)(yyyylsp[0]).first_line));}break;}case 87: _LL274: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct
Cyc_Absyn_RgnHandleType_struct _tmpAE2;struct Cyc_Absyn_RgnHandleType_struct*
_tmpAE1;yyval=Cyc_YY23(Cyc_Parse_type_spec((void*)((_tmpAE1=_cycalloc(sizeof(*
_tmpAE1)),((_tmpAE1[0]=((_tmpAE2.tag=15,((_tmpAE2.f1=(void*)Cyc_yyget_YY44(
yyyyvsp[2]),_tmpAE2)))),_tmpAE1)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 88: _LL275: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_RgnHandleType_struct
_tmpAE5;struct Cyc_Absyn_RgnHandleType_struct*_tmpAE4;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpAE4=_cycalloc(sizeof(*_tmpAE4)),((_tmpAE4[0]=((_tmpAE5.tag=15,((
_tmpAE5.f1=(void*)Cyc_Absyn_new_evar((struct Cyc_Core_Opt*)& Cyc_Tcutil_rko,0),
_tmpAE5)))),_tmpAE4)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 89:
_LL276: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);void*_tmp550=Cyc_Absyn_new_evar((
struct Cyc_Core_Opt*)& Cyc_Tcutil_rko,0);{struct Cyc_Absyn_DynRgnType_struct _tmpAE8;
struct Cyc_Absyn_DynRgnType_struct*_tmpAE7;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpAE7=_cycalloc(sizeof(*_tmpAE7)),((_tmpAE7[0]=((_tmpAE8.tag=16,((
_tmpAE8.f1=(void*)Cyc_yyget_YY44(yyyyvsp[2]),((_tmpAE8.f2=(void*)_tmp550,_tmpAE8)))))),
_tmpAE7)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 90: _LL277: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 5),sizeof(union
Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);{struct Cyc_Absyn_DynRgnType_struct
_tmpAEB;struct Cyc_Absyn_DynRgnType_struct*_tmpAEA;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpAEA=_cycalloc(sizeof(*_tmpAEA)),((_tmpAEA[0]=((_tmpAEB.tag=16,((
_tmpAEB.f1=(void*)Cyc_yyget_YY44(yyyyvsp[2]),((_tmpAEB.f2=(void*)Cyc_yyget_YY44(
yyyyvsp[4]),_tmpAEB)))))),_tmpAEA)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 91: _LL278: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_TagType_struct
_tmpAEE;struct Cyc_Absyn_TagType_struct*_tmpAED;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpAED=_cycalloc(sizeof(*_tmpAED)),((_tmpAED[0]=((_tmpAEE.tag=19,((
_tmpAEE.f1=(void*)Cyc_yyget_YY44(yyyyvsp[2]),_tmpAEE)))),_tmpAED)))),(
unsigned int)(yyyylsp[0]).first_line));}break;}case 92: _LL279: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct
Cyc_Absyn_TagType_struct _tmpAF1;struct Cyc_Absyn_TagType_struct*_tmpAF0;yyval=Cyc_YY23(
Cyc_Parse_type_spec((void*)((_tmpAF0=_cycalloc(sizeof(*_tmpAF0)),((_tmpAF0[0]=((
_tmpAF1.tag=19,((_tmpAF1.f1=(void*)Cyc_Absyn_new_evar((struct Cyc_Core_Opt*)& Cyc_Tcutil_iko,
0),_tmpAF1)))),_tmpAF0)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case
93: _LL27A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_ValueofType_struct
_tmpAF4;struct Cyc_Absyn_ValueofType_struct*_tmpAF3;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpAF3=_cycalloc(sizeof(*_tmpAF3)),((_tmpAF3[0]=((_tmpAF4.tag=18,((
_tmpAF4.f1=Cyc_yyget_YY3(yyyyvsp[2]),_tmpAF4)))),_tmpAF3)))),(unsigned int)(
yyyylsp[0]).first_line));}break;}case 94: _LL27B: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY43(Cyc_Parse_id_to_kind(
Cyc_yyget_String_tok(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}
case 95: _LL27C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);unsigned int loc=(unsigned int)(Cyc_Absyn_porting_c_code?(
yyyylsp[0]).first_line:(int)0);{struct Cyc_Absyn_Tqual _tmpAF5;yyval=Cyc_YY25(((
_tmpAF5.print_const=1,((_tmpAF5.q_volatile=0,((_tmpAF5.q_restrict=0,((_tmpAF5.real_const=
1,((_tmpAF5.loc=loc,_tmpAF5)))))))))));}break;}case 96: _LL27D: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct
Cyc_Absyn_Tqual _tmpAF6;yyval=Cyc_YY25(((_tmpAF6.print_const=0,((_tmpAF6.q_volatile=
1,((_tmpAF6.q_restrict=0,((_tmpAF6.real_const=0,((_tmpAF6.loc=0,_tmpAF6)))))))))));}
break;}case 97: _LL27E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Tqual _tmpAF7;
yyval=Cyc_YY25(((_tmpAF7.print_const=0,((_tmpAF7.q_volatile=0,((_tmpAF7.q_restrict=
1,((_tmpAF7.real_const=0,((_tmpAF7.loc=0,_tmpAF7)))))))))));}break;}case 98:
_LL27F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct Cyc_Absyn_Enum_td_struct*
_tmpB06;struct Cyc_Absyn_Enumdecl*_tmpB05;struct Cyc_Core_Opt*_tmpB04;struct Cyc_Absyn_Enum_td_struct
_tmpB03;struct Cyc_Absyn_TypeDecl*_tmpB02;struct Cyc_Absyn_TypeDecl*_tmp55E=(
_tmpB02=_cycalloc(sizeof(*_tmpB02)),((_tmpB02->r=(void*)((_tmpB06=_cycalloc(
sizeof(*_tmpB06)),((_tmpB06[0]=((_tmpB03.tag=1,((_tmpB03.f1=((_tmpB05=_cycalloc(
sizeof(*_tmpB05)),((_tmpB05->sc=Cyc_Absyn_Public,((_tmpB05->name=Cyc_yyget_QualId_tok(
yyyyvsp[1]),((_tmpB05->fields=((_tmpB04=_cycalloc(sizeof(*_tmpB04)),((_tmpB04->v=
Cyc_yyget_YY48(yyyyvsp[3]),_tmpB04)))),_tmpB05)))))))),_tmpB03)))),_tmpB06)))),((
_tmpB02->loc=(unsigned int)(yyyylsp[0]).first_line,_tmpB02)))));{struct Cyc_Absyn_TypeDeclType_struct
_tmpB09;struct Cyc_Absyn_TypeDeclType_struct*_tmpB08;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpB08=_cycalloc(sizeof(*_tmpB08)),((_tmpB08[0]=((_tmpB09.tag=26,((
_tmpB09.f1=_tmp55E,((_tmpB09.f2=0,_tmpB09)))))),_tmpB08)))),(unsigned int)(
yyyylsp[0]).first_line));}break;}case 99: _LL280: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Absyn_EnumType_struct
_tmpB0C;struct Cyc_Absyn_EnumType_struct*_tmpB0B;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpB0B=_cycalloc(sizeof(*_tmpB0B)),((_tmpB0B[0]=((_tmpB0C.tag=13,((
_tmpB0C.f1=Cyc_yyget_QualId_tok(yyyyvsp[1]),((_tmpB0C.f2=0,_tmpB0C)))))),_tmpB0B)))),(
unsigned int)(yyyylsp[0]).first_line));}break;}case 100: _LL281: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct
Cyc_Absyn_AnonEnumType_struct _tmpB0F;struct Cyc_Absyn_AnonEnumType_struct*_tmpB0E;
yyval=Cyc_YY23(Cyc_Parse_type_spec((void*)((_tmpB0E=_cycalloc(sizeof(*_tmpB0E)),((
_tmpB0E[0]=((_tmpB0F.tag=14,((_tmpB0F.f1=Cyc_yyget_YY48(yyyyvsp[2]),_tmpB0F)))),
_tmpB0E)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 101: _LL282: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Enumfield*_tmpB10;
yyval=Cyc_YY47(((_tmpB10=_cycalloc(sizeof(*_tmpB10)),((_tmpB10->name=Cyc_yyget_QualId_tok(
yyyyvsp[0]),((_tmpB10->tag=0,((_tmpB10->loc=(unsigned int)(yyyylsp[0]).first_line,
_tmpB10)))))))));}break;}case 102: _LL283: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_Enumfield*_tmpB11;
yyval=Cyc_YY47(((_tmpB11=_cycalloc(sizeof(*_tmpB11)),((_tmpB11->name=Cyc_yyget_QualId_tok(
yyyyvsp[0]),((_tmpB11->tag=(struct Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[2]),((
_tmpB11->loc=(unsigned int)(yyyylsp[0]).first_line,_tmpB11)))))))));}break;}case
103: _LL284: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_List_List*_tmpB12;yyval=
Cyc_YY48(((_tmpB12=_cycalloc(sizeof(*_tmpB12)),((_tmpB12->hd=Cyc_yyget_YY47(
yyyyvsp[0]),((_tmpB12->tl=0,_tmpB12)))))));}break;}case 104: _LL285: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct
Cyc_List_List*_tmpB13;yyval=Cyc_YY48(((_tmpB13=_cycalloc(sizeof(*_tmpB13)),((
_tmpB13->hd=Cyc_yyget_YY47(yyyyvsp[0]),((_tmpB13->tl=0,_tmpB13)))))));}break;}
case 105: _LL286: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_List_List*_tmpB14;yyval=
Cyc_YY48(((_tmpB14=_cycalloc(sizeof(*_tmpB14)),((_tmpB14->hd=Cyc_yyget_YY47(
yyyyvsp[0]),((_tmpB14->tl=Cyc_yyget_YY48(yyyyvsp[2]),_tmpB14)))))));}break;}case
106: _LL287: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_AnonAggrType_struct
_tmpB17;struct Cyc_Absyn_AnonAggrType_struct*_tmpB16;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpB16=_cycalloc(sizeof(*_tmpB16)),((_tmpB16[0]=((_tmpB17.tag=12,((
_tmpB17.f1=Cyc_yyget_YY24(yyyyvsp[0]),((_tmpB17.f2=Cyc_yyget_YY26(yyyyvsp[2]),
_tmpB17)))))),_tmpB16)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case
107: _LL288: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 8),sizeof(union
Cyc_YYSTYPE),9);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 8),sizeof(struct Cyc_Yyltype),9);struct Cyc_List_List*_tmp571=((
struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*f)(unsigned int,void*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,(
unsigned int)(yyyylsp[3]).first_line,Cyc_yyget_YY40(yyyyvsp[3]));struct Cyc_List_List*
_tmp572=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*f)(unsigned int,void*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,(
unsigned int)(yyyylsp[5]).first_line,Cyc_yyget_YY40(yyyyvsp[5]));struct Cyc_Absyn_TypeDecl*
_tmp573=Cyc_Absyn_aggr_tdecl(Cyc_yyget_YY24(yyyyvsp[1]),Cyc_Absyn_Public,Cyc_yyget_QualId_tok(
yyyyvsp[2]),_tmp571,(struct Cyc_Absyn_AggrdeclImpl*)Cyc_Absyn_aggrdecl_impl(
_tmp572,Cyc_yyget_YY50(yyyyvsp[6]),Cyc_yyget_YY26(yyyyvsp[7]),1),0,(unsigned int)(
yyyylsp[0]).first_line);{struct Cyc_Absyn_TypeDeclType_struct _tmpB1A;struct Cyc_Absyn_TypeDeclType_struct*
_tmpB19;yyval=Cyc_YY23(Cyc_Parse_type_spec((void*)((_tmpB19=_cycalloc(sizeof(*
_tmpB19)),((_tmpB19[0]=((_tmpB1A.tag=26,((_tmpB1A.f1=_tmp573,((_tmpB1A.f2=0,
_tmpB1A)))))),_tmpB19)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case
108: _LL289: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 7),sizeof(union
Cyc_YYSTYPE),8);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 7),sizeof(struct Cyc_Yyltype),8);struct Cyc_List_List*_tmp576=((
struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*f)(unsigned int,void*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,(
unsigned int)(yyyylsp[2]).first_line,Cyc_yyget_YY40(yyyyvsp[2]));struct Cyc_List_List*
_tmp577=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*f)(unsigned int,void*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,(
unsigned int)(yyyylsp[4]).first_line,Cyc_yyget_YY40(yyyyvsp[4]));struct Cyc_Absyn_TypeDecl*
_tmp578=Cyc_Absyn_aggr_tdecl(Cyc_yyget_YY24(yyyyvsp[0]),Cyc_Absyn_Public,Cyc_yyget_QualId_tok(
yyyyvsp[1]),_tmp576,(struct Cyc_Absyn_AggrdeclImpl*)Cyc_Absyn_aggrdecl_impl(
_tmp577,Cyc_yyget_YY50(yyyyvsp[5]),Cyc_yyget_YY26(yyyyvsp[6]),0),0,(unsigned int)(
yyyylsp[0]).first_line);{struct Cyc_Absyn_TypeDeclType_struct _tmpB1D;struct Cyc_Absyn_TypeDeclType_struct*
_tmpB1C;yyval=Cyc_YY23(Cyc_Parse_type_spec((void*)((_tmpB1C=_cycalloc(sizeof(*
_tmpB1C)),((_tmpB1C[0]=((_tmpB1D.tag=26,((_tmpB1D.f1=_tmp578,((_tmpB1D.f2=0,
_tmpB1D)))))),_tmpB1C)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case
109: _LL28A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_AggrType_struct
_tmpB27;struct Cyc_Core_Opt*_tmpB26;struct Cyc_Absyn_AggrInfo _tmpB25;struct Cyc_Absyn_AggrType_struct*
_tmpB24;yyval=Cyc_YY23(Cyc_Parse_type_spec((void*)((_tmpB24=_cycalloc(sizeof(*
_tmpB24)),((_tmpB24[0]=((_tmpB27.tag=11,((_tmpB27.f1=((_tmpB25.aggr_info=Cyc_Absyn_UnknownAggr(
Cyc_yyget_YY24(yyyyvsp[1]),Cyc_yyget_QualId_tok(yyyyvsp[2]),((_tmpB26=
_cycalloc_atomic(sizeof(*_tmpB26)),((_tmpB26->v=(void*)1,_tmpB26))))),((_tmpB25.targs=
Cyc_yyget_YY40(yyyyvsp[3]),_tmpB25)))),_tmpB27)))),_tmpB24)))),(unsigned int)(
yyyylsp[0]).first_line));}break;}case 110: _LL28B: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_AggrType_struct
_tmpB2D;struct Cyc_Absyn_AggrInfo _tmpB2C;struct Cyc_Absyn_AggrType_struct*_tmpB2B;
yyval=Cyc_YY23(Cyc_Parse_type_spec((void*)((_tmpB2B=_cycalloc(sizeof(*_tmpB2B)),((
_tmpB2B[0]=((_tmpB2D.tag=11,((_tmpB2D.f1=((_tmpB2C.aggr_info=Cyc_Absyn_UnknownAggr(
Cyc_yyget_YY24(yyyyvsp[0]),Cyc_yyget_QualId_tok(yyyyvsp[1]),0),((_tmpB2C.targs=
Cyc_yyget_YY40(yyyyvsp[2]),_tmpB2C)))),_tmpB2D)))),_tmpB2B)))),(unsigned int)(
yyyylsp[0]).first_line));}break;}case 111: _LL28C: yyval=Cyc_YY40(0);break;case 112:
_LL28D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY40(((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_yyget_YY40(yyyyvsp[1])));break;}
case 113: _LL28E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY24(Cyc_Absyn_StructA);
break;}case 114: _LL28F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY24(Cyc_Absyn_UnionA);
break;}case 115: _LL290: yyval=Cyc_YY26(0);break;case 116: _LL291: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);struct
Cyc_List_List*decls=0;{struct Cyc_List_List*_tmp582=Cyc_yyget_YY27(yyyyvsp[0]);
for(0;_tmp582 != 0;_tmp582=_tmp582->tl){decls=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_imp_append)((struct Cyc_List_List*)_tmp582->hd,
decls);}}{struct Cyc_List_List*_tmp583=Cyc_Parse_get_argrfield_tags(decls);if(
_tmp583 != 0)((void(*)(void(*f)(struct Cyc_List_List*,struct Cyc_Absyn_Aggrfield*),
struct Cyc_List_List*env,struct Cyc_List_List*x))Cyc_List_iter_c)(Cyc_Parse_substitute_aggrfield_tags,
_tmp583,decls);yyval=Cyc_YY26(decls);break;};}case 117: _LL292: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct
Cyc_List_List*_tmpB2E;yyval=Cyc_YY27(((_tmpB2E=_cycalloc(sizeof(*_tmpB2E)),((
_tmpB2E->hd=Cyc_yyget_YY26(yyyyvsp[0]),((_tmpB2E->tl=0,_tmpB2E)))))));}break;}
case 118: _LL293: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_List_List*_tmpB2F;yyval=
Cyc_YY27(((_tmpB2F=_cycalloc(sizeof(*_tmpB2F)),((_tmpB2F->hd=Cyc_yyget_YY26(
yyyyvsp[1]),((_tmpB2F->tl=Cyc_yyget_YY27(yyyyvsp[0]),_tmpB2F)))))));}break;}case
119: _LL294: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY21(((struct _tuple13*(*)(
struct _tuple13*x))Cyc_Parse_flat_imp_rev)(Cyc_yyget_YY21(yyyyvsp[0])));break;}
case 120: _LL295: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple13*_tmpB30;yyval=Cyc_YY21(((
_tmpB30=_region_malloc(yyr,sizeof(*_tmpB30)),((_tmpB30->tl=0,((_tmpB30->hd=Cyc_yyget_YY20(
yyyyvsp[0]),_tmpB30)))))));}break;}case 121: _LL296: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _tuple13*_tmpB31;yyval=Cyc_YY21(((
_tmpB31=_region_malloc(yyr,sizeof(*_tmpB31)),((_tmpB31->tl=Cyc_yyget_YY21(
yyyyvsp[0]),((_tmpB31->hd=Cyc_yyget_YY20(yyyyvsp[2]),_tmpB31)))))));}break;}case
122: _LL297: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple12 _tmpB32;yyval=Cyc_YY20(((
_tmpB32.f1=Cyc_yyget_YY29(yyyyvsp[0]),((_tmpB32.f2=0,_tmpB32)))));}break;}case
123: _LL298: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _tuple12 _tmpB33;yyval=Cyc_YY20(((
_tmpB33.f1=Cyc_yyget_YY29(yyyyvsp[0]),((_tmpB33.f2=(struct Cyc_Absyn_Exp*)Cyc_yyget_YY3(
yyyyvsp[2]),_tmpB33)))));}break;}case 124: _LL299: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);struct _RegionHandle _tmp58A=
_new_region("temp");struct _RegionHandle*temp=& _tmp58A;_push_region(temp);{struct
Cyc_Absyn_Tqual _tmp58C;struct Cyc_Parse_Type_specifier _tmp58D;struct Cyc_List_List*
_tmp58E;struct _tuple24 _tmp58B=Cyc_yyget_YY35(yyyyvsp[0]);_tmp58C=_tmp58B.f1;
_tmp58D=_tmp58B.f2;_tmp58E=_tmp58B.f3;if(_tmp58C.loc == 0)_tmp58C.loc=(
unsigned int)(yyyylsp[0]).first_line;{struct _tuple11*decls=0;struct Cyc_List_List*
widths=0;Cyc_Parse_decl_split(temp,Cyc_yyget_YY21(yyyyvsp[1]),& decls,& widths);{
void*_tmp58F=Cyc_Parse_speclist2typ(_tmp58D,(unsigned int)(yyyylsp[0]).first_line);
struct Cyc_List_List*_tmp590=((struct Cyc_List_List*(*)(struct _RegionHandle*r1,
struct _RegionHandle*r2,struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_rzip)(
temp,temp,Cyc_Parse_apply_tmss(temp,_tmp58C,_tmp58F,decls,_tmp58E),widths);yyval=
Cyc_YY26(((struct Cyc_List_List*(*)(struct Cyc_Absyn_Aggrfield*(*f)(unsigned int,
struct _tuple17*),unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_make_aggr_field,(
unsigned int)(yyyylsp[0]).first_line,_tmp590));};};}_npop_handler(0);break;;
_pop_region(temp);}case 125: _LL29A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple24 _tmpB34;yyval=Cyc_YY35(((
_tmpB34.f1=Cyc_Absyn_empty_tqual((unsigned int)(yyyylsp[0]).first_line),((
_tmpB34.f2=Cyc_yyget_YY23(yyyyvsp[0]),((_tmpB34.f3=0,_tmpB34)))))));}break;}case
126: _LL29B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct _tuple24 _tmp592=Cyc_yyget_YY35(
yyyyvsp[1]);{struct _tuple24 _tmpB35;yyval=Cyc_YY35(((_tmpB35.f1=_tmp592.f1,((
_tmpB35.f2=Cyc_Parse_combine_specifiers((unsigned int)(yyyylsp[0]).first_line,
Cyc_yyget_YY23(yyyyvsp[0]),_tmp592.f2),((_tmpB35.f3=_tmp592.f3,_tmpB35)))))));}
break;}case 127: _LL29C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple24 _tmpB36;yyval=Cyc_YY35(((
_tmpB36.f1=Cyc_yyget_YY25(yyyyvsp[0]),((_tmpB36.f2=Cyc_Parse_empty_spec(0),((
_tmpB36.f3=0,_tmpB36)))))));}break;}case 128: _LL29D: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct _tuple24 _tmp595=Cyc_yyget_YY35(
yyyyvsp[1]);{struct _tuple24 _tmpB37;yyval=Cyc_YY35(((_tmpB37.f1=Cyc_Absyn_combine_tqual(
Cyc_yyget_YY25(yyyyvsp[0]),_tmp595.f1),((_tmpB37.f2=_tmp595.f2,((_tmpB37.f3=
_tmp595.f3,_tmpB37)))))));}break;}case 129: _LL29E: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple24 _tmpB38;yyval=Cyc_YY35(((
_tmpB38.f1=Cyc_Absyn_empty_tqual((unsigned int)(yyyylsp[0]).first_line),((
_tmpB38.f2=Cyc_Parse_empty_spec(0),((_tmpB38.f3=Cyc_yyget_YY45(yyyyvsp[0]),
_tmpB38)))))));}break;}case 130: _LL29F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct _tuple24 _tmp598=Cyc_yyget_YY35(
yyyyvsp[1]);{struct _tuple24 _tmpB39;yyval=Cyc_YY35(((_tmpB39.f1=_tmp598.f1,((
_tmpB39.f2=_tmp598.f2,((_tmpB39.f3=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_append)(Cyc_yyget_YY45(yyyyvsp[0]),_tmp598.f3),
_tmpB39)))))));}break;}case 131: _LL2A0: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple24 _tmpB3A;yyval=Cyc_YY35(((
_tmpB3A.f1=Cyc_Absyn_empty_tqual((unsigned int)(yyyylsp[0]).first_line),((
_tmpB3A.f2=Cyc_yyget_YY23(yyyyvsp[0]),((_tmpB3A.f3=0,_tmpB3A)))))));}break;}case
132: _LL2A1: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct _tuple24 _tmp59B=Cyc_yyget_YY35(
yyyyvsp[1]);{struct _tuple24 _tmpB3B;yyval=Cyc_YY35(((_tmpB3B.f1=_tmp59B.f1,((
_tmpB3B.f2=Cyc_Parse_combine_specifiers((unsigned int)(yyyylsp[0]).first_line,
Cyc_yyget_YY23(yyyyvsp[0]),_tmp59B.f2),((_tmpB3B.f3=_tmp59B.f3,_tmpB3B)))))));}
break;}case 133: _LL2A2: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple24 _tmpB3C;yyval=Cyc_YY35(((
_tmpB3C.f1=Cyc_yyget_YY25(yyyyvsp[0]),((_tmpB3C.f2=Cyc_Parse_empty_spec(0),((
_tmpB3C.f3=0,_tmpB3C)))))));}break;}case 134: _LL2A3: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct _tuple24 _tmp59E=Cyc_yyget_YY35(
yyyyvsp[1]);{struct _tuple24 _tmpB3D;yyval=Cyc_YY35(((_tmpB3D.f1=Cyc_Absyn_combine_tqual(
Cyc_yyget_YY25(yyyyvsp[0]),_tmp59E.f1),((_tmpB3D.f2=_tmp59E.f2,((_tmpB3D.f3=
_tmp59E.f3,_tmpB3D)))))));}break;}case 135: _LL2A4: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple24 _tmpB3E;yyval=Cyc_YY35(((
_tmpB3E.f1=Cyc_Absyn_empty_tqual((unsigned int)(yyyylsp[0]).first_line),((
_tmpB3E.f2=Cyc_Parse_empty_spec(0),((_tmpB3E.f3=Cyc_yyget_YY45(yyyyvsp[0]),
_tmpB3E)))))));}break;}case 136: _LL2A5: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct _tuple24 _tmp5A1=Cyc_yyget_YY35(
yyyyvsp[1]);{struct _tuple24 _tmpB3F;yyval=Cyc_YY35(((_tmpB3F.f1=_tmp5A1.f1,((
_tmpB3F.f2=_tmp5A1.f2,((_tmpB3F.f3=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_append)(Cyc_yyget_YY45(yyyyvsp[0]),_tmp5A1.f3),
_tmpB3F)))))));}break;}case 137: _LL2A6: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY21(((struct _tuple13*(*)(
struct _tuple13*x))Cyc_Parse_flat_imp_rev)(Cyc_yyget_YY21(yyyyvsp[0])));break;}
case 138: _LL2A7: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple13*_tmpB40;yyval=Cyc_YY21(((
_tmpB40=_region_malloc(yyr,sizeof(*_tmpB40)),((_tmpB40->tl=0,((_tmpB40->hd=Cyc_yyget_YY20(
yyyyvsp[0]),_tmpB40)))))));}break;}case 139: _LL2A8: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _tuple13*_tmpB41;yyval=Cyc_YY21(((
_tmpB41=_region_malloc(yyr,sizeof(*_tmpB41)),((_tmpB41->tl=Cyc_yyget_YY21(
yyyyvsp[0]),((_tmpB41->hd=Cyc_yyget_YY20(yyyyvsp[2]),_tmpB41)))))));}break;}case
140: _LL2A9: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple12 _tmpB42;yyval=Cyc_YY20(((
_tmpB42.f1=Cyc_yyget_YY29(yyyyvsp[0]),((_tmpB42.f2=0,_tmpB42)))));}break;}case
141: _LL2AA: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Parse_Declarator _tmpB48;
struct _tuple0*_tmpB47;struct _tuple12 _tmpB46;yyval=Cyc_YY20(((_tmpB46.f1=((
_tmpB48.id=((_tmpB47=_cycalloc(sizeof(*_tmpB47)),((_tmpB47->f1=Cyc_Absyn_Rel_n(0),((
_tmpB47->f2=_init_dyneither_ptr(_cycalloc(sizeof(struct _dyneither_ptr)),"",
sizeof(char),1),_tmpB47)))))),((_tmpB48.tms=0,_tmpB48)))),((_tmpB46.f2=(struct
Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[1]),_tmpB46)))));}break;}case 142: _LL2AB:{
struct Cyc_Parse_Declarator _tmpB4E;struct _tuple0*_tmpB4D;struct _tuple12 _tmpB4C;
yyval=Cyc_YY20(((_tmpB4C.f1=((_tmpB4E.id=((_tmpB4D=_cycalloc(sizeof(*_tmpB4D)),((
_tmpB4D->f1=Cyc_Absyn_Rel_n(0),((_tmpB4D->f2=_init_dyneither_ptr(_cycalloc(
sizeof(struct _dyneither_ptr)),"",sizeof(char),1),_tmpB4D)))))),((_tmpB4E.tms=0,
_tmpB4E)))),((_tmpB4C.f2=0,_tmpB4C)))));}break;case 143: _LL2AC: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct
_tuple12 _tmpB4F;yyval=Cyc_YY20(((_tmpB4F.f1=Cyc_yyget_YY29(yyyyvsp[0]),((_tmpB4F.f2=(
struct Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[2]),_tmpB4F)))));}break;}case 144:
_LL2AD: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 5),sizeof(union
Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);int _tmp5AF=Cyc_yyget_YY31(yyyyvsp[
0]);struct Cyc_List_List*_tmp5B0=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*
f)(unsigned int,void*),unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(
Cyc_Parse_typ2tvar,(unsigned int)(yyyylsp[2]).first_line,Cyc_yyget_YY40(yyyyvsp[
2]));struct Cyc_Core_Opt*_tmpB50;struct Cyc_Absyn_TypeDecl*_tmp5B1=Cyc_Absyn_datatype_tdecl(
Cyc_Absyn_Public,Cyc_yyget_QualId_tok(yyyyvsp[1]),_tmp5B0,((_tmpB50=_cycalloc(
sizeof(*_tmpB50)),((_tmpB50->v=Cyc_yyget_YY34(yyyyvsp[4]),_tmpB50)))),_tmp5AF,(
unsigned int)(yyyylsp[0]).first_line);{struct Cyc_Absyn_TypeDeclType_struct
_tmpB53;struct Cyc_Absyn_TypeDeclType_struct*_tmpB52;yyval=Cyc_YY23(Cyc_Parse_type_spec((
void*)((_tmpB52=_cycalloc(sizeof(*_tmpB52)),((_tmpB52[0]=((_tmpB53.tag=26,((
_tmpB53.f1=_tmp5B1,((_tmpB53.f2=0,_tmpB53)))))),_tmpB52)))),(unsigned int)(
yyyylsp[0]).first_line));}break;}case 145: _LL2AE: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);int _tmp5B5=Cyc_yyget_YY31(yyyyvsp[
0]);{struct Cyc_Absyn_DatatypeType_struct _tmpB5D;struct Cyc_Absyn_UnknownDatatypeInfo
_tmpB5C;struct Cyc_Absyn_DatatypeInfo _tmpB5B;struct Cyc_Absyn_DatatypeType_struct*
_tmpB5A;yyval=Cyc_YY23(Cyc_Parse_type_spec((void*)((_tmpB5A=_cycalloc(sizeof(*
_tmpB5A)),((_tmpB5A[0]=((_tmpB5D.tag=3,((_tmpB5D.f1=((_tmpB5B.datatype_info=Cyc_Absyn_UnknownDatatype(((
_tmpB5C.name=Cyc_yyget_QualId_tok(yyyyvsp[1]),((_tmpB5C.is_extensible=_tmp5B5,
_tmpB5C))))),((_tmpB5B.targs=Cyc_yyget_YY40(yyyyvsp[2]),_tmpB5B)))),_tmpB5D)))),
_tmpB5A)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 146: _LL2AF: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);int _tmp5BA=Cyc_yyget_YY31(yyyyvsp[
0]);{struct Cyc_Absyn_DatatypeFieldType_struct _tmpB67;struct Cyc_Absyn_UnknownDatatypeFieldInfo
_tmpB66;struct Cyc_Absyn_DatatypeFieldInfo _tmpB65;struct Cyc_Absyn_DatatypeFieldType_struct*
_tmpB64;yyval=Cyc_YY23(Cyc_Parse_type_spec((void*)((_tmpB64=_cycalloc(sizeof(*
_tmpB64)),((_tmpB64[0]=((_tmpB67.tag=4,((_tmpB67.f1=((_tmpB65.field_info=Cyc_Absyn_UnknownDatatypefield(((
_tmpB66.datatype_name=Cyc_yyget_QualId_tok(yyyyvsp[1]),((_tmpB66.field_name=Cyc_yyget_QualId_tok(
yyyyvsp[3]),((_tmpB66.is_extensible=_tmp5BA,_tmpB66))))))),((_tmpB65.targs=Cyc_yyget_YY40(
yyyyvsp[4]),_tmpB65)))),_tmpB67)))),_tmpB64)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 147: _LL2B0: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY31(0);break;}case 148:
_LL2B1: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY31(1);break;}case 149:
_LL2B2: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_List_List*_tmpB68;yyval=
Cyc_YY34(((_tmpB68=_cycalloc(sizeof(*_tmpB68)),((_tmpB68->hd=Cyc_yyget_YY33(
yyyyvsp[0]),((_tmpB68->tl=0,_tmpB68)))))));}break;}case 150: _LL2B3: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct
Cyc_List_List*_tmpB69;yyval=Cyc_YY34(((_tmpB69=_cycalloc(sizeof(*_tmpB69)),((
_tmpB69->hd=Cyc_yyget_YY33(yyyyvsp[0]),((_tmpB69->tl=0,_tmpB69)))))));}break;}
case 151: _LL2B4: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_List_List*_tmpB6A;yyval=
Cyc_YY34(((_tmpB6A=_cycalloc(sizeof(*_tmpB6A)),((_tmpB6A->hd=Cyc_yyget_YY33(
yyyyvsp[0]),((_tmpB6A->tl=Cyc_yyget_YY34(yyyyvsp[2]),_tmpB6A)))))));}break;}case
152: _LL2B5: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_List_List*_tmpB6B;yyval=
Cyc_YY34(((_tmpB6B=_cycalloc(sizeof(*_tmpB6B)),((_tmpB6B->hd=Cyc_yyget_YY33(
yyyyvsp[0]),((_tmpB6B->tl=Cyc_yyget_YY34(yyyyvsp[2]),_tmpB6B)))))));}break;}case
153: _LL2B6: yyval=Cyc_YY32(Cyc_Absyn_Public);break;case 154: _LL2B7: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
Cyc_YY32(Cyc_Absyn_Extern);break;}case 155: _LL2B8: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY32(Cyc_Absyn_Static);
break;}case 156: _LL2B9: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Absyn_Datatypefield*
_tmpB6C;yyval=Cyc_YY33(((_tmpB6C=_cycalloc(sizeof(*_tmpB6C)),((_tmpB6C->name=Cyc_yyget_QualId_tok(
yyyyvsp[1]),((_tmpB6C->typs=0,((_tmpB6C->loc=(unsigned int)(yyyylsp[0]).first_line,((
_tmpB6C->sc=Cyc_yyget_YY32(yyyyvsp[0]),_tmpB6C)))))))))));}break;}case 157: _LL2BA: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct Cyc_List_List*_tmp5C4=((
struct Cyc_List_List*(*)(struct _tuple19*(*f)(unsigned int,struct _tuple8*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_get_tqual_typ,(
unsigned int)(yyyylsp[3]).first_line,((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_imp_rev)(Cyc_yyget_YY38(yyyyvsp[3])));{struct Cyc_Absyn_Datatypefield*
_tmpB6D;yyval=Cyc_YY33(((_tmpB6D=_cycalloc(sizeof(*_tmpB6D)),((_tmpB6D->name=Cyc_yyget_QualId_tok(
yyyyvsp[1]),((_tmpB6D->typs=_tmp5C4,((_tmpB6D->loc=(unsigned int)(yyyylsp[0]).first_line,((
_tmpB6D->sc=Cyc_yyget_YY32(yyyyvsp[0]),_tmpB6D)))))))))));}break;}case 158: _LL2BB: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 159:
_LL2BC: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_Parse_Declarator _tmp5C6=
Cyc_yyget_YY29(yyyyvsp[1]);{struct Cyc_Parse_Declarator _tmpB6E;yyval=Cyc_YY29(((
_tmpB6E.id=_tmp5C6.id,((_tmpB6E.tms=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_imp_append)(Cyc_yyget_YY28(yyyyvsp[0]),_tmp5C6.tms),
_tmpB6E)))));}break;}case 160: _LL2BD: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 161:
_LL2BE: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_Parse_Declarator _tmp5C8=
Cyc_yyget_YY29(yyyyvsp[1]);{struct Cyc_Parse_Declarator _tmpB6F;yyval=Cyc_YY29(((
_tmpB6F.id=_tmp5C8.id,((_tmpB6F.tms=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_imp_append)(Cyc_yyget_YY28(yyyyvsp[0]),_tmp5C8.tms),
_tmpB6F)))));}break;}case 162: _LL2BF: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Parse_Declarator _tmpB70;
yyval=Cyc_YY29(((_tmpB70.id=Cyc_yyget_QualId_tok(yyyyvsp[0]),((_tmpB70.tms=0,
_tmpB70)))));}break;}case 163: _LL2C0: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=yyyyvsp[1];break;}case 164:
_LL2C1: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_Parse_Declarator _tmp5CB=
Cyc_yyget_YY29(yyyyvsp[2]);{struct Cyc_Absyn_Attributes_mod_struct*_tmpB76;struct
Cyc_Absyn_Attributes_mod_struct _tmpB75;struct Cyc_List_List*_tmpB74;_tmp5CB.tms=((
_tmpB74=_region_malloc(yyr,sizeof(*_tmpB74)),((_tmpB74->hd=(void*)((_tmpB76=
_region_malloc(yyr,sizeof(*_tmpB76)),((_tmpB76[0]=((_tmpB75.tag=5,((_tmpB75.f1=(
unsigned int)(yyyylsp[1]).first_line,((_tmpB75.f2=Cyc_yyget_YY45(yyyyvsp[1]),
_tmpB75)))))),_tmpB76)))),((_tmpB74->tl=_tmp5CB.tms,_tmpB74))))));}yyval=yyyyvsp[
2];break;}case 165: _LL2C2: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_List_List*_tmpB80;
struct Cyc_Absyn_Carray_mod_struct _tmpB7F;struct Cyc_Absyn_Carray_mod_struct*
_tmpB7E;struct Cyc_Parse_Declarator _tmpB7D;yyval=Cyc_YY29(((_tmpB7D.id=(Cyc_yyget_YY29(
yyyyvsp[0])).id,((_tmpB7D.tms=((_tmpB80=_region_malloc(yyr,sizeof(*_tmpB80)),((
_tmpB80->hd=(void*)((_tmpB7E=_region_malloc(yyr,sizeof(*_tmpB7E)),((_tmpB7E[0]=((
_tmpB7F.tag=0,((_tmpB7F.f1=Cyc_yyget_YY51(yyyyvsp[3]),((_tmpB7F.f2=(unsigned int)(
yyyylsp[3]).first_line,_tmpB7F)))))),_tmpB7E)))),((_tmpB80->tl=(Cyc_yyget_YY29(
yyyyvsp[0])).tms,_tmpB80)))))),_tmpB7D)))));}break;}case 166: _LL2C3: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct
Cyc_List_List*_tmpB8A;struct Cyc_Absyn_ConstArray_mod_struct _tmpB89;struct Cyc_Absyn_ConstArray_mod_struct*
_tmpB88;struct Cyc_Parse_Declarator _tmpB87;yyval=Cyc_YY29(((_tmpB87.id=(Cyc_yyget_YY29(
yyyyvsp[0])).id,((_tmpB87.tms=((_tmpB8A=_region_malloc(yyr,sizeof(*_tmpB8A)),((
_tmpB8A->hd=(void*)((_tmpB88=_region_malloc(yyr,sizeof(*_tmpB88)),((_tmpB88[0]=((
_tmpB89.tag=1,((_tmpB89.f1=Cyc_yyget_YY3(yyyyvsp[2]),((_tmpB89.f2=Cyc_yyget_YY51(
yyyyvsp[4]),((_tmpB89.f3=(unsigned int)(yyyylsp[4]).first_line,_tmpB89)))))))),
_tmpB88)))),((_tmpB8A->tl=(Cyc_yyget_YY29(yyyyvsp[0])).tms,_tmpB8A)))))),_tmpB87)))));}
break;}case 167: _LL2C4: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct _tuple25 _tmp5D8;struct Cyc_List_List*
_tmp5D9;int _tmp5DA;struct Cyc_Absyn_VarargInfo*_tmp5DB;void*_tmp5DC;struct Cyc_List_List*
_tmp5DD;struct _tuple25*_tmp5D7=Cyc_yyget_YY39(yyyyvsp[2]);_tmp5D8=*_tmp5D7;
_tmp5D9=_tmp5D8.f1;_tmp5DA=_tmp5D8.f2;_tmp5DB=_tmp5D8.f3;_tmp5DC=_tmp5D8.f4;
_tmp5DD=_tmp5D8.f5;{struct Cyc_List_List*_tmpB9F;struct Cyc_Absyn_Function_mod_struct
_tmpB9E;struct Cyc_Absyn_WithTypes_struct _tmpB9D;struct Cyc_Absyn_WithTypes_struct*
_tmpB9C;struct Cyc_Absyn_Function_mod_struct*_tmpB9B;struct Cyc_Parse_Declarator
_tmpB9A;yyval=Cyc_YY29(((_tmpB9A.id=(Cyc_yyget_YY29(yyyyvsp[0])).id,((_tmpB9A.tms=((
_tmpB9F=_region_malloc(yyr,sizeof(*_tmpB9F)),((_tmpB9F->hd=(void*)((_tmpB9B=
_region_malloc(yyr,sizeof(*_tmpB9B)),((_tmpB9B[0]=((_tmpB9E.tag=3,((_tmpB9E.f1=(
void*)((void*)((_tmpB9C=_region_malloc(yyr,sizeof(*_tmpB9C)),((_tmpB9C[0]=((
_tmpB9D.tag=1,((_tmpB9D.f1=_tmp5D9,((_tmpB9D.f2=_tmp5DA,((_tmpB9D.f3=_tmp5DB,((
_tmpB9D.f4=(void*)_tmp5DC,((_tmpB9D.f5=_tmp5DD,_tmpB9D)))))))))))),_tmpB9C))))),
_tmpB9E)))),_tmpB9B)))),((_tmpB9F->tl=(Cyc_yyget_YY29(yyyyvsp[0])).tms,_tmpB9F)))))),
_tmpB9A)))));}break;}case 168: _LL2C5: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct Cyc_List_List*_tmpBB4;
struct Cyc_Absyn_Function_mod_struct _tmpBB3;struct Cyc_Absyn_WithTypes_struct
_tmpBB2;struct Cyc_Absyn_WithTypes_struct*_tmpBB1;struct Cyc_Absyn_Function_mod_struct*
_tmpBB0;struct Cyc_Parse_Declarator _tmpBAF;yyval=Cyc_YY29(((_tmpBAF.id=(Cyc_yyget_YY29(
yyyyvsp[0])).id,((_tmpBAF.tms=((_tmpBB4=_region_malloc(yyr,sizeof(*_tmpBB4)),((
_tmpBB4->hd=(void*)((_tmpBB0=_region_malloc(yyr,sizeof(*_tmpBB0)),((_tmpBB0[0]=((
_tmpBB3.tag=3,((_tmpBB3.f1=(void*)((void*)((_tmpBB1=_region_malloc(yyr,sizeof(*
_tmpBB1)),((_tmpBB1[0]=((_tmpBB2.tag=1,((_tmpBB2.f1=0,((_tmpBB2.f2=0,((_tmpBB2.f3=
0,((_tmpBB2.f4=(void*)Cyc_yyget_YY49(yyyyvsp[2]),((_tmpBB2.f5=Cyc_yyget_YY50(
yyyyvsp[3]),_tmpBB2)))))))))))),_tmpBB1))))),_tmpBB3)))),_tmpBB0)))),((_tmpBB4->tl=(
Cyc_yyget_YY29(yyyyvsp[0])).tms,_tmpBB4)))))),_tmpBAF)))));}break;}case 169:
_LL2C6: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_List_List*_tmpBC9;
struct Cyc_Absyn_Function_mod_struct _tmpBC8;struct Cyc_Absyn_NoTypes_struct _tmpBC7;
struct Cyc_Absyn_NoTypes_struct*_tmpBC6;struct Cyc_Absyn_Function_mod_struct*
_tmpBC5;struct Cyc_Parse_Declarator _tmpBC4;yyval=Cyc_YY29(((_tmpBC4.id=(Cyc_yyget_YY29(
yyyyvsp[0])).id,((_tmpBC4.tms=((_tmpBC9=_region_malloc(yyr,sizeof(*_tmpBC9)),((
_tmpBC9->hd=(void*)((_tmpBC5=_region_malloc(yyr,sizeof(*_tmpBC5)),((_tmpBC5[0]=((
_tmpBC8.tag=3,((_tmpBC8.f1=(void*)((void*)((_tmpBC6=_region_malloc(yyr,sizeof(*
_tmpBC6)),((_tmpBC6[0]=((_tmpBC7.tag=0,((_tmpBC7.f1=Cyc_yyget_YY36(yyyyvsp[2]),((
_tmpBC7.f2=(unsigned int)(yyyylsp[0]).first_line,_tmpBC7)))))),_tmpBC6))))),
_tmpBC8)))),_tmpBC5)))),((_tmpBC9->tl=(Cyc_yyget_YY29(yyyyvsp[0])).tms,_tmpBC9)))))),
_tmpBC4)))));}break;}case 170: _LL2C7: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_List_List*_tmp5F0=((
struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*f)(unsigned int,void*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,(
unsigned int)(yyyylsp[1]).first_line,((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_imp_rev)(Cyc_yyget_YY40(yyyyvsp[2])));{struct Cyc_List_List*_tmpBD3;
struct Cyc_Absyn_TypeParams_mod_struct _tmpBD2;struct Cyc_Absyn_TypeParams_mod_struct*
_tmpBD1;struct Cyc_Parse_Declarator _tmpBD0;yyval=Cyc_YY29(((_tmpBD0.id=(Cyc_yyget_YY29(
yyyyvsp[0])).id,((_tmpBD0.tms=((_tmpBD3=_region_malloc(yyr,sizeof(*_tmpBD3)),((
_tmpBD3->hd=(void*)((_tmpBD1=_region_malloc(yyr,sizeof(*_tmpBD1)),((_tmpBD1[0]=((
_tmpBD2.tag=4,((_tmpBD2.f1=_tmp5F0,((_tmpBD2.f2=(unsigned int)(yyyylsp[0]).first_line,((
_tmpBD2.f3=0,_tmpBD2)))))))),_tmpBD1)))),((_tmpBD3->tl=(Cyc_yyget_YY29(yyyyvsp[0])).tms,
_tmpBD3)))))),_tmpBD0)))));}break;}case 171: _LL2C8: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_List_List*_tmpBDD;
struct Cyc_Absyn_Attributes_mod_struct _tmpBDC;struct Cyc_Absyn_Attributes_mod_struct*
_tmpBDB;struct Cyc_Parse_Declarator _tmpBDA;yyval=Cyc_YY29(((_tmpBDA.id=(Cyc_yyget_YY29(
yyyyvsp[0])).id,((_tmpBDA.tms=((_tmpBDD=_region_malloc(yyr,sizeof(*_tmpBDD)),((
_tmpBDD->hd=(void*)((_tmpBDB=_region_malloc(yyr,sizeof(*_tmpBDB)),((_tmpBDB[0]=((
_tmpBDC.tag=5,((_tmpBDC.f1=(unsigned int)(yyyylsp[1]).first_line,((_tmpBDC.f2=
Cyc_yyget_YY45(yyyyvsp[1]),_tmpBDC)))))),_tmpBDB)))),((_tmpBDD->tl=(Cyc_yyget_YY29(
yyyyvsp[0])).tms,_tmpBDD)))))),_tmpBDA)))));}break;}case 172: _LL2C9: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct
Cyc_Parse_Declarator _tmpBDE;yyval=Cyc_YY29(((_tmpBDE.id=Cyc_yyget_QualId_tok(
yyyyvsp[0]),((_tmpBDE.tms=0,_tmpBDE)))));}break;}case 173: _LL2CA: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct
Cyc_Parse_Declarator _tmpBDF;yyval=Cyc_YY29(((_tmpBDF.id=Cyc_yyget_QualId_tok(
yyyyvsp[0]),((_tmpBDF.tms=0,_tmpBDF)))));}break;}case 174: _LL2CB: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=
yyyyvsp[1];break;}case 175: _LL2CC: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_Parse_Declarator _tmp5FB=
Cyc_yyget_YY29(yyyyvsp[2]);{struct Cyc_Absyn_Attributes_mod_struct*_tmpBE5;struct
Cyc_Absyn_Attributes_mod_struct _tmpBE4;struct Cyc_List_List*_tmpBE3;_tmp5FB.tms=((
_tmpBE3=_region_malloc(yyr,sizeof(*_tmpBE3)),((_tmpBE3->hd=(void*)((_tmpBE5=
_region_malloc(yyr,sizeof(*_tmpBE5)),((_tmpBE5[0]=((_tmpBE4.tag=5,((_tmpBE4.f1=(
unsigned int)(yyyylsp[1]).first_line,((_tmpBE4.f2=Cyc_yyget_YY45(yyyyvsp[1]),
_tmpBE4)))))),_tmpBE5)))),((_tmpBE3->tl=_tmp5FB.tms,_tmpBE3))))));}yyval=yyyyvsp[
2];break;}case 176: _LL2CD: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_Parse_Declarator _tmp5FF=
Cyc_yyget_YY29(yyyyvsp[0]);{struct Cyc_List_List*_tmpBEF;struct Cyc_Absyn_Carray_mod_struct
_tmpBEE;struct Cyc_Absyn_Carray_mod_struct*_tmpBED;struct Cyc_Parse_Declarator
_tmpBEC;yyval=Cyc_YY29(((_tmpBEC.id=_tmp5FF.id,((_tmpBEC.tms=((_tmpBEF=
_region_malloc(yyr,sizeof(*_tmpBEF)),((_tmpBEF->hd=(void*)((_tmpBED=
_region_malloc(yyr,sizeof(*_tmpBED)),((_tmpBED[0]=((_tmpBEE.tag=0,((_tmpBEE.f1=
Cyc_yyget_YY51(yyyyvsp[3]),((_tmpBEE.f2=(unsigned int)(yyyylsp[3]).first_line,
_tmpBEE)))))),_tmpBED)))),((_tmpBEF->tl=_tmp5FF.tms,_tmpBEF)))))),_tmpBEC)))));}
break;}case 177: _LL2CE: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct Cyc_Parse_Declarator _tmp604=
Cyc_yyget_YY29(yyyyvsp[0]);{struct Cyc_List_List*_tmpBF9;struct Cyc_Absyn_ConstArray_mod_struct
_tmpBF8;struct Cyc_Absyn_ConstArray_mod_struct*_tmpBF7;struct Cyc_Parse_Declarator
_tmpBF6;yyval=Cyc_YY29(((_tmpBF6.id=_tmp604.id,((_tmpBF6.tms=((_tmpBF9=
_region_malloc(yyr,sizeof(*_tmpBF9)),((_tmpBF9->hd=(void*)((_tmpBF7=
_region_malloc(yyr,sizeof(*_tmpBF7)),((_tmpBF7[0]=((_tmpBF8.tag=1,((_tmpBF8.f1=
Cyc_yyget_YY3(yyyyvsp[2]),((_tmpBF8.f2=Cyc_yyget_YY51(yyyyvsp[4]),((_tmpBF8.f3=(
unsigned int)(yyyylsp[4]).first_line,_tmpBF8)))))))),_tmpBF7)))),((_tmpBF9->tl=
_tmp604.tms,_tmpBF9)))))),_tmpBF6)))));}break;}case 178: _LL2CF: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct
_tuple25 _tmp60A;struct Cyc_List_List*_tmp60B;int _tmp60C;struct Cyc_Absyn_VarargInfo*
_tmp60D;void*_tmp60E;struct Cyc_List_List*_tmp60F;struct _tuple25*_tmp609=Cyc_yyget_YY39(
yyyyvsp[2]);_tmp60A=*_tmp609;_tmp60B=_tmp60A.f1;_tmp60C=_tmp60A.f2;_tmp60D=
_tmp60A.f3;_tmp60E=_tmp60A.f4;_tmp60F=_tmp60A.f5;{struct Cyc_Parse_Declarator
_tmp610=Cyc_yyget_YY29(yyyyvsp[0]);{struct Cyc_List_List*_tmpC0E;struct Cyc_Absyn_Function_mod_struct
_tmpC0D;struct Cyc_Absyn_WithTypes_struct _tmpC0C;struct Cyc_Absyn_WithTypes_struct*
_tmpC0B;struct Cyc_Absyn_Function_mod_struct*_tmpC0A;struct Cyc_Parse_Declarator
_tmpC09;yyval=Cyc_YY29(((_tmpC09.id=_tmp610.id,((_tmpC09.tms=((_tmpC0E=
_region_malloc(yyr,sizeof(*_tmpC0E)),((_tmpC0E->hd=(void*)((_tmpC0A=
_region_malloc(yyr,sizeof(*_tmpC0A)),((_tmpC0A[0]=((_tmpC0D.tag=3,((_tmpC0D.f1=(
void*)((void*)((_tmpC0B=_region_malloc(yyr,sizeof(*_tmpC0B)),((_tmpC0B[0]=((
_tmpC0C.tag=1,((_tmpC0C.f1=_tmp60B,((_tmpC0C.f2=_tmp60C,((_tmpC0C.f3=_tmp60D,((
_tmpC0C.f4=(void*)_tmp60E,((_tmpC0C.f5=_tmp60F,_tmpC0C)))))))))))),_tmpC0B))))),
_tmpC0D)))),_tmpC0A)))),((_tmpC0E->tl=_tmp610.tms,_tmpC0E)))))),_tmpC09)))));}
break;};}case 179: _LL2D0: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct Cyc_Parse_Declarator _tmp617=
Cyc_yyget_YY29(yyyyvsp[0]);{struct Cyc_List_List*_tmpC23;struct Cyc_Absyn_Function_mod_struct
_tmpC22;struct Cyc_Absyn_WithTypes_struct _tmpC21;struct Cyc_Absyn_WithTypes_struct*
_tmpC20;struct Cyc_Absyn_Function_mod_struct*_tmpC1F;struct Cyc_Parse_Declarator
_tmpC1E;yyval=Cyc_YY29(((_tmpC1E.id=_tmp617.id,((_tmpC1E.tms=((_tmpC23=
_region_malloc(yyr,sizeof(*_tmpC23)),((_tmpC23->hd=(void*)((_tmpC1F=
_region_malloc(yyr,sizeof(*_tmpC1F)),((_tmpC1F[0]=((_tmpC22.tag=3,((_tmpC22.f1=(
void*)((void*)((_tmpC20=_region_malloc(yyr,sizeof(*_tmpC20)),((_tmpC20[0]=((
_tmpC21.tag=1,((_tmpC21.f1=0,((_tmpC21.f2=0,((_tmpC21.f3=0,((_tmpC21.f4=(void*)
Cyc_yyget_YY49(yyyyvsp[2]),((_tmpC21.f5=Cyc_yyget_YY50(yyyyvsp[3]),_tmpC21)))))))))))),
_tmpC20))))),_tmpC22)))),_tmpC1F)))),((_tmpC23->tl=_tmp617.tms,_tmpC23)))))),
_tmpC1E)))));}break;}case 180: _LL2D1: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_Parse_Declarator _tmp61E=
Cyc_yyget_YY29(yyyyvsp[0]);{struct Cyc_List_List*_tmpC38;struct Cyc_Absyn_Function_mod_struct
_tmpC37;struct Cyc_Absyn_NoTypes_struct _tmpC36;struct Cyc_Absyn_NoTypes_struct*
_tmpC35;struct Cyc_Absyn_Function_mod_struct*_tmpC34;struct Cyc_Parse_Declarator
_tmpC33;yyval=Cyc_YY29(((_tmpC33.id=_tmp61E.id,((_tmpC33.tms=((_tmpC38=
_region_malloc(yyr,sizeof(*_tmpC38)),((_tmpC38->hd=(void*)((_tmpC34=
_region_malloc(yyr,sizeof(*_tmpC34)),((_tmpC34[0]=((_tmpC37.tag=3,((_tmpC37.f1=(
void*)((void*)((_tmpC35=_region_malloc(yyr,sizeof(*_tmpC35)),((_tmpC35[0]=((
_tmpC36.tag=0,((_tmpC36.f1=Cyc_yyget_YY36(yyyyvsp[2]),((_tmpC36.f2=(unsigned int)(
yyyylsp[0]).first_line,_tmpC36)))))),_tmpC35))))),_tmpC37)))),_tmpC34)))),((
_tmpC38->tl=_tmp61E.tms,_tmpC38)))))),_tmpC33)))));}break;}case 181: _LL2D2: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_List_List*_tmp625=((
struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*f)(unsigned int,void*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,(
unsigned int)(yyyylsp[1]).first_line,((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_imp_rev)(Cyc_yyget_YY40(yyyyvsp[2])));struct Cyc_Parse_Declarator
_tmp626=Cyc_yyget_YY29(yyyyvsp[0]);{struct Cyc_List_List*_tmpC42;struct Cyc_Absyn_TypeParams_mod_struct
_tmpC41;struct Cyc_Absyn_TypeParams_mod_struct*_tmpC40;struct Cyc_Parse_Declarator
_tmpC3F;yyval=Cyc_YY29(((_tmpC3F.id=_tmp626.id,((_tmpC3F.tms=((_tmpC42=
_region_malloc(yyr,sizeof(*_tmpC42)),((_tmpC42->hd=(void*)((_tmpC40=
_region_malloc(yyr,sizeof(*_tmpC40)),((_tmpC40[0]=((_tmpC41.tag=4,((_tmpC41.f1=
_tmp625,((_tmpC41.f2=(unsigned int)(yyyylsp[0]).first_line,((_tmpC41.f3=0,
_tmpC41)))))))),_tmpC40)))),((_tmpC42->tl=_tmp626.tms,_tmpC42)))))),_tmpC3F)))));}
break;}case 182: _LL2D3: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_Parse_Declarator _tmp62B=
Cyc_yyget_YY29(yyyyvsp[0]);{struct Cyc_List_List*_tmpC4C;struct Cyc_Absyn_Attributes_mod_struct
_tmpC4B;struct Cyc_Absyn_Attributes_mod_struct*_tmpC4A;struct Cyc_Parse_Declarator
_tmpC49;yyval=Cyc_YY29(((_tmpC49.id=_tmp62B.id,((_tmpC49.tms=((_tmpC4C=
_region_malloc(yyr,sizeof(*_tmpC4C)),((_tmpC4C->hd=(void*)((_tmpC4A=
_region_malloc(yyr,sizeof(*_tmpC4A)),((_tmpC4A[0]=((_tmpC4B.tag=5,((_tmpC4B.f1=(
unsigned int)(yyyylsp[1]).first_line,((_tmpC4B.f2=Cyc_yyget_YY45(yyyyvsp[1]),
_tmpC4B)))))),_tmpC4A)))),((_tmpC4C->tl=_tmp62B.tms,_tmpC4C)))))),_tmpC49)))));}
break;}case 183: _LL2D4: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 184:
_LL2D5: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY28(((struct Cyc_List_List*(*)(
struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(Cyc_yyget_YY28(
yyyyvsp[0]),Cyc_yyget_YY28(yyyyvsp[1])));break;}case 185: _LL2D6: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct
Cyc_List_List*ans=0;if(Cyc_yyget_YY45(yyyyvsp[3])!= 0){struct Cyc_Absyn_Attributes_mod_struct*
_tmpC52;struct Cyc_Absyn_Attributes_mod_struct _tmpC51;struct Cyc_List_List*_tmpC50;
ans=((_tmpC50=_region_malloc(yyr,sizeof(*_tmpC50)),((_tmpC50->hd=(void*)((
_tmpC52=_region_malloc(yyr,sizeof(*_tmpC52)),((_tmpC52[0]=((_tmpC51.tag=5,((
_tmpC51.f1=(unsigned int)(yyyylsp[3]).first_line,((_tmpC51.f2=Cyc_yyget_YY45(
yyyyvsp[3]),_tmpC51)))))),_tmpC52)))),((_tmpC50->tl=ans,_tmpC50))))));}{struct
Cyc_Absyn_PtrLoc*ptrloc=0;unsigned int _tmp634;union Cyc_Absyn_Constraint*_tmp635;
union Cyc_Absyn_Constraint*_tmp636;struct _tuple21 _tmp633=*Cyc_yyget_YY1(yyyyvsp[0]);
_tmp634=_tmp633.f1;_tmp635=_tmp633.f2;_tmp636=_tmp633.f3;if(Cyc_Absyn_porting_c_code){
struct Cyc_Absyn_PtrLoc*_tmpC53;ptrloc=((_tmpC53=_cycalloc_atomic(sizeof(*_tmpC53)),((
_tmpC53->ptr_loc=_tmp634,((_tmpC53->rgn_loc=(unsigned int)(yyyylsp[2]).first_line,((
_tmpC53->zt_loc=(unsigned int)(yyyylsp[1]).first_line,_tmpC53))))))));}{union Cyc_Absyn_Constraint*
_tmp639;union Cyc_Absyn_Constraint*_tmp63A;union Cyc_Absyn_Constraint*_tmp63B;void*
_tmp63C;struct _tuple15 _tmp638=Cyc_Parse_collapse_pointer_quals(_tmp634,_tmp635,
_tmp636,Cyc_yyget_YY44(yyyyvsp[2]),Cyc_yyget_YY54(yyyyvsp[1]));_tmp639=_tmp638.f1;
_tmp63A=_tmp638.f2;_tmp63B=_tmp638.f3;_tmp63C=_tmp638.f4;{struct Cyc_Absyn_Pointer_mod_struct*
_tmpC5D;struct Cyc_Absyn_PtrAtts _tmpC5C;struct Cyc_Absyn_Pointer_mod_struct _tmpC5B;
struct Cyc_List_List*_tmpC5A;ans=((_tmpC5A=_region_malloc(yyr,sizeof(*_tmpC5A)),((
_tmpC5A->hd=(void*)((_tmpC5D=_region_malloc(yyr,sizeof(*_tmpC5D)),((_tmpC5D[0]=((
_tmpC5B.tag=2,((_tmpC5B.f1=((_tmpC5C.rgn=_tmp63C,((_tmpC5C.nullable=_tmp639,((
_tmpC5C.bounds=_tmp63A,((_tmpC5C.zero_term=_tmp63B,((_tmpC5C.ptrloc=ptrloc,
_tmpC5C)))))))))),((_tmpC5B.f2=Cyc_yyget_YY25(yyyyvsp[4]),_tmpC5B)))))),_tmpC5D)))),((
_tmpC5A->tl=ans,_tmpC5A))))));}yyval=Cyc_YY28(ans);break;};};}case 186: _LL2D7:
yyval=Cyc_YY54(0);break;case 187: _LL2D8: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_List_List*_tmpC5E;yyval=
Cyc_YY54(((_tmpC5E=_region_malloc(yyr,sizeof(*_tmpC5E)),((_tmpC5E->hd=Cyc_yyget_YY53(
yyyyvsp[0]),((_tmpC5E->tl=Cyc_yyget_YY54(yyyyvsp[1]),_tmpC5E)))))));}break;}case
188: _LL2D9: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_Numelts_ptrqual_struct
_tmpC61;struct Cyc_Absyn_Numelts_ptrqual_struct*_tmpC60;yyval=Cyc_YY53((void*)((
_tmpC60=_region_malloc(yyr,sizeof(*_tmpC60)),((_tmpC60[0]=((_tmpC61.tag=0,((
_tmpC61.f1=Cyc_yyget_YY3(yyyyvsp[2]),_tmpC61)))),_tmpC60)))));}break;}case 189:
_LL2DA: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_Region_ptrqual_struct
_tmpC64;struct Cyc_Absyn_Region_ptrqual_struct*_tmpC63;yyval=Cyc_YY53((void*)((
_tmpC63=_region_malloc(yyr,sizeof(*_tmpC63)),((_tmpC63[0]=((_tmpC64.tag=1,((
_tmpC64.f1=(void*)Cyc_yyget_YY44(yyyyvsp[2]),_tmpC64)))),_tmpC63)))));}break;}
case 190: _LL2DB: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Thin_ptrqual_struct
_tmpC67;struct Cyc_Absyn_Thin_ptrqual_struct*_tmpC66;yyval=Cyc_YY53((void*)((
_tmpC66=_region_malloc(yyr,sizeof(*_tmpC66)),((_tmpC66[0]=((_tmpC67.tag=2,
_tmpC67)),_tmpC66)))));}break;}case 191: _LL2DC: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Fat_ptrqual_struct
_tmpC6A;struct Cyc_Absyn_Fat_ptrqual_struct*_tmpC69;yyval=Cyc_YY53((void*)((
_tmpC69=_region_malloc(yyr,sizeof(*_tmpC69)),((_tmpC69[0]=((_tmpC6A.tag=3,
_tmpC6A)),_tmpC69)))));}break;}case 192: _LL2DD: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Zeroterm_ptrqual_struct
_tmpC6D;struct Cyc_Absyn_Zeroterm_ptrqual_struct*_tmpC6C;yyval=Cyc_YY53((void*)((
_tmpC6C=_region_malloc(yyr,sizeof(*_tmpC6C)),((_tmpC6C[0]=((_tmpC6D.tag=4,
_tmpC6D)),_tmpC6C)))));}break;}case 193: _LL2DE: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Nozeroterm_ptrqual_struct
_tmpC70;struct Cyc_Absyn_Nozeroterm_ptrqual_struct*_tmpC6F;yyval=Cyc_YY53((void*)((
_tmpC6F=_region_malloc(yyr,sizeof(*_tmpC6F)),((_tmpC6F[0]=((_tmpC70.tag=5,
_tmpC70)),_tmpC6F)))));}break;}case 194: _LL2DF: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Notnull_ptrqual_struct
_tmpC73;struct Cyc_Absyn_Notnull_ptrqual_struct*_tmpC72;yyval=Cyc_YY53((void*)((
_tmpC72=_region_malloc(yyr,sizeof(*_tmpC72)),((_tmpC72[0]=((_tmpC73.tag=6,
_tmpC73)),_tmpC72)))));}break;}case 195: _LL2E0: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Nullable_ptrqual_struct
_tmpC76;struct Cyc_Absyn_Nullable_ptrqual_struct*_tmpC75;yyval=Cyc_YY53((void*)((
_tmpC75=_region_malloc(yyr,sizeof(*_tmpC75)),((_tmpC75[0]=((_tmpC76.tag=7,
_tmpC76)),_tmpC75)))));}break;}case 196: _LL2E1: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);unsigned int loc=(unsigned int)(
yyyylsp[0]).first_line;{struct _tuple21*_tmpC77;yyval=Cyc_YY1(((_tmpC77=_cycalloc(
sizeof(*_tmpC77)),((_tmpC77->f1=loc,((_tmpC77->f2=Cyc_Absyn_true_conref,((
_tmpC77->f3=Cyc_yyget_YY2(yyyyvsp[1]),_tmpC77)))))))));}break;}case 197: _LL2E2: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);unsigned int loc=(unsigned int)(
yyyylsp[0]).first_line;{struct _tuple21*_tmpC78;yyval=Cyc_YY1(((_tmpC78=_cycalloc(
sizeof(*_tmpC78)),((_tmpC78->f1=loc,((_tmpC78->f2=Cyc_Absyn_false_conref,((
_tmpC78->f3=Cyc_yyget_YY2(yyyyvsp[1]),_tmpC78)))))))));}break;}case 198: _LL2E3: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);unsigned int loc=(unsigned int)(
yyyylsp[0]).first_line;{struct _tuple21*_tmpC79;yyval=Cyc_YY1(((_tmpC79=_cycalloc(
sizeof(*_tmpC79)),((_tmpC79->f1=loc,((_tmpC79->f2=Cyc_Absyn_true_conref,((
_tmpC79->f3=Cyc_Absyn_bounds_dyneither_conref,_tmpC79)))))))));}break;}case 199:
_LL2E4: yyval=Cyc_YY2(Cyc_Absyn_bounds_one_conref);break;case 200: _LL2E5: {union
Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(
yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);
struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(
struct Cyc_Yyltype),3);{struct Cyc_Absyn_Upper_b_struct _tmpC7C;struct Cyc_Absyn_Upper_b_struct*
_tmpC7B;yyval=Cyc_YY2(((union Cyc_Absyn_Constraint*(*)(void*x))Cyc_Absyn_new_conref)((
void*)((_tmpC7B=_cycalloc(sizeof(*_tmpC7B)),((_tmpC7B[0]=((_tmpC7C.tag=1,((
_tmpC7C.f1=Cyc_yyget_YY3(yyyyvsp[1]),_tmpC7C)))),_tmpC7B))))));}break;}case 201:
_LL2E6: yyval=Cyc_YY51(((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)());
break;case 202: _LL2E7: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY51(Cyc_Absyn_true_conref);
break;}case 203: _LL2E8: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY51(Cyc_Absyn_false_conref);
break;}case 204: _LL2E9: yyval=Cyc_YY44(Cyc_Absyn_new_evar((struct Cyc_Core_Opt*)&
Cyc_Tcutil_trko,0));break;case 205: _LL2EA: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);Cyc_Parse_set_vartyp_kind(Cyc_yyget_YY44(
yyyyvsp[0]),& Cyc_Tcutil_trk,1);yyval=yyyyvsp[0];break;}case 206: _LL2EB: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
Cyc_YY44(Cyc_Absyn_new_evar((struct Cyc_Core_Opt*)& Cyc_Tcutil_trko,0));break;}
case 207: _LL2EC: yyval=Cyc_YY25(Cyc_Absyn_empty_tqual((unsigned int)(*((struct Cyc_Yyltype*)
_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset + 1))).first_line));
break;case 208: _LL2ED: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY25(Cyc_Absyn_combine_tqual(
Cyc_yyget_YY25(yyyyvsp[0]),Cyc_yyget_YY25(yyyyvsp[1])));break;}case 209: _LL2EE: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _tuple25*_tmpC7D;yyval=Cyc_YY39(((
_tmpC7D=_cycalloc(sizeof(*_tmpC7D)),((_tmpC7D->f1=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_yyget_YY38(yyyyvsp[0])),((_tmpC7D->f2=
0,((_tmpC7D->f3=0,((_tmpC7D->f4=Cyc_yyget_YY49(yyyyvsp[1]),((_tmpC7D->f5=Cyc_yyget_YY50(
yyyyvsp[2]),_tmpC7D)))))))))))));}break;}case 210: _LL2EF: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct
_tuple25*_tmpC7E;yyval=Cyc_YY39(((_tmpC7E=_cycalloc(sizeof(*_tmpC7E)),((_tmpC7E->f1=((
struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_yyget_YY38(
yyyyvsp[0])),((_tmpC7E->f2=1,((_tmpC7E->f3=0,((_tmpC7E->f4=Cyc_yyget_YY49(
yyyyvsp[3]),((_tmpC7E->f5=Cyc_yyget_YY50(yyyyvsp[4]),_tmpC7E)))))))))))));}
break;}case 211: _LL2F0: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct _tuple8 _tmp65A;struct
_dyneither_ptr*_tmp65B;struct Cyc_Absyn_Tqual _tmp65C;void*_tmp65D;struct _tuple8*
_tmp659=Cyc_yyget_YY37(yyyyvsp[2]);_tmp65A=*_tmp659;_tmp65B=_tmp65A.f1;_tmp65C=
_tmp65A.f2;_tmp65D=_tmp65A.f3;{struct Cyc_Absyn_VarargInfo*_tmpC7F;struct Cyc_Absyn_VarargInfo*
_tmp65E=(_tmpC7F=_cycalloc(sizeof(*_tmpC7F)),((_tmpC7F->name=_tmp65B,((_tmpC7F->tq=
_tmp65C,((_tmpC7F->type=_tmp65D,((_tmpC7F->inject=Cyc_yyget_YY31(yyyyvsp[1]),
_tmpC7F)))))))));{struct _tuple25*_tmpC80;yyval=Cyc_YY39(((_tmpC80=_cycalloc(
sizeof(*_tmpC80)),((_tmpC80->f1=0,((_tmpC80->f2=0,((_tmpC80->f3=_tmp65E,((
_tmpC80->f4=Cyc_yyget_YY49(yyyyvsp[3]),((_tmpC80->f5=Cyc_yyget_YY50(yyyyvsp[4]),
_tmpC80)))))))))))));}break;};}case 212: _LL2F1: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 6),sizeof(union Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);struct _tuple8 _tmp662;struct
_dyneither_ptr*_tmp663;struct Cyc_Absyn_Tqual _tmp664;void*_tmp665;struct _tuple8*
_tmp661=Cyc_yyget_YY37(yyyyvsp[4]);_tmp662=*_tmp661;_tmp663=_tmp662.f1;_tmp664=
_tmp662.f2;_tmp665=_tmp662.f3;{struct Cyc_Absyn_VarargInfo*_tmpC81;struct Cyc_Absyn_VarargInfo*
_tmp666=(_tmpC81=_cycalloc(sizeof(*_tmpC81)),((_tmpC81->name=_tmp663,((_tmpC81->tq=
_tmp664,((_tmpC81->type=_tmp665,((_tmpC81->inject=Cyc_yyget_YY31(yyyyvsp[3]),
_tmpC81)))))))));{struct _tuple25*_tmpC82;yyval=Cyc_YY39(((_tmpC82=_cycalloc(
sizeof(*_tmpC82)),((_tmpC82->f1=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))
Cyc_List_imp_rev)(Cyc_yyget_YY38(yyyyvsp[0])),((_tmpC82->f2=0,((_tmpC82->f3=
_tmp666,((_tmpC82->f4=Cyc_yyget_YY49(yyyyvsp[5]),((_tmpC82->f5=Cyc_yyget_YY50(
yyyyvsp[6]),_tmpC82)))))))))))));}break;};}case 213: _LL2F2: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct
Cyc_Absyn_Unknown_kb_struct _tmpC85;struct Cyc_Absyn_Unknown_kb_struct*_tmpC84;
yyval=Cyc_YY44(Cyc_Parse_id2type(Cyc_yyget_String_tok(yyyyvsp[0]),(void*)((
_tmpC84=_cycalloc(sizeof(*_tmpC84)),((_tmpC84[0]=((_tmpC85.tag=1,((_tmpC85.f1=0,
_tmpC85)))),_tmpC84))))));}break;}case 214: _LL2F3: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY44(Cyc_Parse_id2type(
Cyc_yyget_String_tok(yyyyvsp[0]),Cyc_Tcutil_kind_to_bound(Cyc_yyget_YY43(yyyyvsp[
2]))));break;}case 215: _LL2F4: yyval=Cyc_YY49(0);break;case 216: _LL2F5: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct
Cyc_Absyn_JoinEff_struct _tmpC88;struct Cyc_Absyn_JoinEff_struct*_tmpC87;yyval=Cyc_YY49((
void*)((_tmpC87=_cycalloc(sizeof(*_tmpC87)),((_tmpC87[0]=((_tmpC88.tag=24,((
_tmpC88.f1=Cyc_yyget_YY40(yyyyvsp[1]),_tmpC88)))),_tmpC87)))));}break;}case 217:
_LL2F6: yyval=Cyc_YY50(0);break;case 218: _LL2F7: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=yyyyvsp[1];break;}case 219:
_LL2F8: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);struct Cyc_Absyn_Less_kb_struct
_tmpC8B;struct Cyc_Absyn_Less_kb_struct*_tmpC8A;struct Cyc_Absyn_Less_kb_struct*
_tmp66D=(_tmpC8A=_cycalloc(sizeof(*_tmpC8A)),((_tmpC8A[0]=((_tmpC8B.tag=2,((
_tmpC8B.f1=0,((_tmpC8B.f2=& Cyc_Tcutil_trk,_tmpC8B)))))),_tmpC8A)));struct
_dyneither_ptr _tmp66E=Cyc_yyget_String_tok(yyyyvsp[2]);void*_tmp66F=Cyc_Parse_id2type(
_tmp66E,(void*)_tmp66D);{struct _tuple28*_tmpC95;struct Cyc_Absyn_JoinEff_struct
_tmpC94;struct Cyc_Absyn_JoinEff_struct*_tmpC93;struct Cyc_List_List*_tmpC92;yyval=
Cyc_YY50(((_tmpC92=_cycalloc(sizeof(*_tmpC92)),((_tmpC92->hd=((_tmpC95=_cycalloc(
sizeof(*_tmpC95)),((_tmpC95->f1=(void*)((_tmpC93=_cycalloc(sizeof(*_tmpC93)),((
_tmpC93[0]=((_tmpC94.tag=24,((_tmpC94.f1=Cyc_yyget_YY40(yyyyvsp[0]),_tmpC94)))),
_tmpC93)))),((_tmpC95->f2=_tmp66F,_tmpC95)))))),((_tmpC92->tl=0,_tmpC92)))))));}
break;}case 220: _LL2F9: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct Cyc_Absyn_Less_kb_struct
_tmpC98;struct Cyc_Absyn_Less_kb_struct*_tmpC97;struct Cyc_Absyn_Less_kb_struct*
_tmp676=(_tmpC97=_cycalloc(sizeof(*_tmpC97)),((_tmpC97[0]=((_tmpC98.tag=2,((
_tmpC98.f1=0,((_tmpC98.f2=& Cyc_Tcutil_trk,_tmpC98)))))),_tmpC97)));struct
_dyneither_ptr _tmp677=Cyc_yyget_String_tok(yyyyvsp[2]);void*_tmp678=Cyc_Parse_id2type(
_tmp677,(void*)_tmp676);{struct _tuple28*_tmpCA2;struct Cyc_Absyn_JoinEff_struct
_tmpCA1;struct Cyc_Absyn_JoinEff_struct*_tmpCA0;struct Cyc_List_List*_tmpC9F;yyval=
Cyc_YY50(((_tmpC9F=_cycalloc(sizeof(*_tmpC9F)),((_tmpC9F->hd=((_tmpCA2=_cycalloc(
sizeof(*_tmpCA2)),((_tmpCA2->f1=(void*)((_tmpCA0=_cycalloc(sizeof(*_tmpCA0)),((
_tmpCA0[0]=((_tmpCA1.tag=24,((_tmpCA1.f1=Cyc_yyget_YY40(yyyyvsp[0]),_tmpCA1)))),
_tmpCA0)))),((_tmpCA2->f2=_tmp678,_tmpCA2)))))),((_tmpC9F->tl=Cyc_yyget_YY50(
yyyyvsp[4]),_tmpC9F)))))));}break;}case 221: _LL2FA: yyval=Cyc_YY31(0);break;case
222: _LL2FB: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{const char*_tmpCA3;if(Cyc_zstrcmp((
struct _dyneither_ptr)Cyc_yyget_String_tok(yyyyvsp[0]),((_tmpCA3="inject",
_tag_dyneither(_tmpCA3,sizeof(char),7))))!= 0){const char*_tmpCA4;Cyc_Parse_err(((
_tmpCA4="missing type in function declaration",_tag_dyneither(_tmpCA4,sizeof(
char),37))),(unsigned int)(yyyylsp[0]).first_line);}}yyval=Cyc_YY31(1);break;}
case 223: _LL2FC: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 224:
_LL2FD: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY40(((struct Cyc_List_List*(*)(
struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(Cyc_yyget_YY40(
yyyyvsp[0]),Cyc_yyget_YY40(yyyyvsp[2])));break;}case 225: _LL2FE: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=
Cyc_YY40(0);break;}case 226: _LL2FF: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=yyyyvsp[1];break;}case 227:
_LL300: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_RgnsEff_struct*
_tmpCAA;struct Cyc_Absyn_RgnsEff_struct _tmpCA9;struct Cyc_List_List*_tmpCA8;yyval=
Cyc_YY40(((_tmpCA8=_cycalloc(sizeof(*_tmpCA8)),((_tmpCA8->hd=(void*)((_tmpCAA=
_cycalloc(sizeof(*_tmpCAA)),((_tmpCAA[0]=((_tmpCA9.tag=25,((_tmpCA9.f1=(void*)
Cyc_yyget_YY44(yyyyvsp[2]),_tmpCA9)))),_tmpCAA)))),((_tmpCA8->tl=0,_tmpCA8)))))));}
break;}case 228: _LL301: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);Cyc_Parse_set_vartyp_kind(Cyc_yyget_YY44(
yyyyvsp[0]),& Cyc_Tcutil_ek,0);{struct Cyc_List_List*_tmpCAB;yyval=Cyc_YY40(((
_tmpCAB=_cycalloc(sizeof(*_tmpCAB)),((_tmpCAB->hd=Cyc_yyget_YY44(yyyyvsp[0]),((
_tmpCAB->tl=0,_tmpCAB)))))));}break;}case 229: _LL302: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);Cyc_Parse_set_vartyp_kind(Cyc_yyget_YY44(
yyyyvsp[0]),& Cyc_Tcutil_trk,1);{struct Cyc_Absyn_AccessEff_struct*_tmpCB1;struct
Cyc_Absyn_AccessEff_struct _tmpCB0;struct Cyc_List_List*_tmpCAF;yyval=Cyc_YY40(((
_tmpCAF=_cycalloc(sizeof(*_tmpCAF)),((_tmpCAF->hd=(void*)((_tmpCB1=_cycalloc(
sizeof(*_tmpCB1)),((_tmpCB1[0]=((_tmpCB0.tag=23,((_tmpCB0.f1=(void*)Cyc_yyget_YY44(
yyyyvsp[0]),_tmpCB0)))),_tmpCB1)))),((_tmpCAF->tl=0,_tmpCAF)))))));}break;}case
230: _LL303: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);Cyc_Parse_set_vartyp_kind(Cyc_yyget_YY44(
yyyyvsp[0]),& Cyc_Tcutil_trk,1);{struct Cyc_Absyn_AccessEff_struct*_tmpCB7;struct
Cyc_Absyn_AccessEff_struct _tmpCB6;struct Cyc_List_List*_tmpCB5;yyval=Cyc_YY40(((
_tmpCB5=_cycalloc(sizeof(*_tmpCB5)),((_tmpCB5->hd=(void*)((_tmpCB7=_cycalloc(
sizeof(*_tmpCB7)),((_tmpCB7[0]=((_tmpCB6.tag=23,((_tmpCB6.f1=(void*)Cyc_yyget_YY44(
yyyyvsp[0]),_tmpCB6)))),_tmpCB7)))),((_tmpCB5->tl=Cyc_yyget_YY40(yyyyvsp[2]),
_tmpCB5)))))));}break;}case 231: _LL304: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_List_List*_tmpCB8;yyval=
Cyc_YY38(((_tmpCB8=_cycalloc(sizeof(*_tmpCB8)),((_tmpCB8->hd=Cyc_yyget_YY37(
yyyyvsp[0]),((_tmpCB8->tl=0,_tmpCB8)))))));}break;}case 232: _LL305: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct
Cyc_List_List*_tmpCB9;yyval=Cyc_YY38(((_tmpCB9=_cycalloc(sizeof(*_tmpCB9)),((
_tmpCB9->hd=Cyc_yyget_YY37(yyyyvsp[2]),((_tmpCB9->tl=Cyc_yyget_YY38(yyyyvsp[0]),
_tmpCB9)))))));}break;}case 233: _LL306: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_Absyn_Tqual _tmp68E;
struct Cyc_Parse_Type_specifier _tmp68F;struct Cyc_List_List*_tmp690;struct _tuple24
_tmp68D=Cyc_yyget_YY35(yyyyvsp[0]);_tmp68E=_tmp68D.f1;_tmp68F=_tmp68D.f2;_tmp690=
_tmp68D.f3;if(_tmp68E.loc == 0)_tmp68E.loc=(unsigned int)(yyyylsp[0]).first_line;{
struct _tuple0*_tmp692;struct Cyc_List_List*_tmp693;struct Cyc_Parse_Declarator
_tmp691=Cyc_yyget_YY29(yyyyvsp[1]);_tmp692=_tmp691.id;_tmp693=_tmp691.tms;{void*
_tmp694=Cyc_Parse_speclist2typ(_tmp68F,(unsigned int)(yyyylsp[0]).first_line);
struct Cyc_Absyn_Tqual _tmp696;void*_tmp697;struct Cyc_List_List*_tmp698;struct Cyc_List_List*
_tmp699;struct _tuple14 _tmp695=Cyc_Parse_apply_tms(_tmp68E,_tmp694,_tmp690,
_tmp693);_tmp696=_tmp695.f1;_tmp697=_tmp695.f2;_tmp698=_tmp695.f3;_tmp699=
_tmp695.f4;if(_tmp698 != 0){const char*_tmpCBA;Cyc_Parse_err(((_tmpCBA="parameter with bad type params",
_tag_dyneither(_tmpCBA,sizeof(char),31))),(unsigned int)(yyyylsp[1]).first_line);}
if(Cyc_Absyn_is_qvar_qualified(_tmp692)){const char*_tmpCBB;Cyc_Parse_err(((
_tmpCBB="parameter cannot be qualified with a namespace",_tag_dyneither(_tmpCBB,
sizeof(char),47))),(unsigned int)(yyyylsp[0]).first_line);}{struct _dyneither_ptr*
idopt=(struct _dyneither_ptr*)(*_tmp692).f2;if(_tmp699 != 0){const char*_tmpCBE;
void*_tmpCBD;(_tmpCBD=0,Cyc_Tcutil_warn((unsigned int)(yyyylsp[0]).first_line,((
_tmpCBE="extra attributes on parameter, ignoring",_tag_dyneither(_tmpCBE,sizeof(
char),40))),_tag_dyneither(_tmpCBD,sizeof(void*),0)));}{struct _tuple8*_tmpCBF;
yyval=Cyc_YY37(((_tmpCBF=_cycalloc(sizeof(*_tmpCBF)),((_tmpCBF->f1=idopt,((
_tmpCBF->f2=_tmp696,((_tmpCBF->f3=_tmp697,_tmpCBF)))))))));}break;};};};}case 234:
_LL307: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);struct Cyc_Absyn_Tqual _tmp6A0;
struct Cyc_Parse_Type_specifier _tmp6A1;struct Cyc_List_List*_tmp6A2;struct _tuple24
_tmp69F=Cyc_yyget_YY35(yyyyvsp[0]);_tmp6A0=_tmp69F.f1;_tmp6A1=_tmp69F.f2;_tmp6A2=
_tmp69F.f3;if(_tmp6A0.loc == 0)_tmp6A0.loc=(unsigned int)(yyyylsp[0]).first_line;{
void*_tmp6A3=Cyc_Parse_speclist2typ(_tmp6A1,(unsigned int)(yyyylsp[0]).first_line);
if(_tmp6A2 != 0){const char*_tmpCC2;void*_tmpCC1;(_tmpCC1=0,Cyc_Tcutil_warn((
unsigned int)(yyyylsp[0]).first_line,((_tmpCC2="bad attributes on parameter, ignoring",
_tag_dyneither(_tmpCC2,sizeof(char),38))),_tag_dyneither(_tmpCC1,sizeof(void*),0)));}{
struct _tuple8*_tmpCC3;yyval=Cyc_YY37(((_tmpCC3=_cycalloc(sizeof(*_tmpCC3)),((
_tmpCC3->f1=0,((_tmpCC3->f2=_tmp6A0,((_tmpCC3->f3=_tmp6A3,_tmpCC3)))))))));}
break;};}case 235: _LL308: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_Absyn_Tqual _tmp6A8;
struct Cyc_Parse_Type_specifier _tmp6A9;struct Cyc_List_List*_tmp6AA;struct _tuple24
_tmp6A7=Cyc_yyget_YY35(yyyyvsp[0]);_tmp6A8=_tmp6A7.f1;_tmp6A9=_tmp6A7.f2;_tmp6AA=
_tmp6A7.f3;if(_tmp6A8.loc == 0)_tmp6A8.loc=(unsigned int)(yyyylsp[0]).first_line;{
void*_tmp6AB=Cyc_Parse_speclist2typ(_tmp6A9,(unsigned int)(yyyylsp[0]).first_line);
struct Cyc_List_List*_tmp6AC=(Cyc_yyget_YY30(yyyyvsp[1])).tms;struct Cyc_Absyn_Tqual
_tmp6AE;void*_tmp6AF;struct Cyc_List_List*_tmp6B0;struct Cyc_List_List*_tmp6B1;
struct _tuple14 _tmp6AD=Cyc_Parse_apply_tms(_tmp6A8,_tmp6AB,_tmp6AA,_tmp6AC);
_tmp6AE=_tmp6AD.f1;_tmp6AF=_tmp6AD.f2;_tmp6B0=_tmp6AD.f3;_tmp6B1=_tmp6AD.f4;if(
_tmp6B0 != 0){const char*_tmpCC6;void*_tmpCC5;(_tmpCC5=0,Cyc_Tcutil_warn((
unsigned int)(yyyylsp[0]).first_line,((_tmpCC6="bad type parameters on formal argument, ignoring",
_tag_dyneither(_tmpCC6,sizeof(char),49))),_tag_dyneither(_tmpCC5,sizeof(void*),0)));}
if(_tmp6B1 != 0){const char*_tmpCC9;void*_tmpCC8;(_tmpCC8=0,Cyc_Tcutil_warn((
unsigned int)(yyyylsp[0]).first_line,((_tmpCC9="bad attributes on parameter, ignoring",
_tag_dyneither(_tmpCC9,sizeof(char),38))),_tag_dyneither(_tmpCC8,sizeof(void*),0)));}{
struct _tuple8*_tmpCCA;yyval=Cyc_YY37(((_tmpCCA=_cycalloc(sizeof(*_tmpCCA)),((
_tmpCCA->f1=0,((_tmpCCA->f2=_tmp6AE,((_tmpCCA->f3=_tmp6AF,_tmpCCA)))))))));}
break;};}case 236: _LL309: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY36(((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_yyget_YY36(yyyyvsp[0])));break;}
case 237: _LL30A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _dyneither_ptr*_tmpCCD;
struct Cyc_List_List*_tmpCCC;yyval=Cyc_YY36(((_tmpCCC=_cycalloc(sizeof(*_tmpCCC)),((
_tmpCCC->hd=((_tmpCCD=_cycalloc(sizeof(*_tmpCCD)),((_tmpCCD[0]=Cyc_yyget_String_tok(
yyyyvsp[0]),_tmpCCD)))),((_tmpCCC->tl=0,_tmpCCC)))))));}break;}case 238: _LL30B: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _dyneither_ptr*_tmpCD0;
struct Cyc_List_List*_tmpCCF;yyval=Cyc_YY36(((_tmpCCF=_cycalloc(sizeof(*_tmpCCF)),((
_tmpCCF->hd=((_tmpCD0=_cycalloc(sizeof(*_tmpCD0)),((_tmpCD0[0]=Cyc_yyget_String_tok(
yyyyvsp[2]),_tmpCD0)))),((_tmpCCF->tl=Cyc_yyget_YY36(yyyyvsp[0]),_tmpCCF)))))));}
break;}case 239: _LL30C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 240:
_LL30D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 241:
_LL30E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Absyn_UnresolvedMem_e_struct
_tmpCD3;struct Cyc_Absyn_UnresolvedMem_e_struct*_tmpCD2;yyval=Cyc_YY3(Cyc_Absyn_new_exp((
void*)((_tmpCD2=_cycalloc(sizeof(*_tmpCD2)),((_tmpCD2[0]=((_tmpCD3.tag=34,((
_tmpCD3.f1=0,((_tmpCD3.f2=0,_tmpCD3)))))),_tmpCD2)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 242: _LL30F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_UnresolvedMem_e_struct
_tmpCD6;struct Cyc_Absyn_UnresolvedMem_e_struct*_tmpCD5;yyval=Cyc_YY3(Cyc_Absyn_new_exp((
void*)((_tmpCD5=_cycalloc(sizeof(*_tmpCD5)),((_tmpCD5[0]=((_tmpCD6.tag=34,((
_tmpCD6.f1=0,((_tmpCD6.f2=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(
Cyc_yyget_YY6(yyyyvsp[1])),_tmpCD6)))))),_tmpCD5)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 243: _LL310: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_UnresolvedMem_e_struct
_tmpCD9;struct Cyc_Absyn_UnresolvedMem_e_struct*_tmpCD8;yyval=Cyc_YY3(Cyc_Absyn_new_exp((
void*)((_tmpCD8=_cycalloc(sizeof(*_tmpCD8)),((_tmpCD8[0]=((_tmpCD9.tag=34,((
_tmpCD9.f1=0,((_tmpCD9.f2=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(
Cyc_yyget_YY6(yyyyvsp[1])),_tmpCD9)))))),_tmpCD8)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 244: _LL311: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 7),sizeof(union Cyc_YYSTYPE),8);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 7),sizeof(struct Cyc_Yyltype),8);struct _dyneither_ptr*_tmpCDC;
struct _tuple0*_tmpCDB;struct Cyc_Absyn_Vardecl*_tmp6C1=Cyc_Absyn_new_vardecl(((
_tmpCDB=_cycalloc(sizeof(*_tmpCDB)),((_tmpCDB->f1=Cyc_Absyn_Loc_n,((_tmpCDB->f2=((
_tmpCDC=_cycalloc(sizeof(*_tmpCDC)),((_tmpCDC[0]=Cyc_yyget_String_tok(yyyyvsp[2]),
_tmpCDC)))),_tmpCDB)))))),Cyc_Absyn_uint_typ,(struct Cyc_Absyn_Exp*)Cyc_Absyn_uint_exp(
0,(unsigned int)(yyyylsp[2]).first_line));(_tmp6C1->tq).real_const=1;{struct Cyc_Absyn_Comprehension_e_struct
_tmpCDF;struct Cyc_Absyn_Comprehension_e_struct*_tmpCDE;yyval=Cyc_YY3(Cyc_Absyn_new_exp((
void*)((_tmpCDE=_cycalloc(sizeof(*_tmpCDE)),((_tmpCDE[0]=((_tmpCDF.tag=26,((
_tmpCDF.f1=_tmp6C1,((_tmpCDF.f2=Cyc_yyget_YY3(yyyyvsp[4]),((_tmpCDF.f3=Cyc_yyget_YY3(
yyyyvsp[6]),((_tmpCDF.f4=0,_tmpCDF)))))))))),_tmpCDE)))),(unsigned int)(yyyylsp[
0]).first_line));}break;}case 245: _LL312: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple29*_tmpCE2;struct Cyc_List_List*
_tmpCE1;yyval=Cyc_YY6(((_tmpCE1=_cycalloc(sizeof(*_tmpCE1)),((_tmpCE1->hd=((
_tmpCE2=_cycalloc(sizeof(*_tmpCE2)),((_tmpCE2->f1=0,((_tmpCE2->f2=Cyc_yyget_YY3(
yyyyvsp[0]),_tmpCE2)))))),((_tmpCE1->tl=0,_tmpCE1)))))));}break;}case 246: _LL313: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct _tuple29*_tmpCE5;struct Cyc_List_List*
_tmpCE4;yyval=Cyc_YY6(((_tmpCE4=_cycalloc(sizeof(*_tmpCE4)),((_tmpCE4->hd=((
_tmpCE5=_cycalloc(sizeof(*_tmpCE5)),((_tmpCE5->f1=Cyc_yyget_YY41(yyyyvsp[0]),((
_tmpCE5->f2=Cyc_yyget_YY3(yyyyvsp[1]),_tmpCE5)))))),((_tmpCE4->tl=0,_tmpCE4)))))));}
break;}case 247: _LL314: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _tuple29*_tmpCE8;struct Cyc_List_List*
_tmpCE7;yyval=Cyc_YY6(((_tmpCE7=_cycalloc(sizeof(*_tmpCE7)),((_tmpCE7->hd=((
_tmpCE8=_cycalloc(sizeof(*_tmpCE8)),((_tmpCE8->f1=0,((_tmpCE8->f2=Cyc_yyget_YY3(
yyyyvsp[2]),_tmpCE8)))))),((_tmpCE7->tl=Cyc_yyget_YY6(yyyyvsp[0]),_tmpCE7)))))));}
break;}case 248: _LL315: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct _tuple29*_tmpCEB;struct Cyc_List_List*
_tmpCEA;yyval=Cyc_YY6(((_tmpCEA=_cycalloc(sizeof(*_tmpCEA)),((_tmpCEA->hd=((
_tmpCEB=_cycalloc(sizeof(*_tmpCEB)),((_tmpCEB->f1=Cyc_yyget_YY41(yyyyvsp[2]),((
_tmpCEB->f2=Cyc_yyget_YY3(yyyyvsp[3]),_tmpCEB)))))),((_tmpCEA->tl=Cyc_yyget_YY6(
yyyyvsp[0]),_tmpCEA)))))));}break;}case 249: _LL316: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY41(((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_yyget_YY41(yyyyvsp[0])));break;}
case 250: _LL317: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_List_List*_tmpCEC;yyval=
Cyc_YY41(((_tmpCEC=_cycalloc(sizeof(*_tmpCEC)),((_tmpCEC->hd=Cyc_yyget_YY42(
yyyyvsp[0]),((_tmpCEC->tl=0,_tmpCEC)))))));}break;}case 251: _LL318: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct
Cyc_List_List*_tmpCED;yyval=Cyc_YY41(((_tmpCED=_cycalloc(sizeof(*_tmpCED)),((
_tmpCED->hd=Cyc_yyget_YY42(yyyyvsp[1]),((_tmpCED->tl=Cyc_yyget_YY41(yyyyvsp[0]),
_tmpCED)))))));}break;}case 252: _LL319: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_ArrayElement_struct
_tmpCF0;struct Cyc_Absyn_ArrayElement_struct*_tmpCEF;yyval=Cyc_YY42((void*)((
_tmpCEF=_cycalloc(sizeof(*_tmpCEF)),((_tmpCEF[0]=((_tmpCF0.tag=0,((_tmpCF0.f1=
Cyc_yyget_YY3(yyyyvsp[1]),_tmpCF0)))),_tmpCEF)))));}break;}case 253: _LL31A: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Absyn_FieldName_struct
_tmpCF6;struct _dyneither_ptr*_tmpCF5;struct Cyc_Absyn_FieldName_struct*_tmpCF4;
yyval=Cyc_YY42((void*)((_tmpCF4=_cycalloc(sizeof(*_tmpCF4)),((_tmpCF4[0]=((
_tmpCF6.tag=1,((_tmpCF6.f1=((_tmpCF5=_cycalloc(sizeof(*_tmpCF5)),((_tmpCF5[0]=
Cyc_yyget_String_tok(yyyyvsp[1]),_tmpCF5)))),_tmpCF6)))),_tmpCF4)))));}break;}
case 254: _LL31B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);struct Cyc_Absyn_Tqual _tmp6D6;
struct Cyc_Parse_Type_specifier _tmp6D7;struct Cyc_List_List*_tmp6D8;struct _tuple24
_tmp6D5=Cyc_yyget_YY35(yyyyvsp[0]);_tmp6D6=_tmp6D5.f1;_tmp6D7=_tmp6D5.f2;_tmp6D8=
_tmp6D5.f3;{void*_tmp6D9=Cyc_Parse_speclist2typ(_tmp6D7,(unsigned int)(yyyylsp[0]).first_line);
if(_tmp6D8 != 0){const char*_tmpCF9;void*_tmpCF8;(_tmpCF8=0,Cyc_Tcutil_warn((
unsigned int)(yyyylsp[0]).first_line,((_tmpCF9="ignoring attributes in type",
_tag_dyneither(_tmpCF9,sizeof(char),28))),_tag_dyneither(_tmpCF8,sizeof(void*),0)));}{
struct _tuple8*_tmpCFA;yyval=Cyc_YY37(((_tmpCFA=_cycalloc(sizeof(*_tmpCFA)),((
_tmpCFA->f1=0,((_tmpCFA->f2=_tmp6D6,((_tmpCFA->f3=_tmp6D9,_tmpCFA)))))))));}
break;};}case 255: _LL31C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);struct Cyc_Absyn_Tqual _tmp6DE;
struct Cyc_Parse_Type_specifier _tmp6DF;struct Cyc_List_List*_tmp6E0;struct _tuple24
_tmp6DD=Cyc_yyget_YY35(yyyyvsp[0]);_tmp6DE=_tmp6DD.f1;_tmp6DF=_tmp6DD.f2;_tmp6E0=
_tmp6DD.f3;{void*_tmp6E1=Cyc_Parse_speclist2typ(_tmp6DF,(unsigned int)(yyyylsp[0]).first_line);
struct Cyc_List_List*_tmp6E2=(Cyc_yyget_YY30(yyyyvsp[1])).tms;struct _tuple14
_tmp6E3=Cyc_Parse_apply_tms(_tmp6DE,_tmp6E1,_tmp6E0,_tmp6E2);if(_tmp6E3.f3 != 0){
const char*_tmpCFD;void*_tmpCFC;(_tmpCFC=0,Cyc_Tcutil_warn((unsigned int)(yyyylsp[
1]).first_line,((_tmpCFD="bad type params, ignoring",_tag_dyneither(_tmpCFD,
sizeof(char),26))),_tag_dyneither(_tmpCFC,sizeof(void*),0)));}if(_tmp6E3.f4 != 0){
const char*_tmpD00;void*_tmpCFF;(_tmpCFF=0,Cyc_Tcutil_warn((unsigned int)(yyyylsp[
1]).first_line,((_tmpD00="bad specifiers, ignoring",_tag_dyneither(_tmpD00,
sizeof(char),25))),_tag_dyneither(_tmpCFF,sizeof(void*),0)));}{struct _tuple8*
_tmpD01;yyval=Cyc_YY37(((_tmpD01=_cycalloc(sizeof(*_tmpD01)),((_tmpD01->f1=0,((
_tmpD01->f2=_tmp6E3.f1,((_tmpD01->f3=_tmp6E3.f2,_tmpD01)))))))));}break;};}case
256: _LL31D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY44(Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}case
257: _LL31E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Absyn_JoinEff_struct
_tmpD04;struct Cyc_Absyn_JoinEff_struct*_tmpD03;yyval=Cyc_YY44((void*)((_tmpD03=
_cycalloc(sizeof(*_tmpD03)),((_tmpD03[0]=((_tmpD04.tag=24,((_tmpD04.f1=0,_tmpD04)))),
_tmpD03)))));}break;}case 258: _LL31F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_JoinEff_struct
_tmpD07;struct Cyc_Absyn_JoinEff_struct*_tmpD06;yyval=Cyc_YY44((void*)((_tmpD06=
_cycalloc(sizeof(*_tmpD06)),((_tmpD06[0]=((_tmpD07.tag=24,((_tmpD07.f1=Cyc_yyget_YY40(
yyyyvsp[1]),_tmpD07)))),_tmpD06)))));}break;}case 259: _LL320: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct
Cyc_Absyn_RgnsEff_struct _tmpD0A;struct Cyc_Absyn_RgnsEff_struct*_tmpD09;yyval=Cyc_YY44((
void*)((_tmpD09=_cycalloc(sizeof(*_tmpD09)),((_tmpD09[0]=((_tmpD0A.tag=25,((
_tmpD0A.f1=(void*)Cyc_yyget_YY44(yyyyvsp[2]),_tmpD0A)))),_tmpD09)))));}break;}
case 260: _LL321: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_JoinEff_struct
_tmpD10;struct Cyc_List_List*_tmpD0F;struct Cyc_Absyn_JoinEff_struct*_tmpD0E;yyval=
Cyc_YY44((void*)((_tmpD0E=_cycalloc(sizeof(*_tmpD0E)),((_tmpD0E[0]=((_tmpD10.tag=
24,((_tmpD10.f1=((_tmpD0F=_cycalloc(sizeof(*_tmpD0F)),((_tmpD0F->hd=Cyc_yyget_YY44(
yyyyvsp[0]),((_tmpD0F->tl=Cyc_yyget_YY40(yyyyvsp[2]),_tmpD0F)))))),_tmpD10)))),
_tmpD0E)))));}break;}case 261: _LL322: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_List_List*_tmpD11;yyval=
Cyc_YY40(((_tmpD11=_cycalloc(sizeof(*_tmpD11)),((_tmpD11->hd=Cyc_yyget_YY44(
yyyyvsp[0]),((_tmpD11->tl=0,_tmpD11)))))));}break;}case 262: _LL323: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct
Cyc_List_List*_tmpD12;yyval=Cyc_YY40(((_tmpD12=_cycalloc(sizeof(*_tmpD12)),((
_tmpD12->hd=Cyc_yyget_YY44(yyyyvsp[2]),((_tmpD12->tl=Cyc_yyget_YY40(yyyyvsp[0]),
_tmpD12)))))));}break;}case 263: _LL324: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Parse_Abstractdeclarator
_tmpD13;yyval=Cyc_YY30(((_tmpD13.tms=Cyc_yyget_YY28(yyyyvsp[0]),_tmpD13)));}
break;}case 264: _LL325: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 265:
_LL326: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Parse_Abstractdeclarator
_tmpD14;yyval=Cyc_YY30(((_tmpD14.tms=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_imp_append)(Cyc_yyget_YY28(yyyyvsp[0]),(Cyc_yyget_YY30(
yyyyvsp[1])).tms),_tmpD14)));}break;}case 266: _LL327: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=yyyyvsp[1];break;}case 267:
_LL328: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_List_List*_tmpD1E;
struct Cyc_Absyn_Carray_mod_struct _tmpD1D;struct Cyc_Absyn_Carray_mod_struct*
_tmpD1C;struct Cyc_Parse_Abstractdeclarator _tmpD1B;yyval=Cyc_YY30(((_tmpD1B.tms=((
_tmpD1E=_region_malloc(yyr,sizeof(*_tmpD1E)),((_tmpD1E->hd=(void*)((_tmpD1C=
_region_malloc(yyr,sizeof(*_tmpD1C)),((_tmpD1C[0]=((_tmpD1D.tag=0,((_tmpD1D.f1=
Cyc_yyget_YY51(yyyyvsp[2]),((_tmpD1D.f2=(unsigned int)(yyyylsp[2]).first_line,
_tmpD1D)))))),_tmpD1C)))),((_tmpD1E->tl=0,_tmpD1E)))))),_tmpD1B)));}break;}case
268: _LL329: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_List_List*_tmpD28;
struct Cyc_Absyn_Carray_mod_struct _tmpD27;struct Cyc_Absyn_Carray_mod_struct*
_tmpD26;struct Cyc_Parse_Abstractdeclarator _tmpD25;yyval=Cyc_YY30(((_tmpD25.tms=((
_tmpD28=_region_malloc(yyr,sizeof(*_tmpD28)),((_tmpD28->hd=(void*)((_tmpD26=
_region_malloc(yyr,sizeof(*_tmpD26)),((_tmpD26[0]=((_tmpD27.tag=0,((_tmpD27.f1=
Cyc_yyget_YY51(yyyyvsp[3]),((_tmpD27.f2=(unsigned int)(yyyylsp[3]).first_line,
_tmpD27)))))),_tmpD26)))),((_tmpD28->tl=(Cyc_yyget_YY30(yyyyvsp[0])).tms,_tmpD28)))))),
_tmpD25)));}break;}case 269: _LL32A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_List_List*_tmpD32;
struct Cyc_Absyn_ConstArray_mod_struct _tmpD31;struct Cyc_Absyn_ConstArray_mod_struct*
_tmpD30;struct Cyc_Parse_Abstractdeclarator _tmpD2F;yyval=Cyc_YY30(((_tmpD2F.tms=((
_tmpD32=_region_malloc(yyr,sizeof(*_tmpD32)),((_tmpD32->hd=(void*)((_tmpD30=
_region_malloc(yyr,sizeof(*_tmpD30)),((_tmpD30[0]=((_tmpD31.tag=1,((_tmpD31.f1=
Cyc_yyget_YY3(yyyyvsp[1]),((_tmpD31.f2=Cyc_yyget_YY51(yyyyvsp[3]),((_tmpD31.f3=(
unsigned int)(yyyylsp[3]).first_line,_tmpD31)))))))),_tmpD30)))),((_tmpD32->tl=0,
_tmpD32)))))),_tmpD2F)));}break;}case 270: _LL32B: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct Cyc_List_List*_tmpD3C;
struct Cyc_Absyn_ConstArray_mod_struct _tmpD3B;struct Cyc_Absyn_ConstArray_mod_struct*
_tmpD3A;struct Cyc_Parse_Abstractdeclarator _tmpD39;yyval=Cyc_YY30(((_tmpD39.tms=((
_tmpD3C=_region_malloc(yyr,sizeof(*_tmpD3C)),((_tmpD3C->hd=(void*)((_tmpD3A=
_region_malloc(yyr,sizeof(*_tmpD3A)),((_tmpD3A[0]=((_tmpD3B.tag=1,((_tmpD3B.f1=
Cyc_yyget_YY3(yyyyvsp[2]),((_tmpD3B.f2=Cyc_yyget_YY51(yyyyvsp[4]),((_tmpD3B.f3=(
unsigned int)(yyyylsp[4]).first_line,_tmpD3B)))))))),_tmpD3A)))),((_tmpD3C->tl=(
Cyc_yyget_YY30(yyyyvsp[0])).tms,_tmpD3C)))))),_tmpD39)));}break;}case 271: _LL32C: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_List_List*_tmpD51;
struct Cyc_Absyn_Function_mod_struct _tmpD50;struct Cyc_Absyn_WithTypes_struct
_tmpD4F;struct Cyc_Absyn_WithTypes_struct*_tmpD4E;struct Cyc_Absyn_Function_mod_struct*
_tmpD4D;struct Cyc_Parse_Abstractdeclarator _tmpD4C;yyval=Cyc_YY30(((_tmpD4C.tms=((
_tmpD51=_region_malloc(yyr,sizeof(*_tmpD51)),((_tmpD51->hd=(void*)((_tmpD4D=
_region_malloc(yyr,sizeof(*_tmpD4D)),((_tmpD4D[0]=((_tmpD50.tag=3,((_tmpD50.f1=(
void*)((void*)((_tmpD4E=_region_malloc(yyr,sizeof(*_tmpD4E)),((_tmpD4E[0]=((
_tmpD4F.tag=1,((_tmpD4F.f1=0,((_tmpD4F.f2=0,((_tmpD4F.f3=0,((_tmpD4F.f4=(void*)
Cyc_yyget_YY49(yyyyvsp[1]),((_tmpD4F.f5=Cyc_yyget_YY50(yyyyvsp[2]),_tmpD4F)))))))))))),
_tmpD4E))))),_tmpD50)))),_tmpD4D)))),((_tmpD51->tl=0,_tmpD51)))))),_tmpD4C)));}
break;}case 272: _LL32D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);struct _tuple25 _tmp70D;struct Cyc_List_List*
_tmp70E;int _tmp70F;struct Cyc_Absyn_VarargInfo*_tmp710;void*_tmp711;struct Cyc_List_List*
_tmp712;struct _tuple25*_tmp70C=Cyc_yyget_YY39(yyyyvsp[1]);_tmp70D=*_tmp70C;
_tmp70E=_tmp70D.f1;_tmp70F=_tmp70D.f2;_tmp710=_tmp70D.f3;_tmp711=_tmp70D.f4;
_tmp712=_tmp70D.f5;{struct Cyc_List_List*_tmpD66;struct Cyc_Absyn_Function_mod_struct
_tmpD65;struct Cyc_Absyn_WithTypes_struct _tmpD64;struct Cyc_Absyn_WithTypes_struct*
_tmpD63;struct Cyc_Absyn_Function_mod_struct*_tmpD62;struct Cyc_Parse_Abstractdeclarator
_tmpD61;yyval=Cyc_YY30(((_tmpD61.tms=((_tmpD66=_region_malloc(yyr,sizeof(*
_tmpD66)),((_tmpD66->hd=(void*)((_tmpD62=_region_malloc(yyr,sizeof(*_tmpD62)),((
_tmpD62[0]=((_tmpD65.tag=3,((_tmpD65.f1=(void*)((void*)((_tmpD63=_region_malloc(
yyr,sizeof(*_tmpD63)),((_tmpD63[0]=((_tmpD64.tag=1,((_tmpD64.f1=_tmp70E,((
_tmpD64.f2=_tmp70F,((_tmpD64.f3=_tmp710,((_tmpD64.f4=(void*)_tmp711,((_tmpD64.f5=
_tmp712,_tmpD64)))))))))))),_tmpD63))))),_tmpD65)))),_tmpD62)))),((_tmpD66->tl=0,
_tmpD66)))))),_tmpD61)));}break;}case 273: _LL32E: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct Cyc_List_List*_tmpD7B;
struct Cyc_Absyn_Function_mod_struct _tmpD7A;struct Cyc_Absyn_WithTypes_struct
_tmpD79;struct Cyc_Absyn_WithTypes_struct*_tmpD78;struct Cyc_Absyn_Function_mod_struct*
_tmpD77;struct Cyc_Parse_Abstractdeclarator _tmpD76;yyval=Cyc_YY30(((_tmpD76.tms=((
_tmpD7B=_region_malloc(yyr,sizeof(*_tmpD7B)),((_tmpD7B->hd=(void*)((_tmpD77=
_region_malloc(yyr,sizeof(*_tmpD77)),((_tmpD77[0]=((_tmpD7A.tag=3,((_tmpD7A.f1=(
void*)((void*)((_tmpD78=_region_malloc(yyr,sizeof(*_tmpD78)),((_tmpD78[0]=((
_tmpD79.tag=1,((_tmpD79.f1=0,((_tmpD79.f2=0,((_tmpD79.f3=0,((_tmpD79.f4=(void*)
Cyc_yyget_YY49(yyyyvsp[2]),((_tmpD79.f5=Cyc_yyget_YY50(yyyyvsp[3]),_tmpD79)))))))))))),
_tmpD78))))),_tmpD7A)))),_tmpD77)))),((_tmpD7B->tl=(Cyc_yyget_YY30(yyyyvsp[0])).tms,
_tmpD7B)))))),_tmpD76)));}break;}case 274: _LL32F: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct _tuple25 _tmp720;struct Cyc_List_List*
_tmp721;int _tmp722;struct Cyc_Absyn_VarargInfo*_tmp723;void*_tmp724;struct Cyc_List_List*
_tmp725;struct _tuple25*_tmp71F=Cyc_yyget_YY39(yyyyvsp[2]);_tmp720=*_tmp71F;
_tmp721=_tmp720.f1;_tmp722=_tmp720.f2;_tmp723=_tmp720.f3;_tmp724=_tmp720.f4;
_tmp725=_tmp720.f5;{struct Cyc_List_List*_tmpD90;struct Cyc_Absyn_Function_mod_struct
_tmpD8F;struct Cyc_Absyn_WithTypes_struct _tmpD8E;struct Cyc_Absyn_WithTypes_struct*
_tmpD8D;struct Cyc_Absyn_Function_mod_struct*_tmpD8C;struct Cyc_Parse_Abstractdeclarator
_tmpD8B;yyval=Cyc_YY30(((_tmpD8B.tms=((_tmpD90=_region_malloc(yyr,sizeof(*
_tmpD90)),((_tmpD90->hd=(void*)((_tmpD8C=_region_malloc(yyr,sizeof(*_tmpD8C)),((
_tmpD8C[0]=((_tmpD8F.tag=3,((_tmpD8F.f1=(void*)((void*)((_tmpD8D=_region_malloc(
yyr,sizeof(*_tmpD8D)),((_tmpD8D[0]=((_tmpD8E.tag=1,((_tmpD8E.f1=_tmp721,((
_tmpD8E.f2=_tmp722,((_tmpD8E.f3=_tmp723,((_tmpD8E.f4=(void*)_tmp724,((_tmpD8E.f5=
_tmp725,_tmpD8E)))))))))))),_tmpD8D))))),_tmpD8F)))),_tmpD8C)))),((_tmpD90->tl=(
Cyc_yyget_YY30(yyyyvsp[0])).tms,_tmpD90)))))),_tmpD8B)));}break;}case 275: _LL330: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_List_List*_tmp72C=((
struct Cyc_List_List*(*)(struct Cyc_Absyn_Tvar*(*f)(unsigned int,void*),
unsigned int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,(
unsigned int)(yyyylsp[1]).first_line,((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_imp_rev)(Cyc_yyget_YY40(yyyyvsp[2])));{struct Cyc_List_List*_tmpD9A;
struct Cyc_Absyn_TypeParams_mod_struct _tmpD99;struct Cyc_Absyn_TypeParams_mod_struct*
_tmpD98;struct Cyc_Parse_Abstractdeclarator _tmpD97;yyval=Cyc_YY30(((_tmpD97.tms=((
_tmpD9A=_region_malloc(yyr,sizeof(*_tmpD9A)),((_tmpD9A->hd=(void*)((_tmpD98=
_region_malloc(yyr,sizeof(*_tmpD98)),((_tmpD98[0]=((_tmpD99.tag=4,((_tmpD99.f1=
_tmp72C,((_tmpD99.f2=(unsigned int)(yyyylsp[1]).first_line,((_tmpD99.f3=0,
_tmpD99)))))))),_tmpD98)))),((_tmpD9A->tl=(Cyc_yyget_YY30(yyyyvsp[0])).tms,
_tmpD9A)))))),_tmpD97)));}break;}case 276: _LL331: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_List_List*_tmpDA4;
struct Cyc_Absyn_Attributes_mod_struct _tmpDA3;struct Cyc_Absyn_Attributes_mod_struct*
_tmpDA2;struct Cyc_Parse_Abstractdeclarator _tmpDA1;yyval=Cyc_YY30(((_tmpDA1.tms=((
_tmpDA4=_region_malloc(yyr,sizeof(*_tmpDA4)),((_tmpDA4->hd=(void*)((_tmpDA2=
_region_malloc(yyr,sizeof(*_tmpDA2)),((_tmpDA2[0]=((_tmpDA3.tag=5,((_tmpDA3.f1=(
unsigned int)(yyyylsp[1]).first_line,((_tmpDA3.f2=Cyc_yyget_YY45(yyyyvsp[1]),
_tmpDA3)))))),_tmpDA2)))),((_tmpDA4->tl=(Cyc_yyget_YY30(yyyyvsp[0])).tms,_tmpDA4)))))),
_tmpDA1)));}break;}case 277: _LL332: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 278:
_LL333: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 279:
_LL334: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 280:
_LL335: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 281:
_LL336: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 282:
_LL337: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 283:
_LL338: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct Cyc_Absyn_ResetRegion_s_struct
_tmpDA7;struct Cyc_Absyn_ResetRegion_s_struct*_tmpDA6;yyval=Cyc_YY9(Cyc_Absyn_new_stmt((
void*)((_tmpDA6=_cycalloc(sizeof(*_tmpDA6)),((_tmpDA6[0]=((_tmpDA7.tag=16,((
_tmpDA7.f1=Cyc_yyget_YY3(yyyyvsp[2]),_tmpDA7)))),_tmpDA6)))),(unsigned int)(
yyyylsp[0]).first_line));}break;}case 284: _LL339: yyval=Cyc_YY4(0);break;case 285:
_LL33A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{const char*_tmpDA8;if(Cyc_zstrcmp((
struct _dyneither_ptr)Cyc_yyget_String_tok(yyyyvsp[1]),((_tmpDA8="open",
_tag_dyneither(_tmpDA8,sizeof(char),5))))!= 0){const char*_tmpDA9;Cyc_Parse_err(((
_tmpDA9="expecting `open'",_tag_dyneither(_tmpDA9,sizeof(char),17))),(
unsigned int)(yyyylsp[1]).first_line);}}yyval=Cyc_YY4((struct Cyc_Absyn_Exp*)Cyc_yyget_YY3(
yyyyvsp[3]));break;}case 286: _LL33B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct Cyc_Absyn_Label_s_struct
_tmpDAF;struct _dyneither_ptr*_tmpDAE;struct Cyc_Absyn_Label_s_struct*_tmpDAD;
yyval=Cyc_YY9(Cyc_Absyn_new_stmt((void*)((_tmpDAD=_cycalloc(sizeof(*_tmpDAD)),((
_tmpDAD[0]=((_tmpDAF.tag=13,((_tmpDAF.f1=((_tmpDAE=_cycalloc(sizeof(*_tmpDAE)),((
_tmpDAE[0]=Cyc_yyget_String_tok(yyyyvsp[0]),_tmpDAE)))),((_tmpDAF.f2=Cyc_yyget_YY9(
yyyyvsp[2]),_tmpDAF)))))),_tmpDAD)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 287: _LL33C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY9(Cyc_Absyn_skip_stmt((
unsigned int)(yyyylsp[0]).first_line));break;}case 288: _LL33D: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=
Cyc_YY9(Cyc_Absyn_exp_stmt(Cyc_yyget_YY3(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 289: _LL33E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY9(Cyc_Absyn_skip_stmt((
unsigned int)(yyyylsp[0]).first_line));break;}case 290: _LL33F: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=
yyyyvsp[1];break;}case 291: _LL340: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY9(Cyc_Parse_flatten_declarations(
Cyc_yyget_YY18(yyyyvsp[0]),Cyc_Absyn_skip_stmt(0)));break;}case 292: _LL341: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY9(Cyc_Parse_flatten_declarations(
Cyc_yyget_YY18(yyyyvsp[0]),Cyc_yyget_YY9(yyyyvsp[1])));break;}case 293: _LL342: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 294:
_LL343: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY9(Cyc_Absyn_seq_stmt(
Cyc_yyget_YY9(yyyyvsp[0]),Cyc_yyget_YY9(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 295: _LL344: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_Fn_d_struct
_tmpDB2;struct Cyc_Absyn_Fn_d_struct*_tmpDB1;yyval=Cyc_YY9(Cyc_Parse_flatten_decl(
Cyc_Absyn_new_decl((void*)((_tmpDB1=_cycalloc(sizeof(*_tmpDB1)),((_tmpDB1[0]=((
_tmpDB2.tag=1,((_tmpDB2.f1=Cyc_yyget_YY17(yyyyvsp[0]),_tmpDB2)))),_tmpDB1)))),(
unsigned int)(yyyylsp[0]).first_line),Cyc_Absyn_skip_stmt(0)));}break;}case 296:
_LL345: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Absyn_Fn_d_struct
_tmpDB5;struct Cyc_Absyn_Fn_d_struct*_tmpDB4;yyval=Cyc_YY9(Cyc_Parse_flatten_decl(
Cyc_Absyn_new_decl((void*)((_tmpDB4=_cycalloc(sizeof(*_tmpDB4)),((_tmpDB4[0]=((
_tmpDB5.tag=1,((_tmpDB5.f1=Cyc_yyget_YY17(yyyyvsp[0]),_tmpDB5)))),_tmpDB4)))),(
unsigned int)(yyyylsp[0]).first_line),Cyc_yyget_YY9(yyyyvsp[1])));}break;}case
297: _LL346: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);yyval=Cyc_YY9(Cyc_Absyn_ifthenelse_stmt(
Cyc_yyget_YY3(yyyyvsp[2]),Cyc_yyget_YY9(yyyyvsp[4]),Cyc_Absyn_skip_stmt(0),(
unsigned int)(yyyylsp[0]).first_line));break;}case 298: _LL347: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 6),sizeof(union Cyc_YYSTYPE),7);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);yyval=
Cyc_YY9(Cyc_Absyn_ifthenelse_stmt(Cyc_yyget_YY3(yyyyvsp[2]),Cyc_yyget_YY9(
yyyyvsp[4]),Cyc_yyget_YY9(yyyyvsp[6]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 299: _LL348: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 6),sizeof(union Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);yyval=Cyc_YY9(Cyc_Absyn_switch_stmt(
Cyc_yyget_YY3(yyyyvsp[2]),Cyc_yyget_YY10(yyyyvsp[5]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 300: _LL349: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct Cyc_Absyn_Exp*_tmp740=Cyc_Absyn_unknownid_exp(
Cyc_yyget_QualId_tok(yyyyvsp[1]),(unsigned int)(yyyylsp[1]).first_line);yyval=
Cyc_YY9(Cyc_Absyn_switch_stmt(_tmp740,Cyc_yyget_YY10(yyyyvsp[3]),(unsigned int)(
yyyylsp[0]).first_line));break;}case 301: _LL34A: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 7),sizeof(union Cyc_YYSTYPE),8);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 7),sizeof(struct Cyc_Yyltype),8);struct Cyc_Absyn_Exp*_tmp741=Cyc_Absyn_tuple_exp(
Cyc_yyget_YY5(yyyyvsp[3]),(unsigned int)(yyyylsp[1]).first_line);yyval=Cyc_YY9(
Cyc_Absyn_switch_stmt(_tmp741,Cyc_yyget_YY10(yyyyvsp[6]),(unsigned int)(yyyylsp[
0]).first_line));break;}case 302: _LL34B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 5),sizeof(union Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);yyval=Cyc_YY9(Cyc_Absyn_trycatch_stmt(
Cyc_yyget_YY9(yyyyvsp[1]),Cyc_yyget_YY10(yyyyvsp[4]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 303: _LL34C: yyval=Cyc_YY10(0);break;case 304: _LL34D: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct
Cyc_Absyn_Switch_clause*_tmpDB8;struct Cyc_List_List*_tmpDB7;yyval=Cyc_YY10(((
_tmpDB7=_cycalloc(sizeof(*_tmpDB7)),((_tmpDB7->hd=((_tmpDB8=_cycalloc(sizeof(*
_tmpDB8)),((_tmpDB8->pattern=Cyc_Absyn_new_pat((void*)& Cyc_Absyn_Wild_p_val,(
unsigned int)(yyyylsp[0]).first_line),((_tmpDB8->pat_vars=0,((_tmpDB8->where_clause=
0,((_tmpDB8->body=Cyc_yyget_YY9(yyyyvsp[2]),((_tmpDB8->loc=(unsigned int)(
yyyylsp[0]).first_line,_tmpDB8)))))))))))),((_tmpDB7->tl=0,_tmpDB7)))))));}
break;}case 305: _LL34E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_Switch_clause*
_tmpDBB;struct Cyc_List_List*_tmpDBA;yyval=Cyc_YY10(((_tmpDBA=_cycalloc(sizeof(*
_tmpDBA)),((_tmpDBA->hd=((_tmpDBB=_cycalloc(sizeof(*_tmpDBB)),((_tmpDBB->pattern=
Cyc_yyget_YY11(yyyyvsp[1]),((_tmpDBB->pat_vars=0,((_tmpDBB->where_clause=0,((
_tmpDBB->body=Cyc_Absyn_fallthru_stmt(0,(unsigned int)(yyyylsp[2]).first_line),((
_tmpDBB->loc=(unsigned int)(yyyylsp[0]).first_line,_tmpDBB)))))))))))),((_tmpDBA->tl=
Cyc_yyget_YY10(yyyyvsp[3]),_tmpDBA)))))));}break;}case 306: _LL34F: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct
Cyc_Absyn_Switch_clause*_tmpDBE;struct Cyc_List_List*_tmpDBD;yyval=Cyc_YY10(((
_tmpDBD=_cycalloc(sizeof(*_tmpDBD)),((_tmpDBD->hd=((_tmpDBE=_cycalloc(sizeof(*
_tmpDBE)),((_tmpDBE->pattern=Cyc_yyget_YY11(yyyyvsp[1]),((_tmpDBE->pat_vars=0,((
_tmpDBE->where_clause=0,((_tmpDBE->body=Cyc_yyget_YY9(yyyyvsp[3]),((_tmpDBE->loc=(
unsigned int)(yyyylsp[0]).first_line,_tmpDBE)))))))))))),((_tmpDBD->tl=Cyc_yyget_YY10(
yyyyvsp[4]),_tmpDBD)))))));}break;}case 307: _LL350: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 5),sizeof(union Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);{struct Cyc_Absyn_Switch_clause*
_tmpDC1;struct Cyc_List_List*_tmpDC0;yyval=Cyc_YY10(((_tmpDC0=_cycalloc(sizeof(*
_tmpDC0)),((_tmpDC0->hd=((_tmpDC1=_cycalloc(sizeof(*_tmpDC1)),((_tmpDC1->pattern=
Cyc_yyget_YY11(yyyyvsp[1]),((_tmpDC1->pat_vars=0,((_tmpDC1->where_clause=(struct
Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[3]),((_tmpDC1->body=Cyc_Absyn_fallthru_stmt(
0,(unsigned int)(yyyylsp[4]).first_line),((_tmpDC1->loc=(unsigned int)(yyyylsp[0]).first_line,
_tmpDC1)))))))))))),((_tmpDC0->tl=Cyc_yyget_YY10(yyyyvsp[5]),_tmpDC0)))))));}
break;}case 308: _LL351: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 6),sizeof(union Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);{struct Cyc_Absyn_Switch_clause*
_tmpDC4;struct Cyc_List_List*_tmpDC3;yyval=Cyc_YY10(((_tmpDC3=_cycalloc(sizeof(*
_tmpDC3)),((_tmpDC3->hd=((_tmpDC4=_cycalloc(sizeof(*_tmpDC4)),((_tmpDC4->pattern=
Cyc_yyget_YY11(yyyyvsp[1]),((_tmpDC4->pat_vars=0,((_tmpDC4->where_clause=(struct
Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[3]),((_tmpDC4->body=Cyc_yyget_YY9(yyyyvsp[5]),((
_tmpDC4->loc=(unsigned int)(yyyylsp[0]).first_line,_tmpDC4)))))))))))),((_tmpDC3->tl=
Cyc_yyget_YY10(yyyyvsp[6]),_tmpDC3)))))));}break;}case 309: _LL352: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);yyval=
Cyc_YY9(Cyc_Absyn_while_stmt(Cyc_yyget_YY3(yyyyvsp[2]),Cyc_yyget_YY9(yyyyvsp[4]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 310: _LL353: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 6),sizeof(union Cyc_YYSTYPE),7);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);yyval=
Cyc_YY9(Cyc_Absyn_do_stmt(Cyc_yyget_YY9(yyyyvsp[1]),Cyc_yyget_YY3(yyyyvsp[4]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 311: _LL354: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 5),sizeof(union Cyc_YYSTYPE),6);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);yyval=
Cyc_YY9(Cyc_Absyn_for_stmt(Cyc_Absyn_false_exp(0),Cyc_Absyn_true_exp(0),Cyc_Absyn_false_exp(
0),Cyc_yyget_YY9(yyyyvsp[5]),(unsigned int)(yyyylsp[0]).first_line));break;}case
312: _LL355: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 6),sizeof(union
Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);yyval=Cyc_YY9(Cyc_Absyn_for_stmt(
Cyc_Absyn_false_exp(0),Cyc_Absyn_true_exp(0),Cyc_yyget_YY3(yyyyvsp[4]),Cyc_yyget_YY9(
yyyyvsp[6]),(unsigned int)(yyyylsp[0]).first_line));break;}case 313: _LL356: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 6),sizeof(union
Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);yyval=Cyc_YY9(Cyc_Absyn_for_stmt(
Cyc_Absyn_false_exp(0),Cyc_yyget_YY3(yyyyvsp[3]),Cyc_Absyn_false_exp(0),Cyc_yyget_YY9(
yyyyvsp[6]),(unsigned int)(yyyylsp[0]).first_line));break;}case 314: _LL357: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 7),sizeof(union
Cyc_YYSTYPE),8);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 7),sizeof(struct Cyc_Yyltype),8);yyval=Cyc_YY9(Cyc_Absyn_for_stmt(
Cyc_Absyn_false_exp(0),Cyc_yyget_YY3(yyyyvsp[3]),Cyc_yyget_YY3(yyyyvsp[5]),Cyc_yyget_YY9(
yyyyvsp[7]),(unsigned int)(yyyylsp[0]).first_line));break;}case 315: _LL358: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 6),sizeof(union
Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);yyval=Cyc_YY9(Cyc_Absyn_for_stmt(
Cyc_yyget_YY3(yyyyvsp[2]),Cyc_Absyn_true_exp(0),Cyc_Absyn_false_exp(0),Cyc_yyget_YY9(
yyyyvsp[6]),(unsigned int)(yyyylsp[0]).first_line));break;}case 316: _LL359: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 7),sizeof(union
Cyc_YYSTYPE),8);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 7),sizeof(struct Cyc_Yyltype),8);yyval=Cyc_YY9(Cyc_Absyn_for_stmt(
Cyc_yyget_YY3(yyyyvsp[2]),Cyc_Absyn_true_exp(0),Cyc_yyget_YY3(yyyyvsp[5]),Cyc_yyget_YY9(
yyyyvsp[7]),(unsigned int)(yyyylsp[0]).first_line));break;}case 317: _LL35A: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 7),sizeof(union
Cyc_YYSTYPE),8);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 7),sizeof(struct Cyc_Yyltype),8);yyval=Cyc_YY9(Cyc_Absyn_for_stmt(
Cyc_yyget_YY3(yyyyvsp[2]),Cyc_yyget_YY3(yyyyvsp[4]),Cyc_Absyn_false_exp(0),Cyc_yyget_YY9(
yyyyvsp[7]),(unsigned int)(yyyylsp[0]).first_line));break;}case 318: _LL35B: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 8),sizeof(union
Cyc_YYSTYPE),9);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 8),sizeof(struct Cyc_Yyltype),9);yyval=Cyc_YY9(Cyc_Absyn_for_stmt(
Cyc_yyget_YY3(yyyyvsp[2]),Cyc_yyget_YY3(yyyyvsp[4]),Cyc_yyget_YY3(yyyyvsp[6]),
Cyc_yyget_YY9(yyyyvsp[8]),(unsigned int)(yyyylsp[0]).first_line));break;}case 319:
_LL35C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 5),sizeof(union
Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);struct Cyc_List_List*_tmp74C=Cyc_yyget_YY18(
yyyyvsp[2]);struct Cyc_Absyn_Stmt*_tmp74D=Cyc_Absyn_for_stmt(Cyc_Absyn_false_exp(
0),Cyc_Absyn_true_exp(0),Cyc_Absyn_false_exp(0),Cyc_yyget_YY9(yyyyvsp[5]),(
unsigned int)(yyyylsp[0]).first_line);yyval=Cyc_YY9(Cyc_Parse_flatten_declarations(
_tmp74C,_tmp74D));break;}case 320: _LL35D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 6),sizeof(union Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);struct Cyc_List_List*_tmp74E=Cyc_yyget_YY18(
yyyyvsp[2]);struct Cyc_Absyn_Stmt*_tmp74F=Cyc_Absyn_for_stmt(Cyc_Absyn_false_exp(
0),Cyc_yyget_YY3(yyyyvsp[3]),Cyc_Absyn_false_exp(0),Cyc_yyget_YY9(yyyyvsp[6]),(
unsigned int)(yyyylsp[0]).first_line);yyval=Cyc_YY9(Cyc_Parse_flatten_declarations(
_tmp74E,_tmp74F));break;}case 321: _LL35E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 6),sizeof(union Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);struct Cyc_List_List*_tmp750=Cyc_yyget_YY18(
yyyyvsp[2]);struct Cyc_Absyn_Stmt*_tmp751=Cyc_Absyn_for_stmt(Cyc_Absyn_false_exp(
0),Cyc_Absyn_true_exp(0),Cyc_yyget_YY3(yyyyvsp[4]),Cyc_yyget_YY9(yyyyvsp[6]),(
unsigned int)(yyyylsp[0]).first_line);yyval=Cyc_YY9(Cyc_Parse_flatten_declarations(
_tmp750,_tmp751));break;}case 322: _LL35F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 7),sizeof(union Cyc_YYSTYPE),8);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 7),sizeof(struct Cyc_Yyltype),8);struct Cyc_List_List*_tmp752=Cyc_yyget_YY18(
yyyyvsp[2]);struct Cyc_Absyn_Stmt*_tmp753=Cyc_Absyn_for_stmt(Cyc_Absyn_false_exp(
0),Cyc_yyget_YY3(yyyyvsp[3]),Cyc_yyget_YY3(yyyyvsp[5]),Cyc_yyget_YY9(yyyyvsp[7]),(
unsigned int)(yyyylsp[0]).first_line);yyval=Cyc_YY9(Cyc_Parse_flatten_declarations(
_tmp752,_tmp753));break;}case 323: _LL360: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _dyneither_ptr*_tmpDC5;
yyval=Cyc_YY9(Cyc_Absyn_goto_stmt(((_tmpDC5=_cycalloc(sizeof(*_tmpDC5)),((
_tmpDC5[0]=Cyc_yyget_String_tok(yyyyvsp[1]),_tmpDC5)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 324: _LL361: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY9(Cyc_Absyn_continue_stmt((
unsigned int)(yyyylsp[0]).first_line));break;}case 325: _LL362: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=
Cyc_YY9(Cyc_Absyn_break_stmt((unsigned int)(yyyylsp[0]).first_line));break;}case
326: _LL363: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY9(Cyc_Absyn_return_stmt(
0,(unsigned int)(yyyylsp[0]).first_line));break;}case 327: _LL364: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=
Cyc_YY9(Cyc_Absyn_return_stmt((struct Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[1]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 328: _LL365: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=
Cyc_YY9(Cyc_Absyn_fallthru_stmt(0,(unsigned int)(yyyylsp[0]).first_line));break;}
case 329: _LL366: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);yyval=Cyc_YY9(Cyc_Absyn_fallthru_stmt(
0,(unsigned int)(yyyylsp[0]).first_line));break;}case 330: _LL367: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);yyval=
Cyc_YY9(Cyc_Absyn_fallthru_stmt(Cyc_yyget_YY5(yyyyvsp[2]),(unsigned int)(yyyylsp[
0]).first_line));break;}case 331: _LL368: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 332:
_LL369: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 333:
_LL36A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_conditional_exp(Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),Cyc_yyget_YY3(
yyyyvsp[2]),Cyc_yyget_YY3(yyyyvsp[4]),(unsigned int)(yyyylsp[0]).first_line)));
break;}case 334: _LL36B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 335:
_LL36C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_or_exp(Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case 336: _LL36D: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 337:
_LL36E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_and_exp(Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case 338: _LL36F: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 339:
_LL370: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Bitor,Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),
Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case
340: _LL371: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 341:
_LL372: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Bitxor,Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),
Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case
342: _LL373: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 343:
_LL374: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Bitand,Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),
Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case
344: _LL375: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 345:
_LL376: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_eq_exp(Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case 346: _LL377: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_neq_exp(Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case 347: _LL378: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 348:
_LL379: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_lt_exp(Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case 349: _LL37A: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_gt_exp(Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case 350: _LL37B: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_lte_exp(Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case 351: _LL37C: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_gte_exp(Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case 352: _LL37D: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 353:
_LL37E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Bitlshift,Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[
0])),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}
case 354: _LL37F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Bitlrshift,Cyc_Parse_pat2exp(Cyc_yyget_YY11(
yyyyvsp[0])),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));
break;}case 355: _LL380: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 356:
_LL381: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Plus,Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),
Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case
357: _LL382: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Minus,Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),
Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case
358: _LL383: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 359:
_LL384: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Times,Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),
Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case
360: _LL385: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Div,Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),
Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case
361: _LL386: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim2_exp(Cyc_Absyn_Mod,Cyc_Parse_pat2exp(Cyc_yyget_YY11(yyyyvsp[0])),
Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line)));break;}case
362: _LL387: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 363:
_LL388: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);void*_tmp755=Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[1]),(unsigned int)(yyyylsp[1]).first_line);yyval=Cyc_YY11(
Cyc_Absyn_exp_pat(Cyc_Absyn_cast_exp(_tmp755,Cyc_yyget_YY3(yyyyvsp[3]),1,Cyc_Absyn_Unknown_coercion,(
unsigned int)(yyyylsp[0]).first_line)));break;}case 364: _LL389: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
yyyyvsp[0];break;}case 365: _LL38A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_yyget_YY3(yyyyvsp[1])));break;}case 366: _LL38B: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_prim1_exp(Cyc_yyget_YY7(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[1]),(
unsigned int)(yyyylsp[0]).first_line)));break;}case 367: _LL38C: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);void*
_tmp756=Cyc_Parse_type_name_to_type(Cyc_yyget_YY37(yyyyvsp[2]),(unsigned int)(
yyyylsp[2]).first_line);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(Cyc_Absyn_sizeoftyp_exp(
_tmp756,(unsigned int)(yyyylsp[0]).first_line)));break;}case 368: _LL38D: {union
Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(
yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);
struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(
struct Cyc_Yyltype),2);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(Cyc_Absyn_sizeofexp_exp(
Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line)));break;}case
369: _LL38E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 5),sizeof(union
Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);{struct Cyc_Absyn_StructField_struct
_tmpDCB;struct _dyneither_ptr*_tmpDCA;struct Cyc_Absyn_StructField_struct*_tmpDC9;
yyval=Cyc_YY11(Cyc_Absyn_exp_pat(Cyc_Absyn_offsetof_exp((*Cyc_yyget_YY37(yyyyvsp[
2])).f3,(void*)((_tmpDC9=_cycalloc(sizeof(*_tmpDC9)),((_tmpDC9[0]=((_tmpDCB.tag=
0,((_tmpDCB.f1=((_tmpDCA=_cycalloc(sizeof(*_tmpDCA)),((_tmpDCA[0]=Cyc_yyget_String_tok(
yyyyvsp[4]),_tmpDCA)))),_tmpDCB)))),_tmpDC9)))),(unsigned int)(yyyylsp[0]).first_line)));}
break;}case 370: _LL38F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 5),sizeof(union Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);void*_tmp75A=Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[2]),(unsigned int)(yyyylsp[2]).first_line);{struct Cyc_Absyn_TupleIndex_struct
_tmpDCE;struct Cyc_Absyn_TupleIndex_struct*_tmpDCD;yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_Absyn_offsetof_exp(_tmp75A,(void*)((_tmpDCD=_cycalloc_atomic(sizeof(*_tmpDCD)),((
_tmpDCD[0]=((_tmpDCE.tag=1,((_tmpDCE.f1=(unsigned int)(Cyc_yyget_Int_tok(yyyyvsp[
4])).f2,_tmpDCE)))),_tmpDCD)))),(unsigned int)(yyyylsp[0]).first_line)));}break;}
case 371: _LL390: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 372:
_LL391: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 373:
_LL392: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY11(Cyc_Absyn_new_pat((
void*)& Cyc_Absyn_Wild_p_val,(unsigned int)(yyyylsp[0]).first_line));break;}case
374: _LL393: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY11(Cyc_Absyn_exp_pat(
Cyc_yyget_YY3(yyyyvsp[1])));break;}case 375: _LL394: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);struct Cyc_Absyn_Exp*e=Cyc_yyget_YY3(
yyyyvsp[0]);{void*_tmp75D=e->r;union Cyc_Absyn_Cnst _tmp75F;struct _tuple3 _tmp760;
enum Cyc_Absyn_Sign _tmp761;char _tmp762;union Cyc_Absyn_Cnst _tmp764;struct _tuple4
_tmp765;enum Cyc_Absyn_Sign _tmp766;short _tmp767;union Cyc_Absyn_Cnst _tmp769;
struct _tuple5 _tmp76A;enum Cyc_Absyn_Sign _tmp76B;int _tmp76C;union Cyc_Absyn_Cnst
_tmp76E;struct _tuple7 _tmp76F;struct _dyneither_ptr _tmp770;int _tmp771;union Cyc_Absyn_Cnst
_tmp773;int _tmp774;union Cyc_Absyn_Cnst _tmp776;struct _dyneither_ptr _tmp777;union
Cyc_Absyn_Cnst _tmp779;struct _dyneither_ptr _tmp77A;union Cyc_Absyn_Cnst _tmp77C;
struct _tuple6 _tmp77D;_LL397: {struct Cyc_Absyn_Const_e_struct*_tmp75E=(struct Cyc_Absyn_Const_e_struct*)
_tmp75D;if(_tmp75E->tag != 0)goto _LL399;else{_tmp75F=_tmp75E->f1;if((_tmp75F.Char_c).tag
!= 2)goto _LL399;_tmp760=(struct _tuple3)(_tmp75F.Char_c).val;_tmp761=_tmp760.f1;
_tmp762=_tmp760.f2;}}_LL398:{struct Cyc_Absyn_Char_p_struct _tmpDD1;struct Cyc_Absyn_Char_p_struct*
_tmpDD0;yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpDD0=_cycalloc_atomic(
sizeof(*_tmpDD0)),((_tmpDD0[0]=((_tmpDD1.tag=10,((_tmpDD1.f1=_tmp762,_tmpDD1)))),
_tmpDD0)))),e->loc));}goto _LL396;_LL399: {struct Cyc_Absyn_Const_e_struct*_tmp763=(
struct Cyc_Absyn_Const_e_struct*)_tmp75D;if(_tmp763->tag != 0)goto _LL39B;else{
_tmp764=_tmp763->f1;if((_tmp764.Short_c).tag != 4)goto _LL39B;_tmp765=(struct
_tuple4)(_tmp764.Short_c).val;_tmp766=_tmp765.f1;_tmp767=_tmp765.f2;}}_LL39A:{
struct Cyc_Absyn_Int_p_struct _tmpDD4;struct Cyc_Absyn_Int_p_struct*_tmpDD3;yyval=
Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpDD3=_cycalloc_atomic(sizeof(*_tmpDD3)),((
_tmpDD3[0]=((_tmpDD4.tag=9,((_tmpDD4.f1=_tmp766,((_tmpDD4.f2=(int)_tmp767,
_tmpDD4)))))),_tmpDD3)))),e->loc));}goto _LL396;_LL39B: {struct Cyc_Absyn_Const_e_struct*
_tmp768=(struct Cyc_Absyn_Const_e_struct*)_tmp75D;if(_tmp768->tag != 0)goto _LL39D;
else{_tmp769=_tmp768->f1;if((_tmp769.Int_c).tag != 5)goto _LL39D;_tmp76A=(struct
_tuple5)(_tmp769.Int_c).val;_tmp76B=_tmp76A.f1;_tmp76C=_tmp76A.f2;}}_LL39C:{
struct Cyc_Absyn_Int_p_struct _tmpDD7;struct Cyc_Absyn_Int_p_struct*_tmpDD6;yyval=
Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpDD6=_cycalloc_atomic(sizeof(*_tmpDD6)),((
_tmpDD6[0]=((_tmpDD7.tag=9,((_tmpDD7.f1=_tmp76B,((_tmpDD7.f2=_tmp76C,_tmpDD7)))))),
_tmpDD6)))),e->loc));}goto _LL396;_LL39D: {struct Cyc_Absyn_Const_e_struct*_tmp76D=(
struct Cyc_Absyn_Const_e_struct*)_tmp75D;if(_tmp76D->tag != 0)goto _LL39F;else{
_tmp76E=_tmp76D->f1;if((_tmp76E.Float_c).tag != 7)goto _LL39F;_tmp76F=(struct
_tuple7)(_tmp76E.Float_c).val;_tmp770=_tmp76F.f1;_tmp771=_tmp76F.f2;}}_LL39E:{
struct Cyc_Absyn_Float_p_struct _tmpDDA;struct Cyc_Absyn_Float_p_struct*_tmpDD9;
yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpDD9=_cycalloc(sizeof(*_tmpDD9)),((
_tmpDD9[0]=((_tmpDDA.tag=11,((_tmpDDA.f1=_tmp770,((_tmpDDA.f2=_tmp771,_tmpDDA)))))),
_tmpDD9)))),e->loc));}goto _LL396;_LL39F: {struct Cyc_Absyn_Const_e_struct*_tmp772=(
struct Cyc_Absyn_Const_e_struct*)_tmp75D;if(_tmp772->tag != 0)goto _LL3A1;else{
_tmp773=_tmp772->f1;if((_tmp773.Null_c).tag != 1)goto _LL3A1;_tmp774=(int)(_tmp773.Null_c).val;}}
_LL3A0: yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)& Cyc_Absyn_Null_p_val,e->loc));
goto _LL396;_LL3A1: {struct Cyc_Absyn_Const_e_struct*_tmp775=(struct Cyc_Absyn_Const_e_struct*)
_tmp75D;if(_tmp775->tag != 0)goto _LL3A3;else{_tmp776=_tmp775->f1;if((_tmp776.String_c).tag
!= 8)goto _LL3A3;_tmp777=(struct _dyneither_ptr)(_tmp776.String_c).val;}}_LL3A2:{
const char*_tmpDDB;Cyc_Parse_err(((_tmpDDB="strings cannot occur within patterns",
_tag_dyneither(_tmpDDB,sizeof(char),37))),(unsigned int)(yyyylsp[0]).first_line);}
goto _LL396;_LL3A3: {struct Cyc_Absyn_Const_e_struct*_tmp778=(struct Cyc_Absyn_Const_e_struct*)
_tmp75D;if(_tmp778->tag != 0)goto _LL3A5;else{_tmp779=_tmp778->f1;if((_tmp779.Wstring_c).tag
!= 9)goto _LL3A5;_tmp77A=(struct _dyneither_ptr)(_tmp779.Wstring_c).val;}}_LL3A4:{
const char*_tmpDDC;Cyc_Parse_err(((_tmpDDC="strings cannot occur within patterns",
_tag_dyneither(_tmpDDC,sizeof(char),37))),(unsigned int)(yyyylsp[0]).first_line);}
goto _LL396;_LL3A5: {struct Cyc_Absyn_Const_e_struct*_tmp77B=(struct Cyc_Absyn_Const_e_struct*)
_tmp75D;if(_tmp77B->tag != 0)goto _LL3A7;else{_tmp77C=_tmp77B->f1;if((_tmp77C.LongLong_c).tag
!= 6)goto _LL3A7;_tmp77D=(struct _tuple6)(_tmp77C.LongLong_c).val;}}_LL3A6:{const
char*_tmpDDD;Cyc_Parse_unimp(((_tmpDDD="long long's in patterns",_tag_dyneither(
_tmpDDD,sizeof(char),24))),(unsigned int)(yyyylsp[0]).first_line);}goto _LL396;
_LL3A7:;_LL3A8: {const char*_tmpDDE;Cyc_Parse_err(((_tmpDDE="bad constant in case",
_tag_dyneither(_tmpDDE,sizeof(char),21))),(unsigned int)(yyyylsp[0]).first_line);}
_LL396:;}break;}case 376: _LL395: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Absyn_UnknownId_p_struct
_tmpDE1;struct Cyc_Absyn_UnknownId_p_struct*_tmpDE0;yyval=Cyc_YY11(Cyc_Absyn_new_pat((
void*)((_tmpDE0=_cycalloc(sizeof(*_tmpDE0)),((_tmpDE0[0]=((_tmpDE1.tag=14,((
_tmpDE1.f1=Cyc_yyget_QualId_tok(yyyyvsp[0]),_tmpDE1)))),_tmpDE0)))),(
unsigned int)(yyyylsp[0]).first_line));}break;}case 377: _LL3A9: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{const
char*_tmpDE2;if(Cyc_strcmp((struct _dyneither_ptr)Cyc_yyget_String_tok(yyyyvsp[1]),((
_tmpDE2="as",_tag_dyneither(_tmpDE2,sizeof(char),3))))!= 0){const char*_tmpDE3;
Cyc_Parse_err(((_tmpDE3="expecting `as'",_tag_dyneither(_tmpDE3,sizeof(char),15))),(
unsigned int)(yyyylsp[1]).first_line);}}{struct Cyc_Absyn_Var_p_struct _tmpDED;
struct _dyneither_ptr*_tmpDEC;struct _tuple0*_tmpDEB;struct Cyc_Absyn_Var_p_struct*
_tmpDEA;yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpDEA=_cycalloc(sizeof(*
_tmpDEA)),((_tmpDEA[0]=((_tmpDED.tag=1,((_tmpDED.f1=Cyc_Absyn_new_vardecl(((
_tmpDEB=_cycalloc(sizeof(*_tmpDEB)),((_tmpDEB->f1=Cyc_Absyn_Loc_n,((_tmpDEB->f2=((
_tmpDEC=_cycalloc(sizeof(*_tmpDEC)),((_tmpDEC[0]=Cyc_yyget_String_tok(yyyyvsp[0]),
_tmpDEC)))),_tmpDEB)))))),(void*)& Cyc_Absyn_VoidType_val,0),((_tmpDED.f2=Cyc_yyget_YY11(
yyyyvsp[2]),_tmpDED)))))),_tmpDEA)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 378: _LL3AA: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_List_List*_tmp793;int
_tmp794;struct _tuple22 _tmp792=*Cyc_yyget_YY12(yyyyvsp[2]);_tmp793=_tmp792.f1;
_tmp794=_tmp792.f2;{struct Cyc_Absyn_Tuple_p_struct _tmpDF0;struct Cyc_Absyn_Tuple_p_struct*
_tmpDEF;yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpDEF=_cycalloc(sizeof(*
_tmpDEF)),((_tmpDEF[0]=((_tmpDF0.tag=4,((_tmpDF0.f1=_tmp793,((_tmpDF0.f2=_tmp794,
_tmpDF0)))))),_tmpDEF)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case
379: _LL3AB: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_List_List*_tmp798;int
_tmp799;struct _tuple22 _tmp797=*Cyc_yyget_YY12(yyyyvsp[2]);_tmp798=_tmp797.f1;
_tmp799=_tmp797.f2;{struct Cyc_Absyn_UnknownCall_p_struct _tmpDF3;struct Cyc_Absyn_UnknownCall_p_struct*
_tmpDF2;yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpDF2=_cycalloc(sizeof(*
_tmpDF2)),((_tmpDF2[0]=((_tmpDF3.tag=15,((_tmpDF3.f1=Cyc_yyget_QualId_tok(
yyyyvsp[0]),((_tmpDF3.f2=_tmp798,((_tmpDF3.f3=_tmp799,_tmpDF3)))))))),_tmpDF2)))),(
unsigned int)(yyyylsp[0]).first_line));}break;}case 380: _LL3AC: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);struct
Cyc_List_List*_tmp79D;int _tmp79E;struct _tuple22 _tmp79C=*Cyc_yyget_YY16(yyyyvsp[3]);
_tmp79D=_tmp79C.f1;_tmp79E=_tmp79C.f2;{struct Cyc_List_List*_tmp79F=((struct Cyc_List_List*(*)(
struct Cyc_Absyn_Tvar*(*f)(unsigned int,void*),unsigned int env,struct Cyc_List_List*
x))Cyc_List_map_c)(Cyc_Parse_typ2tvar,(unsigned int)(yyyylsp[2]).first_line,Cyc_yyget_YY40(
yyyyvsp[2]));{struct Cyc_Absyn_Aggr_p_struct _tmpDF9;struct Cyc_Absyn_AggrInfo*
_tmpDF8;struct Cyc_Absyn_Aggr_p_struct*_tmpDF7;yyval=Cyc_YY11(Cyc_Absyn_new_pat((
void*)((_tmpDF7=_cycalloc(sizeof(*_tmpDF7)),((_tmpDF7[0]=((_tmpDF9.tag=6,((
_tmpDF9.f1=((_tmpDF8=_cycalloc(sizeof(*_tmpDF8)),((_tmpDF8->aggr_info=Cyc_Absyn_UnknownAggr(
Cyc_Absyn_StructA,Cyc_yyget_QualId_tok(yyyyvsp[0]),0),((_tmpDF8->targs=0,_tmpDF8)))))),((
_tmpDF9.f2=_tmp79F,((_tmpDF9.f3=_tmp79D,((_tmpDF9.f4=_tmp79E,_tmpDF9)))))))))),
_tmpDF7)))),(unsigned int)(yyyylsp[0]).first_line));}break;};}case 381: _LL3AD: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_List_List*_tmp7A4;int
_tmp7A5;struct _tuple22 _tmp7A3=*Cyc_yyget_YY16(yyyyvsp[2]);_tmp7A4=_tmp7A3.f1;
_tmp7A5=_tmp7A3.f2;{struct Cyc_List_List*_tmp7A6=((struct Cyc_List_List*(*)(struct
Cyc_Absyn_Tvar*(*f)(unsigned int,void*),unsigned int env,struct Cyc_List_List*x))
Cyc_List_map_c)(Cyc_Parse_typ2tvar,(unsigned int)(yyyylsp[1]).first_line,Cyc_yyget_YY40(
yyyyvsp[1]));{struct Cyc_Absyn_Aggr_p_struct _tmpDFC;struct Cyc_Absyn_Aggr_p_struct*
_tmpDFB;yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpDFB=_cycalloc(sizeof(*
_tmpDFB)),((_tmpDFB[0]=((_tmpDFC.tag=6,((_tmpDFC.f1=0,((_tmpDFC.f2=_tmp7A6,((
_tmpDFC.f3=_tmp7A4,((_tmpDFC.f4=_tmp7A5,_tmpDFC)))))))))),_tmpDFB)))),(
unsigned int)(yyyylsp[0]).first_line));}break;};}case 382: _LL3AE: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct
Cyc_Absyn_Pointer_p_struct _tmpDFF;struct Cyc_Absyn_Pointer_p_struct*_tmpDFE;yyval=
Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpDFE=_cycalloc(sizeof(*_tmpDFE)),((
_tmpDFE[0]=((_tmpDFF.tag=5,((_tmpDFF.f1=Cyc_yyget_YY11(yyyyvsp[1]),_tmpDFF)))),
_tmpDFE)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 383: _LL3AF: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct Cyc_Absyn_Pointer_p_struct
_tmpE09;struct Cyc_Absyn_Pointer_p_struct _tmpE08;struct Cyc_Absyn_Pointer_p_struct*
_tmpE07;struct Cyc_Absyn_Pointer_p_struct*_tmpE06;yyval=Cyc_YY11(Cyc_Absyn_new_pat((
void*)((_tmpE06=_cycalloc(sizeof(*_tmpE06)),((_tmpE06[0]=((_tmpE09.tag=5,((
_tmpE09.f1=Cyc_Absyn_new_pat((void*)((_tmpE07=_cycalloc(sizeof(*_tmpE07)),((
_tmpE07[0]=((_tmpE08.tag=5,((_tmpE08.f1=Cyc_yyget_YY11(yyyyvsp[1]),_tmpE08)))),
_tmpE07)))),(unsigned int)(yyyylsp[0]).first_line),_tmpE09)))),_tmpE06)))),(
unsigned int)(yyyylsp[0]).first_line));}break;}case 384: _LL3B0: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct
Cyc_Absyn_Reference_p_struct _tmpE13;struct _dyneither_ptr*_tmpE12;struct _tuple0*
_tmpE11;struct Cyc_Absyn_Reference_p_struct*_tmpE10;yyval=Cyc_YY11(Cyc_Absyn_new_pat((
void*)((_tmpE10=_cycalloc(sizeof(*_tmpE10)),((_tmpE10[0]=((_tmpE13.tag=2,((
_tmpE13.f1=Cyc_Absyn_new_vardecl(((_tmpE11=_cycalloc(sizeof(*_tmpE11)),((_tmpE11->f1=
Cyc_Absyn_Loc_n,((_tmpE11->f2=((_tmpE12=_cycalloc(sizeof(*_tmpE12)),((_tmpE12[0]=
Cyc_yyget_String_tok(yyyyvsp[1]),_tmpE12)))),_tmpE11)))))),(void*)& Cyc_Absyn_VoidType_val,
0),((_tmpE13.f2=Cyc_Absyn_new_pat((void*)& Cyc_Absyn_Wild_p_val,(unsigned int)(
yyyylsp[1]).first_line),_tmpE13)))))),_tmpE10)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 385: _LL3B1: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{const char*_tmpE14;if(Cyc_strcmp((
struct _dyneither_ptr)Cyc_yyget_String_tok(yyyyvsp[2]),((_tmpE14="as",
_tag_dyneither(_tmpE14,sizeof(char),3))))!= 0){const char*_tmpE15;Cyc_Parse_err(((
_tmpE15="expecting `as'",_tag_dyneither(_tmpE15,sizeof(char),15))),(unsigned int)(
yyyylsp[2]).first_line);}}{struct Cyc_Absyn_Reference_p_struct _tmpE1F;struct
_dyneither_ptr*_tmpE1E;struct _tuple0*_tmpE1D;struct Cyc_Absyn_Reference_p_struct*
_tmpE1C;yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpE1C=_cycalloc(sizeof(*
_tmpE1C)),((_tmpE1C[0]=((_tmpE1F.tag=2,((_tmpE1F.f1=Cyc_Absyn_new_vardecl(((
_tmpE1D=_cycalloc(sizeof(*_tmpE1D)),((_tmpE1D->f1=Cyc_Absyn_Loc_n,((_tmpE1D->f2=((
_tmpE1E=_cycalloc(sizeof(*_tmpE1E)),((_tmpE1E[0]=Cyc_yyget_String_tok(yyyyvsp[1]),
_tmpE1E)))),_tmpE1D)))))),(void*)& Cyc_Absyn_VoidType_val,0),((_tmpE1F.f2=Cyc_yyget_YY11(
yyyyvsp[3]),_tmpE1F)))))),_tmpE1C)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 386: _LL3B2: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);void*_tmp7B9=Cyc_Parse_id2type(Cyc_yyget_String_tok(
yyyyvsp[2]),Cyc_Tcutil_kind_to_bound(& Cyc_Tcutil_ik));{struct Cyc_Absyn_TagInt_p_struct
_tmpE30;struct Cyc_Absyn_TagType_struct _tmpE2F;struct Cyc_Absyn_TagType_struct*
_tmpE2E;struct _dyneither_ptr*_tmpE2D;struct _tuple0*_tmpE2C;struct Cyc_Absyn_TagInt_p_struct*
_tmpE2B;yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpE2B=_cycalloc(sizeof(*
_tmpE2B)),((_tmpE2B[0]=((_tmpE30.tag=3,((_tmpE30.f1=Cyc_Parse_typ2tvar((
unsigned int)(yyyylsp[2]).first_line,_tmp7B9),((_tmpE30.f2=Cyc_Absyn_new_vardecl(((
_tmpE2C=_cycalloc(sizeof(*_tmpE2C)),((_tmpE2C->f1=Cyc_Absyn_Loc_n,((_tmpE2C->f2=((
_tmpE2D=_cycalloc(sizeof(*_tmpE2D)),((_tmpE2D[0]=Cyc_yyget_String_tok(yyyyvsp[0]),
_tmpE2D)))),_tmpE2C)))))),(void*)((_tmpE2E=_cycalloc(sizeof(*_tmpE2E)),((_tmpE2E[
0]=((_tmpE2F.tag=19,((_tmpE2F.f1=(void*)_tmp7B9,_tmpE2F)))),_tmpE2E)))),0),
_tmpE30)))))),_tmpE2B)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case
387: _LL3B3: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);struct Cyc_Absyn_Tvar*_tmp7C0=Cyc_Tcutil_new_tvar(
Cyc_Tcutil_kind_to_bound(& Cyc_Tcutil_ik));{struct Cyc_Absyn_TagInt_p_struct
_tmpE4C;struct Cyc_Absyn_TagType_struct _tmpE4B;struct Cyc_Absyn_VarType_struct
_tmpE4A;struct Cyc_Absyn_VarType_struct*_tmpE49;struct Cyc_Absyn_TagType_struct*
_tmpE48;struct _dyneither_ptr*_tmpE47;struct _tuple0*_tmpE46;struct Cyc_Absyn_TagInt_p_struct*
_tmpE45;yyval=Cyc_YY11(Cyc_Absyn_new_pat((void*)((_tmpE45=_cycalloc(sizeof(*
_tmpE45)),((_tmpE45[0]=((_tmpE4C.tag=3,((_tmpE4C.f1=_tmp7C0,((_tmpE4C.f2=Cyc_Absyn_new_vardecl(((
_tmpE46=_cycalloc(sizeof(*_tmpE46)),((_tmpE46->f1=Cyc_Absyn_Loc_n,((_tmpE46->f2=((
_tmpE47=_cycalloc(sizeof(*_tmpE47)),((_tmpE47[0]=Cyc_yyget_String_tok(yyyyvsp[0]),
_tmpE47)))),_tmpE46)))))),(void*)((_tmpE48=_cycalloc(sizeof(*_tmpE48)),((_tmpE48[
0]=((_tmpE4B.tag=19,((_tmpE4B.f1=(void*)((void*)((_tmpE49=_cycalloc(sizeof(*
_tmpE49)),((_tmpE49[0]=((_tmpE4A.tag=2,((_tmpE4A.f1=_tmp7C0,_tmpE4A)))),_tmpE49))))),
_tmpE4B)))),_tmpE48)))),0),_tmpE4C)))))),_tmpE45)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 388: _LL3B4: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple22*_tmpE4D;yyval=Cyc_YY12(((
_tmpE4D=_cycalloc(sizeof(*_tmpE4D)),((_tmpE4D->f1=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_rev)(Cyc_yyget_YY13(yyyyvsp[0])),((_tmpE4D->f2=0,
_tmpE4D)))))));}break;}case 389: _LL3B5: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _tuple22*_tmpE4E;yyval=Cyc_YY12(((
_tmpE4E=_cycalloc(sizeof(*_tmpE4E)),((_tmpE4E->f1=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_rev)(Cyc_yyget_YY13(yyyyvsp[0])),((_tmpE4E->f2=1,
_tmpE4E)))))));}break;}case 390: _LL3B6: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple22*_tmpE4F;yyval=Cyc_YY12(((
_tmpE4F=_cycalloc(sizeof(*_tmpE4F)),((_tmpE4F->f1=0,((_tmpE4F->f2=1,_tmpE4F)))))));}
break;}case 391: _LL3B7: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_List_List*_tmpE50;yyval=
Cyc_YY13(((_tmpE50=_cycalloc(sizeof(*_tmpE50)),((_tmpE50->hd=Cyc_yyget_YY11(
yyyyvsp[0]),((_tmpE50->tl=0,_tmpE50)))))));}break;}case 392: _LL3B8: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct
Cyc_List_List*_tmpE51;yyval=Cyc_YY13(((_tmpE51=_cycalloc(sizeof(*_tmpE51)),((
_tmpE51->hd=Cyc_yyget_YY11(yyyyvsp[2]),((_tmpE51->tl=Cyc_yyget_YY13(yyyyvsp[0]),
_tmpE51)))))));}break;}case 393: _LL3B9: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple23*_tmpE52;yyval=Cyc_YY14(((
_tmpE52=_cycalloc(sizeof(*_tmpE52)),((_tmpE52->f1=0,((_tmpE52->f2=Cyc_yyget_YY11(
yyyyvsp[0]),_tmpE52)))))));}break;}case 394: _LL3BA: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);{struct _tuple23*_tmpE53;yyval=Cyc_YY14(((
_tmpE53=_cycalloc(sizeof(*_tmpE53)),((_tmpE53->f1=Cyc_yyget_YY41(yyyyvsp[0]),((
_tmpE53->f2=Cyc_yyget_YY11(yyyyvsp[1]),_tmpE53)))))));}break;}case 395: _LL3BB: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple22*_tmpE54;yyval=Cyc_YY16(((
_tmpE54=_cycalloc(sizeof(*_tmpE54)),((_tmpE54->f1=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_rev)(Cyc_yyget_YY15(yyyyvsp[0])),((_tmpE54->f2=0,
_tmpE54)))))));}break;}case 396: _LL3BC: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _tuple22*_tmpE55;yyval=Cyc_YY16(((
_tmpE55=_cycalloc(sizeof(*_tmpE55)),((_tmpE55->f1=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_rev)(Cyc_yyget_YY15(yyyyvsp[0])),((_tmpE55->f2=1,
_tmpE55)))))));}break;}case 397: _LL3BD: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _tuple22*_tmpE56;yyval=Cyc_YY16(((
_tmpE56=_cycalloc(sizeof(*_tmpE56)),((_tmpE56->f1=0,((_tmpE56->f2=1,_tmpE56)))))));}
break;}case 398: _LL3BE: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_List_List*_tmpE57;yyval=
Cyc_YY15(((_tmpE57=_cycalloc(sizeof(*_tmpE57)),((_tmpE57->hd=Cyc_yyget_YY14(
yyyyvsp[0]),((_tmpE57->tl=0,_tmpE57)))))));}break;}case 399: _LL3BF: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct
Cyc_List_List*_tmpE58;yyval=Cyc_YY15(((_tmpE58=_cycalloc(sizeof(*_tmpE58)),((
_tmpE58->hd=Cyc_yyget_YY14(yyyyvsp[2]),((_tmpE58->tl=Cyc_yyget_YY15(yyyyvsp[0]),
_tmpE58)))))));}break;}case 400: _LL3C0: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 401:
_LL3C1: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_seq_exp(
Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 402: _LL3C2: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 403:
_LL3C3: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_assignop_exp(
Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY8(yyyyvsp[1]),Cyc_yyget_YY3(yyyyvsp[2]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 404: _LL3C4: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=
Cyc_YY3(Cyc_Absyn_swap_exp(Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 405: _LL3C5: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
Cyc_YY8(0);break;}case 406: _LL3C6: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE59;yyval=
Cyc_YY8(((_tmpE59=_cycalloc_atomic(sizeof(*_tmpE59)),((_tmpE59->v=(void*)Cyc_Absyn_Times,
_tmpE59)))));}break;}case 407: _LL3C7: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE5A;yyval=
Cyc_YY8(((_tmpE5A=_cycalloc_atomic(sizeof(*_tmpE5A)),((_tmpE5A->v=(void*)Cyc_Absyn_Div,
_tmpE5A)))));}break;}case 408: _LL3C8: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE5B;yyval=
Cyc_YY8(((_tmpE5B=_cycalloc_atomic(sizeof(*_tmpE5B)),((_tmpE5B->v=(void*)Cyc_Absyn_Mod,
_tmpE5B)))));}break;}case 409: _LL3C9: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE5C;yyval=
Cyc_YY8(((_tmpE5C=_cycalloc_atomic(sizeof(*_tmpE5C)),((_tmpE5C->v=(void*)Cyc_Absyn_Plus,
_tmpE5C)))));}break;}case 410: _LL3CA: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE5D;yyval=
Cyc_YY8(((_tmpE5D=_cycalloc_atomic(sizeof(*_tmpE5D)),((_tmpE5D->v=(void*)Cyc_Absyn_Minus,
_tmpE5D)))));}break;}case 411: _LL3CB: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE5E;yyval=
Cyc_YY8(((_tmpE5E=_cycalloc_atomic(sizeof(*_tmpE5E)),((_tmpE5E->v=(void*)Cyc_Absyn_Bitlshift,
_tmpE5E)))));}break;}case 412: _LL3CC: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE5F;yyval=
Cyc_YY8(((_tmpE5F=_cycalloc_atomic(sizeof(*_tmpE5F)),((_tmpE5F->v=(void*)Cyc_Absyn_Bitlrshift,
_tmpE5F)))));}break;}case 413: _LL3CD: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE60;yyval=
Cyc_YY8(((_tmpE60=_cycalloc_atomic(sizeof(*_tmpE60)),((_tmpE60->v=(void*)Cyc_Absyn_Bitand,
_tmpE60)))));}break;}case 414: _LL3CE: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE61;yyval=
Cyc_YY8(((_tmpE61=_cycalloc_atomic(sizeof(*_tmpE61)),((_tmpE61->v=(void*)Cyc_Absyn_Bitxor,
_tmpE61)))));}break;}case 415: _LL3CF: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_Core_Opt*_tmpE62;yyval=
Cyc_YY8(((_tmpE62=_cycalloc_atomic(sizeof(*_tmpE62)),((_tmpE62->v=(void*)Cyc_Absyn_Bitor,
_tmpE62)))));}break;}case 416: _LL3D0: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 417:
_LL3D1: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);yyval=Cyc_YY3(Cyc_Absyn_conditional_exp(
Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),Cyc_yyget_YY3(yyyyvsp[4]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 418: _LL3D2: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=
Cyc_YY3(Cyc_Absyn_throw_exp(Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 419: _LL3D3: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_New_exp(0,
Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));break;}case 420:
_LL3D4: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_New_exp(0,
Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));break;}case 421:
_LL3D5: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);yyval=Cyc_YY3(Cyc_Absyn_New_exp((
struct Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[2]),Cyc_yyget_YY3(yyyyvsp[4]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 422: _LL3D6: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);yyval=
Cyc_YY3(Cyc_Absyn_New_exp((struct Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[2]),Cyc_yyget_YY3(
yyyyvsp[4]),(unsigned int)(yyyylsp[0]).first_line));break;}case 423: _LL3D7: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 424:
_LL3D8: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 425:
_LL3D9: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_or_exp(Cyc_yyget_YY3(
yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 426: _LL3DA: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 427:
_LL3DB: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_and_exp(
Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 428: _LL3DC: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 429:
_LL3DD: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_prim2_exp(
Cyc_Absyn_Bitor,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 430: _LL3DE: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
yyyyvsp[0];break;}case 431: _LL3DF: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_prim2_exp(
Cyc_Absyn_Bitxor,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 432: _LL3E0: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
yyyyvsp[0];break;}case 433: _LL3E1: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_prim2_exp(
Cyc_Absyn_Bitand,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 434: _LL3E2: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
yyyyvsp[0];break;}case 435: _LL3E3: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_eq_exp(Cyc_yyget_YY3(
yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 436: _LL3E4: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_neq_exp(
Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 437: _LL3E5: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 438:
_LL3E6: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_lt_exp(Cyc_yyget_YY3(
yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 439: _LL3E7: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_gt_exp(Cyc_yyget_YY3(
yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 440: _LL3E8: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_lte_exp(
Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 441: _LL3E9: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_gte_exp(
Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 442: _LL3EA: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 443:
_LL3EB: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_prim2_exp(
Cyc_Absyn_Bitlshift,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 444: _LL3EC: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=
Cyc_YY3(Cyc_Absyn_prim2_exp(Cyc_Absyn_Bitlrshift,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));break;}case 445: _LL3ED: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 446:
_LL3EE: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_prim2_exp(
Cyc_Absyn_Plus,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(
yyyylsp[0]).first_line));break;}case 447: _LL3EF: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_prim2_exp(
Cyc_Absyn_Minus,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 448: _LL3F0: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
yyyyvsp[0];break;}case 449: _LL3F1: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_prim2_exp(
Cyc_Absyn_Times,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(
unsigned int)(yyyylsp[0]).first_line));break;}case 450: _LL3F2: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=
Cyc_YY3(Cyc_Absyn_prim2_exp(Cyc_Absyn_Div,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));break;}case 451: _LL3F3: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_prim2_exp(
Cyc_Absyn_Mod,Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(
yyyylsp[0]).first_line));break;}case 452: _LL3F4: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 453:
_LL3F5: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);void*_tmp7DF=Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[1]),(unsigned int)(yyyylsp[1]).first_line);yyval=Cyc_YY3(
Cyc_Absyn_cast_exp(_tmp7DF,Cyc_yyget_YY3(yyyyvsp[3]),1,Cyc_Absyn_Unknown_coercion,(
unsigned int)(yyyylsp[0]).first_line));break;}case 454: _LL3F6: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=
yyyyvsp[0];break;}case 455: _LL3F7: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_pre_inc_exp(
Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));break;}case 456:
_LL3F8: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_pre_dec_exp(
Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));break;}case 457:
_LL3F9: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_address_exp(
Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));break;}case 458:
_LL3FA: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_deref_exp(
Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));break;}case 459:
_LL3FB: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=yyyyvsp[1];break;}case 460:
_LL3FC: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_prim1_exp(
Cyc_yyget_YY7(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 461: _LL3FD: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);void*_tmp7E0=Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[2]),(unsigned int)(yyyylsp[2]).first_line);yyval=Cyc_YY3(
Cyc_Absyn_sizeoftyp_exp(_tmp7E0,(unsigned int)(yyyylsp[0]).first_line));break;}
case 462: _LL3FE: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 1),sizeof(union Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_sizeofexp_exp(
Cyc_yyget_YY3(yyyyvsp[1]),(unsigned int)(yyyylsp[0]).first_line));break;}case 463:
_LL3FF: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 5),sizeof(union
Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);void*_tmp7E1=Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[2]),(unsigned int)(yyyylsp[2]).first_line);{struct Cyc_Absyn_StructField_struct
_tmpE68;struct _dyneither_ptr*_tmpE67;struct Cyc_Absyn_StructField_struct*_tmpE66;
yyval=Cyc_YY3(Cyc_Absyn_offsetof_exp(_tmp7E1,(void*)((_tmpE66=_cycalloc(sizeof(*
_tmpE66)),((_tmpE66[0]=((_tmpE68.tag=0,((_tmpE68.f1=((_tmpE67=_cycalloc(sizeof(*
_tmpE67)),((_tmpE67[0]=Cyc_yyget_String_tok(yyyyvsp[4]),_tmpE67)))),_tmpE68)))),
_tmpE66)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 464: _LL400: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 5),sizeof(union
Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);void*_tmp7E5=Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[2]),(unsigned int)(yyyylsp[2]).first_line);{struct Cyc_Absyn_TupleIndex_struct
_tmpE6B;struct Cyc_Absyn_TupleIndex_struct*_tmpE6A;yyval=Cyc_YY3(Cyc_Absyn_offsetof_exp(
_tmp7E5,(void*)((_tmpE6A=_cycalloc_atomic(sizeof(*_tmpE6A)),((_tmpE6A[0]=((
_tmpE6B.tag=1,((_tmpE6B.f1=(unsigned int)(Cyc_yyget_Int_tok(yyyyvsp[4])).f2,
_tmpE6B)))),_tmpE6A)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 465:
_LL401: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_Malloc_e_struct
_tmpE71;struct Cyc_Absyn_MallocInfo _tmpE70;struct Cyc_Absyn_Malloc_e_struct*
_tmpE6F;yyval=Cyc_YY3(Cyc_Absyn_new_exp((void*)((_tmpE6F=_cycalloc(sizeof(*
_tmpE6F)),((_tmpE6F[0]=((_tmpE71.tag=32,((_tmpE71.f1=((_tmpE70.is_calloc=0,((
_tmpE70.rgn=0,((_tmpE70.elt_type=0,((_tmpE70.num_elts=Cyc_yyget_YY3(yyyyvsp[2]),((
_tmpE70.fat_result=0,_tmpE70)))))))))),_tmpE71)))),_tmpE6F)))),(unsigned int)(
yyyylsp[0]).first_line));}break;}case 466: _LL402: {union Cyc_YYSTYPE*yyyyvsp=(
union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 5),sizeof(union Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);{struct Cyc_Absyn_Malloc_e_struct
_tmpE77;struct Cyc_Absyn_MallocInfo _tmpE76;struct Cyc_Absyn_Malloc_e_struct*
_tmpE75;yyval=Cyc_YY3(Cyc_Absyn_new_exp((void*)((_tmpE75=_cycalloc(sizeof(*
_tmpE75)),((_tmpE75[0]=((_tmpE77.tag=32,((_tmpE77.f1=((_tmpE76.is_calloc=0,((
_tmpE76.rgn=(struct Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[2]),((_tmpE76.elt_type=0,((
_tmpE76.num_elts=Cyc_yyget_YY3(yyyyvsp[4]),((_tmpE76.fat_result=0,_tmpE76)))))))))),
_tmpE77)))),_tmpE75)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 467:
_LL403: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 8),sizeof(union
Cyc_YYSTYPE),9);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 8),sizeof(struct Cyc_Yyltype),9);void*_tmp7EE=Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[6]),(unsigned int)(yyyylsp[6]).first_line);{struct Cyc_Absyn_Malloc_e_struct
_tmpE81;void**_tmpE80;struct Cyc_Absyn_MallocInfo _tmpE7F;struct Cyc_Absyn_Malloc_e_struct*
_tmpE7E;yyval=Cyc_YY3(Cyc_Absyn_new_exp((void*)((_tmpE7E=_cycalloc(sizeof(*
_tmpE7E)),((_tmpE7E[0]=((_tmpE81.tag=32,((_tmpE81.f1=((_tmpE7F.is_calloc=1,((
_tmpE7F.rgn=0,((_tmpE7F.elt_type=((_tmpE80=_cycalloc(sizeof(*_tmpE80)),((_tmpE80[
0]=_tmp7EE,_tmpE80)))),((_tmpE7F.num_elts=Cyc_yyget_YY3(yyyyvsp[2]),((_tmpE7F.fat_result=
0,_tmpE7F)))))))))),_tmpE81)))),_tmpE7E)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 468: _LL404: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 10),sizeof(union Cyc_YYSTYPE),11);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 10),sizeof(struct Cyc_Yyltype),11);void*_tmp7F3=Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[8]),(unsigned int)(yyyylsp[8]).first_line);{struct Cyc_Absyn_Malloc_e_struct
_tmpE8B;void**_tmpE8A;struct Cyc_Absyn_MallocInfo _tmpE89;struct Cyc_Absyn_Malloc_e_struct*
_tmpE88;yyval=Cyc_YY3(Cyc_Absyn_new_exp((void*)((_tmpE88=_cycalloc(sizeof(*
_tmpE88)),((_tmpE88[0]=((_tmpE8B.tag=32,((_tmpE8B.f1=((_tmpE89.is_calloc=1,((
_tmpE89.rgn=(struct Cyc_Absyn_Exp*)Cyc_yyget_YY3(yyyyvsp[2]),((_tmpE89.elt_type=((
_tmpE8A=_cycalloc(sizeof(*_tmpE8A)),((_tmpE8A[0]=_tmp7F3,_tmpE8A)))),((_tmpE89.num_elts=
Cyc_yyget_YY3(yyyyvsp[4]),((_tmpE89.fat_result=0,_tmpE89)))))))))),_tmpE8B)))),
_tmpE88)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 469: _LL405: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);{struct Cyc_Absyn_Exp*_tmpE8C[1];
yyval=Cyc_YY3(Cyc_Absyn_primop_exp(Cyc_Absyn_Numelts,((_tmpE8C[0]=Cyc_yyget_YY3(
yyyyvsp[2]),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(
_tag_dyneither(_tmpE8C,sizeof(struct Cyc_Absyn_Exp*),1)))),(unsigned int)(yyyylsp[
0]).first_line));}break;}case 470: _LL406: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 5),sizeof(union Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);{struct Cyc_Absyn_Tagcheck_e_struct
_tmpE92;struct _dyneither_ptr*_tmpE91;struct Cyc_Absyn_Tagcheck_e_struct*_tmpE90;
yyval=Cyc_YY3(Cyc_Absyn_new_exp((void*)((_tmpE90=_cycalloc(sizeof(*_tmpE90)),((
_tmpE90[0]=((_tmpE92.tag=36,((_tmpE92.f1=Cyc_yyget_YY3(yyyyvsp[2]),((_tmpE92.f2=((
_tmpE91=_cycalloc(sizeof(*_tmpE91)),((_tmpE91[0]=Cyc_yyget_String_tok(yyyyvsp[4]),
_tmpE91)))),_tmpE92)))))),_tmpE90)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 471: _LL407: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 5),sizeof(union Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);{struct Cyc_Absyn_Tagcheck_e_struct
_tmpE98;struct _dyneither_ptr*_tmpE97;struct Cyc_Absyn_Tagcheck_e_struct*_tmpE96;
yyval=Cyc_YY3(Cyc_Absyn_new_exp((void*)((_tmpE96=_cycalloc(sizeof(*_tmpE96)),((
_tmpE96[0]=((_tmpE98.tag=36,((_tmpE98.f1=Cyc_Absyn_deref_exp(Cyc_yyget_YY3(
yyyyvsp[2]),(unsigned int)(yyyylsp[2]).first_line),((_tmpE98.f2=((_tmpE97=
_cycalloc(sizeof(*_tmpE97)),((_tmpE97[0]=Cyc_yyget_String_tok(yyyyvsp[4]),
_tmpE97)))),_tmpE98)))))),_tmpE96)))),(unsigned int)(yyyylsp[0]).first_line));}
break;}case 472: _LL408: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 3),sizeof(union Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);void*_tmp7FF=Cyc_Parse_type_name_to_type(
Cyc_yyget_YY37(yyyyvsp[2]),(unsigned int)(yyyylsp[2]).first_line);yyval=Cyc_YY3(
Cyc_Absyn_valueof_exp(_tmp7FF,(unsigned int)(yyyylsp[0]).first_line));break;}
case 473: _LL409: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);int _tmp801;struct _dyneither_ptr
_tmp802;struct _tuple20 _tmp800=Cyc_yyget_Asm_tok(yyyyvsp[0]);_tmp801=_tmp800.f1;
_tmp802=_tmp800.f2;yyval=Cyc_YY3(Cyc_Absyn_asm_exp(_tmp801,_tmp802,(unsigned int)(
yyyylsp[0]).first_line));break;}case 474: _LL40A: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY7(Cyc_Absyn_Bitnot);
break;}case 475: _LL40B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY7(Cyc_Absyn_Not);
break;}case 476: _LL40C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY7(Cyc_Absyn_Minus);
break;}case 477: _LL40D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 478:
_LL40E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);yyval=Cyc_YY3(Cyc_Absyn_subscript_exp(
Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY3(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 479: _LL40F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=Cyc_YY3(Cyc_Absyn_unknowncall_exp(
Cyc_yyget_YY3(yyyyvsp[0]),0,(unsigned int)(yyyylsp[0]).first_line));break;}case
480: _LL410: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);yyval=Cyc_YY3(Cyc_Absyn_unknowncall_exp(
Cyc_yyget_YY3(yyyyvsp[0]),Cyc_yyget_YY5(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 481: _LL411: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _dyneither_ptr*_tmpE99;
yyval=Cyc_YY3(Cyc_Absyn_aggrmember_exp(Cyc_yyget_YY3(yyyyvsp[0]),((_tmpE99=
_cycalloc(sizeof(*_tmpE99)),((_tmpE99[0]=Cyc_yyget_String_tok(yyyyvsp[2]),
_tmpE99)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 482: _LL412: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union
Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct _dyneither_ptr*_tmpE9A;
yyval=Cyc_YY3(Cyc_Absyn_aggrarrow_exp(Cyc_yyget_YY3(yyyyvsp[0]),((_tmpE9A=
_cycalloc(sizeof(*_tmpE9A)),((_tmpE9A[0]=Cyc_yyget_String_tok(yyyyvsp[2]),
_tmpE9A)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 483: _LL413: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_post_inc_exp(
Cyc_yyget_YY3(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}case 484:
_LL414: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_post_dec_exp(
Cyc_yyget_YY3(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}case 485:
_LL415: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 5),sizeof(union
Cyc_YYSTYPE),6);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 5),sizeof(struct Cyc_Yyltype),6);{struct Cyc_Absyn_CompoundLit_e_struct
_tmpE9D;struct Cyc_Absyn_CompoundLit_e_struct*_tmpE9C;yyval=Cyc_YY3(Cyc_Absyn_new_exp((
void*)((_tmpE9C=_cycalloc(sizeof(*_tmpE9C)),((_tmpE9C[0]=((_tmpE9D.tag=24,((
_tmpE9D.f1=Cyc_yyget_YY37(yyyyvsp[1]),((_tmpE9D.f2=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_yyget_YY6(yyyyvsp[4])),_tmpE9D)))))),
_tmpE9C)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 486: _LL416: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 6),sizeof(union
Cyc_YYSTYPE),7);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 6),sizeof(struct Cyc_Yyltype),7);{struct Cyc_Absyn_CompoundLit_e_struct
_tmpEA0;struct Cyc_Absyn_CompoundLit_e_struct*_tmpE9F;yyval=Cyc_YY3(Cyc_Absyn_new_exp((
void*)((_tmpE9F=_cycalloc(sizeof(*_tmpE9F)),((_tmpE9F[0]=((_tmpEA0.tag=24,((
_tmpEA0.f1=Cyc_yyget_YY37(yyyyvsp[1]),((_tmpEA0.f2=((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_yyget_YY6(yyyyvsp[4])),_tmpEA0)))))),
_tmpE9F)))),(unsigned int)(yyyylsp[0]).first_line));}break;}case 487: _LL417: {
union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY3(Cyc_Absyn_unknownid_exp(
Cyc_yyget_QualId_tok(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}
case 488: _LL418: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 489:
_LL419: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY3(Cyc_Absyn_string_exp(
Cyc_yyget_String_tok(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}
case 490: _LL41A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY3(Cyc_Absyn_wstring_exp(
Cyc_yyget_String_tok(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}
case 491: _LL41B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);yyval=yyyyvsp[1];break;}case 492:
_LL41C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 1),sizeof(union
Cyc_YYSTYPE),2);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1),sizeof(struct Cyc_Yyltype),2);yyval=Cyc_YY3(Cyc_Absyn_noinstantiate_exp(
Cyc_yyget_YY3(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}case 493:
_LL41D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);yyval=Cyc_YY3(Cyc_Absyn_instantiate_exp(
Cyc_yyget_YY3(yyyyvsp[0]),((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(
Cyc_yyget_YY40(yyyyvsp[3])),(unsigned int)(yyyylsp[0]).first_line));break;}case
494: _LL41E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 3),sizeof(union
Cyc_YYSTYPE),4);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 3),sizeof(struct Cyc_Yyltype),4);yyval=Cyc_YY3(Cyc_Absyn_tuple_exp(
Cyc_yyget_YY5(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));break;}case 495:
_LL41F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union
Cyc_YYSTYPE),5);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);{struct Cyc_Absyn_Aggregate_e_struct
_tmpEA3;struct Cyc_Absyn_Aggregate_e_struct*_tmpEA2;yyval=Cyc_YY3(Cyc_Absyn_new_exp((
void*)((_tmpEA2=_cycalloc(sizeof(*_tmpEA2)),((_tmpEA2[0]=((_tmpEA3.tag=27,((
_tmpEA3.f1=Cyc_yyget_QualId_tok(yyyyvsp[0]),((_tmpEA3.f2=Cyc_yyget_YY40(yyyyvsp[
2]),((_tmpEA3.f3=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(
Cyc_yyget_YY6(yyyyvsp[3])),((_tmpEA3.f4=0,_tmpEA3)))))))))),_tmpEA2)))),(
unsigned int)(yyyylsp[0]).first_line));}break;}case 496: _LL420: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 4),sizeof(union Cyc_YYSTYPE),5);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 4),sizeof(struct Cyc_Yyltype),5);yyval=
Cyc_YY3(Cyc_Absyn_stmt_exp(Cyc_yyget_YY9(yyyyvsp[2]),(unsigned int)(yyyylsp[0]).first_line));
break;}case 497: _LL421: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY5(((struct Cyc_List_List*(*)(
struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_yyget_YY5(yyyyvsp[0])));break;}case
498: _LL422: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct Cyc_List_List*_tmpEA4;yyval=
Cyc_YY5(((_tmpEA4=_cycalloc(sizeof(*_tmpEA4)),((_tmpEA4->hd=Cyc_yyget_YY3(
yyyyvsp[0]),((_tmpEA4->tl=0,_tmpEA4)))))));}break;}case 499: _LL423: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 2),sizeof(union Cyc_YYSTYPE),3);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 2),sizeof(struct Cyc_Yyltype),3);{struct
Cyc_List_List*_tmpEA5;yyval=Cyc_YY5(((_tmpEA5=_cycalloc(sizeof(*_tmpEA5)),((
_tmpEA5->hd=Cyc_yyget_YY3(yyyyvsp[2]),((_tmpEA5->tl=Cyc_yyget_YY5(yyyyvsp[0]),
_tmpEA5)))))));}break;}case 500: _LL424: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);struct _tuple5 _tmp80D=Cyc_yyget_Int_tok(
yyyyvsp[0]);yyval=Cyc_YY3(Cyc_Absyn_int_exp(_tmp80D.f1,_tmp80D.f2,(unsigned int)(
yyyylsp[0]).first_line));break;}case 501: _LL425: {union Cyc_YYSTYPE*yyyyvsp=(union
Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY3(Cyc_Absyn_char_exp(
Cyc_yyget_Char_tok(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}
case 502: _LL426: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=Cyc_YY3(Cyc_Absyn_wchar_exp(
Cyc_yyget_String_tok(yyyyvsp[0]),(unsigned int)(yyyylsp[0]).first_line));break;}
case 503: _LL427: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);struct _dyneither_ptr _tmp80E=Cyc_yyget_String_tok(
yyyyvsp[0]);int l=(int)Cyc_strlen((struct _dyneither_ptr)_tmp80E);int i=1;if(l > 0){
char c=*((const char*)_check_dyneither_subscript(_tmp80E,sizeof(char),l - 1));if(c
== 'f'  || c == 'F')i=0;else{if(c == 'l'  || c == 'L')i=2;}}yyval=Cyc_YY3(Cyc_Absyn_float_exp(
_tmp80E,i,(unsigned int)(yyyylsp[0]).first_line));break;}case 504: _LL428: {union
Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(
yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);
struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(
struct Cyc_Yyltype),1);yyval=Cyc_YY3(Cyc_Absyn_null_exp((unsigned int)(yyyylsp[0]).first_line));
break;}case 505: _LL429: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _dyneither_ptr*_tmpEA8;
struct _tuple0*_tmpEA7;yyval=Cyc_QualId_tok(((_tmpEA7=_cycalloc(sizeof(*_tmpEA7)),((
_tmpEA7->f1=Cyc_Absyn_Rel_n(0),((_tmpEA7->f2=((_tmpEA8=_cycalloc(sizeof(*_tmpEA8)),((
_tmpEA8[0]=Cyc_yyget_String_tok(yyyyvsp[0]),_tmpEA8)))),_tmpEA7)))))));}break;}
case 506: _LL42A: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 507:
_LL42B: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);{struct _dyneither_ptr*_tmpEAB;
struct _tuple0*_tmpEAA;yyval=Cyc_QualId_tok(((_tmpEAA=_cycalloc(sizeof(*_tmpEAA)),((
_tmpEAA->f1=Cyc_Absyn_Rel_n(0),((_tmpEAA->f2=((_tmpEAB=_cycalloc(sizeof(*_tmpEAB)),((
_tmpEAB[0]=Cyc_yyget_String_tok(yyyyvsp[0]),_tmpEAB)))),_tmpEAA)))))));}break;}
case 508: _LL42C: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),
yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct
Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 509:
_LL42D: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 510:
_LL42E: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 511:
_LL42F: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 512:
_LL430: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);yyval=yyyyvsp[0];break;}case 513:
_LL431: {union Cyc_YYSTYPE*yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(
_dyneither_ptr_plus(yyvs,sizeof(union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union
Cyc_YYSTYPE),1);struct Cyc_Yyltype*yyyylsp=(struct Cyc_Yyltype*)
_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);break;}case 514: _LL432: {union Cyc_YYSTYPE*
yyyyvsp=(union Cyc_YYSTYPE*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyvs,sizeof(
union Cyc_YYSTYPE),yyvsp_offset - 0),sizeof(union Cyc_YYSTYPE),1);struct Cyc_Yyltype*
yyyylsp=(struct Cyc_Yyltype*)_untag_dyneither_ptr(_dyneither_ptr_plus(yyls,
sizeof(struct Cyc_Yyltype),yylsp_offset - 0),sizeof(struct Cyc_Yyltype),1);
yylex_buf->lex_curr_pos -=1;break;}default: _LL433: break;}yyvsp_offset -=yylen;
yyssp_offset -=yylen;yylsp_offset -=yylen;*((union Cyc_YYSTYPE*)
_check_dyneither_subscript(yyvs,sizeof(union Cyc_YYSTYPE),++ yyvsp_offset))=yyval;
++ yylsp_offset;if(yylen == 0){(*((struct Cyc_Yyltype*)_check_dyneither_subscript(
yyls,sizeof(struct Cyc_Yyltype),yylsp_offset))).first_line=yylloc.first_line;(*((
struct Cyc_Yyltype*)_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset))).first_column=yylloc.first_column;(*((struct Cyc_Yyltype*)
_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset))).last_line=(*((
struct Cyc_Yyltype*)_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset - 1))).last_line;(*((struct Cyc_Yyltype*)_check_dyneither_subscript(
yyls,sizeof(struct Cyc_Yyltype),yylsp_offset))).last_column=(*((struct Cyc_Yyltype*)
_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset - 1))).last_column;}
else{(*((struct Cyc_Yyltype*)_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),
yylsp_offset))).last_line=(*((struct Cyc_Yyltype*)_check_dyneither_subscript(yyls,
sizeof(struct Cyc_Yyltype),(yylsp_offset + yylen)- 1))).last_line;(*((struct Cyc_Yyltype*)
_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),yylsp_offset))).last_column=(*((
struct Cyc_Yyltype*)_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),(
yylsp_offset + yylen)- 1))).last_column;}yyn=(int)Cyc_yyr1[
_check_known_subscript_notnull(515,yyn)];yystate=Cyc_yypgoto[
_check_known_subscript_notnull(143,yyn - 146)]+ *((short*)
_check_dyneither_subscript(yyss,sizeof(short),yyssp_offset));if((yystate >= 0  && 
yystate <= 6375) && Cyc_yycheck[_check_known_subscript_notnull(6376,yystate)]== *((
short*)_check_dyneither_subscript(yyss,sizeof(short),yyssp_offset)))yystate=(int)
Cyc_yytable[_check_known_subscript_notnull(6376,yystate)];else{yystate=(int)Cyc_yydefgoto[
_check_known_subscript_notnull(143,yyn - 146)];}goto yynewstate;yyerrlab: if(
yyerrstatus == 0){++ yynerrs;yyn=(int)Cyc_yypact[_check_known_subscript_notnull(
1030,yystate)];if(yyn > - 32768  && yyn < 6375){int sze=0;struct _dyneither_ptr msg;int
x;int count;count=0;for(x=yyn < 0?- yyn: 0;x < 289 / sizeof(char*);++ x){if(Cyc_yycheck[
_check_known_subscript_notnull(6376,x + yyn)]== x)(sze +=Cyc_strlen((struct
_dyneither_ptr)Cyc_yytname[_check_known_subscript_notnull(289,x)])+ 15,count ++);}{
unsigned int _tmpEB0;unsigned int _tmpEAF;struct _dyneither_ptr _tmpEAE;char*_tmpEAD;
unsigned int _tmpEAC;msg=((_tmpEAC=(unsigned int)(sze + 15),((_tmpEAD=(char*)
_region_malloc(yyregion,_check_times(sizeof(char),_tmpEAC + 1)),((_tmpEAE=
_tag_dyneither(_tmpEAD,sizeof(char),_tmpEAC + 1),((((_tmpEAF=_tmpEAC,((_tmpEB1(&
_tmpEB0,& _tmpEAF,& _tmpEAD),_tmpEAD[_tmpEAF]=(char)0)))),_tmpEAE))))))));}{const
char*_tmpEB2;Cyc_strcpy(msg,((_tmpEB2="parse error",_tag_dyneither(_tmpEB2,
sizeof(char),12))));}if(count < 5){count=0;for(x=yyn < 0?- yyn: 0;x < 289 / sizeof(
char*);++ x){if(Cyc_yycheck[_check_known_subscript_notnull(6376,x + yyn)]== x){{
const char*_tmpEB4;const char*_tmpEB3;Cyc_strcat(msg,(struct _dyneither_ptr)(count
== 0?(struct _dyneither_ptr)((_tmpEB4=", expecting `",_tag_dyneither(_tmpEB4,
sizeof(char),14))):(struct _dyneither_ptr)((_tmpEB3=" or `",_tag_dyneither(
_tmpEB3,sizeof(char),6)))));}Cyc_strcat(msg,(struct _dyneither_ptr)Cyc_yytname[
_check_known_subscript_notnull(289,x)]);{const char*_tmpEB5;Cyc_strcat(msg,((
_tmpEB5="'",_tag_dyneither(_tmpEB5,sizeof(char),2))));}++ count;}}}Cyc_yyerror((
struct _dyneither_ptr)msg);}else{const char*_tmpEB6;Cyc_yyerror(((_tmpEB6="parse error",
_tag_dyneither(_tmpEB6,sizeof(char),12))));}}goto yyerrlab1;yyerrlab1: if(
yyerrstatus == 3){if(yychar == 0){int _tmp81C=1;_npop_handler(0);return _tmp81C;}
yychar=- 2;}yyerrstatus=3;goto yyerrhandle;yyerrdefault: yyerrpop: if(yyssp_offset == 
0){int _tmp81D=1;_npop_handler(0);return _tmp81D;}-- yyvsp_offset;yystate=(int)*((
short*)_check_dyneither_subscript(yyss,sizeof(short),-- yyssp_offset));--
yylsp_offset;yyerrhandle: yyn=(int)Cyc_yypact[_check_known_subscript_notnull(1030,
yystate)];if(yyn == - 32768)goto yyerrdefault;yyn +=1;if((yyn < 0  || yyn > 6375) || 
Cyc_yycheck[_check_known_subscript_notnull(6376,yyn)]!= 1)goto yyerrdefault;yyn=(
int)Cyc_yytable[_check_known_subscript_notnull(6376,yyn)];if(yyn < 0){if(yyn == -
32768)goto yyerrpop;yyn=- yyn;goto yyreduce;}else{if(yyn == 0)goto yyerrpop;}if(yyn == 
1029){int _tmp81E=0;_npop_handler(0);return _tmp81E;}*((union Cyc_YYSTYPE*)
_check_dyneither_subscript(yyvs,sizeof(union Cyc_YYSTYPE),++ yyvsp_offset))=yylval;*((
struct Cyc_Yyltype*)_check_dyneither_subscript(yyls,sizeof(struct Cyc_Yyltype),++
yylsp_offset))=yylloc;goto yynewstate;};_pop_region(yyregion);}void Cyc_yyprint(
int i,union Cyc_YYSTYPE v){union Cyc_YYSTYPE _tmp828=v;struct _tuple5 _tmp829;int
_tmp82A;char _tmp82B;struct _dyneither_ptr _tmp82C;struct _tuple0*_tmp82D;struct
_tuple0 _tmp82E;union Cyc_Absyn_Nmspace _tmp82F;struct _dyneither_ptr*_tmp830;_LL436:
if((_tmp828.Int_tok).tag != 1)goto _LL438;_tmp829=(struct _tuple5)(_tmp828.Int_tok).val;
_tmp82A=_tmp829.f2;_LL437:{const char*_tmpEBA;void*_tmpEB9[1];struct Cyc_Int_pa_struct
_tmpEB8;(_tmpEB8.tag=1,((_tmpEB8.f1=(unsigned long)_tmp82A,((_tmpEB9[0]=& _tmpEB8,
Cyc_fprintf(Cyc_stderr,((_tmpEBA="%d",_tag_dyneither(_tmpEBA,sizeof(char),3))),
_tag_dyneither(_tmpEB9,sizeof(void*),1)))))));}goto _LL435;_LL438: if((_tmp828.Char_tok).tag
!= 2)goto _LL43A;_tmp82B=(char)(_tmp828.Char_tok).val;_LL439:{const char*_tmpEBE;
void*_tmpEBD[1];struct Cyc_Int_pa_struct _tmpEBC;(_tmpEBC.tag=1,((_tmpEBC.f1=(
unsigned long)((int)_tmp82B),((_tmpEBD[0]=& _tmpEBC,Cyc_fprintf(Cyc_stderr,((
_tmpEBE="%c",_tag_dyneither(_tmpEBE,sizeof(char),3))),_tag_dyneither(_tmpEBD,
sizeof(void*),1)))))));}goto _LL435;_LL43A: if((_tmp828.String_tok).tag != 3)goto
_LL43C;_tmp82C=(struct _dyneither_ptr)(_tmp828.String_tok).val;_LL43B:{const char*
_tmpEC2;void*_tmpEC1[1];struct Cyc_String_pa_struct _tmpEC0;(_tmpEC0.tag=0,((
_tmpEC0.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp82C),((_tmpEC1[0]=&
_tmpEC0,Cyc_fprintf(Cyc_stderr,((_tmpEC2="\"%s\"",_tag_dyneither(_tmpEC2,sizeof(
char),5))),_tag_dyneither(_tmpEC1,sizeof(void*),1)))))));}goto _LL435;_LL43C: if((
_tmp828.QualId_tok).tag != 5)goto _LL43E;_tmp82D=(struct _tuple0*)(_tmp828.QualId_tok).val;
_tmp82E=*_tmp82D;_tmp82F=_tmp82E.f1;_tmp830=_tmp82E.f2;_LL43D: {struct Cyc_List_List*
_tmp83A=0;{union Cyc_Absyn_Nmspace _tmp83B=_tmp82F;struct Cyc_List_List*_tmp83C;
struct Cyc_List_List*_tmp83D;struct Cyc_List_List*_tmp83E;int _tmp83F;_LL441: if((
_tmp83B.Rel_n).tag != 1)goto _LL443;_tmp83C=(struct Cyc_List_List*)(_tmp83B.Rel_n).val;
_LL442: _tmp83A=_tmp83C;goto _LL440;_LL443: if((_tmp83B.Abs_n).tag != 2)goto _LL445;
_tmp83D=(struct Cyc_List_List*)(_tmp83B.Abs_n).val;_LL444: _tmp83A=_tmp83D;goto
_LL440;_LL445: if((_tmp83B.C_n).tag != 3)goto _LL447;_tmp83E=(struct Cyc_List_List*)(
_tmp83B.C_n).val;_LL446: _tmp83A=_tmp83E;goto _LL440;_LL447: if((_tmp83B.Loc_n).tag
!= 4)goto _LL440;_tmp83F=(int)(_tmp83B.Loc_n).val;_LL448: goto _LL440;_LL440:;}for(
0;_tmp83A != 0;_tmp83A=_tmp83A->tl){const char*_tmpEC6;void*_tmpEC5[1];struct Cyc_String_pa_struct
_tmpEC4;(_tmpEC4.tag=0,((_tmpEC4.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*((
struct _dyneither_ptr*)_tmp83A->hd)),((_tmpEC5[0]=& _tmpEC4,Cyc_fprintf(Cyc_stderr,((
_tmpEC6="%s::",_tag_dyneither(_tmpEC6,sizeof(char),5))),_tag_dyneither(_tmpEC5,
sizeof(void*),1)))))));}{const char*_tmpECA;void*_tmpEC9[1];struct Cyc_String_pa_struct
_tmpEC8;(_tmpEC8.tag=0,((_tmpEC8.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*
_tmp830),((_tmpEC9[0]=& _tmpEC8,Cyc_fprintf(Cyc_stderr,((_tmpECA="%s::",
_tag_dyneither(_tmpECA,sizeof(char),5))),_tag_dyneither(_tmpEC9,sizeof(void*),1)))))));}
goto _LL435;}_LL43E:;_LL43F:{const char*_tmpECD;void*_tmpECC;(_tmpECC=0,Cyc_fprintf(
Cyc_stderr,((_tmpECD="?",_tag_dyneither(_tmpECD,sizeof(char),2))),_tag_dyneither(
_tmpECC,sizeof(void*),0)));}goto _LL435;_LL435:;}struct Cyc_List_List*Cyc_Parse_parse_file(
struct Cyc___cycFILE*f){Cyc_Parse_parse_result=0;{struct _RegionHandle _tmp848=
_new_region("yyr");struct _RegionHandle*yyr=& _tmp848;_push_region(yyr);Cyc_yyparse(
yyr,Cyc_Lexing_from_file(f));{struct Cyc_List_List*_tmp849=Cyc_Parse_parse_result;
_npop_handler(0);return _tmp849;};;_pop_region(yyr);};}
