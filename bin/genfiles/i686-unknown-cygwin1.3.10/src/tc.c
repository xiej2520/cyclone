/* This is a C header file to be used by the output of the Cyclone
   to C translator.  The corresponding definitions are in file lib/runtime_cyc.c
*/
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

#include <setjmp.h>

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
struct _dynforward_ptr {
  unsigned char *curr;
  unsigned char *last_plus_one;
};

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
  char data[0];
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
#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#ifdef _INLINE_FUNCTIONS
static inline void *
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
static inline char *
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
static inline unsigned
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
#define _zero_arr_plus(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#else
#define _zero_arr_plus(orig_x,orig_sz,orig_i) ({ \
  typedef _czs_tx = (*orig_x); \
  _czs_tx *_czs_x = (_czs_tx *)(orig_x); \
  unsigned int _czs_sz = (orig_sz); \
  int _czs_i = (orig_i); \
  unsigned int _czs_temp; \
  if ((_czs_x) == 0) _throw_null(); \
  if (_czs_i < 0) _throw_arraybounds(); \
  for (_czs_temp=_czs_sz; _czs_temp < _czs_i; _czs_temp++) \
    if (_czs_x[_czs_temp] == 0) _throw_arraybounds(); \
  _czs_x+_czs_i; })
#endif

/* Calculates the number of elements in a zero-terminated, thin array.
   If non-null, the array is guaranteed to have orig_offset elements. */
#define _get_zero_arr_size(orig_x,orig_offset) ({ \
  typedef _gres_tx = (*orig_x); \
  _gres_tx *_gres_x = (_gres_tx *)(orig_x); \
  unsigned int _gres_offset = (orig_offset); \
  unsigned int _gres = 0; \
  if (_gres_x != 0) { \
     _gres = _gres_offset; \
     _gres_x += _gres_offset - 1; \
     while (*_gres_x != 0) { _gres_x++; _gres++; } \
  } _gres; })

/* Does in-place addition of a zero-terminated pointer (x += e and ++x).  
   Note that this expands to call _zero_arr_plus. */
#define _zero_arr_inplace_plus(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  *_zap_x = _zero_arr_plus(*_zap_x,1,(orig_i)); })

/* Does in-place increment of a zero-terminated pointer (e.g., x++).
   Note that this expands to call _zero_arr_plus. */
#define _zero_arr_inplace_plus_post(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  _zap_tx *_zap_res = *_zap_x; \
  *_zap_x = _zero_arr_plus(_zap_res,1,(orig_i)); \
  _zap_res; })
  


/* functions for dealing with dynamically sized pointers */
#ifdef NO_CYC_BOUNDS_CHECKS
#ifdef _INLINE_FUNCTIONS
static inline unsigned char *
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  return _cus_ans;
}
static inline unsigned char *
_check_dynforward_subscript(struct _dynforward_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dynforward_ptr _cus_arr = (arr);
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
#define _check_dynforward_subscript(arr,elt_sz,index) ({ \
  struct _dynforward_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
#endif
#else
#ifdef _INLINE_FUNCTIONS
static inline unsigned char *
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  if (!_cus_arr.base) _throw_null();
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds();
  return _cus_ans;
}
static inline unsigned char *
_check_dynforward_subscript(struct _dynforward_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dynforward_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  if (!_cus_arr.last_plus_one) _throw_null();
  if (_cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds();
  return _cus_ans;
}
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  if (!_cus_arr.base) _throw_null(); \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#define _check_dynforward_subscript(arr,elt_sz,index) ({ \
  struct _dynforward_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  if (!_cus_arr.last_plus_one) _throw_null(); \
  if (_cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _dyneither_ptr
_tag_dyneither(const void *tcurr,unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _tag_arr_ans;
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr);
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts);
  return _tag_arr_ans;
}
static inline struct _dynforward_ptr
_tag_dynforward(const void *tcurr,unsigned elt_sz,unsigned num_elts) {
  struct _dynforward_ptr _tag_arr_ans;
  _tag_arr_ans.curr = (void*)(tcurr);
  _tag_arr_ans.last_plus_one = _tag_arr_ans.curr + (elt_sz) * (num_elts);
  return _tag_arr_ans;
}
#else
#define _tag_dyneither(tcurr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })
#define _tag_dynforward(tcurr,elt_sz,num_elts) ({ \
  struct _dynforward_ptr _tag_arr_ans; \
  _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.curr + (elt_sz) * (num_elts); \
  _tag_arr_ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _dyneither_ptr *
_init_dyneither_ptr(struct _dyneither_ptr *arr_ptr,
                    void *arr, unsigned elt_sz, unsigned num_elts) {
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr);
  void* _itarr = (arr);
  _itarr_ptr->base = _itarr_ptr->curr = _itarr;
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts);
  return _itarr_ptr;
}
static inline struct _dynforward_ptr *
_init_dynforward_ptr(struct _dynforward_ptr *arr_ptr,
                    void *arr, unsigned elt_sz, unsigned num_elts) {
  struct _dynforward_ptr *_itarr_ptr = (arr_ptr);
  void* _itarr = (arr);
  _itarr_ptr->curr = _itarr;
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts);
  return _itarr_ptr;
}
#else
#define _init_dyneither_ptr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->base = _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })
#define _init_dynforward_ptr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _dynforward_ptr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_dynforward_ptr(arr,elt_sz,num_elts) ((arr).curr)
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ((arr).curr)
#else
#ifdef _INLINE_FUNCTIONS
static inline unsigned char *
_untag_dyneither_ptr(struct _dyneither_ptr arr, 
                     unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds();
  return _curr;
}
static inline unsigned char *
_untag_dynforward_ptr(struct _dynforward_ptr arr, 
                      unsigned elt_sz,unsigned num_elts) {
  struct _dynforward_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
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
#define _untag_dynforward_ptr(arr,elt_sz,num_elts) ({ \
  struct _dynforward_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
    _throw_arraybounds(); \
  _curr; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static inline unsigned
_get_dyneither_size(struct _dyneither_ptr arr,unsigned elt_sz) {
  struct _dyneither_ptr _get_arr_size_temp = (arr);
  unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr;
  unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one;
  return (_get_arr_size_curr < _get_arr_size_temp.base ||
          _get_arr_size_curr >= _get_arr_size_last) ? 0 :
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));
}
static inline unsigned
_get_dynforward_size(struct _dynforward_ptr arr,unsigned elt_sz) {
  struct _dynforward_ptr _get_arr_size_temp = (arr);
  unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr;
  unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one;
  return (_get_arr_size_curr >= _get_arr_size_last) ? 0 :
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
#define _get_dynforward_size(arr,elt_sz) \
  ({struct _dynforward_ptr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _dyneither_ptr
_dyneither_ptr_plus(struct _dyneither_ptr arr,unsigned elt_sz,int change) {
  struct _dyneither_ptr _ans = (arr);
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
/* Here we have to worry about wrapping around, so if we go past the
 * end, we set the end to 0. */
static inline struct _dynforward_ptr
_dynforward_ptr_plus(struct _dynforward_ptr arr,unsigned elt_sz,int change) {
  struct _dynforward_ptr _ans = (arr);
  unsigned int _dfpp_elts = (((unsigned)_ans.last_plus_one) - 
                             ((unsigned)_ans.curr)) / elt_sz;
  if (change < 0 || ((unsigned)change) > _dfpp_elts)
    _ans.last_plus_one = 0;
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_plus(arr,elt_sz,change) ({ \
  struct _dyneither_ptr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })
#define _dynforward_ptr_plus(arr,elt_sz,change) ({ \
  struct _dynforward_ptr _ans = (arr); \
  unsigned _dfpp_elt_sz = (elt_sz); \
  int _dfpp_change = (change); \
  unsigned int _dfpp_elts = (((unsigned)_ans.last_plus_one) - \
                            ((unsigned)_ans.curr)) / _dfpp_elt_sz; \
  if (_dfpp_change < 0 || ((unsigned)_dfpp_change) > _dfpp_elts) \
    _ans.last_plus_one = 0; \
  _ans.curr += ((int)(_dfpp_elt_sz))*(_dfpp_change); \
  _ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _dyneither_ptr
_dyneither_ptr_inplace_plus(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,
                            int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return *_arr_ptr;
}
static inline struct _dynforward_ptr
_dynforward_ptr_inplace_plus(struct _dynforward_ptr *arr_ptr,unsigned elt_sz,
                             int change) {
  struct _dynforward_ptr * _arr_ptr = (arr_ptr);
  unsigned int _dfpp_elts = (((unsigned)_arr_ptr->last_plus_one) - 
                             ((unsigned)_arr_ptr->curr)) / elt_sz;
  if (change < 0 || ((unsigned)change) > _dfpp_elts) 
    _arr_ptr->last_plus_one = 0;
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return *_arr_ptr;
}
#else
#define _dyneither_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })
#define _dynforward_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dynforward_ptr * _arr_ptr = (arr_ptr); \
  unsigned _dfpp_elt_sz = (elt_sz); \
  int _dfpp_change = (change); \
  unsigned int _dfpp_elts = (((unsigned)_arr_ptr->last_plus_one) - \
                            ((unsigned)_arr_ptr->curr)) / _dfpp_elt_sz; \
  if (_dfpp_change < 0 || ((unsigned)_dfpp_change) > _dfpp_elts) \
    _arr_ptr->last_plus_one = 0; \
  _arr_ptr->curr += ((int)(_dfpp_elt_sz))*(_dfpp_change); \
  *_arr_ptr; })
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _dyneither_ptr
_dyneither_ptr_inplace_plus_post(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  struct _dyneither_ptr _ans = *_arr_ptr;
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return _ans;
}
static inline struct _dynforward_ptr
_dynforward_ptr_inplace_plus_post(struct _dynforward_ptr *arr_ptr,unsigned elt_sz,int change) {
  struct _dynforward_ptr * _arr_ptr = (arr_ptr);
  struct _dynforward_ptr _ans = *_arr_ptr;
  unsigned int _dfpp_elts = (((unsigned)_arr_ptr->last_plus_one) - 
                            ((unsigned)_arr_ptr->curr)) / elt_sz; 
  if (change < 0 || ((unsigned)change) > _dfpp_elts) 
    _arr_ptr->last_plus_one = 0; 
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  struct _dyneither_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })
#define _dynforward_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dynforward_ptr * _arr_ptr = (arr_ptr); \
  struct _dynforward_ptr _ans = *_arr_ptr; \
  unsigned _dfpp_elt_sz = (elt_sz); \
  int _dfpp_change = (change); \
  unsigned int _dfpp_elts = (((unsigned)_arr_ptr->last_plus_one) - \
                            ((unsigned)_arr_ptr->curr)) / _dfpp_elt_sz; \
  if (_dfpp_change < 0 || ((unsigned)_dfpp_change) > _dfpp_elts) \
    _arr_ptr->last_plus_one = 0; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

// Decrease the upper bound on a fat pointer by numelts where sz is
// the size of the pointer's type.  Note that this can't be a macro
// if we're to get initializers right.
static struct 
_dyneither_ptr _dyneither_ptr_decrease_size(struct _dyneither_ptr x,
                                            unsigned int sz,
                                            unsigned int numelts) {
  x.last_plus_one -= sz * numelts; 
  return x; 
}
static struct 
_dynforward_ptr _dynforward_ptr_decrease_size(struct _dynforward_ptr x,
                                            unsigned int sz,
                                            unsigned int numelts) {
  if (x.last_plus_one != 0)
    x.last_plus_one -= sz * numelts; 
  return x; 
}

/* Convert between the two forms of dynamic pointers */
#ifdef _INLINE_FUNCTIONS 
static struct _dynforward_ptr
_dyneither_to_dynforward(struct _dyneither_ptr p) {
  struct _dynforward_ptr res;
  res.curr = p.curr;
  res.last_plus_one = (p.base == 0) ? 0 : p.last_plus_one;
  return res;
}
static struct _dyneither_ptr
_dynforward_to_dyneither(struct _dynforward_ptr p) {
  struct _dyneither_ptr res;
  res.base = res.curr = p.curr;
  res.last_plus_one = p.last_plus_one;
  if (p.last_plus_one == 0) 
    res.base = 0;
  return res;
}
#else 
#define _dyneither_to_dynforward(_dnfptr) ({ \
  struct _dyneither_ptr _dnfp = (_dnfptr); \
  struct _dynforward_ptr _dnfpres; \
  _dnfpres.curr = _dnfp.curr; \
  _dnfpres.last_plus_one = (_dnfp.base == 0) ? 0 : _dnfp.last_plus_one; \
  _dnfpres; })
#define _dynforward_to_dyneither(_dfnptr) ({ \
  struct _dynforward_ptr _dfnp = (_dfnptr); \
  struct _dyneither_ptr _dfnres; \
  _dfnres.base = _dfnres.curr = _dfnp.curr; \
  _dfnres.last_plus_one = _dfnp.last_plus_one; \
  if (_dfnp.last_plus_one == 0) \
    _dfnres.base = 0; \
  _dfnres; })
#endif 


//// Allocation
extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);

