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
#else
  unsigned used_bytes;
  unsigned wasted_bytes;
#endif
};

struct _DynRegionFrame {
  struct _RuntimeStack s;
  struct _DynRegionHandle *x;
};

// A dynamic region is just a region handle.  We have the
// wrapper struct for type abstraction reasons.
struct Cyc_Core_DynamicRegion {
  struct _RegionHandle h;
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
extern int _throw_null_fn(const char *filename, unsigned lineno);
extern int _throw_arraybounds_fn(const char *filename, unsigned lineno);
extern int _throw_badalloc_fn(const char *filename, unsigned lineno);
extern int _throw_match_fn(const char *filename, unsigned lineno);
extern int _throw_fn(void* e, const char *filename, unsigned lineno);
extern int _rethrow(void* e);
#define _throw_null() (_throw_null_fn(__FILE__,__LINE__))
#define _throw_arraybounds() (_throw_arraybounds_fn(__FILE__,__LINE__))
#define _throw_badalloc() (_throw_badalloc_fn(__FILE__,__LINE__))
#define _throw_match() (_throw_match_fn(__FILE__,__LINE__))
#define _throw(e) (_throw_fn((e),__FILE__,__LINE__))
#endif

extern struct _xtunion_struct *_exn_thrown;

/* Built-in Exceptions */
struct Cyc_Null_Exception_exn_struct { char *tag; };
struct Cyc_Array_bounds_exn_struct { char *tag; };
struct Cyc_Match_Exception_exn_struct { char *tag; };
struct Cyc_Bad_alloc_exn_struct { char *tag; };
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
_check_null_fn(const void *ptr, const char *filename, unsigned lineno) {
  void*_check_null_temp = (void*)(ptr);
  if (!_check_null_temp) _throw_null_fn(filename,lineno);
  return _check_null_temp;
}
#define _check_null(p) (_check_null_fn((p),__FILE__,__LINE__))
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
_check_known_subscript_null_fn(void *ptr, unsigned bound, unsigned elt_sz, unsigned index, const char *filename, unsigned lineno) {
  void*_cks_ptr = (void*)(ptr);
  unsigned _cks_bound = (bound);
  unsigned _cks_elt_sz = (elt_sz);
  unsigned _cks_index = (index);
  if (!_cks_ptr) _throw_null_fn(filename,lineno);
  if (_cks_index >= _cks_bound) _throw_arraybounds_fn(filename,lineno);
  return ((char *)_cks_ptr) + _cks_elt_sz*_cks_index;
}
#define _check_known_subscript_null(p,b,e) (_check_known_subscript_null_fn(p,b,e,__FILE__,__LINE__))
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
_check_known_subscript_notnull_fn(unsigned bound,unsigned index,const char *filename,unsigned lineno) { 
  unsigned _cksnn_bound = (bound); 
  unsigned _cksnn_index = (index); 
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds_fn(filename,lineno); 
  return _cksnn_index;
}
#define _check_known_subscript_notnull(b,i) (_check_known_subscript_notnull_fn(b,i,__FILE__,__LINE__))
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
#define _zero_arr_plus_char_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_short_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_int_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_float_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_double_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#else
static _INLINE char *
_zero_arr_plus_char_fn(char *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE short *
_zero_arr_plus_short_fn(short *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE int *
_zero_arr_plus_int_fn(int *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE float *
_zero_arr_plus_float_fn(float *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE double *
_zero_arr_plus_double_fn(double *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE long double *
_zero_arr_plus_longdouble_fn(long double *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE void *
_zero_arr_plus_voidstar_fn(void **orig_x, unsigned int orig_sz, int orig_i,const char *filename,unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
#endif

#define _zero_arr_plus_char(x,s,i) \
  (_zero_arr_plus_char_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_short(x,s,i) \
  (_zero_arr_plus_short_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_int(x,s,i) \
  (_zero_arr_plus_int_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_float(x,s,i) \
  (_zero_arr_plus_float_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_double(x,s,i) \
  (_zero_arr_plus_double_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_longdouble(x,s,i) \
  (_zero_arr_plus_longdouble_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_voidstar(x,s,i) \
  (_zero_arr_plus_voidstar_fn(x,s,i,__FILE__,__LINE__))


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
   Note that this expands to call _zero_arr_plus_<type>_fn. */
static _INLINE char *
_zero_arr_inplace_plus_char_fn(char **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_char_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_char(x,i) \
  _zero_arr_inplace_plus_char_fn((char **)(x),i,__FILE__,__LINE__)
static _INLINE short *
_zero_arr_inplace_plus_short_fn(short **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_short_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_short(x,i) \
  _zero_arr_inplace_plus_short_fn((short **)(x),i,__FILE__,__LINE__)
static _INLINE int *
_zero_arr_inplace_plus_int(int **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_int_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_int(x,i) \
  _zero_arr_inplace_plus_int_fn((int **)(x),i,__FILE__,__LINE__)
static _INLINE float *
_zero_arr_inplace_plus_float_fn(float **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_float_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_float(x,i) \
  _zero_arr_inplace_plus_float_fn((float **)(x),i,__FILE__,__LINE__)
static _INLINE double *
_zero_arr_inplace_plus_double_fn(double **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_double_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_double(x,i) \
  _zero_arr_inplace_plus_double_fn((double **)(x),i,__FILE__,__LINE__)
static _INLINE long double *
_zero_arr_inplace_plus_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_longdouble_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_longdouble(x,i) \
  _zero_arr_inplace_plus_longdouble_fn((long double **)(x),i,__FILE__,__LINE__)
static _INLINE void *
_zero_arr_inplace_plus_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_voidstar_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_voidstar(x,i) \
  _zero_arr_inplace_plus_voidstar_fn((void ***)(x),i,__FILE__,__LINE__)

/* Does in-place increment of a zero-terminated pointer (e.g., x++). */
static _INLINE char *
_zero_arr_inplace_plus_post_char_fn(char **x, int orig_i,const char *filename,unsigned lineno){
  char * _zap_res = *x;
  *x = _zero_arr_plus_char_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_char(x,i) \
  _zero_arr_inplace_plus_post_char_fn((char **)(x),(i),__FILE__,__LINE__)
static _INLINE short *
_zero_arr_inplace_plus_post_short_fn(short **x, int orig_i,const char *filename,unsigned lineno){
  short * _zap_res = *x;
  *x = _zero_arr_plus_short_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_short(x,i) \
  _zero_arr_inplace_plus_post_short_fn((short **)(x),(i),__FILE__,__LINE__)
static _INLINE int *
_zero_arr_inplace_plus_post_int_fn(int **x, int orig_i,const char *filename, unsigned lineno){
  int * _zap_res = *x;
  *x = _zero_arr_plus_int_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_int(x,i) \
  _zero_arr_inplace_plus_post_int_fn((int **)(x),(i),__FILE__,__LINE__)
static _INLINE float *
_zero_arr_inplace_plus_post_float_fn(float **x, int orig_i,const char *filename, unsigned lineno){
  float * _zap_res = *x;
  *x = _zero_arr_plus_float_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_float(x,i) \
  _zero_arr_inplace_plus_post_float_fn((float **)(x),(i),__FILE__,__LINE__)
static _INLINE double *
_zero_arr_inplace_plus_post_double_fn(double **x, int orig_i,const char *filename,unsigned lineno){
  double * _zap_res = *x;
  *x = _zero_arr_plus_double_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_double(x,i) \
  _zero_arr_inplace_plus_post_double_fn((double **)(x),(i),__FILE__,__LINE__)
static _INLINE long double *
_zero_arr_inplace_plus_post_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno){
  long double * _zap_res = *x;
  *x = _zero_arr_plus_longdouble_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_longdouble(x,i) \
  _zero_arr_inplace_plus_post_longdouble_fn((long double **)(x),(i),__FILE__,__LINE__)
static _INLINE void **
_zero_arr_inplace_plus_post_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno){
  void ** _zap_res = *x;
  *x = _zero_arr_plus_voidstar_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_voidstar(x,i) \
  _zero_arr_inplace_plus_post_voidstar_fn((void***)(x),(i),__FILE__,__LINE__)

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
_check_dyneither_subscript_fn(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index,const char *filename, unsigned lineno) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  /* JGM: not needed! if (!_cus_arr.base) _throw_null(); */ 
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds_fn(filename,lineno);
  return _cus_ans;
}
#define _check_dyneither_subscript(a,s,i) \
  _check_dyneither_subscript_fn(a,s,i,__FILE__,__LINE__)
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
_untag_dyneither_ptr_fn(struct _dyneither_ptr arr, 
                        unsigned elt_sz,unsigned num_elts,
                        const char *filename, unsigned lineno) {
  struct _dyneither_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds_fn(filename,lineno);
  return _curr;
}
#define _untag_dyneither_ptr(a,s,e) \
  _untag_dyneither_ptr_fn(a,s,e,__FILE__,__LINE__)
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

#define _CYC_MAX_REGION_CONST 2
#define _CYC_MIN_ALIGNMENT (sizeof(double))

#ifdef CYC_REGION_PROFILE
extern int rgn_total_bytes;
#endif

static _INLINE void *_fast_region_malloc(struct _RegionHandle *r, unsigned orig_s) {  
  if (r > (struct _RegionHandle *)_CYC_MAX_REGION_CONST && r->curr != 0) { 
#ifdef CYC_NOALIGN
    unsigned s =  orig_s;
#else
    unsigned s =  (orig_s + _CYC_MIN_ALIGNMENT - 1) & (~(_CYC_MIN_ALIGNMENT -1)); 
#endif
    char *result; 
    result = r->offset; 
    if (s <= (r->last_plus_one - result)) {
      r->offset = result + s; 
#ifdef CYC_REGION_PROFILE
    r->curr->free_bytes = r->curr->free_bytes - s;
    rgn_total_bytes += s;
#endif
      return result;
    }
  } 
  return _region_malloc(r,orig_s); 
}

/* FIX?  Not sure if we want to pass filename and lineno in here... */
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
extern void* _profile_region_calloc(struct _RegionHandle *, unsigned,
                                    unsigned,
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
#define _region_calloc(rh,n,t) _profile_region_calloc(rh,n,t,__FILE__,__FUNCTION__,__LINE__)
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

# 35 "core.h"
 typedef char*Cyc_Cstring;
typedef char*Cyc_CstringNN;
typedef struct _dyneither_ptr Cyc_string_t;
# 40
typedef struct _dyneither_ptr Cyc_mstring_t;
# 43
typedef struct _dyneither_ptr*Cyc_stringptr_t;
# 47
typedef struct _dyneither_ptr*Cyc_mstringptr_t;
# 50
typedef char*Cyc_Cbuffer_t;
# 52
typedef char*Cyc_CbufferNN_t;
# 54
typedef struct _dyneither_ptr Cyc_buffer_t;
# 56
typedef struct _dyneither_ptr Cyc_mbuffer_t;
# 59
typedef int Cyc_bool;
# 26 "cycboot.h"
typedef unsigned long Cyc_size_t;
# 33
typedef unsigned short Cyc_mode_t;struct Cyc___cycFILE;
# 49
typedef struct Cyc___cycFILE Cyc_FILE;
# 53
extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_String_pa_PrintArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_PrintArg_struct{int tag;double f1;};struct Cyc_LongDouble_pa_PrintArg_struct{int tag;long double f1;};struct Cyc_ShortPtr_pa_PrintArg_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_PrintArg_struct{int tag;unsigned long*f1;};
# 68
typedef void*Cyc_parg_t;
# 73
struct _dyneither_ptr Cyc_aprintf(struct _dyneither_ptr,struct _dyneither_ptr);
# 100
int Cyc_fprintf(struct Cyc___cycFILE*,struct _dyneither_ptr,struct _dyneither_ptr);struct Cyc_ShortPtr_sa_ScanfArg_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_ScanfArg_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_ScanfArg_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_ScanfArg_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_ScanfArg_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_ScanfArg_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};
# 127
typedef void*Cyc_sarg_t;
# 157 "cycboot.h"
int Cyc_printf(struct _dyneither_ptr,struct _dyneither_ptr);extern char Cyc_FileCloseError[15];struct Cyc_FileCloseError_exn_struct{char*tag;};extern char Cyc_FileOpenError[14];struct Cyc_FileOpenError_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 79 "core.h"
typedef unsigned int Cyc_Core_sizeof_t;struct Cyc_Core_Opt{void*v;};
# 83
typedef struct Cyc_Core_Opt*Cyc_Core_opt_t;extern char Cyc_Core_Invalid_argument[17];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[8];struct Cyc_Core_Failure_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[11];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[10];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12];struct Cyc_Core_Unreachable_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 170 "core.h"
extern struct _RegionHandle*Cyc_Core_unique_region;struct Cyc_Core_DynamicRegion;
# 205
typedef struct Cyc_Core_DynamicRegion*Cyc_Core_region_key_t;
# 211
typedef struct Cyc_Core_DynamicRegion*Cyc_Core_uregion_key_t;
# 216
typedef struct Cyc_Core_DynamicRegion*Cyc_Core_rcregion_key_t;struct Cyc_Core_NewDynamicRegion{struct Cyc_Core_DynamicRegion*key;};
# 299 "core.h"
typedef void*Cyc_Core___cyclone_internal_array_t;
typedef unsigned int Cyc_Core___cyclone_internal_singleton;
# 303
inline static void* arrcast(struct _dyneither_ptr dyn,unsigned int bd,unsigned int sz){
# 308
if(bd >> 20  || sz >> 12)
return 0;{
unsigned char*ptrbd=dyn.curr + bd * sz;
if(((ptrbd < dyn.curr  || dyn.curr == 0) || dyn.curr < dyn.base) || ptrbd > dyn.last_plus_one)
# 315
return 0;
return dyn.curr;};}struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 39 "list.h"
typedef struct Cyc_List_List*Cyc_List_list_t;
# 49 "list.h"
typedef struct Cyc_List_List*Cyc_List_List_t;
# 54
struct Cyc_List_List*Cyc_List_list(struct _dyneither_ptr);
# 76
struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);
# 83
struct Cyc_List_List*Cyc_List_map_c(void*(*f)(void*,void*),void*env,struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[14];struct Cyc_List_List_mismatch_exn_struct{char*tag;};
# 135
void Cyc_List_iter_c(void(*f)(void*,void*),void*env,struct Cyc_List_List*x);
# 178
struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*x);
# 210
struct Cyc_List_List*Cyc_List_merge_sort(int(*cmp)(void*,void*),struct Cyc_List_List*x);extern char Cyc_List_Nth[4];struct Cyc_List_Nth_exn_struct{char*tag;};
# 49 "string.h"
int Cyc_strcmp(struct _dyneither_ptr s1,struct _dyneither_ptr s2);
int Cyc_strptrcmp(struct _dyneither_ptr*s1,struct _dyneither_ptr*s2);
# 64
struct _dyneither_ptr Cyc_strconcat_l(struct Cyc_List_List*);struct Cyc_Lineno_Pos{struct _dyneither_ptr logical_file;struct _dyneither_ptr line;int line_no;int col;};
# 32 "lineno.h"
typedef struct Cyc_Lineno_Pos*Cyc_Lineno_pos_t;extern char Cyc_Position_Exit[5];struct Cyc_Position_Exit_exn_struct{char*tag;};
# 37 "position.h"
typedef unsigned int Cyc_Position_seg_t;
# 42
struct Cyc_List_List*Cyc_Position_strings_of_segments(struct Cyc_List_List*);struct Cyc_Position_Lex_Position_Error_kind_struct{int tag;};struct Cyc_Position_Parse_Position_Error_kind_struct{int tag;};struct Cyc_Position_Elab_Position_Error_kind_struct{int tag;};
# 46
typedef void*Cyc_Position_error_kind_t;struct Cyc_Position_Error{struct _dyneither_ptr source;unsigned int seg;void*kind;struct _dyneither_ptr desc;};
# 53
typedef struct Cyc_Position_Error*Cyc_Position_error_t;extern char Cyc_Position_Nocontext[10];struct Cyc_Position_Nocontext_exn_struct{char*tag;};
# 61
extern int Cyc_Position_use_gcc_style_location;struct Cyc_Relations_Reln;
# 77 "absyn.h"
typedef struct Cyc_Relations_Reln*Cyc_Relations_reln_t;
typedef struct Cyc_List_List*Cyc_Relations_relns_t;
# 83
typedef void*Cyc_Tcpat_decision_opt_t;
# 91
typedef struct _dyneither_ptr*Cyc_Absyn_field_name_t;
typedef struct _dyneither_ptr*Cyc_Absyn_var_t;
typedef struct _dyneither_ptr*Cyc_Absyn_tvarname_t;
typedef struct _dyneither_ptr*Cyc_Absyn_var_opt_t;struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};
# 103
typedef union Cyc_Absyn_Nmspace Cyc_Absyn_nmspace_t;
union Cyc_Absyn_Nmspace Cyc_Absyn_Loc_n;
union Cyc_Absyn_Nmspace Cyc_Absyn_Rel_n(struct Cyc_List_List*);
# 107
union Cyc_Absyn_Nmspace Cyc_Absyn_Abs_n(struct Cyc_List_List*ns,int C_scope);struct _tuple0{union Cyc_Absyn_Nmspace f1;struct _dyneither_ptr*f2;};
# 110
typedef struct _tuple0*Cyc_Absyn_qvar_t;typedef struct _tuple0*Cyc_Absyn_qvar_opt_t;
typedef struct _tuple0*Cyc_Absyn_typedef_name_t;
typedef struct _tuple0*Cyc_Absyn_typedef_name_opt_t;
# 115
typedef enum Cyc_Absyn_Scope Cyc_Absyn_scope_t;
typedef struct Cyc_Absyn_Tqual Cyc_Absyn_tqual_t;
typedef enum Cyc_Absyn_Size_of Cyc_Absyn_size_of_t;
typedef struct Cyc_Absyn_Kind*Cyc_Absyn_kind_t;
typedef void*Cyc_Absyn_kindbound_t;
typedef struct Cyc_Absyn_Tvar*Cyc_Absyn_tvar_t;
typedef enum Cyc_Absyn_Sign Cyc_Absyn_sign_t;
typedef enum Cyc_Absyn_AggrKind Cyc_Absyn_aggr_kind_t;
typedef void*Cyc_Absyn_bounds_t;
typedef struct Cyc_Absyn_PtrAtts Cyc_Absyn_ptr_atts_t;
typedef struct Cyc_Absyn_PtrInfo Cyc_Absyn_ptr_info_t;
typedef struct Cyc_Absyn_VarargInfo Cyc_Absyn_vararg_info_t;
typedef struct Cyc_Absyn_FnInfo Cyc_Absyn_fn_info_t;
typedef struct Cyc_Absyn_DatatypeInfo Cyc_Absyn_datatype_info_t;
typedef struct Cyc_Absyn_DatatypeFieldInfo Cyc_Absyn_datatype_field_info_t;
typedef struct Cyc_Absyn_AggrInfo Cyc_Absyn_aggr_info_t;
typedef struct Cyc_Absyn_ArrayInfo Cyc_Absyn_array_info_t;
typedef void*Cyc_Absyn_type_t;typedef void*Cyc_Absyn_rgntype_t;typedef void*Cyc_Absyn_type_opt_t;
typedef union Cyc_Absyn_Cnst Cyc_Absyn_cnst_t;
typedef enum Cyc_Absyn_Primop Cyc_Absyn_primop_t;
typedef enum Cyc_Absyn_Incrementor Cyc_Absyn_incrementor_t;
typedef struct Cyc_Absyn_VarargCallInfo Cyc_Absyn_vararg_call_info_t;
typedef void*Cyc_Absyn_raw_exp_t;
typedef struct Cyc_Absyn_Exp*Cyc_Absyn_exp_t;typedef struct Cyc_Absyn_Exp*Cyc_Absyn_exp_opt_t;
typedef void*Cyc_Absyn_raw_stmt_t;
typedef struct Cyc_Absyn_Stmt*Cyc_Absyn_stmt_t;typedef struct Cyc_Absyn_Stmt*Cyc_Absyn_stmt_opt_t;
typedef void*Cyc_Absyn_raw_pat_t;
typedef struct Cyc_Absyn_Pat*Cyc_Absyn_pat_t;
typedef void*Cyc_Absyn_binding_t;
typedef struct Cyc_Absyn_Switch_clause*Cyc_Absyn_switch_clause_t;
typedef struct Cyc_Absyn_Fndecl*Cyc_Absyn_fndecl_t;
typedef struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_aggrdecl_t;
typedef struct Cyc_Absyn_Datatypefield*Cyc_Absyn_datatypefield_t;
typedef struct Cyc_Absyn_Datatypedecl*Cyc_Absyn_datatypedecl_t;
typedef struct Cyc_Absyn_Typedefdecl*Cyc_Absyn_typedefdecl_t;
typedef struct Cyc_Absyn_Enumfield*Cyc_Absyn_enumfield_t;
typedef struct Cyc_Absyn_Enumdecl*Cyc_Absyn_enumdecl_t;
typedef struct Cyc_Absyn_Vardecl*Cyc_Absyn_vardecl_t;typedef struct Cyc_Absyn_Vardecl*Cyc_Absyn_vardecl_opt_t;
typedef void*Cyc_Absyn_raw_decl_t;
typedef struct Cyc_Absyn_Decl*Cyc_Absyn_decl_t;
typedef void*Cyc_Absyn_designator_t;
typedef void*Cyc_Absyn_absyn_annot_t;
typedef void*Cyc_Absyn_attribute_t;
typedef struct Cyc_List_List*Cyc_Absyn_attributes_t;
typedef struct Cyc_Absyn_Aggrfield*Cyc_Absyn_aggrfield_t;
typedef void*Cyc_Absyn_offsetof_field_t;
typedef struct Cyc_Absyn_MallocInfo Cyc_Absyn_malloc_info_t;
typedef enum Cyc_Absyn_Coercion Cyc_Absyn_coercion_t;
typedef struct Cyc_Absyn_PtrLoc*Cyc_Absyn_ptrloc_t;
# 166
enum Cyc_Absyn_Scope{Cyc_Absyn_Static  = 0,Cyc_Absyn_Abstract  = 1,Cyc_Absyn_Public  = 2,Cyc_Absyn_Extern  = 3,Cyc_Absyn_ExternC  = 4,Cyc_Absyn_Register  = 5};struct Cyc_Absyn_Tqual{int print_const;int q_volatile;int q_restrict;int real_const;unsigned int loc;};
# 187
enum Cyc_Absyn_Size_of{Cyc_Absyn_Char_sz  = 0,Cyc_Absyn_Short_sz  = 1,Cyc_Absyn_Int_sz  = 2,Cyc_Absyn_Long_sz  = 3,Cyc_Absyn_LongLong_sz  = 4};
# 192
enum Cyc_Absyn_AliasQual{Cyc_Absyn_Aliasable  = 0,Cyc_Absyn_Unique  = 1,Cyc_Absyn_Top  = 2};
# 199
enum Cyc_Absyn_KindQual{Cyc_Absyn_AnyKind  = 0,Cyc_Absyn_MemKind  = 1,Cyc_Absyn_BoxKind  = 2,Cyc_Absyn_RgnKind  = 3,Cyc_Absyn_EffKind  = 4,Cyc_Absyn_IntKind  = 5};struct Cyc_Absyn_Kind{enum Cyc_Absyn_KindQual kind;enum Cyc_Absyn_AliasQual aliasqual;};
# 219
enum Cyc_Absyn_Sign{Cyc_Absyn_Signed  = 0,Cyc_Absyn_Unsigned  = 1,Cyc_Absyn_None  = 2};
# 221
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA  = 0,Cyc_Absyn_UnionA  = 1};struct _union_Constraint_Eq_constr{int tag;void*val;};struct _union_Constraint_Forward_constr{int tag;union Cyc_Absyn_Constraint*val;};struct _union_Constraint_No_constr{int tag;int val;};union Cyc_Absyn_Constraint{struct _union_Constraint_Eq_constr Eq_constr;struct _union_Constraint_Forward_constr Forward_constr;struct _union_Constraint_No_constr No_constr;};
# 230
typedef union Cyc_Absyn_Constraint*Cyc_Absyn_conref_t;struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct{int tag;struct Cyc_Absyn_Kind*f1;};struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_DynEither_b_Absyn_Bounds_struct{int tag;};struct Cyc_Absyn_Upper_b_Absyn_Bounds_struct{int tag;struct Cyc_Absyn_Exp*f1;};
# 256
extern struct Cyc_Absyn_DynEither_b_Absyn_Bounds_struct Cyc_Absyn_DynEither_b_val;struct Cyc_Absyn_PtrLoc{unsigned int ptr_loc;unsigned int rgn_loc;unsigned int zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;union Cyc_Absyn_Constraint*nullable;union Cyc_Absyn_Constraint*bounds;union Cyc_Absyn_Constraint*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_Numelts_ptrqual_Absyn_Pointer_qual_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Region_ptrqual_Absyn_Pointer_qual_struct{int tag;void*f1;};struct Cyc_Absyn_Thin_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Fat_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Zeroterm_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Nozeroterm_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Notnull_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Nullable_ptrqual_Absyn_Pointer_qual_struct{int tag;};
# 291
typedef void*Cyc_Absyn_pointer_qual_t;
typedef struct Cyc_List_List*Cyc_Absyn_pointer_quals_t;struct Cyc_Absyn_VarargInfo{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_typ;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_UnknownDatatypeInfo{struct _tuple0*name;int is_extensible;};struct _union_DatatypeInfoU_UnknownDatatype{int tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct _union_DatatypeInfoU_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};union Cyc_Absyn_DatatypeInfoU{struct _union_DatatypeInfoU_UnknownDatatype UnknownDatatype;struct _union_DatatypeInfoU_KnownDatatype KnownDatatype;};struct Cyc_Absyn_DatatypeInfo{union Cyc_Absyn_DatatypeInfoU datatype_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{struct _tuple0*datatype_name;struct _tuple0*field_name;int is_extensible;};struct _union_DatatypeFieldInfoU_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo val;};struct _tuple1{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct _union_DatatypeFieldInfoU_KnownDatatypefield{int tag;struct _tuple1 val;};union Cyc_Absyn_DatatypeFieldInfoU{struct _union_DatatypeFieldInfoU_UnknownDatatypefield UnknownDatatypefield;struct _union_DatatypeFieldInfoU_KnownDatatypefield KnownDatatypefield;};struct Cyc_Absyn_DatatypeFieldInfo{union Cyc_Absyn_DatatypeFieldInfoU field_info;struct Cyc_List_List*targs;};struct _tuple2{enum Cyc_Absyn_AggrKind f1;struct _tuple0*f2;struct Cyc_Core_Opt*f3;};struct _union_AggrInfoU_UnknownAggr{int tag;struct _tuple2 val;};struct _union_AggrInfoU_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfoU{struct _union_AggrInfoU_UnknownAggr UnknownAggr;struct _union_AggrInfoU_KnownAggr KnownAggr;};struct Cyc_Absyn_AggrInfo{union Cyc_Absyn_AggrInfoU aggr_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;union Cyc_Absyn_Constraint*zero_term;unsigned int zt_loc;};struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};
# 390
typedef void*Cyc_Absyn_raw_type_decl_t;struct Cyc_Absyn_TypeDecl{void*r;unsigned int loc;};
# 395
typedef struct Cyc_Absyn_TypeDecl*Cyc_Absyn_type_decl_t;struct Cyc_Absyn_VoidType_Absyn_Type_struct{int tag;};struct Cyc_Absyn_Evar_Absyn_Type_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_DatatypeType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeInfo f1;};struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeFieldInfo f1;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_Absyn_Type_struct{int tag;enum Cyc_Absyn_Sign f1;enum Cyc_Absyn_Size_of f2;};struct Cyc_Absyn_FloatType_Absyn_Type_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AggrType_Absyn_Type_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_Absyn_Type_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnHandleType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_DynRgnType_Absyn_Type_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TagType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_HeapRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_UniqueRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_RefCntRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_AccessEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_JoinEff_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnsEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct{int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};struct Cyc_Absyn_TypeofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_BuiltinType_Absyn_Type_struct{int tag;struct _dyneither_ptr f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_NoTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;};struct Cyc_Absyn_WithTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;struct Cyc_Absyn_Exp*f6;struct Cyc_Absyn_Exp*f7;};
# 466 "absyn.h"
typedef void*Cyc_Absyn_funcparams_t;
# 469
enum Cyc_Absyn_Format_Type{Cyc_Absyn_Printf_ft  = 0,Cyc_Absyn_Scanf_ft  = 1};struct Cyc_Absyn_Regparm_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Stdcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Cdecl_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Fastcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Noreturn_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Const_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Aligned_att_Absyn_Attribute_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Packed_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Section_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Nocommon_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Shared_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Unused_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Weak_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllimport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllexport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_instrument_function_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Constructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Destructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_check_memory_usage_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Format_att_Absyn_Attribute_struct{int tag;enum Cyc_Absyn_Format_Type f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noconsume_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Pure_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Mode_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Alias_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Always_inline_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Carray_mod_Absyn_Type_modifier_struct{int tag;union Cyc_Absyn_Constraint*f1;unsigned int f2;};struct Cyc_Absyn_ConstArray_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_Exp*f1;union Cyc_Absyn_Constraint*f2;unsigned int f3;};struct Cyc_Absyn_Pointer_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_Absyn_Type_modifier_struct{int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;int f3;};struct Cyc_Absyn_Attributes_mod_Absyn_Type_modifier_struct{int tag;unsigned int f1;struct Cyc_List_List*f2;};
# 533
typedef void*Cyc_Absyn_type_modifier_t;struct _union_Cnst_Null_c{int tag;int val;};struct _tuple3{enum Cyc_Absyn_Sign f1;char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple3 val;};struct _union_Cnst_Wchar_c{int tag;struct _dyneither_ptr val;};struct _tuple4{enum Cyc_Absyn_Sign f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple4 val;};struct _tuple5{enum Cyc_Absyn_Sign f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple5 val;};struct _tuple6{enum Cyc_Absyn_Sign f1;long long f2;};struct _union_Cnst_LongLong_c{int tag;struct _tuple6 val;};struct _tuple7{struct _dyneither_ptr f1;int f2;};struct _union_Cnst_Float_c{int tag;struct _tuple7 val;};struct _union_Cnst_String_c{int tag;struct _dyneither_ptr val;};struct _union_Cnst_Wstring_c{int tag;struct _dyneither_ptr val;};union Cyc_Absyn_Cnst{struct _union_Cnst_Null_c Null_c;struct _union_Cnst_Char_c Char_c;struct _union_Cnst_Wchar_c Wchar_c;struct _union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct _union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct _union_Cnst_String_c String_c;struct _union_Cnst_Wstring_c Wstring_c;};
# 559
enum Cyc_Absyn_Primop{Cyc_Absyn_Plus  = 0,Cyc_Absyn_Times  = 1,Cyc_Absyn_Minus  = 2,Cyc_Absyn_Div  = 3,Cyc_Absyn_Mod  = 4,Cyc_Absyn_Eq  = 5,Cyc_Absyn_Neq  = 6,Cyc_Absyn_Gt  = 7,Cyc_Absyn_Lt  = 8,Cyc_Absyn_Gte  = 9,Cyc_Absyn_Lte  = 10,Cyc_Absyn_Not  = 11,Cyc_Absyn_Bitnot  = 12,Cyc_Absyn_Bitand  = 13,Cyc_Absyn_Bitor  = 14,Cyc_Absyn_Bitxor  = 15,Cyc_Absyn_Bitlshift  = 16,Cyc_Absyn_Bitlrshift  = 17,Cyc_Absyn_Bitarshift  = 18,Cyc_Absyn_Numelts  = 19};
# 566
enum Cyc_Absyn_Incrementor{Cyc_Absyn_PreInc  = 0,Cyc_Absyn_PostInc  = 1,Cyc_Absyn_PreDec  = 2,Cyc_Absyn_PostDec  = 3};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct{int tag;unsigned int f1;};
# 584
enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion  = 0,Cyc_Absyn_No_coercion  = 1,Cyc_Absyn_NonNull_to_Null  = 2,Cyc_Absyn_Other_coercion  = 3};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;int inline_call;};struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct{int tag;union Cyc_Absyn_Cnst f1;};struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;void*f2;};struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct{int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor f2;};struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;int f4;};struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;int f2;};struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct _tuple8{struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct{int tag;struct _tuple8*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;int f3;};struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct{int tag;int f1;struct _dyneither_ptr f2;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple9{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct{int tag;struct _tuple9 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple9 f2;struct _tuple9 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple9 f2;};struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_ResetRegion_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Stmt{void*r;unsigned int loc;struct Cyc_List_List*non_local_preds;int try_depth;void*annot;};struct Cyc_Absyn_Wild_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct{int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_AggrInfo*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Null_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct{int tag;enum Cyc_Absyn_Sign f1;int f2;};struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_Absyn_Raw_pat_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_Absyn_Raw_pat_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;void*topt;unsigned int loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;unsigned int loc;};struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct{int tag;};struct Cyc_Absyn_Global_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Local_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple0*name;struct Cyc_List_List*tvs;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;void*cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind kind;enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*impl;struct Cyc_List_List*attributes;int expected_mem_kind;};struct Cyc_Absyn_Datatypefield{struct _tuple0*name;struct Cyc_List_List*typs;unsigned int loc;enum Cyc_Absyn_Scope sc;};struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct _tuple0*name;struct Cyc_Absyn_Exp*tag;unsigned int loc;};struct Cyc_Absyn_Enumdecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct _tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;int extern_c;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;void*f4;};struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;int f3;struct Cyc_Absyn_Exp*f4;};struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Datatype_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Decl{void*r;unsigned int loc;};struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_Absyn_Designator_struct{int tag;struct _dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[11];struct Cyc_Absyn_EmptyAnnot_Absyn_AbsynAnnot_struct{char*tag;};
# 944 "absyn.h"
int Cyc_Absyn_qvar_cmp(struct _tuple0*,struct _tuple0*);
# 954
struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(unsigned int);
# 958
union Cyc_Absyn_Constraint*Cyc_Absyn_empty_conref();
# 961
void*Cyc_Absyn_conref_val(union Cyc_Absyn_Constraint*x);
# 979
void*Cyc_Absyn_wildtyp(struct Cyc_Core_Opt*);
# 984
extern void*Cyc_Absyn_sint_typ;
# 1001
void*Cyc_Absyn_string_typ(void*rgn);
# 1023
void*Cyc_Absyn_dyneither_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,union Cyc_Absyn_Constraint*zero_term);struct Cyc_PP_Ppstate;
# 41 "pp.h"
typedef struct Cyc_PP_Ppstate*Cyc_PP_ppstate_t;struct Cyc_PP_Out;
# 43
typedef struct Cyc_PP_Out*Cyc_PP_out_t;struct Cyc_PP_Doc;
# 45
typedef struct Cyc_PP_Doc*Cyc_PP_doc_t;struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;};
# 70 "absynpp.h"
struct _dyneither_ptr Cyc_Absynpp_exp2string(struct Cyc_Absyn_Exp*);
# 72
struct _dyneither_ptr Cyc_Absynpp_qvar2string(struct _tuple0*);struct Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};
# 34 "iter.h"
typedef struct Cyc_Iter_Iter Cyc_Iter_iter_t;
# 37
int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Set_Set;
# 40 "set.h"
typedef struct Cyc_Set_Set*Cyc_Set_set_t;extern char Cyc_Set_Absent[7];struct Cyc_Set_Absent_exn_struct{char*tag;};struct Cyc_Dict_T;
# 46 "dict.h"
typedef const struct Cyc_Dict_T*Cyc_Dict_tree;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};
# 52
typedef struct Cyc_Dict_Dict Cyc_Dict_dict_t;extern char Cyc_Dict_Present[8];struct Cyc_Dict_Present_exn_struct{char*tag;};extern char Cyc_Dict_Absent[7];struct Cyc_Dict_Absent_exn_struct{char*tag;};
# 62
struct Cyc_Dict_Dict Cyc_Dict_empty(int(*cmp)(void*,void*));
# 83
int Cyc_Dict_member(struct Cyc_Dict_Dict d,void*k);
# 87
struct Cyc_Dict_Dict Cyc_Dict_insert(struct Cyc_Dict_Dict d,void*k,void*v);
# 110
void*Cyc_Dict_lookup(struct Cyc_Dict_Dict d,void*k);
# 122 "dict.h"
void**Cyc_Dict_lookup_opt(struct Cyc_Dict_Dict d,void*k);struct Cyc_RgnOrder_RgnPO;
# 33 "rgnorder.h"
typedef struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_rgn_po_t;
# 35
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_initial_fn_po(struct _RegionHandle*,struct Cyc_List_List*tvs,struct Cyc_List_List*po,void*effect,struct Cyc_Absyn_Tvar*fst_rgn,unsigned int);
# 42
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn,unsigned int loc);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*rgn,int resetable,int opened);
int Cyc_RgnOrder_is_region_resetable(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*r);
int Cyc_RgnOrder_effect_outlives(struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);
int Cyc_RgnOrder_satisfies_constraints(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_List_List*constraints,void*default_bound,int do_pin);
# 48
int Cyc_RgnOrder_eff_outlives_eff(struct Cyc_RgnOrder_RgnPO*po,void*eff1,void*eff2);
# 51
void Cyc_RgnOrder_print_region_po(struct Cyc_RgnOrder_RgnPO*po);extern char Cyc_Tcenv_Env_error[10];struct Cyc_Tcenv_Env_error_exn_struct{char*tag;};struct Cyc_Tcenv_CList{void*hd;const struct Cyc_Tcenv_CList*tl;};
# 44 "tcenv.h"
typedef const struct Cyc_Tcenv_CList*Cyc_Tcenv_mclist_t;
typedef const struct Cyc_Tcenv_CList*const Cyc_Tcenv_clist_t;struct Cyc_Tcenv_VarRes_Tcenv_Resolved_struct{int tag;void*f1;};struct Cyc_Tcenv_AggrRes_Tcenv_Resolved_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_DatatypeRes_Tcenv_Resolved_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct Cyc_Tcenv_EnumRes_Tcenv_Resolved_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_Tcenv_Resolved_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};
# 55
typedef void*Cyc_Tcenv_resolved_t;struct Cyc_Tcenv_Genv{struct _RegionHandle*grgn;struct Cyc_Set_Set*namespaces;struct Cyc_Dict_Dict aggrdecls;struct Cyc_Dict_Dict datatypedecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict typedefs;struct Cyc_Dict_Dict ordinaries;struct Cyc_List_List*availables;};
# 74
typedef struct Cyc_Tcenv_Genv*Cyc_Tcenv_genv_t;struct Cyc_Tcenv_Fenv;
# 78
typedef struct Cyc_Tcenv_Fenv*Cyc_Tcenv_fenv_t;struct Cyc_Tcenv_NotLoop_j_Tcenv_Jumpee_struct{int tag;};struct Cyc_Tcenv_CaseEnd_j_Tcenv_Jumpee_struct{int tag;};struct Cyc_Tcenv_FnEnd_j_Tcenv_Jumpee_struct{int tag;};struct Cyc_Tcenv_Stmt_j_Tcenv_Jumpee_struct{int tag;struct Cyc_Absyn_Stmt*f1;};
# 89
typedef void*Cyc_Tcenv_jumpee_t;struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict ae;struct Cyc_Tcenv_Fenv*le;int allow_valueof;int in_extern_c_include;};
# 101
typedef struct Cyc_Tcenv_Tenv*Cyc_Tcenv_tenv_t;
# 139 "tcenv.h"
enum Cyc_Tcenv_NewStatus{Cyc_Tcenv_NoneNew  = 0,Cyc_Tcenv_InNew  = 1,Cyc_Tcenv_InNewAggr  = 2};
# 38 "tcutil.h"
void*Cyc_Tcutil_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);extern char Cyc_Tcutil_AbortTypeCheckingFunction[26];struct Cyc_Tcutil_AbortTypeCheckingFunction_exn_struct{char*tag;};struct _tuple10{void*f1;void*f2;};struct Cyc_Tcexp_TestEnv{struct _tuple10*eq;int isTrue;};
# 39 "tcexp.h"
typedef struct Cyc_Tcexp_TestEnv Cyc_Tcexp_testenv_t;
struct Cyc_Tcexp_TestEnv Cyc_Tcexp_tcTest(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e,struct _dyneither_ptr msg_part);extern char Cyc_Tcdecl_Incompatible[13];struct Cyc_Tcdecl_Incompatible_exn_struct{char*tag;};struct Cyc_Tcdecl_Xdatatypefielddecl{struct Cyc_Absyn_Datatypedecl*base;struct Cyc_Absyn_Datatypefield*field;};
# 41 "tcdecl.h"
typedef struct Cyc_Tcdecl_Xdatatypefielddecl*Cyc_Tcdecl_xdatatypefielddecl_t;struct Cyc_Port_Edit{unsigned int loc;struct _dyneither_ptr old_string;struct _dyneither_ptr new_string;};
# 88 "port.cyc"
typedef struct Cyc_Port_Edit*Cyc_Port_edit_t;
# 93
typedef struct Cyc_List_List*Cyc_Port_edits_t;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 95
int Cyc_Port_cmp_edit(struct Cyc_Port_Edit*e1,struct Cyc_Port_Edit*e2){
return(int)e1 - (int)e2;}
# 103
typedef void*Cyc_Port_ctype_t;struct Cyc_Port_Cvar{int id;void**eq;struct Cyc_List_List*uppers;struct Cyc_List_List*lowers;};
# 105
typedef struct Cyc_Port_Cvar*Cyc_Port_cvar_t;struct Cyc_Port_Cfield{void*qual;struct _dyneither_ptr*f;void*type;};
# 112
typedef struct Cyc_Port_Cfield*Cyc_Port_cfield_t;
# 117
typedef struct Cyc_List_List*Cyc_Port_cfields_t;struct Cyc_Port_Const_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Notconst_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Thin_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Fat_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Void_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Zero_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Arith_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Heap_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Zterm_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Nozterm_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_RgnVar_ct_Port_Ctype_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Port_Ptr_ct_Port_Ctype_struct{int tag;void*f1;void*f2;void*f3;void*f4;void*f5;};struct Cyc_Port_Array_ct_Port_Ctype_struct{int tag;void*f1;void*f2;void*f3;};struct _tuple11{struct Cyc_Absyn_Aggrdecl*f1;struct Cyc_List_List*f2;};struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct{int tag;struct _tuple11*f1;};struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct{int tag;struct Cyc_List_List*f1;void**f2;};struct Cyc_Port_Fn_ct_Port_Ctype_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Port_Cvar;struct Cyc_Port_Var_ct_Port_Ctype_struct{int tag;struct Cyc_Port_Cvar*f1;};
# 147
struct Cyc_Port_Const_ct_Port_Ctype_struct Cyc_Port_Const_ct_val={0};
struct Cyc_Port_Notconst_ct_Port_Ctype_struct Cyc_Port_Notconst_ct_val={1};
struct Cyc_Port_Thin_ct_Port_Ctype_struct Cyc_Port_Thin_ct_val={2};
struct Cyc_Port_Fat_ct_Port_Ctype_struct Cyc_Port_Fat_ct_val={3};
struct Cyc_Port_Void_ct_Port_Ctype_struct Cyc_Port_Void_ct_val={4};
struct Cyc_Port_Zero_ct_Port_Ctype_struct Cyc_Port_Zero_ct_val={5};
struct Cyc_Port_Arith_ct_Port_Ctype_struct Cyc_Port_Arith_ct_val={6};
struct Cyc_Port_Heap_ct_Port_Ctype_struct Cyc_Port_Heap_ct_val={7};
struct Cyc_Port_Zterm_ct_Port_Ctype_struct Cyc_Port_Zterm_ct_val={8};
struct Cyc_Port_Nozterm_ct_Port_Ctype_struct Cyc_Port_Nozterm_ct_val={9};
# 160
static struct _dyneither_ptr Cyc_Port_ctypes2string(int deep,struct Cyc_List_List*ts);
static struct _dyneither_ptr Cyc_Port_cfields2string(int deep,struct Cyc_List_List*ts);struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
static struct _dyneither_ptr Cyc_Port_ctype2string(int deep,void*t){
void*_tmpA=t;struct Cyc_Port_Cvar*_tmpB;void*_tmpC;struct Cyc_List_List*_tmpD;struct Cyc_List_List*_tmpE;struct Cyc_List_List*_tmpF;void*_tmp10;struct Cyc_Absyn_Aggrdecl*_tmp11;struct Cyc_List_List*_tmp12;void*_tmp13;void*_tmp14;void*_tmp15;void*_tmp16;void*_tmp17;void*_tmp18;void*_tmp19;void*_tmp1A;struct _dyneither_ptr*_tmp1B;switch(*((int*)_tmpA)){case 0: _LL1: _LL2: {
const char*_tmp38A;return(_tmp38A="const",_tag_dyneither(_tmp38A,sizeof(char),6));}case 1: _LL3: _LL4: {
const char*_tmp38B;return(_tmp38B="notconst",_tag_dyneither(_tmp38B,sizeof(char),9));}case 2: _LL5: _LL6: {
const char*_tmp38C;return(_tmp38C="thin",_tag_dyneither(_tmp38C,sizeof(char),5));}case 3: _LL7: _LL8: {
const char*_tmp38D;return(_tmp38D="fat",_tag_dyneither(_tmp38D,sizeof(char),4));}case 4: _LL9: _LLA: {
const char*_tmp38E;return(_tmp38E="void",_tag_dyneither(_tmp38E,sizeof(char),5));}case 5: _LLB: _LLC: {
const char*_tmp38F;return(_tmp38F="zero",_tag_dyneither(_tmp38F,sizeof(char),5));}case 6: _LLD: _LLE: {
const char*_tmp390;return(_tmp390="arith",_tag_dyneither(_tmp390,sizeof(char),6));}case 7: _LLF: _LL10: {
const char*_tmp391;return(_tmp391="heap",_tag_dyneither(_tmp391,sizeof(char),5));}case 8: _LL11: _LL12: {
const char*_tmp392;return(_tmp392="ZT",_tag_dyneither(_tmp392,sizeof(char),3));}case 9: _LL13: _LL14: {
const char*_tmp393;return(_tmp393="NZT",_tag_dyneither(_tmp393,sizeof(char),4));}case 10: _LL15: _tmp1B=((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmpA)->f1;_LL16: {
const char*_tmp397;void*_tmp396[1];struct Cyc_String_pa_PrintArg_struct _tmp395;return(struct _dyneither_ptr)((_tmp395.tag=0,((_tmp395.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp1B),((_tmp396[0]=& _tmp395,Cyc_aprintf(((_tmp397="%s",_tag_dyneither(_tmp397,sizeof(char),3))),_tag_dyneither(_tmp396,sizeof(void*),1))))))));}case 11: _LL17: _tmp16=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA)->f1;_tmp17=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA)->f2;_tmp18=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA)->f3;_tmp19=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA)->f4;_tmp1A=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA)->f5;_LL18: {
# 176
const char*_tmp39F;void*_tmp39E[5];struct Cyc_String_pa_PrintArg_struct _tmp39D;struct Cyc_String_pa_PrintArg_struct _tmp39C;struct Cyc_String_pa_PrintArg_struct _tmp39B;struct Cyc_String_pa_PrintArg_struct _tmp39A;struct Cyc_String_pa_PrintArg_struct _tmp399;return(struct _dyneither_ptr)((_tmp399.tag=0,((_tmp399.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 178
Cyc_Port_ctype2string(deep,_tmp1A)),((_tmp39A.tag=0,((_tmp39A.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp19)),((_tmp39B.tag=0,((_tmp39B.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 177
Cyc_Port_ctype2string(deep,_tmp18)),((_tmp39C.tag=0,((_tmp39C.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp17)),((_tmp39D.tag=0,((_tmp39D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 176
Cyc_Port_ctype2string(deep,_tmp16)),((_tmp39E[0]=& _tmp39D,((_tmp39E[1]=& _tmp39C,((_tmp39E[2]=& _tmp39B,((_tmp39E[3]=& _tmp39A,((_tmp39E[4]=& _tmp399,Cyc_aprintf(((_tmp39F="ptr(%s,%s,%s,%s,%s)",_tag_dyneither(_tmp39F,sizeof(char),20))),_tag_dyneither(_tmp39E,sizeof(void*),5))))))))))))))))))))))))))))))));}case 12: _LL19: _tmp13=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpA)->f1;_tmp14=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpA)->f2;_tmp15=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpA)->f3;_LL1A: {
# 180
const char*_tmp3A5;void*_tmp3A4[3];struct Cyc_String_pa_PrintArg_struct _tmp3A3;struct Cyc_String_pa_PrintArg_struct _tmp3A2;struct Cyc_String_pa_PrintArg_struct _tmp3A1;return(struct _dyneither_ptr)((_tmp3A1.tag=0,((_tmp3A1.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Port_ctype2string(deep,_tmp15)),((_tmp3A2.tag=0,((_tmp3A2.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp14)),((_tmp3A3.tag=0,((_tmp3A3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 180
Cyc_Port_ctype2string(deep,_tmp13)),((_tmp3A4[0]=& _tmp3A3,((_tmp3A4[1]=& _tmp3A2,((_tmp3A4[2]=& _tmp3A1,Cyc_aprintf(((_tmp3A5="array(%s,%s,%s)",_tag_dyneither(_tmp3A5,sizeof(char),16))),_tag_dyneither(_tmp3A4,sizeof(void*),3))))))))))))))))))));}case 13: _LL1B: _tmp11=(((struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpA)->f1)->f1;_tmp12=(((struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpA)->f1)->f2;_LL1C: {
# 183
const char*_tmp3A7;const char*_tmp3A6;struct _dyneither_ptr s=_tmp11->kind == Cyc_Absyn_StructA?(_tmp3A7="struct",_tag_dyneither(_tmp3A7,sizeof(char),7)):((_tmp3A6="union",_tag_dyneither(_tmp3A6,sizeof(char),6)));
if(!deep){
const char*_tmp3AC;void*_tmp3AB[2];struct Cyc_String_pa_PrintArg_struct _tmp3AA;struct Cyc_String_pa_PrintArg_struct _tmp3A9;return(struct _dyneither_ptr)((_tmp3A9.tag=0,((_tmp3A9.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp11->name)),((_tmp3AA.tag=0,((_tmp3AA.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s),((_tmp3AB[0]=& _tmp3AA,((_tmp3AB[1]=& _tmp3A9,Cyc_aprintf(((_tmp3AC="%s %s",_tag_dyneither(_tmp3AC,sizeof(char),6))),_tag_dyneither(_tmp3AB,sizeof(void*),2))))))))))))));}else{
# 187
const char*_tmp3B2;void*_tmp3B1[3];struct Cyc_String_pa_PrintArg_struct _tmp3B0;struct Cyc_String_pa_PrintArg_struct _tmp3AF;struct Cyc_String_pa_PrintArg_struct _tmp3AE;return(struct _dyneither_ptr)((_tmp3AE.tag=0,((_tmp3AE.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Port_cfields2string(0,_tmp12)),((_tmp3AF.tag=0,((_tmp3AF.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 187
Cyc_Absynpp_qvar2string(_tmp11->name)),((_tmp3B0.tag=0,((_tmp3B0.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s),((_tmp3B1[0]=& _tmp3B0,((_tmp3B1[1]=& _tmp3AF,((_tmp3B1[2]=& _tmp3AE,Cyc_aprintf(((_tmp3B2="%s %s {%s}",_tag_dyneither(_tmp3B2,sizeof(char),11))),_tag_dyneither(_tmp3B1,sizeof(void*),3))))))))))))))))))));}}case 14: if(((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpA)->f2 != 0){_LL1D: _tmpF=((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpA)->f1;_tmp10=*((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpA)->f2;_LL1E:
# 189
 return Cyc_Port_ctype2string(deep,_tmp10);}else{_LL1F: _tmpE=((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpA)->f1;_LL20: {
# 191
const char*_tmp3B6;void*_tmp3B5[1];struct Cyc_String_pa_PrintArg_struct _tmp3B4;return(struct _dyneither_ptr)((_tmp3B4.tag=0,((_tmp3B4.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_cfields2string(deep,_tmpE)),((_tmp3B5[0]=& _tmp3B4,Cyc_aprintf(((_tmp3B6="aggr {%s}",_tag_dyneither(_tmp3B6,sizeof(char),10))),_tag_dyneither(_tmp3B5,sizeof(void*),1))))))));}}case 15: _LL21: _tmpC=(void*)((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpA)->f1;_tmpD=((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpA)->f2;_LL22: {
# 193
const char*_tmp3BB;void*_tmp3BA[2];struct Cyc_String_pa_PrintArg_struct _tmp3B9;struct Cyc_String_pa_PrintArg_struct _tmp3B8;return(struct _dyneither_ptr)((_tmp3B8.tag=0,((_tmp3B8.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmpC)),((_tmp3B9.tag=0,((_tmp3B9.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctypes2string(deep,_tmpD)),((_tmp3BA[0]=& _tmp3B9,((_tmp3BA[1]=& _tmp3B8,Cyc_aprintf(((_tmp3BB="fn(%s)->%s",_tag_dyneither(_tmp3BB,sizeof(char),11))),_tag_dyneither(_tmp3BA,sizeof(void*),2))))))))))))));}default: _LL23: _tmpB=((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmpA)->f1;_LL24:
# 195
 if((unsigned int)_tmpB->eq)
return Cyc_Port_ctype2string(deep,*((void**)_check_null(_tmpB->eq)));else{
if(!deep  || _tmpB->uppers == 0  && _tmpB->lowers == 0){
const char*_tmp3BF;void*_tmp3BE[1];struct Cyc_Int_pa_PrintArg_struct _tmp3BD;return(struct _dyneither_ptr)((_tmp3BD.tag=1,((_tmp3BD.f1=(unsigned long)_tmpB->id,((_tmp3BE[0]=& _tmp3BD,Cyc_aprintf(((_tmp3BF="var(%d)",_tag_dyneither(_tmp3BF,sizeof(char),8))),_tag_dyneither(_tmp3BE,sizeof(void*),1))))))));}else{
const char*_tmp3C5;void*_tmp3C4[3];struct Cyc_String_pa_PrintArg_struct _tmp3C3;struct Cyc_Int_pa_PrintArg_struct _tmp3C2;struct Cyc_String_pa_PrintArg_struct _tmp3C1;return(struct _dyneither_ptr)((_tmp3C1.tag=0,((_tmp3C1.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 202
Cyc_Port_ctypes2string(0,_tmpB->uppers)),((_tmp3C2.tag=1,((_tmp3C2.f1=(unsigned long)_tmpB->id,((_tmp3C3.tag=0,((_tmp3C3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 200
Cyc_Port_ctypes2string(0,_tmpB->lowers)),((_tmp3C4[0]=& _tmp3C3,((_tmp3C4[1]=& _tmp3C2,((_tmp3C4[2]=& _tmp3C1,Cyc_aprintf(((_tmp3C5="var([%s]<=%d<=[%s])",_tag_dyneither(_tmp3C5,sizeof(char),20))),_tag_dyneither(_tmp3C4,sizeof(void*),3))))))))))))))))))));}}}_LL0:;}
# 205
static struct _dyneither_ptr*Cyc_Port_ctype2stringptr(int deep,void*t){struct _dyneither_ptr*_tmp3C6;return(_tmp3C6=_cycalloc(sizeof(*_tmp3C6)),((_tmp3C6[0]=Cyc_Port_ctype2string(deep,t),_tmp3C6)));}
struct Cyc_List_List*Cyc_Port_sep(struct _dyneither_ptr s,struct Cyc_List_List*xs){
struct _dyneither_ptr*_tmp3C7;struct _dyneither_ptr*_tmp50=(_tmp3C7=_cycalloc(sizeof(*_tmp3C7)),((_tmp3C7[0]=s,_tmp3C7)));
if(xs == 0)return xs;{
struct Cyc_List_List*_tmp51=xs;
struct Cyc_List_List*_tmp52=xs->tl;
for(0;_tmp52 != 0;(_tmp51=_tmp52,_tmp52=_tmp52->tl)){
struct Cyc_List_List*_tmp3C8;_tmp51->tl=((_tmp3C8=_cycalloc(sizeof(*_tmp3C8)),((_tmp3C8->hd=_tmp50,((_tmp3C8->tl=_tmp52,_tmp3C8))))));}
# 214
return xs;};}struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 216
static struct _dyneither_ptr*Cyc_Port_cfield2stringptr(int deep,struct Cyc_Port_Cfield*f){
const char*_tmp3CE;void*_tmp3CD[3];struct Cyc_String_pa_PrintArg_struct _tmp3CC;struct Cyc_String_pa_PrintArg_struct _tmp3CB;struct Cyc_String_pa_PrintArg_struct _tmp3CA;struct _dyneither_ptr s=(struct _dyneither_ptr)(
(_tmp3CA.tag=0,((_tmp3CA.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,f->type)),((_tmp3CB.tag=0,((_tmp3CB.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f->f),((_tmp3CC.tag=0,((_tmp3CC.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,f->qual)),((_tmp3CD[0]=& _tmp3CC,((_tmp3CD[1]=& _tmp3CB,((_tmp3CD[2]=& _tmp3CA,Cyc_aprintf(((_tmp3CE="%s %s: %s",_tag_dyneither(_tmp3CE,sizeof(char),10))),_tag_dyneither(_tmp3CD,sizeof(void*),3))))))))))))))))))));
struct _dyneither_ptr*_tmp3CF;return(_tmp3CF=_cycalloc(sizeof(*_tmp3CF)),((_tmp3CF[0]=s,_tmp3CF)));}
# 221
static struct _dyneither_ptr Cyc_Port_ctypes2string(int deep,struct Cyc_List_List*ts){
const char*_tmp3D0;return(struct _dyneither_ptr)Cyc_strconcat_l(Cyc_Port_sep(((_tmp3D0=",",_tag_dyneither(_tmp3D0,sizeof(char),2))),((struct Cyc_List_List*(*)(struct _dyneither_ptr*(*f)(int,void*),int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_ctype2stringptr,deep,ts)));}struct Cyc_Port_Cfield;
# 224
static struct _dyneither_ptr Cyc_Port_cfields2string(int deep,struct Cyc_List_List*fs){
const char*_tmp3D1;return(struct _dyneither_ptr)Cyc_strconcat_l(Cyc_Port_sep(((_tmp3D1=";",_tag_dyneither(_tmp3D1,sizeof(char),2))),((struct Cyc_List_List*(*)(struct _dyneither_ptr*(*f)(int,struct Cyc_Port_Cfield*),int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_cfield2stringptr,deep,fs)));}
# 230
static void*Cyc_Port_notconst_ct(){return(void*)& Cyc_Port_Notconst_ct_val;}
static void*Cyc_Port_const_ct(){return(void*)& Cyc_Port_Const_ct_val;}
static void*Cyc_Port_thin_ct(){return(void*)& Cyc_Port_Thin_ct_val;}
static void*Cyc_Port_fat_ct(){return(void*)& Cyc_Port_Fat_ct_val;}
static void*Cyc_Port_void_ct(){return(void*)& Cyc_Port_Void_ct_val;}
static void*Cyc_Port_zero_ct(){return(void*)& Cyc_Port_Zero_ct_val;}
static void*Cyc_Port_arith_ct(){return(void*)& Cyc_Port_Arith_ct_val;}
static void*Cyc_Port_heap_ct(){return(void*)& Cyc_Port_Heap_ct_val;}
static void*Cyc_Port_zterm_ct(){return(void*)& Cyc_Port_Zterm_ct_val;}
static void*Cyc_Port_nozterm_ct(){return(void*)& Cyc_Port_Nozterm_ct_val;}
static void*Cyc_Port_rgnvar_ct(struct _dyneither_ptr*n){struct Cyc_Port_RgnVar_ct_Port_Ctype_struct _tmp3D4;struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp3D3;return(void*)((_tmp3D3=_cycalloc(sizeof(*_tmp3D3)),((_tmp3D3[0]=((_tmp3D4.tag=10,((_tmp3D4.f1=n,_tmp3D4)))),_tmp3D3))));}
static void*Cyc_Port_unknown_aggr_ct(struct Cyc_List_List*fs){
struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct _tmp3D7;struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp3D6;return(void*)((_tmp3D6=_cycalloc(sizeof(*_tmp3D6)),((_tmp3D6[0]=((_tmp3D7.tag=14,((_tmp3D7.f1=fs,((_tmp3D7.f2=0,_tmp3D7)))))),_tmp3D6))));}
# 244
static void*Cyc_Port_known_aggr_ct(struct _tuple11*p){
struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct _tmp3DA;struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp3D9;return(void*)((_tmp3D9=_cycalloc(sizeof(*_tmp3D9)),((_tmp3D9[0]=((_tmp3DA.tag=13,((_tmp3DA.f1=p,_tmp3DA)))),_tmp3D9))));}
# 247
static void*Cyc_Port_ptr_ct(void*elt,void*qual,void*ptr_kind,void*r,void*zt){
# 249
struct Cyc_Port_Ptr_ct_Port_Ctype_struct _tmp3DD;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp3DC;return(void*)((_tmp3DC=_cycalloc(sizeof(*_tmp3DC)),((_tmp3DC[0]=((_tmp3DD.tag=11,((_tmp3DD.f1=elt,((_tmp3DD.f2=qual,((_tmp3DD.f3=ptr_kind,((_tmp3DD.f4=r,((_tmp3DD.f5=zt,_tmp3DD)))))))))))),_tmp3DC))));}
# 251
static void*Cyc_Port_array_ct(void*elt,void*qual,void*zt){
struct Cyc_Port_Array_ct_Port_Ctype_struct _tmp3E0;struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp3DF;return(void*)((_tmp3DF=_cycalloc(sizeof(*_tmp3DF)),((_tmp3DF[0]=((_tmp3E0.tag=12,((_tmp3E0.f1=elt,((_tmp3E0.f2=qual,((_tmp3E0.f3=zt,_tmp3E0)))))))),_tmp3DF))));}
# 254
static void*Cyc_Port_fn_ct(void*return_type,struct Cyc_List_List*args){
struct Cyc_Port_Fn_ct_Port_Ctype_struct _tmp3E3;struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp3E2;return(void*)((_tmp3E2=_cycalloc(sizeof(*_tmp3E2)),((_tmp3E2[0]=((_tmp3E3.tag=15,((_tmp3E3.f1=return_type,((_tmp3E3.f2=args,_tmp3E3)))))),_tmp3E2))));}struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
# 257
static void*Cyc_Port_var(){
static int counter=0;
struct Cyc_Port_Var_ct_Port_Ctype_struct _tmp3E9;struct Cyc_Port_Cvar*_tmp3E8;struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp3E7;return(void*)((_tmp3E7=_cycalloc(sizeof(*_tmp3E7)),((_tmp3E7[0]=((_tmp3E9.tag=16,((_tmp3E9.f1=((_tmp3E8=_cycalloc(sizeof(*_tmp3E8)),((_tmp3E8->id=counter ++,((_tmp3E8->eq=0,((_tmp3E8->uppers=0,((_tmp3E8->lowers=0,_tmp3E8)))))))))),_tmp3E9)))),_tmp3E7))));}
# 261
static void*Cyc_Port_new_var(void*x){
return Cyc_Port_var();}
# 264
static struct _dyneither_ptr*Cyc_Port_new_region_var(){
static int counter=0;
const char*_tmp3ED;void*_tmp3EC[1];struct Cyc_Int_pa_PrintArg_struct _tmp3EB;struct _dyneither_ptr s=(struct _dyneither_ptr)((_tmp3EB.tag=1,((_tmp3EB.f1=(unsigned long)counter ++,((_tmp3EC[0]=& _tmp3EB,Cyc_aprintf(((_tmp3ED="`R%d",_tag_dyneither(_tmp3ED,sizeof(char),5))),_tag_dyneither(_tmp3EC,sizeof(void*),1))))))));
struct _dyneither_ptr*_tmp3EE;return(_tmp3EE=_cycalloc(sizeof(*_tmp3EE)),((_tmp3EE[0]=s,_tmp3EE)));}
# 272
static int Cyc_Port_unifies(void*t1,void*t2);struct Cyc_Port_Cvar;
# 274
static void*Cyc_Port_compress_ct(void*t){
void*_tmp70=t;void**_tmp71;void***_tmp72;struct Cyc_List_List*_tmp73;struct Cyc_List_List*_tmp74;switch(*((int*)_tmp70)){case 16: _LL26: _tmp72=(void***)&(((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp70)->f1)->eq;_tmp73=(((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp70)->f1)->uppers;_tmp74=(((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp70)->f1)->lowers;_LL27: {
# 277
void**_tmp75=*_tmp72;
if((unsigned int)_tmp75){
# 281
void*r=Cyc_Port_compress_ct(*_tmp75);
if(*_tmp75 != r){void**_tmp3EF;*_tmp72=((_tmp3EF=_cycalloc(sizeof(*_tmp3EF)),((_tmp3EF[0]=r,_tmp3EF))));}
return r;}
# 288
for(0;_tmp74 != 0;_tmp74=_tmp74->tl){
void*_tmp77=(void*)_tmp74->hd;void*_tmp78=_tmp77;switch(*((int*)_tmp78)){case 0: _LL2D: _LL2E:
 goto _LL30;case 9: _LL2F: _LL30:
 goto _LL32;case 4: _LL31: _LL32:
 goto _LL34;case 13: _LL33: _LL34:
 goto _LL36;case 15: _LL35: _LL36:
# 295
{void**_tmp3F0;*_tmp72=((_tmp3F0=_cycalloc(sizeof(*_tmp3F0)),((_tmp3F0[0]=(void*)_tmp74->hd,_tmp3F0))));}
return(void*)_tmp74->hd;default: _LL37: _LL38:
# 298
 goto _LL2C;}_LL2C:;}
# 301
for(0;_tmp73 != 0;_tmp73=_tmp73->tl){
void*_tmp7A=(void*)_tmp73->hd;void*_tmp7B=_tmp7A;switch(*((int*)_tmp7B)){case 1: _LL3A: _LL3B:
 goto _LL3D;case 8: _LL3C: _LL3D:
 goto _LL3F;case 5: _LL3E: _LL3F:
 goto _LL41;case 13: _LL40: _LL41:
 goto _LL43;case 15: _LL42: _LL43:
# 308
{void**_tmp3F1;*_tmp72=((_tmp3F1=_cycalloc(sizeof(*_tmp3F1)),((_tmp3F1[0]=(void*)_tmp73->hd,_tmp3F1))));}
return(void*)_tmp73->hd;default: _LL44: _LL45:
# 311
 goto _LL39;}_LL39:;}
# 314
return t;}case 14: _LL28: _tmp71=((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmp70)->f2;_LL29:
# 317
 if((unsigned int)_tmp71)return Cyc_Port_compress_ct(*_tmp71);else{
return t;}default: _LL2A: _LL2B:
# 321
 return t;}_LL25:;}
# 327
static void*Cyc_Port_inst(struct Cyc_Dict_Dict*instenv,void*t){
void*_tmp7D=Cyc_Port_compress_ct(t);void*_tmp7E=_tmp7D;void*_tmp7F;struct Cyc_List_List*_tmp80;void*_tmp81;void*_tmp82;void*_tmp83;void*_tmp84;void*_tmp85;void*_tmp86;void*_tmp87;void*_tmp88;struct _dyneither_ptr*_tmp89;switch(*((int*)_tmp7E)){case 0: _LL47: _LL48:
 goto _LL4A;case 1: _LL49: _LL4A:
 goto _LL4C;case 2: _LL4B: _LL4C:
 goto _LL4E;case 3: _LL4D: _LL4E:
 goto _LL50;case 4: _LL4F: _LL50:
 goto _LL52;case 5: _LL51: _LL52:
 goto _LL54;case 6: _LL53: _LL54:
 goto _LL56;case 8: _LL55: _LL56:
 goto _LL58;case 9: _LL57: _LL58:
 goto _LL5A;case 14: _LL59: _LL5A:
 goto _LL5C;case 13: _LL5B: _LL5C:
 goto _LL5E;case 16: _LL5D: _LL5E:
 goto _LL60;case 7: _LL5F: _LL60:
 return t;case 10: _LL61: _tmp89=((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp7E)->f1;_LL62:
# 343
 if(!((int(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_member)(*instenv,_tmp89))
*instenv=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k,void*v))Cyc_Dict_insert)(*instenv,_tmp89,Cyc_Port_var());
return((void*(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup)(*instenv,_tmp89);case 11: _LL63: _tmp84=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp7E)->f1;_tmp85=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp7E)->f2;_tmp86=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp7E)->f3;_tmp87=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp7E)->f4;_tmp88=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp7E)->f5;_LL64: {
# 347
struct _tuple10 _tmp3F2;struct _tuple10 _tmp8A=(_tmp3F2.f1=Cyc_Port_inst(instenv,_tmp84),((_tmp3F2.f2=Cyc_Port_inst(instenv,_tmp87),_tmp3F2)));void*_tmp8C;void*_tmp8D;struct _tuple10 _tmp8B=_tmp8A;_tmp8C=_tmp8B.f1;_tmp8D=_tmp8B.f2;
if(_tmp8C == _tmp84  && _tmp8D == _tmp87)return t;{
struct Cyc_Port_Ptr_ct_Port_Ctype_struct _tmp3F5;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp3F4;return(void*)((_tmp3F4=_cycalloc(sizeof(*_tmp3F4)),((_tmp3F4[0]=((_tmp3F5.tag=11,((_tmp3F5.f1=_tmp8C,((_tmp3F5.f2=_tmp85,((_tmp3F5.f3=_tmp86,((_tmp3F5.f4=_tmp8D,((_tmp3F5.f5=_tmp88,_tmp3F5)))))))))))),_tmp3F4))));};}case 12: _LL65: _tmp81=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp7E)->f1;_tmp82=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp7E)->f2;_tmp83=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp7E)->f3;_LL66: {
# 351
void*_tmp91=Cyc_Port_inst(instenv,_tmp81);
if(_tmp91 == _tmp81)return t;{
struct Cyc_Port_Array_ct_Port_Ctype_struct _tmp3F8;struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp3F7;return(void*)((_tmp3F7=_cycalloc(sizeof(*_tmp3F7)),((_tmp3F7[0]=((_tmp3F8.tag=12,((_tmp3F8.f1=_tmp91,((_tmp3F8.f2=_tmp82,((_tmp3F8.f3=_tmp83,_tmp3F8)))))))),_tmp3F7))));};}default: _LL67: _tmp7F=(void*)((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp7E)->f1;_tmp80=((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp7E)->f2;_LL68: {
# 355
struct Cyc_Port_Fn_ct_Port_Ctype_struct _tmp3FB;struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp3FA;return(void*)((_tmp3FA=_cycalloc(sizeof(*_tmp3FA)),((_tmp3FA[0]=((_tmp3FB.tag=15,((_tmp3FB.f1=Cyc_Port_inst(instenv,_tmp7F),((_tmp3FB.f2=((struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Dict_Dict*,void*),struct Cyc_Dict_Dict*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_inst,instenv,_tmp80),_tmp3FB)))))),_tmp3FA))));}}_LL46:;}
# 359
void*Cyc_Port_instantiate(void*t){
struct Cyc_Dict_Dict*_tmp3FC;return Cyc_Port_inst(((_tmp3FC=_cycalloc(sizeof(*_tmp3FC)),((_tmp3FC[0]=((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Dict_empty)(Cyc_strptrcmp),_tmp3FC)))),t);}
# 366
struct Cyc_List_List*Cyc_Port_filter_self(void*t,struct Cyc_List_List*ts){
int found=0;
{struct Cyc_List_List*_tmp97=ts;for(0;(unsigned int)_tmp97;_tmp97=_tmp97->tl){
void*_tmp98=Cyc_Port_compress_ct((void*)_tmp97->hd);
if(t == _tmp98)found=1;}}
# 372
if(!found)return ts;{
struct Cyc_List_List*res=0;
for(0;ts != 0;ts=ts->tl){
if(t == Cyc_Port_compress_ct((void*)ts->hd))continue;{
struct Cyc_List_List*_tmp3FD;res=((_tmp3FD=_cycalloc(sizeof(*_tmp3FD)),((_tmp3FD->hd=(void*)ts->hd,((_tmp3FD->tl=res,_tmp3FD))))));};}
# 378
return res;};}struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
# 383
void Cyc_Port_generalize(int is_rgn,void*t){
t=Cyc_Port_compress_ct(t);{
void*_tmp9A=t;void*_tmp9B;struct Cyc_List_List*_tmp9C;void*_tmp9D;void*_tmp9E;void*_tmp9F;void*_tmpA0;void*_tmpA1;void*_tmpA2;void*_tmpA3;void*_tmpA4;struct Cyc_Port_Cvar*_tmpA5;switch(*((int*)_tmp9A)){case 16: _LL6A: _tmpA5=((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp9A)->f1;_LL6B:
# 388
 _tmpA5->uppers=Cyc_Port_filter_self(t,_tmpA5->uppers);
_tmpA5->lowers=Cyc_Port_filter_self(t,_tmpA5->lowers);
if(is_rgn){
# 393
if(_tmpA5->uppers == 0  && _tmpA5->lowers == 0){
Cyc_Port_unifies(t,Cyc_Port_rgnvar_ct(Cyc_Port_new_region_var()));
return;}
# 398
if((unsigned int)_tmpA5->uppers){
Cyc_Port_unifies(t,(void*)((struct Cyc_List_List*)_check_null(_tmpA5->uppers))->hd);
Cyc_Port_generalize(1,t);}else{
# 402
Cyc_Port_unifies(t,(void*)((struct Cyc_List_List*)_check_null(_tmpA5->lowers))->hd);
Cyc_Port_generalize(1,t);}}
# 406
return;case 0: _LL6C: _LL6D:
 goto _LL6F;case 1: _LL6E: _LL6F:
 goto _LL71;case 2: _LL70: _LL71:
 goto _LL73;case 3: _LL72: _LL73:
 goto _LL75;case 4: _LL74: _LL75:
 goto _LL77;case 5: _LL76: _LL77:
 goto _LL79;case 6: _LL78: _LL79:
 goto _LL7B;case 14: _LL7A: _LL7B:
 goto _LL7D;case 13: _LL7C: _LL7D:
 goto _LL7F;case 10: _LL7E: _LL7F:
 goto _LL81;case 9: _LL80: _LL81:
 goto _LL83;case 8: _LL82: _LL83:
 goto _LL85;case 7: _LL84: _LL85:
 return;case 11: _LL86: _tmpA0=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp9A)->f1;_tmpA1=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp9A)->f2;_tmpA2=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp9A)->f3;_tmpA3=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp9A)->f4;_tmpA4=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp9A)->f5;_LL87:
# 423
 Cyc_Port_generalize(0,_tmpA0);Cyc_Port_generalize(1,_tmpA3);goto _LL69;case 12: _LL88: _tmp9D=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp9A)->f1;_tmp9E=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp9A)->f2;_tmp9F=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp9A)->f3;_LL89:
# 425
 Cyc_Port_generalize(0,_tmp9D);Cyc_Port_generalize(0,_tmp9E);goto _LL69;default: _LL8A: _tmp9B=(void*)((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp9A)->f1;_tmp9C=((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp9A)->f2;_LL8B:
# 427
 Cyc_Port_generalize(0,_tmp9B);((void(*)(void(*f)(int,void*),int env,struct Cyc_List_List*x))Cyc_List_iter_c)(Cyc_Port_generalize,0,_tmp9C);goto _LL69;}_LL69:;};}struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 433