static inline void* _cycalloc(int n) {
  void * ans = (void *)GC_malloc(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static inline void* _cycalloc_atomic(int n) {
  void * ans = (void *)GC_malloc_atomic(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static inline void* _cyccalloc(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
static inline void* _cyccalloc_atomic(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc_atomic(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
#define MAX_MALLOC_SIZE (1 << 28)
static inline unsigned int _check_times(unsigned x, unsigned y) {
  unsigned long long whole_ans = 
    ((unsigned long long)x)*((unsigned long long)y);
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

/* the next three routines swap [x] and [y]; not thread safe! */
static inline void _swap_word(void *x, void *y) {
  unsigned long *lx = (unsigned long *)x, *ly = (unsigned long *)y, tmp;
  tmp = *lx;
  *lx = *ly;
  *ly = tmp;
}
static inline void _swap_dynforward(struct _dynforward_ptr *x, 
				    struct _dynforward_ptr *y) {
  struct _dynforward_ptr tmp = *x;
  *x = *y;
  *y = tmp;
}
static inline void _swap_dyneither(struct _dyneither_ptr *x, 
				   struct _dyneither_ptr *y) {
  struct _dyneither_ptr tmp = *x;
  *x = *y;
  *y = tmp;
}
 struct Cyc_Core_NewRegion{struct _DynRegionHandle*dynregion;};struct Cyc_Core_Opt{
void*v;};extern char Cyc_Core_Invalid_argument[21];struct Cyc_Core_Invalid_argument_struct{
char*tag;struct _dynforward_ptr f1;};extern char Cyc_Core_Failure[12];struct Cyc_Core_Failure_struct{
char*tag;struct _dynforward_ptr f1;};extern char Cyc_Core_Impossible[15];struct Cyc_Core_Impossible_struct{
char*tag;struct _dynforward_ptr f1;};extern char Cyc_Core_Not_found[14];extern char
Cyc_Core_Unreachable[16];struct Cyc_Core_Unreachable_struct{char*tag;struct
_dynforward_ptr f1;};extern struct _RegionHandle*Cyc_Core_heap_region;extern char Cyc_Core_Open_Region[
16];extern char Cyc_Core_Free_Region[16];struct Cyc_List_List{void*hd;struct Cyc_List_List*
tl;};int Cyc_List_length(struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[
18];void Cyc_List_iter(void(*f)(void*),struct Cyc_List_List*x);void Cyc_List_iter_c(
void(*f)(void*,void*),void*env,struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_append(
struct Cyc_List_List*x,struct Cyc_List_List*y);struct Cyc_List_List*Cyc_List_imp_append(
struct Cyc_List_List*x,struct Cyc_List_List*y);extern char Cyc_List_Nth[8];int Cyc_List_mem(
int(*compare)(void*,void*),struct Cyc_List_List*l,void*x);struct Cyc_List_List*Cyc_List_filter_c(
int(*f)(void*,void*),void*env,struct Cyc_List_List*x);int Cyc_strcmp(struct
_dynforward_ptr s1,struct _dynforward_ptr s2);int Cyc_strptrcmp(struct
_dynforward_ptr*s1,struct _dynforward_ptr*s2);struct Cyc_Lineno_Pos{struct
_dynforward_ptr logical_file;struct _dynforward_ptr line;int line_no;int col;};extern
char Cyc_Position_Exit[9];struct Cyc_Position_Segment;struct Cyc_Position_Error{
struct _dynforward_ptr source;struct Cyc_Position_Segment*seg;void*kind;struct
_dynforward_ptr desc;};extern char Cyc_Position_Nocontext[14];struct Cyc_Absyn_Loc_n_struct{
int tag;};struct Cyc_Absyn_Rel_n_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Abs_n_struct{
int tag;struct Cyc_List_List*f1;};union Cyc_Absyn_Nmspace_union{struct Cyc_Absyn_Loc_n_struct
Loc_n;struct Cyc_Absyn_Rel_n_struct Rel_n;struct Cyc_Absyn_Abs_n_struct Abs_n;};
struct _tuple0{union Cyc_Absyn_Nmspace_union f1;struct _dynforward_ptr*f2;};struct
Cyc_Absyn_Conref;struct Cyc_Absyn_Tqual{int print_const;int q_volatile;int
q_restrict;int real_const;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Eq_constr_struct{
int tag;void*f1;};struct Cyc_Absyn_Forward_constr_struct{int tag;struct Cyc_Absyn_Conref*
f1;};struct Cyc_Absyn_No_constr_struct{int tag;};union Cyc_Absyn_Constraint_union{
struct Cyc_Absyn_Eq_constr_struct Eq_constr;struct Cyc_Absyn_Forward_constr_struct
Forward_constr;struct Cyc_Absyn_No_constr_struct No_constr;};struct Cyc_Absyn_Conref{
union Cyc_Absyn_Constraint_union v;};struct Cyc_Absyn_Eq_kb_struct{int tag;void*f1;}
;struct Cyc_Absyn_Unknown_kb_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_struct{
int tag;struct Cyc_Core_Opt*f1;void*f2;};struct Cyc_Absyn_Tvar{struct
_dynforward_ptr*name;int identity;void*kind;};struct Cyc_Absyn_Upper_b_struct{int
tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AbsUpper_b_struct{int tag;void*f1;};
struct Cyc_Absyn_PtrLoc{struct Cyc_Position_Segment*ptr_loc;struct Cyc_Position_Segment*
rgn_loc;struct Cyc_Position_Segment*zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;
struct Cyc_Absyn_Conref*nullable;struct Cyc_Absyn_Conref*bounds;struct Cyc_Absyn_Conref*
zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{void*elt_typ;
struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{
struct Cyc_Core_Opt*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{
struct Cyc_List_List*tvars;struct Cyc_Core_Opt*effect;void*ret_typ;struct Cyc_List_List*
args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*
rgn_po;struct Cyc_List_List*attributes;};struct Cyc_Absyn_UnknownTunionInfo{struct
_tuple0*name;int is_xtunion;int is_flat;};struct Cyc_Absyn_UnknownTunion_struct{int
tag;struct Cyc_Absyn_UnknownTunionInfo f1;};struct Cyc_Absyn_KnownTunion_struct{int
tag;struct Cyc_Absyn_Tuniondecl**f1;};union Cyc_Absyn_TunionInfoU_union{struct Cyc_Absyn_UnknownTunion_struct
UnknownTunion;struct Cyc_Absyn_KnownTunion_struct KnownTunion;};struct Cyc_Absyn_TunionInfo{
union Cyc_Absyn_TunionInfoU_union tunion_info;struct Cyc_List_List*targs;struct Cyc_Core_Opt*
rgn;};struct Cyc_Absyn_UnknownTunionFieldInfo{struct _tuple0*tunion_name;struct
_tuple0*field_name;int is_xtunion;};struct Cyc_Absyn_UnknownTunionfield_struct{int
tag;struct Cyc_Absyn_UnknownTunionFieldInfo f1;};struct Cyc_Absyn_KnownTunionfield_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};union Cyc_Absyn_TunionFieldInfoU_union{
struct Cyc_Absyn_UnknownTunionfield_struct UnknownTunionfield;struct Cyc_Absyn_KnownTunionfield_struct
KnownTunionfield;};struct Cyc_Absyn_TunionFieldInfo{union Cyc_Absyn_TunionFieldInfoU_union
field_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownAggr_struct{int tag;
void*f1;struct _tuple0*f2;};struct Cyc_Absyn_KnownAggr_struct{int tag;struct Cyc_Absyn_Aggrdecl**
f1;};union Cyc_Absyn_AggrInfoU_union{struct Cyc_Absyn_UnknownAggr_struct
UnknownAggr;struct Cyc_Absyn_KnownAggr_struct KnownAggr;};struct Cyc_Absyn_AggrInfo{
union Cyc_Absyn_AggrInfoU_union aggr_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_ArrayInfo{
void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;struct Cyc_Absyn_Conref*
zero_term;struct Cyc_Position_Segment*zt_loc;};struct Cyc_Absyn_Evar_struct{int tag;
struct Cyc_Core_Opt*f1;struct Cyc_Core_Opt*f2;int f3;struct Cyc_Core_Opt*f4;};struct
Cyc_Absyn_VarType_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_TunionType_struct{
int tag;struct Cyc_Absyn_TunionInfo f1;};struct Cyc_Absyn_TunionFieldType_struct{int
tag;struct Cyc_Absyn_TunionFieldInfo f1;};struct Cyc_Absyn_PointerType_struct{int
tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_struct{int tag;void*f1;
void*f2;};struct Cyc_Absyn_DoubleType_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_struct{
int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_struct{int tag;struct
Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_AggrType_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct
Cyc_Absyn_AnonAggrType_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_struct{
int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_SizeofType_struct{int tag;void*f1;
};struct Cyc_Absyn_RgnHandleType_struct{int tag;void*f1;};struct Cyc_Absyn_DynRgnType_struct{
int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_struct{int tag;struct _tuple0*
f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void**f4;};struct Cyc_Absyn_TagType_struct{
int tag;void*f1;};struct Cyc_Absyn_TypeInt_struct{int tag;int f1;};struct Cyc_Absyn_AccessEff_struct{
int tag;void*f1;};struct Cyc_Absyn_JoinEff_struct{int tag;struct Cyc_List_List*f1;};
struct Cyc_Absyn_RgnsEff_struct{int tag;void*f1;};struct Cyc_Absyn_NoTypes_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*f2;};struct Cyc_Absyn_WithTypes_struct{
int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;struct Cyc_Core_Opt*
f4;struct Cyc_List_List*f5;};struct Cyc_Absyn_Regparm_att_struct{int tag;int f1;};
struct Cyc_Absyn_Aligned_att_struct{int tag;int f1;};struct Cyc_Absyn_Section_att_struct{
int tag;struct _dynforward_ptr f1;};struct Cyc_Absyn_Format_att_struct{int tag;void*
f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_struct{int tag;int f1;};struct Cyc_Absyn_Mode_att_struct{
int tag;struct _dynforward_ptr f1;};struct Cyc_Absyn_Carray_mod_struct{int tag;struct
Cyc_Absyn_Conref*f1;struct Cyc_Position_Segment*f2;};struct Cyc_Absyn_ConstArray_mod_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Conref*f2;struct Cyc_Position_Segment*
f3;};struct Cyc_Absyn_Pointer_mod_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct
Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_struct{int tag;void*f1;};struct
Cyc_Absyn_TypeParams_mod_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*
f2;int f3;};struct Cyc_Absyn_Attributes_mod_struct{int tag;struct Cyc_Position_Segment*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Char_c_struct{int tag;void*f1;char f2;
};struct Cyc_Absyn_Short_c_struct{int tag;void*f1;short f2;};struct Cyc_Absyn_Int_c_struct{
int tag;void*f1;int f2;};struct Cyc_Absyn_LongLong_c_struct{int tag;void*f1;
long long f2;};struct Cyc_Absyn_Float_c_struct{int tag;struct _dynforward_ptr f1;};
struct Cyc_Absyn_String_c_struct{int tag;struct _dynforward_ptr f1;};struct Cyc_Absyn_Null_c_struct{
int tag;};union Cyc_Absyn_Cnst_union{struct Cyc_Absyn_Char_c_struct Char_c;struct Cyc_Absyn_Short_c_struct
Short_c;struct Cyc_Absyn_Int_c_struct Int_c;struct Cyc_Absyn_LongLong_c_struct
LongLong_c;struct Cyc_Absyn_Float_c_struct Float_c;struct Cyc_Absyn_String_c_struct
String_c;struct Cyc_Absyn_Null_c_struct Null_c;};struct Cyc_Absyn_VarargCallInfo{
int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};
struct Cyc_Absyn_StructField_struct{int tag;struct _dynforward_ptr*f1;};struct Cyc_Absyn_TupleIndex_struct{
int tag;unsigned int f1;};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*
rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;};struct Cyc_Absyn_Const_e_struct{
int tag;union Cyc_Absyn_Cnst_union f1;};struct Cyc_Absyn_Var_e_struct{int tag;struct
_tuple0*f1;void*f2;};struct Cyc_Absyn_UnknownId_e_struct{int tag;struct _tuple0*f1;
};struct Cyc_Absyn_Primop_e_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct
Cyc_Absyn_AssignOp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;
struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;void*f2;};struct Cyc_Absyn_Conditional_e_struct{int tag;struct Cyc_Absyn_Exp*f1;
struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_struct{int
tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnknownCall_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_FnCall_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*
f3;};struct Cyc_Absyn_Throw_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoInstantiate_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_struct{
int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;void*f4;};struct Cyc_Absyn_Address_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_struct{int tag;void*f1;};
struct Cyc_Absyn_Sizeofexp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_struct{
int tag;void*f1;void*f2;};struct Cyc_Absyn_Gentyp_e_struct{int tag;struct Cyc_List_List*
f1;void*f2;};struct Cyc_Absyn_Deref_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_AggrMember_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
_dynforward_ptr*f2;};struct Cyc_Absyn_AggrArrow_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct _dynforward_ptr*f2;};struct Cyc_Absyn_Subscript_e_struct{int tag;struct
Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_struct{int tag;
struct Cyc_List_List*f1;};struct _tuple1{struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Tqual
f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_struct{int tag;struct _tuple1*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_struct{int tag;struct Cyc_List_List*f1;
};struct Cyc_Absyn_Comprehension_e_struct{int tag;struct Cyc_Absyn_Vardecl*f1;
struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_Struct_e_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*
f4;};struct Cyc_Absyn_AnonStruct_e_struct{int tag;void*f1;struct Cyc_List_List*f2;}
;struct Cyc_Absyn_Tunion_e_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Tuniondecl*
f2;struct Cyc_Absyn_Tunionfield*f3;};struct Cyc_Absyn_Enum_e_struct{int tag;struct
_tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_AnonEnum_e_struct{
int tag;struct _tuple0*f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_Malloc_e_struct{
int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_struct{
int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Exp{struct Cyc_Core_Opt*topt;
void*r;struct Cyc_Position_Segment*loc;void*annot;};struct Cyc_Absyn_Exp_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_IfThenElse_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*
f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple2{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_While_s_struct{int tag;struct _tuple2 f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_Break_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Continue_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Goto_s_struct{int tag;struct
_dynforward_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_For_s_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct _tuple2 f2;struct _tuple2 f3;struct Cyc_Absyn_Stmt*f4;}
;struct Cyc_Absyn_Switch_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Fallthru_s_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**
f2;};struct Cyc_Absyn_Decl_s_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_Label_s_struct{int tag;struct _dynforward_ptr*f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_Do_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple2 f2;
};struct Cyc_Absyn_TryCatch_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Region_s_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*
f2;int f3;struct Cyc_Absyn_Exp*f4;struct Cyc_Absyn_Stmt*f5;};struct Cyc_Absyn_ResetRegion_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Alias_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Tvar*f2;struct Cyc_Absyn_Vardecl*f3;struct Cyc_Absyn_Stmt*f4;};
struct Cyc_Absyn_Stmt{void*r;struct Cyc_Position_Segment*loc;struct Cyc_List_List*
non_local_preds;int try_depth;void*annot;};struct Cyc_Absyn_Var_p_struct{int tag;
struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_Reference_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_struct{
int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_struct{
int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_struct{int tag;
struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_struct{int tag;struct Cyc_Absyn_AggrInfo
f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Tunion_p_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;struct Cyc_List_List*
f3;int f4;};struct Cyc_Absyn_Int_p_struct{int tag;void*f1;int f2;};struct Cyc_Absyn_Char_p_struct{
int tag;char f1;};struct Cyc_Absyn_Float_p_struct{int tag;struct _dynforward_ptr f1;};
struct Cyc_Absyn_Enum_p_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Absyn_AnonEnum_p_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Absyn_UnknownId_p_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;struct Cyc_Core_Opt*
topt;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*
pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*
body;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Global_b_struct{int tag;
struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Param_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Local_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{void*sc;struct
_tuple0*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;
struct Cyc_Core_Opt*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{
void*sc;int is_inline;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
effect;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*
cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;struct Cyc_Core_Opt*
cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;
struct Cyc_List_List*attributes;};struct Cyc_Absyn_Aggrfield{struct _dynforward_ptr*
name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*
attributes;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct
Cyc_List_List*rgn_po;struct Cyc_List_List*fields;};struct Cyc_Absyn_Aggrdecl{void*
kind;void*sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*
impl;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Tunionfield{struct _tuple0*
name;struct Cyc_List_List*typs;struct Cyc_Position_Segment*loc;void*sc;};struct Cyc_Absyn_Tuniondecl{
void*sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int
is_xtunion;int is_flat;};struct Cyc_Absyn_Enumfield{struct _tuple0*name;struct Cyc_Absyn_Exp*
tag;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Enumdecl{void*sc;struct
_tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct
_tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
kind;struct Cyc_Core_Opt*defn;struct Cyc_List_List*atts;};struct Cyc_Absyn_Var_d_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Let_d_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*
f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Letv_d_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_Aggr_d_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct
Cyc_Absyn_Tunion_d_struct{int tag;struct Cyc_Absyn_Tuniondecl*f1;};struct Cyc_Absyn_Enum_d_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_struct{int tag;
struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_struct{int tag;
struct _dynforward_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_struct{int tag;
struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Decl{void*r;
struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_ArrayElement_struct{int tag;
struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_struct{int tag;struct
_dynforward_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[15];int Cyc_Absyn_qvar_cmp(
struct _tuple0*,struct _tuple0*);struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual(struct
Cyc_Position_Segment*);struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(struct Cyc_Position_Segment*);
struct Cyc_Absyn_Conref*Cyc_Absyn_empty_conref();void*Cyc_Absyn_compress_kb(void*);
void*Cyc_Absyn_new_evar(struct Cyc_Core_Opt*k,struct Cyc_Core_Opt*tenv);void*Cyc_Absyn_string_typ(
void*rgn);void*Cyc_Absyn_const_string_typ(void*rgn);void*Cyc_Absyn_dynforward_typ(
void*t,void*rgn,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*zero_term);void*
Cyc_Absyn_array_typ(void*elt_type,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Exp*
num_elts,struct Cyc_Absyn_Conref*zero_term,struct Cyc_Position_Segment*ztloc);
struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(unsigned int,struct Cyc_Position_Segment*);
struct _dynforward_ptr Cyc_Absyn_attribute2string(void*);struct Cyc___cycFILE;
struct Cyc_Cstdio___abstractFILE;struct Cyc_String_pa_struct{int tag;struct
_dynforward_ptr f1;};struct Cyc_Int_pa_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_struct{
int tag;double f1;};struct Cyc_LongDouble_pa_struct{int tag;long double f1;};struct
Cyc_ShortPtr_pa_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_struct{int tag;
unsigned long*f1;};struct Cyc_ShortPtr_sa_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_struct{
int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_struct{
int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_struct{int tag;struct
_dynforward_ptr f1;};struct Cyc_DoublePtr_sa_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_struct{
int tag;float*f1;};struct Cyc_CharPtr_sa_struct{int tag;struct _dynforward_ptr f1;};
extern char Cyc_FileCloseError[19];extern char Cyc_FileOpenError[18];struct Cyc_FileOpenError_struct{
char*tag;struct _dynforward_ptr f1;};struct Cyc_PP_Ppstate;struct Cyc_PP_Out;struct
Cyc_PP_Doc;struct Cyc_Absynpp_Params{int expand_typedefs: 1;int qvar_to_Cids: 1;int
add_cyc_prefix: 1;int to_VC: 1;int decls_first: 1;int rewrite_temp_tvars: 1;int
print_all_tvars: 1;int print_all_kinds: 1;int print_all_effects: 1;int
print_using_stmts: 1;int print_externC_stmts: 1;int print_full_evars: 1;int
print_zeroterm: 1;int generate_line_directives: 1;int use_curr_namespace: 1;struct Cyc_List_List*
curr_namespace;};void Cyc_Absynpp_set_params(struct Cyc_Absynpp_Params*fs);extern
struct Cyc_Absynpp_Params Cyc_Absynpp_tc_params_r;struct _dynforward_ptr Cyc_Absynpp_typ2string(
void*);struct _dynforward_ptr Cyc_Absynpp_qvar2string(struct _tuple0*);struct Cyc_Iter_Iter{
void*env;int(*next)(void*env,void*dest);};int Cyc_Iter_next(struct Cyc_Iter_Iter,
void*);struct Cyc_Set_Set;struct Cyc_Set_Set*Cyc_Set_rempty(struct _RegionHandle*r,
int(*cmp)(void*,void*));struct Cyc_Set_Set*Cyc_Set_rinsert(struct _RegionHandle*r,
struct Cyc_Set_Set*s,void*elt);int Cyc_Set_member(struct Cyc_Set_Set*s,void*elt);
extern char Cyc_Set_Absent[11];struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,
void*);struct _RegionHandle*r;struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[12];
extern char Cyc_Dict_Absent[11];int Cyc_Dict_is_empty(struct Cyc_Dict_Dict d);int Cyc_Dict_member(
struct Cyc_Dict_Dict d,void*k);struct Cyc_Dict_Dict Cyc_Dict_insert(struct Cyc_Dict_Dict
d,void*k,void*v);void*Cyc_Dict_lookup(struct Cyc_Dict_Dict d,void*k);void**Cyc_Dict_lookup_opt(
struct Cyc_Dict_Dict d,void*k);struct Cyc_Dict_Dict Cyc_Dict_rmap_c(struct
_RegionHandle*,void*(*f)(void*,void*),void*env,struct Cyc_Dict_Dict d);struct
_tuple3{void*f1;void*f2;};struct _tuple3*Cyc_Dict_rchoose(struct _RegionHandle*r,
struct Cyc_Dict_Dict d);struct _tuple3*Cyc_Dict_rchoose(struct _RegionHandle*,struct
Cyc_Dict_Dict d);struct Cyc_Dict_Dict Cyc_Dict_rfilter_c(struct _RegionHandle*,int(*
f)(void*,void*,void*),void*env,struct Cyc_Dict_Dict d);struct Cyc_Iter_Iter Cyc_Dict_make_iter(
struct _RegionHandle*rgn,struct Cyc_Dict_Dict d);struct Cyc_RgnOrder_RgnPO;struct Cyc_RgnOrder_RgnPO*
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
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_TunionRes_struct{int tag;
struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Tcenv_EnumRes_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_struct{
int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_Genv{struct
_RegionHandle*grgn;struct Cyc_Set_Set*namespaces;struct Cyc_Dict_Dict aggrdecls;
struct Cyc_Dict_Dict tuniondecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict
typedefs;struct Cyc_Dict_Dict ordinaries;struct Cyc_List_List*availables;};struct
Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Stmt_j_struct{int tag;struct Cyc_Absyn_Stmt*f1;};
struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict ae;struct Cyc_Tcenv_Fenv*
le;};struct Cyc_Tcenv_Genv*Cyc_Tcenv_empty_genv(struct _RegionHandle*);struct Cyc_Tcenv_Fenv*
Cyc_Tcenv_new_fenv(struct _RegionHandle*,struct Cyc_Position_Segment*,struct Cyc_Absyn_Fndecl*);
struct Cyc_List_List*Cyc_Tcenv_resolve_namespace(struct Cyc_Tcenv_Tenv*,struct Cyc_Position_Segment*,
struct _dynforward_ptr*,struct Cyc_List_List*);struct Cyc_Absyn_Tuniondecl***Cyc_Tcenv_lookup_xtuniondecl(
struct _RegionHandle*,struct Cyc_Tcenv_Tenv*,struct Cyc_Position_Segment*,struct
_tuple0*);int Cyc_Tcenv_all_labels_resolved(struct Cyc_Tcenv_Tenv*);void Cyc_Tcenv_check_delayed_effects(
struct Cyc_Tcenv_Tenv*te);void Cyc_Tcenv_check_delayed_constraints(struct Cyc_Tcenv_Tenv*
te);void*Cyc_Tcutil_impos(struct _dynforward_ptr fmt,struct _dynforward_ptr ap);void
Cyc_Tcutil_terr(struct Cyc_Position_Segment*,struct _dynforward_ptr fmt,struct
_dynforward_ptr ap);void Cyc_Tcutil_warn(struct Cyc_Position_Segment*,struct
_dynforward_ptr fmt,struct _dynforward_ptr ap);void*Cyc_Tcutil_compress(void*t);int
Cyc_Tcutil_coerce_assign(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Exp*,void*);int
Cyc_Tcutil_is_function_type(void*t);void*Cyc_Tcutil_kind_to_bound(void*k);void
Cyc_Tcutil_explain_failure();int Cyc_Tcutil_unify(void*,void*);void*Cyc_Tcutil_fndecl2typ(
struct Cyc_Absyn_Fndecl*);void Cyc_Tcutil_check_bitfield(struct Cyc_Position_Segment*
loc,struct Cyc_Tcenv_Tenv*te,void*field_typ,struct Cyc_Absyn_Exp*width,struct
_dynforward_ptr*fn);void Cyc_Tcutil_check_valid_toplevel_type(struct Cyc_Position_Segment*,
struct Cyc_Tcenv_Tenv*,void*);void Cyc_Tcutil_check_fndecl_valid_type(struct Cyc_Position_Segment*,
struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Fndecl*);void Cyc_Tcutil_check_type(struct
Cyc_Position_Segment*,struct Cyc_Tcenv_Tenv*,struct Cyc_List_List*bound_tvars,void*
k,int allow_evars,void*);void Cyc_Tcutil_check_unique_tvars(struct Cyc_Position_Segment*,
struct Cyc_List_List*);int Cyc_Tcutil_is_noalias_pointer_or_aggr(void*t);void Cyc_Tcutil_add_tvar_identities(
struct Cyc_List_List*);int Cyc_Tcutil_bits_only(void*t);int Cyc_Tcutil_is_const_exp(
struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e);int Cyc_Tcutil_supports_default(
void*);int Cyc_Tcutil_extract_const_from_typedef(struct Cyc_Position_Segment*,int
declared_const,void*);struct Cyc_List_List*Cyc_Tcutil_transfer_fn_type_atts(void*
t,struct Cyc_List_List*atts);void*Cyc_Tcexp_tcExpInitializer(struct Cyc_Tcenv_Tenv*,
void**,struct Cyc_Absyn_Exp*);void Cyc_Tcstmt_tcStmt(struct Cyc_Tcenv_Tenv*te,
struct Cyc_Absyn_Stmt*s,int new_block);struct _tuple4{unsigned int f1;int f2;};struct
_tuple4 Cyc_Evexp_eval_const_uint_exp(struct Cyc_Absyn_Exp*e);void Cyc_Tc_tc(struct
_RegionHandle*,struct Cyc_Tcenv_Tenv*te,int var_default_init,struct Cyc_List_List*
ds);struct Cyc_List_List*Cyc_Tc_treeshake(struct Cyc_Tcenv_Tenv*te,struct Cyc_List_List*);
void Cyc_Tc_tcAggrdecl(struct Cyc_Tcenv_Tenv*,struct Cyc_Tcenv_Genv*,struct Cyc_Position_Segment*,
struct Cyc_Absyn_Aggrdecl*);void Cyc_Tc_tcTuniondecl(struct Cyc_Tcenv_Tenv*,struct
Cyc_Tcenv_Genv*,struct Cyc_Position_Segment*,struct Cyc_Absyn_Tuniondecl*);void Cyc_Tc_tcEnumdecl(
struct Cyc_Tcenv_Tenv*,struct Cyc_Tcenv_Genv*,struct Cyc_Position_Segment*,struct
Cyc_Absyn_Enumdecl*);extern char Cyc_Tcdecl_Incompatible[17];struct Cyc_Tcdecl_Xtunionfielddecl{
struct Cyc_Absyn_Tuniondecl*base;struct Cyc_Absyn_Tunionfield*field;};struct Cyc_Absyn_Aggrdecl*
Cyc_Tcdecl_merge_aggrdecl(struct Cyc_Absyn_Aggrdecl*d0,struct Cyc_Absyn_Aggrdecl*
d1,struct Cyc_Position_Segment*loc,struct _dynforward_ptr*msg);struct Cyc_Absyn_Tuniondecl*
Cyc_Tcdecl_merge_tuniondecl(struct Cyc_Absyn_Tuniondecl*d0,struct Cyc_Absyn_Tuniondecl*
d1,struct Cyc_Position_Segment*loc,struct _dynforward_ptr*msg);struct Cyc_Absyn_Enumdecl*
Cyc_Tcdecl_merge_enumdecl(struct Cyc_Absyn_Enumdecl*d0,struct Cyc_Absyn_Enumdecl*
d1,struct Cyc_Position_Segment*loc,struct _dynforward_ptr*msg);void*Cyc_Tcdecl_merge_binding(
void*d0,void*d1,struct Cyc_Position_Segment*loc,struct _dynforward_ptr*msg);struct
Cyc_List_List*Cyc_Tcdecl_sort_xtunion_fields(struct Cyc_List_List*f,int*res,
struct _dynforward_ptr*v,struct Cyc_Position_Segment*loc,struct _dynforward_ptr*msg);
struct Cyc_Tcgenrep_RepInfo;struct Cyc_Dict_Dict Cyc_Tcgenrep_empty_typerep_dict();
struct _tuple5{struct Cyc_Dict_Dict f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Exp*
f3;};struct _tuple5 Cyc_Tcgenrep_tcGenrep(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*
ge,struct Cyc_Position_Segment*loc,void*type,struct Cyc_Dict_Dict dict);static char
_tmp0[1]="";static struct _dynforward_ptr Cyc_Tc_tc_msg_c={_tmp0,_tmp0 + 1};static
struct _dynforward_ptr*Cyc_Tc_tc_msg=(struct _dynforward_ptr*)& Cyc_Tc_tc_msg_c;
struct _tuple6{struct Cyc_Position_Segment*f1;struct _tuple0*f2;int f3;};static int
Cyc_Tc_export_member(struct _tuple0*x,struct Cyc_List_List*exports){for(0;exports
!= 0;exports=exports->tl){struct _tuple6*_tmp1=(struct _tuple6*)exports->hd;if(Cyc_Absyn_qvar_cmp(
x,(*_tmp1).f2)== 0){(*_tmp1).f3=1;return 1;}}return 0;}struct _tuple7{void*f1;int f2;
};static void Cyc_Tc_tcVardecl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,
struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Vardecl*vd,int check_var_init,int
ignore_init,struct Cyc_List_List**exports){struct Cyc_Absyn_Vardecl _tmp3;void*
_tmp4;struct _tuple0*_tmp5;struct _tuple0 _tmp6;union Cyc_Absyn_Nmspace_union _tmp7;
struct _dynforward_ptr*_tmp8;void*_tmp9;struct Cyc_Absyn_Exp*_tmpA;struct Cyc_List_List*
_tmpB;struct Cyc_Absyn_Vardecl*_tmp2=vd;_tmp3=*_tmp2;_tmp4=(void*)_tmp3.sc;_tmp5=
_tmp3.name;_tmp6=*_tmp5;_tmp7=_tmp6.f1;_tmp8=_tmp6.f2;_tmp9=(void*)_tmp3.type;
_tmpA=_tmp3.initializer;_tmpB=_tmp3.attributes;{union Cyc_Absyn_Nmspace_union
_tmpC=_tmp7;struct Cyc_List_List*_tmpD;struct Cyc_List_List*_tmpE;_LL1: if((_tmpC.Rel_n).tag
!= 1)goto _LL3;_tmpD=(_tmpC.Rel_n).f1;if(_tmpD != 0)goto _LL3;_LL2: goto _LL4;_LL3:
if((_tmpC.Abs_n).tag != 2)goto _LL5;_tmpE=(_tmpC.Abs_n).f1;if(_tmpE != 0)goto _LL5;
_LL4: goto _LL0;_LL5:;_LL6:({struct Cyc_String_pa_struct _tmp11;_tmp11.tag=0;_tmp11.f1=(
struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_qvar2string(vd->name));{
void*_tmpF[1]={& _tmp11};Cyc_Tcutil_terr(loc,({const char*_tmp10="qualified variable declarations are not implemented (%s)";
_tag_dynforward(_tmp10,sizeof(char),_get_zero_arr_size(_tmp10,57));}),
_tag_dynforward(_tmpF,sizeof(void*),1));}});return;_LL0:;}(*vd->name).f1=(union
Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union _tmp12;(_tmp12.Abs_n).tag=
2;(_tmp12.Abs_n).f1=te->ns;_tmp12;});{void*_tmp13=Cyc_Tcutil_compress(_tmp9);
struct Cyc_Absyn_ArrayInfo _tmp14;void*_tmp15;struct Cyc_Absyn_Tqual _tmp16;struct
Cyc_Absyn_Exp*_tmp17;struct Cyc_Absyn_Conref*_tmp18;struct Cyc_Position_Segment*
_tmp19;_LL8: if(_tmp13 <= (void*)4)goto _LLA;if(*((int*)_tmp13)!= 7)goto _LLA;_tmp14=((
struct Cyc_Absyn_ArrayType_struct*)_tmp13)->f1;_tmp15=(void*)_tmp14.elt_type;
_tmp16=_tmp14.tq;_tmp17=_tmp14.num_elts;if(_tmp17 != 0)goto _LLA;_tmp18=_tmp14.zero_term;
_tmp19=_tmp14.zt_loc;if(!(_tmpA != 0))goto _LLA;_LL9:{void*_tmp1A=(void*)_tmpA->r;
union Cyc_Absyn_Cnst_union _tmp1B;struct _dynforward_ptr _tmp1C;struct Cyc_Absyn_Exp*
_tmp1D;struct Cyc_List_List*_tmp1E;struct Cyc_List_List*_tmp1F;_LLD: if(*((int*)
_tmp1A)!= 0)goto _LLF;_tmp1B=((struct Cyc_Absyn_Const_e_struct*)_tmp1A)->f1;if(((((
struct Cyc_Absyn_Const_e_struct*)_tmp1A)->f1).String_c).tag != 5)goto _LLF;_tmp1C=(
_tmp1B.String_c).f1;_LLE: _tmp9=(void*)(vd->type=(void*)Cyc_Absyn_array_typ(
_tmp15,_tmp16,(struct Cyc_Absyn_Exp*)Cyc_Absyn_uint_exp(_get_dynforward_size(
_tmp1C,sizeof(char)),0),_tmp18,_tmp19));goto _LLC;_LLF: if(*((int*)_tmp1A)!= 29)
goto _LL11;_tmp1D=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp1A)->f2;_LL10:
_tmp9=(void*)(vd->type=(void*)Cyc_Absyn_array_typ(_tmp15,_tmp16,(struct Cyc_Absyn_Exp*)
_tmp1D,_tmp18,_tmp19));goto _LLC;_LL11: if(*((int*)_tmp1A)!= 37)goto _LL13;_tmp1E=((
struct Cyc_Absyn_UnresolvedMem_e_struct*)_tmp1A)->f2;_LL12: _tmp1F=_tmp1E;goto
_LL14;_LL13: if(*((int*)_tmp1A)!= 28)goto _LL15;_tmp1F=((struct Cyc_Absyn_Array_e_struct*)
_tmp1A)->f1;_LL14: _tmp9=(void*)(vd->type=(void*)Cyc_Absyn_array_typ(_tmp15,
_tmp16,(struct Cyc_Absyn_Exp*)Cyc_Absyn_uint_exp((unsigned int)((int(*)(struct Cyc_List_List*
x))Cyc_List_length)(_tmp1F),0),_tmp18,_tmp19));goto _LLC;_LL15:;_LL16: goto _LLC;
_LLC:;}goto _LL7;_LLA:;_LLB: goto _LL7;_LL7:;}Cyc_Tcutil_check_valid_toplevel_type(
loc,te,_tmp9);(vd->tq).real_const=Cyc_Tcutil_extract_const_from_typedef(loc,(vd->tq).print_const,
_tmp9);{void*_tmp20=Cyc_Tcutil_compress(_tmp9);_LL18: if(_tmp20 <= (void*)4)goto
_LL1A;if(*((int*)_tmp20)!= 7)goto _LL1A;_LL19: vd->escapes=0;goto _LL17;_LL1A:;
_LL1B: vd->escapes=1;goto _LL17;_LL17:;}if(Cyc_Tcutil_is_function_type(_tmp9))
_tmpB=Cyc_Tcutil_transfer_fn_type_atts(_tmp9,_tmpB);if(_tmp4 == (void*)3  || _tmp4
== (void*)4){if(_tmpA != 0)({void*_tmp21[0]={};Cyc_Tcutil_terr(loc,({const char*
_tmp22="extern declaration should not have initializer";_tag_dynforward(_tmp22,
sizeof(char),_get_zero_arr_size(_tmp22,47));}),_tag_dynforward(_tmp21,sizeof(
void*),0));});}else{if(!Cyc_Tcutil_is_function_type(_tmp9)){for(0;_tmpB != 0;
_tmpB=_tmpB->tl){void*_tmp23=(void*)_tmpB->hd;_LL1D: if(_tmp23 <= (void*)17)goto
_LL21;if(*((int*)_tmp23)!= 1)goto _LL1F;_LL1E: goto _LL20;_LL1F: if(*((int*)_tmp23)
!= 2)goto _LL21;_LL20: goto _LL22;_LL21: if((int)_tmp23 != 6)goto _LL23;_LL22: goto
_LL24;_LL23: if((int)_tmp23 != 7)goto _LL25;_LL24: goto _LL26;_LL25: if((int)_tmp23 != 
8)goto _LL27;_LL26: goto _LL28;_LL27: if((int)_tmp23 != 9)goto _LL29;_LL28: goto _LL2A;
_LL29: if((int)_tmp23 != 10)goto _LL2B;_LL2A: goto _LL2C;_LL2B: if((int)_tmp23 != 11)
goto _LL2D;_LL2C: continue;_LL2D:;_LL2E:({struct Cyc_String_pa_struct _tmp27;_tmp27.tag=
0;_tmp27.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_qvar2string(
vd->name));{struct Cyc_String_pa_struct _tmp26;_tmp26.tag=0;_tmp26.f1=(struct
_dynforward_ptr)((struct _dynforward_ptr)Cyc_Absyn_attribute2string((void*)_tmpB->hd));{
void*_tmp24[2]={& _tmp26,& _tmp27};Cyc_Tcutil_terr(loc,({const char*_tmp25="bad attribute %s for variable %s";
_tag_dynforward(_tmp25,sizeof(char),_get_zero_arr_size(_tmp25,33));}),
_tag_dynforward(_tmp24,sizeof(void*),2));}}});goto _LL1C;_LL1C:;}if(_tmpA == 0  || 
ignore_init){if(check_var_init  && !Cyc_Tcutil_supports_default(_tmp9))({struct
Cyc_String_pa_struct _tmp2B;_tmp2B.tag=0;_tmp2B.f1=(struct _dynforward_ptr)((
struct _dynforward_ptr)Cyc_Absynpp_typ2string(_tmp9));{struct Cyc_String_pa_struct
_tmp2A;_tmp2A.tag=0;_tmp2A.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_qvar2string(
vd->name));{void*_tmp28[2]={& _tmp2A,& _tmp2B};Cyc_Tcutil_terr(loc,({const char*
_tmp29="initializer required for variable %s of type %s";_tag_dynforward(_tmp29,
sizeof(char),_get_zero_arr_size(_tmp29,48));}),_tag_dynforward(_tmp28,sizeof(
void*),2));}}});}else{struct Cyc_Absyn_Exp*_tmp2C=(struct Cyc_Absyn_Exp*)_tmpA;
void*_tmp2D=Cyc_Tcexp_tcExpInitializer(te,(void**)& _tmp9,_tmp2C);if(!Cyc_Tcutil_coerce_assign(
te,_tmp2C,_tmp9)){({struct Cyc_String_pa_struct _tmp32;_tmp32.tag=0;_tmp32.f1=(
struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_typ2string(_tmp2D));{
struct Cyc_String_pa_struct _tmp31;_tmp31.tag=0;_tmp31.f1=(struct _dynforward_ptr)((
struct _dynforward_ptr)Cyc_Absynpp_typ2string(_tmp9));{struct Cyc_String_pa_struct
_tmp30;_tmp30.tag=0;_tmp30.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_qvar2string(
vd->name));{void*_tmp2E[3]={& _tmp30,& _tmp31,& _tmp32};Cyc_Tcutil_terr(loc,({const
char*_tmp2F="%s declared with type \n%s\n but initialized with type \n%s";
_tag_dynforward(_tmp2F,sizeof(char),_get_zero_arr_size(_tmp2F,57));}),
_tag_dynforward(_tmp2E,sizeof(void*),3));}}}});Cyc_Tcutil_explain_failure();}if(
!Cyc_Tcutil_is_const_exp(te,_tmp2C))({void*_tmp33[0]={};Cyc_Tcutil_terr(loc,({
const char*_tmp34="initializer is not a constant expression";_tag_dynforward(
_tmp34,sizeof(char),_get_zero_arr_size(_tmp34,41));}),_tag_dynforward(_tmp33,
sizeof(void*),0));});}}else{for(0;_tmpB != 0;_tmpB=_tmpB->tl){void*_tmp35=(void*)
_tmpB->hd;_LL30: if(_tmp35 <= (void*)17)goto _LL32;if(*((int*)_tmp35)!= 0)goto _LL32;
_LL31: goto _LL33;_LL32: if((int)_tmp35 != 0)goto _LL34;_LL33: goto _LL35;_LL34: if((int)
_tmp35 != 1)goto _LL36;_LL35: goto _LL37;_LL36: if((int)_tmp35 != 2)goto _LL38;_LL37:
goto _LL39;_LL38: if((int)_tmp35 != 3)goto _LL3A;_LL39: goto _LL3B;_LL3A: if(_tmp35 <= (
void*)17)goto _LL3E;if(*((int*)_tmp35)!= 3)goto _LL3C;_LL3B: goto _LL3D;_LL3C: if(*((
int*)_tmp35)!= 4)goto _LL3E;_LL3D: goto _LL3F;_LL3E: if((int)_tmp35 != 16)goto _LL40;
_LL3F: goto _LL41;_LL40: if((int)_tmp35 != 4)goto _LL42;_LL41:({void*_tmp36[0]={};((
int(*)(struct _dynforward_ptr fmt,struct _dynforward_ptr ap))Cyc_Tcutil_impos)(({
const char*_tmp37="tcVardecl: fn type atts in function var decl";_tag_dynforward(
_tmp37,sizeof(char),_get_zero_arr_size(_tmp37,45));}),_tag_dynforward(_tmp36,
sizeof(void*),0));});_LL42: if(_tmp35 <= (void*)17)goto _LL44;if(*((int*)_tmp35)!= 
1)goto _LL44;_LL43: goto _LL45;_LL44: if((int)_tmp35 != 5)goto _LL46;_LL45:({struct Cyc_String_pa_struct
_tmp3A;_tmp3A.tag=0;_tmp3A.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absyn_attribute2string((
void*)_tmpB->hd));{void*_tmp38[1]={& _tmp3A};Cyc_Tcutil_terr(loc,({const char*
_tmp39="bad attribute %s in function declaration";_tag_dynforward(_tmp39,sizeof(
char),_get_zero_arr_size(_tmp39,41));}),_tag_dynforward(_tmp38,sizeof(void*),1));}});
goto _LL2F;_LL46:;_LL47: continue;_LL2F:;}}}{struct _handler_cons _tmp3B;
_push_handler(& _tmp3B);{int _tmp3D=0;if(setjmp(_tmp3B.handler))_tmp3D=1;if(!
_tmp3D){{struct _tuple7*_tmp3E=((struct _tuple7*(*)(struct Cyc_Dict_Dict d,struct
_dynforward_ptr*k))Cyc_Dict_lookup)(ge->ordinaries,_tmp8);void*_tmp3F=(*_tmp3E).f1;
void*_tmp40;_LL49: if(*((int*)_tmp3F)!= 0)goto _LL4B;_tmp40=(void*)((struct Cyc_Tcenv_VarRes_struct*)
_tmp3F)->f1;_LL4A: {struct Cyc_Absyn_Global_b_struct*_tmp41=({struct Cyc_Absyn_Global_b_struct*
_tmp46=_cycalloc(sizeof(*_tmp46));_tmp46[0]=({struct Cyc_Absyn_Global_b_struct
_tmp47;_tmp47.tag=0;_tmp47.f1=vd;_tmp47;});_tmp46;});void*_tmp42=Cyc_Tcdecl_merge_binding(
_tmp40,(void*)_tmp41,loc,Cyc_Tc_tc_msg);if(_tmp42 == (void*)0){_npop_handler(0);
return;}if(_tmp42 == _tmp40  && (*_tmp3E).f2){_npop_handler(0);return;}if(exports
== 0  || Cyc_Tc_export_member(vd->name,*exports))ge->ordinaries=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct _tuple7*v))Cyc_Dict_insert)(
ge->ordinaries,_tmp8,(struct _tuple7*)({struct _tuple7*_tmp43=_cycalloc(sizeof(*
_tmp43));_tmp43->f1=(void*)({struct Cyc_Tcenv_VarRes_struct*_tmp44=_cycalloc(
sizeof(*_tmp44));_tmp44[0]=({struct Cyc_Tcenv_VarRes_struct _tmp45;_tmp45.tag=0;
_tmp45.f1=(void*)_tmp42;_tmp45;});_tmp44;});_tmp43->f2=1;_tmp43;}));
_npop_handler(0);return;}_LL4B: if(*((int*)_tmp3F)!= 1)goto _LL4D;_LL4C:({void*
_tmp48[0]={};Cyc_Tcutil_warn(loc,({const char*_tmp49="variable declaration shadows previous struct declaration";
_tag_dynforward(_tmp49,sizeof(char),_get_zero_arr_size(_tmp49,57));}),
_tag_dynforward(_tmp48,sizeof(void*),0));});goto _LL48;_LL4D: if(*((int*)_tmp3F)!= 
2)goto _LL4F;_LL4E:({void*_tmp4A[0]={};Cyc_Tcutil_warn(loc,({const char*_tmp4B="variable declaration shadows previous [x]tunion constructor";
_tag_dynforward(_tmp4B,sizeof(char),_get_zero_arr_size(_tmp4B,60));}),
_tag_dynforward(_tmp4A,sizeof(void*),0));});goto _LL48;_LL4F: if(*((int*)_tmp3F)!= 
4)goto _LL51;_LL50: goto _LL52;_LL51: if(*((int*)_tmp3F)!= 3)goto _LL48;_LL52:({void*
_tmp4C[0]={};Cyc_Tcutil_warn(loc,({const char*_tmp4D="variable declaration shadows previous enum tag";
_tag_dynforward(_tmp4D,sizeof(char),_get_zero_arr_size(_tmp4D,47));}),
_tag_dynforward(_tmp4C,sizeof(void*),0));});goto _LL48;_LL48:;};_pop_handler();}
else{void*_tmp3C=(void*)_exn_thrown;void*_tmp4F=_tmp3C;_LL54: if(_tmp4F != Cyc_Dict_Absent)
goto _LL56;_LL55: goto _LL53;_LL56:;_LL57:(void)_throw(_tmp4F);_LL53:;}}}if(exports
== 0  || Cyc_Tc_export_member(vd->name,*exports))ge->ordinaries=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct _tuple7*v))Cyc_Dict_insert)(
ge->ordinaries,_tmp8,(struct _tuple7*)({struct _tuple7*_tmp50=_cycalloc(sizeof(*
_tmp50));_tmp50->f1=(void*)({struct Cyc_Tcenv_VarRes_struct*_tmp51=_cycalloc(
sizeof(*_tmp51));_tmp51[0]=({struct Cyc_Tcenv_VarRes_struct _tmp52;_tmp52.tag=0;
_tmp52.f1=(void*)((void*)({struct Cyc_Absyn_Global_b_struct*_tmp53=_cycalloc(
sizeof(*_tmp53));_tmp53[0]=({struct Cyc_Absyn_Global_b_struct _tmp54;_tmp54.tag=0;
_tmp54.f1=vd;_tmp54;});_tmp53;}));_tmp52;});_tmp51;});_tmp50->f2=0;_tmp50;}));}
static int Cyc_Tc_is_main(struct _tuple0*n){struct _tuple0 _tmp56;union Cyc_Absyn_Nmspace_union
_tmp57;struct _dynforward_ptr*_tmp58;struct _tuple0*_tmp55=n;_tmp56=*_tmp55;_tmp57=
_tmp56.f1;_tmp58=_tmp56.f2;{union Cyc_Absyn_Nmspace_union _tmp59=_tmp57;struct Cyc_List_List*
_tmp5A;_LL59: if((_tmp59.Abs_n).tag != 2)goto _LL5B;_tmp5A=(_tmp59.Abs_n).f1;if(
_tmp5A != 0)goto _LL5B;_LL5A: return Cyc_strcmp((struct _dynforward_ptr)*_tmp58,({
const char*_tmp5B="main";_tag_dynforward(_tmp5B,sizeof(char),_get_zero_arr_size(
_tmp5B,5));}))== 0;_LL5B:;_LL5C: return 0;_LL58:;}}struct _tuple8{struct
_dynforward_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};static void Cyc_Tc_tcFndecl(
struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,
struct Cyc_Absyn_Fndecl*fd,int ignore_body,struct Cyc_List_List**exports){struct
_dynforward_ptr*v=(*fd->name).f2;if((void*)fd->sc == (void*)4  && !ignore_body)({
void*_tmp5C[0]={};Cyc_Tcutil_terr(loc,({const char*_tmp5D="extern \"C\" functions cannot be implemented in Cyclone";
_tag_dynforward(_tmp5D,sizeof(char),_get_zero_arr_size(_tmp5D,54));}),
_tag_dynforward(_tmp5C,sizeof(void*),0));});{union Cyc_Absyn_Nmspace_union _tmp5E=(*
fd->name).f1;struct Cyc_List_List*_tmp5F;struct Cyc_List_List*_tmp60;_LL5E: if((
_tmp5E.Rel_n).tag != 1)goto _LL60;_tmp5F=(_tmp5E.Rel_n).f1;if(_tmp5F != 0)goto _LL60;
_LL5F: goto _LL5D;_LL60: if((_tmp5E.Abs_n).tag != 2)goto _LL62;_tmp60=(_tmp5E.Abs_n).f1;
_LL61:({void*_tmp61[0]={};((int(*)(struct _dynforward_ptr fmt,struct
_dynforward_ptr ap))Cyc_Tcutil_impos)(({const char*_tmp62="tc: Abs_n in tcFndecl";
_tag_dynforward(_tmp62,sizeof(char),_get_zero_arr_size(_tmp62,22));}),
_tag_dynforward(_tmp61,sizeof(void*),0));});_LL62:;_LL63:({struct Cyc_String_pa_struct
_tmp65;_tmp65.tag=0;_tmp65.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_qvar2string(
fd->name));{void*_tmp63[1]={& _tmp65};Cyc_Tcutil_terr(loc,({const char*_tmp64="qualified function declarations are not implemented (%s)";
_tag_dynforward(_tmp64,sizeof(char),_get_zero_arr_size(_tmp64,57));}),
_tag_dynforward(_tmp63,sizeof(void*),1));}});return;_LL5D:;}(*fd->name).f1=(
union Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union _tmp66;(_tmp66.Abs_n).tag=
2;(_tmp66.Abs_n).f1=te->ns;_tmp66;});Cyc_Tcutil_check_fndecl_valid_type(loc,te,
fd);{void*t=Cyc_Tcutil_fndecl2typ(fd);fd->attributes=Cyc_Tcutil_transfer_fn_type_atts(
t,fd->attributes);{struct Cyc_List_List*atts=fd->attributes;for(0;atts != 0;atts=
atts->tl){void*_tmp67=(void*)atts->hd;_LL65: if((int)_tmp67 != 5)goto _LL67;_LL66:
goto _LL68;_LL67: if(_tmp67 <= (void*)17)goto _LL69;if(*((int*)_tmp67)!= 1)goto _LL69;
_LL68:({struct Cyc_String_pa_struct _tmp6A;_tmp6A.tag=0;_tmp6A.f1=(struct
_dynforward_ptr)((struct _dynforward_ptr)Cyc_Absyn_attribute2string((void*)atts->hd));{
void*_tmp68[1]={& _tmp6A};Cyc_Tcutil_terr(loc,({const char*_tmp69="bad attribute %s for function";
_tag_dynforward(_tmp69,sizeof(char),_get_zero_arr_size(_tmp69,30));}),
_tag_dynforward(_tmp68,sizeof(void*),1));}});goto _LL64;_LL69:;_LL6A: goto _LL64;
_LL64:;}}{struct _handler_cons _tmp6B;_push_handler(& _tmp6B);{int _tmp6D=0;if(
setjmp(_tmp6B.handler))_tmp6D=1;if(!_tmp6D){{struct _tuple7*_tmp6E=((struct
_tuple7*(*)(struct Cyc_Dict_Dict d,struct _dynforward_ptr*k))Cyc_Dict_lookup)(ge->ordinaries,
v);void*_tmp6F=(*_tmp6E).f1;void*_tmp70;_LL6C: if(*((int*)_tmp6F)!= 0)goto _LL6E;
_tmp70=(void*)((struct Cyc_Tcenv_VarRes_struct*)_tmp6F)->f1;_LL6D: {struct Cyc_Absyn_Funname_b_struct*
_tmp71=({struct Cyc_Absyn_Funname_b_struct*_tmp76=_cycalloc(sizeof(*_tmp76));
_tmp76[0]=({struct Cyc_Absyn_Funname_b_struct _tmp77;_tmp77.tag=1;_tmp77.f1=fd;
_tmp77;});_tmp76;});void*_tmp72=Cyc_Tcdecl_merge_binding(_tmp70,(void*)_tmp71,
loc,Cyc_Tc_tc_msg);if(_tmp72 == (void*)0)goto _LL6B;if(_tmp72 == _tmp70  && (*_tmp6E).f2)
goto _LL6B;if(exports == 0  || Cyc_Tc_export_member(fd->name,*exports))ge->ordinaries=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct
_tuple7*v))Cyc_Dict_insert)(ge->ordinaries,v,(struct _tuple7*)({struct _tuple7*
_tmp73=_cycalloc(sizeof(*_tmp73));_tmp73->f1=(void*)({struct Cyc_Tcenv_VarRes_struct*
_tmp74=_cycalloc(sizeof(*_tmp74));_tmp74[0]=({struct Cyc_Tcenv_VarRes_struct
_tmp75;_tmp75.tag=0;_tmp75.f1=(void*)_tmp72;_tmp75;});_tmp74;});_tmp73->f2=1;
_tmp73;}));goto _LL6B;}_LL6E: if(*((int*)_tmp6F)!= 1)goto _LL70;_LL6F:({void*_tmp78[
0]={};Cyc_Tcutil_warn(loc,({const char*_tmp79="function declaration shadows previous type declaration";
_tag_dynforward(_tmp79,sizeof(char),_get_zero_arr_size(_tmp79,55));}),
_tag_dynforward(_tmp78,sizeof(void*),0));});goto _LL6B;_LL70: if(*((int*)_tmp6F)!= 
2)goto _LL72;_LL71:({void*_tmp7A[0]={};Cyc_Tcutil_warn(loc,({const char*_tmp7B="function declaration shadows previous [x]tunion constructor";
_tag_dynforward(_tmp7B,sizeof(char),_get_zero_arr_size(_tmp7B,60));}),
_tag_dynforward(_tmp7A,sizeof(void*),0));});goto _LL6B;_LL72: if(*((int*)_tmp6F)!= 
4)goto _LL74;_LL73: goto _LL75;_LL74: if(*((int*)_tmp6F)!= 3)goto _LL6B;_LL75:({void*
_tmp7C[0]={};Cyc_Tcutil_warn(loc,({const char*_tmp7D="function declaration shadows previous enum tag";
_tag_dynforward(_tmp7D,sizeof(char),_get_zero_arr_size(_tmp7D,47));}),
_tag_dynforward(_tmp7C,sizeof(void*),0));});goto _LL6B;_LL6B:;};_pop_handler();}
else{void*_tmp6C=(void*)_exn_thrown;void*_tmp7F=_tmp6C;_LL77: if(_tmp7F != Cyc_Dict_Absent)
goto _LL79;_LL78: if(exports == 0  || Cyc_Tc_export_member(fd->name,*exports))ge->ordinaries=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct
_tuple7*v))Cyc_Dict_insert)(ge->ordinaries,v,(struct _tuple7*)({struct _tuple7*
_tmp80=_cycalloc(sizeof(*_tmp80));_tmp80->f1=(void*)({struct Cyc_Tcenv_VarRes_struct*
_tmp81=_cycalloc(sizeof(*_tmp81));_tmp81[0]=({struct Cyc_Tcenv_VarRes_struct
_tmp82;_tmp82.tag=0;_tmp82.f1=(void*)((void*)({struct Cyc_Absyn_Funname_b_struct*
_tmp83=_cycalloc(sizeof(*_tmp83));_tmp83[0]=({struct Cyc_Absyn_Funname_b_struct
_tmp84;_tmp84.tag=1;_tmp84.f1=fd;_tmp84;});_tmp83;}));_tmp82;});_tmp81;});_tmp80->f2=
0;_tmp80;}));goto _LL76;_LL79:;_LL7A:(void)_throw(_tmp7F);_LL76:;}}}if(
ignore_body)return;{struct _RegionHandle _tmp85=_new_region("fnrgn");struct
_RegionHandle*fnrgn=& _tmp85;_push_region(fnrgn);{struct Cyc_Tcenv_Fenv*_tmp86=Cyc_Tcenv_new_fenv(
fnrgn,loc,fd);struct Cyc_Tcenv_Tenv*_tmp87=({struct Cyc_Tcenv_Tenv*_tmp8A=
_region_malloc(fnrgn,sizeof(*_tmp8A));_tmp8A->ns=te->ns;_tmp8A->ae=te->ae;_tmp8A->le=(
struct Cyc_Tcenv_Fenv*)_tmp86;_tmp8A;});Cyc_Tcstmt_tcStmt(_tmp87,fd->body,0);Cyc_Tcenv_check_delayed_effects(
_tmp87);Cyc_Tcenv_check_delayed_constraints(_tmp87);if(!Cyc_Tcenv_all_labels_resolved(
_tmp87))({void*_tmp88[0]={};Cyc_Tcutil_terr(loc,({const char*_tmp89="function has goto statements to undefined labels";
_tag_dynforward(_tmp89,sizeof(char),_get_zero_arr_size(_tmp89,49));}),
_tag_dynforward(_tmp88,sizeof(void*),0));});};_pop_region(fnrgn);}if(Cyc_Tc_is_main(
fd->name)){{void*_tmp8B=Cyc_Tcutil_compress((void*)fd->ret_type);void*_tmp8C;
_LL7C: if((int)_tmp8B != 0)goto _LL7E;_LL7D:({void*_tmp8D[0]={};Cyc_Tcutil_warn(loc,({
const char*_tmp8E="main declared with return type void";_tag_dynforward(_tmp8E,
sizeof(char),_get_zero_arr_size(_tmp8E,36));}),_tag_dynforward(_tmp8D,sizeof(
void*),0));});goto _LL7B;_LL7E: if(_tmp8B <= (void*)4)goto _LL80;if(*((int*)_tmp8B)
!= 5)goto _LL80;_tmp8C=(void*)((struct Cyc_Absyn_IntType_struct*)_tmp8B)->f2;_LL7F:
goto _LL7B;_LL80:;_LL81:({struct Cyc_String_pa_struct _tmp91;_tmp91.tag=0;_tmp91.f1=(
struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_typ2string((void*)fd->ret_type));{
void*_tmp8F[1]={& _tmp91};Cyc_Tcutil_terr(loc,({const char*_tmp90="main declared with return type %s instead of int or void";
_tag_dynforward(_tmp90,sizeof(char),_get_zero_arr_size(_tmp90,57));}),
_tag_dynforward(_tmp8F,sizeof(void*),1));}});goto _LL7B;_LL7B:;}if(fd->c_varargs
 || fd->cyc_varargs != 0)({void*_tmp92[0]={};Cyc_Tcutil_terr(loc,({const char*
_tmp93="main declared with varargs";_tag_dynforward(_tmp93,sizeof(char),
_get_zero_arr_size(_tmp93,27));}),_tag_dynforward(_tmp92,sizeof(void*),0));});{
struct Cyc_List_List*_tmp94=fd->args;if(_tmp94 != 0){struct _tuple8 _tmp96;void*
_tmp97;struct _tuple8*_tmp95=(struct _tuple8*)_tmp94->hd;_tmp96=*_tmp95;_tmp97=
_tmp96.f3;{void*_tmp98=Cyc_Tcutil_compress(_tmp97);void*_tmp99;_LL83: if(_tmp98 <= (
void*)4)goto _LL85;if(*((int*)_tmp98)!= 5)goto _LL85;_tmp99=(void*)((struct Cyc_Absyn_IntType_struct*)
_tmp98)->f2;_LL84: goto _LL82;_LL85:;_LL86:({struct Cyc_String_pa_struct _tmp9C;
_tmp9C.tag=0;_tmp9C.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_typ2string(
_tmp97));{void*_tmp9A[1]={& _tmp9C};Cyc_Tcutil_terr(loc,({const char*_tmp9B="main declared with first argument of type %s instead of int";
_tag_dynforward(_tmp9B,sizeof(char),_get_zero_arr_size(_tmp9B,60));}),
_tag_dynforward(_tmp9A,sizeof(void*),1));}});goto _LL82;_LL82:;}_tmp94=_tmp94->tl;
if(_tmp94 != 0){struct _tuple8 _tmp9E;void*_tmp9F;struct _tuple8*_tmp9D=(struct
_tuple8*)_tmp94->hd;_tmp9E=*_tmp9D;_tmp9F=_tmp9E.f3;_tmp94=_tmp94->tl;if(_tmp94
!= 0)({void*_tmpA0[0]={};Cyc_Tcutil_terr(loc,({const char*_tmpA1="main declared with too many arguments";
_tag_dynforward(_tmpA1,sizeof(char),_get_zero_arr_size(_tmpA1,38));}),
_tag_dynforward(_tmpA0,sizeof(void*),0));});{struct Cyc_Core_Opt*tvs=({struct Cyc_Core_Opt*
_tmpAD=_cycalloc(sizeof(*_tmpAD));_tmpAD->v=fd->tvs;_tmpAD;});if(((!Cyc_Tcutil_unify(
_tmp9F,Cyc_Absyn_dynforward_typ(Cyc_Absyn_string_typ(Cyc_Absyn_new_evar(({struct
Cyc_Core_Opt*_tmpA2=_cycalloc(sizeof(*_tmpA2));_tmpA2->v=(void*)((void*)3);
_tmpA2;}),tvs)),Cyc_Absyn_new_evar(({struct Cyc_Core_Opt*_tmpA3=_cycalloc(sizeof(*
_tmpA3));_tmpA3->v=(void*)((void*)3);_tmpA3;}),tvs),Cyc_Absyn_empty_tqual(0),((
struct Cyc_Absyn_Conref*(*)())Cyc_Absyn_empty_conref)())) && !Cyc_Tcutil_unify(
_tmp9F,Cyc_Absyn_dynforward_typ(Cyc_Absyn_const_string_typ(Cyc_Absyn_new_evar(({
struct Cyc_Core_Opt*_tmpA4=_cycalloc(sizeof(*_tmpA4));_tmpA4->v=(void*)((void*)3);
_tmpA4;}),tvs)),Cyc_Absyn_new_evar(({struct Cyc_Core_Opt*_tmpA5=_cycalloc(sizeof(*
_tmpA5));_tmpA5->v=(void*)((void*)3);_tmpA5;}),tvs),Cyc_Absyn_empty_tqual(0),((
struct Cyc_Absyn_Conref*(*)())Cyc_Absyn_empty_conref)()))) && !Cyc_Tcutil_unify(
_tmp9F,Cyc_Absyn_dynforward_typ(Cyc_Absyn_string_typ(Cyc_Absyn_new_evar(({struct
Cyc_Core_Opt*_tmpA6=_cycalloc(sizeof(*_tmpA6));_tmpA6->v=(void*)((void*)3);
_tmpA6;}),tvs)),Cyc_Absyn_new_evar(({struct Cyc_Core_Opt*_tmpA7=_cycalloc(sizeof(*
_tmpA7));_tmpA7->v=(void*)((void*)3);_tmpA7;}),tvs),Cyc_Absyn_const_tqual(0),((
struct Cyc_Absyn_Conref*(*)())Cyc_Absyn_empty_conref)()))) && !Cyc_Tcutil_unify(
_tmp9F,Cyc_Absyn_dynforward_typ(Cyc_Absyn_const_string_typ(Cyc_Absyn_new_evar(({
struct Cyc_Core_Opt*_tmpA8=_cycalloc(sizeof(*_tmpA8));_tmpA8->v=(void*)((void*)3);
_tmpA8;}),tvs)),Cyc_Absyn_new_evar(({struct Cyc_Core_Opt*_tmpA9=_cycalloc(sizeof(*
_tmpA9));_tmpA9->v=(void*)((void*)3);_tmpA9;}),tvs),Cyc_Absyn_const_tqual(0),((
struct Cyc_Absyn_Conref*(*)())Cyc_Absyn_empty_conref)())))({struct Cyc_String_pa_struct
_tmpAC;_tmpAC.tag=0;_tmpAC.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_typ2string(
_tmp9F));{void*_tmpAA[1]={& _tmpAC};Cyc_Tcutil_terr(loc,({const char*_tmpAB="second argument of main has type %s instead of char??";
_tag_dynforward(_tmpAB,sizeof(char),_get_zero_arr_size(_tmpAB,54));}),
_tag_dynforward(_tmpAA,sizeof(void*),1));}});}}}}}}}static void Cyc_Tc_tcTypedefdecl(
struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,
struct Cyc_Absyn_Typedefdecl*td){struct _dynforward_ptr*v=(*td->name).f2;{union Cyc_Absyn_Nmspace_union
_tmpAE=(*td->name).f1;struct Cyc_List_List*_tmpAF;struct Cyc_List_List*_tmpB0;
_LL88: if((_tmpAE.Rel_n).tag != 1)goto _LL8A;_tmpAF=(_tmpAE.Rel_n).f1;if(_tmpAF != 0)
goto _LL8A;_LL89: goto _LL8B;_LL8A: if((_tmpAE.Abs_n).tag != 2)goto _LL8C;_tmpB0=(
_tmpAE.Abs_n).f1;if(_tmpB0 != 0)goto _LL8C;_LL8B: goto _LL87;_LL8C:;_LL8D:({struct
Cyc_String_pa_struct _tmpB3;_tmpB3.tag=0;_tmpB3.f1=(struct _dynforward_ptr)((
struct _dynforward_ptr)Cyc_Absynpp_qvar2string(td->name));{void*_tmpB1[1]={&
_tmpB3};Cyc_Tcutil_terr(loc,({const char*_tmpB2="qualified typedef declarations are not implemented (%s)";
_tag_dynforward(_tmpB2,sizeof(char),_get_zero_arr_size(_tmpB2,56));}),
_tag_dynforward(_tmpB1,sizeof(void*),1));}});return;_LL87:;}if(((int(*)(struct
Cyc_Dict_Dict d,struct _dynforward_ptr*k))Cyc_Dict_member)(ge->typedefs,v)){({
struct Cyc_String_pa_struct _tmpB6;_tmpB6.tag=0;_tmpB6.f1=(struct _dynforward_ptr)((
struct _dynforward_ptr)*v);{void*_tmpB4[1]={& _tmpB6};Cyc_Tcutil_terr(loc,({const
char*_tmpB5="redeclaration of typedef %s";_tag_dynforward(_tmpB5,sizeof(char),
_get_zero_arr_size(_tmpB5,28));}),_tag_dynforward(_tmpB4,sizeof(void*),1));}});
return;}(*td->name).f1=(union Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union
_tmpB7;(_tmpB7.Abs_n).tag=2;(_tmpB7.Abs_n).f1=te->ns;_tmpB7;});Cyc_Tcutil_check_unique_tvars(
loc,td->tvs);Cyc_Tcutil_add_tvar_identities(td->tvs);if(td->defn != 0){Cyc_Tcutil_check_type(
loc,te,td->tvs,(void*)0,0,(void*)((struct Cyc_Core_Opt*)_check_null(td->defn))->v);(
td->tq).real_const=Cyc_Tcutil_extract_const_from_typedef(loc,(td->tq).print_const,(
void*)((struct Cyc_Core_Opt*)_check_null(td->defn))->v);}{struct Cyc_List_List*tvs=
td->tvs;for(0;tvs != 0;tvs=tvs->tl){void*_tmpB8=Cyc_Absyn_compress_kb((void*)((
struct Cyc_Absyn_Tvar*)tvs->hd)->kind);struct Cyc_Core_Opt*_tmpB9;struct Cyc_Core_Opt**
_tmpBA;struct Cyc_Core_Opt*_tmpBB;struct Cyc_Core_Opt**_tmpBC;void*_tmpBD;_LL8F:
if(*((int*)_tmpB8)!= 1)goto _LL91;_tmpB9=((struct Cyc_Absyn_Unknown_kb_struct*)
_tmpB8)->f1;_tmpBA=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_struct*)
_tmpB8)->f1;_LL90: if(td->defn != 0)({struct Cyc_String_pa_struct _tmpC0;_tmpC0.tag=
0;_tmpC0.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*((struct Cyc_Absyn_Tvar*)
tvs->hd)->name);{void*_tmpBE[1]={& _tmpC0};Cyc_Tcutil_warn(loc,({const char*_tmpBF="type variable %s is not used in typedef definition";
_tag_dynforward(_tmpBF,sizeof(char),_get_zero_arr_size(_tmpBF,51));}),
_tag_dynforward(_tmpBE,sizeof(void*),1));}});*_tmpBA=({struct Cyc_Core_Opt*_tmpC1=
_cycalloc(sizeof(*_tmpC1));_tmpC1->v=(void*)Cyc_Tcutil_kind_to_bound((void*)2);
_tmpC1;});goto _LL8E;_LL91: if(*((int*)_tmpB8)!= 2)goto _LL93;_tmpBB=((struct Cyc_Absyn_Less_kb_struct*)
_tmpB8)->f1;_tmpBC=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)
_tmpB8)->f1;_tmpBD=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmpB8)->f2;_LL92:*
_tmpBC=({struct Cyc_Core_Opt*_tmpC2=_cycalloc(sizeof(*_tmpC2));_tmpC2->v=(void*)
Cyc_Tcutil_kind_to_bound(_tmpBD);_tmpC2;});goto _LL8E;_LL93:;_LL94: continue;_LL8E:;}}
ge->typedefs=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dynforward_ptr*
k,struct Cyc_Absyn_Typedefdecl*v))Cyc_Dict_insert)(ge->typedefs,v,td);}static void
Cyc_Tc_tcAggrImpl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*
loc,struct Cyc_List_List*tvs,struct Cyc_List_List*rpo,struct Cyc_List_List*fields){
struct _RegionHandle _tmpC3=_new_region("uprev_rgn");struct _RegionHandle*uprev_rgn=&
_tmpC3;_push_region(uprev_rgn);for(0;rpo != 0;rpo=rpo->tl){struct _tuple3 _tmpC5;
void*_tmpC6;void*_tmpC7;struct _tuple3*_tmpC4=(struct _tuple3*)rpo->hd;_tmpC5=*
_tmpC4;_tmpC6=_tmpC5.f1;_tmpC7=_tmpC5.f2;Cyc_Tcutil_check_type(loc,te,tvs,(void*)
6,0,_tmpC6);Cyc_Tcutil_check_type(loc,te,tvs,(void*)5,0,_tmpC7);}{struct Cyc_List_List*
prev_fields=0;struct Cyc_List_List*_tmpC8=fields;for(0;_tmpC8 != 0;_tmpC8=_tmpC8->tl){
struct Cyc_Absyn_Aggrfield _tmpCA;struct _dynforward_ptr*_tmpCB;struct Cyc_Absyn_Tqual
_tmpCC;void*_tmpCD;struct Cyc_Absyn_Exp*_tmpCE;struct Cyc_List_List*_tmpCF;struct
Cyc_Absyn_Aggrfield*_tmpC9=(struct Cyc_Absyn_Aggrfield*)_tmpC8->hd;_tmpCA=*_tmpC9;
_tmpCB=_tmpCA.name;_tmpCC=_tmpCA.tq;_tmpCD=(void*)_tmpCA.type;_tmpCE=_tmpCA.width;
_tmpCF=_tmpCA.attributes;if(((int(*)(int(*compare)(struct _dynforward_ptr*,struct
_dynforward_ptr*),struct Cyc_List_List*l,struct _dynforward_ptr*x))Cyc_List_mem)(
Cyc_strptrcmp,prev_fields,_tmpCB))({struct Cyc_String_pa_struct _tmpD2;_tmpD2.tag=
0;_tmpD2.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*_tmpCB);{void*_tmpD0[
1]={& _tmpD2};Cyc_Tcutil_terr(loc,({const char*_tmpD1="duplicate field %s";
_tag_dynforward(_tmpD1,sizeof(char),_get_zero_arr_size(_tmpD1,19));}),
_tag_dynforward(_tmpD0,sizeof(void*),1));}});if(Cyc_strcmp((struct
_dynforward_ptr)*_tmpCB,({const char*_tmpD3="";_tag_dynforward(_tmpD3,sizeof(char),
_get_zero_arr_size(_tmpD3,1));}))!= 0)prev_fields=({struct Cyc_List_List*_tmpD4=
_region_malloc(uprev_rgn,sizeof(*_tmpD4));_tmpD4->hd=_tmpCB;_tmpD4->tl=
prev_fields;_tmpD4;});Cyc_Tcutil_check_type(loc,te,tvs,(void*)1,0,_tmpCD);(((
struct Cyc_Absyn_Aggrfield*)_tmpC8->hd)->tq).real_const=Cyc_Tcutil_extract_const_from_typedef(
loc,(((struct Cyc_Absyn_Aggrfield*)_tmpC8->hd)->tq).print_const,_tmpCD);Cyc_Tcutil_check_bitfield(
loc,te,_tmpCD,_tmpCE,_tmpCB);}};_pop_region(uprev_rgn);}struct _tuple9{struct Cyc_Absyn_AggrdeclImpl*
f1;struct Cyc_Absyn_Aggrdecl***f2;};void Cyc_Tc_tcAggrdecl(struct Cyc_Tcenv_Tenv*te,
struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Aggrdecl*
ad){struct _dynforward_ptr*_tmpD5=(*ad->name).f2;{struct Cyc_List_List*atts=ad->attributes;
for(0;atts != 0;atts=atts->tl){void*_tmpD6=(void*)atts->hd;_LL96: if((int)_tmpD6 != 
5)goto _LL98;_LL97: goto _LL99;_LL98: if(_tmpD6 <= (void*)17)goto _LL9A;if(*((int*)
_tmpD6)!= 1)goto _LL9A;_LL99: continue;_LL9A:;_LL9B:({struct Cyc_String_pa_struct
_tmpDA;_tmpDA.tag=0;_tmpDA.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*
_tmpD5);{struct Cyc_String_pa_struct _tmpD9;_tmpD9.tag=0;_tmpD9.f1=(struct
_dynforward_ptr)((struct _dynforward_ptr)Cyc_Absyn_attribute2string((void*)atts->hd));{
void*_tmpD7[2]={& _tmpD9,& _tmpDA};Cyc_Tcutil_terr(loc,({const char*_tmpD8="bad attribute %s in  %s definition";
_tag_dynforward(_tmpD8,sizeof(char),_get_zero_arr_size(_tmpD8,35));}),
_tag_dynforward(_tmpD7,sizeof(void*),2));}}});goto _LL95;_LL95:;}}{struct Cyc_List_List*
_tmpDB=ad->tvs;{struct Cyc_List_List*tvs2=_tmpDB;for(0;tvs2 != 0;tvs2=tvs2->tl){
void*_tmpDC=Cyc_Absyn_compress_kb((void*)((struct Cyc_Absyn_Tvar*)tvs2->hd)->kind);
struct Cyc_Core_Opt*_tmpDD;struct Cyc_Core_Opt**_tmpDE;struct Cyc_Core_Opt*_tmpDF;
struct Cyc_Core_Opt**_tmpE0;void*_tmpE1;struct Cyc_Core_Opt*_tmpE2;struct Cyc_Core_Opt**
_tmpE3;void*_tmpE4;void*_tmpE5;_LL9D: if(*((int*)_tmpDC)!= 1)goto _LL9F;_tmpDD=((
struct Cyc_Absyn_Unknown_kb_struct*)_tmpDC)->f1;_tmpDE=(struct Cyc_Core_Opt**)&((
struct Cyc_Absyn_Unknown_kb_struct*)_tmpDC)->f1;_LL9E: _tmpE0=_tmpDE;goto _LLA0;
_LL9F: if(*((int*)_tmpDC)!= 2)goto _LLA1;_tmpDF=((struct Cyc_Absyn_Less_kb_struct*)
_tmpDC)->f1;_tmpE0=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)
_tmpDC)->f1;_tmpE1=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmpDC)->f2;if((int)
_tmpE1 != 1)goto _LLA1;_LLA0: _tmpE3=_tmpE0;goto _LLA2;_LLA1: if(*((int*)_tmpDC)!= 2)
goto _LLA3;_tmpE2=((struct Cyc_Absyn_Less_kb_struct*)_tmpDC)->f1;_tmpE3=(struct Cyc_Core_Opt**)&((
struct Cyc_Absyn_Less_kb_struct*)_tmpDC)->f1;_tmpE4=(void*)((struct Cyc_Absyn_Less_kb_struct*)
_tmpDC)->f2;if((int)_tmpE4 != 0)goto _LLA3;_LLA2:*_tmpE3=({struct Cyc_Core_Opt*
_tmpE6=_cycalloc(sizeof(*_tmpE6));_tmpE6->v=(void*)Cyc_Tcutil_kind_to_bound((
void*)2);_tmpE6;});continue;_LLA3: if(*((int*)_tmpDC)!= 0)goto _LLA5;_tmpE5=(void*)((
struct Cyc_Absyn_Eq_kb_struct*)_tmpDC)->f1;if((int)_tmpE5 != 1)goto _LLA5;_LLA4:({
struct Cyc_String_pa_struct _tmpEA;_tmpEA.tag=0;_tmpEA.f1=(struct _dynforward_ptr)((
struct _dynforward_ptr)*((struct Cyc_Absyn_Tvar*)tvs2->hd)->name);{struct Cyc_String_pa_struct
_tmpE9;_tmpE9.tag=0;_tmpE9.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*
_tmpD5);{void*_tmpE7[2]={& _tmpE9,& _tmpEA};Cyc_Tcutil_terr(loc,({const char*_tmpE8="type %s attempts to abstract type variable %s of kind M";
_tag_dynforward(_tmpE8,sizeof(char),_get_zero_arr_size(_tmpE8,56));}),
_tag_dynforward(_tmpE7,sizeof(void*),2));}}});continue;_LLA5:;_LLA6: continue;
_LL9C:;}}{union Cyc_Absyn_Nmspace_union _tmpEB=(*ad->name).f1;struct Cyc_List_List*
_tmpEC;struct Cyc_List_List*_tmpED;_LLA8: if((_tmpEB.Rel_n).tag != 1)goto _LLAA;
_tmpEC=(_tmpEB.Rel_n).f1;if(_tmpEC != 0)goto _LLAA;_LLA9: goto _LLAB;_LLAA: if((
_tmpEB.Abs_n).tag != 2)goto _LLAC;_tmpED=(_tmpEB.Abs_n).f1;if(_tmpED != 0)goto _LLAC;
_LLAB: goto _LLA7;_LLAC:;_LLAD:({struct Cyc_String_pa_struct _tmpF0;_tmpF0.tag=0;
_tmpF0.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_qvar2string(
ad->name));{void*_tmpEE[1]={& _tmpF0};Cyc_Tcutil_terr(loc,({const char*_tmpEF="qualified struct declarations are not implemented (%s)";
_tag_dynforward(_tmpEF,sizeof(char),_get_zero_arr_size(_tmpEF,55));}),
_tag_dynforward(_tmpEE,sizeof(void*),1));}});return;_LLA7:;}(*ad->name).f1=(
union Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union _tmpF1;(_tmpF1.Abs_n).tag=
2;(_tmpF1.Abs_n).f1=te->ns;_tmpF1;});Cyc_Tcutil_check_unique_tvars(loc,ad->tvs);
Cyc_Tcutil_add_tvar_identities(ad->tvs);{struct _tuple9 _tmpF3=({struct _tuple9
_tmpF2;_tmpF2.f1=ad->impl;_tmpF2.f2=((struct Cyc_Absyn_Aggrdecl***(*)(struct Cyc_Dict_Dict
d,struct _dynforward_ptr*k))Cyc_Dict_lookup_opt)(ge->aggrdecls,_tmpD5);_tmpF2;});
struct Cyc_Absyn_AggrdeclImpl*_tmpF4;struct Cyc_Absyn_Aggrdecl***_tmpF5;struct Cyc_Absyn_AggrdeclImpl*
_tmpF6;struct Cyc_Absyn_AggrdeclImpl _tmpF7;struct Cyc_List_List*_tmpF8;struct Cyc_List_List*
_tmpF9;struct Cyc_List_List*_tmpFA;struct Cyc_Absyn_Aggrdecl***_tmpFB;struct Cyc_Absyn_AggrdeclImpl*
_tmpFC;struct Cyc_Absyn_AggrdeclImpl _tmpFD;struct Cyc_List_List*_tmpFE;struct Cyc_List_List*
_tmpFF;struct Cyc_List_List*_tmp100;struct Cyc_Absyn_Aggrdecl***_tmp101;struct Cyc_Absyn_Aggrdecl**
_tmp102;struct Cyc_Absyn_AggrdeclImpl*_tmp103;struct Cyc_Absyn_Aggrdecl***_tmp104;
struct Cyc_Absyn_Aggrdecl**_tmp105;_LLAF: _tmpF4=_tmpF3.f1;if(_tmpF4 != 0)goto _LLB1;
_tmpF5=_tmpF3.f2;if(_tmpF5 != 0)goto _LLB1;_LLB0: ge->aggrdecls=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct Cyc_Absyn_Aggrdecl**v))Cyc_Dict_insert)(
ge->aggrdecls,_tmpD5,({struct Cyc_Absyn_Aggrdecl**_tmp106=_cycalloc(sizeof(*
_tmp106));_tmp106[0]=ad;_tmp106;}));goto _LLAE;_LLB1: _tmpF6=_tmpF3.f1;if(_tmpF6 == 
0)goto _LLB3;_tmpF7=*_tmpF6;_tmpF8=_tmpF7.exist_vars;_tmpF9=_tmpF7.rgn_po;_tmpFA=
_tmpF7.fields;_tmpFB=_tmpF3.f2;if(_tmpFB != 0)goto _LLB3;_LLB2: {struct Cyc_Absyn_Aggrdecl**
_tmp107=({struct Cyc_Absyn_Aggrdecl**_tmp10D=_cycalloc(sizeof(*_tmp10D));_tmp10D[
0]=({struct Cyc_Absyn_Aggrdecl*_tmp10E=_cycalloc(sizeof(*_tmp10E));_tmp10E->kind=(
void*)((void*)ad->kind);_tmp10E->sc=(void*)((void*)3);_tmp10E->name=ad->name;
_tmp10E->tvs=_tmpDB;_tmp10E->impl=0;_tmp10E->attributes=ad->attributes;_tmp10E;});
_tmp10D;});ge->aggrdecls=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct
_dynforward_ptr*k,struct Cyc_Absyn_Aggrdecl**v))Cyc_Dict_insert)(ge->aggrdecls,
_tmpD5,_tmp107);Cyc_Tcutil_check_unique_tvars(loc,_tmpF8);Cyc_Tcutil_add_tvar_identities(
_tmpF8);Cyc_Tc_tcAggrImpl(te,ge,loc,((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_append)(_tmpDB,_tmpF8),_tmpF9,_tmpFA);if((void*)
ad->kind == (void*)1){struct Cyc_List_List*f=_tmpFA;for(0;f != 0;f=f->tl){if(!Cyc_Tcutil_bits_only((
void*)((struct Cyc_Absyn_Aggrfield*)f->hd)->type))({struct Cyc_String_pa_struct
_tmp10C;_tmp10C.tag=0;_tmp10C.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)
Cyc_Absynpp_typ2string((void*)((struct Cyc_Absyn_Aggrfield*)f->hd)->type));{
struct Cyc_String_pa_struct _tmp10B;_tmp10B.tag=0;_tmp10B.f1=(struct
_dynforward_ptr)((struct _dynforward_ptr)*_tmpD5);{struct Cyc_String_pa_struct
_tmp10A;_tmp10A.tag=0;_tmp10A.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*((
struct Cyc_Absyn_Aggrfield*)f->hd)->name);{void*_tmp108[3]={& _tmp10A,& _tmp10B,&
_tmp10C};Cyc_Tcutil_warn(loc,({const char*_tmp109="member %s of union %s has type %s which is not `bits-only' so it can only be written and not read";
_tag_dynforward(_tmp109,sizeof(char),_get_zero_arr_size(_tmp109,98));}),
_tag_dynforward(_tmp108,sizeof(void*),3));}}}});}}*_tmp107=ad;goto _LLAE;}_LLB3:
_tmpFC=_tmpF3.f1;if(_tmpFC == 0)goto _LLB5;_tmpFD=*_tmpFC;_tmpFE=_tmpFD.exist_vars;
_tmpFF=_tmpFD.rgn_po;_tmp100=_tmpFD.fields;_tmp101=_tmpF3.f2;if(_tmp101 == 0)goto
_LLB5;_tmp102=*_tmp101;_LLB4: if((void*)ad->kind != (void*)(*_tmp102)->kind)({void*
_tmp10F[0]={};Cyc_Tcutil_terr(loc,({const char*_tmp110="cannot reuse struct names for unions and vice-versa";
_tag_dynforward(_tmp110,sizeof(char),_get_zero_arr_size(_tmp110,52));}),
_tag_dynforward(_tmp10F,sizeof(void*),0));});{struct Cyc_Absyn_Aggrdecl*_tmp111=*
_tmp102;*_tmp102=({struct Cyc_Absyn_Aggrdecl*_tmp112=_cycalloc(sizeof(*_tmp112));
_tmp112->kind=(void*)((void*)ad->kind);_tmp112->sc=(void*)((void*)3);_tmp112->name=
ad->name;_tmp112->tvs=_tmpDB;_tmp112->impl=0;_tmp112->attributes=ad->attributes;
_tmp112;});Cyc_Tcutil_check_unique_tvars(loc,_tmpFE);Cyc_Tcutil_add_tvar_identities(
_tmpFE);Cyc_Tc_tcAggrImpl(te,ge,loc,((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_append)(_tmpDB,_tmpFE),_tmpFF,_tmp100);*
_tmp102=_tmp111;_tmp105=_tmp102;goto _LLB6;}_LLB5: _tmp103=_tmpF3.f1;if(_tmp103 != 
0)goto _LLAE;_tmp104=_tmpF3.f2;if(_tmp104 == 0)goto _LLAE;_tmp105=*_tmp104;_LLB6: {
struct Cyc_Absyn_Aggrdecl*_tmp113=Cyc_Tcdecl_merge_aggrdecl(*_tmp105,ad,loc,Cyc_Tc_tc_msg);
if(_tmp113 == 0)return;else{*_tmp105=(struct Cyc_Absyn_Aggrdecl*)_tmp113;ad=(
struct Cyc_Absyn_Aggrdecl*)_tmp113;goto _LLAE;}}_LLAE:;}ge->ordinaries=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct _tuple7*v))Cyc_Dict_insert)(
ge->ordinaries,_tmpD5,(struct _tuple7*)({struct _tuple7*_tmp114=_cycalloc(sizeof(*
_tmp114));_tmp114->f1=(void*)({struct Cyc_Tcenv_AggrRes_struct*_tmp115=_cycalloc(
sizeof(*_tmp115));_tmp115[0]=({struct Cyc_Tcenv_AggrRes_struct _tmp116;_tmp116.tag=
1;_tmp116.f1=ad;_tmp116;});_tmp115;});_tmp114->f2=1;_tmp114;}));}}struct _tuple10{
struct Cyc_Absyn_Tqual f1;void*f2;};static struct Cyc_List_List*Cyc_Tc_tcTunionFields(
struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,
struct _dynforward_ptr obj,int is_xtunion,struct _tuple0*name,struct Cyc_List_List*
fields,struct Cyc_List_List*tvs,struct Cyc_Absyn_Tuniondecl*tudres){{struct Cyc_List_List*
_tmp117=fields;for(0;_tmp117 != 0;_tmp117=_tmp117->tl){struct Cyc_Absyn_Tunionfield*
_tmp118=(struct Cyc_Absyn_Tunionfield*)_tmp117->hd;{struct Cyc_List_List*typs=
_tmp118->typs;for(0;typs != 0;typs=typs->tl){Cyc_Tcutil_check_type(_tmp118->loc,
te,tvs,(void*)1,0,(*((struct _tuple10*)typs->hd)).f2);if(!tudres->is_flat  && Cyc_Tcutil_is_noalias_pointer_or_aggr((*((
struct _tuple10*)typs->hd)).f2))({struct Cyc_String_pa_struct _tmp11B;_tmp11B.tag=0;
_tmp11B.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_qvar2string(
_tmp118->name));{void*_tmp119[1]={& _tmp11B};Cyc_Tcutil_terr(_tmp118->loc,({const
char*_tmp11A="noalias pointers in non-flat tunions are not allowed (%s)";
_tag_dynforward(_tmp11A,sizeof(char),_get_zero_arr_size(_tmp11A,58));}),
_tag_dynforward(_tmp119,sizeof(void*),1));}});((*((struct _tuple10*)typs->hd)).f1).real_const=
Cyc_Tcutil_extract_const_from_typedef(_tmp118->loc,((*((struct _tuple10*)typs->hd)).f1).print_const,(*((
struct _tuple10*)typs->hd)).f2);}}{union Cyc_Absyn_Nmspace_union _tmp11C=(*_tmp118->name).f1;
struct Cyc_List_List*_tmp11D;_LLB8: if((_tmp11C.Rel_n).tag != 1)goto _LLBA;_tmp11D=(
_tmp11C.Rel_n).f1;if(_tmp11D != 0)goto _LLBA;_LLB9: if(is_xtunion)(*_tmp118->name).f1=(
union Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union _tmp11E;(_tmp11E.Abs_n).tag=
2;(_tmp11E.Abs_n).f1=te->ns;_tmp11E;});else{(*_tmp118->name).f1=(*name).f1;}goto
_LLB7;_LLBA: if((_tmp11C.Rel_n).tag != 1)goto _LLBC;_LLBB:({struct Cyc_String_pa_struct
_tmp121;_tmp121.tag=0;_tmp121.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)
Cyc_Absynpp_qvar2string(_tmp118->name));{void*_tmp11F[1]={& _tmp121};Cyc_Tcutil_terr(
_tmp118->loc,({const char*_tmp120="qualified tunionfield declarations are not allowed (%s)";
_tag_dynforward(_tmp120,sizeof(char),_get_zero_arr_size(_tmp120,56));}),
_tag_dynforward(_tmp11F,sizeof(void*),1));}});goto _LLB7;_LLBC: if((_tmp11C.Abs_n).tag
!= 2)goto _LLBE;_LLBD: goto _LLB7;_LLBE: if((_tmp11C.Loc_n).tag != 0)goto _LLB7;_LLBF:({
void*_tmp122[0]={};((int(*)(struct _dynforward_ptr fmt,struct _dynforward_ptr ap))
Cyc_Tcutil_impos)(({const char*_tmp123="tcTunionFields: Loc_n";_tag_dynforward(
_tmp123,sizeof(char),_get_zero_arr_size(_tmp123,22));}),_tag_dynforward(_tmp122,
sizeof(void*),0));});_LLB7:;}}}{struct Cyc_List_List*fields2;if(is_xtunion){int
_tmp124=1;struct Cyc_List_List*_tmp125=Cyc_Tcdecl_sort_xtunion_fields(fields,&
_tmp124,(*name).f2,loc,Cyc_Tc_tc_msg);if(_tmp124)fields2=_tmp125;else{fields2=0;}}
else{struct _RegionHandle _tmp126=_new_region("uprev_rgn");struct _RegionHandle*
uprev_rgn=& _tmp126;_push_region(uprev_rgn);{struct Cyc_List_List*prev_fields=0;{
struct Cyc_List_List*fs=fields;for(0;fs != 0;fs=fs->tl){struct Cyc_Absyn_Tunionfield*
_tmp127=(struct Cyc_Absyn_Tunionfield*)fs->hd;if(((int(*)(int(*compare)(struct
_dynforward_ptr*,struct _dynforward_ptr*),struct Cyc_List_List*l,struct
_dynforward_ptr*x))Cyc_List_mem)(Cyc_strptrcmp,prev_fields,(*_tmp127->name).f2))({
struct Cyc_String_pa_struct _tmp12B;_tmp12B.tag=0;_tmp12B.f1=(struct
_dynforward_ptr)((struct _dynforward_ptr)obj);{struct Cyc_String_pa_struct _tmp12A;
_tmp12A.tag=0;_tmp12A.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*(*
_tmp127->name).f2);{void*_tmp128[2]={& _tmp12A,& _tmp12B};Cyc_Tcutil_terr(_tmp127->loc,({
const char*_tmp129="duplicate field name %s in %s";_tag_dynforward(_tmp129,
sizeof(char),_get_zero_arr_size(_tmp129,30));}),_tag_dynforward(_tmp128,sizeof(
void*),2));}}});else{prev_fields=({struct Cyc_List_List*_tmp12C=_region_malloc(
uprev_rgn,sizeof(*_tmp12C));_tmp12C->hd=(*_tmp127->name).f2;_tmp12C->tl=
prev_fields;_tmp12C;});}if((void*)_tmp127->sc != (void*)2){({struct Cyc_String_pa_struct
_tmp12F;_tmp12F.tag=0;_tmp12F.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*(*
_tmp127->name).f2);{void*_tmp12D[1]={& _tmp12F};Cyc_Tcutil_warn(loc,({const char*
_tmp12E="ignoring scope of field %s";_tag_dynforward(_tmp12E,sizeof(char),
_get_zero_arr_size(_tmp12E,27));}),_tag_dynforward(_tmp12D,sizeof(void*),1));}});(
void*)(_tmp127->sc=(void*)((void*)2));}}}fields2=fields;};_pop_region(uprev_rgn);}{
struct Cyc_List_List*_tmp130=fields;for(0;_tmp130 != 0;_tmp130=_tmp130->tl){struct
Cyc_Absyn_Tunionfield*_tmp131=(struct Cyc_Absyn_Tunionfield*)_tmp130->hd;ge->ordinaries=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct
_tuple7*v))Cyc_Dict_insert)(ge->ordinaries,(*_tmp131->name).f2,(struct _tuple7*)({
struct _tuple7*_tmp132=_cycalloc(sizeof(*_tmp132));_tmp132->f1=(void*)({struct Cyc_Tcenv_TunionRes_struct*
_tmp133=_cycalloc(sizeof(*_tmp133));_tmp133[0]=({struct Cyc_Tcenv_TunionRes_struct
_tmp134;_tmp134.tag=2;_tmp134.f1=tudres;_tmp134.f2=_tmp131;_tmp134;});_tmp133;});
_tmp132->f2=1;_tmp132;}));}}return fields2;}}struct _tuple11{struct Cyc_Core_Opt*f1;
struct Cyc_Absyn_Tuniondecl***f2;};void Cyc_Tc_tcTuniondecl(struct Cyc_Tcenv_Tenv*
te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Tuniondecl*
tud){struct _dynforward_ptr*v=(*tud->name).f2;struct _dynforward_ptr obj=tud->is_xtunion?({
const char*_tmp183="xtunion";_tag_dynforward(_tmp183,sizeof(char),
_get_zero_arr_size(_tmp183,8));}):({const char*_tmp184="tunion";_tag_dynforward(
_tmp184,sizeof(char),_get_zero_arr_size(_tmp184,7));});struct Cyc_List_List*tvs=
tud->tvs;{struct Cyc_List_List*tvs2=tvs;for(0;tvs2 != 0;tvs2=tvs2->tl){void*
_tmp135=Cyc_Absyn_compress_kb((void*)((struct Cyc_Absyn_Tvar*)tvs2->hd)->kind);
struct Cyc_Core_Opt*_tmp136;struct Cyc_Core_Opt**_tmp137;struct Cyc_Core_Opt*
_tmp138;struct Cyc_Core_Opt**_tmp139;void*_tmp13A;struct Cyc_Core_Opt*_tmp13B;
struct Cyc_Core_Opt**_tmp13C;void*_tmp13D;struct Cyc_Core_Opt*_tmp13E;struct Cyc_Core_Opt**
_tmp13F;void*_tmp140;void*_tmp141;struct Cyc_Core_Opt*_tmp142;struct Cyc_Core_Opt**
_tmp143;void*_tmp144;void*_tmp145;void*_tmp146;_LLC1: if(*((int*)_tmp135)!= 1)
goto _LLC3;_tmp136=((struct Cyc_Absyn_Unknown_kb_struct*)_tmp135)->f1;_tmp137=(
struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_struct*)_tmp135)->f1;_LLC2:
_tmp139=_tmp137;goto _LLC4;_LLC3: if(*((int*)_tmp135)!= 2)goto _LLC5;_tmp138=((
struct Cyc_Absyn_Less_kb_struct*)_tmp135)->f1;_tmp139=(struct Cyc_Core_Opt**)&((
struct Cyc_Absyn_Less_kb_struct*)_tmp135)->f1;_tmp13A=(void*)((struct Cyc_Absyn_Less_kb_struct*)
_tmp135)->f2;if((int)_tmp13A != 1)goto _LLC5;_LLC4: _tmp13C=_tmp139;goto _LLC6;_LLC5:
if(*((int*)_tmp135)!= 2)goto _LLC7;_tmp13B=((struct Cyc_Absyn_Less_kb_struct*)
_tmp135)->f1;_tmp13C=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)
_tmp135)->f1;_tmp13D=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmp135)->f2;if((
int)_tmp13D != 0)goto _LLC7;_LLC6:*_tmp13C=({struct Cyc_Core_Opt*_tmp147=_cycalloc(
sizeof(*_tmp147));_tmp147->v=(void*)Cyc_Tcutil_kind_to_bound((void*)2);_tmp147;});
goto _LLC0;_LLC7: if(*((int*)_tmp135)!= 2)goto _LLC9;_tmp13E=((struct Cyc_Absyn_Less_kb_struct*)
_tmp135)->f1;_tmp13F=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)
_tmp135)->f1;_tmp140=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmp135)->f2;if((
int)_tmp140 != 5)goto _LLC9;_LLC8:*_tmp13F=({struct Cyc_Core_Opt*_tmp148=_cycalloc(
sizeof(*_tmp148));_tmp148->v=(void*)Cyc_Tcutil_kind_to_bound((void*)3);_tmp148;});
goto _LLC0;_LLC9: if(*((int*)_tmp135)!= 0)goto _LLCB;_tmp141=(void*)((struct Cyc_Absyn_Eq_kb_struct*)
_tmp135)->f1;if((int)_tmp141 != 5)goto _LLCB;_LLCA: if(!tud->is_flat)({struct Cyc_String_pa_struct
_tmp14D;_tmp14D.tag=0;_tmp14D.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*((
struct Cyc_Absyn_Tvar*)tvs2->hd)->name);{struct Cyc_String_pa_struct _tmp14C;
_tmp14C.tag=0;_tmp14C.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*v);{
struct Cyc_String_pa_struct _tmp14B;_tmp14B.tag=0;_tmp14B.f1=(struct
_dynforward_ptr)((struct _dynforward_ptr)obj);{void*_tmp149[3]={& _tmp14B,& _tmp14C,&
_tmp14D};Cyc_Tcutil_terr(loc,({const char*_tmp14A="%s %s attempts to abstract type variable %s of kind TR";
_tag_dynforward(_tmp14A,sizeof(char),_get_zero_arr_size(_tmp14A,55));}),
_tag_dynforward(_tmp149,sizeof(void*),3));}}}});goto _LLC0;_LLCB: if(*((int*)
_tmp135)!= 2)goto _LLCD;_tmp142=((struct Cyc_Absyn_Less_kb_struct*)_tmp135)->f1;
_tmp143=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)_tmp135)->f1;
_tmp144=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmp135)->f2;_LLCC: goto _LLCE;
_LLCD: if(*((int*)_tmp135)!= 0)goto _LLCF;_tmp145=(void*)((struct Cyc_Absyn_Eq_kb_struct*)
_tmp135)->f1;if((int)_tmp145 != 4)goto _LLCF;_LLCE: if(!tud->is_flat)({struct Cyc_String_pa_struct
_tmp152;_tmp152.tag=0;_tmp152.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*((
struct Cyc_Absyn_Tvar*)tvs2->hd)->name);{struct Cyc_String_pa_struct _tmp151;
_tmp151.tag=0;_tmp151.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*v);{
struct Cyc_String_pa_struct _tmp150;_tmp150.tag=0;_tmp150.f1=(struct
_dynforward_ptr)((struct _dynforward_ptr)obj);{void*_tmp14E[3]={& _tmp150,& _tmp151,&
_tmp152};Cyc_Tcutil_terr(loc,({const char*_tmp14F="%s %s attempts to abstract type variable %s of kind UR";
_tag_dynforward(_tmp14F,sizeof(char),_get_zero_arr_size(_tmp14F,55));}),
_tag_dynforward(_tmp14E,sizeof(void*),3));}}}});goto _LLC0;_LLCF: if(*((int*)
_tmp135)!= 0)goto _LLD1;_tmp146=(void*)((struct Cyc_Absyn_Eq_kb_struct*)_tmp135)->f1;
if((int)_tmp146 != 1)goto _LLD1;_LLD0:({struct Cyc_String_pa_struct _tmp157;_tmp157.tag=
0;_tmp157.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*((struct Cyc_Absyn_Tvar*)
tvs2->hd)->name);{struct Cyc_String_pa_struct _tmp156;_tmp156.tag=0;_tmp156.f1=(
struct _dynforward_ptr)((struct _dynforward_ptr)*v);{struct Cyc_String_pa_struct
_tmp155;_tmp155.tag=0;_tmp155.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)
obj);{void*_tmp153[3]={& _tmp155,& _tmp156,& _tmp157};Cyc_Tcutil_terr(loc,({const
char*_tmp154="%s %s attempts to abstract type variable %s of kind M";
_tag_dynforward(_tmp154,sizeof(char),_get_zero_arr_size(_tmp154,54));}),
_tag_dynforward(_tmp153,sizeof(void*),3));}}}});goto _LLC0;_LLD1:;_LLD2: goto _LLC0;
_LLC0:;}}Cyc_Tcutil_check_unique_tvars(loc,tvs);Cyc_Tcutil_add_tvar_identities(
tvs);{struct _RegionHandle _tmp158=_new_region("temp");struct _RegionHandle*temp=&
_tmp158;_push_region(temp);{struct Cyc_Absyn_Tuniondecl***tud_opt;if(tud->is_xtunion){{
struct _handler_cons _tmp159;_push_handler(& _tmp159);{int _tmp15B=0;if(setjmp(
_tmp159.handler))_tmp15B=1;if(!_tmp15B){tud_opt=Cyc_Tcenv_lookup_xtuniondecl(
temp,te,loc,tud->name);;_pop_handler();}else{void*_tmp15A=(void*)_exn_thrown;
void*_tmp15D=_tmp15A;_LLD4: if(_tmp15D != Cyc_Dict_Absent)goto _LLD6;_LLD5:({struct
Cyc_String_pa_struct _tmp160;_tmp160.tag=0;_tmp160.f1=(struct _dynforward_ptr)((
struct _dynforward_ptr)Cyc_Absynpp_qvar2string(tud->name));{void*_tmp15E[1]={&
_tmp160};Cyc_Tcutil_terr(loc,({const char*_tmp15F="qualified xtunion declaration %s is not an existing xtunion";
_tag_dynforward(_tmp15F,sizeof(char),_get_zero_arr_size(_tmp15F,60));}),
_tag_dynforward(_tmp15E,sizeof(void*),1));}});_npop_handler(0);return;_LLD6:;
_LLD7:(void)_throw(_tmp15D);_LLD3:;}}}if(tud_opt != 0)tud->name=(*(*tud_opt))->name;
else{(*tud->name).f1=(union Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union
_tmp161;(_tmp161.Abs_n).tag=2;(_tmp161.Abs_n).f1=te->ns;_tmp161;});}}else{{union
Cyc_Absyn_Nmspace_union _tmp162=(*tud->name).f1;struct Cyc_List_List*_tmp163;_LLD9:
if((_tmp162.Rel_n).tag != 1)goto _LLDB;_tmp163=(_tmp162.Rel_n).f1;if(_tmp163 != 0)
goto _LLDB;_LLDA:(*tud->name).f1=(union Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union
_tmp164;(_tmp164.Abs_n).tag=2;(_tmp164.Abs_n).f1=te->ns;_tmp164;});goto _LLD8;
_LLDB: if((_tmp162.Abs_n).tag != 2)goto _LLDD;_LLDC: goto _LLDE;_LLDD:;_LLDE:({struct
Cyc_String_pa_struct _tmp167;_tmp167.tag=0;_tmp167.f1=(struct _dynforward_ptr)((
struct _dynforward_ptr)Cyc_Absynpp_qvar2string(tud->name));{void*_tmp165[1]={&
_tmp167};Cyc_Tcutil_terr(loc,({const char*_tmp166="qualified tunion declarations are not implemented (%s)";
_tag_dynforward(_tmp166,sizeof(char),_get_zero_arr_size(_tmp166,55));}),
_tag_dynforward(_tmp165,sizeof(void*),1));}});_npop_handler(0);return;_LLD8:;}{
struct Cyc_Absyn_Tuniondecl***_tmp168=((struct Cyc_Absyn_Tuniondecl***(*)(struct
Cyc_Dict_Dict d,struct _dynforward_ptr*k))Cyc_Dict_lookup_opt)(ge->tuniondecls,v);
tud_opt=(unsigned int)_tmp168?({struct Cyc_Absyn_Tuniondecl***_tmp169=
_region_malloc(temp,sizeof(*_tmp169));_tmp169[0]=*_tmp168;_tmp169;}): 0;}}{struct
_tuple11 _tmp16B=({struct _tuple11 _tmp16A;_tmp16A.f1=tud->fields;_tmp16A.f2=
tud_opt;_tmp16A;});struct Cyc_Core_Opt*_tmp16C;struct Cyc_Absyn_Tuniondecl***
_tmp16D;struct Cyc_Core_Opt*_tmp16E;struct Cyc_Core_Opt _tmp16F;struct Cyc_List_List*
_tmp170;struct Cyc_List_List**_tmp171;struct Cyc_Absyn_Tuniondecl***_tmp172;struct
Cyc_Core_Opt*_tmp173;struct Cyc_Core_Opt _tmp174;struct Cyc_List_List*_tmp175;
struct Cyc_List_List**_tmp176;struct Cyc_Absyn_Tuniondecl***_tmp177;struct Cyc_Absyn_Tuniondecl**
_tmp178;struct Cyc_Core_Opt*_tmp179;struct Cyc_Absyn_Tuniondecl***_tmp17A;struct
Cyc_Absyn_Tuniondecl**_tmp17B;_LLE0: _tmp16C=_tmp16B.f1;if(_tmp16C != 0)goto _LLE2;
_tmp16D=_tmp16B.f2;if(_tmp16D != 0)goto _LLE2;_LLE1: ge->tuniondecls=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct Cyc_Absyn_Tuniondecl**v))Cyc_Dict_insert)(
ge->tuniondecls,v,({struct Cyc_Absyn_Tuniondecl**_tmp17C=_cycalloc(sizeof(*
_tmp17C));_tmp17C[0]=tud;_tmp17C;}));goto _LLDF;_LLE2: _tmp16E=_tmp16B.f1;if(
_tmp16E == 0)goto _LLE4;_tmp16F=*_tmp16E;_tmp170=(struct Cyc_List_List*)_tmp16F.v;
_tmp171=(struct Cyc_List_List**)&(*_tmp16B.f1).v;_tmp172=_tmp16B.f2;if(_tmp172 != 
0)goto _LLE4;_LLE3: {struct Cyc_Absyn_Tuniondecl**_tmp17D=({struct Cyc_Absyn_Tuniondecl**
_tmp17E=_cycalloc(sizeof(*_tmp17E));_tmp17E[0]=({struct Cyc_Absyn_Tuniondecl*
_tmp17F=_cycalloc(sizeof(*_tmp17F));_tmp17F->sc=(void*)((void*)3);_tmp17F->name=
tud->name;_tmp17F->tvs=tvs;_tmp17F->fields=0;_tmp17F->is_xtunion=tud->is_xtunion;
_tmp17F->is_flat=tud->is_flat;_tmp17F;});_tmp17E;});ge->tuniondecls=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct Cyc_Absyn_Tuniondecl**v))Cyc_Dict_insert)(
ge->tuniondecls,v,_tmp17D);*_tmp171=Cyc_Tc_tcTunionFields(te,ge,loc,obj,tud->is_xtunion,
tud->name,*_tmp171,tvs,tud);*_tmp17D=tud;goto _LLDF;}_LLE4: _tmp173=_tmp16B.f1;if(
_tmp173 == 0)goto _LLE6;_tmp174=*_tmp173;_tmp175=(struct Cyc_List_List*)_tmp174.v;
_tmp176=(struct Cyc_List_List**)&(*_tmp16B.f1).v;_tmp177=_tmp16B.f2;if(_tmp177 == 
0)goto _LLE6;_tmp178=*_tmp177;_LLE5: {struct Cyc_Absyn_Tuniondecl*_tmp180=*_tmp178;*
_tmp178=({struct Cyc_Absyn_Tuniondecl*_tmp181=_cycalloc(sizeof(*_tmp181));_tmp181->sc=(
void*)((void*)3);_tmp181->name=tud->name;_tmp181->tvs=tvs;_tmp181->fields=0;
_tmp181->is_xtunion=tud->is_xtunion;_tmp181->is_flat=tud->is_flat;_tmp181;});*
_tmp176=Cyc_Tc_tcTunionFields(te,ge,loc,obj,tud->is_xtunion,tud->name,*_tmp176,
tvs,tud);*_tmp178=_tmp180;_tmp17B=_tmp178;goto _LLE7;}_LLE6: _tmp179=_tmp16B.f1;
if(_tmp179 != 0)goto _LLDF;_tmp17A=_tmp16B.f2;if(_tmp17A == 0)goto _LLDF;_tmp17B=*
_tmp17A;_LLE7: {struct Cyc_Absyn_Tuniondecl*_tmp182=Cyc_Tcdecl_merge_tuniondecl(*
_tmp17B,tud,loc,Cyc_Tc_tc_msg);if(_tmp182 == 0){_npop_handler(0);return;}else{*
_tmp17B=(struct Cyc_Absyn_Tuniondecl*)_tmp182;goto _LLDF;}}_LLDF:;}};_pop_region(
temp);}}void Cyc_Tc_tcEnumdecl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,
struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Enumdecl*ed){struct
_dynforward_ptr*v=(*ed->name).f2;{union Cyc_Absyn_Nmspace_union _tmp185=(*ed->name).f1;
struct Cyc_List_List*_tmp186;struct Cyc_List_List*_tmp187;_LLE9: if((_tmp185.Rel_n).tag
!= 1)goto _LLEB;_tmp186=(_tmp185.Rel_n).f1;if(_tmp186 != 0)goto _LLEB;_LLEA: goto
_LLEC;_LLEB: if((_tmp185.Abs_n).tag != 2)goto _LLED;_tmp187=(_tmp185.Abs_n).f1;if(
_tmp187 != 0)goto _LLED;_LLEC: goto _LLE8;_LLED:;_LLEE:({struct Cyc_String_pa_struct
_tmp18A;_tmp18A.tag=0;_tmp18A.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)
Cyc_Absynpp_qvar2string(ed->name));{void*_tmp188[1]={& _tmp18A};Cyc_Tcutil_terr(
loc,({const char*_tmp189="qualified enum declarations are not implemented (%s)";
_tag_dynforward(_tmp189,sizeof(char),_get_zero_arr_size(_tmp189,53));}),
_tag_dynforward(_tmp188,sizeof(void*),1));}});return;_LLE8:;}(*ed->name).f1=(
union Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union _tmp18B;(_tmp18B.Abs_n).tag=
2;(_tmp18B.Abs_n).f1=te->ns;_tmp18B;});if(ed->fields != 0){struct _RegionHandle
_tmp18C=_new_region("uprev_rgn");struct _RegionHandle*uprev_rgn=& _tmp18C;
_push_region(uprev_rgn);{struct Cyc_List_List*prev_fields=0;unsigned int tag_count=
0;struct Cyc_List_List*fs=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(
ed->fields))->v;for(0;fs != 0;fs=fs->tl){struct Cyc_Absyn_Enumfield*_tmp18D=(
struct Cyc_Absyn_Enumfield*)fs->hd;if(((int(*)(int(*compare)(struct
_dynforward_ptr*,struct _dynforward_ptr*),struct Cyc_List_List*l,struct
_dynforward_ptr*x))Cyc_List_mem)(Cyc_strptrcmp,prev_fields,(*_tmp18D->name).f2))({
struct Cyc_String_pa_struct _tmp190;_tmp190.tag=0;_tmp190.f1=(struct
_dynforward_ptr)((struct _dynforward_ptr)*(*_tmp18D->name).f2);{void*_tmp18E[1]={&
_tmp190};Cyc_Tcutil_terr(_tmp18D->loc,({const char*_tmp18F="duplicate field name %s";
_tag_dynforward(_tmp18F,sizeof(char),_get_zero_arr_size(_tmp18F,24));}),
_tag_dynforward(_tmp18E,sizeof(void*),1));}});else{prev_fields=({struct Cyc_List_List*
_tmp191=_region_malloc(uprev_rgn,sizeof(*_tmp191));_tmp191->hd=(*_tmp18D->name).f2;
_tmp191->tl=prev_fields;_tmp191;});}if(_tmp18D->tag == 0)_tmp18D->tag=(struct Cyc_Absyn_Exp*)
Cyc_Absyn_uint_exp(tag_count,_tmp18D->loc);else{if(!Cyc_Tcutil_is_const_exp(te,(
struct Cyc_Absyn_Exp*)_check_null(_tmp18D->tag)))({struct Cyc_String_pa_struct
_tmp195;_tmp195.tag=0;_tmp195.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)*(*
_tmp18D->name).f2);{struct Cyc_String_pa_struct _tmp194;_tmp194.tag=0;_tmp194.f1=(
struct _dynforward_ptr)((struct _dynforward_ptr)*v);{void*_tmp192[2]={& _tmp194,&
_tmp195};Cyc_Tcutil_terr(loc,({const char*_tmp193="enum %s, field %s: expression is not constant";
_tag_dynforward(_tmp193,sizeof(char),_get_zero_arr_size(_tmp193,46));}),
_tag_dynforward(_tmp192,sizeof(void*),2));}}});}{unsigned int _tmp197;int _tmp198;
struct _tuple4 _tmp196=Cyc_Evexp_eval_const_uint_exp((struct Cyc_Absyn_Exp*)
_check_null(_tmp18D->tag));_tmp197=_tmp196.f1;_tmp198=_tmp196.f2;if(!_tmp198)({
void*_tmp199[0]={};Cyc_Tcutil_terr(loc,({const char*_tmp19A="Cyclone enum tags cannot use sizeof or offsetof";
_tag_dynforward(_tmp19A,sizeof(char),_get_zero_arr_size(_tmp19A,48));}),
_tag_dynforward(_tmp199,sizeof(void*),0));});tag_count=_tmp197 + 1;(*_tmp18D->name).f1=(
union Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union _tmp19B;(_tmp19B.Abs_n).tag=
2;(_tmp19B.Abs_n).f1=te->ns;_tmp19B;});}}};_pop_region(uprev_rgn);}{struct
_handler_cons _tmp19C;_push_handler(& _tmp19C);{int _tmp19E=0;if(setjmp(_tmp19C.handler))
_tmp19E=1;if(!_tmp19E){{struct Cyc_Absyn_Enumdecl**_tmp19F=((struct Cyc_Absyn_Enumdecl**(*)(
struct Cyc_Dict_Dict d,struct _dynforward_ptr*k))Cyc_Dict_lookup)(ge->enumdecls,v);
struct Cyc_Absyn_Enumdecl*_tmp1A0=Cyc_Tcdecl_merge_enumdecl(*_tmp19F,ed,loc,Cyc_Tc_tc_msg);
if(_tmp1A0 == 0){_npop_handler(0);return;}*_tmp19F=(struct Cyc_Absyn_Enumdecl*)
_tmp1A0;};_pop_handler();}else{void*_tmp19D=(void*)_exn_thrown;void*_tmp1A2=
_tmp19D;_LLF0: if(_tmp1A2 != Cyc_Dict_Absent)goto _LLF2;_LLF1: {struct Cyc_Absyn_Enumdecl**
_tmp1A3=({struct Cyc_Absyn_Enumdecl**_tmp1A4=_cycalloc(sizeof(*_tmp1A4));_tmp1A4[
0]=ed;_tmp1A4;});ge->enumdecls=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,
struct _dynforward_ptr*k,struct Cyc_Absyn_Enumdecl**v))Cyc_Dict_insert)(ge->enumdecls,
v,_tmp1A3);goto _LLEF;}_LLF2:;_LLF3:(void)_throw(_tmp1A2);_LLEF:;}}}if(ed->fields
!= 0){struct Cyc_List_List*fs=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(ed->fields))->v;for(0;fs != 0;fs=fs->tl){struct Cyc_Absyn_Enumfield*
_tmp1A5=(struct Cyc_Absyn_Enumfield*)fs->hd;ge->ordinaries=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _dynforward_ptr*k,struct _tuple7*v))Cyc_Dict_insert)(
ge->ordinaries,(*_tmp1A5->name).f2,(struct _tuple7*)({struct _tuple7*_tmp1A6=
_cycalloc(sizeof(*_tmp1A6));_tmp1A6->f1=(void*)({struct Cyc_Tcenv_EnumRes_struct*
_tmp1A7=_cycalloc(sizeof(*_tmp1A7));_tmp1A7[0]=({struct Cyc_Tcenv_EnumRes_struct
_tmp1A8;_tmp1A8.tag=3;_tmp1A8.f1=ed;_tmp1A8.f2=_tmp1A5;_tmp1A8;});_tmp1A7;});
_tmp1A6->f2=1;_tmp1A6;}));}}}static int Cyc_Tc_okay_externC(struct Cyc_Position_Segment*
loc,void*sc){void*_tmp1A9=sc;_LLF5: if((int)_tmp1A9 != 0)goto _LLF7;_LLF6:({void*
_tmp1AA[0]={};Cyc_Tcutil_warn(loc,({const char*_tmp1AB="static declaration nested within extern \"C\"";
_tag_dynforward(_tmp1AB,sizeof(char),_get_zero_arr_size(_tmp1AB,44));}),
_tag_dynforward(_tmp1AA,sizeof(void*),0));});return 0;_LLF7: if((int)_tmp1A9 != 1)
goto _LLF9;_LLF8:({void*_tmp1AC[0]={};Cyc_Tcutil_warn(loc,({const char*_tmp1AD="abstract declaration nested within extern \"C\"";
_tag_dynforward(_tmp1AD,sizeof(char),_get_zero_arr_size(_tmp1AD,46));}),
_tag_dynforward(_tmp1AC,sizeof(void*),0));});return 0;_LLF9: if((int)_tmp1A9 != 2)
goto _LLFB;_LLFA: goto _LLFC;_LLFB: if((int)_tmp1A9 != 5)goto _LLFD;_LLFC: goto _LLFE;
_LLFD: if((int)_tmp1A9 != 3)goto _LLFF;_LLFE: return 1;_LLFF: if((int)_tmp1A9 != 4)goto
_LLF4;_LL100:({void*_tmp1AE[0]={};Cyc_Tcutil_warn(loc,({const char*_tmp1AF="nested extern \"C\" declaration";
_tag_dynforward(_tmp1AF,sizeof(char),_get_zero_arr_size(_tmp1AF,30));}),
_tag_dynforward(_tmp1AE,sizeof(void*),0));});return 1;_LLF4:;}static void Cyc_Tc_resolve_export_namespace(
struct Cyc_Tcenv_Tenv*te,struct _tuple6*exp){struct Cyc_Position_Segment*_tmp1B1;
struct _tuple0*_tmp1B2;struct _tuple6 _tmp1B0=*exp;_tmp1B1=_tmp1B0.f1;_tmp1B2=
_tmp1B0.f2;{struct _tuple0 _tmp1B4;union Cyc_Absyn_Nmspace_union _tmp1B5;struct
_dynforward_ptr*_tmp1B6;struct _tuple0*_tmp1B3=_tmp1B2;_tmp1B4=*_tmp1B3;_tmp1B5=
_tmp1B4.f1;_tmp1B6=_tmp1B4.f2;{union Cyc_Absyn_Nmspace_union _tmp1B7=_tmp1B5;
struct Cyc_List_List*_tmp1B8;struct Cyc_List_List*_tmp1B9;_LL102: if((_tmp1B7.Rel_n).tag
!= 1)goto _LL104;_tmp1B8=(_tmp1B7.Rel_n).f1;if(_tmp1B8 != 0)goto _LL104;_LL103: goto
_LL105;_LL104: if((_tmp1B7.Abs_n).tag != 2)goto _LL106;_tmp1B9=(_tmp1B7.Abs_n).f1;
if(_tmp1B9 != 0)goto _LL106;_LL105: goto _LL101;_LL106:;_LL107:({struct Cyc_String_pa_struct
_tmp1BC;_tmp1BC.tag=0;_tmp1BC.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)
Cyc_Absynpp_qvar2string(_tmp1B2));{void*_tmp1BA[1]={& _tmp1BC};Cyc_Tcutil_terr(
_tmp1B1,({const char*_tmp1BB="qualified export variables are not implemented (%s)";
_tag_dynforward(_tmp1BB,sizeof(char),_get_zero_arr_size(_tmp1BB,52));}),
_tag_dynforward(_tmp1BA,sizeof(void*),1));}});return;_LL101:;}(*_tmp1B2).f1=(
union Cyc_Absyn_Nmspace_union)({union Cyc_Absyn_Nmspace_union _tmp1BD;(_tmp1BD.Abs_n).tag=
2;(_tmp1BD.Abs_n).f1=te->ns;_tmp1BD;});}}static void Cyc_Tc_tc_decls(struct Cyc_Tcenv_Tenv*
te,struct Cyc_List_List*ds0,int in_externC,int in_externCinclude,int check_var_init,
struct _RegionHandle*grgn,struct Cyc_List_List**exports){struct Cyc_Tcenv_Genv*ge=((
struct Cyc_Tcenv_Genv*(*)(struct Cyc_Dict_Dict d,struct Cyc_List_List*k))Cyc_Dict_lookup)(
te->ae,te->ns);struct Cyc_List_List*last=0;struct Cyc_Dict_Dict dict=Cyc_Tcgenrep_empty_typerep_dict();
struct Cyc_List_List*_tmp1BE=ds0;for(0;_tmp1BE != 0;(last=_tmp1BE,_tmp1BE=_tmp1BE->tl)){
struct Cyc_Absyn_Decl*d=(struct Cyc_Absyn_Decl*)_tmp1BE->hd;struct Cyc_Position_Segment*
loc=d->loc;void*_tmp1BF=(void*)d->r;struct Cyc_Absyn_Vardecl*_tmp1C0;struct Cyc_Absyn_Fndecl*
_tmp1C1;struct Cyc_Absyn_Typedefdecl*_tmp1C2;struct Cyc_Absyn_Aggrdecl*_tmp1C3;
struct Cyc_Absyn_Tuniondecl*_tmp1C4;struct Cyc_Absyn_Enumdecl*_tmp1C5;struct
_dynforward_ptr*_tmp1C6;struct Cyc_List_List*_tmp1C7;struct _tuple0*_tmp1C8;struct
_tuple0 _tmp1C9;union Cyc_Absyn_Nmspace_union _tmp1CA;struct _dynforward_ptr*_tmp1CB;
struct Cyc_List_List*_tmp1CC;struct Cyc_List_List*_tmp1CD;struct Cyc_List_List*
_tmp1CE;struct Cyc_List_List*_tmp1CF;_LL109: if(_tmp1BF <= (void*)2)goto _LL119;if(*((
int*)_tmp1BF)!= 2)goto _LL10B;_LL10A: goto _LL10C;_LL10B: if(*((int*)_tmp1BF)!= 3)
goto _LL10D;_LL10C:({void*_tmp1D0[0]={};Cyc_Tcutil_terr(loc,({const char*_tmp1D1="top level let-declarations are not implemented";
_tag_dynforward(_tmp1D1,sizeof(char),_get_zero_arr_size(_tmp1D1,47));}),
_tag_dynforward(_tmp1D0,sizeof(void*),0));});goto _LL108;_LL10D: if(*((int*)
_tmp1BF)!= 0)goto _LL10F;_tmp1C0=((struct Cyc_Absyn_Var_d_struct*)_tmp1BF)->f1;
_LL10E: if(in_externC  && Cyc_Tc_okay_externC(d->loc,(void*)_tmp1C0->sc))(void*)(
_tmp1C0->sc=(void*)((void*)4));if(_tmp1C0->initializer != 0){void*_tmp1D2=(void*)((
struct Cyc_Absyn_Exp*)_check_null(_tmp1C0->initializer))->r;void*_tmp1D3;_LL126:
if(*((int*)_tmp1D2)!= 21)goto _LL128;_tmp1D3=(void*)((struct Cyc_Absyn_Gentyp_e_struct*)
_tmp1D2)->f2;_LL127: {struct Cyc_Dict_Dict _tmp1D5;struct Cyc_List_List*_tmp1D6;
struct Cyc_Absyn_Exp*_tmp1D7;struct _tuple5 _tmp1D4=Cyc_Tcgenrep_tcGenrep(te,ge,loc,
_tmp1D3,dict);_tmp1D5=_tmp1D4.f1;_tmp1D6=_tmp1D4.f2;_tmp1D7=_tmp1D4.f3;dict=
_tmp1D5;Cyc_Tc_tc_decls(te,_tmp1D6,in_externC,in_externCinclude,check_var_init,
grgn,exports);_tmp1C0->initializer=(struct Cyc_Absyn_Exp*)_tmp1D7;Cyc_Tc_tcVardecl(
te,ge,loc,_tmp1C0,check_var_init,in_externCinclude,exports);if(_tmp1D6 != 0){if(
last != 0){((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_imp_append)(_tmp1D6,_tmp1BE);last->tl=_tmp1D6;}else{struct Cyc_List_List
tmp=({struct Cyc_List_List _tmp1D8;_tmp1D8.hd=(struct Cyc_Absyn_Decl*)_tmp1BE->hd;
_tmp1D8.tl=_tmp1BE->tl;_tmp1D8;});(struct Cyc_Absyn_Decl*)(_tmp1BE->hd=(void*)((
struct Cyc_Absyn_Decl*)_tmp1D6->hd));_tmp1BE->tl=_tmp1D6->tl;(struct Cyc_Absyn_Decl*)(
_tmp1D6->hd=(void*)((struct Cyc_Absyn_Decl*)tmp.hd));_tmp1D6->tl=tmp.tl;((struct
Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(
_tmp1BE,_tmp1D6);}}continue;}_LL128:;_LL129: goto _LL125;_LL125:;}Cyc_Tc_tcVardecl(
te,ge,loc,_tmp1C0,check_var_init,in_externCinclude,exports);goto _LL108;_LL10F:
if(*((int*)_tmp1BF)!= 1)goto _LL111;_tmp1C1=((struct Cyc_Absyn_Fn_d_struct*)
_tmp1BF)->f1;_LL110: if(in_externC  && Cyc_Tc_okay_externC(d->loc,(void*)_tmp1C1->sc))(
void*)(_tmp1C1->sc=(void*)((void*)4));Cyc_Tc_tcFndecl(te,ge,loc,_tmp1C1,
in_externCinclude,exports);goto _LL108;_LL111: if(*((int*)_tmp1BF)!= 7)goto _LL113;
_tmp1C2=((struct Cyc_Absyn_Typedef_d_struct*)_tmp1BF)->f1;_LL112: Cyc_Tc_tcTypedefdecl(
te,ge,loc,_tmp1C2);goto _LL108;_LL113: if(*((int*)_tmp1BF)!= 4)goto _LL115;_tmp1C3=((
struct Cyc_Absyn_Aggr_d_struct*)_tmp1BF)->f1;_LL114: if(in_externC  && Cyc_Tc_okay_externC(
d->loc,(void*)_tmp1C3->sc))(void*)(_tmp1C3->sc=(void*)((void*)4));Cyc_Tc_tcAggrdecl(
te,ge,loc,_tmp1C3);goto _LL108;_LL115: if(*((int*)_tmp1BF)!= 5)goto _LL117;_tmp1C4=((
struct Cyc_Absyn_Tunion_d_struct*)_tmp1BF)->f1;_LL116: if(in_externC  && Cyc_Tc_okay_externC(
d->loc,(void*)_tmp1C4->sc))(void*)(_tmp1C4->sc=(void*)((void*)4));Cyc_Tc_tcTuniondecl(
te,ge,loc,_tmp1C4);goto _LL108;_LL117: if(*((int*)_tmp1BF)!= 6)goto _LL119;_tmp1C5=((
struct Cyc_Absyn_Enum_d_struct*)_tmp1BF)->f1;_LL118: if(in_externC  && Cyc_Tc_okay_externC(
d->loc,(void*)_tmp1C5->sc))(void*)(_tmp1C5->sc=(void*)((void*)4));Cyc_Tc_tcEnumdecl(
te,ge,loc,_tmp1C5);goto _LL108;_LL119: if((int)_tmp1BF != 0)goto _LL11B;_LL11A:({
void*_tmp1D9[0]={};Cyc_Tcutil_warn(d->loc,({const char*_tmp1DA="spurious __cyclone_port_on__";
_tag_dynforward(_tmp1DA,sizeof(char),_get_zero_arr_size(_tmp1DA,29));}),
_tag_dynforward(_tmp1D9,sizeof(void*),0));});goto _LL108;_LL11B: if((int)_tmp1BF != 
1)goto _LL11D;_LL11C: goto _LL108;_LL11D: if(_tmp1BF <= (void*)2)goto _LL11F;if(*((int*)
_tmp1BF)!= 8)goto _LL11F;_tmp1C6=((struct Cyc_Absyn_Namespace_d_struct*)_tmp1BF)->f1;
_tmp1C7=((struct Cyc_Absyn_Namespace_d_struct*)_tmp1BF)->f2;_LL11E: {struct Cyc_List_List*
_tmp1DB=te->ns;struct Cyc_List_List*_tmp1DC=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_append)(_tmp1DB,({struct Cyc_List_List*_tmp1DD=
_cycalloc(sizeof(*_tmp1DD));_tmp1DD->hd=_tmp1C6;_tmp1DD->tl=0;_tmp1DD;}));if(!((
int(*)(struct Cyc_Set_Set*s,struct _dynforward_ptr*elt))Cyc_Set_member)(ge->namespaces,
_tmp1C6)){ge->namespaces=((struct Cyc_Set_Set*(*)(struct _RegionHandle*r,struct Cyc_Set_Set*
s,struct _dynforward_ptr*elt))Cyc_Set_rinsert)(grgn,ge->namespaces,_tmp1C6);te->ae=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct Cyc_List_List*k,struct Cyc_Tcenv_Genv*
v))Cyc_Dict_insert)(te->ae,_tmp1DC,Cyc_Tcenv_empty_genv(grgn));}te->ns=_tmp1DC;
Cyc_Tc_tc_decls(te,_tmp1C7,in_externC,in_externCinclude,check_var_init,grgn,
exports);te->ns=_tmp1DB;goto _LL108;}_LL11F: if(_tmp1BF <= (void*)2)goto _LL121;if(*((
int*)_tmp1BF)!= 9)goto _LL121;_tmp1C8=((struct Cyc_Absyn_Using_d_struct*)_tmp1BF)->f1;
_tmp1C9=*_tmp1C8;_tmp1CA=_tmp1C9.f1;_tmp1CB=_tmp1C9.f2;_tmp1CC=((struct Cyc_Absyn_Using_d_struct*)
_tmp1BF)->f2;_LL120: {struct _dynforward_ptr*first;struct Cyc_List_List*rest;{
union Cyc_Absyn_Nmspace_union _tmp1DE=_tmp1CA;struct Cyc_List_List*_tmp1DF;struct
Cyc_List_List*_tmp1E0;struct Cyc_List_List*_tmp1E1;struct Cyc_List_List _tmp1E2;
struct _dynforward_ptr*_tmp1E3;struct Cyc_List_List*_tmp1E4;struct Cyc_List_List*
_tmp1E5;struct Cyc_List_List _tmp1E6;struct _dynforward_ptr*_tmp1E7;struct Cyc_List_List*
_tmp1E8;_LL12B: if((_tmp1DE.Loc_n).tag != 0)goto _LL12D;_LL12C: goto _LL12E;_LL12D:
if((_tmp1DE.Rel_n).tag != 1)goto _LL12F;_tmp1DF=(_tmp1DE.Rel_n).f1;if(_tmp1DF != 0)
goto _LL12F;_LL12E: goto _LL130;_LL12F: if((_tmp1DE.Abs_n).tag != 2)goto _LL131;
_tmp1E0=(_tmp1DE.Abs_n).f1;if(_tmp1E0 != 0)goto _LL131;_LL130: first=_tmp1CB;rest=0;
goto _LL12A;_LL131: if((_tmp1DE.Rel_n).tag != 1)goto _LL133;_tmp1E1=(_tmp1DE.Rel_n).f1;
if(_tmp1E1 == 0)goto _LL133;_tmp1E2=*_tmp1E1;_tmp1E3=(struct _dynforward_ptr*)
_tmp1E2.hd;_tmp1E4=_tmp1E2.tl;_LL132: _tmp1E7=_tmp1E3;_tmp1E8=_tmp1E4;goto _LL134;
_LL133: if((_tmp1DE.Abs_n).tag != 2)goto _LL12A;_tmp1E5=(_tmp1DE.Abs_n).f1;if(
_tmp1E5 == 0)goto _LL12A;_tmp1E6=*_tmp1E5;_tmp1E7=(struct _dynforward_ptr*)_tmp1E6.hd;
_tmp1E8=_tmp1E6.tl;_LL134: first=_tmp1E7;rest=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_append)(_tmp1E8,({struct Cyc_List_List*_tmp1E9=
_cycalloc(sizeof(*_tmp1E9));_tmp1E9->hd=_tmp1CB;_tmp1E9->tl=0;_tmp1E9;}));goto
_LL12A;_LL12A:;}{struct Cyc_List_List*_tmp1EA=Cyc_Tcenv_resolve_namespace(te,loc,
first,rest);ge->availables=(struct Cyc_List_List*)({struct Cyc_List_List*_tmp1EB=
_cycalloc(sizeof(*_tmp1EB));_tmp1EB->hd=_tmp1EA;_tmp1EB->tl=ge->availables;
_tmp1EB;});Cyc_Tc_tc_decls(te,_tmp1CC,in_externC,in_externCinclude,
check_var_init,grgn,exports);ge->availables=((struct Cyc_List_List*)_check_null(
ge->availables))->tl;goto _LL108;}}_LL121: if(_tmp1BF <= (void*)2)goto _LL123;if(*((
int*)_tmp1BF)!= 10)goto _LL123;_tmp1CD=((struct Cyc_Absyn_ExternC_d_struct*)
_tmp1BF)->f1;_LL122: Cyc_Tc_tc_decls(te,_tmp1CD,1,in_externCinclude,
check_var_init,grgn,exports);goto _LL108;_LL123: if(_tmp1BF <= (void*)2)goto _LL108;
if(*((int*)_tmp1BF)!= 11)goto _LL108;_tmp1CE=((struct Cyc_Absyn_ExternCinclude_d_struct*)
_tmp1BF)->f1;_tmp1CF=((struct Cyc_Absyn_ExternCinclude_d_struct*)_tmp1BF)->f2;
_LL124:((void(*)(void(*f)(struct Cyc_Tcenv_Tenv*,struct _tuple6*),struct Cyc_Tcenv_Tenv*
env,struct Cyc_List_List*x))Cyc_List_iter_c)(Cyc_Tc_resolve_export_namespace,te,
_tmp1CF);{struct Cyc_List_List*newexs=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_append)(_tmp1CF,(unsigned int)exports?*exports:
0);Cyc_Tc_tc_decls(te,_tmp1CE,1,1,check_var_init,grgn,(struct Cyc_List_List**)&
newexs);for(0;_tmp1CF != 0;_tmp1CF=_tmp1CF->tl){struct _tuple6*_tmp1EC=(struct
_tuple6*)_tmp1CF->hd;if(!(*_tmp1EC).f3)({struct Cyc_String_pa_struct _tmp1EF;
_tmp1EF.tag=0;_tmp1EF.f1=(struct _dynforward_ptr)((struct _dynforward_ptr)Cyc_Absynpp_qvar2string((*
_tmp1EC).f2));{void*_tmp1ED[1]={& _tmp1EF};Cyc_Tcutil_warn((*_tmp1EC).f1,({const
char*_tmp1EE="%s is exported but not defined";_tag_dynforward(_tmp1EE,sizeof(
char),_get_zero_arr_size(_tmp1EE,31));}),_tag_dynforward(_tmp1ED,sizeof(void*),1));}});}
goto _LL108;}_LL108:;}}void Cyc_Tc_tc(struct _RegionHandle*g,struct Cyc_Tcenv_Tenv*
te,int check_var_init,struct Cyc_List_List*ds){Cyc_Absynpp_set_params(& Cyc_Absynpp_tc_params_r);
Cyc_Tc_tc_decls(te,ds,0,0,check_var_init,g,0);}struct Cyc_Tc_TreeshakeEnv{struct
_RegionHandle*rgn;int in_cinclude;struct Cyc_Dict_Dict nsdict;};static int Cyc_Tc_vardecl_needed(
struct Cyc_Tc_TreeshakeEnv*env,struct Cyc_Absyn_Decl*d);static struct Cyc_List_List*
Cyc_Tc_treeshake_f(struct Cyc_Tc_TreeshakeEnv*env,struct Cyc_List_List*ds){return((
struct Cyc_List_List*(*)(int(*f)(struct Cyc_Tc_TreeshakeEnv*,struct Cyc_Absyn_Decl*),
struct Cyc_Tc_TreeshakeEnv*env,struct Cyc_List_List*x))Cyc_List_filter_c)(Cyc_Tc_vardecl_needed,
env,ds);}struct _tuple12{struct Cyc_Tcenv_Genv*f1;struct Cyc_Set_Set*f2;};static int
Cyc_Tc_vardecl_needed(struct Cyc_Tc_TreeshakeEnv*env,struct Cyc_Absyn_Decl*d){void*
_tmp1F0=(void*)d->r;struct Cyc_Absyn_Vardecl*_tmp1F1;struct Cyc_List_List*_tmp1F2;
struct Cyc_List_List**_tmp1F3;struct Cyc_List_List*_tmp1F4;struct Cyc_List_List**
_tmp1F5;struct Cyc_List_List*_tmp1F6;struct Cyc_List_List**_tmp1F7;struct Cyc_List_List*
_tmp1F8;struct Cyc_List_List**_tmp1F9;_LL136: if(_tmp1F0 <= (void*)2)goto _LL140;if(*((
int*)_tmp1F0)!= 0)goto _LL138;_tmp1F1=((struct Cyc_Absyn_Var_d_struct*)_tmp1F0)->f1;
_LL137: if(env->in_cinclude  || (void*)_tmp1F1->sc != (void*)3  && (void*)_tmp1F1->sc
!= (void*)4)return 1;{struct _tuple0 _tmp1FB;union Cyc_Absyn_Nmspace_union _tmp1FC;
struct _dynforward_ptr*_tmp1FD;struct _tuple0*_tmp1FA=_tmp1F1->name;_tmp1FB=*
_tmp1FA;_tmp1FC=_tmp1FB.f1;_tmp1FD=_tmp1FB.f2;{struct Cyc_List_List*ns;{union Cyc_Absyn_Nmspace_union
_tmp1FE=_tmp1FC;struct Cyc_List_List*_tmp1FF;struct Cyc_List_List*_tmp200;_LL143:
if((_tmp1FE.Loc_n).tag != 0)goto _LL145;_LL144: ns=0;goto _LL142;_LL145: if((_tmp1FE.Rel_n).tag
!= 1)goto _LL147;_tmp1FF=(_tmp1FE.Rel_n).f1;_LL146: ns=_tmp1FF;goto _LL142;_LL147:
if((_tmp1FE.Abs_n).tag != 2)goto _LL142;_tmp200=(_tmp1FE.Abs_n).f1;_LL148: ns=
_tmp200;goto _LL142;_LL142:;}{struct _tuple12*_tmp201=((struct _tuple12*(*)(struct
Cyc_Dict_Dict d,struct Cyc_List_List*k))Cyc_Dict_lookup)(env->nsdict,ns);struct Cyc_Tcenv_Genv*
_tmp202=(*_tmp201).f1;int _tmp203=(*((struct _tuple7*(*)(struct Cyc_Dict_Dict d,
struct _dynforward_ptr*k))Cyc_Dict_lookup)(_tmp202->ordinaries,_tmp1FD)).f2;if(!
_tmp203)(*_tmp201).f2=((struct Cyc_Set_Set*(*)(struct _RegionHandle*r,struct Cyc_Set_Set*
s,struct _dynforward_ptr*elt))Cyc_Set_rinsert)(env->rgn,(*_tmp201).f2,_tmp1FD);
return _tmp203;}}}_LL138: if(*((int*)_tmp1F0)!= 10)goto _LL13A;_tmp1F2=((struct Cyc_Absyn_ExternC_d_struct*)
_tmp1F0)->f1;_tmp1F3=(struct Cyc_List_List**)&((struct Cyc_Absyn_ExternC_d_struct*)
_tmp1F0)->f1;_LL139: _tmp1F5=_tmp1F3;goto _LL13B;_LL13A: if(*((int*)_tmp1F0)!= 9)
goto _LL13C;_tmp1F4=((struct Cyc_Absyn_Using_d_struct*)_tmp1F0)->f2;_tmp1F5=(
struct Cyc_List_List**)&((struct Cyc_Absyn_Using_d_struct*)_tmp1F0)->f2;_LL13B:
_tmp1F7=_tmp1F5;goto _LL13D;_LL13C: if(*((int*)_tmp1F0)!= 8)goto _LL13E;_tmp1F6=((
struct Cyc_Absyn_Namespace_d_struct*)_tmp1F0)->f2;_tmp1F7=(struct Cyc_List_List**)&((
struct Cyc_Absyn_Namespace_d_struct*)_tmp1F0)->f2;_LL13D:*_tmp1F7=Cyc_Tc_treeshake_f(
env,*_tmp1F7);return 1;_LL13E: if(*((int*)_tmp1F0)!= 11)goto _LL140;_tmp1F8=((
struct Cyc_Absyn_ExternCinclude_d_struct*)_tmp1F0)->f1;_tmp1F9=(struct Cyc_List_List**)&((
struct Cyc_Absyn_ExternCinclude_d_struct*)_tmp1F0)->f1;_LL13F: {int in_cinclude=
env->in_cinclude;env->in_cinclude=1;*_tmp1F9=Cyc_Tc_treeshake_f(env,*_tmp1F9);
env->in_cinclude=in_cinclude;return 1;}_LL140:;_LL141: return 1;_LL135:;}static int
Cyc_Tc_treeshake_remove_f(struct Cyc_Set_Set*set,struct _dynforward_ptr*x,void*y){
return !((int(*)(struct Cyc_Set_Set*s,struct _dynforward_ptr*elt))Cyc_Set_member)(
set,x);}static struct _tuple12*Cyc_Tc_treeshake_make_env_f(struct _RegionHandle*rgn,
struct Cyc_Tcenv_Genv*ge){return({struct _tuple12*_tmp204=_region_malloc(rgn,
sizeof(*_tmp204));_tmp204->f1=ge;_tmp204->f2=((struct Cyc_Set_Set*(*)(struct
_RegionHandle*r,int(*cmp)(struct _dynforward_ptr*,struct _dynforward_ptr*)))Cyc_Set_rempty)(
rgn,Cyc_strptrcmp);_tmp204;});}struct _tuple13{struct Cyc_List_List*f1;struct
_tuple12*f2;};struct Cyc_List_List*Cyc_Tc_treeshake(struct Cyc_Tcenv_Tenv*te,
struct Cyc_List_List*ds){struct _RegionHandle _tmp205=_new_region("rgn");struct
_RegionHandle*rgn=& _tmp205;_push_region(rgn);{struct Cyc_Tc_TreeshakeEnv _tmp206=({
struct Cyc_Tc_TreeshakeEnv _tmp211;_tmp211.rgn=rgn;_tmp211.in_cinclude=0;_tmp211.nsdict=((
struct Cyc_Dict_Dict(*)(struct _RegionHandle*,struct _tuple12*(*f)(struct
_RegionHandle*,struct Cyc_Tcenv_Genv*),struct _RegionHandle*env,struct Cyc_Dict_Dict
d))Cyc_Dict_rmap_c)(rgn,Cyc_Tc_treeshake_make_env_f,rgn,te->ae);_tmp211;});
struct Cyc_List_List*_tmp207=Cyc_Tc_treeshake_f(& _tmp206,ds);if(((int(*)(struct
Cyc_Dict_Dict d))Cyc_Dict_is_empty)(_tmp206.nsdict)){struct Cyc_List_List*_tmp208=
_tmp207;_npop_handler(0);return _tmp208;}{struct Cyc_Iter_Iter _tmp209=((struct Cyc_Iter_Iter(*)(
struct _RegionHandle*rgn,struct Cyc_Dict_Dict d))Cyc_Dict_make_iter)(Cyc_Core_heap_region,
_tmp206.nsdict);struct _tuple13 _tmp20A=*((struct _tuple13*(*)(struct _RegionHandle*
r,struct Cyc_Dict_Dict d))Cyc_Dict_rchoose)(rgn,_tmp206.nsdict);while(((int(*)(
struct Cyc_Iter_Iter,struct _tuple13*))Cyc_Iter_next)(_tmp209,& _tmp20A)){struct
_tuple12*_tmp20C;struct _tuple12 _tmp20D;struct Cyc_Tcenv_Genv*_tmp20E;struct Cyc_Set_Set*
_tmp20F;struct _tuple13 _tmp20B=_tmp20A;_tmp20C=_tmp20B.f2;_tmp20D=*_tmp20C;
_tmp20E=_tmp20D.f1;_tmp20F=_tmp20D.f2;_tmp20E->ordinaries=((struct Cyc_Dict_Dict(*)(
struct _RegionHandle*,int(*f)(struct Cyc_Set_Set*,struct _dynforward_ptr*,struct
_tuple7*),struct Cyc_Set_Set*env,struct Cyc_Dict_Dict d))Cyc_Dict_rfilter_c)(
_tmp20E->grgn,(int(*)(struct Cyc_Set_Set*set,struct _dynforward_ptr*x,struct
_tuple7*y))Cyc_Tc_treeshake_remove_f,_tmp20F,_tmp20E->ordinaries);}{struct Cyc_List_List*
_tmp210=_tmp207;_npop_handler(0);return _tmp210;}}};_pop_region(rgn);}