static int Cyc_Port_occurs(void*v,void*t){
t=Cyc_Port_compress_ct(t);
if(t == v)return 1;{
void*_tmpA6=t;struct Cyc_List_List*_tmpA7;struct Cyc_List_List*_tmpA8;void*_tmpA9;struct Cyc_List_List*_tmpAA;void*_tmpAB;void*_tmpAC;void*_tmpAD;void*_tmpAE;void*_tmpAF;void*_tmpB0;void*_tmpB1;void*_tmpB2;switch(*((int*)_tmpA6)){case 11: _LL8D: _tmpAE=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA6)->f1;_tmpAF=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA6)->f2;_tmpB0=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA6)->f3;_tmpB1=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA6)->f4;_tmpB2=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA6)->f5;_LL8E:
# 438
 return(((Cyc_Port_occurs(v,_tmpAE) || Cyc_Port_occurs(v,_tmpAF)) || Cyc_Port_occurs(v,_tmpB0)) || Cyc_Port_occurs(v,_tmpB1)) || 
Cyc_Port_occurs(v,_tmpB2);case 12: _LL8F: _tmpAB=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpA6)->f1;_tmpAC=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpA6)->f2;_tmpAD=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpA6)->f3;_LL90:
# 441
 return(Cyc_Port_occurs(v,_tmpAB) || Cyc_Port_occurs(v,_tmpAC)) || Cyc_Port_occurs(v,_tmpAD);case 15: _LL91: _tmpA9=(void*)((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpA6)->f1;_tmpAA=((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpA6)->f2;_LL92:
# 443
 if(Cyc_Port_occurs(v,_tmpA9))return 1;
for(0;(unsigned int)_tmpAA;_tmpAA=_tmpAA->tl){
if(Cyc_Port_occurs(v,(void*)_tmpAA->hd))return 1;}
return 0;case 13: _LL93: _tmpA8=(((struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpA6)->f1)->f2;_LL94:
 return 0;case 14: _LL95: _tmpA7=((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpA6)->f1;_LL96:
# 449
 for(0;(unsigned int)_tmpA7;_tmpA7=_tmpA7->tl){
if(Cyc_Port_occurs(v,((struct Cyc_Port_Cfield*)_tmpA7->hd)->qual) || Cyc_Port_occurs(v,((struct Cyc_Port_Cfield*)_tmpA7->hd)->type))return 1;}
return 0;default: _LL97: _LL98:
 return 0;}_LL8C:;};}char Cyc_Port_Unify_ct[9]="Unify_ct";struct Cyc_Port_Unify_ct_exn_struct{char*tag;};
# 461
struct Cyc_Port_Unify_ct_exn_struct Cyc_Port_Unify_ct_val={Cyc_Port_Unify_ct};
# 463
static int Cyc_Port_leq(void*t1,void*t2);
static void Cyc_Port_unify_cts(struct Cyc_List_List*t1,struct Cyc_List_List*t2);
static struct Cyc_List_List*Cyc_Port_merge_fields(struct Cyc_List_List*fs1,struct Cyc_List_List*fs2,int allow_subset);struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
# 467
static void Cyc_Port_unify_ct(void*t1,void*t2){
t1=Cyc_Port_compress_ct(t1);
t2=Cyc_Port_compress_ct(t2);
if(t1 == t2)return;{
struct _tuple10 _tmp3FE;struct _tuple10 _tmpB4=(_tmp3FE.f1=t1,((_tmp3FE.f2=t2,_tmp3FE)));struct _tuple10 _tmpB5=_tmpB4;struct Cyc_List_List*_tmpB6;void***_tmpB7;struct Cyc_Absyn_Aggrdecl*_tmpB8;struct Cyc_List_List*_tmpB9;struct Cyc_List_List*_tmpBA;void***_tmpBB;struct Cyc_List_List*_tmpBC;void***_tmpBD;struct Cyc_Absyn_Aggrdecl*_tmpBE;struct Cyc_List_List*_tmpBF;struct Cyc_List_List*_tmpC0;void***_tmpC1;struct _tuple11*_tmpC2;struct _tuple11*_tmpC3;void*_tmpC4;struct Cyc_List_List*_tmpC5;void*_tmpC6;struct Cyc_List_List*_tmpC7;void*_tmpC8;void*_tmpC9;void*_tmpCA;void*_tmpCB;void*_tmpCC;void*_tmpCD;struct _dyneither_ptr _tmpCE;struct _dyneither_ptr _tmpCF;void*_tmpD0;void*_tmpD1;void*_tmpD2;void*_tmpD3;void*_tmpD4;void*_tmpD5;void*_tmpD6;void*_tmpD7;void*_tmpD8;void*_tmpD9;struct Cyc_Port_Cvar*_tmpDA;struct Cyc_Port_Cvar*_tmpDB;if(((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmpB5.f1)->tag == 16){_LL9A: _tmpDB=((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmpB5.f1)->f1;_LL9B:
# 473
 if(!Cyc_Port_occurs(t1,t2)){
# 476
{struct Cyc_List_List*_tmpDC=Cyc_Port_filter_self(t1,_tmpDB->uppers);for(0;(unsigned int)_tmpDC;_tmpDC=_tmpDC->tl){
if(!Cyc_Port_leq(t2,(void*)_tmpDC->hd))(int)_throw((void*)& Cyc_Port_Unify_ct_val);}}
{struct Cyc_List_List*_tmpDD=Cyc_Port_filter_self(t1,_tmpDB->lowers);for(0;(unsigned int)_tmpDD;_tmpDD=_tmpDD->tl){
if(!Cyc_Port_leq((void*)_tmpDD->hd,t2))(int)_throw((void*)& Cyc_Port_Unify_ct_val);}}
{void**_tmp3FF;_tmpDB->eq=((_tmp3FF=_cycalloc(sizeof(*_tmp3FF)),((_tmp3FF[0]=t2,_tmp3FF))));}
return;}else{
(int)_throw((void*)& Cyc_Port_Unify_ct_val);}}else{if(((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmpB5.f2)->tag == 16){_LL9C: _tmpDA=((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmpB5.f2)->f1;_LL9D:
 Cyc_Port_unify_ct(t2,t1);return;}else{switch(*((int*)_tmpB5.f1)){case 11: if(((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f2)->tag == 11){_LL9E: _tmpD0=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f1)->f1;_tmpD1=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f1)->f2;_tmpD2=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f1)->f3;_tmpD3=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f1)->f4;_tmpD4=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f1)->f5;_tmpD5=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f2)->f1;_tmpD6=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f2)->f2;_tmpD7=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f2)->f3;_tmpD8=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f2)->f4;_tmpD9=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpB5.f2)->f5;_LL9F:
# 485
 Cyc_Port_unify_ct(_tmpD0,_tmpD5);Cyc_Port_unify_ct(_tmpD1,_tmpD6);Cyc_Port_unify_ct(_tmpD2,_tmpD7);Cyc_Port_unify_ct(_tmpD3,_tmpD8);
Cyc_Port_unify_ct(_tmpD4,_tmpD9);
return;}else{goto _LLAE;}case 10: if(((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmpB5.f2)->tag == 10){_LLA0: _tmpCE=*((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmpB5.f1)->f1;_tmpCF=*((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmpB5.f2)->f1;_LLA1:
# 489
 if(Cyc_strcmp((struct _dyneither_ptr)_tmpCE,(struct _dyneither_ptr)_tmpCF)!= 0)(int)_throw((void*)& Cyc_Port_Unify_ct_val);
return;}else{goto _LLAE;}case 12: if(((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpB5.f2)->tag == 12){_LLA2: _tmpC8=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpB5.f1)->f1;_tmpC9=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpB5.f1)->f2;_tmpCA=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpB5.f1)->f3;_tmpCB=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpB5.f2)->f1;_tmpCC=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpB5.f2)->f2;_tmpCD=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpB5.f2)->f3;_LLA3:
# 492
 Cyc_Port_unify_ct(_tmpC8,_tmpCB);Cyc_Port_unify_ct(_tmpC9,_tmpCC);Cyc_Port_unify_ct(_tmpCA,_tmpCD);return;}else{goto _LLAE;}case 15: if(((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpB5.f2)->tag == 15){_LLA4: _tmpC4=(void*)((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpB5.f1)->f1;_tmpC5=((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpB5.f1)->f2;_tmpC6=(void*)((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpB5.f2)->f1;_tmpC7=((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpB5.f2)->f2;_LLA5:
# 494
 Cyc_Port_unify_ct(_tmpC4,_tmpC6);Cyc_Port_unify_cts(_tmpC5,_tmpC7);return;}else{goto _LLAE;}case 13: switch(*((int*)_tmpB5.f2)){case 13: _LLA6: _tmpC2=((struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpB5.f1)->f1;_tmpC3=((struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpB5.f2)->f1;_LLA7:
# 496
 if(_tmpC2 == _tmpC3)return;else{(int)_throw((void*)& Cyc_Port_Unify_ct_val);}case 14: _LLAC: _tmpBE=(((struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpB5.f1)->f1)->f1;_tmpBF=(((struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpB5.f1)->f1)->f2;_tmpC0=((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpB5.f2)->f1;_tmpC1=(void***)&((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpB5.f2)->f2;_LLAD:
# 506
 Cyc_Port_merge_fields(_tmpBF,_tmpC0,0);
{void**_tmp400;*_tmpC1=((_tmp400=_cycalloc(sizeof(*_tmp400)),((_tmp400[0]=t1,_tmp400))));}
return;default: goto _LLAE;}case 14: switch(*((int*)_tmpB5.f2)){case 14: _LLA8: _tmpBA=((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpB5.f1)->f1;_tmpBB=(void***)&((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpB5.f1)->f2;_tmpBC=((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpB5.f2)->f1;_tmpBD=(void***)&((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpB5.f2)->f2;_LLA9: {
# 498
void*_tmpDF=Cyc_Port_unknown_aggr_ct(Cyc_Port_merge_fields(_tmpBA,_tmpBC,1));
{void**_tmp401;*_tmpBB=(*_tmpBD=((_tmp401=_cycalloc(sizeof(*_tmp401)),((_tmp401[0]=_tmpDF,_tmp401)))));}
return;}case 13: _LLAA: _tmpB6=((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpB5.f1)->f1;_tmpB7=(void***)&((struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpB5.f1)->f2;_tmpB8=(((struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpB5.f2)->f1)->f1;_tmpB9=(((struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpB5.f2)->f1)->f2;_LLAB:
# 502
 Cyc_Port_merge_fields(_tmpB9,_tmpB6,0);
{void**_tmp402;*_tmpB7=((_tmp402=_cycalloc(sizeof(*_tmp402)),((_tmp402[0]=t2,_tmp402))));}
return;default: goto _LLAE;}default: _LLAE: _LLAF:
# 509
(int)_throw((void*)& Cyc_Port_Unify_ct_val);}}}_LL99:;};}
# 513
static void Cyc_Port_unify_cts(struct Cyc_List_List*t1,struct Cyc_List_List*t2){
for(0;t1 != 0  && t2 != 0;(t1=t1->tl,t2=t2->tl)){
Cyc_Port_unify_ct((void*)t1->hd,(void*)t2->hd);}
# 517
if(t1 != 0  || t2 != 0)
(int)_throw((void*)& Cyc_Port_Unify_ct_val);}struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 523
static struct Cyc_List_List*Cyc_Port_merge_fields(struct Cyc_List_List*fs1,struct Cyc_List_List*fs2,int allow_f1_subset_f2){
# 525
struct Cyc_List_List*common=0;
{struct Cyc_List_List*_tmpE4=fs2;for(0;(unsigned int)_tmpE4;_tmpE4=_tmpE4->tl){
struct Cyc_Port_Cfield*_tmpE5=(struct Cyc_Port_Cfield*)_tmpE4->hd;
int found=0;
{struct Cyc_List_List*_tmpE6=fs1;for(0;(unsigned int)_tmpE6;_tmpE6=_tmpE6->tl){
struct Cyc_Port_Cfield*_tmpE7=(struct Cyc_Port_Cfield*)_tmpE6->hd;
if(Cyc_strptrcmp(_tmpE7->f,_tmpE5->f)== 0){
{struct Cyc_List_List*_tmp403;common=((_tmp403=_cycalloc(sizeof(*_tmp403)),((_tmp403->hd=_tmpE7,((_tmp403->tl=common,_tmp403))))));}
Cyc_Port_unify_ct(_tmpE7->qual,_tmpE5->qual);
Cyc_Port_unify_ct(_tmpE7->type,_tmpE5->type);
found=1;
break;}}}
# 539
if(!found){
if(allow_f1_subset_f2){
struct Cyc_List_List*_tmp404;common=((_tmp404=_cycalloc(sizeof(*_tmp404)),((_tmp404->hd=_tmpE5,((_tmp404->tl=common,_tmp404))))));}else{
(int)_throw((void*)& Cyc_Port_Unify_ct_val);}}}}
# 545
{struct Cyc_List_List*_tmpEA=fs1;for(0;(unsigned int)_tmpEA;_tmpEA=_tmpEA->tl){
struct Cyc_Port_Cfield*_tmpEB=(struct Cyc_Port_Cfield*)_tmpEA->hd;
int found=0;
{struct Cyc_List_List*_tmpEC=fs2;for(0;(unsigned int)_tmpEC;_tmpEC=_tmpEC->tl){
struct Cyc_Port_Cfield*_tmpED=(struct Cyc_Port_Cfield*)_tmpEC->hd;
if(Cyc_strptrcmp(_tmpEB->f,_tmpED->f))found=1;}}
# 552
if(!found){
struct Cyc_List_List*_tmp405;common=((_tmp405=_cycalloc(sizeof(*_tmp405)),((_tmp405->hd=_tmpEB,((_tmp405->tl=common,_tmp405))))));}}}
# 555
return common;}
# 558
static int Cyc_Port_unifies(void*t1,void*t2){
{struct _handler_cons _tmpEF;_push_handler(& _tmpEF);{int _tmpF1=0;if(setjmp(_tmpEF.handler))_tmpF1=1;if(!_tmpF1){
# 565
Cyc_Port_unify_ct(t1,t2);;_pop_handler();}else{void*_tmpF0=(void*)_exn_thrown;void*_tmpF2=_tmpF0;void*_tmpF3;if(((struct Cyc_Port_Unify_ct_exn_struct*)_tmpF2)->tag == Cyc_Port_Unify_ct){_LLB1: _LLB2:
# 572
 return 0;}else{_LLB3: _tmpF3=_tmpF2;_LLB4:(int)_rethrow(_tmpF3);}_LLB0:;}};}
# 574
return 1;}struct _tuple12{void*f1;void*f2;void*f3;void*f4;void*f5;};
# 580
static struct Cyc_List_List*Cyc_Port_insert_upper(void*v,void*t,struct Cyc_List_List**uppers){
# 582
t=Cyc_Port_compress_ct(t);
{void*_tmpF4=t;switch(*((int*)_tmpF4)){case 1: _LLB6: _LLB7:
# 586
 goto _LLB9;case 8: _LLB8: _LLB9:
 goto _LLBB;case 5: _LLBA: _LLBB:
 goto _LLBD;case 2: _LLBC: _LLBD:
 goto _LLBF;case 3: _LLBE: _LLBF:
 goto _LLC1;case 12: _LLC0: _LLC1:
 goto _LLC3;case 13: _LLC2: _LLC3:
 goto _LLC5;case 15: _LLC4: _LLC5:
 goto _LLC7;case 7: _LLC6: _LLC7:
# 597
*uppers=0;
Cyc_Port_unifies(v,t);
return*uppers;case 4: _LLC8: _LLC9:
# 602
 goto _LLCB;case 0: _LLCA: _LLCB:
 goto _LLCD;case 9: _LLCC: _LLCD:
# 605
 return*uppers;default: _LLCE: _LLCF:
 goto _LLB5;}_LLB5:;}
# 609
{struct Cyc_List_List*_tmpF5=*uppers;for(0;(unsigned int)_tmpF5;_tmpF5=_tmpF5->tl){
void*_tmpF6=Cyc_Port_compress_ct((void*)_tmpF5->hd);
# 612
if(t == _tmpF6)return*uppers;{
struct _tuple10 _tmp406;struct _tuple10 _tmpF7=(_tmp406.f1=t,((_tmp406.f2=_tmpF6,_tmp406)));struct _tuple10 _tmpF8=_tmpF7;void*_tmpF9;void*_tmpFA;void*_tmpFB;void*_tmpFC;void*_tmpFD;void*_tmpFE;void*_tmpFF;void*_tmp100;void*_tmp101;void*_tmp102;switch(*((int*)_tmpF8.f1)){case 6: switch(*((int*)_tmpF8.f2)){case 11: _LLD1: _LLD2:
# 617
 goto _LLD4;case 5: _LLD3: _LLD4:
 goto _LLD6;case 12: _LLD5: _LLD6:
# 620
 return*uppers;default: goto _LLD9;}case 11: if(((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f2)->tag == 11){_LLD7: _tmpF9=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f1)->f1;_tmpFA=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f1)->f2;_tmpFB=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f1)->f3;_tmpFC=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f1)->f4;_tmpFD=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f1)->f5;_tmpFE=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f2)->f1;_tmpFF=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f2)->f2;_tmp100=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f2)->f3;_tmp101=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f2)->f4;_tmp102=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF8.f2)->f5;_LLD8: {
# 625
struct _tuple12 _tmp407;struct _tuple12 _tmp103=(_tmp407.f1=Cyc_Port_var(),((_tmp407.f2=Cyc_Port_var(),((_tmp407.f3=Cyc_Port_var(),((_tmp407.f4=Cyc_Port_var(),((_tmp407.f5=Cyc_Port_var(),_tmp407)))))))));void*_tmp105;void*_tmp106;void*_tmp107;void*_tmp108;void*_tmp109;struct _tuple12 _tmp104=_tmp103;_tmp105=_tmp104.f1;_tmp106=_tmp104.f2;_tmp107=_tmp104.f3;_tmp108=_tmp104.f4;_tmp109=_tmp104.f5;{
struct Cyc_Port_Ptr_ct_Port_Ctype_struct _tmp40A;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp409;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp10A=(_tmp409=_cycalloc(sizeof(*_tmp409)),((_tmp409[0]=((_tmp40A.tag=11,((_tmp40A.f1=_tmp105,((_tmp40A.f2=_tmp106,((_tmp40A.f3=_tmp107,((_tmp40A.f4=_tmp108,((_tmp40A.f5=_tmp109,_tmp40A)))))))))))),_tmp409)));
Cyc_Port_leq(_tmp105,_tmpF9);Cyc_Port_leq(_tmp105,_tmpFE);
Cyc_Port_leq(_tmp106,_tmpFA);Cyc_Port_leq(_tmp106,_tmpFF);
Cyc_Port_leq(_tmp107,_tmpFB);Cyc_Port_leq(_tmp107,_tmpFF);
Cyc_Port_leq(_tmp108,_tmpFC);Cyc_Port_leq(_tmp108,_tmp101);
Cyc_Port_leq(_tmp109,_tmpFD);Cyc_Port_leq(_tmp109,_tmp102);
_tmpF5->hd=(void*)((void*)_tmp10A);
return*uppers;};}}else{goto _LLD9;}default: _LLD9: _LLDA:
 goto _LLD0;}_LLD0:;};}}{
# 637
struct Cyc_List_List*_tmp40B;return(_tmp40B=_cycalloc(sizeof(*_tmp40B)),((_tmp40B->hd=t,((_tmp40B->tl=*uppers,_tmp40B)))));};}
# 642
static struct Cyc_List_List*Cyc_Port_insert_lower(void*v,void*t,struct Cyc_List_List**lowers){
# 644
t=Cyc_Port_compress_ct(t);
{void*_tmp110=t;switch(*((int*)_tmp110)){case 0: _LLDC: _LLDD:
 goto _LLDF;case 8: _LLDE: _LLDF:
 goto _LLE1;case 2: _LLE0: _LLE1:
 goto _LLE3;case 3: _LLE2: _LLE3:
 goto _LLE5;case 4: _LLE4: _LLE5:
 goto _LLE7;case 13: _LLE6: _LLE7:
 goto _LLE9;case 15: _LLE8: _LLE9:
 goto _LLEB;case 10: _LLEA: _LLEB:
# 654
*lowers=0;
Cyc_Port_unifies(v,t);
return*lowers;case 7: _LLEC: _LLED:
 goto _LLEF;case 1: _LLEE: _LLEF:
 goto _LLF1;case 9: _LLF0: _LLF1:
# 660
 return*lowers;default: _LLF2: _LLF3:
# 662
 goto _LLDB;}_LLDB:;}
# 664
{struct Cyc_List_List*_tmp111=*lowers;for(0;(unsigned int)_tmp111;_tmp111=_tmp111->tl){
void*_tmp112=Cyc_Port_compress_ct((void*)_tmp111->hd);
if(t == _tmp112)return*lowers;{
struct _tuple10 _tmp40C;struct _tuple10 _tmp113=(_tmp40C.f1=t,((_tmp40C.f2=_tmp112,_tmp40C)));struct _tuple10 _tmp114=_tmp113;void*_tmp115;void*_tmp116;void*_tmp117;void*_tmp118;void*_tmp119;void*_tmp11A;void*_tmp11B;void*_tmp11C;void*_tmp11D;void*_tmp11E;if(((struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp114.f2)->tag == 4){_LLF5: _LLF6:
 goto _LLF8;}else{switch(*((int*)_tmp114.f1)){case 5: switch(*((int*)_tmp114.f2)){case 6: _LLF7: _LLF8:
 goto _LLFA;case 11: _LLF9: _LLFA:
 goto _LLFC;default: goto _LL101;}case 11: switch(*((int*)_tmp114.f2)){case 6: _LLFB: _LLFC:
 goto _LLFE;case 11: _LLFF: _tmp115=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f1)->f1;_tmp116=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f1)->f2;_tmp117=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f1)->f3;_tmp118=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f1)->f4;_tmp119=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f1)->f5;_tmp11A=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f2)->f1;_tmp11B=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f2)->f2;_tmp11C=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f2)->f3;_tmp11D=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f2)->f4;_tmp11E=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp114.f2)->f5;_LL100: {
# 678
struct _tuple12 _tmp40D;struct _tuple12 _tmp11F=(_tmp40D.f1=Cyc_Port_var(),((_tmp40D.f2=Cyc_Port_var(),((_tmp40D.f3=Cyc_Port_var(),((_tmp40D.f4=Cyc_Port_var(),((_tmp40D.f5=Cyc_Port_var(),_tmp40D)))))))));void*_tmp121;void*_tmp122;void*_tmp123;void*_tmp124;void*_tmp125;struct _tuple12 _tmp120=_tmp11F;_tmp121=_tmp120.f1;_tmp122=_tmp120.f2;_tmp123=_tmp120.f3;_tmp124=_tmp120.f4;_tmp125=_tmp120.f5;{
struct Cyc_Port_Ptr_ct_Port_Ctype_struct _tmp410;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp40F;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp126=(_tmp40F=_cycalloc(sizeof(*_tmp40F)),((_tmp40F[0]=((_tmp410.tag=11,((_tmp410.f1=_tmp121,((_tmp410.f2=_tmp122,((_tmp410.f3=_tmp123,((_tmp410.f4=_tmp124,((_tmp410.f5=_tmp125,_tmp410)))))))))))),_tmp40F)));
Cyc_Port_leq(_tmp115,_tmp121);Cyc_Port_leq(_tmp11A,_tmp121);
Cyc_Port_leq(_tmp116,_tmp122);Cyc_Port_leq(_tmp11B,_tmp122);
Cyc_Port_leq(_tmp117,_tmp123);Cyc_Port_leq(_tmp11B,_tmp123);
Cyc_Port_leq(_tmp118,_tmp124);Cyc_Port_leq(_tmp11D,_tmp124);
Cyc_Port_leq(_tmp119,_tmp125);Cyc_Port_leq(_tmp11E,_tmp125);
_tmp111->hd=(void*)((void*)_tmp126);
return*lowers;};}default: goto _LL101;}case 12: if(((struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp114.f2)->tag == 6){_LLFD: _LLFE:
# 673
 return*lowers;}else{goto _LL101;}default: _LL101: _LL102:
# 687
 goto _LLF4;}}_LLF4:;};}}{
# 690
struct Cyc_List_List*_tmp411;return(_tmp411=_cycalloc(sizeof(*_tmp411)),((_tmp411->hd=t,((_tmp411->tl=*lowers,_tmp411)))));};}struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
# 697
static int Cyc_Port_leq(void*t1,void*t2){
# 703
if(t1 == t2)return 1;
t1=Cyc_Port_compress_ct(t1);
t2=Cyc_Port_compress_ct(t2);{
struct _tuple10 _tmp412;struct _tuple10 _tmp12C=(_tmp412.f1=t1,((_tmp412.f2=t2,_tmp412)));struct _tuple10 _tmp12D=_tmp12C;struct Cyc_Port_Cvar*_tmp12E;void*_tmp12F;void*_tmp130;void*_tmp131;void*_tmp132;void*_tmp133;void*_tmp134;void*_tmp135;void*_tmp136;void*_tmp137;void*_tmp138;void*_tmp139;void*_tmp13A;void*_tmp13B;void*_tmp13C;void*_tmp13D;void*_tmp13E;void*_tmp13F;void*_tmp140;void*_tmp141;void*_tmp142;void*_tmp143;void*_tmp144;void*_tmp145;struct Cyc_Port_Cvar*_tmp146;struct Cyc_Port_Cvar*_tmp147;struct Cyc_Port_Cvar*_tmp148;struct _dyneither_ptr _tmp149;struct _dyneither_ptr _tmp14A;struct _dyneither_ptr _tmp14B;switch(*((int*)_tmp12D.f1)){case 7: _LL104: _LL105:
 return 1;case 10: switch(*((int*)_tmp12D.f2)){case 10: _LL106: _tmp14A=*((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp12D.f1)->f1;_tmp14B=*((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp12D.f2)->f1;_LL107:
 return Cyc_strcmp((struct _dyneither_ptr)_tmp14A,(struct _dyneither_ptr)_tmp14B)== 0;case 7: _LL108: _tmp149=*((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp12D.f1)->f1;_LL109:
 return 0;case 16: goto _LL130;default: goto _LL132;}case 1: switch(*((int*)_tmp12D.f2)){case 0: _LL10A: _LL10B:
 return 1;case 16: goto _LL130;default: goto _LL132;}case 0: switch(*((int*)_tmp12D.f2)){case 1: _LL10C: _LL10D:
 return 0;case 16: goto _LL130;default: goto _LL132;}case 9: switch(*((int*)_tmp12D.f2)){case 8: _LL10E: _LL10F:
 return 0;case 16: goto _LL130;default: goto _LL132;}case 8: switch(*((int*)_tmp12D.f2)){case 9: _LL110: _LL111:
 return 1;case 16: goto _LL130;default: goto _LL132;}case 16: switch(*((int*)_tmp12D.f2)){case 0: _LL112: _LL113:
 return 1;case 4: _LL114: _LL115:
 return 1;case 16: _LL12C: _tmp147=((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp12D.f1)->f1;_tmp148=((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp12D.f2)->f1;_LL12D:
# 733
 _tmp147->uppers=Cyc_Port_filter_self(t1,_tmp147->uppers);
_tmp148->lowers=Cyc_Port_filter_self(t2,_tmp148->lowers);
_tmp147->uppers=Cyc_Port_insert_upper(t1,t2,& _tmp147->uppers);
_tmp148->lowers=Cyc_Port_insert_lower(t2,t1,& _tmp148->lowers);
return 1;default: _LL12E: _tmp146=((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp12D.f1)->f1;_LL12F:
# 739
 _tmp146->uppers=Cyc_Port_filter_self(t1,_tmp146->uppers);
_tmp146->uppers=Cyc_Port_insert_upper(t1,t2,& _tmp146->uppers);
return 1;}case 4: _LL116: _LL117:
# 716
 return 0;case 5: switch(*((int*)_tmp12D.f2)){case 6: _LL118: _LL119:
 return 1;case 11: _LL11A: _LL11B:
 return 1;case 4: _LL11C: _LL11D:
 return 1;case 16: goto _LL130;default: goto _LL132;}case 11: switch(*((int*)_tmp12D.f2)){case 6: _LL11E: _LL11F:
 return 1;case 4: _LL120: _LL121:
 return 1;case 11: _LL126: _tmp13C=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f1)->f1;_tmp13D=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f1)->f2;_tmp13E=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f1)->f3;_tmp13F=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f1)->f4;_tmp140=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f1)->f5;_tmp141=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f2)->f1;_tmp142=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f2)->f2;_tmp143=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f2)->f3;_tmp144=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f2)->f4;_tmp145=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f2)->f5;_LL127:
# 725
 return(((Cyc_Port_leq(_tmp13C,_tmp141) && Cyc_Port_leq(_tmp13D,_tmp142)) && Cyc_Port_unifies(_tmp13E,_tmp143)) && Cyc_Port_leq(_tmp13F,_tmp144)) && 
Cyc_Port_leq(_tmp140,_tmp145);case 16: goto _LL130;default: goto _LL132;}case 12: switch(*((int*)_tmp12D.f2)){case 6: _LL122: _LL123:
# 722
 return 1;case 4: _LL124: _LL125:
 return 1;case 12: _LL128: _tmp136=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp12D.f1)->f1;_tmp137=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp12D.f1)->f2;_tmp138=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp12D.f1)->f3;_tmp139=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp12D.f2)->f1;_tmp13A=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp12D.f2)->f2;_tmp13B=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp12D.f2)->f3;_LL129:
# 728
 return(Cyc_Port_leq(_tmp136,_tmp139) && Cyc_Port_leq(_tmp137,_tmp13A)) && Cyc_Port_leq(_tmp138,_tmp13B);case 11: _LL12A: _tmp12F=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp12D.f1)->f1;_tmp130=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp12D.f1)->f2;_tmp131=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp12D.f1)->f3;_tmp132=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f2)->f1;_tmp133=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f2)->f2;_tmp134=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f2)->f3;_tmp135=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp12D.f2)->f5;_LL12B:
# 730
 return((Cyc_Port_leq(_tmp12F,_tmp132) && Cyc_Port_leq(_tmp130,_tmp133)) && Cyc_Port_unifies((void*)& Cyc_Port_Fat_ct_val,_tmp134)) && 
Cyc_Port_leq(_tmp131,_tmp135);case 16: goto _LL130;default: goto _LL132;}default: if(((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp12D.f2)->tag == 16){_LL130: _tmp12E=((struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp12D.f2)->f1;_LL131:
# 743
 _tmp12E->lowers=Cyc_Port_filter_self(t2,_tmp12E->lowers);
_tmp12E->lowers=Cyc_Port_insert_lower(t2,t1,& _tmp12E->lowers);
return 1;}else{_LL132: _LL133:
 return Cyc_Port_unifies(t1,t2);}}_LL103:;};}struct Cyc_Port_GlobalCenv{struct Cyc_Dict_Dict typedef_dict;struct Cyc_Dict_Dict struct_dict;struct Cyc_Dict_Dict union_dict;void*return_type;struct Cyc_List_List*qualifier_edits;struct Cyc_List_List*pointer_edits;struct Cyc_List_List*zeroterm_edits;struct Cyc_List_List*edits;int porting;};
# 752
typedef struct Cyc_Port_GlobalCenv*Cyc_Port_gcenv_t;
# 775
enum Cyc_Port_CPos{Cyc_Port_FnRes_pos  = 0,Cyc_Port_FnArg_pos  = 1,Cyc_Port_FnBody_pos  = 2,Cyc_Port_Toplevel_pos  = 3};
typedef enum Cyc_Port_CPos Cyc_Port_cpos_t;struct Cyc_Port_GlobalCenv;struct Cyc_Port_Cenv{struct Cyc_Port_GlobalCenv*gcenv;struct Cyc_Dict_Dict var_dict;enum Cyc_Port_CPos cpos;};
# 778
typedef struct Cyc_Port_Cenv*Cyc_Port_cenv_t;struct Cyc_Port_Cenv;struct Cyc_Port_GlobalCenv;struct Cyc_Port_Cenv;struct Cyc_Port_GlobalCenv;
# 788
static struct Cyc_Port_Cenv*Cyc_Port_empty_cenv(){
struct Cyc_Port_GlobalCenv*_tmp413;struct Cyc_Port_GlobalCenv*g=(_tmp413=_cycalloc(sizeof(*_tmp413)),((_tmp413->typedef_dict=((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),((_tmp413->struct_dict=
((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),((_tmp413->union_dict=
((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),((_tmp413->qualifier_edits=0,((_tmp413->pointer_edits=0,((_tmp413->zeroterm_edits=0,((_tmp413->edits=0,((_tmp413->porting=0,((_tmp413->return_type=
# 797
Cyc_Port_void_ct(),_tmp413)))))))))))))))))));
struct Cyc_Port_Cenv*_tmp414;return(_tmp414=_cycalloc(sizeof(*_tmp414)),((_tmp414->gcenv=g,((_tmp414->cpos=Cyc_Port_Toplevel_pos,((_tmp414->var_dict=
# 800
((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),_tmp414)))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 806
static int Cyc_Port_in_fn_arg(struct Cyc_Port_Cenv*env){
return env->cpos == Cyc_Port_FnArg_pos;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 809
static int Cyc_Port_in_fn_res(struct Cyc_Port_Cenv*env){
return env->cpos == Cyc_Port_FnRes_pos;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 812
static int Cyc_Port_in_toplevel(struct Cyc_Port_Cenv*env){
return env->cpos == Cyc_Port_Toplevel_pos;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 815
static void*Cyc_Port_lookup_return_type(struct Cyc_Port_Cenv*env){
return(env->gcenv)->return_type;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 818
static void*Cyc_Port_lookup_typedef(struct Cyc_Port_Cenv*env,struct _tuple0*n){
struct _handler_cons _tmp14F;_push_handler(& _tmp14F);{int _tmp151=0;if(setjmp(_tmp14F.handler))_tmp151=1;if(!_tmp151){
{struct _tuple10 _tmp152=*((struct _tuple10*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)((env->gcenv)->typedef_dict,n);void*_tmp154;struct _tuple10 _tmp153=_tmp152;_tmp154=_tmp153.f1;{
void*_tmp155=_tmp154;_npop_handler(0);return _tmp155;};}
# 820
;_pop_handler();}else{void*_tmp150=(void*)_exn_thrown;void*_tmp156=_tmp150;void*_tmp157;if(((struct Cyc_Dict_Absent_exn_struct*)_tmp156)->tag == Cyc_Dict_Absent){_LL135: _LL136:
# 827
 return Cyc_Absyn_sint_typ;}else{_LL137: _tmp157=_tmp156;_LL138:(int)_rethrow(_tmp157);}_LL134:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 832
static void*Cyc_Port_lookup_typedef_ctype(struct Cyc_Port_Cenv*env,struct _tuple0*n){
struct _handler_cons _tmp158;_push_handler(& _tmp158);{int _tmp15A=0;if(setjmp(_tmp158.handler))_tmp15A=1;if(!_tmp15A){
{struct _tuple10 _tmp15B=*((struct _tuple10*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)((env->gcenv)->typedef_dict,n);void*_tmp15D;struct _tuple10 _tmp15C=_tmp15B;_tmp15D=_tmp15C.f2;{
void*_tmp15E=_tmp15D;_npop_handler(0);return _tmp15E;};}
# 834
;_pop_handler();}else{void*_tmp159=(void*)_exn_thrown;void*_tmp15F=_tmp159;void*_tmp160;if(((struct Cyc_Dict_Absent_exn_struct*)_tmp15F)->tag == Cyc_Dict_Absent){_LL13A: _LL13B:
# 841
 return Cyc_Port_var();}else{_LL13C: _tmp160=_tmp15F;_LL13D:(int)_rethrow(_tmp160);}_LL139:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 847
static struct _tuple11*Cyc_Port_lookup_struct_decl(struct Cyc_Port_Cenv*env,struct _tuple0*n){
# 849
struct _tuple11**_tmp161=((struct _tuple11**(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup_opt)((env->gcenv)->struct_dict,n);
if((unsigned int)_tmp161)
return*_tmp161;else{
# 853
struct Cyc_Absyn_Aggrdecl*_tmp415;struct Cyc_Absyn_Aggrdecl*_tmp162=(_tmp415=_cycalloc(sizeof(*_tmp415)),((_tmp415->kind=Cyc_Absyn_StructA,((_tmp415->sc=Cyc_Absyn_Public,((_tmp415->name=n,((_tmp415->tvs=0,((_tmp415->impl=0,((_tmp415->attributes=0,((_tmp415->expected_mem_kind=0,_tmp415)))))))))))))));
# 856
struct _tuple11*_tmp416;struct _tuple11*p=(_tmp416=_cycalloc(sizeof(*_tmp416)),((_tmp416->f1=_tmp162,((_tmp416->f2=0,_tmp416)))));
(env->gcenv)->struct_dict=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple11*v))Cyc_Dict_insert)((env->gcenv)->struct_dict,n,p);
return p;}}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 864
static struct _tuple11*Cyc_Port_lookup_union_decl(struct Cyc_Port_Cenv*env,struct _tuple0*n){
# 866
struct _tuple11**_tmp165=((struct _tuple11**(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup_opt)((env->gcenv)->union_dict,n);
if((unsigned int)_tmp165)
return*_tmp165;else{
# 870
struct Cyc_Absyn_Aggrdecl*_tmp417;struct Cyc_Absyn_Aggrdecl*_tmp166=(_tmp417=_cycalloc(sizeof(*_tmp417)),((_tmp417->kind=Cyc_Absyn_UnionA,((_tmp417->sc=Cyc_Absyn_Public,((_tmp417->name=n,((_tmp417->tvs=0,((_tmp417->impl=0,((_tmp417->attributes=0,((_tmp417->expected_mem_kind=0,_tmp417)))))))))))))));
# 873
struct _tuple11*_tmp418;struct _tuple11*p=(_tmp418=_cycalloc(sizeof(*_tmp418)),((_tmp418->f1=_tmp166,((_tmp418->f2=0,_tmp418)))));
(env->gcenv)->union_dict=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple11*v))Cyc_Dict_insert)((env->gcenv)->union_dict,n,p);
return p;}}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct _tuple13{void*f1;struct _tuple10*f2;};
# 880
static struct _tuple10 Cyc_Port_lookup_var(struct Cyc_Port_Cenv*env,struct _tuple0*x){
struct _handler_cons _tmp169;_push_handler(& _tmp169);{int _tmp16B=0;if(setjmp(_tmp169.handler))_tmp16B=1;if(!_tmp16B){
{struct _tuple13 _tmp16C=*((struct _tuple13*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);struct _tuple10*_tmp16E;struct _tuple13 _tmp16D=_tmp16C;_tmp16E=_tmp16D.f2;{
struct _tuple10 _tmp16F=*_tmp16E;_npop_handler(0);return _tmp16F;};}
# 882
;_pop_handler();}else{void*_tmp16A=(void*)_exn_thrown;void*_tmp170=_tmp16A;void*_tmp171;if(((struct Cyc_Dict_Absent_exn_struct*)_tmp170)->tag == Cyc_Dict_Absent){_LL13F: _LL140: {
# 889
struct _tuple10 _tmp419;return(_tmp419.f1=Cyc_Port_var(),((_tmp419.f2=Cyc_Port_var(),_tmp419)));}}else{_LL141: _tmp171=_tmp170;_LL142:(int)_rethrow(_tmp171);}_LL13E:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 892
static struct _tuple13*Cyc_Port_lookup_full_var(struct Cyc_Port_Cenv*env,struct _tuple0*x){
return((struct _tuple13*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 896
static int Cyc_Port_declared_var(struct Cyc_Port_Cenv*env,struct _tuple0*x){
return((int(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_member)(env->var_dict,x);}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 900
static int Cyc_Port_isfn(struct Cyc_Port_Cenv*env,struct _tuple0*x){
struct _handler_cons _tmp173;_push_handler(& _tmp173);{int _tmp175=0;if(setjmp(_tmp173.handler))_tmp175=1;if(!_tmp175){
{struct _tuple13 _tmp176=*((struct _tuple13*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);void*_tmp178;struct _tuple13 _tmp177=_tmp176;_tmp178=_tmp177.f1;
LOOP: {void*_tmp179=_tmp178;struct _tuple0*_tmp17A;switch(*((int*)_tmp179)){case 17: _LL144: _tmp17A=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp179)->f1;_LL145:
 _tmp178=Cyc_Port_lookup_typedef(env,_tmp17A);goto LOOP;case 9: _LL146: _LL147: {
int _tmp17B=1;_npop_handler(0);return _tmp17B;}default: _LL148: _LL149: {
int _tmp17C=0;_npop_handler(0);return _tmp17C;}}_LL143:;}}
# 902
;_pop_handler();}else{void*_tmp174=(void*)_exn_thrown;void*_tmp17D=_tmp174;void*_tmp17E;if(((struct Cyc_Dict_Absent_exn_struct*)_tmp17D)->tag == Cyc_Dict_Absent){_LL14B: _LL14C:
# 913
 return 0;}else{_LL14D: _tmp17E=_tmp17D;_LL14E:(int)_rethrow(_tmp17E);}_LL14A:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 917
static int Cyc_Port_isarray(struct Cyc_Port_Cenv*env,struct _tuple0*x){
struct _handler_cons _tmp17F;_push_handler(& _tmp17F);{int _tmp181=0;if(setjmp(_tmp17F.handler))_tmp181=1;if(!_tmp181){
{struct _tuple13 _tmp182=*((struct _tuple13*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);void*_tmp184;struct _tuple13 _tmp183=_tmp182;_tmp184=_tmp183.f1;
LOOP: {void*_tmp185=_tmp184;struct _tuple0*_tmp186;switch(*((int*)_tmp185)){case 17: _LL150: _tmp186=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp185)->f1;_LL151:
 _tmp184=Cyc_Port_lookup_typedef(env,_tmp186);goto LOOP;case 8: _LL152: _LL153: {
int _tmp187=1;_npop_handler(0);return _tmp187;}default: _LL154: _LL155: {
int _tmp188=0;_npop_handler(0);return _tmp188;}}_LL14F:;}}
# 919
;_pop_handler();}else{void*_tmp180=(void*)_exn_thrown;void*_tmp189=_tmp180;void*_tmp18A;if(((struct Cyc_Dict_Absent_exn_struct*)_tmp189)->tag == Cyc_Dict_Absent){_LL157: _LL158:
# 930
 return 0;}else{_LL159: _tmp18A=_tmp189;_LL15A:(int)_rethrow(_tmp18A);}_LL156:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 936
static struct Cyc_Port_Cenv*Cyc_Port_set_cpos(struct Cyc_Port_Cenv*env,enum Cyc_Port_CPos cpos){
struct Cyc_Port_Cenv*_tmp41A;return(_tmp41A=_cycalloc(sizeof(*_tmp41A)),((_tmp41A->gcenv=env->gcenv,((_tmp41A->var_dict=env->var_dict,((_tmp41A->cpos=cpos,_tmp41A)))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 941
static void Cyc_Port_add_return_type(struct Cyc_Port_Cenv*env,void*t){
(env->gcenv)->return_type=t;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 946
static struct Cyc_Port_Cenv*Cyc_Port_add_var(struct Cyc_Port_Cenv*env,struct _tuple0*x,void*t,void*qual,void*ctype){
# 948
struct _tuple13*_tmp420;struct _tuple10*_tmp41F;struct Cyc_Port_Cenv*_tmp41E;return(_tmp41E=_cycalloc(sizeof(*_tmp41E)),((_tmp41E->gcenv=env->gcenv,((_tmp41E->var_dict=
((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple13*v))Cyc_Dict_insert)(env->var_dict,x,((_tmp420=_cycalloc(sizeof(*_tmp420)),((_tmp420->f1=t,((_tmp420->f2=((_tmp41F=_cycalloc(sizeof(*_tmp41F)),((_tmp41F->f1=qual,((_tmp41F->f2=ctype,_tmp41F)))))),_tmp420))))))),((_tmp41E->cpos=env->cpos,_tmp41E)))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 954
static void Cyc_Port_add_typedef(struct Cyc_Port_Cenv*env,struct _tuple0*n,void*t,void*ct){
struct _tuple10*_tmp421;(env->gcenv)->typedef_dict=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple10*v))Cyc_Dict_insert)((env->gcenv)->typedef_dict,n,(
(_tmp421=_cycalloc(sizeof(*_tmp421)),((_tmp421->f1=t,((_tmp421->f2=ct,_tmp421)))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct _tuple14{void*f1;void*f2;unsigned int f3;};
# 961
static void Cyc_Port_register_const_cvar(struct Cyc_Port_Cenv*env,void*new_qual,void*orig_qual,unsigned int loc){
# 963
struct _tuple14*_tmp424;struct Cyc_List_List*_tmp423;(env->gcenv)->qualifier_edits=((_tmp423=_cycalloc(sizeof(*_tmp423)),((_tmp423->hd=((_tmp424=_cycalloc(sizeof(*_tmp424)),((_tmp424->f1=new_qual,((_tmp424->f2=orig_qual,((_tmp424->f3=loc,_tmp424)))))))),((_tmp423->tl=(env->gcenv)->qualifier_edits,_tmp423))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 969
static void Cyc_Port_register_ptr_cvars(struct Cyc_Port_Cenv*env,void*new_ptr,void*orig_ptr,unsigned int loc){
# 971
struct _tuple14*_tmp427;struct Cyc_List_List*_tmp426;(env->gcenv)->pointer_edits=((_tmp426=_cycalloc(sizeof(*_tmp426)),((_tmp426->hd=((_tmp427=_cycalloc(sizeof(*_tmp427)),((_tmp427->f1=new_ptr,((_tmp427->f2=orig_ptr,((_tmp427->f3=loc,_tmp427)))))))),((_tmp426->tl=(env->gcenv)->pointer_edits,_tmp426))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 975
static void Cyc_Port_register_zeroterm_cvars(struct Cyc_Port_Cenv*env,void*new_zt,void*orig_zt,unsigned int loc){
# 977
struct _tuple14*_tmp42A;struct Cyc_List_List*_tmp429;(env->gcenv)->zeroterm_edits=((_tmp429=_cycalloc(sizeof(*_tmp429)),((_tmp429->hd=((_tmp42A=_cycalloc(sizeof(*_tmp42A)),((_tmp42A->f1=new_zt,((_tmp42A->f2=orig_zt,((_tmp42A->f3=loc,_tmp42A)))))))),((_tmp429->tl=(env->gcenv)->zeroterm_edits,_tmp429))))));}
# 983
static void*Cyc_Port_main_type(){
struct _tuple8*_tmp42B;struct _tuple8*arg1=
(_tmp42B=_cycalloc(sizeof(*_tmp42B)),((_tmp42B->f1=0,((_tmp42B->f2=Cyc_Absyn_empty_tqual(0),((_tmp42B->f3=Cyc_Absyn_sint_typ,_tmp42B)))))));
struct _tuple8*_tmp42C;struct _tuple8*arg2=
(_tmp42C=_cycalloc(sizeof(*_tmp42C)),((_tmp42C->f1=0,((_tmp42C->f2=Cyc_Absyn_empty_tqual(0),((_tmp42C->f3=
Cyc_Absyn_dyneither_typ(Cyc_Absyn_string_typ(Cyc_Absyn_wildtyp(0)),Cyc_Absyn_wildtyp(0),
Cyc_Absyn_empty_tqual(0),((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)()),_tmp42C)))))));
struct Cyc_Absyn_FnType_Absyn_Type_struct _tmp436;struct _tuple8*_tmp435[2];struct Cyc_Absyn_FnInfo _tmp434;struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp433;return(void*)((_tmp433=_cycalloc(sizeof(*_tmp433)),((_tmp433[0]=((_tmp436.tag=9,((_tmp436.f1=((_tmp434.tvars=0,((_tmp434.effect=0,((_tmp434.ret_tqual=
Cyc_Absyn_empty_tqual(0),((_tmp434.ret_typ=Cyc_Absyn_sint_typ,((_tmp434.args=(
# 993
(_tmp435[1]=arg2,((_tmp435[0]=arg1,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp435,sizeof(struct _tuple8*),2)))))),((_tmp434.c_varargs=0,((_tmp434.cyc_varargs=0,((_tmp434.rgn_po=0,((_tmp434.attributes=0,((_tmp434.requires_clause=0,((_tmp434.requires_relns=0,((_tmp434.ensures_clause=0,((_tmp434.ensures_relns=0,_tmp434)))))))))))))))))))))))))),_tmp436)))),_tmp433))));}struct Cyc_Port_Cenv;
# 1002
static void*Cyc_Port_type_to_ctype(struct Cyc_Port_Cenv*env,void*t);struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1005
static struct Cyc_Port_Cenv*Cyc_Port_initial_cenv(){
struct Cyc_Port_Cenv*_tmp19C=Cyc_Port_empty_cenv();
# 1008
{struct _tuple0*_tmp437;_tmp19C=Cyc_Port_add_var(_tmp19C,((_tmp437=_cycalloc(sizeof(*_tmp437)),((_tmp437->f1=Cyc_Absyn_Loc_n,((_tmp437->f2=_init_dyneither_ptr(_cycalloc(sizeof(struct _dyneither_ptr)),"main",sizeof(char),5),_tmp437)))))),Cyc_Port_main_type(),Cyc_Port_const_ct(),
Cyc_Port_type_to_ctype(_tmp19C,Cyc_Port_main_type()));}
return _tmp19C;}
# 1016
static void Cyc_Port_region_counts(struct Cyc_Dict_Dict*cts,void*t){
void*_tmp19F=Cyc_Port_compress_ct(t);void*_tmp1A0=_tmp19F;void*_tmp1A1;struct Cyc_List_List*_tmp1A2;void*_tmp1A3;void*_tmp1A4;void*_tmp1A5;void*_tmp1A6;void*_tmp1A7;void*_tmp1A8;void*_tmp1A9;void*_tmp1AA;struct _dyneither_ptr*_tmp1AB;switch(*((int*)_tmp1A0)){case 0: _LL15C: _LL15D:
 goto _LL15F;case 1: _LL15E: _LL15F:
 goto _LL161;case 2: _LL160: _LL161:
 goto _LL163;case 3: _LL162: _LL163:
 goto _LL165;case 4: _LL164: _LL165:
 goto _LL167;case 5: _LL166: _LL167:
 goto _LL169;case 6: _LL168: _LL169:
 goto _LL16B;case 14: _LL16A: _LL16B:
 goto _LL16D;case 13: _LL16C: _LL16D:
 goto _LL16F;case 16: _LL16E: _LL16F:
 goto _LL171;case 8: _LL170: _LL171:
 goto _LL173;case 9: _LL172: _LL173:
 goto _LL175;case 7: _LL174: _LL175:
 return;case 10: _LL176: _tmp1AB=((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp1A0)->f1;_LL177:
# 1032
 if(!((int(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_member)(*cts,_tmp1AB)){
int*_tmp438;*cts=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k,int*v))Cyc_Dict_insert)(*cts,_tmp1AB,((_tmp438=_cycalloc_atomic(sizeof(*_tmp438)),((_tmp438[0]=0,_tmp438)))));}{
int*_tmp1AD=((int*(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup)(*cts,_tmp1AB);
*_tmp1AD=*_tmp1AD + 1;
return;};case 11: _LL178: _tmp1A6=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A0)->f1;_tmp1A7=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A0)->f2;_tmp1A8=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A0)->f3;_tmp1A9=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A0)->f4;_tmp1AA=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A0)->f5;_LL179:
# 1038
 Cyc_Port_region_counts(cts,_tmp1A6);
Cyc_Port_region_counts(cts,_tmp1A9);
return;case 12: _LL17A: _tmp1A3=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1A0)->f1;_tmp1A4=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1A0)->f2;_tmp1A5=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1A0)->f3;_LL17B:
# 1042
 Cyc_Port_region_counts(cts,_tmp1A3);
return;default: _LL17C: _tmp1A1=(void*)((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp1A0)->f1;_tmp1A2=((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp1A0)->f2;_LL17D:
# 1045
 Cyc_Port_region_counts(cts,_tmp1A1);
for(0;(unsigned int)_tmp1A2;_tmp1A2=_tmp1A2->tl){Cyc_Port_region_counts(cts,(void*)_tmp1A2->hd);}
return;}_LL15B:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 1056
static void Cyc_Port_register_rgns(struct Cyc_Port_Cenv*env,struct Cyc_Dict_Dict counts,int fn_res,void*t,void*c){
# 1058
c=Cyc_Port_compress_ct(c);{
struct _tuple10 _tmp439;struct _tuple10 _tmp1AE=(_tmp439.f1=t,((_tmp439.f2=c,_tmp439)));struct _tuple10 _tmp1AF=_tmp1AE;void*_tmp1B0;struct Cyc_List_List*_tmp1B1;void*_tmp1B2;struct Cyc_List_List*_tmp1B3;void*_tmp1B4;void*_tmp1B5;void*_tmp1B6;void*_tmp1B7;struct Cyc_Absyn_PtrLoc*_tmp1B8;void*_tmp1B9;void*_tmp1BA;switch(*((int*)_tmp1AF.f1)){case 5: if(((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1AF.f2)->tag == 11){_LL17F: _tmp1B6=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1AF.f1)->f1).elt_typ;_tmp1B7=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1AF.f1)->f1).ptr_atts).rgn;_tmp1B8=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1AF.f1)->f1).ptr_atts).ptrloc;_tmp1B9=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1AF.f2)->f1;_tmp1BA=(void*)((struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1AF.f2)->f4;_LL180:
# 1061
 Cyc_Port_register_rgns(env,counts,fn_res,_tmp1B6,_tmp1B9);{
unsigned int _tmp1BB=(unsigned int)_tmp1B8?_tmp1B8->rgn_loc:(unsigned int)0;
_tmp1BA=Cyc_Port_compress_ct(_tmp1BA);
{struct _tuple10 _tmp43A;struct _tuple10 _tmp1BC=(_tmp43A.f1=_tmp1B7,((_tmp43A.f2=_tmp1BA,_tmp43A)));struct _tuple10 _tmp1BD=_tmp1BC;struct _dyneither_ptr*_tmp1BE;if(((struct Cyc_Absyn_Evar_Absyn_Type_struct*)_tmp1BD.f1)->tag == 1)switch(*((int*)_tmp1BD.f2)){case 7: _LL188: if(!fn_res){_LL189:
# 1066
{struct Cyc_Port_Edit*_tmp443;const char*_tmp442;const char*_tmp441;struct Cyc_List_List*_tmp440;(env->gcenv)->edits=(
(_tmp440=_cycalloc(sizeof(*_tmp440)),((_tmp440->hd=((_tmp443=_cycalloc(sizeof(*_tmp443)),((_tmp443->loc=_tmp1BB,((_tmp443->old_string=((_tmp442="`H ",_tag_dyneither(_tmp442,sizeof(char),4))),((_tmp443->new_string=((_tmp441="",_tag_dyneither(_tmp441,sizeof(char),1))),_tmp443)))))))),((_tmp440->tl=(env->gcenv)->edits,_tmp440))))));}
goto _LL187;}else{goto _LL18C;}case 10: _LL18A: _tmp1BE=((struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp1BD.f2)->f1;_LL18B: {
# 1070
int _tmp1C3=*((int*(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup)(counts,_tmp1BE);
if(_tmp1C3 > 1){
struct Cyc_Port_Edit*_tmp453;const char*_tmp452;void*_tmp451[1];struct Cyc_String_pa_PrintArg_struct _tmp450;const char*_tmp44F;struct Cyc_List_List*_tmp44E;(env->gcenv)->edits=(
(_tmp44E=_cycalloc(sizeof(*_tmp44E)),((_tmp44E->hd=((_tmp453=_cycalloc(sizeof(*_tmp453)),((_tmp453->loc=_tmp1BB,((_tmp453->old_string=(struct _dyneither_ptr)((_tmp450.tag=0,((_tmp450.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp1BE),((_tmp451[0]=& _tmp450,Cyc_aprintf(((_tmp452="%s ",_tag_dyneither(_tmp452,sizeof(char),4))),_tag_dyneither(_tmp451,sizeof(void*),1)))))))),((_tmp453->new_string=((_tmp44F="",_tag_dyneither(_tmp44F,sizeof(char),1))),_tmp453)))))))),((_tmp44E->tl=(env->gcenv)->edits,_tmp44E))))));}
goto _LL187;}default: goto _LL18C;}else{_LL18C: _LL18D:
 goto _LL187;}_LL187:;}
# 1077
goto _LL17E;};}else{goto _LL185;}case 8: if(((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1AF.f2)->tag == 12){_LL181: _tmp1B4=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp1AF.f1)->f1).elt_type;_tmp1B5=(void*)((struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1AF.f2)->f1;_LL182:
# 1079
 Cyc_Port_register_rgns(env,counts,fn_res,_tmp1B4,_tmp1B5);
goto _LL17E;}else{goto _LL185;}case 9: if(((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp1AF.f2)->tag == 15){_LL183: _tmp1B0=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1AF.f1)->f1).ret_typ;_tmp1B1=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1AF.f1)->f1).args;_tmp1B2=(void*)((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp1AF.f2)->f1;_tmp1B3=((struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp1AF.f2)->f2;_LL184:
# 1082
 Cyc_Port_register_rgns(env,counts,1,_tmp1B0,_tmp1B2);
for(0;_tmp1B1 != 0  && _tmp1B3 != 0;(_tmp1B1=_tmp1B1->tl,_tmp1B3=_tmp1B3->tl)){
struct _tuple8 _tmp1CB=*((struct _tuple8*)_tmp1B1->hd);void*_tmp1CD;struct _tuple8 _tmp1CC=_tmp1CB;_tmp1CD=_tmp1CC.f3;{
void*_tmp1CE=(void*)_tmp1B3->hd;
Cyc_Port_register_rgns(env,counts,0,_tmp1CD,_tmp1CE);};}
# 1088
goto _LL17E;}else{goto _LL185;}default: _LL185: _LL186:
 goto _LL17E;}_LL17E:;};}struct Cyc_Port_Cenv;
# 1095
static void*Cyc_Port_gen_exp(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Exp*e);struct Cyc_Port_Cenv;
static void Cyc_Port_gen_stmt(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Stmt*s);struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
static struct Cyc_Port_Cenv*Cyc_Port_gen_localdecl(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Decl*d);struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1100
static int Cyc_Port_is_char(struct Cyc_Port_Cenv*env,void*t){
void*_tmp1D0=t;enum Cyc_Absyn_Size_of _tmp1D1;struct _tuple0*_tmp1D2;switch(*((int*)_tmp1D0)){case 17: _LL18F: _tmp1D2=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp1D0)->f1;_LL190:
# 1103
(*_tmp1D2).f1=Cyc_Absyn_Loc_n;
return Cyc_Port_is_char(env,Cyc_Port_lookup_typedef(env,_tmp1D2));case 6: _LL191: _tmp1D1=((struct Cyc_Absyn_IntType_Absyn_Type_struct*)_tmp1D0)->f2;_LL192:
 return 1;default: _LL193: _LL194:
 return 0;}_LL18E:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1111
static void*Cyc_Port_type_to_ctype(struct Cyc_Port_Cenv*env,void*t){
void*_tmp1D3=t;struct Cyc_List_List*_tmp1D4;union Cyc_Absyn_AggrInfoU _tmp1D5;void*_tmp1D6;struct Cyc_List_List*_tmp1D7;void*_tmp1D8;struct Cyc_Absyn_Tqual _tmp1D9;union Cyc_Absyn_Constraint*_tmp1DA;unsigned int _tmp1DB;void*_tmp1DC;struct Cyc_Absyn_Tqual _tmp1DD;void*_tmp1DE;union Cyc_Absyn_Constraint*_tmp1DF;union Cyc_Absyn_Constraint*_tmp1E0;union Cyc_Absyn_Constraint*_tmp1E1;struct Cyc_Absyn_PtrLoc*_tmp1E2;struct _tuple0*_tmp1E3;switch(*((int*)_tmp1D3)){case 17: _LL196: _tmp1E3=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp1D3)->f1;_LL197:
# 1114
(*_tmp1E3).f1=Cyc_Absyn_Loc_n;
return Cyc_Port_lookup_typedef_ctype(env,_tmp1E3);case 0: _LL198: _LL199:
 return Cyc_Port_void_ct();case 5: _LL19A: _tmp1DC=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D3)->f1).elt_typ;_tmp1DD=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D3)->f1).elt_tq;_tmp1DE=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D3)->f1).ptr_atts).rgn;_tmp1DF=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D3)->f1).ptr_atts).nullable;_tmp1E0=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D3)->f1).ptr_atts).bounds;_tmp1E1=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D3)->f1).ptr_atts).zero_term;_tmp1E2=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D3)->f1).ptr_atts).ptrloc;_LL19B: {
# 1118
unsigned int _tmp1E4=(unsigned int)_tmp1E2?_tmp1E2->ptr_loc:(unsigned int)0;
unsigned int _tmp1E5=(unsigned int)_tmp1E2?_tmp1E2->rgn_loc:(unsigned int)0;
unsigned int _tmp1E6=(unsigned int)_tmp1E2?_tmp1E2->zt_loc:(unsigned int)0;
# 1124
void*_tmp1E7=Cyc_Port_type_to_ctype(env,_tmp1DC);
void*new_rgn;
# 1127
{void*_tmp1E8=_tmp1DE;struct _dyneither_ptr*_tmp1E9;switch(*((int*)_tmp1E8)){case 20: _LL1AD: _LL1AE:
# 1129
 new_rgn=Cyc_Port_heap_ct();goto _LL1AC;case 2: _LL1AF: _tmp1E9=(((struct Cyc_Absyn_VarType_Absyn_Type_struct*)_tmp1E8)->f1)->name;_LL1B0:
# 1131
 new_rgn=Cyc_Port_rgnvar_ct(_tmp1E9);goto _LL1AC;case 1: _LL1B1: _LL1B2:
# 1135
 if(Cyc_Port_in_fn_arg(env))
new_rgn=Cyc_Port_rgnvar_ct(Cyc_Port_new_region_var());else{
# 1138
if(Cyc_Port_in_fn_res(env) || Cyc_Port_in_toplevel(env))
new_rgn=Cyc_Port_heap_ct();else{
new_rgn=Cyc_Port_var();}}
goto _LL1AC;default: _LL1B3: _LL1B4:
# 1143
 new_rgn=Cyc_Port_heap_ct();goto _LL1AC;}_LL1AC:;}{
# 1145
int _tmp1EA=((void*(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp1E0)== (void*)& Cyc_Absyn_DynEither_b_val;
int orig_zt;
{union Cyc_Absyn_Constraint _tmp1EB=*_tmp1E1;union Cyc_Absyn_Constraint _tmp1EC=_tmp1EB;if((_tmp1EC.No_constr).tag == 3){_LL1B6: _LL1B7:
 orig_zt=Cyc_Port_is_char(env,_tmp1DC);goto _LL1B5;}else{_LL1B8: _LL1B9:
 orig_zt=((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp1E1);goto _LL1B5;}_LL1B5:;}
# 1151
if((env->gcenv)->porting){
void*_tmp1ED=Cyc_Port_var();
# 1156
Cyc_Port_register_const_cvar(env,_tmp1ED,_tmp1DD.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),_tmp1DD.loc);
# 1159
if(_tmp1DD.print_const)Cyc_Port_unify_ct(_tmp1ED,Cyc_Port_const_ct());{
# 1165
void*_tmp1EE=Cyc_Port_var();
# 1168
Cyc_Port_register_ptr_cvars(env,_tmp1EE,_tmp1EA?Cyc_Port_fat_ct(): Cyc_Port_thin_ct(),_tmp1E4);
# 1170
if(_tmp1EA)Cyc_Port_unify_ct(_tmp1EE,Cyc_Port_fat_ct());{
void*_tmp1EF=Cyc_Port_var();
# 1174
Cyc_Port_register_zeroterm_cvars(env,_tmp1EF,orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct(),_tmp1E6);
# 1176
{union Cyc_Absyn_Constraint _tmp1F0=*_tmp1E1;union Cyc_Absyn_Constraint _tmp1F1=_tmp1F0;if((_tmp1F1.No_constr).tag == 3){_LL1BB: _LL1BC:
# 1178
 goto _LL1BA;}else{_LL1BD: _LL1BE:
# 1180
 Cyc_Port_unify_ct(_tmp1EF,((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp1E1)?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());
goto _LL1BA;}_LL1BA:;}
# 1186
return Cyc_Port_ptr_ct(_tmp1E7,_tmp1ED,_tmp1EE,new_rgn,_tmp1EF);};};}else{
# 1190
return Cyc_Port_ptr_ct(_tmp1E7,_tmp1DD.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),
_tmp1EA?Cyc_Port_fat_ct(): Cyc_Port_thin_ct(),new_rgn,
orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());}};}case 6: _LL19C: _LL19D:
# 1194
 goto _LL19F;case 7: _LL19E: _LL19F:
 return Cyc_Port_arith_ct();case 8: _LL1A0: _tmp1D8=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp1D3)->f1).elt_type;_tmp1D9=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp1D3)->f1).tq;_tmp1DA=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp1D3)->f1).zero_term;_tmp1DB=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp1D3)->f1).zt_loc;_LL1A1: {
# 1198
void*_tmp1F2=Cyc_Port_type_to_ctype(env,_tmp1D8);
int orig_zt;
{union Cyc_Absyn_Constraint _tmp1F3=*_tmp1DA;union Cyc_Absyn_Constraint _tmp1F4=_tmp1F3;if((_tmp1F4.No_constr).tag == 3){_LL1C0: _LL1C1:
 orig_zt=0;goto _LL1BF;}else{_LL1C2: _LL1C3:
 orig_zt=((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp1DA);goto _LL1BF;}_LL1BF:;}
# 1204
if((env->gcenv)->porting){
void*_tmp1F5=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,_tmp1F5,_tmp1D9.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),_tmp1D9.loc);{
# 1213
void*_tmp1F6=Cyc_Port_var();
Cyc_Port_register_zeroterm_cvars(env,_tmp1F6,orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct(),_tmp1DB);
# 1216
{union Cyc_Absyn_Constraint _tmp1F7=*_tmp1DA;union Cyc_Absyn_Constraint _tmp1F8=_tmp1F7;if((_tmp1F8.No_constr).tag == 3){_LL1C5: _LL1C6:
# 1218
 goto _LL1C4;}else{_LL1C7: _LL1C8:
# 1220
 Cyc_Port_unify_ct(_tmp1F6,((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp1DA)?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());
goto _LL1C4;}_LL1C4:;}
# 1223
return Cyc_Port_array_ct(_tmp1F2,_tmp1F5,_tmp1F6);};}else{
# 1225
return Cyc_Port_array_ct(_tmp1F2,_tmp1D9.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),
orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());}}case 9: _LL1A2: _tmp1D6=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D3)->f1).ret_typ;_tmp1D7=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D3)->f1).args;_LL1A3: {
# 1229
void*_tmp1F9=Cyc_Port_type_to_ctype(Cyc_Port_set_cpos(env,Cyc_Port_FnRes_pos),_tmp1D6);
struct Cyc_Port_Cenv*_tmp1FA=Cyc_Port_set_cpos(env,Cyc_Port_FnArg_pos);
struct Cyc_List_List*cargs=0;
for(0;_tmp1D7 != 0;_tmp1D7=_tmp1D7->tl){
struct _tuple8 _tmp1FB=*((struct _tuple8*)_tmp1D7->hd);struct Cyc_Absyn_Tqual _tmp1FD;void*_tmp1FE;struct _tuple8 _tmp1FC=_tmp1FB;_tmp1FD=_tmp1FC.f2;_tmp1FE=_tmp1FC.f3;{
struct Cyc_List_List*_tmp454;cargs=((_tmp454=_cycalloc(sizeof(*_tmp454)),((_tmp454->hd=Cyc_Port_type_to_ctype(_tmp1FA,_tmp1FE),((_tmp454->tl=cargs,_tmp454))))));};}
# 1236
return Cyc_Port_fn_ct(_tmp1F9,((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(cargs));}case 11: _LL1A4: _tmp1D5=(((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp1D3)->f1).aggr_info;_LL1A5: {
# 1238
union Cyc_Absyn_AggrInfoU _tmp200=_tmp1D5;struct Cyc_Absyn_Aggrdecl**_tmp201;struct _tuple0*_tmp202;struct _tuple0*_tmp203;if((_tmp200.UnknownAggr).tag == 1){if(((_tmp200.UnknownAggr).val).f1 == Cyc_Absyn_StructA){_LL1CA: _tmp203=((_tmp200.UnknownAggr).val).f2;_LL1CB:
# 1240
(*_tmp203).f1=Cyc_Absyn_Loc_n;{
struct _tuple11*_tmp204=Cyc_Port_lookup_struct_decl(env,_tmp203);
return Cyc_Port_known_aggr_ct(_tmp204);};}else{_LL1CC: _tmp202=((_tmp200.UnknownAggr).val).f2;_LL1CD:
# 1244
(*_tmp202).f1=Cyc_Absyn_Loc_n;{
struct _tuple11*_tmp205=Cyc_Port_lookup_union_decl(env,_tmp202);
return Cyc_Port_known_aggr_ct(_tmp205);};}}else{_LL1CE: _tmp201=(_tmp200.KnownAggr).val;_LL1CF: {
# 1248
struct Cyc_Absyn_Aggrdecl*_tmp206=*_tmp201;
enum Cyc_Absyn_AggrKind _tmp207=_tmp206->kind;enum Cyc_Absyn_AggrKind _tmp208=_tmp207;if(_tmp208 == Cyc_Absyn_StructA){_LL1D1: _LL1D2: {
# 1251
struct _tuple11*_tmp209=Cyc_Port_lookup_struct_decl(env,_tmp206->name);
return Cyc_Port_known_aggr_ct(_tmp209);}}else{_LL1D3: _LL1D4: {
# 1254
struct _tuple11*_tmp20A=Cyc_Port_lookup_union_decl(env,_tmp206->name);
return Cyc_Port_known_aggr_ct(_tmp20A);}}_LL1D0:;}}_LL1C9:;}case 13: _LL1A6: _LL1A7:
# 1258
 return Cyc_Port_arith_ct();case 14: _LL1A8: _tmp1D4=((struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct*)_tmp1D3)->f1;_LL1A9:
# 1261
 for(0;(unsigned int)_tmp1D4;_tmp1D4=_tmp1D4->tl){
(*((struct Cyc_Absyn_Enumfield*)_tmp1D4->hd)->name).f1=Cyc_Absyn_Loc_n;
env=Cyc_Port_add_var(env,((struct Cyc_Absyn_Enumfield*)_tmp1D4->hd)->name,Cyc_Absyn_sint_typ,Cyc_Port_const_ct(),Cyc_Port_arith_ct());}
# 1265
return Cyc_Port_arith_ct();default: _LL1AA: _LL1AB:
 return Cyc_Port_arith_ct();}_LL195:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1271
static void*Cyc_Port_gen_primop(struct Cyc_Port_Cenv*env,enum Cyc_Absyn_Primop p,struct Cyc_List_List*args){
void*_tmp20B=Cyc_Port_compress_ct((void*)((struct Cyc_List_List*)_check_null(args))->hd);
struct Cyc_List_List*_tmp20C=args->tl;
enum Cyc_Absyn_Primop _tmp20D=p;enum Cyc_Absyn_Primop _tmp20E;switch(_tmp20D){case Cyc_Absyn_Plus: _LL1D6: _LL1D7: {
# 1279
void*_tmp20F=Cyc_Port_compress_ct((void*)((struct Cyc_List_List*)_check_null(_tmp20C))->hd);
if(Cyc_Port_leq(_tmp20B,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()))){
Cyc_Port_leq(_tmp20F,Cyc_Port_arith_ct());
return _tmp20B;}else{
if(Cyc_Port_leq(_tmp20F,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()))){
Cyc_Port_leq(_tmp20B,Cyc_Port_arith_ct());
return _tmp20F;}else{
# 1287
Cyc_Port_leq(_tmp20B,Cyc_Port_arith_ct());
Cyc_Port_leq(_tmp20F,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();}}}case Cyc_Absyn_Minus: _LL1D8: _LL1D9:
# 1296
 if(_tmp20C == 0){
# 1298
Cyc_Port_leq(_tmp20B,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();}else{
# 1301
void*_tmp210=Cyc_Port_compress_ct((void*)_tmp20C->hd);
if(Cyc_Port_leq(_tmp20B,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()))){
if(Cyc_Port_leq(_tmp210,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var())))
return Cyc_Port_arith_ct();else{
# 1306
Cyc_Port_leq(_tmp210,Cyc_Port_arith_ct());
return _tmp20B;}}else{
# 1310
Cyc_Port_leq(_tmp20B,Cyc_Port_arith_ct());
Cyc_Port_leq(_tmp20B,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();}}case Cyc_Absyn_Times: _LL1DA: _LL1DB:
# 1315
 goto _LL1DD;case Cyc_Absyn_Div: _LL1DC: _LL1DD: goto _LL1DF;case Cyc_Absyn_Mod: _LL1DE: _LL1DF: goto _LL1E1;case Cyc_Absyn_Eq: _LL1E0: _LL1E1: goto _LL1E3;case Cyc_Absyn_Neq: _LL1E2: _LL1E3: goto _LL1E5;case Cyc_Absyn_Lt: _LL1E4: _LL1E5: goto _LL1E7;case Cyc_Absyn_Gt: _LL1E6: _LL1E7: goto _LL1E9;case Cyc_Absyn_Lte: _LL1E8: _LL1E9:
 goto _LL1EB;case Cyc_Absyn_Gte: _LL1EA: _LL1EB: goto _LL1ED;case Cyc_Absyn_Bitand: _LL1EC: _LL1ED: goto _LL1EF;case Cyc_Absyn_Bitor: _LL1EE: _LL1EF: goto _LL1F1;case Cyc_Absyn_Bitxor: _LL1F0: _LL1F1: goto _LL1F3;case Cyc_Absyn_Bitlshift: _LL1F2: _LL1F3: goto _LL1F5;case Cyc_Absyn_Bitlrshift: _LL1F4: _LL1F5:
 goto _LL1F7;case Cyc_Absyn_Bitarshift: _LL1F6: _LL1F7:
 Cyc_Port_leq(_tmp20B,Cyc_Port_arith_ct());
Cyc_Port_leq((void*)((struct Cyc_List_List*)_check_null(_tmp20C))->hd,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();case Cyc_Absyn_Not: _LL1F8: _LL1F9:
 goto _LL1FB;case Cyc_Absyn_Bitnot: _LL1FA: _LL1FB:
# 1323
 Cyc_Port_leq(_tmp20B,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();default: _LL1FC: _tmp20E=_tmp20D;_LL1FD: {
const char*_tmp457;void*_tmp456;(_tmp456=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp457=".size primop",_tag_dyneither(_tmp457,sizeof(char),13))),_tag_dyneither(_tmp456,sizeof(void*),0)));}}_LL1D5:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 1330
static struct _tuple10 Cyc_Port_gen_lhs(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Exp*e){
void*_tmp213=e->r;void*_tmp214=_tmp213;struct Cyc_Absyn_Exp*_tmp215;struct _dyneither_ptr*_tmp216;struct Cyc_Absyn_Exp*_tmp217;struct _dyneither_ptr*_tmp218;struct Cyc_Absyn_Exp*_tmp219;struct Cyc_Absyn_Exp*_tmp21A;struct Cyc_Absyn_Exp*_tmp21B;struct _tuple0*_tmp21C;switch(*((int*)_tmp214)){case 1: _LL1FF: _tmp21C=((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp214)->f1;_LL200:
# 1333
(*_tmp21C).f1=Cyc_Absyn_Loc_n;
return Cyc_Port_lookup_var(env,_tmp21C);case 22: _LL201: _tmp21A=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp214)->f1;_tmp21B=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp214)->f2;_LL202: {
# 1336
void*_tmp21D=Cyc_Port_var();
void*_tmp21E=Cyc_Port_var();
void*_tmp21F=Cyc_Port_gen_exp(env,_tmp21A);
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp21B),Cyc_Port_arith_ct());{
void*_tmp220=Cyc_Port_ptr_ct(_tmp21D,_tmp21E,Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var());
Cyc_Port_leq(_tmp21F,_tmp220);{
struct _tuple10 _tmp458;return(_tmp458.f1=_tmp21E,((_tmp458.f2=_tmp21D,_tmp458)));};};}case 19: _LL203: _tmp219=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp214)->f1;_LL204: {
# 1344
void*_tmp222=Cyc_Port_var();
void*_tmp223=Cyc_Port_var();
void*_tmp224=Cyc_Port_ptr_ct(_tmp222,_tmp223,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var());
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp219),_tmp224);{
struct _tuple10 _tmp459;return(_tmp459.f1=_tmp223,((_tmp459.f2=_tmp222,_tmp459)));};}case 20: _LL205: _tmp217=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp214)->f1;_tmp218=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp214)->f2;_LL206: {
# 1350
void*_tmp226=Cyc_Port_var();
void*_tmp227=Cyc_Port_var();
void*_tmp228=Cyc_Port_gen_exp(env,_tmp217);
{struct Cyc_Port_Cfield*_tmp45C;struct Cyc_Port_Cfield*_tmp45B[1];Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp217),Cyc_Port_unknown_aggr_ct(((_tmp45B[0]=((_tmp45C=_cycalloc(sizeof(*_tmp45C)),((_tmp45C->qual=_tmp227,((_tmp45C->f=_tmp218,((_tmp45C->type=_tmp226,_tmp45C)))))))),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp45B,sizeof(struct Cyc_Port_Cfield*),1))))));}{
struct _tuple10 _tmp45D;return(_tmp45D.f1=_tmp227,((_tmp45D.f2=_tmp226,_tmp45D)));};}case 21: _LL207: _tmp215=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp214)->f1;_tmp216=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp214)->f2;_LL208: {
# 1356
void*_tmp22C=Cyc_Port_var();
void*_tmp22D=Cyc_Port_var();
void*_tmp22E=Cyc_Port_gen_exp(env,_tmp215);
{struct Cyc_Port_Cfield*_tmp460;struct Cyc_Port_Cfield*_tmp45F[1];Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp215),Cyc_Port_ptr_ct(Cyc_Port_unknown_aggr_ct(((_tmp45F[0]=((_tmp460=_cycalloc(sizeof(*_tmp460)),((_tmp460->qual=_tmp22D,((_tmp460->f=_tmp216,((_tmp460->type=_tmp22C,_tmp460)))))))),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp45F,sizeof(struct Cyc_Port_Cfield*),1))))),
Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var()));}{
struct _tuple10 _tmp461;return(_tmp461.f1=_tmp22D,((_tmp461.f2=_tmp22C,_tmp461)));};}default: _LL209: _LL20A: {
struct Cyc_String_pa_PrintArg_struct _tmp469;void*_tmp468[1];const char*_tmp467;void*_tmp466;(_tmp466=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)((struct _dyneither_ptr)((_tmp469.tag=0,((_tmp469.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(e)),((_tmp468[0]=& _tmp469,Cyc_aprintf(((_tmp467="gen_lhs: %s",_tag_dyneither(_tmp467,sizeof(char),12))),_tag_dyneither(_tmp468,sizeof(void*),1)))))))),_tag_dyneither(_tmp466,sizeof(void*),0)));}}_LL1FE:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 1367
static void*Cyc_Port_gen_exp(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Exp*e){
void*_tmp236=e->r;void*_tmp237=_tmp236;struct Cyc_Absyn_Stmt*_tmp238;struct Cyc_Absyn_Exp*_tmp239;struct Cyc_Absyn_Exp*_tmp23A;void**_tmp23B;struct Cyc_Absyn_Exp*_tmp23C;struct Cyc_Absyn_Exp*_tmp23D;struct _dyneither_ptr*_tmp23E;struct Cyc_Absyn_Exp*_tmp23F;struct _dyneither_ptr*_tmp240;struct Cyc_Absyn_Exp*_tmp241;struct Cyc_Absyn_Exp*_tmp242;struct Cyc_Absyn_Exp*_tmp243;struct Cyc_Absyn_Exp*_tmp244;void*_tmp245;struct Cyc_Absyn_Exp*_tmp246;struct Cyc_Absyn_Exp*_tmp247;struct Cyc_Absyn_Exp*_tmp248;struct Cyc_List_List*_tmp249;struct Cyc_Absyn_Exp*_tmp24A;struct Cyc_Absyn_Exp*_tmp24B;struct Cyc_Absyn_Exp*_tmp24C;struct Cyc_Absyn_Exp*_tmp24D;struct Cyc_Absyn_Exp*_tmp24E;struct Cyc_Absyn_Exp*_tmp24F;struct Cyc_Absyn_Exp*_tmp250;struct Cyc_Absyn_Exp*_tmp251;struct Cyc_Absyn_Exp*_tmp252;struct Cyc_Absyn_Exp*_tmp253;enum Cyc_Absyn_Incrementor _tmp254;struct Cyc_Absyn_Exp*_tmp255;struct Cyc_Core_Opt*_tmp256;struct Cyc_Absyn_Exp*_tmp257;enum Cyc_Absyn_Primop _tmp258;struct Cyc_List_List*_tmp259;struct _tuple0*_tmp25A;switch(*((int*)_tmp237)){case 0: switch(((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp237)->f1).Wstring_c).tag){case 2: _LL20C: _LL20D:
 goto _LL20F;case 3: _LL20E: _LL20F:
 goto _LL211;case 4: _LL210: _LL211:
 goto _LL213;case 6: _LL212: _LL213:
 goto _LL215;case 7: _LL21E: _LL21F:
# 1378
 return Cyc_Port_arith_ct();case 5: if((((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp237)->f1).Int_c).val).f2 == 0){_LL220: _LL221:
 return Cyc_Port_zero_ct();}else{_LL222: _LL223:
 return Cyc_Port_arith_ct();}case 8: _LL224: _LL225:
# 1382
 return Cyc_Port_ptr_ct(Cyc_Port_arith_ct(),Cyc_Port_const_ct(),Cyc_Port_fat_ct(),Cyc_Port_heap_ct(),Cyc_Port_zterm_ct());case 9: _LL226: _LL227:
# 1384
 return Cyc_Port_ptr_ct(Cyc_Port_arith_ct(),Cyc_Port_const_ct(),Cyc_Port_fat_ct(),Cyc_Port_heap_ct(),Cyc_Port_zterm_ct());default: _LL228: _LL229:
 return Cyc_Port_zero_ct();}case 16: _LL214: _LL215:
# 1373
 goto _LL217;case 17: _LL216: _LL217:
 goto _LL219;case 18: _LL218: _LL219:
 goto _LL21B;case 31: _LL21A: _LL21B:
 goto _LL21D;case 32: _LL21C: _LL21D:
 goto _LL21F;case 1: _LL22A: _tmp25A=((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_LL22B:
# 1387
(*_tmp25A).f1=Cyc_Absyn_Loc_n;{
struct _tuple10 _tmp25B=Cyc_Port_lookup_var(env,_tmp25A);void*_tmp25D;struct _tuple10 _tmp25C=_tmp25B;_tmp25D=_tmp25C.f2;
if(Cyc_Port_isfn(env,_tmp25A)){
_tmp25D=Cyc_Port_instantiate(_tmp25D);
return Cyc_Port_ptr_ct(_tmp25D,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_heap_ct(),Cyc_Port_nozterm_ct());}else{
# 1393
if(Cyc_Port_isarray(env,_tmp25A)){
void*_tmp25E=Cyc_Port_var();
void*_tmp25F=Cyc_Port_var();
void*_tmp260=Cyc_Port_var();
void*_tmp261=Cyc_Port_array_ct(_tmp25E,_tmp25F,_tmp260);
Cyc_Port_unifies(_tmp25D,_tmp261);{
void*_tmp262=Cyc_Port_ptr_ct(_tmp25E,_tmp25F,Cyc_Port_fat_ct(),Cyc_Port_var(),_tmp260);
return _tmp262;};}else{
return _tmp25D;}}};case 2: _LL22C: _tmp258=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp259=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL22D:
 return Cyc_Port_gen_primop(env,_tmp258,((struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Port_Cenv*,struct Cyc_Absyn_Exp*),struct Cyc_Port_Cenv*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_gen_exp,env,_tmp259));case 3: _LL22E: _tmp255=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp256=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_tmp257=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp237)->f3;_LL22F: {
# 1404
struct _tuple10 _tmp263=Cyc_Port_gen_lhs(env,_tmp255);void*_tmp265;void*_tmp266;struct _tuple10 _tmp264=_tmp263;_tmp265=_tmp264.f1;_tmp266=_tmp264.f2;
Cyc_Port_unifies(_tmp265,Cyc_Port_notconst_ct());{
void*_tmp267=Cyc_Port_gen_exp(env,_tmp257);
if((unsigned int)_tmp256){
struct Cyc_List_List _tmp46A;struct Cyc_List_List x2=(_tmp46A.hd=_tmp267,((_tmp46A.tl=0,_tmp46A)));
struct Cyc_List_List _tmp46B;struct Cyc_List_List x1=(_tmp46B.hd=_tmp266,((_tmp46B.tl=& x2,_tmp46B)));
void*_tmp268=Cyc_Port_gen_primop(env,(enum Cyc_Absyn_Primop)_tmp256->v,& x1);
Cyc_Port_leq(_tmp268,_tmp266);
return _tmp266;}else{
# 1414
Cyc_Port_leq(_tmp267,_tmp266);
return _tmp266;}};}case 4: _LL230: _tmp253=((struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp254=((struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL231: {
# 1418
struct _tuple10 _tmp26B=Cyc_Port_gen_lhs(env,_tmp253);void*_tmp26D;void*_tmp26E;struct _tuple10 _tmp26C=_tmp26B;_tmp26D=_tmp26C.f1;_tmp26E=_tmp26C.f2;
Cyc_Port_unifies(_tmp26D,Cyc_Port_notconst_ct());
# 1421
Cyc_Port_leq(_tmp26E,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()));
Cyc_Port_leq(_tmp26E,Cyc_Port_arith_ct());
return _tmp26E;}case 5: _LL232: _tmp250=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp251=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_tmp252=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp237)->f3;_LL233: {
# 1425
void*_tmp26F=Cyc_Port_var();
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp250),Cyc_Port_arith_ct());
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp251),_tmp26F);
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp252),_tmp26F);
return _tmp26F;}case 6: _LL234: _tmp24E=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp24F=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL235:
 _tmp24C=_tmp24E;_tmp24D=_tmp24F;goto _LL237;case 7: _LL236: _tmp24C=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp24D=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL237:
# 1432
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp24C),Cyc_Port_arith_ct());
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp24D),Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();case 8: _LL238: _tmp24A=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp24B=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL239:
# 1436
 Cyc_Port_gen_exp(env,_tmp24A);
return Cyc_Port_gen_exp(env,_tmp24B);case 9: _LL23A: _tmp248=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp249=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL23B: {
# 1439
void*_tmp270=Cyc_Port_var();
void*_tmp271=Cyc_Port_gen_exp(env,_tmp248);
struct Cyc_List_List*_tmp272=((struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Port_Cenv*,struct Cyc_Absyn_Exp*),struct Cyc_Port_Cenv*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_gen_exp,env,_tmp249);
struct Cyc_List_List*_tmp273=((struct Cyc_List_List*(*)(void*(*f)(void*),struct Cyc_List_List*x))Cyc_List_map)((void*(*)(void*x))Cyc_Port_new_var,_tmp272);
Cyc_Port_unifies(_tmp271,Cyc_Port_ptr_ct(Cyc_Port_fn_ct(_tmp270,_tmp273),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_nozterm_ct()));
for(0;_tmp272 != 0;(_tmp272=_tmp272->tl,_tmp273=_tmp273->tl)){
Cyc_Port_leq((void*)_tmp272->hd,(void*)((struct Cyc_List_List*)_check_null(_tmp273))->hd);}
# 1447
return _tmp270;}case 10: _LL23C: _LL23D: {
const char*_tmp46E;void*_tmp46D;(_tmp46D=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp46E="throw",_tag_dyneither(_tmp46E,sizeof(char),6))),_tag_dyneither(_tmp46D,sizeof(void*),0)));}case 11: _LL23E: _tmp247=((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_LL23F:
 return Cyc_Port_gen_exp(env,_tmp247);case 12: _LL240: _LL241: {
const char*_tmp471;void*_tmp470;(_tmp470=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp471="instantiate",_tag_dyneither(_tmp471,sizeof(char),12))),_tag_dyneither(_tmp470,sizeof(void*),0)));}case 13: _LL242: _tmp245=(void*)((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp246=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL243: {
# 1452
void*_tmp278=Cyc_Port_type_to_ctype(env,_tmp245);
void*_tmp279=Cyc_Port_gen_exp(env,_tmp246);
Cyc_Port_leq(_tmp278,_tmp279);
return _tmp279;}case 14: _LL244: _tmp244=((struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_LL245: {
# 1457
struct _tuple10 _tmp27A=Cyc_Port_gen_lhs(env,_tmp244);void*_tmp27C;void*_tmp27D;struct _tuple10 _tmp27B=_tmp27A;_tmp27C=_tmp27B.f1;_tmp27D=_tmp27B.f2;
return Cyc_Port_ptr_ct(_tmp27D,_tmp27C,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var());}case 22: _LL246: _tmp242=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp243=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL247: {
# 1460
void*_tmp27E=Cyc_Port_var();
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp243),Cyc_Port_arith_ct());{
void*_tmp27F=Cyc_Port_gen_exp(env,_tmp242);
Cyc_Port_leq(_tmp27F,Cyc_Port_ptr_ct(_tmp27E,Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()));
return _tmp27E;};}case 19: _LL248: _tmp241=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_LL249: {
# 1466
void*_tmp280=Cyc_Port_var();
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp241),Cyc_Port_ptr_ct(_tmp280,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var()));
return _tmp280;}case 20: _LL24A: _tmp23F=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp240=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL24B: {
# 1470
void*_tmp281=Cyc_Port_var();
void*_tmp282=Cyc_Port_gen_exp(env,_tmp23F);
{struct Cyc_Port_Cfield*_tmp474;struct Cyc_Port_Cfield*_tmp473[1];Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp23F),Cyc_Port_unknown_aggr_ct(((_tmp473[0]=((_tmp474=_cycalloc(sizeof(*_tmp474)),((_tmp474->qual=Cyc_Port_var(),((_tmp474->f=_tmp240,((_tmp474->type=_tmp281,_tmp474)))))))),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp473,sizeof(struct Cyc_Port_Cfield*),1))))));}
return _tmp281;}case 21: _LL24C: _tmp23D=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp23E=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL24D: {
# 1475
void*_tmp285=Cyc_Port_var();
void*_tmp286=Cyc_Port_gen_exp(env,_tmp23D);
{struct Cyc_Port_Cfield*_tmp477;struct Cyc_Port_Cfield*_tmp476[1];Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp23D),Cyc_Port_ptr_ct(Cyc_Port_unknown_aggr_ct(((_tmp476[0]=((_tmp477=_cycalloc(sizeof(*_tmp477)),((_tmp477->qual=Cyc_Port_var(),((_tmp477->f=_tmp23E,((_tmp477->type=_tmp285,_tmp477)))))))),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp476,sizeof(struct Cyc_Port_Cfield*),1))))),
Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var()));}
return _tmp285;}case 33: _LL24E: _tmp23B=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp237)->f1).elt_type;_tmp23C=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp237)->f1).num_elts;_LL24F:
# 1481
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp23C),Cyc_Port_arith_ct());{
void*_tmp289=(unsigned int)_tmp23B?Cyc_Port_type_to_ctype(env,*_tmp23B): Cyc_Port_var();
return Cyc_Port_ptr_ct(_tmp289,Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_heap_ct(),Cyc_Port_var());};case 34: _LL250: _tmp239=((struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_tmp23A=((struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*)_tmp237)->f2;_LL251: {
const char*_tmp47A;void*_tmp479;(_tmp479=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp47A="Swap_e",_tag_dyneither(_tmp47A,sizeof(char),7))),_tag_dyneither(_tmp479,sizeof(void*),0)));}case 15: _LL252: _LL253: {
const char*_tmp47D;void*_tmp47C;(_tmp47C=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp47D="New_e",_tag_dyneither(_tmp47D,sizeof(char),6))),_tag_dyneither(_tmp47C,sizeof(void*),0)));}case 23: _LL254: _LL255: {
const char*_tmp480;void*_tmp47F;(_tmp47F=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp480="Tuple_e",_tag_dyneither(_tmp480,sizeof(char),8))),_tag_dyneither(_tmp47F,sizeof(void*),0)));}case 24: _LL256: _LL257: {
const char*_tmp483;void*_tmp482;(_tmp482=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp483="CompoundLit_e",_tag_dyneither(_tmp483,sizeof(char),14))),_tag_dyneither(_tmp482,sizeof(void*),0)));}case 25: _LL258: _LL259: {
const char*_tmp486;void*_tmp485;(_tmp485=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp486="Array_e",_tag_dyneither(_tmp486,sizeof(char),8))),_tag_dyneither(_tmp485,sizeof(void*),0)));}case 26: _LL25A: _LL25B: {
const char*_tmp489;void*_tmp488;(_tmp488=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp489="Comprehension_e",_tag_dyneither(_tmp489,sizeof(char),16))),_tag_dyneither(_tmp488,sizeof(void*),0)));}case 27: _LL25C: _LL25D: {
const char*_tmp48C;void*_tmp48B;(_tmp48B=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp48C="ComprehensionNoinit_e",_tag_dyneither(_tmp48C,sizeof(char),22))),_tag_dyneither(_tmp48B,sizeof(void*),0)));}case 28: _LL25E: _LL25F: {
const char*_tmp48F;void*_tmp48E;(_tmp48E=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp48F="Aggregate_e",_tag_dyneither(_tmp48F,sizeof(char),12))),_tag_dyneither(_tmp48E,sizeof(void*),0)));}case 29: _LL260: _LL261: {
const char*_tmp492;void*_tmp491;(_tmp491=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp492="AnonStruct_e",_tag_dyneither(_tmp492,sizeof(char),13))),_tag_dyneither(_tmp491,sizeof(void*),0)));}case 30: _LL262: _LL263: {
const char*_tmp495;void*_tmp494;(_tmp494=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp495="Datatype_e",_tag_dyneither(_tmp495,sizeof(char),11))),_tag_dyneither(_tmp494,sizeof(void*),0)));}case 35: _LL264: _LL265: {
const char*_tmp498;void*_tmp497;(_tmp497=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp498="UnresolvedMem_e",_tag_dyneither(_tmp498,sizeof(char),16))),_tag_dyneither(_tmp497,sizeof(void*),0)));}case 36: _LL266: _tmp238=((struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*)_tmp237)->f1;_LL267: {
const char*_tmp49B;void*_tmp49A;(_tmp49A=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp49B="StmtExp_e",_tag_dyneither(_tmp49B,sizeof(char),10))),_tag_dyneither(_tmp49A,sizeof(void*),0)));}case 38: _LL268: _LL269: {
const char*_tmp49E;void*_tmp49D;(_tmp49D=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp49E="Valueof_e",_tag_dyneither(_tmp49E,sizeof(char),10))),_tag_dyneither(_tmp49D,sizeof(void*),0)));}case 39: _LL26A: _LL26B: {
const char*_tmp4A1;void*_tmp4A0;(_tmp4A0=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4A1="Asm_e",_tag_dyneither(_tmp4A1,sizeof(char),6))),_tag_dyneither(_tmp4A0,sizeof(void*),0)));}default: _LL26C: _LL26D: {
const char*_tmp4A4;void*_tmp4A3;(_tmp4A3=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4A4="Tagcheck_e",_tag_dyneither(_tmp4A4,sizeof(char),11))),_tag_dyneither(_tmp4A3,sizeof(void*),0)));}}_LL20B:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1503
static void Cyc_Port_gen_stmt(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Stmt*s){
void*_tmp2A8=s->r;void*_tmp2A9=_tmp2A8;struct Cyc_Absyn_Stmt*_tmp2AA;struct Cyc_Absyn_Exp*_tmp2AB;struct Cyc_Absyn_Stmt*_tmp2AC;struct Cyc_Absyn_Decl*_tmp2AD;struct Cyc_Absyn_Stmt*_tmp2AE;struct Cyc_Absyn_Exp*_tmp2AF;struct Cyc_List_List*_tmp2B0;struct Cyc_Absyn_Exp*_tmp2B1;struct Cyc_Absyn_Exp*_tmp2B2;struct Cyc_Absyn_Exp*_tmp2B3;struct Cyc_Absyn_Stmt*_tmp2B4;struct Cyc_Absyn_Exp*_tmp2B5;struct Cyc_Absyn_Stmt*_tmp2B6;struct Cyc_Absyn_Exp*_tmp2B7;struct Cyc_Absyn_Stmt*_tmp2B8;struct Cyc_Absyn_Stmt*_tmp2B9;struct Cyc_Absyn_Exp*_tmp2BA;struct Cyc_Absyn_Stmt*_tmp2BB;struct Cyc_Absyn_Stmt*_tmp2BC;struct Cyc_Absyn_Exp*_tmp2BD;switch(*((int*)_tmp2A9)){case 0: _LL26F: _LL270:
 return;case 1: _LL271: _tmp2BD=((struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f1;_LL272:
 Cyc_Port_gen_exp(env,_tmp2BD);return;case 2: _LL273: _tmp2BB=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f1;_tmp2BC=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f2;_LL274:
 Cyc_Port_gen_stmt(env,_tmp2BB);Cyc_Port_gen_stmt(env,_tmp2BC);return;case 3: _LL275: _tmp2BA=((struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f1;_LL276: {
# 1509
void*_tmp2BE=Cyc_Port_lookup_return_type(env);
void*_tmp2BF=(unsigned int)_tmp2BA?Cyc_Port_gen_exp(env,_tmp2BA): Cyc_Port_void_ct();
Cyc_Port_leq(_tmp2BF,_tmp2BE);
return;}case 4: _LL277: _tmp2B7=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f1;_tmp2B8=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f2;_tmp2B9=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f3;_LL278:
# 1514
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2B7),Cyc_Port_arith_ct());
Cyc_Port_gen_stmt(env,_tmp2B8);
Cyc_Port_gen_stmt(env,_tmp2B9);
return;case 5: _LL279: _tmp2B5=(((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f1).f1;_tmp2B6=((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f2;_LL27A:
# 1519
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2B5),Cyc_Port_arith_ct());
Cyc_Port_gen_stmt(env,_tmp2B6);
return;case 6: _LL27B: _LL27C:
 goto _LL27E;case 7: _LL27D: _LL27E:
 goto _LL280;case 8: _LL27F: _LL280:
 return;case 9: _LL281: _tmp2B1=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f1;_tmp2B2=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f2).f1;_tmp2B3=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f3).f1;_tmp2B4=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f4;_LL282:
# 1526
 Cyc_Port_gen_exp(env,_tmp2B1);
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2B2),Cyc_Port_arith_ct());
Cyc_Port_gen_exp(env,_tmp2B3);
Cyc_Port_gen_stmt(env,_tmp2B4);
return;case 10: _LL283: _tmp2AF=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f1;_tmp2B0=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f2;_LL284:
# 1532
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2AF),Cyc_Port_arith_ct());
for(0;(unsigned int)_tmp2B0;_tmp2B0=_tmp2B0->tl){
# 1535
Cyc_Port_gen_stmt(env,((struct Cyc_Absyn_Switch_clause*)_tmp2B0->hd)->body);}
# 1537
return;case 11: _LL285: _LL286: {
const char*_tmp4A7;void*_tmp4A6;(_tmp4A6=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4A7="fallthru",_tag_dyneither(_tmp4A7,sizeof(char),9))),_tag_dyneither(_tmp4A6,sizeof(void*),0)));}case 12: _LL287: _tmp2AD=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f1;_tmp2AE=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f2;_LL288:
 env=Cyc_Port_gen_localdecl(env,_tmp2AD);Cyc_Port_gen_stmt(env,_tmp2AE);return;case 13: _LL289: _tmp2AC=((struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f2;_LL28A:
 Cyc_Port_gen_stmt(env,_tmp2AC);return;case 14: _LL28B: _tmp2AA=((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f1;_tmp2AB=(((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp2A9)->f2).f1;_LL28C:
# 1542
 Cyc_Port_gen_stmt(env,_tmp2AA);
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2AB),Cyc_Port_arith_ct());
return;case 15: _LL28D: _LL28E: {
const char*_tmp4AA;void*_tmp4A9;(_tmp4A9=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4AA="try/catch",_tag_dyneither(_tmp4AA,sizeof(char),10))),_tag_dyneither(_tmp4A9,sizeof(void*),0)));}default: _LL28F: _LL290: {
const char*_tmp4AD;void*_tmp4AC;(_tmp4AC=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4AD="reset region",_tag_dyneither(_tmp4AD,sizeof(char),13))),_tag_dyneither(_tmp4AC,sizeof(void*),0)));}}_LL26E:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct _tuple15{struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};
# 1551
static void*Cyc_Port_gen_initializer(struct Cyc_Port_Cenv*env,void*t,struct Cyc_Absyn_Exp*e){
void*_tmp2C6=e->r;void*_tmp2C7=_tmp2C6;struct Cyc_List_List*_tmp2C8;struct Cyc_List_List*_tmp2C9;struct Cyc_List_List*_tmp2CA;switch(*((int*)_tmp2C7)){case 35: _LL292: _tmp2CA=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp2C7)->f2;_LL293:
 _tmp2C9=_tmp2CA;goto _LL295;case 25: _LL294: _tmp2C9=((struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*)_tmp2C7)->f1;_LL295:
 _tmp2C8=_tmp2C9;goto _LL297;case 24: _LL296: _tmp2C8=((struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct*)_tmp2C7)->f2;_LL297:
# 1556
 LOOP: {
void*_tmp2CB=t;struct Cyc_Absyn_Aggrdecl*_tmp2CC;struct _tuple0*_tmp2CD;void*_tmp2CE;union Cyc_Absyn_Constraint*_tmp2CF;unsigned int _tmp2D0;struct _tuple0*_tmp2D1;switch(*((int*)_tmp2CB)){case 17: _LL29F: _tmp2D1=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp2CB)->f1;_LL2A0:
# 1559
(*_tmp2D1).f1=Cyc_Absyn_Loc_n;
t=Cyc_Port_lookup_typedef(env,_tmp2D1);goto LOOP;case 8: _LL2A1: _tmp2CE=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp2CB)->f1).elt_type;_tmp2CF=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp2CB)->f1).zero_term;_tmp2D0=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp2CB)->f1).zt_loc;_LL2A2: {
# 1562
void*_tmp2D2=Cyc_Port_var();
void*_tmp2D3=Cyc_Port_var();
void*_tmp2D4=Cyc_Port_var();
void*_tmp2D5=Cyc_Port_type_to_ctype(env,_tmp2CE);
for(0;_tmp2C8 != 0;_tmp2C8=_tmp2C8->tl){
struct _tuple15 _tmp2D6=*((struct _tuple15*)_tmp2C8->hd);struct Cyc_List_List*_tmp2D8;struct Cyc_Absyn_Exp*_tmp2D9;struct _tuple15 _tmp2D7=_tmp2D6;_tmp2D8=_tmp2D7.f1;_tmp2D9=_tmp2D7.f2;
if((unsigned int)_tmp2D8){const char*_tmp4B0;void*_tmp4AF;(_tmp4AF=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4B0="designators in initializers",_tag_dyneither(_tmp4B0,sizeof(char),28))),_tag_dyneither(_tmp4AF,sizeof(void*),0)));}{
void*_tmp2DC=Cyc_Port_gen_initializer(env,_tmp2CE,_tmp2D9);
Cyc_Port_leq(_tmp2DC,_tmp2D2);};}
# 1572
return Cyc_Port_array_ct(_tmp2D2,_tmp2D3,_tmp2D4);}case 11: if((((((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp2CB)->f1).aggr_info).UnknownAggr).tag == 1){if(((((((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp2CB)->f1).aggr_info).UnknownAggr).val).f1 == Cyc_Absyn_StructA){_LL2A3: _tmp2CD=((((((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp2CB)->f1).aggr_info).UnknownAggr).val).f2;_LL2A4:
# 1574
(*_tmp2CD).f1=Cyc_Absyn_Loc_n;{
struct _tuple11 _tmp2DD=*Cyc_Port_lookup_struct_decl(env,_tmp2CD);struct Cyc_Absyn_Aggrdecl*_tmp2DF;struct _tuple11 _tmp2DE=_tmp2DD;_tmp2DF=_tmp2DE.f1;
if(_tmp2DF == 0){const char*_tmp4B3;void*_tmp4B2;(_tmp4B2=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4B3="struct is not yet defined",_tag_dyneither(_tmp4B3,sizeof(char),26))),_tag_dyneither(_tmp4B2,sizeof(void*),0)));}
_tmp2CC=_tmp2DF;goto _LL2A6;};}else{goto _LL2A7;}}else{_LL2A5: _tmp2CC=*(((((struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp2CB)->f1).aggr_info).KnownAggr).val;_LL2A6: {
# 1579
struct Cyc_List_List*_tmp2E2=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp2CC->impl))->fields;
for(0;_tmp2C8 != 0;_tmp2C8=_tmp2C8->tl){
struct _tuple15 _tmp2E3=*((struct _tuple15*)_tmp2C8->hd);struct Cyc_List_List*_tmp2E5;struct Cyc_Absyn_Exp*_tmp2E6;struct _tuple15 _tmp2E4=_tmp2E3;_tmp2E5=_tmp2E4.f1;_tmp2E6=_tmp2E4.f2;
if((unsigned int)_tmp2E5){const char*_tmp4B6;void*_tmp4B5;(_tmp4B5=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4B6="designators in initializers",_tag_dyneither(_tmp4B6,sizeof(char),28))),_tag_dyneither(_tmp4B5,sizeof(void*),0)));}{
struct Cyc_Absyn_Aggrfield*_tmp2E9=(struct Cyc_Absyn_Aggrfield*)((struct Cyc_List_List*)_check_null(_tmp2E2))->hd;
_tmp2E2=_tmp2E2->tl;{
void*_tmp2EA=Cyc_Port_gen_initializer(env,_tmp2E9->type,_tmp2E6);;};};}
# 1587
return Cyc_Port_type_to_ctype(env,t);}}default: _LL2A7: _LL2A8: {
const char*_tmp4B9;void*_tmp4B8;(_tmp4B8=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4B9="bad type for aggregate initializer",_tag_dyneither(_tmp4B9,sizeof(char),35))),_tag_dyneither(_tmp4B8,sizeof(void*),0)));}}_LL29E:;}case 0: switch(((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp2C7)->f1).Wstring_c).tag){case 8: _LL298: _LL299:
# 1590
 goto _LL29B;case 9: _LL29A: _LL29B:
# 1592
 LOOP2: {
void*_tmp2ED=t;struct _tuple0*_tmp2EE;switch(*((int*)_tmp2ED)){case 17: _LL2AA: _tmp2EE=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp2ED)->f1;_LL2AB:
# 1595
(*_tmp2EE).f1=Cyc_Absyn_Loc_n;
t=Cyc_Port_lookup_typedef(env,_tmp2EE);goto LOOP2;case 8: _LL2AC: _LL2AD:
 return Cyc_Port_array_ct(Cyc_Port_arith_ct(),Cyc_Port_const_ct(),Cyc_Port_zterm_ct());default: _LL2AE: _LL2AF:
 return Cyc_Port_gen_exp(env,e);}_LL2A9:;}default: goto _LL29C;}default: _LL29C: _LL29D:
# 1600
 return Cyc_Port_gen_exp(env,e);}_LL291:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1605
static struct Cyc_Port_Cenv*Cyc_Port_gen_localdecl(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Decl*d){
void*_tmp2EF=d->r;void*_tmp2F0=_tmp2EF;struct Cyc_Absyn_Vardecl*_tmp2F1;if(((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp2F0)->tag == 0){_LL2B1: _tmp2F1=((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp2F0)->f1;_LL2B2: {
# 1608
void*_tmp2F2=Cyc_Port_var();
void*q;
if((env->gcenv)->porting){
q=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,q,
(_tmp2F1->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),(_tmp2F1->tq).loc);}else{
# 1621
q=(_tmp2F1->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}
# 1623
(*_tmp2F1->name).f1=Cyc_Absyn_Loc_n;
env=Cyc_Port_add_var(env,_tmp2F1->name,_tmp2F1->type,q,_tmp2F2);
Cyc_Port_unifies(_tmp2F2,Cyc_Port_type_to_ctype(env,_tmp2F1->type));
if((unsigned int)_tmp2F1->initializer){
struct Cyc_Absyn_Exp*e=(struct Cyc_Absyn_Exp*)_check_null(_tmp2F1->initializer);
void*t2=Cyc_Port_gen_initializer(env,_tmp2F1->type,e);
Cyc_Port_leq(t2,_tmp2F2);}
# 1631
return env;}}else{_LL2B3: _LL2B4: {
const char*_tmp4BC;void*_tmp4BB;(_tmp4BB=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp4BC="non-local decl that isn't a variable",_tag_dyneither(_tmp4BC,sizeof(char),37))),_tag_dyneither(_tmp4BB,sizeof(void*),0)));}}_LL2B0:;}
# 1636
static struct _tuple8*Cyc_Port_make_targ(struct _tuple8*arg){
struct _tuple8 _tmp2F5=*arg;struct _dyneither_ptr*_tmp2F7;struct Cyc_Absyn_Tqual _tmp2F8;void*_tmp2F9;struct _tuple8 _tmp2F6=_tmp2F5;_tmp2F7=_tmp2F6.f1;_tmp2F8=_tmp2F6.f2;_tmp2F9=_tmp2F6.f3;{
struct _tuple8*_tmp4BD;return(_tmp4BD=_cycalloc(sizeof(*_tmp4BD)),((_tmp4BD->f1=0,((_tmp4BD->f2=_tmp2F8,((_tmp4BD->f3=_tmp2F9,_tmp4BD)))))));};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1641
static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecls(struct Cyc_Port_Cenv*env,struct Cyc_List_List*ds);struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct _tuple16{struct _dyneither_ptr*f1;void*f2;void*f3;void*f4;};struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1643
static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecl(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Decl*d){
void*_tmp2FB=d->r;void*_tmp2FC=_tmp2FB;struct Cyc_Absyn_Enumdecl*_tmp2FD;struct Cyc_Absyn_Aggrdecl*_tmp2FE;struct Cyc_Absyn_Typedefdecl*_tmp2FF;struct Cyc_Absyn_Fndecl*_tmp300;struct Cyc_Absyn_Vardecl*_tmp301;switch(*((int*)_tmp2FC)){case 13: _LL2B6: _LL2B7:
# 1646
(env->gcenv)->porting=1;
return env;case 14: _LL2B8: _LL2B9:
# 1649
(env->gcenv)->porting=0;
return env;case 0: _LL2BA: _tmp301=((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp2FC)->f1;_LL2BB:
# 1652
(*_tmp301->name).f1=Cyc_Absyn_Loc_n;
# 1656
if(Cyc_Port_declared_var(env,_tmp301->name)){
struct _tuple10 _tmp302=Cyc_Port_lookup_var(env,_tmp301->name);void*_tmp304;void*_tmp305;struct _tuple10 _tmp303=_tmp302;_tmp304=_tmp303.f1;_tmp305=_tmp303.f2;{
void*q2;
if((env->gcenv)->porting  && !Cyc_Port_isfn(env,_tmp301->name)){
q2=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,q2,
(_tmp301->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),(_tmp301->tq).loc);}else{
# 1670
q2=(_tmp301->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}
# 1672
Cyc_Port_unifies(_tmp304,q2);
Cyc_Port_unifies(_tmp305,Cyc_Port_type_to_ctype(env,_tmp301->type));
if((unsigned int)_tmp301->initializer){
struct Cyc_Absyn_Exp*e=(struct Cyc_Absyn_Exp*)_check_null(_tmp301->initializer);
Cyc_Port_leq(Cyc_Port_gen_initializer(env,_tmp301->type,e),_tmp305);}
# 1678
return env;};}else{
# 1680
return Cyc_Port_gen_localdecl(env,d);}case 1: _LL2BC: _tmp300=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp2FC)->f1;_LL2BD:
# 1686
(*_tmp300->name).f1=Cyc_Absyn_Loc_n;{
struct _tuple13*predeclared=0;
# 1689
if(Cyc_Port_declared_var(env,_tmp300->name))
predeclared=Cyc_Port_lookup_full_var(env,_tmp300->name);{
# 1692
void*_tmp306=_tmp300->ret_type;
struct Cyc_List_List*_tmp307=_tmp300->args;
struct Cyc_List_List*_tmp308=((struct Cyc_List_List*(*)(struct _tuple8*(*f)(struct _tuple8*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Port_make_targ,_tmp307);
struct Cyc_Absyn_FnType_Absyn_Type_struct _tmp4C3;struct Cyc_Absyn_FnInfo _tmp4C2;struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp4C1;struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp309=
(_tmp4C1=_cycalloc(sizeof(*_tmp4C1)),((_tmp4C1[0]=((_tmp4C3.tag=9,((_tmp4C3.f1=((_tmp4C2.tvars=0,((_tmp4C2.effect=0,((_tmp4C2.ret_tqual=Cyc_Absyn_empty_tqual(0),((_tmp4C2.ret_typ=_tmp306,((_tmp4C2.args=_tmp308,((_tmp4C2.c_varargs=0,((_tmp4C2.cyc_varargs=0,((_tmp4C2.rgn_po=0,((_tmp4C2.attributes=0,((_tmp4C2.requires_clause=0,((_tmp4C2.requires_relns=0,((_tmp4C2.ensures_clause=0,((_tmp4C2.ensures_relns=0,_tmp4C2)))))))))))))))))))))))))),_tmp4C3)))),_tmp4C1)));
# 1699
struct Cyc_Port_Cenv*_tmp30A=Cyc_Port_set_cpos(env,Cyc_Port_FnBody_pos);
void*_tmp30B=Cyc_Port_type_to_ctype(_tmp30A,_tmp306);
struct Cyc_List_List*c_args=0;
struct Cyc_List_List*c_arg_types=0;
{struct Cyc_List_List*_tmp30C=_tmp307;for(0;(unsigned int)_tmp30C;_tmp30C=_tmp30C->tl){
struct _tuple8 _tmp30D=*((struct _tuple8*)_tmp30C->hd);struct _dyneither_ptr*_tmp30F;struct Cyc_Absyn_Tqual _tmp310;void*_tmp311;struct _tuple8 _tmp30E=_tmp30D;_tmp30F=_tmp30E.f1;_tmp310=_tmp30E.f2;_tmp311=_tmp30E.f3;{
# 1707
void*_tmp312=Cyc_Port_type_to_ctype(_tmp30A,_tmp311);
void*tqv;
if((env->gcenv)->porting){
tqv=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,tqv,_tmp310.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),_tmp310.loc);}else{
# 1719
tqv=_tmp310.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}
# 1721
{struct _tuple16*_tmp4C6;struct Cyc_List_List*_tmp4C5;c_args=((_tmp4C5=_cycalloc(sizeof(*_tmp4C5)),((_tmp4C5->hd=((_tmp4C6=_cycalloc(sizeof(*_tmp4C6)),((_tmp4C6->f1=_tmp30F,((_tmp4C6->f2=_tmp311,((_tmp4C6->f3=tqv,((_tmp4C6->f4=_tmp312,_tmp4C6)))))))))),((_tmp4C5->tl=c_args,_tmp4C5))))));}{
struct Cyc_List_List*_tmp4C7;c_arg_types=((_tmp4C7=_cycalloc(sizeof(*_tmp4C7)),((_tmp4C7->hd=_tmp312,((_tmp4C7->tl=c_arg_types,_tmp4C7))))));};};}}
# 1724
c_args=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(c_args);
c_arg_types=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(c_arg_types);{
void*_tmp316=Cyc_Port_fn_ct(_tmp30B,c_arg_types);
# 1730
(*_tmp300->name).f1=Cyc_Absyn_Loc_n;
_tmp30A=Cyc_Port_add_var(_tmp30A,_tmp300->name,(void*)_tmp309,Cyc_Port_const_ct(),_tmp316);
Cyc_Port_add_return_type(_tmp30A,_tmp30B);
{struct Cyc_List_List*_tmp317=c_args;for(0;(unsigned int)_tmp317;_tmp317=_tmp317->tl){
struct _tuple16 _tmp318=*((struct _tuple16*)_tmp317->hd);struct _dyneither_ptr*_tmp31A;void*_tmp31B;void*_tmp31C;void*_tmp31D;struct _tuple16 _tmp319=_tmp318;_tmp31A=_tmp319.f1;_tmp31B=_tmp319.f2;_tmp31C=_tmp319.f3;_tmp31D=_tmp319.f4;{
struct _tuple0*_tmp4C8;_tmp30A=Cyc_Port_add_var(_tmp30A,((_tmp4C8=_cycalloc(sizeof(*_tmp4C8)),((_tmp4C8->f1=Cyc_Absyn_Loc_n,((_tmp4C8->f2=_tmp31A,_tmp4C8)))))),_tmp31B,_tmp31C,_tmp31D);};}}
# 1737
Cyc_Port_gen_stmt(_tmp30A,_tmp300->body);
# 1742
Cyc_Port_generalize(0,_tmp316);{
# 1749
struct Cyc_Dict_Dict counts=((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Dict_empty)(Cyc_strptrcmp);
Cyc_Port_region_counts(& counts,_tmp316);
# 1753
Cyc_Port_register_rgns(env,counts,1,(void*)_tmp309,_tmp316);
env=Cyc_Port_add_var(_tmp30A,_tmp300->name,(void*)_tmp309,Cyc_Port_const_ct(),_tmp316);
if((unsigned int)predeclared){
# 1762
struct _tuple13 _tmp31F=*predeclared;void*_tmp321;void*_tmp322;void*_tmp323;struct _tuple13 _tmp320=_tmp31F;_tmp321=_tmp320.f1;_tmp322=(_tmp320.f2)->f1;_tmp323=(_tmp320.f2)->f2;
Cyc_Port_unifies(_tmp322,Cyc_Port_const_ct());
Cyc_Port_unifies(_tmp323,Cyc_Port_instantiate(_tmp316));
# 1766
Cyc_Port_register_rgns(env,counts,1,_tmp321,_tmp316);}
# 1768
return env;};};};};case 8: _LL2BE: _tmp2FF=((struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*)_tmp2FC)->f1;_LL2BF: {
# 1774
void*_tmp327=(void*)_check_null(_tmp2FF->defn);
void*_tmp328=Cyc_Port_type_to_ctype(env,_tmp327);
(*_tmp2FF->name).f1=Cyc_Absyn_Loc_n;
Cyc_Port_add_typedef(env,_tmp2FF->name,_tmp327,_tmp328);
return env;}case 5: _LL2C0: _tmp2FE=((struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_tmp2FC)->f1;_LL2C1: {
# 1784
struct _tuple0*_tmp329=_tmp2FE->name;
(*_tmp2FE->name).f1=Cyc_Absyn_Loc_n;{
struct _tuple11*previous;
struct Cyc_List_List*curr=0;
{enum Cyc_Absyn_AggrKind _tmp32A=_tmp2FE->kind;enum Cyc_Absyn_AggrKind _tmp32B=_tmp32A;if(_tmp32B == Cyc_Absyn_StructA){_LL2C7: _LL2C8:
# 1790
 previous=Cyc_Port_lookup_struct_decl(env,_tmp329);
goto _LL2C6;}else{_LL2C9: _LL2CA:
# 1793
 previous=Cyc_Port_lookup_union_decl(env,_tmp329);
goto _LL2C6;}_LL2C6:;}
# 1796
if((unsigned int)_tmp2FE->impl){
struct Cyc_List_List*cf=(*previous).f2;
{struct Cyc_List_List*_tmp32C=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp2FE->impl))->fields;for(0;(unsigned int)_tmp32C;_tmp32C=_tmp32C->tl){
struct Cyc_Absyn_Aggrfield*_tmp32D=(struct Cyc_Absyn_Aggrfield*)_tmp32C->hd;
void*qv;
if((env->gcenv)->porting){
qv=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,qv,
(_tmp32D->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),(_tmp32D->tq).loc);}else{
# 1812
qv=(_tmp32D->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}{
# 1814
void*_tmp32E=Cyc_Port_type_to_ctype(env,_tmp32D->type);
if(cf != 0){
struct Cyc_Port_Cfield _tmp32F=*((struct Cyc_Port_Cfield*)cf->hd);void*_tmp331;void*_tmp332;struct Cyc_Port_Cfield _tmp330=_tmp32F;_tmp331=_tmp330.qual;_tmp332=_tmp330.type;
cf=cf->tl;
Cyc_Port_unifies(qv,_tmp331);
Cyc_Port_unifies(_tmp32E,_tmp332);}{
# 1821
struct Cyc_Port_Cfield*_tmp4CB;struct Cyc_List_List*_tmp4CA;curr=((_tmp4CA=_cycalloc(sizeof(*_tmp4CA)),((_tmp4CA->hd=((_tmp4CB=_cycalloc(sizeof(*_tmp4CB)),((_tmp4CB->qual=qv,((_tmp4CB->f=_tmp32D->name,((_tmp4CB->type=_tmp32E,_tmp4CB)))))))),((_tmp4CA->tl=curr,_tmp4CA))))));};};}}
# 1823
curr=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(curr);
if((*previous).f2 == 0)
(*previous).f2=curr;}
# 1828
return env;};}case 7: _LL2C2: _tmp2FD=((struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_tmp2FC)->f1;_LL2C3:
# 1833
(*_tmp2FD->name).f1=Cyc_Absyn_Loc_n;
# 1835
if((unsigned int)_tmp2FD->fields){
struct Cyc_List_List*_tmp335=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp2FD->fields))->v;for(0;(unsigned int)_tmp335;_tmp335=_tmp335->tl){
(*((struct Cyc_Absyn_Enumfield*)_tmp335->hd)->name).f1=Cyc_Absyn_Loc_n;{
struct Cyc_Absyn_EnumType_Absyn_Type_struct _tmp4CE;struct Cyc_Absyn_EnumType_Absyn_Type_struct*_tmp4CD;env=Cyc_Port_add_var(env,((struct Cyc_Absyn_Enumfield*)_tmp335->hd)->name,(void*)((_tmp4CD=_cycalloc(sizeof(*_tmp4CD)),((_tmp4CD[0]=((_tmp4CE.tag=13,((_tmp4CE.f1=_tmp2FD->name,((_tmp4CE.f2=_tmp2FD,_tmp4CE)))))),_tmp4CD)))),
Cyc_Port_const_ct(),Cyc_Port_arith_ct());};}}
# 1841
return env;default: _LL2C4: _LL2C5:
# 1843
 if((env->gcenv)->porting){
const char*_tmp4D1;void*_tmp4D0;(_tmp4D0=0,Cyc_fprintf(Cyc_stderr,((_tmp4D1="Warning: Cyclone declaration found in code to be ported -- skipping.",_tag_dyneither(_tmp4D1,sizeof(char),69))),_tag_dyneither(_tmp4D0,sizeof(void*),0)));}
return env;}_LL2B5:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1850
static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecls(struct Cyc_Port_Cenv*env,struct Cyc_List_List*ds){
for(0;(unsigned int)ds;ds=ds->tl){
env=Cyc_Port_gen_topdecl(env,(struct Cyc_Absyn_Decl*)ds->hd);}
return env;}struct Cyc_Port_Cenv;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 1857
static struct Cyc_List_List*Cyc_Port_gen_edits(struct Cyc_List_List*ds){
# 1859
struct Cyc_Port_Cenv*env=Cyc_Port_gen_topdecls(Cyc_Port_initial_cenv(),ds);
# 1861
struct Cyc_List_List*_tmp33A=(env->gcenv)->pointer_edits;
struct Cyc_List_List*_tmp33B=(env->gcenv)->qualifier_edits;
struct Cyc_List_List*_tmp33C=(env->gcenv)->zeroterm_edits;
struct Cyc_List_List*_tmp33D=(env->gcenv)->edits;
# 1866
{struct Cyc_List_List*_tmp33E=_tmp33A;for(0;(unsigned int)_tmp33E;_tmp33E=_tmp33E->tl){
struct _tuple14 _tmp33F=*((struct _tuple14*)_tmp33E->hd);void*_tmp341;void*_tmp342;unsigned int _tmp343;struct _tuple14 _tmp340=_tmp33F;_tmp341=_tmp340.f1;_tmp342=_tmp340.f2;_tmp343=_tmp340.f3;
Cyc_Port_unifies(_tmp341,_tmp342);}}
# 1870
{struct Cyc_List_List*_tmp344=_tmp33B;for(0;(unsigned int)_tmp344;_tmp344=_tmp344->tl){
struct _tuple14 _tmp345=*((struct _tuple14*)_tmp344->hd);void*_tmp347;void*_tmp348;unsigned int _tmp349;struct _tuple14 _tmp346=_tmp345;_tmp347=_tmp346.f1;_tmp348=_tmp346.f2;_tmp349=_tmp346.f3;
Cyc_Port_unifies(_tmp347,_tmp348);}}
# 1874
{struct Cyc_List_List*_tmp34A=_tmp33C;for(0;(unsigned int)_tmp34A;_tmp34A=_tmp34A->tl){
struct _tuple14 _tmp34B=*((struct _tuple14*)_tmp34A->hd);void*_tmp34D;void*_tmp34E;unsigned int _tmp34F;struct _tuple14 _tmp34C=_tmp34B;_tmp34D=_tmp34C.f1;_tmp34E=_tmp34C.f2;_tmp34F=_tmp34C.f3;
Cyc_Port_unifies(_tmp34D,_tmp34E);}}
# 1880
for(0;(unsigned int)_tmp33A;_tmp33A=_tmp33A->tl){
struct _tuple14 _tmp350=*((struct _tuple14*)_tmp33A->hd);void*_tmp352;void*_tmp353;unsigned int _tmp354;struct _tuple14 _tmp351=_tmp350;_tmp352=_tmp351.f1;_tmp353=_tmp351.f2;_tmp354=_tmp351.f3;
if(!Cyc_Port_unifies(_tmp352,_tmp353) && (int)_tmp354){
void*_tmp355=Cyc_Port_compress_ct(_tmp353);void*_tmp356=_tmp355;switch(*((int*)_tmp356)){case 2: _LL2CC: _LL2CD:
# 1885
{struct Cyc_Port_Edit*_tmp4DA;const char*_tmp4D9;const char*_tmp4D8;struct Cyc_List_List*_tmp4D7;_tmp33D=((_tmp4D7=_cycalloc(sizeof(*_tmp4D7)),((_tmp4D7->hd=((_tmp4DA=_cycalloc(sizeof(*_tmp4DA)),((_tmp4DA->loc=_tmp354,((_tmp4DA->old_string=((_tmp4D9="?",_tag_dyneither(_tmp4D9,sizeof(char),2))),((_tmp4DA->new_string=((_tmp4D8="*",_tag_dyneither(_tmp4D8,sizeof(char),2))),_tmp4DA)))))))),((_tmp4D7->tl=_tmp33D,_tmp4D7))))));}
goto _LL2CB;case 3: _LL2CE: _LL2CF:
# 1888
{struct Cyc_Port_Edit*_tmp4E3;const char*_tmp4E2;const char*_tmp4E1;struct Cyc_List_List*_tmp4E0;_tmp33D=((_tmp4E0=_cycalloc(sizeof(*_tmp4E0)),((_tmp4E0->hd=((_tmp4E3=_cycalloc(sizeof(*_tmp4E3)),((_tmp4E3->loc=_tmp354,((_tmp4E3->old_string=((_tmp4E2="*",_tag_dyneither(_tmp4E2,sizeof(char),2))),((_tmp4E3->new_string=((_tmp4E1="?",_tag_dyneither(_tmp4E1,sizeof(char),2))),_tmp4E3)))))))),((_tmp4E0->tl=_tmp33D,_tmp4E0))))));}
goto _LL2CB;default: _LL2D0: _LL2D1:
 goto _LL2CB;}_LL2CB:;}}
# 1895
for(0;(unsigned int)_tmp33B;_tmp33B=_tmp33B->tl){
struct _tuple14 _tmp35F=*((struct _tuple14*)_tmp33B->hd);void*_tmp361;void*_tmp362;unsigned int _tmp363;struct _tuple14 _tmp360=_tmp35F;_tmp361=_tmp360.f1;_tmp362=_tmp360.f2;_tmp363=_tmp360.f3;
if(!Cyc_Port_unifies(_tmp361,_tmp362) && (int)_tmp363){
void*_tmp364=Cyc_Port_compress_ct(_tmp362);void*_tmp365=_tmp364;switch(*((int*)_tmp365)){case 1: _LL2D3: _LL2D4:
# 1900
{struct Cyc_Port_Edit*_tmp4EC;const char*_tmp4EB;const char*_tmp4EA;struct Cyc_List_List*_tmp4E9;_tmp33D=((_tmp4E9=_cycalloc(sizeof(*_tmp4E9)),((_tmp4E9->hd=((_tmp4EC=_cycalloc(sizeof(*_tmp4EC)),((_tmp4EC->loc=_tmp363,((_tmp4EC->old_string=((_tmp4EB="const ",_tag_dyneither(_tmp4EB,sizeof(char),7))),((_tmp4EC->new_string=((_tmp4EA="",_tag_dyneither(_tmp4EA,sizeof(char),1))),_tmp4EC)))))))),((_tmp4E9->tl=_tmp33D,_tmp4E9))))));}goto _LL2D2;case 0: _LL2D5: _LL2D6:
# 1902
{struct Cyc_Port_Edit*_tmp4F5;const char*_tmp4F4;const char*_tmp4F3;struct Cyc_List_List*_tmp4F2;_tmp33D=((_tmp4F2=_cycalloc(sizeof(*_tmp4F2)),((_tmp4F2->hd=((_tmp4F5=_cycalloc(sizeof(*_tmp4F5)),((_tmp4F5->loc=_tmp363,((_tmp4F5->old_string=((_tmp4F4="",_tag_dyneither(_tmp4F4,sizeof(char),1))),((_tmp4F5->new_string=((_tmp4F3="const ",_tag_dyneither(_tmp4F3,sizeof(char),7))),_tmp4F5)))))))),((_tmp4F2->tl=_tmp33D,_tmp4F2))))));}goto _LL2D2;default: _LL2D7: _LL2D8:
 goto _LL2D2;}_LL2D2:;}}
# 1908
for(0;(unsigned int)_tmp33C;_tmp33C=_tmp33C->tl){
struct _tuple14 _tmp36E=*((struct _tuple14*)_tmp33C->hd);void*_tmp370;void*_tmp371;unsigned int _tmp372;struct _tuple14 _tmp36F=_tmp36E;_tmp370=_tmp36F.f1;_tmp371=_tmp36F.f2;_tmp372=_tmp36F.f3;
if(!Cyc_Port_unifies(_tmp370,_tmp371) && (int)_tmp372){
void*_tmp373=Cyc_Port_compress_ct(_tmp371);void*_tmp374=_tmp373;switch(*((int*)_tmp374)){case 8: _LL2DA: _LL2DB:
# 1913
{struct Cyc_Port_Edit*_tmp4FE;const char*_tmp4FD;const char*_tmp4FC;struct Cyc_List_List*_tmp4FB;_tmp33D=((_tmp4FB=_cycalloc(sizeof(*_tmp4FB)),((_tmp4FB->hd=((_tmp4FE=_cycalloc(sizeof(*_tmp4FE)),((_tmp4FE->loc=_tmp372,((_tmp4FE->old_string=((_tmp4FD="@nozeroterm ",_tag_dyneither(_tmp4FD,sizeof(char),13))),((_tmp4FE->new_string=((_tmp4FC="",_tag_dyneither(_tmp4FC,sizeof(char),1))),_tmp4FE)))))))),((_tmp4FB->tl=_tmp33D,_tmp4FB))))));}goto _LL2D9;case 9: _LL2DC: _LL2DD:
# 1915
{struct Cyc_Port_Edit*_tmp507;const char*_tmp506;const char*_tmp505;struct Cyc_List_List*_tmp504;_tmp33D=((_tmp504=_cycalloc(sizeof(*_tmp504)),((_tmp504->hd=((_tmp507=_cycalloc(sizeof(*_tmp507)),((_tmp507->loc=_tmp372,((_tmp507->old_string=((_tmp506="@zeroterm ",_tag_dyneither(_tmp506,sizeof(char),11))),((_tmp507->new_string=((_tmp505="",_tag_dyneither(_tmp505,sizeof(char),1))),_tmp507)))))))),((_tmp504->tl=_tmp33D,_tmp504))))));}goto _LL2D9;default: _LL2DE: _LL2DF:
 goto _LL2D9;}_LL2D9:;}}
# 1922
_tmp33D=((struct Cyc_List_List*(*)(int(*cmp)(struct Cyc_Port_Edit*,struct Cyc_Port_Edit*),struct Cyc_List_List*x))Cyc_List_merge_sort)(Cyc_Port_cmp_edit,_tmp33D);
# 1924
while((unsigned int)_tmp33D  && ((struct Cyc_Port_Edit*)_tmp33D->hd)->loc == 0){
# 1928
_tmp33D=_tmp33D->tl;}
# 1930
return _tmp33D;}struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 1933
static unsigned int Cyc_Port_get_seg(struct Cyc_Port_Edit*e){
return e->loc;}struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 1939
void Cyc_Port_port(struct Cyc_List_List*ds){
struct Cyc_List_List*_tmp37D=Cyc_Port_gen_edits(ds);
struct Cyc_List_List*_tmp37E=((struct Cyc_List_List*(*)(unsigned int(*f)(struct Cyc_Port_Edit*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Port_get_seg,_tmp37D);
Cyc_Position_use_gcc_style_location=0;{
struct Cyc_List_List*_tmp37F=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_Position_strings_of_segments(_tmp37E));
for(0;(unsigned int)_tmp37D;(_tmp37D=_tmp37D->tl,_tmp37F=_tmp37F->tl)){
struct Cyc_Port_Edit _tmp380=*((struct Cyc_Port_Edit*)_tmp37D->hd);unsigned int _tmp382;struct _dyneither_ptr _tmp383;struct _dyneither_ptr _tmp384;struct Cyc_Port_Edit _tmp381=_tmp380;_tmp382=_tmp381.loc;_tmp383=_tmp381.old_string;_tmp384=_tmp381.new_string;{
struct _dyneither_ptr sloc=(struct _dyneither_ptr)*((struct _dyneither_ptr*)((struct Cyc_List_List*)_check_null(_tmp37F))->hd);
const char*_tmp50D;void*_tmp50C[3];struct Cyc_String_pa_PrintArg_struct _tmp50B;struct Cyc_String_pa_PrintArg_struct _tmp50A;struct Cyc_String_pa_PrintArg_struct _tmp509;(_tmp509.tag=0,((_tmp509.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp384),((_tmp50A.tag=0,((_tmp50A.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp383),((_tmp50B.tag=0,((_tmp50B.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)sloc),((_tmp50C[0]=& _tmp50B,((_tmp50C[1]=& _tmp50A,((_tmp50C[2]=& _tmp509,Cyc_printf(((_tmp50D="%s: insert `%s' for `%s'\n",_tag_dyneither(_tmp50D,sizeof(char),26))),_tag_dyneither(_tmp50C,sizeof(void*),3)))))))))))))))))));};}};}
