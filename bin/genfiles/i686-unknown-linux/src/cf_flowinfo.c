// This is a C header file to be used by the output of the Cyclone
// to C translator.  The corresponding definitions are in file lib/runtime_cyc.c
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

#include <setjmp.h>

#ifdef NO_CYC_PREFIX
#define ADD_PREFIX(x) x
#else
#define ADD_PREFIX(x) Cyc_##x
#endif

#ifndef offsetof
// should be size_t, but int is fine.
#define offsetof(t,n) ((int)(&(((t *)0)->n)))
#endif

//// Tagged arrays
struct _tagged_arr { 
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};

//// Discriminated Unions
struct _xtunion_struct { char *tag; };

// Need one of these per thread (we don't have threads)
// The runtime maintains a stack that contains either _handler_cons
// structs or _RegionHandle structs.  The tag is 0 for a handler_cons
// and 1 for a region handle.  
struct _RuntimeStack {
  int tag; // 0 for an exception handler, 1 for a region handle
  struct _RuntimeStack *next;
};

//// Regions
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
#ifdef CYC_REGION_PROFILE
  const char         *name;
#endif
};

extern struct _RegionHandle _new_region(const char *);
extern void * _region_malloc(struct _RegionHandle *, unsigned);
extern void * _region_calloc(struct _RegionHandle *, unsigned t, unsigned n);
extern void   _free_region(struct _RegionHandle *);
extern void   _reset_region(struct _RegionHandle *);

//// Exceptions 
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

//// Built-in Exceptions
extern struct _xtunion_struct ADD_PREFIX(Null_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Null_Exception);
extern struct _xtunion_struct ADD_PREFIX(Array_bounds_struct);
extern struct _xtunion_struct * ADD_PREFIX(Array_bounds);
extern struct _xtunion_struct ADD_PREFIX(Match_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Match_Exception);
extern struct _xtunion_struct ADD_PREFIX(Bad_alloc_struct);
extern struct _xtunion_struct * ADD_PREFIX(Bad_alloc);

//// Built-in Run-time Checks and company
#ifdef __APPLE__
#define _INLINE_FUNCTIONS
#endif

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

#ifdef NO_CYC_BOUNDS_CHECKS
#ifdef _INLINE_FUNCTIONS
static inline unsigned char *
_check_unknown_subscript(struct _tagged_arr arr,unsigned elt_sz,unsigned index) {
  struct _tagged_arr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  return _cus_ans;
}
#else
#define _check_unknown_subscript(arr,elt_sz,index) ({ \
  struct _tagged_arr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
#endif
#else
#ifdef _INLINE_FUNCTIONS
static inline unsigned char *
_check_unknown_subscript(struct _tagged_arr arr,unsigned elt_sz,unsigned index) {
  struct _tagged_arr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  if (!_cus_arr.base) _throw_null();
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds();
  return _cus_ans;
}
#else
#define _check_unknown_subscript(arr,elt_sz,index) ({ \
  struct _tagged_arr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  if (!_cus_arr.base) _throw_null(); \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr
_tag_arr(const void *tcurr,unsigned elt_sz,unsigned num_elts) {
  struct _tagged_arr _tag_arr_ans;
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr);
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts);
  return _tag_arr_ans;
}
#else
#define _tag_arr(tcurr,elt_sz,num_elts) ({ \
  struct _tagged_arr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr *
_init_tag_arr(struct _tagged_arr *arr_ptr,
              void *arr, unsigned elt_sz, unsigned num_elts) {
  struct _tagged_arr *_itarr_ptr = (arr_ptr);
  void* _itarr = (arr);
  _itarr_ptr->base = _itarr_ptr->curr = _itarr;
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts);
  return _itarr_ptr;
}
#else
#define _init_tag_arr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _tagged_arr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->base = _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_arr(arr,elt_sz,num_elts) ((arr).curr)
#else
#ifdef _INLINE_FUNCTIONS
static inline unsigned char *
_untag_arr(struct _tagged_arr arr, unsigned elt_sz,unsigned num_elts) {
  struct _tagged_arr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds();
  return _curr;
}
#else
#define _untag_arr(arr,elt_sz,num_elts) ({ \
  struct _tagged_arr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
    _throw_arraybounds(); \
  _curr; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static inline unsigned
_get_arr_size(struct _tagged_arr arr,unsigned elt_sz) {
  struct _tagged_arr _get_arr_size_temp = (arr);
  unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr;
  unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one;
  return (_get_arr_size_curr < _get_arr_size_temp.base ||
          _get_arr_size_curr >= _get_arr_size_last) ? 0 :
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));
}
#else
#define _get_arr_size(arr,elt_sz) \
  ({struct _tagged_arr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr
_tagged_arr_plus(struct _tagged_arr arr,unsigned elt_sz,int change) {
  struct _tagged_arr _ans = (arr);
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _tagged_arr_plus(arr,elt_sz,change) ({ \
  struct _tagged_arr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr
_tagged_arr_inplace_plus(struct _tagged_arr *arr_ptr,unsigned elt_sz,int change) {
  struct _tagged_arr * _arr_ptr = (arr_ptr);
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return *_arr_ptr;
}
#else
#define _tagged_arr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _tagged_arr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })
#endif

#ifdef _INLINE_FUNCTIONS
static inline struct _tagged_arr
_tagged_arr_inplace_plus_post(struct _tagged_arr *arr_ptr,unsigned elt_sz,int change) {
  struct _tagged_arr * _arr_ptr = (arr_ptr);
  struct _tagged_arr _ans = *_arr_ptr;
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _tagged_arr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _tagged_arr * _arr_ptr = (arr_ptr); \
  struct _tagged_arr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

// Decrease the upper bound on a fat pointer by numelts where sz is
// the size of the pointer's type.  Note that this can't be a macro
// if we're to get initializers right.
static struct _tagged_arr _tagged_ptr_decrease_size(struct _tagged_arr x,
                                                    unsigned int sz,
                                                    unsigned int numelts) {
  x.last_plus_one -= sz * numelts; 
  return x; 
}

// Add i to zero-terminated pointer x.  Checks for x being null and
// ensures that x[0..i-1] are not 0.
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

// Calculates the number of elements in a zero-terminated, thin array.
// If non-null, the array is guaranteed to have orig_offset elements.
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

// Does in-place addition of a zero-terminated pointer (x += e and ++x).  
// Note that this expands to call _zero_arr_plus.
#define _zero_arr_inplace_plus(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  *_zap_x = _zero_arr_plus(*_zap_x,1,(orig_i)); })

// Does in-place increment of a zero-terminated pointer (e.g., x++).
// Note that this expands to call _zero_arr_plus.
#define _zero_arr_inplace_plus_post(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  _zap_tx *_zap_res = *_zap_x; \
  *_zap_x = _zero_arr_plus(_zap_res,1,(orig_i)); \
  _zap_res; })
  
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
 struct Cyc_Core_Opt{void*v;};extern char Cyc_Core_Invalid_argument[21];struct Cyc_Core_Invalid_argument_struct{
char*tag;struct _tagged_arr f1;};extern char Cyc_Core_Failure[12];struct Cyc_Core_Failure_struct{
char*tag;struct _tagged_arr f1;};extern char Cyc_Core_Impossible[15];struct Cyc_Core_Impossible_struct{
char*tag;struct _tagged_arr f1;};extern char Cyc_Core_Not_found[14];extern char Cyc_Core_Unreachable[
16];struct Cyc_Core_Unreachable_struct{char*tag;struct _tagged_arr f1;};struct Cyc_List_List{
void*hd;struct Cyc_List_List*tl;};struct Cyc_List_List*Cyc_List_list(struct
_tagged_arr);void*Cyc_List_hd(struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_tl(
struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[18];void Cyc_List_iter(
void(*f)(void*),struct Cyc_List_List*x);extern char Cyc_List_Nth[8];int Cyc_List_exists_c(
int(*pred)(void*,void*),void*env,struct Cyc_List_List*x);int Cyc_List_mem(int(*
compare)(void*,void*),struct Cyc_List_List*l,void*x);int Cyc_List_list_cmp(int(*
cmp)(void*,void*),struct Cyc_List_List*l1,struct Cyc_List_List*l2);struct Cyc_Iter_Iter{
void*env;int(*next)(void*env,void*dest);};int Cyc_Iter_next(struct Cyc_Iter_Iter,
void*);struct Cyc_Set_Set;struct Cyc_Set_Set*Cyc_Set_empty(int(*cmp)(void*,void*));
struct Cyc_Set_Set*Cyc_Set_insert(struct Cyc_Set_Set*s,void*elt);int Cyc_Set_member(
struct Cyc_Set_Set*s,void*elt);extern char Cyc_Set_Absent[11];struct Cyc_Iter_Iter
Cyc_Set_make_iter(struct _RegionHandle*rgn,struct Cyc_Set_Set*s);typedef struct{int
__count;union{unsigned int __wch;char __wchb[4];}__value;}Cyc___mbstate_t;typedef
struct{int __pos;Cyc___mbstate_t __state;}Cyc__G_fpos_t;typedef Cyc__G_fpos_t Cyc_fpos_t;
struct Cyc___cycFILE;extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_Cstdio___abstractFILE;
struct Cyc_String_pa_struct{int tag;struct _tagged_arr f1;};struct Cyc_Int_pa_struct{
int tag;unsigned int f1;};struct Cyc_Double_pa_struct{int tag;double f1;};struct Cyc_LongDouble_pa_struct{
int tag;long double f1;};struct Cyc_ShortPtr_pa_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_struct{
int tag;unsigned int*f1;};struct _tagged_arr Cyc_aprintf(struct _tagged_arr,struct
_tagged_arr);int Cyc_fprintf(struct Cyc___cycFILE*,struct _tagged_arr,struct
_tagged_arr);struct Cyc_ShortPtr_sa_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_struct{
int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_struct{
int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_struct{int tag;struct _tagged_arr
f1;};struct Cyc_DoublePtr_sa_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_struct{
int tag;float*f1;};struct Cyc_CharPtr_sa_struct{int tag;struct _tagged_arr f1;};
extern char Cyc_FileCloseError[19];extern char Cyc_FileOpenError[18];struct Cyc_FileOpenError_struct{
char*tag;struct _tagged_arr f1;};struct Cyc_Dict_Dict;extern char Cyc_Dict_Present[12];
extern char Cyc_Dict_Absent[11];struct Cyc_Dict_Dict*Cyc_Dict_empty(int(*cmp)(void*,
void*));struct Cyc_Dict_Dict*Cyc_Dict_insert(struct Cyc_Dict_Dict*d,void*k,void*v);
void*Cyc_Dict_lookup(struct Cyc_Dict_Dict*d,void*k);void*Cyc_Dict_fold_c(void*(*f)(
void*,void*,void*,void*),void*env,struct Cyc_Dict_Dict*d,void*accum);void Cyc_Dict_iter_c(
void(*f)(void*,void*,void*),void*env,struct Cyc_Dict_Dict*d);struct Cyc_Dict_Dict*
Cyc_Dict_map_c(void*(*f)(void*,void*),void*env,struct Cyc_Dict_Dict*d);struct Cyc_Dict_Dict*
Cyc_Dict_union_two_c(void*(*f)(void*,void*,void*,void*),void*env,struct Cyc_Dict_Dict*
d1,struct Cyc_Dict_Dict*d2);struct Cyc_Dict_Dict*Cyc_Dict_intersect_c(void*(*f)(
void*,void*,void*,void*),void*env,struct Cyc_Dict_Dict*d1,struct Cyc_Dict_Dict*d2);
int Cyc_Dict_forall_c(int(*f)(void*,void*,void*),void*env,struct Cyc_Dict_Dict*d);
int Cyc_Dict_forall_intersect(int(*f)(void*,void*,void*),struct Cyc_Dict_Dict*d1,
struct Cyc_Dict_Dict*d2);struct _tuple0{void*f1;void*f2;};struct _tuple0*Cyc_Dict_rchoose(
struct _RegionHandle*r,struct Cyc_Dict_Dict*d);struct _tuple0*Cyc_Dict_rchoose(
struct _RegionHandle*,struct Cyc_Dict_Dict*d);int Cyc_strptrcmp(struct _tagged_arr*
s1,struct _tagged_arr*s2);struct Cyc_Lineno_Pos{struct _tagged_arr logical_file;
struct _tagged_arr line;int line_no;int col;};extern char Cyc_Position_Exit[9];struct
Cyc_Position_Segment;struct Cyc_Position_Error{struct _tagged_arr source;struct Cyc_Position_Segment*
seg;void*kind;struct _tagged_arr desc;};extern char Cyc_Position_Nocontext[14];
struct Cyc_Absyn_Rel_n_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Abs_n_struct{
int tag;struct Cyc_List_List*f1;};struct _tuple1{void*f1;struct _tagged_arr*f2;};
struct Cyc_Absyn_Conref;struct Cyc_Absyn_Tqual{int q_const: 1;int q_volatile: 1;int
q_restrict: 1;};struct Cyc_Absyn_Conref{void*v;};struct Cyc_Absyn_Eq_constr_struct{
int tag;void*f1;};struct Cyc_Absyn_Forward_constr_struct{int tag;struct Cyc_Absyn_Conref*
f1;};struct Cyc_Absyn_Eq_kb_struct{int tag;void*f1;};struct Cyc_Absyn_Unknown_kb_struct{
int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_struct{int tag;struct Cyc_Core_Opt*
f1;void*f2;};struct Cyc_Absyn_Tvar{struct _tagged_arr*name;int*identity;void*kind;
};struct Cyc_Absyn_Upper_b_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AbsUpper_b_struct{
int tag;void*f1;};struct Cyc_Absyn_PtrAtts{void*rgn;struct Cyc_Absyn_Conref*
nullable;struct Cyc_Absyn_Conref*bounds;struct Cyc_Absyn_Conref*zero_term;};struct
Cyc_Absyn_PtrInfo{void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts
ptr_atts;};struct Cyc_Absyn_VarargInfo{struct Cyc_Core_Opt*name;struct Cyc_Absyn_Tqual
tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;struct
Cyc_Core_Opt*effect;void*ret_typ;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*
cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;};struct
Cyc_Absyn_UnknownTunionInfo{struct _tuple1*name;int is_xtunion;};struct Cyc_Absyn_UnknownTunion_struct{
int tag;struct Cyc_Absyn_UnknownTunionInfo f1;};struct Cyc_Absyn_KnownTunion_struct{
int tag;struct Cyc_Absyn_Tuniondecl**f1;};struct Cyc_Absyn_TunionInfo{void*
tunion_info;struct Cyc_List_List*targs;void*rgn;};struct Cyc_Absyn_UnknownTunionFieldInfo{
struct _tuple1*tunion_name;struct _tuple1*field_name;int is_xtunion;};struct Cyc_Absyn_UnknownTunionfield_struct{
int tag;struct Cyc_Absyn_UnknownTunionFieldInfo f1;};struct Cyc_Absyn_KnownTunionfield_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Absyn_TunionFieldInfo{
void*field_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownAggr_struct{
int tag;void*f1;struct _tuple1*f2;};struct Cyc_Absyn_KnownAggr_struct{int tag;struct
Cyc_Absyn_Aggrdecl**f1;};struct Cyc_Absyn_AggrInfo{void*aggr_info;struct Cyc_List_List*
targs;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct
Cyc_Absyn_Exp*num_elts;struct Cyc_Absyn_Conref*zero_term;};struct Cyc_Absyn_Evar_struct{
int tag;struct Cyc_Core_Opt*f1;struct Cyc_Core_Opt*f2;int f3;struct Cyc_Core_Opt*f4;}
;struct Cyc_Absyn_VarType_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_TunionType_struct{
int tag;struct Cyc_Absyn_TunionInfo f1;};struct Cyc_Absyn_TunionFieldType_struct{int
tag;struct Cyc_Absyn_TunionFieldInfo f1;};struct Cyc_Absyn_PointerType_struct{int
tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_struct{int tag;void*f1;
void*f2;};struct Cyc_Absyn_DoubleType_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_struct{
int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_struct{int tag;struct
Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_AggrType_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct
Cyc_Absyn_AnonAggrType_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_struct{
int tag;struct _tuple1*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_SizeofType_struct{int tag;void*f1;
};struct Cyc_Absyn_RgnHandleType_struct{int tag;void*f1;};struct Cyc_Absyn_TypedefType_struct{
int tag;struct _tuple1*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;
void**f4;};struct Cyc_Absyn_TagType_struct{int tag;void*f1;};struct Cyc_Absyn_TypeInt_struct{
int tag;int f1;};struct Cyc_Absyn_AccessEff_struct{int tag;void*f1;};struct Cyc_Absyn_JoinEff_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnsEff_struct{int tag;void*f1;};
struct Cyc_Absyn_NoTypes_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*
f2;};struct Cyc_Absyn_WithTypes_struct{int tag;struct Cyc_List_List*f1;int f2;struct
Cyc_Absyn_VarargInfo*f3;struct Cyc_Core_Opt*f4;struct Cyc_List_List*f5;};struct Cyc_Absyn_Regparm_att_struct{
int tag;int f1;};struct Cyc_Absyn_Aligned_att_struct{int tag;int f1;};struct Cyc_Absyn_Section_att_struct{
int tag;struct _tagged_arr f1;};struct Cyc_Absyn_Format_att_struct{int tag;void*f1;
int f2;int f3;};struct Cyc_Absyn_Initializes_att_struct{int tag;int f1;};struct Cyc_Absyn_Carray_mod_struct{
int tag;struct Cyc_Absyn_Conref*f1;};struct Cyc_Absyn_ConstArray_mod_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Conref*f2;};struct Cyc_Absyn_Pointer_mod_struct{
int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_struct{
int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_struct{int tag;struct Cyc_List_List*
f1;struct Cyc_Position_Segment*f2;int f3;};struct Cyc_Absyn_Attributes_mod_struct{
int tag;struct Cyc_Position_Segment*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Char_c_struct{
int tag;void*f1;char f2;};struct Cyc_Absyn_Short_c_struct{int tag;void*f1;short f2;};
struct Cyc_Absyn_Int_c_struct{int tag;void*f1;int f2;};struct Cyc_Absyn_LongLong_c_struct{
int tag;void*f1;long long f2;};struct Cyc_Absyn_Float_c_struct{int tag;struct
_tagged_arr f1;};struct Cyc_Absyn_String_c_struct{int tag;struct _tagged_arr f1;};
struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;
struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_struct{int tag;
struct _tagged_arr*f1;};struct Cyc_Absyn_TupleIndex_struct{int tag;unsigned int f1;}
;struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;
struct Cyc_Absyn_Exp*num_elts;int fat_result;};struct Cyc_Absyn_Const_e_struct{int
tag;void*f1;};struct Cyc_Absyn_Var_e_struct{int tag;struct _tuple1*f1;void*f2;};
struct Cyc_Absyn_UnknownId_e_struct{int tag;struct _tuple1*f1;};struct Cyc_Absyn_Primop_e_struct{
int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;void*f2;};struct Cyc_Absyn_Conditional_e_struct{int
tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};
struct Cyc_Absyn_SeqExp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*
f2;};struct Cyc_Absyn_UnknownCall_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_FnCall_e_struct{int tag;struct Cyc_Absyn_Exp*f1;
struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;};struct Cyc_Absyn_Throw_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoInstantiate_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_struct{int tag;void*f1;struct
Cyc_Absyn_Exp*f2;int f3;void*f4;};struct Cyc_Absyn_Address_e_struct{int tag;struct
Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_struct{int tag;struct Cyc_Absyn_Exp*f1;
struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_struct{int tag;void*f1;};
struct Cyc_Absyn_Sizeofexp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_struct{
int tag;void*f1;void*f2;};struct Cyc_Absyn_Gentyp_e_struct{int tag;struct Cyc_List_List*
f1;void*f2;};struct Cyc_Absyn_Deref_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_AggrMember_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
_tagged_arr*f2;};struct Cyc_Absyn_AggrArrow_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct _tagged_arr*f2;};struct Cyc_Absyn_Subscript_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_struct{int tag;struct Cyc_List_List*
f1;};struct _tuple2{struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Tqual f2;void*f3;};
struct Cyc_Absyn_CompoundLit_e_struct{int tag;struct _tuple2*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Array_e_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;
int f4;};struct Cyc_Absyn_Struct_e_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*
f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_struct{
int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Tunion_e_struct{int tag;
struct Cyc_List_List*f1;struct Cyc_Absyn_Tuniondecl*f2;struct Cyc_Absyn_Tunionfield*
f3;};struct Cyc_Absyn_Enum_e_struct{int tag;struct _tuple1*f1;struct Cyc_Absyn_Enumdecl*
f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_AnonEnum_e_struct{int tag;
struct _tuple1*f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_Malloc_e_struct{
int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_UnresolvedMem_e_struct{int
tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Codegen_e_struct{int tag;struct
Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Fill_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_Exp{struct Cyc_Core_Opt*topt;void*r;struct Cyc_Position_Segment*
loc;void*annot;};struct _tuple3{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};
struct Cyc_Absyn_ForArrayInfo{struct Cyc_List_List*defns;struct _tuple3 condition;
struct _tuple3 delta;struct Cyc_Absyn_Stmt*body;};struct Cyc_Absyn_Exp_s_struct{int
tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_IfThenElse_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*
f2;struct Cyc_Absyn_Stmt*f3;};struct Cyc_Absyn_While_s_struct{int tag;struct _tuple3
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;};struct Cyc_Absyn_Continue_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct
Cyc_Absyn_Goto_s_struct{int tag;struct _tagged_arr*f1;struct Cyc_Absyn_Stmt*f2;};
struct Cyc_Absyn_For_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple3 f2;
struct _tuple3 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_SwitchC_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Fallthru_s_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_struct{
int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Cut_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Splice_s_struct{int tag;struct
Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Label_s_struct{int tag;struct _tagged_arr*f1;
struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct _tuple3 f2;};struct Cyc_Absyn_TryCatch_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Region_s_struct{int tag;struct Cyc_Absyn_Tvar*
f1;struct Cyc_Absyn_Vardecl*f2;int f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_ForArray_s_struct{
int tag;struct Cyc_Absyn_ForArrayInfo f1;};struct Cyc_Absyn_ResetRegion_s_struct{int
tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Stmt{void*r;struct Cyc_Position_Segment*
loc;struct Cyc_List_List*non_local_preds;int try_depth;void*annot;};struct Cyc_Absyn_Var_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Reference_p_struct{int tag;
struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_TagInt_p_struct{int tag;struct Cyc_Absyn_Tvar*
f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_Pointer_p_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_struct{
int tag;struct Cyc_Absyn_AggrInfo f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;
};struct Cyc_Absyn_Tunion_p_struct{int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*
f2;struct Cyc_List_List*f3;};struct Cyc_Absyn_Int_p_struct{int tag;void*f1;int f2;};
struct Cyc_Absyn_Char_p_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_struct{int
tag;struct _tagged_arr f1;};struct Cyc_Absyn_Enum_p_struct{int tag;struct Cyc_Absyn_Enumdecl*
f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_struct{int tag;void*
f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_struct{int tag;
struct _tuple1*f1;};struct Cyc_Absyn_UnknownCall_p_struct{int tag;struct _tuple1*f1;
struct Cyc_List_List*f2;};struct Cyc_Absyn_Pat{void*r;struct Cyc_Core_Opt*topt;
struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*
pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*
body;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_SwitchC_clause{struct Cyc_Absyn_Exp*
cnst_exp;struct Cyc_Absyn_Stmt*body;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Global_b_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_struct{int tag;
struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_struct{int tag;struct Cyc_Absyn_Vardecl*
f1;};struct Cyc_Absyn_Local_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Pat_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{
void*sc;struct _tuple1*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*
initializer;struct Cyc_Core_Opt*rgn;struct Cyc_List_List*attributes;int escapes;};
struct Cyc_Absyn_Fndecl{void*sc;int is_inline;struct _tuple1*name;struct Cyc_List_List*
tvs;struct Cyc_Core_Opt*effect;void*ret_type;struct Cyc_List_List*args;int
c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;
struct Cyc_Absyn_Stmt*body;struct Cyc_Core_Opt*cached_typ;struct Cyc_Core_Opt*
param_vardecls;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Aggrfield{struct
_tagged_arr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;
struct Cyc_List_List*attributes;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*
exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;};struct Cyc_Absyn_Aggrdecl{
void*kind;void*sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*
impl;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Tunionfield{struct _tuple1*
name;struct Cyc_List_List*typs;struct Cyc_Position_Segment*loc;void*sc;};struct Cyc_Absyn_Tuniondecl{
void*sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int
is_xtunion;};struct Cyc_Absyn_Enumfield{struct _tuple1*name;struct Cyc_Absyn_Exp*
tag;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Enumdecl{void*sc;struct
_tuple1*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct
_tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;struct Cyc_Core_Opt*
defn;};struct Cyc_Absyn_Var_d_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Fn_d_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_struct{
int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};
struct Cyc_Absyn_Letv_d_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Aggr_d_struct{
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Tunion_d_struct{int tag;
struct Cyc_Absyn_Tuniondecl*f1;};struct Cyc_Absyn_Enum_d_struct{int tag;struct Cyc_Absyn_Enumdecl*
f1;};struct Cyc_Absyn_Typedef_d_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};
struct Cyc_Absyn_Namespace_d_struct{int tag;struct _tagged_arr*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Using_d_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_ExternC_d_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Decl{
void*r;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_ArrayElement_struct{int
tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_struct{int tag;struct
_tagged_arr*f1;};extern char Cyc_Absyn_EmptyAnnot[15];int Cyc_Absyn_tvar_cmp(struct
Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*);void*Cyc_Absyn_conref_def(void*,struct Cyc_Absyn_Conref*
x);struct _tagged_arr*Cyc_Absyn_fieldname(int);struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_get_known_aggrdecl(
void*info);int Cyc_Absyn_is_union_type(void*);struct Cyc_RgnOrder_RgnPO;struct Cyc_RgnOrder_RgnPO*
Cyc_RgnOrder_initial_fn_po(struct Cyc_List_List*tvs,struct Cyc_List_List*po,void*
effect,struct Cyc_Absyn_Tvar*fst_rgn);struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(
struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*rgn,int resetable);int Cyc_RgnOrder_is_region_resetable(
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*r);int Cyc_RgnOrder_effect_outlives(
struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);int Cyc_RgnOrder_satisfies_constraints(
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_List_List*constraints,void*default_bound,
int do_pin);int Cyc_RgnOrder_eff_outlives_eff(struct Cyc_RgnOrder_RgnPO*po,void*
eff1,void*eff2);struct Cyc_Tcenv_VarRes_struct{int tag;void*f1;};struct Cyc_Tcenv_AggrRes_struct{
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_TunionRes_struct{int tag;
struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Tcenv_EnumRes_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_struct{
int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_Genv{struct Cyc_Set_Set*
namespaces;struct Cyc_Dict_Dict*aggrdecls;struct Cyc_Dict_Dict*tuniondecls;struct
Cyc_Dict_Dict*enumdecls;struct Cyc_Dict_Dict*typedefs;struct Cyc_Dict_Dict*
ordinaries;struct Cyc_List_List*availables;};struct Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Stmt_j_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Tcenv_Outermost_struct{int tag;void*f1;
};struct Cyc_Tcenv_Frame_struct{int tag;void*f1;void*f2;};struct Cyc_Tcenv_Hidden_struct{
int tag;void*f1;void*f2;};struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict*
ae;struct Cyc_Core_Opt*le;};void Cyc_Tcutil_terr(struct Cyc_Position_Segment*,
struct _tagged_arr fmt,struct _tagged_arr ap);void*Cyc_Tcutil_compress(void*t);int
Cyc_Tcutil_typecmp(void*,void*);int Cyc_Tcutil_bits_only(void*t);struct Cyc_PP_Ppstate;
struct Cyc_PP_Out;struct Cyc_PP_Doc;struct Cyc_Absynpp_Params{int expand_typedefs: 1;
int qvar_to_Cids: 1;int add_cyc_prefix: 1;int to_VC: 1;int decls_first: 1;int
rewrite_temp_tvars: 1;int print_all_tvars: 1;int print_all_kinds: 1;int
print_all_effects: 1;int print_using_stmts: 1;int print_externC_stmts: 1;int
print_full_evars: 1;int print_zeroterm: 1;int generate_line_directives: 1;int
use_curr_namespace: 1;struct Cyc_List_List*curr_namespace;};struct _tagged_arr Cyc_Absynpp_qvar2string(
struct _tuple1*);struct Cyc_CfFlowInfo_VarRoot_struct{int tag;struct Cyc_Absyn_Vardecl*
f1;};struct Cyc_CfFlowInfo_MallocPt_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;
};struct Cyc_CfFlowInfo_InitParam_struct{int tag;int f1;void*f2;};struct Cyc_CfFlowInfo_Place{
void*root;struct Cyc_List_List*fields;};struct Cyc_CfFlowInfo_EqualConst_struct{
int tag;unsigned int f1;};struct Cyc_CfFlowInfo_LessVar_struct{int tag;struct Cyc_Absyn_Vardecl*
f1;};struct Cyc_CfFlowInfo_LessSize_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};
struct Cyc_CfFlowInfo_LessConst_struct{int tag;unsigned int f1;};struct Cyc_CfFlowInfo_LessEqSize_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_CfFlowInfo_Reln{struct Cyc_Absyn_Vardecl*
vd;void*rop;};struct Cyc_CfFlowInfo_TagCmp{void*cmp;void*bd;};char Cyc_CfFlowInfo_HasTagCmps[
15]="\000\000\000\000HasTagCmps\000";struct Cyc_CfFlowInfo_HasTagCmps_struct{char*
tag;struct Cyc_List_List*f1;};char Cyc_CfFlowInfo_IsZero[11]="\000\000\000\000IsZero\000";
char Cyc_CfFlowInfo_NotZero[12]="\000\000\000\000NotZero\000";struct Cyc_CfFlowInfo_NotZero_struct{
char*tag;struct Cyc_List_List*f1;};char Cyc_CfFlowInfo_UnknownZ[13]="\000\000\000\000UnknownZ\000";
struct Cyc_CfFlowInfo_UnknownZ_struct{char*tag;struct Cyc_List_List*f1;};struct Cyc_CfFlowInfo_PlaceL_struct{
int tag;struct Cyc_CfFlowInfo_Place*f1;};struct Cyc_CfFlowInfo_UnknownR_struct{int
tag;void*f1;};struct Cyc_CfFlowInfo_Esc_struct{int tag;void*f1;};struct Cyc_CfFlowInfo_AddressOf_struct{
int tag;struct Cyc_CfFlowInfo_Place*f1;};struct Cyc_CfFlowInfo_TagCmps_struct{int
tag;struct Cyc_List_List*f1;};struct Cyc_CfFlowInfo_Aggregate_struct{int tag;struct
Cyc_Dict_Dict*f1;};struct Cyc_CfFlowInfo_ReachableFL_struct{int tag;struct Cyc_Dict_Dict*
f1;struct Cyc_List_List*f2;};struct Cyc_Set_Set*Cyc_CfFlowInfo_mt_place_set();
extern void*Cyc_CfFlowInfo_unknown_none;extern void*Cyc_CfFlowInfo_unknown_this;
extern void*Cyc_CfFlowInfo_unknown_all;extern void*Cyc_CfFlowInfo_esc_none;extern
void*Cyc_CfFlowInfo_esc_this;extern void*Cyc_CfFlowInfo_esc_all;int Cyc_CfFlowInfo_root_cmp(
void*,void*);int Cyc_CfFlowInfo_place_cmp(struct Cyc_CfFlowInfo_Place*,struct Cyc_CfFlowInfo_Place*);
void*Cyc_CfFlowInfo_typ_to_absrval(void*t,void*leafval);void*Cyc_CfFlowInfo_initlevel(
struct Cyc_Dict_Dict*d,void*r);void*Cyc_CfFlowInfo_lookup_place(struct Cyc_Dict_Dict*
d,struct Cyc_CfFlowInfo_Place*place);int Cyc_CfFlowInfo_is_unescaped(struct Cyc_Dict_Dict*
d,struct Cyc_CfFlowInfo_Place*place);int Cyc_CfFlowInfo_flow_lessthan_approx(void*
f1,void*f2);struct Cyc_List_List*Cyc_CfFlowInfo_reln_assign_var(struct Cyc_List_List*,
struct Cyc_Absyn_Vardecl*,struct Cyc_Absyn_Exp*);struct Cyc_List_List*Cyc_CfFlowInfo_reln_assign_exp(
struct Cyc_List_List*,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*);struct Cyc_List_List*
Cyc_CfFlowInfo_reln_kill_var(struct Cyc_List_List*,struct Cyc_Absyn_Vardecl*);
struct Cyc_List_List*Cyc_CfFlowInfo_reln_kill_exp(struct Cyc_List_List*,struct Cyc_Absyn_Exp*);
void Cyc_CfFlowInfo_print_relns(struct Cyc_List_List*);struct Cyc_Dict_Dict*Cyc_CfFlowInfo_escape_deref(
struct Cyc_Dict_Dict*d,struct Cyc_Set_Set**all_changed,void*r);struct Cyc_Dict_Dict*
Cyc_CfFlowInfo_assign_place(struct Cyc_Position_Segment*loc,struct Cyc_Dict_Dict*d,
struct Cyc_Set_Set**all_changed,struct Cyc_CfFlowInfo_Place*place,void*r);void*Cyc_CfFlowInfo_join_flow(
struct Cyc_Set_Set**,void*,void*);struct _tuple0 Cyc_CfFlowInfo_join_flow_and_rval(
struct Cyc_Set_Set**all_changed,struct _tuple0 pr1,struct _tuple0 pr2);void*Cyc_CfFlowInfo_after_flow(
struct Cyc_Set_Set**,void*,void*,struct Cyc_Set_Set*,struct Cyc_Set_Set*);void*Cyc_CfFlowInfo_kill_flow_region(
void*f,void*rgn);static struct Cyc_CfFlowInfo_UnknownR_struct Cyc_CfFlowInfo_unknown_none_v={
0,(void*)((void*)0)};static struct Cyc_CfFlowInfo_UnknownR_struct Cyc_CfFlowInfo_unknown_this_v={
0,(void*)((void*)1)};static struct Cyc_CfFlowInfo_UnknownR_struct Cyc_CfFlowInfo_unknown_all_v={
0,(void*)((void*)2)};static struct Cyc_CfFlowInfo_Esc_struct Cyc_CfFlowInfo_esc_none_v={
1,(void*)((void*)0)};static struct Cyc_CfFlowInfo_Esc_struct Cyc_CfFlowInfo_esc_this_v={
1,(void*)((void*)1)};static struct Cyc_CfFlowInfo_Esc_struct Cyc_CfFlowInfo_esc_all_v={
1,(void*)((void*)2)};void*Cyc_CfFlowInfo_unknown_none=(void*)& Cyc_CfFlowInfo_unknown_none_v;
void*Cyc_CfFlowInfo_unknown_this=(void*)& Cyc_CfFlowInfo_unknown_this_v;void*Cyc_CfFlowInfo_unknown_all=(
void*)& Cyc_CfFlowInfo_unknown_all_v;void*Cyc_CfFlowInfo_esc_none=(void*)& Cyc_CfFlowInfo_esc_none_v;
void*Cyc_CfFlowInfo_esc_this=(void*)& Cyc_CfFlowInfo_esc_this_v;void*Cyc_CfFlowInfo_esc_all=(
void*)& Cyc_CfFlowInfo_esc_all_v;struct Cyc_Set_Set*Cyc_CfFlowInfo_mt_place_set(){
static struct Cyc_Set_Set**mt_place_set_opt=0;if(mt_place_set_opt == 0)
mt_place_set_opt=({struct Cyc_Set_Set**_tmp6=_cycalloc(sizeof(*_tmp6));_tmp6[0]=((
struct Cyc_Set_Set*(*)(int(*cmp)(struct Cyc_CfFlowInfo_Place*,struct Cyc_CfFlowInfo_Place*)))
Cyc_Set_empty)(Cyc_CfFlowInfo_place_cmp);_tmp6;});return*mt_place_set_opt;}int
Cyc_CfFlowInfo_root_cmp(void*r1,void*r2){if((int)r1 == (int)r2)return 0;{struct
_tuple0 _tmp8=({struct _tuple0 _tmp7;_tmp7.f1=r1;_tmp7.f2=r2;_tmp7;});void*_tmp9;
struct Cyc_Absyn_Vardecl*_tmpA;void*_tmpB;struct Cyc_Absyn_Vardecl*_tmpC;void*
_tmpD;void*_tmpE;void*_tmpF;struct Cyc_Absyn_Exp*_tmp10;void*_tmp11;struct Cyc_Absyn_Exp*
_tmp12;void*_tmp13;void*_tmp14;void*_tmp15;int _tmp16;void*_tmp17;int _tmp18;_LL1:
_tmp9=_tmp8.f1;if(*((int*)_tmp9)!= 0)goto _LL3;_tmpA=((struct Cyc_CfFlowInfo_VarRoot_struct*)
_tmp9)->f1;_tmpB=_tmp8.f2;if(*((int*)_tmpB)!= 0)goto _LL3;_tmpC=((struct Cyc_CfFlowInfo_VarRoot_struct*)
_tmpB)->f1;_LL2: return(int)_tmpA - (int)_tmpC;_LL3: _tmpD=_tmp8.f1;if(*((int*)
_tmpD)!= 0)goto _LL5;_LL4: return - 1;_LL5: _tmpE=_tmp8.f2;if(*((int*)_tmpE)!= 0)goto
_LL7;_LL6: return 1;_LL7: _tmpF=_tmp8.f1;if(*((int*)_tmpF)!= 1)goto _LL9;_tmp10=((
struct Cyc_CfFlowInfo_MallocPt_struct*)_tmpF)->f1;_tmp11=_tmp8.f2;if(*((int*)
_tmp11)!= 1)goto _LL9;_tmp12=((struct Cyc_CfFlowInfo_MallocPt_struct*)_tmp11)->f1;
_LL8: return(int)_tmp10 - (int)_tmp12;_LL9: _tmp13=_tmp8.f1;if(*((int*)_tmp13)!= 1)
goto _LLB;_LLA: return - 1;_LLB: _tmp14=_tmp8.f2;if(*((int*)_tmp14)!= 1)goto _LLD;_LLC:
return 1;_LLD: _tmp15=_tmp8.f1;if(*((int*)_tmp15)!= 2)goto _LL0;_tmp16=((struct Cyc_CfFlowInfo_InitParam_struct*)
_tmp15)->f1;_tmp17=_tmp8.f2;if(*((int*)_tmp17)!= 2)goto _LL0;_tmp18=((struct Cyc_CfFlowInfo_InitParam_struct*)
_tmp17)->f1;_LLE: return _tmp16 - _tmp18;_LL0:;}}int Cyc_CfFlowInfo_place_cmp(struct
Cyc_CfFlowInfo_Place*p1,struct Cyc_CfFlowInfo_Place*p2){if((int)p1 == (int)p2)
return 0;{int i=Cyc_CfFlowInfo_root_cmp((void*)p1->root,(void*)p2->root);if(i != 0)
return i;return((int(*)(int(*cmp)(struct _tagged_arr*,struct _tagged_arr*),struct
Cyc_List_List*l1,struct Cyc_List_List*l2))Cyc_List_list_cmp)(Cyc_strptrcmp,p1->fields,
p2->fields);}}static struct _tagged_arr*Cyc_CfFlowInfo_place2string(struct Cyc_CfFlowInfo_Place*
p){struct Cyc_List_List*sl=0;{void*_tmp19=(void*)p->root;struct Cyc_Absyn_Vardecl*
_tmp1A;struct Cyc_Absyn_Exp*_tmp1B;int _tmp1C;_LL10: if(*((int*)_tmp19)!= 0)goto
_LL12;_tmp1A=((struct Cyc_CfFlowInfo_VarRoot_struct*)_tmp19)->f1;_LL11: sl=({
struct Cyc_List_List*_tmp1D=_cycalloc(sizeof(*_tmp1D));_tmp1D->hd=({struct
_tagged_arr*_tmp1E=_cycalloc(sizeof(*_tmp1E));_tmp1E[0]=({struct Cyc_String_pa_struct
_tmp21;_tmp21.tag=0;_tmp21.f1=(struct _tagged_arr)((struct _tagged_arr)*(*_tmp1A->name).f2);{
void*_tmp1F[1]={& _tmp21};Cyc_aprintf(({const char*_tmp20="%s";_tag_arr(_tmp20,
sizeof(char),_get_zero_arr_size(_tmp20,3));}),_tag_arr(_tmp1F,sizeof(void*),1));}});
_tmp1E;});_tmp1D->tl=sl;_tmp1D;});goto _LLF;_LL12: if(*((int*)_tmp19)!= 1)goto
_LL14;_tmp1B=((struct Cyc_CfFlowInfo_MallocPt_struct*)_tmp19)->f1;_LL13: sl=({
struct Cyc_List_List*_tmp22=_cycalloc(sizeof(*_tmp22));_tmp22->hd=({struct
_tagged_arr*_tmp23=_cycalloc(sizeof(*_tmp23));_tmp23[0]=({struct Cyc_Int_pa_struct
_tmp26;_tmp26.tag=1;_tmp26.f1=(unsigned int)((int)_tmp1B);{void*_tmp24[1]={&
_tmp26};Cyc_aprintf(({const char*_tmp25="mpt%d";_tag_arr(_tmp25,sizeof(char),
_get_zero_arr_size(_tmp25,6));}),_tag_arr(_tmp24,sizeof(void*),1));}});_tmp23;});
_tmp22->tl=sl;_tmp22;});goto _LLF;_LL14: if(*((int*)_tmp19)!= 2)goto _LLF;_tmp1C=((
struct Cyc_CfFlowInfo_InitParam_struct*)_tmp19)->f1;_LL15: sl=({struct Cyc_List_List*
_tmp27=_cycalloc(sizeof(*_tmp27));_tmp27->hd=({struct _tagged_arr*_tmp28=
_cycalloc(sizeof(*_tmp28));_tmp28[0]=({struct Cyc_Int_pa_struct _tmp2B;_tmp2B.tag=
1;_tmp2B.f1=(unsigned int)_tmp1C;{void*_tmp29[1]={& _tmp2B};Cyc_aprintf(({const
char*_tmp2A="param%d";_tag_arr(_tmp2A,sizeof(char),_get_zero_arr_size(_tmp2A,8));}),
_tag_arr(_tmp29,sizeof(void*),1));}});_tmp28;});_tmp27->tl=sl;_tmp27;});goto _LLF;
_LLF:;}{struct Cyc_List_List*fields=p->fields;for(0;fields != 0;fields=fields->tl){
sl=({struct Cyc_List_List*_tmp2C=_cycalloc(sizeof(*_tmp2C));_tmp2C->hd=({struct
_tagged_arr*_tmp2D=_cycalloc(sizeof(*_tmp2D));_tmp2D[0]=({struct Cyc_String_pa_struct
_tmp30;_tmp30.tag=0;_tmp30.f1=(struct _tagged_arr)((struct _tagged_arr)*((struct
_tagged_arr*)fields->hd));{void*_tmp2E[1]={& _tmp30};Cyc_aprintf(({const char*
_tmp2F="%s";_tag_arr(_tmp2F,sizeof(char),_get_zero_arr_size(_tmp2F,3));}),
_tag_arr(_tmp2E,sizeof(void*),1));}});_tmp2D;});_tmp2C->tl=sl;_tmp2C;});}}{
struct _tagged_arr*_tmp31=({struct _tagged_arr*_tmp36=_cycalloc(sizeof(*_tmp36));
_tmp36[0]=({struct Cyc_String_pa_struct _tmp39;_tmp39.tag=0;_tmp39.f1=(struct
_tagged_arr)({const char*_tmp3A="";_tag_arr(_tmp3A,sizeof(char),
_get_zero_arr_size(_tmp3A,1));});{void*_tmp37[1]={& _tmp39};Cyc_aprintf(({const
char*_tmp38="%s";_tag_arr(_tmp38,sizeof(char),_get_zero_arr_size(_tmp38,3));}),
_tag_arr(_tmp37,sizeof(void*),1));}});_tmp36;});for(0;sl != 0;sl=sl->tl){*_tmp31=({
struct Cyc_String_pa_struct _tmp35;_tmp35.tag=0;_tmp35.f1=(struct _tagged_arr)((
struct _tagged_arr)*_tmp31);{struct Cyc_String_pa_struct _tmp34;_tmp34.tag=0;_tmp34.f1=(
struct _tagged_arr)((struct _tagged_arr)*((struct _tagged_arr*)sl->hd));{void*
_tmp32[2]={& _tmp34,& _tmp35};Cyc_aprintf(({const char*_tmp33="%s.%s";_tag_arr(
_tmp33,sizeof(char),_get_zero_arr_size(_tmp33,6));}),_tag_arr(_tmp32,sizeof(void*),
2));}}});}return _tmp31;}}struct _tuple4{struct Cyc_Absyn_Tqual f1;void*f2;};static
void*Cyc_CfFlowInfo_i_typ_to_absrval(int allow_zeroterm,void*t,void*leafval){if(!
Cyc_Absyn_is_union_type(t)){void*_tmp3B=Cyc_Tcutil_compress(t);struct Cyc_Absyn_TunionFieldInfo
_tmp3C;void*_tmp3D;struct Cyc_Absyn_Tunionfield*_tmp3E;struct Cyc_List_List*_tmp3F;
struct Cyc_Absyn_AggrInfo _tmp40;void*_tmp41;void*_tmp42;struct Cyc_List_List*
_tmp43;struct Cyc_Absyn_ArrayInfo _tmp44;void*_tmp45;struct Cyc_Absyn_Conref*_tmp46;
void*_tmp47;struct Cyc_Absyn_PtrInfo _tmp48;struct Cyc_Absyn_PtrAtts _tmp49;struct
Cyc_Absyn_Conref*_tmp4A;_LL17: if(_tmp3B <= (void*)3?1:*((int*)_tmp3B)!= 3)goto
_LL19;_tmp3C=((struct Cyc_Absyn_TunionFieldType_struct*)_tmp3B)->f1;_tmp3D=(void*)
_tmp3C.field_info;if(*((int*)_tmp3D)!= 1)goto _LL19;_tmp3E=((struct Cyc_Absyn_KnownTunionfield_struct*)
_tmp3D)->f2;_LL18: if(_tmp3E->typs == 0)return leafval;_tmp3F=_tmp3E->typs;goto
_LL1A;_LL19: if(_tmp3B <= (void*)3?1:*((int*)_tmp3B)!= 9)goto _LL1B;_tmp3F=((struct
Cyc_Absyn_TupleType_struct*)_tmp3B)->f1;_LL1A: {struct Cyc_Dict_Dict*d=((struct
Cyc_Dict_Dict*(*)(int(*cmp)(struct _tagged_arr*,struct _tagged_arr*)))Cyc_Dict_empty)(
Cyc_strptrcmp);{int i=0;for(0;_tmp3F != 0;(_tmp3F=_tmp3F->tl,++ i)){d=((struct Cyc_Dict_Dict*(*)(
struct Cyc_Dict_Dict*d,struct _tagged_arr*k,void*v))Cyc_Dict_insert)(d,Cyc_Absyn_fieldname(
i),Cyc_CfFlowInfo_i_typ_to_absrval(0,(*((struct _tuple4*)_tmp3F->hd)).f2,leafval));}}
return(void*)({struct Cyc_CfFlowInfo_Aggregate_struct*_tmp4B=_cycalloc(sizeof(*
_tmp4B));_tmp4B[0]=({struct Cyc_CfFlowInfo_Aggregate_struct _tmp4C;_tmp4C.tag=4;
_tmp4C.f1=d;_tmp4C;});_tmp4B;});}_LL1B: if(_tmp3B <= (void*)3?1:*((int*)_tmp3B)!= 
10)goto _LL1D;_tmp40=((struct Cyc_Absyn_AggrType_struct*)_tmp3B)->f1;_tmp41=(void*)
_tmp40.aggr_info;_LL1C: {struct Cyc_Absyn_Aggrdecl*_tmp4D=Cyc_Absyn_get_known_aggrdecl(
_tmp41);if(_tmp4D->impl == 0)goto _LL16;_tmp43=((struct Cyc_Absyn_AggrdeclImpl*)
_check_null(_tmp4D->impl))->fields;goto _LL1E;}_LL1D: if(_tmp3B <= (void*)3?1:*((
int*)_tmp3B)!= 11)goto _LL1F;_tmp42=(void*)((struct Cyc_Absyn_AnonAggrType_struct*)
_tmp3B)->f1;if((int)_tmp42 != 0)goto _LL1F;_tmp43=((struct Cyc_Absyn_AnonAggrType_struct*)
_tmp3B)->f2;_LL1E: {struct Cyc_Dict_Dict*d=((struct Cyc_Dict_Dict*(*)(int(*cmp)(
struct _tagged_arr*,struct _tagged_arr*)))Cyc_Dict_empty)(Cyc_strptrcmp);for(0;
_tmp43 != 0;_tmp43=_tmp43->tl){struct Cyc_Absyn_Aggrfield _tmp4F;struct _tagged_arr*
_tmp50;void*_tmp51;struct Cyc_Absyn_Aggrfield*_tmp4E=(struct Cyc_Absyn_Aggrfield*)
_tmp43->hd;_tmp4F=*_tmp4E;_tmp50=_tmp4F.name;_tmp51=(void*)_tmp4F.type;if(
_get_arr_size(*_tmp50,sizeof(char))!= 1)d=((struct Cyc_Dict_Dict*(*)(struct Cyc_Dict_Dict*
d,struct _tagged_arr*k,void*v))Cyc_Dict_insert)(d,_tmp50,Cyc_CfFlowInfo_i_typ_to_absrval(
0,_tmp51,leafval));}return(void*)({struct Cyc_CfFlowInfo_Aggregate_struct*_tmp52=
_cycalloc(sizeof(*_tmp52));_tmp52[0]=({struct Cyc_CfFlowInfo_Aggregate_struct
_tmp53;_tmp53.tag=4;_tmp53.f1=d;_tmp53;});_tmp52;});}_LL1F: if(_tmp3B <= (void*)3?
1:*((int*)_tmp3B)!= 7)goto _LL21;_tmp44=((struct Cyc_Absyn_ArrayType_struct*)
_tmp3B)->f1;_tmp45=(void*)_tmp44.elt_type;_tmp46=_tmp44.zero_term;if(!((int(*)(
int,struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,_tmp46))goto _LL21;_LL20:
return(allow_zeroterm?Cyc_Tcutil_bits_only(_tmp45): 0)?Cyc_CfFlowInfo_unknown_all:
leafval;_LL21: if(_tmp3B <= (void*)3?1:*((int*)_tmp3B)!= 17)goto _LL23;_tmp47=(void*)((
struct Cyc_Absyn_TagType_struct*)_tmp3B)->f1;_LL22: {void*_tmp54=leafval;void*
_tmp55;void*_tmp56;_LL28: if(_tmp54 <= (void*)3?1:*((int*)_tmp54)!= 0)goto _LL2A;
_tmp55=(void*)((struct Cyc_CfFlowInfo_UnknownR_struct*)_tmp54)->f1;if((int)_tmp55
!= 2)goto _LL2A;_LL29: goto _LL2B;_LL2A: if(_tmp54 <= (void*)3?1:*((int*)_tmp54)!= 1)
goto _LL2C;_tmp56=(void*)((struct Cyc_CfFlowInfo_Esc_struct*)_tmp54)->f1;if((int)
_tmp56 != 2)goto _LL2C;_LL2B: goto _LL2D;_LL2C: if((int)_tmp54 != 0)goto _LL2E;_LL2D:
goto _LL2F;_LL2E: if((int)_tmp54 != 1)goto _LL30;_LL2F: return(void*)({struct Cyc_CfFlowInfo_TagCmps_struct*
_tmp57=_cycalloc(sizeof(*_tmp57));_tmp57[0]=({struct Cyc_CfFlowInfo_TagCmps_struct
_tmp58;_tmp58.tag=3;_tmp58.f1=({struct Cyc_CfFlowInfo_TagCmp*_tmp59[1];_tmp59[0]=({
struct Cyc_CfFlowInfo_TagCmp*_tmp5A=_cycalloc(sizeof(*_tmp5A));_tmp5A->cmp=(void*)((
void*)5);_tmp5A->bd=(void*)_tmp47;_tmp5A;});((struct Cyc_List_List*(*)(struct
_tagged_arr))Cyc_List_list)(_tag_arr(_tmp59,sizeof(struct Cyc_CfFlowInfo_TagCmp*),
1));});_tmp58;});_tmp57;});_LL30:;_LL31: return leafval;_LL27:;}_LL23: if(_tmp3B <= (
void*)3?1:*((int*)_tmp3B)!= 4)goto _LL25;_tmp48=((struct Cyc_Absyn_PointerType_struct*)
_tmp3B)->f1;_tmp49=_tmp48.ptr_atts;_tmp4A=_tmp49.nullable;if(!(!((int(*)(int,
struct Cyc_Absyn_Conref*x))Cyc_Absyn_conref_def)(0,_tmp4A)))goto _LL25;_LL24:
return(void*)1;_LL25:;_LL26: goto _LL16;_LL16:;}return Cyc_Tcutil_bits_only(t)?Cyc_CfFlowInfo_unknown_all:
leafval;}void*Cyc_CfFlowInfo_typ_to_absrval(void*t,void*leafval){return Cyc_CfFlowInfo_i_typ_to_absrval(
1,t,leafval);}static int Cyc_CfFlowInfo_prefix_of_member(struct _RegionHandle*r,
struct Cyc_CfFlowInfo_Place*place,struct Cyc_Set_Set*set){struct Cyc_CfFlowInfo_Place*
_tmp5B=place;struct Cyc_Iter_Iter _tmp5C=((struct Cyc_Iter_Iter(*)(struct
_RegionHandle*rgn,struct Cyc_Set_Set*s))Cyc_Set_make_iter)(r,set);while(((int(*)(
struct Cyc_Iter_Iter,struct Cyc_CfFlowInfo_Place**))Cyc_Iter_next)(_tmp5C,& _tmp5B)){
if(Cyc_CfFlowInfo_root_cmp((void*)place->root,(void*)_tmp5B->root)!= 0)continue;{
struct Cyc_List_List*_tmp5D=place->fields;struct Cyc_List_List*_tmp5E=_tmp5B->fields;
for(0;_tmp5D != 0?_tmp5E != 0: 0;(_tmp5D=_tmp5D->tl,_tmp5E=_tmp5E->tl)){if(Cyc_strptrcmp((
struct _tagged_arr*)_tmp5D->hd,(struct _tagged_arr*)_tmp5E->hd)!= 0)break;}if(
_tmp5D == 0)return 1;}}return 0;}struct Cyc_CfFlowInfo_EscPile{struct _RegionHandle*
rgn;struct Cyc_List_List*places;};static void Cyc_CfFlowInfo_add_place(struct Cyc_CfFlowInfo_EscPile*
pile,struct Cyc_CfFlowInfo_Place*place){if(!((int(*)(int(*compare)(struct Cyc_CfFlowInfo_Place*,
struct Cyc_CfFlowInfo_Place*),struct Cyc_List_List*l,struct Cyc_CfFlowInfo_Place*x))
Cyc_List_mem)(Cyc_CfFlowInfo_place_cmp,pile->places,place))pile->places=({struct
Cyc_List_List*_tmp5F=_region_malloc(pile->rgn,sizeof(*_tmp5F));_tmp5F->hd=place;
_tmp5F->tl=pile->places;_tmp5F;});}static void Cyc_CfFlowInfo_add_places(struct Cyc_CfFlowInfo_EscPile*
pile,void*a,void*r){void*_tmp60=r;struct Cyc_CfFlowInfo_Place*_tmp61;struct Cyc_Dict_Dict*
_tmp62;_LL33: if(_tmp60 <= (void*)3?1:*((int*)_tmp60)!= 2)goto _LL35;_tmp61=((
struct Cyc_CfFlowInfo_AddressOf_struct*)_tmp60)->f1;_LL34: Cyc_CfFlowInfo_add_place(
pile,_tmp61);return;_LL35: if(_tmp60 <= (void*)3?1:*((int*)_tmp60)!= 4)goto _LL37;
_tmp62=((struct Cyc_CfFlowInfo_Aggregate_struct*)_tmp60)->f1;_LL36:((void(*)(void(*
f)(struct Cyc_CfFlowInfo_EscPile*,struct _tagged_arr*,void*),struct Cyc_CfFlowInfo_EscPile*
env,struct Cyc_Dict_Dict*d))Cyc_Dict_iter_c)((void(*)(struct Cyc_CfFlowInfo_EscPile*
pile,struct _tagged_arr*a,void*r))Cyc_CfFlowInfo_add_places,pile,_tmp62);return;
_LL37:;_LL38: return;_LL32:;}static void*Cyc_CfFlowInfo_insert_place_inner(void*
new_val,void*old_val){void*_tmp63=old_val;struct Cyc_Dict_Dict*_tmp64;_LL3A: if(
_tmp63 <= (void*)3?1:*((int*)_tmp63)!= 4)goto _LL3C;_tmp64=((struct Cyc_CfFlowInfo_Aggregate_struct*)
_tmp63)->f1;_LL3B: return(void*)({struct Cyc_CfFlowInfo_Aggregate_struct*_tmp65=
_cycalloc(sizeof(*_tmp65));_tmp65[0]=({struct Cyc_CfFlowInfo_Aggregate_struct
_tmp66;_tmp66.tag=4;_tmp66.f1=((struct Cyc_Dict_Dict*(*)(void*(*f)(void*,void*),
void*env,struct Cyc_Dict_Dict*d))Cyc_Dict_map_c)(Cyc_CfFlowInfo_insert_place_inner,
new_val,_tmp64);_tmp66;});_tmp65;});_LL3C:;_LL3D: return new_val;_LL39:;}struct
_tuple5{struct Cyc_List_List*f1;void*f2;};static void*Cyc_CfFlowInfo_insert_place_outer(
struct Cyc_List_List*fs,void*old_val,void*new_val){if(fs == 0)return Cyc_CfFlowInfo_insert_place_inner(
new_val,old_val);{struct _tuple5 _tmp68=({struct _tuple5 _tmp67;_tmp67.f1=fs;_tmp67.f2=
old_val;_tmp67;});struct Cyc_List_List*_tmp69;struct Cyc_List_List _tmp6A;struct
_tagged_arr*_tmp6B;struct Cyc_List_List*_tmp6C;void*_tmp6D;struct Cyc_Dict_Dict*
_tmp6E;_LL3F: _tmp69=_tmp68.f1;if(_tmp69 == 0)goto _LL41;_tmp6A=*_tmp69;_tmp6B=(
struct _tagged_arr*)_tmp6A.hd;_tmp6C=_tmp6A.tl;_tmp6D=_tmp68.f2;if(_tmp6D <= (void*)
3?1:*((int*)_tmp6D)!= 4)goto _LL41;_tmp6E=((struct Cyc_CfFlowInfo_Aggregate_struct*)
_tmp6D)->f1;_LL40: {void*_tmp6F=Cyc_CfFlowInfo_insert_place_outer(_tmp6C,((void*(*)(
struct Cyc_Dict_Dict*d,struct _tagged_arr*k))Cyc_Dict_lookup)(_tmp6E,_tmp6B),
new_val);return(void*)({struct Cyc_CfFlowInfo_Aggregate_struct*_tmp70=_cycalloc(
sizeof(*_tmp70));_tmp70[0]=({struct Cyc_CfFlowInfo_Aggregate_struct _tmp71;_tmp71.tag=
4;_tmp71.f1=((struct Cyc_Dict_Dict*(*)(struct Cyc_Dict_Dict*d,struct _tagged_arr*k,
void*v))Cyc_Dict_insert)(_tmp6E,_tmp6B,_tmp6F);_tmp71;});_tmp70;});}_LL41:;_LL42:(
int)_throw((void*)({struct Cyc_Core_Impossible_struct*_tmp72=_cycalloc(sizeof(*
_tmp72));_tmp72[0]=({struct Cyc_Core_Impossible_struct _tmp73;_tmp73.tag=Cyc_Core_Impossible;
_tmp73.f1=({const char*_tmp74="bad insert place";_tag_arr(_tmp74,sizeof(char),
_get_zero_arr_size(_tmp74,17));});_tmp73;});_tmp72;}));_LL3E:;}}static struct Cyc_Dict_Dict*
Cyc_CfFlowInfo_escape_these(struct Cyc_CfFlowInfo_EscPile*pile,struct Cyc_Set_Set**
all_changed,struct Cyc_Dict_Dict*d){while(pile->places != 0){struct Cyc_CfFlowInfo_Place*
_tmp75=(struct Cyc_CfFlowInfo_Place*)((struct Cyc_List_List*)_check_null(pile->places))->hd;
pile->places=((struct Cyc_List_List*)_check_null(pile->places))->tl;if(
all_changed != 0)*all_changed=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*s,struct
Cyc_CfFlowInfo_Place*elt))Cyc_Set_insert)(*all_changed,_tmp75);{void*oldval;void*
newval;{struct _handler_cons _tmp76;_push_handler(& _tmp76);{int _tmp78=0;if(setjmp(
_tmp76.handler))_tmp78=1;if(!_tmp78){oldval=Cyc_CfFlowInfo_lookup_place(d,_tmp75);;
_pop_handler();}else{void*_tmp77=(void*)_exn_thrown;void*_tmp7A=_tmp77;_LL44: if(
_tmp7A != Cyc_Dict_Absent)goto _LL46;_LL45: continue;_LL46:;_LL47:(void)_throw(
_tmp7A);_LL43:;}}}{void*_tmp7B=Cyc_CfFlowInfo_initlevel(d,oldval);_LL49: if((int)
_tmp7B != 2)goto _LL4B;_LL4A: newval=Cyc_CfFlowInfo_esc_all;goto _LL48;_LL4B: if((int)
_tmp7B != 1)goto _LL4D;_LL4C: newval=Cyc_CfFlowInfo_esc_this;goto _LL48;_LL4D: if((
int)_tmp7B != 0)goto _LL48;_LL4E: newval=Cyc_CfFlowInfo_esc_none;goto _LL48;_LL48:;}((
void(*)(struct Cyc_CfFlowInfo_EscPile*pile,int a,void*r))Cyc_CfFlowInfo_add_places)(
pile,0,oldval);d=Cyc_Dict_insert(d,(void*)_tmp75->root,Cyc_CfFlowInfo_insert_place_outer(
_tmp75->fields,Cyc_Dict_lookup(d,(void*)_tmp75->root),newval));}}return d;}struct
Cyc_CfFlowInfo_InitlevelEnv{struct Cyc_Dict_Dict*d;struct Cyc_List_List*seen;};
static void*Cyc_CfFlowInfo_initlevel_approx(void*r){void*_tmp7C=r;void*_tmp7D;
void*_tmp7E;_LL50: if(_tmp7C <= (void*)3?1:*((int*)_tmp7C)!= 0)goto _LL52;_tmp7D=(
void*)((struct Cyc_CfFlowInfo_UnknownR_struct*)_tmp7C)->f1;_LL51: return _tmp7D;
_LL52: if(_tmp7C <= (void*)3?1:*((int*)_tmp7C)!= 1)goto _LL54;_tmp7E=(void*)((
struct Cyc_CfFlowInfo_Esc_struct*)_tmp7C)->f1;_LL53: return _tmp7E;_LL54: if((int)
_tmp7C != 0)goto _LL56;_LL55: goto _LL57;_LL56: if((int)_tmp7C != 1)goto _LL58;_LL57:
return(void*)2;_LL58: if((int)_tmp7C != 2)goto _LL5A;_LL59: return(void*)1;_LL5A: if(
_tmp7C <= (void*)3?1:*((int*)_tmp7C)!= 3)goto _LL5C;_LL5B: return(void*)2;_LL5C:;
_LL5D:(int)_throw((void*)({struct Cyc_Core_Impossible_struct*_tmp7F=_cycalloc(
sizeof(*_tmp7F));_tmp7F[0]=({struct Cyc_Core_Impossible_struct _tmp80;_tmp80.tag=
Cyc_Core_Impossible;_tmp80.f1=({const char*_tmp81="initlevel_approx";_tag_arr(
_tmp81,sizeof(char),_get_zero_arr_size(_tmp81,17));});_tmp80;});_tmp7F;}));_LL4F:;}
static void*Cyc_CfFlowInfo_initlevel_rec(struct Cyc_CfFlowInfo_InitlevelEnv*env,
void*a,void*r,void*acc){void*this_ans;if(acc == (void*)0)return(void*)0;{void*
_tmp82=r;struct Cyc_Dict_Dict*_tmp83;struct Cyc_CfFlowInfo_Place*_tmp84;_LL5F: if(
_tmp82 <= (void*)3?1:*((int*)_tmp82)!= 4)goto _LL61;_tmp83=((struct Cyc_CfFlowInfo_Aggregate_struct*)
_tmp82)->f1;_LL60: this_ans=((void*(*)(void*(*f)(struct Cyc_CfFlowInfo_InitlevelEnv*,
struct _tagged_arr*,void*,void*),struct Cyc_CfFlowInfo_InitlevelEnv*env,struct Cyc_Dict_Dict*
d,void*accum))Cyc_Dict_fold_c)((void*(*)(struct Cyc_CfFlowInfo_InitlevelEnv*env,
struct _tagged_arr*a,void*r,void*acc))Cyc_CfFlowInfo_initlevel_rec,env,_tmp83,(
void*)2);goto _LL5E;_LL61: if(_tmp82 <= (void*)3?1:*((int*)_tmp82)!= 2)goto _LL63;
_tmp84=((struct Cyc_CfFlowInfo_AddressOf_struct*)_tmp82)->f1;_LL62: if(((int(*)(
int(*compare)(struct Cyc_CfFlowInfo_Place*,struct Cyc_CfFlowInfo_Place*),struct Cyc_List_List*
l,struct Cyc_CfFlowInfo_Place*x))Cyc_List_mem)(Cyc_CfFlowInfo_place_cmp,env->seen,
_tmp84))this_ans=(void*)2;else{env->seen=({struct Cyc_List_List*_tmp85=_cycalloc(
sizeof(*_tmp85));_tmp85->hd=_tmp84;_tmp85->tl=env->seen;_tmp85;});this_ans=((
void*(*)(struct Cyc_CfFlowInfo_InitlevelEnv*env,int a,void*r,void*acc))Cyc_CfFlowInfo_initlevel_rec)(
env,0,Cyc_CfFlowInfo_lookup_place(env->d,_tmp84),(void*)2);env->seen=((struct Cyc_List_List*)
_check_null(env->seen))->tl;if(this_ans == (void*)0)this_ans=(void*)1;}goto _LL5E;
_LL63:;_LL64: this_ans=Cyc_CfFlowInfo_initlevel_approx(r);_LL5E:;}if(this_ans == (
void*)0)return(void*)0;if(this_ans == (void*)1?1: acc == (void*)1)return(void*)1;
return(void*)2;}void*Cyc_CfFlowInfo_initlevel(struct Cyc_Dict_Dict*d,void*r){
struct Cyc_CfFlowInfo_InitlevelEnv _tmp86=({struct Cyc_CfFlowInfo_InitlevelEnv
_tmp87;_tmp87.d=d;_tmp87.seen=0;_tmp87;});return((void*(*)(struct Cyc_CfFlowInfo_InitlevelEnv*
env,int a,void*r,void*acc))Cyc_CfFlowInfo_initlevel_rec)(& _tmp86,0,r,(void*)2);}
void*Cyc_CfFlowInfo_lookup_place(struct Cyc_Dict_Dict*d,struct Cyc_CfFlowInfo_Place*
place){struct Cyc_CfFlowInfo_Place _tmp89;void*_tmp8A;struct Cyc_List_List*_tmp8B;
struct Cyc_CfFlowInfo_Place*_tmp88=place;_tmp89=*_tmp88;_tmp8A=(void*)_tmp89.root;
_tmp8B=_tmp89.fields;{void*_tmp8C=Cyc_Dict_lookup(d,_tmp8A);for(0;_tmp8B != 0;
_tmp8B=_tmp8B->tl){struct _tuple1 _tmp8E=({struct _tuple1 _tmp8D;_tmp8D.f1=_tmp8C;
_tmp8D.f2=(struct _tagged_arr*)_tmp8B->hd;_tmp8D;});void*_tmp8F;struct Cyc_Dict_Dict*
_tmp90;struct _tagged_arr*_tmp91;_LL66: _tmp8F=_tmp8E.f1;if(_tmp8F <= (void*)3?1:*((
int*)_tmp8F)!= 4)goto _LL68;_tmp90=((struct Cyc_CfFlowInfo_Aggregate_struct*)
_tmp8F)->f1;_tmp91=_tmp8E.f2;_LL67: _tmp8C=((void*(*)(struct Cyc_Dict_Dict*d,
struct _tagged_arr*k))Cyc_Dict_lookup)(_tmp90,_tmp91);goto _LL65;_LL68:;_LL69:(int)
_throw((void*)({struct Cyc_Core_Impossible_struct*_tmp92=_cycalloc(sizeof(*_tmp92));
_tmp92[0]=({struct Cyc_Core_Impossible_struct _tmp93;_tmp93.tag=Cyc_Core_Impossible;
_tmp93.f1=({const char*_tmp94="bad lookup_place";_tag_arr(_tmp94,sizeof(char),
_get_zero_arr_size(_tmp94,17));});_tmp93;});_tmp92;}));_LL65:;}return _tmp8C;}}
static int Cyc_CfFlowInfo_is_rval_unescaped(void*a,void*b,void*rval){void*_tmp95=
rval;struct Cyc_Dict_Dict*_tmp96;_LL6B: if(_tmp95 <= (void*)3?1:*((int*)_tmp95)!= 1)
goto _LL6D;_LL6C: return 0;_LL6D: if(_tmp95 <= (void*)3?1:*((int*)_tmp95)!= 4)goto
_LL6F;_tmp96=((struct Cyc_CfFlowInfo_Aggregate_struct*)_tmp95)->f1;_LL6E: return((
int(*)(int(*f)(int,struct _tagged_arr*,void*),int env,struct Cyc_Dict_Dict*d))Cyc_Dict_forall_c)((
int(*)(int a,struct _tagged_arr*b,void*rval))Cyc_CfFlowInfo_is_rval_unescaped,0,
_tmp96);_LL6F:;_LL70: return 1;_LL6A:;}int Cyc_CfFlowInfo_is_unescaped(struct Cyc_Dict_Dict*
d,struct Cyc_CfFlowInfo_Place*place){return((int(*)(int a,int b,void*rval))Cyc_CfFlowInfo_is_rval_unescaped)(
0,0,Cyc_CfFlowInfo_lookup_place(d,place));}struct Cyc_Dict_Dict*Cyc_CfFlowInfo_escape_deref(
struct Cyc_Dict_Dict*d,struct Cyc_Set_Set**all_changed,void*r){struct _RegionHandle
_tmp97=_new_region("rgn");struct _RegionHandle*rgn=& _tmp97;_push_region(rgn);{
struct Cyc_CfFlowInfo_EscPile*pile=({struct Cyc_CfFlowInfo_EscPile*_tmp99=
_region_malloc(rgn,sizeof(*_tmp99));_tmp99->rgn=rgn;_tmp99->places=0;_tmp99;});((
void(*)(struct Cyc_CfFlowInfo_EscPile*pile,int a,void*r))Cyc_CfFlowInfo_add_places)(
pile,0,r);{struct Cyc_Dict_Dict*_tmp98=Cyc_CfFlowInfo_escape_these(pile,
all_changed,d);_npop_handler(0);return _tmp98;}};_pop_region(rgn);}struct Cyc_CfFlowInfo_AssignEnv{
struct Cyc_CfFlowInfo_EscPile*pile;struct Cyc_Dict_Dict*d;struct Cyc_Position_Segment*
loc;};static void*Cyc_CfFlowInfo_assign_place_inner(struct Cyc_CfFlowInfo_AssignEnv*
env,void*a,void*oldval,void*newval){struct _tuple0 _tmp9B=({struct _tuple0 _tmp9A;
_tmp9A.f1=oldval;_tmp9A.f2=newval;_tmp9A;});void*_tmp9C;void*_tmp9D;struct Cyc_CfFlowInfo_Place*
_tmp9E;void*_tmp9F;void*_tmpA0;struct Cyc_Dict_Dict*_tmpA1;void*_tmpA2;struct Cyc_Dict_Dict*
_tmpA3;void*_tmpA4;void*_tmpA5;_LL72: _tmp9C=_tmp9B.f1;if(_tmp9C <= (void*)3?1:*((
int*)_tmp9C)!= 1)goto _LL74;_tmp9D=_tmp9B.f2;if(_tmp9D <= (void*)3?1:*((int*)
_tmp9D)!= 2)goto _LL74;_tmp9E=((struct Cyc_CfFlowInfo_AddressOf_struct*)_tmp9D)->f1;
_LL73: Cyc_CfFlowInfo_add_place(env->pile,_tmp9E);goto _LL75;_LL74: _tmp9F=_tmp9B.f1;
if(_tmp9F <= (void*)3?1:*((int*)_tmp9F)!= 1)goto _LL76;_LL75: if(Cyc_CfFlowInfo_initlevel(
env->d,newval)!= (void*)2)({void*_tmpA6[0]={};Cyc_Tcutil_terr(env->loc,({const
char*_tmpA7="assignment puts possibly-uninitialized data in an escaped location";
_tag_arr(_tmpA7,sizeof(char),_get_zero_arr_size(_tmpA7,67));}),_tag_arr(_tmpA6,
sizeof(void*),0));});return Cyc_CfFlowInfo_esc_all;_LL76: _tmpA0=_tmp9B.f1;if(
_tmpA0 <= (void*)3?1:*((int*)_tmpA0)!= 4)goto _LL78;_tmpA1=((struct Cyc_CfFlowInfo_Aggregate_struct*)
_tmpA0)->f1;_tmpA2=_tmp9B.f2;if(_tmpA2 <= (void*)3?1:*((int*)_tmpA2)!= 4)goto
_LL78;_tmpA3=((struct Cyc_CfFlowInfo_Aggregate_struct*)_tmpA2)->f1;_LL77: {struct
Cyc_Dict_Dict*_tmpA8=((struct Cyc_Dict_Dict*(*)(void*(*f)(struct Cyc_CfFlowInfo_AssignEnv*,
struct _tagged_arr*,void*,void*),struct Cyc_CfFlowInfo_AssignEnv*env,struct Cyc_Dict_Dict*
d1,struct Cyc_Dict_Dict*d2))Cyc_Dict_union_two_c)((void*(*)(struct Cyc_CfFlowInfo_AssignEnv*
env,struct _tagged_arr*a,void*oldval,void*newval))Cyc_CfFlowInfo_assign_place_inner,
env,_tmpA1,_tmpA3);if(_tmpA8 == _tmpA1)return oldval;if(_tmpA8 == _tmpA3)return
newval;return(void*)({struct Cyc_CfFlowInfo_Aggregate_struct*_tmpA9=_cycalloc(
sizeof(*_tmpA9));_tmpA9[0]=({struct Cyc_CfFlowInfo_Aggregate_struct _tmpAA;_tmpAA.tag=
4;_tmpAA.f1=_tmpA8;_tmpAA;});_tmpA9;});}_LL78: _tmpA4=_tmp9B.f2;if(_tmpA4 <= (void*)
3?1:*((int*)_tmpA4)!= 1)goto _LL7A;_tmpA5=(void*)((struct Cyc_CfFlowInfo_Esc_struct*)
_tmpA4)->f1;_LL79: {void*_tmpAB=_tmpA5;_LL7D: if((int)_tmpAB != 0)goto _LL7F;_LL7E:
return Cyc_CfFlowInfo_unknown_none;_LL7F: if((int)_tmpAB != 1)goto _LL81;_LL80:
return Cyc_CfFlowInfo_unknown_this;_LL81: if((int)_tmpAB != 2)goto _LL7C;_LL82:
return Cyc_CfFlowInfo_unknown_all;_LL7C:;}_LL7A:;_LL7B: return newval;_LL71:;}
static void*Cyc_CfFlowInfo_assign_place_outer(struct Cyc_CfFlowInfo_AssignEnv*env,
struct Cyc_List_List*fs,void*oldval,void*newval){if(fs == 0)return((void*(*)(
struct Cyc_CfFlowInfo_AssignEnv*env,int a,void*oldval,void*newval))Cyc_CfFlowInfo_assign_place_inner)(
env,0,oldval,newval);{struct _tuple5 _tmpAD=({struct _tuple5 _tmpAC;_tmpAC.f1=fs;
_tmpAC.f2=oldval;_tmpAC;});struct Cyc_List_List*_tmpAE;struct Cyc_List_List _tmpAF;
struct _tagged_arr*_tmpB0;struct Cyc_List_List*_tmpB1;void*_tmpB2;struct Cyc_Dict_Dict*
_tmpB3;_LL84: _tmpAE=_tmpAD.f1;if(_tmpAE == 0)goto _LL86;_tmpAF=*_tmpAE;_tmpB0=(
struct _tagged_arr*)_tmpAF.hd;_tmpB1=_tmpAF.tl;_tmpB2=_tmpAD.f2;if(_tmpB2 <= (void*)
3?1:*((int*)_tmpB2)!= 4)goto _LL86;_tmpB3=((struct Cyc_CfFlowInfo_Aggregate_struct*)
_tmpB2)->f1;_LL85: {void*_tmpB4=Cyc_CfFlowInfo_assign_place_outer(env,_tmpB1,((
void*(*)(struct Cyc_Dict_Dict*d,struct _tagged_arr*k))Cyc_Dict_lookup)(_tmpB3,
_tmpB0),newval);return(void*)({struct Cyc_CfFlowInfo_Aggregate_struct*_tmpB5=
_cycalloc(sizeof(*_tmpB5));_tmpB5[0]=({struct Cyc_CfFlowInfo_Aggregate_struct
_tmpB6;_tmpB6.tag=4;_tmpB6.f1=((struct Cyc_Dict_Dict*(*)(struct Cyc_Dict_Dict*d,
struct _tagged_arr*k,void*v))Cyc_Dict_insert)(_tmpB3,_tmpB0,_tmpB4);_tmpB6;});
_tmpB5;});}_LL86:;_LL87:(int)_throw((void*)({struct Cyc_Core_Impossible_struct*
_tmpB7=_cycalloc(sizeof(*_tmpB7));_tmpB7[0]=({struct Cyc_Core_Impossible_struct
_tmpB8;_tmpB8.tag=Cyc_Core_Impossible;_tmpB8.f1=({const char*_tmpB9="bad insert place";
_tag_arr(_tmpB9,sizeof(char),_get_zero_arr_size(_tmpB9,17));});_tmpB8;});_tmpB7;}));
_LL83:;}}struct Cyc_Dict_Dict*Cyc_CfFlowInfo_assign_place(struct Cyc_Position_Segment*
loc,struct Cyc_Dict_Dict*d,struct Cyc_Set_Set**all_changed,struct Cyc_CfFlowInfo_Place*
place,void*r){if(all_changed != 0)*all_changed=((struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*
s,struct Cyc_CfFlowInfo_Place*elt))Cyc_Set_insert)(*all_changed,place);{struct
_RegionHandle _tmpBA=_new_region("rgn");struct _RegionHandle*rgn=& _tmpBA;
_push_region(rgn);{struct Cyc_CfFlowInfo_Place _tmpBC;void*_tmpBD;struct Cyc_List_List*
_tmpBE;struct Cyc_CfFlowInfo_Place*_tmpBB=place;_tmpBC=*_tmpBB;_tmpBD=(void*)
_tmpBC.root;_tmpBE=_tmpBC.fields;{struct Cyc_CfFlowInfo_AssignEnv env=({struct Cyc_CfFlowInfo_AssignEnv
_tmpC0;_tmpC0.pile=({struct Cyc_CfFlowInfo_EscPile*_tmpC1=_region_malloc(rgn,
sizeof(*_tmpC1));_tmpC1->rgn=rgn;_tmpC1->places=0;_tmpC1;});_tmpC0.d=d;_tmpC0.loc=
loc;_tmpC0;});void*newval=Cyc_CfFlowInfo_assign_place_outer(& env,_tmpBE,Cyc_Dict_lookup(
d,_tmpBD),r);struct Cyc_Dict_Dict*_tmpBF=Cyc_CfFlowInfo_escape_these(env.pile,
all_changed,Cyc_Dict_insert(d,_tmpBD,newval));_npop_handler(0);return _tmpBF;}};
_pop_region(rgn);}}struct Cyc_CfFlowInfo_JoinEnv{struct Cyc_CfFlowInfo_EscPile*
pile;struct Cyc_Dict_Dict*d1;struct Cyc_Dict_Dict*d2;};enum Cyc_CfFlowInfo_WhoIsChanged{
Cyc_CfFlowInfo_Neither  = 0,Cyc_CfFlowInfo_One  = 1,Cyc_CfFlowInfo_Two  = 2};struct
Cyc_CfFlowInfo_AfterEnv{struct Cyc_CfFlowInfo_JoinEnv joinenv;struct Cyc_Set_Set*
chg1;struct Cyc_Set_Set*chg2;struct Cyc_CfFlowInfo_Place*curr_place;struct Cyc_List_List**
last_field_cell;enum Cyc_CfFlowInfo_WhoIsChanged changed;};static int Cyc_CfFlowInfo_absRval_lessthan_approx(
void*ignore,void*r1,void*r2);static struct Cyc_List_List*Cyc_CfFlowInfo_join_tag_cmps(
struct Cyc_List_List*l1,struct Cyc_List_List*l2){if(l1 == l2)return l1;{struct Cyc_List_List*
_tmpC2=0;for(0;l2 != 0;l2=l2->tl){struct Cyc_CfFlowInfo_TagCmp _tmpC4;void*_tmpC5;
void*_tmpC6;struct Cyc_CfFlowInfo_TagCmp*_tmpC3=(struct Cyc_CfFlowInfo_TagCmp*)l2->hd;
_tmpC4=*_tmpC3;_tmpC5=(void*)_tmpC4.cmp;_tmpC6=(void*)_tmpC4.bd;{int found=0;void*
joined_cmp=(void*)10;{struct Cyc_List_List*_tmpC7=l1;for(0;_tmpC7 != 0;_tmpC7=
_tmpC7->tl){struct Cyc_CfFlowInfo_TagCmp _tmpC9;void*_tmpCA;void*_tmpCB;struct Cyc_CfFlowInfo_TagCmp*
_tmpC8=(struct Cyc_CfFlowInfo_TagCmp*)_tmpC7->hd;_tmpC9=*_tmpC8;_tmpCA=(void*)
_tmpC9.cmp;_tmpCB=(void*)_tmpC9.bd;if(Cyc_Tcutil_typecmp(_tmpC6,_tmpCB)== 0){
found=1;if(_tmpCA == _tmpC5){joined_cmp=_tmpCA;break;}}}}if(found)_tmpC2=({struct
Cyc_List_List*_tmpCC=_cycalloc(sizeof(*_tmpCC));_tmpCC->hd=({struct Cyc_CfFlowInfo_TagCmp*
_tmpCD=_cycalloc(sizeof(*_tmpCD));_tmpCD->cmp=(void*)joined_cmp;_tmpCD->bd=(void*)
_tmpC6;_tmpCD;});_tmpCC->tl=_tmpC2;_tmpCC;});}}return _tmpC2;}}static void*Cyc_CfFlowInfo_join_absRval(
struct Cyc_CfFlowInfo_JoinEnv*env,void*a,void*r1,void*r2){if(r1 == r2)return r1;{
struct _tuple0 _tmpCF=({struct _tuple0 _tmpCE;_tmpCE.f1=r1;_tmpCE.f2=r2;_tmpCE;});
void*_tmpD0;struct Cyc_CfFlowInfo_Place*_tmpD1;void*_tmpD2;struct Cyc_CfFlowInfo_Place*
_tmpD3;void*_tmpD4;struct Cyc_CfFlowInfo_Place*_tmpD5;void*_tmpD6;struct Cyc_CfFlowInfo_Place*
_tmpD7;void*_tmpD8;void*_tmpD9;void*_tmpDA;void*_tmpDB;void*_tmpDC;struct Cyc_Dict_Dict*
_tmpDD;void*_tmpDE;struct Cyc_Dict_Dict*_tmpDF;void*_tmpE0;struct Cyc_List_List*
_tmpE1;void*_tmpE2;struct Cyc_List_List*_tmpE3;void*_tmpE4;void*_tmpE5;_LL89:
_tmpD0=_tmpCF.f1;if(_tmpD0 <= (void*)3?1:*((int*)_tmpD0)!= 2)goto _LL8B;_tmpD1=((
struct Cyc_CfFlowInfo_AddressOf_struct*)_tmpD0)->f1;_tmpD2=_tmpCF.f2;if(_tmpD2 <= (
void*)3?1:*((int*)_tmpD2)!= 2)goto _LL8B;_tmpD3=((struct Cyc_CfFlowInfo_AddressOf_struct*)
_tmpD2)->f1;_LL8A: if(Cyc_CfFlowInfo_place_cmp(_tmpD1,_tmpD3)== 0)return r1;Cyc_CfFlowInfo_add_place(
env->pile,_tmpD1);Cyc_CfFlowInfo_add_place(env->pile,_tmpD3);goto _LL88;_LL8B:
_tmpD4=_tmpCF.f1;if(_tmpD4 <= (void*)3?1:*((int*)_tmpD4)!= 2)goto _LL8D;_tmpD5=((
struct Cyc_CfFlowInfo_AddressOf_struct*)_tmpD4)->f1;_LL8C: Cyc_CfFlowInfo_add_place(
env->pile,_tmpD5);goto _LL88;_LL8D: _tmpD6=_tmpCF.f2;if(_tmpD6 <= (void*)3?1:*((int*)
_tmpD6)!= 2)goto _LL8F;_tmpD7=((struct Cyc_CfFlowInfo_AddressOf_struct*)_tmpD6)->f1;
_LL8E: Cyc_CfFlowInfo_add_place(env->pile,_tmpD7);goto _LL88;_LL8F: _tmpD8=_tmpCF.f1;
if((int)_tmpD8 != 1)goto _LL91;_tmpD9=_tmpCF.f2;if((int)_tmpD9 != 2)goto _LL91;_LL90:
goto _LL92;_LL91: _tmpDA=_tmpCF.f1;if((int)_tmpDA != 2)goto _LL93;_tmpDB=_tmpCF.f2;
if((int)_tmpDB != 1)goto _LL93;_LL92: return(void*)2;_LL93: _tmpDC=_tmpCF.f1;if(
_tmpDC <= (void*)3?1:*((int*)_tmpDC)!= 4)goto _LL95;_tmpDD=((struct Cyc_CfFlowInfo_Aggregate_struct*)
_tmpDC)->f1;_tmpDE=_tmpCF.f2;if(_tmpDE <= (void*)3?1:*((int*)_tmpDE)!= 4)goto
_LL95;_tmpDF=((struct Cyc_CfFlowInfo_Aggregate_struct*)_tmpDE)->f1;_LL94: {struct
Cyc_Dict_Dict*_tmpE6=((struct Cyc_Dict_Dict*(*)(void*(*f)(struct Cyc_CfFlowInfo_JoinEnv*,
struct _tagged_arr*,void*,void*),struct Cyc_CfFlowInfo_JoinEnv*env,struct Cyc_Dict_Dict*
d1,struct Cyc_Dict_Dict*d2))Cyc_Dict_union_two_c)((void*(*)(struct Cyc_CfFlowInfo_JoinEnv*
env,struct _tagged_arr*a,void*r1,void*r2))Cyc_CfFlowInfo_join_absRval,env,_tmpDD,
_tmpDF);if(_tmpE6 == _tmpDD)return r1;if(_tmpE6 == _tmpDF)return r2;return(void*)({
struct Cyc_CfFlowInfo_Aggregate_struct*_tmpE7=_cycalloc(sizeof(*_tmpE7));_tmpE7[0]=({
struct Cyc_CfFlowInfo_Aggregate_struct _tmpE8;_tmpE8.tag=4;_tmpE8.f1=_tmpE6;_tmpE8;});
_tmpE7;});}_LL95: _tmpE0=_tmpCF.f1;if(_tmpE0 <= (void*)3?1:*((int*)_tmpE0)!= 3)
goto _LL97;_tmpE1=((struct Cyc_CfFlowInfo_TagCmps_struct*)_tmpE0)->f1;_tmpE2=
_tmpCF.f2;if(_tmpE2 <= (void*)3?1:*((int*)_tmpE2)!= 3)goto _LL97;_tmpE3=((struct
Cyc_CfFlowInfo_TagCmps_struct*)_tmpE2)->f1;_LL96: {struct Cyc_List_List*_tmpE9=
Cyc_CfFlowInfo_join_tag_cmps(_tmpE1,_tmpE3);if(_tmpE9 == _tmpE1)return r1;return(
void*)({struct Cyc_CfFlowInfo_TagCmps_struct*_tmpEA=_cycalloc(sizeof(*_tmpEA));
_tmpEA[0]=({struct Cyc_CfFlowInfo_TagCmps_struct _tmpEB;_tmpEB.tag=3;_tmpEB.f1=
_tmpE9;_tmpEB;});_tmpEA;});}_LL97: _tmpE4=_tmpCF.f1;if(_tmpE4 <= (void*)3?1:*((int*)
_tmpE4)!= 3)goto _LL99;_LL98: return r2;_LL99: _tmpE5=_tmpCF.f2;if(_tmpE5 <= (void*)3?
1:*((int*)_tmpE5)!= 3)goto _LL9B;_LL9A: return r1;_LL9B:;_LL9C: goto _LL88;_LL88:;}{
void*il1=Cyc_CfFlowInfo_initlevel(env->d1,r1);void*il2=Cyc_CfFlowInfo_initlevel(
env->d2,r2);struct _tuple0 _tmpED=({struct _tuple0 _tmpEC;_tmpEC.f1=r1;_tmpEC.f2=r2;
_tmpEC;});void*_tmpEE;void*_tmpEF;_LL9E: _tmpEE=_tmpED.f1;if(_tmpEE <= (void*)3?1:*((
int*)_tmpEE)!= 1)goto _LLA0;_LL9F: goto _LLA1;_LLA0: _tmpEF=_tmpED.f2;if(_tmpEF <= (
void*)3?1:*((int*)_tmpEF)!= 1)goto _LLA2;_LLA1: {struct _tuple0 _tmpF1=({struct
_tuple0 _tmpF0;_tmpF0.f1=il1;_tmpF0.f2=il2;_tmpF0;});void*_tmpF2;void*_tmpF3;void*
_tmpF4;void*_tmpF5;_LLA5: _tmpF2=_tmpF1.f2;if((int)_tmpF2 != 0)goto _LLA7;_LLA6:
goto _LLA8;_LLA7: _tmpF3=_tmpF1.f1;if((int)_tmpF3 != 0)goto _LLA9;_LLA8: return Cyc_CfFlowInfo_esc_none;
_LLA9: _tmpF4=_tmpF1.f2;if((int)_tmpF4 != 1)goto _LLAB;_LLAA: goto _LLAC;_LLAB: _tmpF5=
_tmpF1.f1;if((int)_tmpF5 != 1)goto _LLAD;_LLAC: return Cyc_CfFlowInfo_esc_this;_LLAD:;
_LLAE: return Cyc_CfFlowInfo_esc_all;_LLA4:;}_LLA2:;_LLA3: {struct _tuple0 _tmpF7=({
struct _tuple0 _tmpF6;_tmpF6.f1=il1;_tmpF6.f2=il2;_tmpF6;});void*_tmpF8;void*
_tmpF9;void*_tmpFA;void*_tmpFB;_LLB0: _tmpF8=_tmpF7.f2;if((int)_tmpF8 != 0)goto
_LLB2;_LLB1: goto _LLB3;_LLB2: _tmpF9=_tmpF7.f1;if((int)_tmpF9 != 0)goto _LLB4;_LLB3:
return Cyc_CfFlowInfo_unknown_none;_LLB4: _tmpFA=_tmpF7.f2;if((int)_tmpFA != 1)goto
_LLB6;_LLB5: goto _LLB7;_LLB6: _tmpFB=_tmpF7.f1;if((int)_tmpFB != 1)goto _LLB8;_LLB7:
return Cyc_CfFlowInfo_unknown_this;_LLB8:;_LLB9: return Cyc_CfFlowInfo_unknown_all;
_LLAF:;}_LL9D:;}}static int Cyc_CfFlowInfo_same_relop(void*r1,void*r2){if(r1 == r2)
return 1;{struct _tuple0 _tmpFD=({struct _tuple0 _tmpFC;_tmpFC.f1=r1;_tmpFC.f2=r2;
_tmpFC;});void*_tmpFE;unsigned int _tmpFF;void*_tmp100;unsigned int _tmp101;void*
_tmp102;struct Cyc_Absyn_Vardecl*_tmp103;void*_tmp104;struct Cyc_Absyn_Vardecl*
_tmp105;void*_tmp106;struct Cyc_Absyn_Vardecl*_tmp107;void*_tmp108;struct Cyc_Absyn_Vardecl*
_tmp109;void*_tmp10A;unsigned int _tmp10B;void*_tmp10C;unsigned int _tmp10D;void*
_tmp10E;struct Cyc_Absyn_Vardecl*_tmp10F;void*_tmp110;struct Cyc_Absyn_Vardecl*
_tmp111;_LLBB: _tmpFE=_tmpFD.f1;if(*((int*)_tmpFE)!= 0)goto _LLBD;_tmpFF=((struct
Cyc_CfFlowInfo_EqualConst_struct*)_tmpFE)->f1;_tmp100=_tmpFD.f2;if(*((int*)
_tmp100)!= 0)goto _LLBD;_tmp101=((struct Cyc_CfFlowInfo_EqualConst_struct*)_tmp100)->f1;
_LLBC: return _tmpFF == _tmp101;_LLBD: _tmp102=_tmpFD.f1;if(*((int*)_tmp102)!= 1)
goto _LLBF;_tmp103=((struct Cyc_CfFlowInfo_LessVar_struct*)_tmp102)->f1;_tmp104=
_tmpFD.f2;if(*((int*)_tmp104)!= 1)goto _LLBF;_tmp105=((struct Cyc_CfFlowInfo_LessVar_struct*)
_tmp104)->f1;_LLBE: return _tmp103 == _tmp105;_LLBF: _tmp106=_tmpFD.f1;if(*((int*)
_tmp106)!= 2)goto _LLC1;_tmp107=((struct Cyc_CfFlowInfo_LessSize_struct*)_tmp106)->f1;
_tmp108=_tmpFD.f2;if(*((int*)_tmp108)!= 2)goto _LLC1;_tmp109=((struct Cyc_CfFlowInfo_LessSize_struct*)
_tmp108)->f1;_LLC0: return _tmp107 == _tmp109;_LLC1: _tmp10A=_tmpFD.f1;if(*((int*)
_tmp10A)!= 3)goto _LLC3;_tmp10B=((struct Cyc_CfFlowInfo_LessConst_struct*)_tmp10A)->f1;
_tmp10C=_tmpFD.f2;if(*((int*)_tmp10C)!= 3)goto _LLC3;_tmp10D=((struct Cyc_CfFlowInfo_LessConst_struct*)
_tmp10C)->f1;_LLC2: return _tmp10B == _tmp10D;_LLC3: _tmp10E=_tmpFD.f1;if(*((int*)
_tmp10E)!= 4)goto _LLC5;_tmp10F=((struct Cyc_CfFlowInfo_LessEqSize_struct*)_tmp10E)->f1;
_tmp110=_tmpFD.f2;if(*((int*)_tmp110)!= 4)goto _LLC5;_tmp111=((struct Cyc_CfFlowInfo_LessEqSize_struct*)
_tmp110)->f1;_LLC4: return _tmp10F == _tmp111;_LLC5:;_LLC6: return 0;_LLBA:;}}static
struct Cyc_List_List*Cyc_CfFlowInfo_join_relns(struct Cyc_List_List*r1s,struct Cyc_List_List*
r2s){if(r1s == r2s)return r1s;{struct Cyc_List_List*res=0;int diff=0;{struct Cyc_List_List*
_tmp112=r1s;for(0;_tmp112 != 0;_tmp112=_tmp112->tl){struct Cyc_CfFlowInfo_Reln*
_tmp113=(struct Cyc_CfFlowInfo_Reln*)_tmp112->hd;int found=0;{struct Cyc_List_List*
_tmp114=r2s;for(0;_tmp114 != 0;_tmp114=_tmp114->tl){struct Cyc_CfFlowInfo_Reln*
_tmp115=(struct Cyc_CfFlowInfo_Reln*)_tmp114->hd;if(_tmp113 == _tmp115?1:(_tmp113->vd
== _tmp115->vd?Cyc_CfFlowInfo_same_relop((void*)_tmp113->rop,(void*)_tmp115->rop):
0)){res=({struct Cyc_List_List*_tmp116=_cycalloc(sizeof(*_tmp116));_tmp116->hd=
_tmp113;_tmp116->tl=res;_tmp116;});found=1;break;}}}if(!found)diff=1;}}if(!diff)
res=r1s;return res;}}void*Cyc_CfFlowInfo_join_flow(struct Cyc_Set_Set**all_changed,
void*f1,void*f2){if(f1 == f2)return f1;{struct _tuple0 _tmp118=({struct _tuple0
_tmp117;_tmp117.f1=f1;_tmp117.f2=f2;_tmp117;});void*_tmp119;void*_tmp11A;void*
_tmp11B;struct Cyc_Dict_Dict*_tmp11C;struct Cyc_List_List*_tmp11D;void*_tmp11E;
struct Cyc_Dict_Dict*_tmp11F;struct Cyc_List_List*_tmp120;_LLC8: _tmp119=_tmp118.f1;
if((int)_tmp119 != 0)goto _LLCA;_LLC9: return f2;_LLCA: _tmp11A=_tmp118.f2;if((int)
_tmp11A != 0)goto _LLCC;_LLCB: return f1;_LLCC: _tmp11B=_tmp118.f1;if(_tmp11B <= (void*)
1?1:*((int*)_tmp11B)!= 0)goto _LLC7;_tmp11C=((struct Cyc_CfFlowInfo_ReachableFL_struct*)
_tmp11B)->f1;_tmp11D=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp11B)->f2;
_tmp11E=_tmp118.f2;if(_tmp11E <= (void*)1?1:*((int*)_tmp11E)!= 0)goto _LLC7;
_tmp11F=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp11E)->f1;_tmp120=((struct
Cyc_CfFlowInfo_ReachableFL_struct*)_tmp11E)->f2;_LLCD: if(_tmp11C == _tmp11F?
_tmp11D == _tmp120: 0)return f1;if(Cyc_CfFlowInfo_flow_lessthan_approx(f1,f2))
return f2;if(Cyc_CfFlowInfo_flow_lessthan_approx(f2,f1))return f1;{struct
_RegionHandle _tmp121=_new_region("rgn");struct _RegionHandle*rgn=& _tmp121;
_push_region(rgn);{struct Cyc_CfFlowInfo_JoinEnv _tmp122=({struct Cyc_CfFlowInfo_JoinEnv
_tmp128;_tmp128.pile=({struct Cyc_CfFlowInfo_EscPile*_tmp129=_region_malloc(rgn,
sizeof(*_tmp129));_tmp129->rgn=rgn;_tmp129->places=0;_tmp129;});_tmp128.d1=
_tmp11C;_tmp128.d2=_tmp11F;_tmp128;});struct Cyc_Dict_Dict*_tmp123=((struct Cyc_Dict_Dict*(*)(
void*(*f)(struct Cyc_CfFlowInfo_JoinEnv*,void*,void*,void*),struct Cyc_CfFlowInfo_JoinEnv*
env,struct Cyc_Dict_Dict*d1,struct Cyc_Dict_Dict*d2))Cyc_Dict_intersect_c)(Cyc_CfFlowInfo_join_absRval,&
_tmp122,_tmp11C,_tmp11F);struct Cyc_List_List*_tmp124=Cyc_CfFlowInfo_join_relns(
_tmp11D,_tmp120);void*_tmp127=(void*)({struct Cyc_CfFlowInfo_ReachableFL_struct*
_tmp125=_cycalloc(sizeof(*_tmp125));_tmp125[0]=({struct Cyc_CfFlowInfo_ReachableFL_struct
_tmp126;_tmp126.tag=0;_tmp126.f1=Cyc_CfFlowInfo_escape_these(_tmp122.pile,
all_changed,_tmp123);_tmp126.f2=_tmp124;_tmp126;});_tmp125;});_npop_handler(0);
return _tmp127;};_pop_region(rgn);}_LLC7:;}}struct _tuple6{void*f1;void*f2;void*f3;
};struct _tuple0 Cyc_CfFlowInfo_join_flow_and_rval(struct Cyc_Set_Set**all_changed,
struct _tuple0 pr1,struct _tuple0 pr2){void*_tmp12B;void*_tmp12C;struct _tuple0
_tmp12A=pr1;_tmp12B=_tmp12A.f1;_tmp12C=_tmp12A.f2;{void*_tmp12E;void*_tmp12F;
struct _tuple0 _tmp12D=pr2;_tmp12E=_tmp12D.f1;_tmp12F=_tmp12D.f2;{void*outflow=Cyc_CfFlowInfo_join_flow(
all_changed,_tmp12B,_tmp12E);struct _tuple6 _tmp131=({struct _tuple6 _tmp130;_tmp130.f1=
_tmp12B;_tmp130.f2=_tmp12E;_tmp130.f3=outflow;_tmp130;});void*_tmp132;void*
_tmp133;void*_tmp134;struct Cyc_Dict_Dict*_tmp135;void*_tmp136;struct Cyc_Dict_Dict*
_tmp137;void*_tmp138;struct Cyc_Dict_Dict*_tmp139;struct Cyc_List_List*_tmp13A;
_LLCF: _tmp132=_tmp131.f1;if((int)_tmp132 != 0)goto _LLD1;_LLD0: return({struct
_tuple0 _tmp13B;_tmp13B.f1=outflow;_tmp13B.f2=_tmp12F;_tmp13B;});_LLD1: _tmp133=
_tmp131.f2;if((int)_tmp133 != 0)goto _LLD3;_LLD2: return({struct _tuple0 _tmp13C;
_tmp13C.f1=outflow;_tmp13C.f2=_tmp12C;_tmp13C;});_LLD3: _tmp134=_tmp131.f1;if(
_tmp134 <= (void*)1?1:*((int*)_tmp134)!= 0)goto _LLD5;_tmp135=((struct Cyc_CfFlowInfo_ReachableFL_struct*)
_tmp134)->f1;_tmp136=_tmp131.f2;if(_tmp136 <= (void*)1?1:*((int*)_tmp136)!= 0)
goto _LLD5;_tmp137=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp136)->f1;
_tmp138=_tmp131.f3;if(_tmp138 <= (void*)1?1:*((int*)_tmp138)!= 0)goto _LLD5;
_tmp139=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp138)->f1;_tmp13A=((struct
Cyc_CfFlowInfo_ReachableFL_struct*)_tmp138)->f2;_LLD4: if(((int(*)(int ignore,void*
r1,void*r2))Cyc_CfFlowInfo_absRval_lessthan_approx)(0,_tmp12C,_tmp12F))return({
struct _tuple0 _tmp13D;_tmp13D.f1=outflow;_tmp13D.f2=_tmp12F;_tmp13D;});if(((int(*)(
int ignore,void*r1,void*r2))Cyc_CfFlowInfo_absRval_lessthan_approx)(0,_tmp12F,
_tmp12C))return({struct _tuple0 _tmp13E;_tmp13E.f1=outflow;_tmp13E.f2=_tmp12C;
_tmp13E;});{struct _RegionHandle _tmp13F=_new_region("rgn");struct _RegionHandle*
rgn=& _tmp13F;_push_region(rgn);{struct Cyc_CfFlowInfo_JoinEnv _tmp140=({struct Cyc_CfFlowInfo_JoinEnv
_tmp146;_tmp146.pile=({struct Cyc_CfFlowInfo_EscPile*_tmp147=_region_malloc(rgn,
sizeof(*_tmp147));_tmp147->rgn=rgn;_tmp147->places=0;_tmp147;});_tmp146.d1=
_tmp135;_tmp146.d2=_tmp137;_tmp146;});void*_tmp141=((void*(*)(struct Cyc_CfFlowInfo_JoinEnv*
env,int a,void*r1,void*r2))Cyc_CfFlowInfo_join_absRval)(& _tmp140,0,_tmp12C,
_tmp12F);struct _tuple0 _tmp145=({struct _tuple0 _tmp142;_tmp142.f1=(void*)({struct
Cyc_CfFlowInfo_ReachableFL_struct*_tmp143=_cycalloc(sizeof(*_tmp143));_tmp143[0]=({
struct Cyc_CfFlowInfo_ReachableFL_struct _tmp144;_tmp144.tag=0;_tmp144.f1=Cyc_CfFlowInfo_escape_these(
_tmp140.pile,all_changed,_tmp139);_tmp144.f2=_tmp13A;_tmp144;});_tmp143;});
_tmp142.f2=_tmp141;_tmp142;});_npop_handler(0);return _tmp145;};_pop_region(rgn);}
_LLD5:;_LLD6:(int)_throw((void*)({struct Cyc_Core_Impossible_struct*_tmp148=
_cycalloc(sizeof(*_tmp148));_tmp148[0]=({struct Cyc_Core_Impossible_struct _tmp149;
_tmp149.tag=Cyc_Core_Impossible;_tmp149.f1=({const char*_tmp14A="join_flow_and_rval: BottomFL outflow";
_tag_arr(_tmp14A,sizeof(char),_get_zero_arr_size(_tmp14A,37));});_tmp149;});
_tmp148;}));_LLCE:;}}}static void*Cyc_CfFlowInfo_after_absRval_d(struct Cyc_CfFlowInfo_AfterEnv*,
struct _tagged_arr*,void*,void*);static void*Cyc_CfFlowInfo_after_absRval(struct
Cyc_CfFlowInfo_AfterEnv*env,void*r1,void*r2){int changed1=env->changed == Cyc_CfFlowInfo_One?
1:((int(*)(struct Cyc_Set_Set*s,struct Cyc_CfFlowInfo_Place*elt))Cyc_Set_member)(
env->chg1,env->curr_place);int changed2=env->changed == Cyc_CfFlowInfo_Two?1:((int(*)(
struct Cyc_Set_Set*s,struct Cyc_CfFlowInfo_Place*elt))Cyc_Set_member)(env->chg2,
env->curr_place);if(changed1?changed2: 0)return((void*(*)(struct Cyc_CfFlowInfo_JoinEnv*
env,int a,void*r1,void*r2))Cyc_CfFlowInfo_join_absRval)(& env->joinenv,0,r1,r2);
if(changed1){if(!Cyc_CfFlowInfo_prefix_of_member(((env->joinenv).pile)->rgn,env->curr_place,
env->chg2))return r1;env->changed=Cyc_CfFlowInfo_One;}if(changed2){if(!Cyc_CfFlowInfo_prefix_of_member(((
env->joinenv).pile)->rgn,env->curr_place,env->chg1))return r2;env->changed=Cyc_CfFlowInfo_Two;}{
struct _tuple0 _tmp14C=({struct _tuple0 _tmp14B;_tmp14B.f1=r1;_tmp14B.f2=r2;_tmp14B;});
void*_tmp14D;struct Cyc_Dict_Dict*_tmp14E;void*_tmp14F;struct Cyc_Dict_Dict*
_tmp150;_LLD8: _tmp14D=_tmp14C.f1;if(_tmp14D <= (void*)3?1:*((int*)_tmp14D)!= 4)
goto _LLDA;_tmp14E=((struct Cyc_CfFlowInfo_Aggregate_struct*)_tmp14D)->f1;_tmp14F=
_tmp14C.f2;if(_tmp14F <= (void*)3?1:*((int*)_tmp14F)!= 4)goto _LLDA;_tmp150=((
struct Cyc_CfFlowInfo_Aggregate_struct*)_tmp14F)->f1;_LLD9: {struct Cyc_Dict_Dict*
_tmp151=((struct Cyc_Dict_Dict*(*)(void*(*f)(struct Cyc_CfFlowInfo_AfterEnv*,
struct _tagged_arr*,void*,void*),struct Cyc_CfFlowInfo_AfterEnv*env,struct Cyc_Dict_Dict*
d1,struct Cyc_Dict_Dict*d2))Cyc_Dict_union_two_c)(Cyc_CfFlowInfo_after_absRval_d,
env,_tmp14E,_tmp150);if(_tmp151 == _tmp14E)return r1;if(_tmp151 == _tmp150)return r2;
return(void*)({struct Cyc_CfFlowInfo_Aggregate_struct*_tmp152=_cycalloc(sizeof(*
_tmp152));_tmp152[0]=({struct Cyc_CfFlowInfo_Aggregate_struct _tmp153;_tmp153.tag=
4;_tmp153.f1=_tmp151;_tmp153;});_tmp152;});}_LLDA:;_LLDB:(int)_throw((void*)({
struct Cyc_Core_Impossible_struct*_tmp154=_cycalloc(sizeof(*_tmp154));_tmp154[0]=({
struct Cyc_Core_Impossible_struct _tmp155;_tmp155.tag=Cyc_Core_Impossible;_tmp155.f1=({
const char*_tmp156="after_absRval -- non-aggregates!";_tag_arr(_tmp156,sizeof(
char),_get_zero_arr_size(_tmp156,33));});_tmp155;});_tmp154;}));_LLD7:;}}static
void*Cyc_CfFlowInfo_after_absRval_d(struct Cyc_CfFlowInfo_AfterEnv*env,struct
_tagged_arr*key,void*r1,void*r2){if(r1 == r2)return r1;{struct Cyc_List_List**
_tmp157=env->last_field_cell;enum Cyc_CfFlowInfo_WhoIsChanged _tmp158=env->changed;*
env->last_field_cell=({struct Cyc_List_List*_tmp159=_cycalloc(sizeof(*_tmp159));
_tmp159->hd=key;_tmp159->tl=0;_tmp159;});env->last_field_cell=&((struct Cyc_List_List*)
_check_null(*env->last_field_cell))->tl;{void*_tmp15A=Cyc_CfFlowInfo_after_absRval(
env,r1,r2);env->last_field_cell=_tmp157;((struct Cyc_List_List*)_check_null(*env->last_field_cell))->tl=
0;env->changed=_tmp158;return _tmp15A;}}}static void*Cyc_CfFlowInfo_after_root(
struct Cyc_CfFlowInfo_AfterEnv*env,void*root,void*r1,void*r2){if(r1 == r2)return r1;*
env->curr_place=({struct Cyc_CfFlowInfo_Place _tmp15B;_tmp15B.root=(void*)root;
_tmp15B.fields=0;_tmp15B;});env->last_field_cell=&(env->curr_place)->fields;env->changed=
Cyc_CfFlowInfo_Neither;return Cyc_CfFlowInfo_after_absRval(env,r1,r2);}void*Cyc_CfFlowInfo_after_flow(
struct Cyc_Set_Set**all_changed,void*f1,void*f2,struct Cyc_Set_Set*chg1,struct Cyc_Set_Set*
chg2){static struct Cyc_Absyn_Const_e_struct dummy_rawexp={0,(void*)((void*)0)};
static struct Cyc_Absyn_Exp dummy_exp={0,(void*)((void*)& dummy_rawexp),0,(void*)((
void*)Cyc_Absyn_EmptyAnnot)};static struct Cyc_CfFlowInfo_MallocPt_struct
dummy_root={1,& dummy_exp,(void*)((void*)0)};if(f1 == f2)return f1;{struct _tuple0
_tmp15D=({struct _tuple0 _tmp15C;_tmp15C.f1=f1;_tmp15C.f2=f2;_tmp15C;});void*
_tmp15E;void*_tmp15F;void*_tmp160;struct Cyc_Dict_Dict*_tmp161;struct Cyc_List_List*
_tmp162;void*_tmp163;struct Cyc_Dict_Dict*_tmp164;struct Cyc_List_List*_tmp165;
_LLDD: _tmp15E=_tmp15D.f1;if((int)_tmp15E != 0)goto _LLDF;_LLDE: goto _LLE0;_LLDF:
_tmp15F=_tmp15D.f2;if((int)_tmp15F != 0)goto _LLE1;_LLE0: return(void*)0;_LLE1:
_tmp160=_tmp15D.f1;if(_tmp160 <= (void*)1?1:*((int*)_tmp160)!= 0)goto _LLDC;
_tmp161=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp160)->f1;_tmp162=((struct
Cyc_CfFlowInfo_ReachableFL_struct*)_tmp160)->f2;_tmp163=_tmp15D.f2;if(_tmp163 <= (
void*)1?1:*((int*)_tmp163)!= 0)goto _LLDC;_tmp164=((struct Cyc_CfFlowInfo_ReachableFL_struct*)
_tmp163)->f1;_tmp165=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp163)->f2;
_LLE2: if(_tmp161 == _tmp164?_tmp162 == _tmp165: 0)return f1;{struct _RegionHandle
_tmp166=_new_region("rgn");struct _RegionHandle*rgn=& _tmp166;_push_region(rgn);{
struct Cyc_CfFlowInfo_Place*_tmp167=({struct Cyc_CfFlowInfo_Place*_tmp171=
_cycalloc(sizeof(*_tmp171));_tmp171->root=(void*)((void*)& dummy_root);_tmp171->fields=
0;_tmp171;});struct Cyc_CfFlowInfo_AfterEnv _tmp168=({struct Cyc_CfFlowInfo_AfterEnv
_tmp16E;_tmp16E.joinenv=({struct Cyc_CfFlowInfo_JoinEnv _tmp16F;_tmp16F.pile=({
struct Cyc_CfFlowInfo_EscPile*_tmp170=_region_malloc(rgn,sizeof(*_tmp170));
_tmp170->rgn=rgn;_tmp170->places=0;_tmp170;});_tmp16F.d1=_tmp161;_tmp16F.d2=
_tmp164;_tmp16F;});_tmp16E.chg1=chg1;_tmp16E.chg2=chg2;_tmp16E.curr_place=
_tmp167;_tmp16E.last_field_cell=& _tmp167->fields;_tmp16E.changed=Cyc_CfFlowInfo_Neither;
_tmp16E;});struct Cyc_Dict_Dict*_tmp169=((struct Cyc_Dict_Dict*(*)(void*(*f)(
struct Cyc_CfFlowInfo_AfterEnv*,void*,void*,void*),struct Cyc_CfFlowInfo_AfterEnv*
env,struct Cyc_Dict_Dict*d1,struct Cyc_Dict_Dict*d2))Cyc_Dict_union_two_c)(Cyc_CfFlowInfo_after_root,&
_tmp168,_tmp161,_tmp164);struct Cyc_List_List*_tmp16A=Cyc_CfFlowInfo_join_relns(
_tmp162,_tmp165);void*_tmp16D=(void*)({struct Cyc_CfFlowInfo_ReachableFL_struct*
_tmp16B=_cycalloc(sizeof(*_tmp16B));_tmp16B[0]=({struct Cyc_CfFlowInfo_ReachableFL_struct
_tmp16C;_tmp16C.tag=0;_tmp16C.f1=Cyc_CfFlowInfo_escape_these((_tmp168.joinenv).pile,
all_changed,_tmp169);_tmp16C.f2=_tmp16A;_tmp16C;});_tmp16B;});_npop_handler(0);
return _tmp16D;};_pop_region(rgn);}_LLDC:;}}static int Cyc_CfFlowInfo_tag_cmps_lessthan_approx(
struct Cyc_List_List*l1,struct Cyc_List_List*l2){for(0;l2 != 0;l2=l2->tl){struct Cyc_CfFlowInfo_TagCmp
_tmp175;void*_tmp176;void*_tmp177;struct Cyc_CfFlowInfo_TagCmp*_tmp174=(struct Cyc_CfFlowInfo_TagCmp*)
l2->hd;_tmp175=*_tmp174;_tmp176=(void*)_tmp175.cmp;_tmp177=(void*)_tmp175.bd;{
struct Cyc_List_List*_tmp178=l1;for(0;_tmp178 != 0;_tmp178=_tmp178->tl){struct Cyc_CfFlowInfo_TagCmp
_tmp17A;void*_tmp17B;void*_tmp17C;struct Cyc_CfFlowInfo_TagCmp*_tmp179=(struct Cyc_CfFlowInfo_TagCmp*)
_tmp178->hd;_tmp17A=*_tmp179;_tmp17B=(void*)_tmp17A.cmp;_tmp17C=(void*)_tmp17A.bd;
if(_tmp17B == _tmp176?Cyc_Tcutil_typecmp(_tmp17C,_tmp177)== 0: 0)break;}if(_tmp178
== 0)return 0;}}return 1;}static int Cyc_CfFlowInfo_absRval_lessthan_approx(void*
ignore,void*r1,void*r2){if(r1 == r2)return 1;{struct _tuple0 _tmp17E=({struct _tuple0
_tmp17D;_tmp17D.f1=r1;_tmp17D.f2=r2;_tmp17D;});void*_tmp17F;struct Cyc_CfFlowInfo_Place*
_tmp180;void*_tmp181;struct Cyc_CfFlowInfo_Place*_tmp182;void*_tmp183;void*
_tmp184;void*_tmp185;struct Cyc_Dict_Dict*_tmp186;void*_tmp187;struct Cyc_Dict_Dict*
_tmp188;void*_tmp189;void*_tmp18A;void*_tmp18B;void*_tmp18C;void*_tmp18D;void*
_tmp18E;void*_tmp18F;struct Cyc_List_List*_tmp190;void*_tmp191;struct Cyc_List_List*
_tmp192;void*_tmp193;_LLE4: _tmp17F=_tmp17E.f1;if(_tmp17F <= (void*)3?1:*((int*)
_tmp17F)!= 2)goto _LLE6;_tmp180=((struct Cyc_CfFlowInfo_AddressOf_struct*)_tmp17F)->f1;
_tmp181=_tmp17E.f2;if(_tmp181 <= (void*)3?1:*((int*)_tmp181)!= 2)goto _LLE6;
_tmp182=((struct Cyc_CfFlowInfo_AddressOf_struct*)_tmp181)->f1;_LLE5: return Cyc_CfFlowInfo_place_cmp(
_tmp180,_tmp182)== 0;_LLE6: _tmp183=_tmp17E.f1;if(_tmp183 <= (void*)3?1:*((int*)
_tmp183)!= 2)goto _LLE8;_LLE7: goto _LLE9;_LLE8: _tmp184=_tmp17E.f2;if(_tmp184 <= (
void*)3?1:*((int*)_tmp184)!= 2)goto _LLEA;_LLE9: return 0;_LLEA: _tmp185=_tmp17E.f1;
if(_tmp185 <= (void*)3?1:*((int*)_tmp185)!= 4)goto _LLEC;_tmp186=((struct Cyc_CfFlowInfo_Aggregate_struct*)
_tmp185)->f1;_tmp187=_tmp17E.f2;if(_tmp187 <= (void*)3?1:*((int*)_tmp187)!= 4)
goto _LLEC;_tmp188=((struct Cyc_CfFlowInfo_Aggregate_struct*)_tmp187)->f1;_LLEB:
return _tmp186 == _tmp188?1:((int(*)(int(*f)(struct _tagged_arr*,void*,void*),
struct Cyc_Dict_Dict*d1,struct Cyc_Dict_Dict*d2))Cyc_Dict_forall_intersect)((int(*)(
struct _tagged_arr*ignore,void*r1,void*r2))Cyc_CfFlowInfo_absRval_lessthan_approx,
_tmp186,_tmp188);_LLEC: _tmp189=_tmp17E.f2;if((int)_tmp189 != 2)goto _LLEE;_LLED:
return r1 == (void*)1;_LLEE: _tmp18A=_tmp17E.f2;if((int)_tmp18A != 0)goto _LLF0;_LLEF:
goto _LLF1;_LLF0: _tmp18B=_tmp17E.f2;if((int)_tmp18B != 1)goto _LLF2;_LLF1: return 0;
_LLF2: _tmp18C=_tmp17E.f1;if(_tmp18C <= (void*)3?1:*((int*)_tmp18C)!= 1)goto _LLF4;
_tmp18D=_tmp17E.f2;if(_tmp18D <= (void*)3?1:*((int*)_tmp18D)!= 1)goto _LLF4;_LLF3:
goto _LLE3;_LLF4: _tmp18E=_tmp17E.f1;if(_tmp18E <= (void*)3?1:*((int*)_tmp18E)!= 1)
goto _LLF6;_LLF5: return 0;_LLF6: _tmp18F=_tmp17E.f1;if(_tmp18F <= (void*)3?1:*((int*)
_tmp18F)!= 3)goto _LLF8;_tmp190=((struct Cyc_CfFlowInfo_TagCmps_struct*)_tmp18F)->f1;
_tmp191=_tmp17E.f2;if(_tmp191 <= (void*)3?1:*((int*)_tmp191)!= 3)goto _LLF8;
_tmp192=((struct Cyc_CfFlowInfo_TagCmps_struct*)_tmp191)->f1;_LLF7: return Cyc_CfFlowInfo_tag_cmps_lessthan_approx(
_tmp190,_tmp192);_LLF8: _tmp193=_tmp17E.f2;if(_tmp193 <= (void*)3?1:*((int*)
_tmp193)!= 3)goto _LLFA;_LLF9: return 0;_LLFA:;_LLFB: goto _LLE3;_LLE3:;}{struct
_tuple0 _tmp195=({struct _tuple0 _tmp194;_tmp194.f1=Cyc_CfFlowInfo_initlevel_approx(
r1);_tmp194.f2=Cyc_CfFlowInfo_initlevel_approx(r2);_tmp194;});void*_tmp196;void*
_tmp197;void*_tmp198;void*_tmp199;void*_tmp19A;void*_tmp19B;_LLFD: _tmp196=
_tmp195.f1;if((int)_tmp196 != 2)goto _LLFF;_tmp197=_tmp195.f2;if((int)_tmp197 != 2)
goto _LLFF;_LLFE: return 1;_LLFF: _tmp198=_tmp195.f2;if((int)_tmp198 != 0)goto _LL101;
_LL100: return 1;_LL101: _tmp199=_tmp195.f1;if((int)_tmp199 != 0)goto _LL103;_LL102:
return 0;_LL103: _tmp19A=_tmp195.f2;if((int)_tmp19A != 1)goto _LL105;_LL104: return 1;
_LL105: _tmp19B=_tmp195.f1;if((int)_tmp19B != 1)goto _LLFC;_LL106: return 0;_LLFC:;}}
static int Cyc_CfFlowInfo_relns_approx(struct Cyc_List_List*r2s,struct Cyc_List_List*
r1s){if(r1s == r2s)return 1;for(0;r1s != 0;r1s=r1s->tl){struct Cyc_CfFlowInfo_Reln*
_tmp19C=(struct Cyc_CfFlowInfo_Reln*)r1s->hd;int found=0;{struct Cyc_List_List*
_tmp19D=r2s;for(0;_tmp19D != 0;_tmp19D=_tmp19D->tl){struct Cyc_CfFlowInfo_Reln*
_tmp19E=(struct Cyc_CfFlowInfo_Reln*)_tmp19D->hd;if(_tmp19C == _tmp19E?1:(_tmp19C->vd
== _tmp19E->vd?Cyc_CfFlowInfo_same_relop((void*)_tmp19C->rop,(void*)_tmp19E->rop):
0)){found=1;break;}}}if(!found)return 0;}return 1;}int Cyc_CfFlowInfo_flow_lessthan_approx(
void*f1,void*f2){if(f1 == f2)return 1;{struct _tuple0 _tmp1A0=({struct _tuple0 _tmp19F;
_tmp19F.f1=f1;_tmp19F.f2=f2;_tmp19F;});void*_tmp1A1;void*_tmp1A2;void*_tmp1A3;
struct Cyc_Dict_Dict*_tmp1A4;struct Cyc_List_List*_tmp1A5;void*_tmp1A6;struct Cyc_Dict_Dict*
_tmp1A7;struct Cyc_List_List*_tmp1A8;_LL108: _tmp1A1=_tmp1A0.f1;if((int)_tmp1A1 != 
0)goto _LL10A;_LL109: return 1;_LL10A: _tmp1A2=_tmp1A0.f2;if((int)_tmp1A2 != 0)goto
_LL10C;_LL10B: return 0;_LL10C: _tmp1A3=_tmp1A0.f1;if(_tmp1A3 <= (void*)1?1:*((int*)
_tmp1A3)!= 0)goto _LL107;_tmp1A4=((struct Cyc_CfFlowInfo_ReachableFL_struct*)
_tmp1A3)->f1;_tmp1A5=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp1A3)->f2;
_tmp1A6=_tmp1A0.f2;if(_tmp1A6 <= (void*)1?1:*((int*)_tmp1A6)!= 0)goto _LL107;
_tmp1A7=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp1A6)->f1;_tmp1A8=((struct
Cyc_CfFlowInfo_ReachableFL_struct*)_tmp1A6)->f2;_LL10D: if(_tmp1A4 == _tmp1A7?
_tmp1A5 == _tmp1A8: 0)return 1;return Cyc_Dict_forall_intersect(Cyc_CfFlowInfo_absRval_lessthan_approx,
_tmp1A4,_tmp1A7)?Cyc_CfFlowInfo_relns_approx(_tmp1A5,_tmp1A8): 0;_LL107:;}}struct
Cyc_List_List*Cyc_CfFlowInfo_reln_kill_var(struct Cyc_List_List*rs,struct Cyc_Absyn_Vardecl*
v){struct Cyc_List_List*p;int found=0;for(p=rs;!found?p != 0: 0;p=p->tl){struct Cyc_CfFlowInfo_Reln*
_tmp1A9=(struct Cyc_CfFlowInfo_Reln*)p->hd;if(_tmp1A9->vd == v){found=1;break;}{
void*_tmp1AA=(void*)_tmp1A9->rop;struct Cyc_Absyn_Vardecl*_tmp1AB;struct Cyc_Absyn_Vardecl*
_tmp1AC;struct Cyc_Absyn_Vardecl*_tmp1AD;_LL10F: if(*((int*)_tmp1AA)!= 1)goto
_LL111;_tmp1AB=((struct Cyc_CfFlowInfo_LessVar_struct*)_tmp1AA)->f1;_LL110:
_tmp1AC=_tmp1AB;goto _LL112;_LL111: if(*((int*)_tmp1AA)!= 2)goto _LL113;_tmp1AC=((
struct Cyc_CfFlowInfo_LessSize_struct*)_tmp1AA)->f1;_LL112: _tmp1AD=_tmp1AC;goto
_LL114;_LL113: if(*((int*)_tmp1AA)!= 4)goto _LL115;_tmp1AD=((struct Cyc_CfFlowInfo_LessEqSize_struct*)
_tmp1AA)->f1;_LL114: if(v == _tmp1AD)found=1;goto _LL10E;_LL115:;_LL116: goto _LL10E;
_LL10E:;}}if(!found)return rs;{struct Cyc_List_List*_tmp1AE=0;for(p=rs;p != 0;p=p->tl){
struct Cyc_CfFlowInfo_Reln*_tmp1AF=(struct Cyc_CfFlowInfo_Reln*)p->hd;if(_tmp1AF->vd
!= v){{void*_tmp1B0=(void*)_tmp1AF->rop;struct Cyc_Absyn_Vardecl*_tmp1B1;struct
Cyc_Absyn_Vardecl*_tmp1B2;struct Cyc_Absyn_Vardecl*_tmp1B3;_LL118: if(*((int*)
_tmp1B0)!= 1)goto _LL11A;_tmp1B1=((struct Cyc_CfFlowInfo_LessVar_struct*)_tmp1B0)->f1;
_LL119: _tmp1B2=_tmp1B1;goto _LL11B;_LL11A: if(*((int*)_tmp1B0)!= 2)goto _LL11C;
_tmp1B2=((struct Cyc_CfFlowInfo_LessSize_struct*)_tmp1B0)->f1;_LL11B: _tmp1B3=
_tmp1B2;goto _LL11D;_LL11C: if(*((int*)_tmp1B0)!= 4)goto _LL11E;_tmp1B3=((struct Cyc_CfFlowInfo_LessEqSize_struct*)
_tmp1B0)->f1;_LL11D: if(v == _tmp1B3)continue;goto _LL117;_LL11E:;_LL11F: goto _LL117;
_LL117:;}_tmp1AE=({struct Cyc_List_List*_tmp1B4=_cycalloc(sizeof(*_tmp1B4));
_tmp1B4->hd=_tmp1AF;_tmp1B4->tl=_tmp1AE;_tmp1B4;});}}return _tmp1AE;}}struct Cyc_List_List*
Cyc_CfFlowInfo_reln_kill_exp(struct Cyc_List_List*r,struct Cyc_Absyn_Exp*e){{void*
_tmp1B5=(void*)e->r;void*_tmp1B6;struct Cyc_Absyn_Vardecl*_tmp1B7;void*_tmp1B8;
struct Cyc_Absyn_Vardecl*_tmp1B9;void*_tmp1BA;struct Cyc_Absyn_Vardecl*_tmp1BB;
void*_tmp1BC;struct Cyc_Absyn_Vardecl*_tmp1BD;_LL121: if(*((int*)_tmp1B5)!= 1)goto
_LL123;_tmp1B6=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1B5)->f2;if(_tmp1B6 <= (
void*)1?1:*((int*)_tmp1B6)!= 0)goto _LL123;_tmp1B7=((struct Cyc_Absyn_Global_b_struct*)
_tmp1B6)->f1;_LL122: _tmp1B9=_tmp1B7;goto _LL124;_LL123: if(*((int*)_tmp1B5)!= 1)
goto _LL125;_tmp1B8=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1B5)->f2;if(
_tmp1B8 <= (void*)1?1:*((int*)_tmp1B8)!= 2)goto _LL125;_tmp1B9=((struct Cyc_Absyn_Param_b_struct*)
_tmp1B8)->f1;_LL124: _tmp1BB=_tmp1B9;goto _LL126;_LL125: if(*((int*)_tmp1B5)!= 1)
goto _LL127;_tmp1BA=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1B5)->f2;if(
_tmp1BA <= (void*)1?1:*((int*)_tmp1BA)!= 3)goto _LL127;_tmp1BB=((struct Cyc_Absyn_Local_b_struct*)
_tmp1BA)->f1;_LL126: _tmp1BD=_tmp1BB;goto _LL128;_LL127: if(*((int*)_tmp1B5)!= 1)
goto _LL129;_tmp1BC=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1B5)->f2;if(
_tmp1BC <= (void*)1?1:*((int*)_tmp1BC)!= 4)goto _LL129;_tmp1BD=((struct Cyc_Absyn_Pat_b_struct*)
_tmp1BC)->f1;_LL128: if(!_tmp1BD->escapes)return Cyc_CfFlowInfo_reln_kill_var(r,
_tmp1BD);goto _LL120;_LL129:;_LL12A: goto _LL120;_LL120:;}return r;}struct Cyc_List_List*
Cyc_CfFlowInfo_reln_assign_var(struct Cyc_List_List*r,struct Cyc_Absyn_Vardecl*v,
struct Cyc_Absyn_Exp*e){if(v->escapes)return r;r=Cyc_CfFlowInfo_reln_kill_var(r,v);{
void*_tmp1BE=(void*)e->r;struct Cyc_Absyn_MallocInfo _tmp1BF;struct Cyc_Absyn_Exp*
_tmp1C0;int _tmp1C1;_LL12C: if(*((int*)_tmp1BE)!= 33)goto _LL12E;_tmp1BF=((struct
Cyc_Absyn_Malloc_e_struct*)_tmp1BE)->f1;_tmp1C0=_tmp1BF.num_elts;_tmp1C1=_tmp1BF.fat_result;
if(_tmp1C1 != 1)goto _LL12E;_LL12D: malloc_loop: {void*_tmp1C2=(void*)_tmp1C0->r;
struct Cyc_Absyn_Exp*_tmp1C3;void*_tmp1C4;struct Cyc_Absyn_Vardecl*_tmp1C5;void*
_tmp1C6;struct Cyc_Absyn_Vardecl*_tmp1C7;void*_tmp1C8;struct Cyc_Absyn_Vardecl*
_tmp1C9;void*_tmp1CA;struct Cyc_Absyn_Vardecl*_tmp1CB;_LL131: if(*((int*)_tmp1C2)
!= 13)goto _LL133;_tmp1C3=((struct Cyc_Absyn_Cast_e_struct*)_tmp1C2)->f2;_LL132:
_tmp1C0=_tmp1C3;goto malloc_loop;_LL133: if(*((int*)_tmp1C2)!= 1)goto _LL135;
_tmp1C4=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1C2)->f2;if(_tmp1C4 <= (void*)
1?1:*((int*)_tmp1C4)!= 4)goto _LL135;_tmp1C5=((struct Cyc_Absyn_Pat_b_struct*)
_tmp1C4)->f1;_LL134: _tmp1C7=_tmp1C5;goto _LL136;_LL135: if(*((int*)_tmp1C2)!= 1)
goto _LL137;_tmp1C6=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1C2)->f2;if(
_tmp1C6 <= (void*)1?1:*((int*)_tmp1C6)!= 3)goto _LL137;_tmp1C7=((struct Cyc_Absyn_Local_b_struct*)
_tmp1C6)->f1;_LL136: _tmp1C9=_tmp1C7;goto _LL138;_LL137: if(*((int*)_tmp1C2)!= 1)
goto _LL139;_tmp1C8=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1C2)->f2;if(
_tmp1C8 <= (void*)1?1:*((int*)_tmp1C8)!= 2)goto _LL139;_tmp1C9=((struct Cyc_Absyn_Param_b_struct*)
_tmp1C8)->f1;_LL138: _tmp1CB=_tmp1C9;goto _LL13A;_LL139: if(*((int*)_tmp1C2)!= 1)
goto _LL13B;_tmp1CA=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1C2)->f2;if(
_tmp1CA <= (void*)1?1:*((int*)_tmp1CA)!= 0)goto _LL13B;_tmp1CB=((struct Cyc_Absyn_Global_b_struct*)
_tmp1CA)->f1;_LL13A: if(_tmp1CB->escapes)return r;return({struct Cyc_List_List*
_tmp1CC=_cycalloc(sizeof(*_tmp1CC));_tmp1CC->hd=({struct Cyc_CfFlowInfo_Reln*
_tmp1CD=_cycalloc(sizeof(*_tmp1CD));_tmp1CD->vd=_tmp1CB;_tmp1CD->rop=(void*)((
void*)({struct Cyc_CfFlowInfo_LessEqSize_struct*_tmp1CE=_cycalloc(sizeof(*_tmp1CE));
_tmp1CE[0]=({struct Cyc_CfFlowInfo_LessEqSize_struct _tmp1CF;_tmp1CF.tag=4;_tmp1CF.f1=
v;_tmp1CF;});_tmp1CE;}));_tmp1CD;});_tmp1CC->tl=r;_tmp1CC;});_LL13B:;_LL13C:
return r;_LL130:;}_LL12E:;_LL12F: goto _LL12B;_LL12B:;}{void*_tmp1D0=Cyc_Tcutil_compress((
void*)v->type);_LL13E: if(_tmp1D0 <= (void*)3?1:*((int*)_tmp1D0)!= 5)goto _LL140;
_LL13F: goto _LL13D;_LL140:;_LL141: return r;_LL13D:;}loop: {void*_tmp1D1=(void*)e->r;
struct Cyc_Absyn_Exp*_tmp1D2;void*_tmp1D3;int _tmp1D4;void*_tmp1D5;struct Cyc_List_List*
_tmp1D6;struct Cyc_List_List _tmp1D7;struct Cyc_List_List*_tmp1D8;struct Cyc_List_List
_tmp1D9;struct Cyc_Absyn_Exp*_tmp1DA;void*_tmp1DB;struct Cyc_List_List*_tmp1DC;
struct Cyc_List_List _tmp1DD;struct Cyc_Absyn_Exp*_tmp1DE;_LL143: if(*((int*)_tmp1D1)
!= 13)goto _LL145;_tmp1D2=((struct Cyc_Absyn_Cast_e_struct*)_tmp1D1)->f2;_LL144: e=
_tmp1D2;goto loop;_LL145: if(*((int*)_tmp1D1)!= 0)goto _LL147;_tmp1D3=(void*)((
struct Cyc_Absyn_Const_e_struct*)_tmp1D1)->f1;if(_tmp1D3 <= (void*)1?1:*((int*)
_tmp1D3)!= 2)goto _LL147;_tmp1D4=((struct Cyc_Absyn_Int_c_struct*)_tmp1D3)->f2;
_LL146: return({struct Cyc_List_List*_tmp1DF=_cycalloc(sizeof(*_tmp1DF));_tmp1DF->hd=({
struct Cyc_CfFlowInfo_Reln*_tmp1E0=_cycalloc(sizeof(*_tmp1E0));_tmp1E0->vd=v;
_tmp1E0->rop=(void*)((void*)({struct Cyc_CfFlowInfo_EqualConst_struct*_tmp1E1=
_cycalloc_atomic(sizeof(*_tmp1E1));_tmp1E1[0]=({struct Cyc_CfFlowInfo_EqualConst_struct
_tmp1E2;_tmp1E2.tag=0;_tmp1E2.f1=(unsigned int)_tmp1D4;_tmp1E2;});_tmp1E1;}));
_tmp1E0;});_tmp1DF->tl=r;_tmp1DF;});_LL147: if(*((int*)_tmp1D1)!= 3)goto _LL149;
_tmp1D5=(void*)((struct Cyc_Absyn_Primop_e_struct*)_tmp1D1)->f1;if((int)_tmp1D5 != 
4)goto _LL149;_tmp1D6=((struct Cyc_Absyn_Primop_e_struct*)_tmp1D1)->f2;if(_tmp1D6
== 0)goto _LL149;_tmp1D7=*_tmp1D6;_tmp1D8=_tmp1D7.tl;if(_tmp1D8 == 0)goto _LL149;
_tmp1D9=*_tmp1D8;_tmp1DA=(struct Cyc_Absyn_Exp*)_tmp1D9.hd;_LL148:{void*_tmp1E3=(
void*)_tmp1DA->r;void*_tmp1E4;struct Cyc_List_List*_tmp1E5;struct Cyc_List_List
_tmp1E6;struct Cyc_Absyn_Exp*_tmp1E7;_LL14E: if(*((int*)_tmp1E3)!= 3)goto _LL150;
_tmp1E4=(void*)((struct Cyc_Absyn_Primop_e_struct*)_tmp1E3)->f1;if((int)_tmp1E4 != 
19)goto _LL150;_tmp1E5=((struct Cyc_Absyn_Primop_e_struct*)_tmp1E3)->f2;if(_tmp1E5
== 0)goto _LL150;_tmp1E6=*_tmp1E5;_tmp1E7=(struct Cyc_Absyn_Exp*)_tmp1E6.hd;_LL14F:{
void*_tmp1E8=(void*)_tmp1E7->r;void*_tmp1E9;struct Cyc_Absyn_Vardecl*_tmp1EA;void*
_tmp1EB;struct Cyc_Absyn_Vardecl*_tmp1EC;void*_tmp1ED;struct Cyc_Absyn_Vardecl*
_tmp1EE;void*_tmp1EF;struct Cyc_Absyn_Vardecl*_tmp1F0;_LL153: if(*((int*)_tmp1E8)
!= 1)goto _LL155;_tmp1E9=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1E8)->f2;if(
_tmp1E9 <= (void*)1?1:*((int*)_tmp1E9)!= 0)goto _LL155;_tmp1EA=((struct Cyc_Absyn_Global_b_struct*)
_tmp1E9)->f1;_LL154: _tmp1EC=_tmp1EA;goto _LL156;_LL155: if(*((int*)_tmp1E8)!= 1)
goto _LL157;_tmp1EB=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1E8)->f2;if(
_tmp1EB <= (void*)1?1:*((int*)_tmp1EB)!= 3)goto _LL157;_tmp1EC=((struct Cyc_Absyn_Local_b_struct*)
_tmp1EB)->f1;_LL156: _tmp1EE=_tmp1EC;goto _LL158;_LL157: if(*((int*)_tmp1E8)!= 1)
goto _LL159;_tmp1ED=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1E8)->f2;if(
_tmp1ED <= (void*)1?1:*((int*)_tmp1ED)!= 2)goto _LL159;_tmp1EE=((struct Cyc_Absyn_Param_b_struct*)
_tmp1ED)->f1;_LL158: _tmp1F0=_tmp1EE;goto _LL15A;_LL159: if(*((int*)_tmp1E8)!= 1)
goto _LL15B;_tmp1EF=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1E8)->f2;if(
_tmp1EF <= (void*)1?1:*((int*)_tmp1EF)!= 4)goto _LL15B;_tmp1F0=((struct Cyc_Absyn_Pat_b_struct*)
_tmp1EF)->f1;_LL15A: if(_tmp1F0->escapes)return r;return({struct Cyc_List_List*
_tmp1F1=_cycalloc(sizeof(*_tmp1F1));_tmp1F1->hd=({struct Cyc_CfFlowInfo_Reln*
_tmp1F2=_cycalloc(sizeof(*_tmp1F2));_tmp1F2->vd=v;_tmp1F2->rop=(void*)((void*)({
struct Cyc_CfFlowInfo_LessSize_struct*_tmp1F3=_cycalloc(sizeof(*_tmp1F3));_tmp1F3[
0]=({struct Cyc_CfFlowInfo_LessSize_struct _tmp1F4;_tmp1F4.tag=2;_tmp1F4.f1=
_tmp1F0;_tmp1F4;});_tmp1F3;}));_tmp1F2;});_tmp1F1->tl=r;_tmp1F1;});_LL15B:;
_LL15C: goto _LL152;_LL152:;}goto _LL14D;_LL150:;_LL151: goto _LL14D;_LL14D:;}goto
_LL142;_LL149: if(*((int*)_tmp1D1)!= 3)goto _LL14B;_tmp1DB=(void*)((struct Cyc_Absyn_Primop_e_struct*)
_tmp1D1)->f1;if((int)_tmp1DB != 19)goto _LL14B;_tmp1DC=((struct Cyc_Absyn_Primop_e_struct*)
_tmp1D1)->f2;if(_tmp1DC == 0)goto _LL14B;_tmp1DD=*_tmp1DC;_tmp1DE=(struct Cyc_Absyn_Exp*)
_tmp1DD.hd;_LL14A:{void*_tmp1F5=(void*)_tmp1DE->r;void*_tmp1F6;struct Cyc_Absyn_Vardecl*
_tmp1F7;void*_tmp1F8;struct Cyc_Absyn_Vardecl*_tmp1F9;void*_tmp1FA;struct Cyc_Absyn_Vardecl*
_tmp1FB;void*_tmp1FC;struct Cyc_Absyn_Vardecl*_tmp1FD;_LL15E: if(*((int*)_tmp1F5)
!= 1)goto _LL160;_tmp1F6=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1F5)->f2;if(
_tmp1F6 <= (void*)1?1:*((int*)_tmp1F6)!= 0)goto _LL160;_tmp1F7=((struct Cyc_Absyn_Global_b_struct*)
_tmp1F6)->f1;_LL15F: _tmp1F9=_tmp1F7;goto _LL161;_LL160: if(*((int*)_tmp1F5)!= 1)
goto _LL162;_tmp1F8=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1F5)->f2;if(
_tmp1F8 <= (void*)1?1:*((int*)_tmp1F8)!= 3)goto _LL162;_tmp1F9=((struct Cyc_Absyn_Local_b_struct*)
_tmp1F8)->f1;_LL161: _tmp1FB=_tmp1F9;goto _LL163;_LL162: if(*((int*)_tmp1F5)!= 1)
goto _LL164;_tmp1FA=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1F5)->f2;if(
_tmp1FA <= (void*)1?1:*((int*)_tmp1FA)!= 2)goto _LL164;_tmp1FB=((struct Cyc_Absyn_Param_b_struct*)
_tmp1FA)->f1;_LL163: _tmp1FD=_tmp1FB;goto _LL165;_LL164: if(*((int*)_tmp1F5)!= 1)
goto _LL166;_tmp1FC=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp1F5)->f2;if(
_tmp1FC <= (void*)1?1:*((int*)_tmp1FC)!= 4)goto _LL166;_tmp1FD=((struct Cyc_Absyn_Pat_b_struct*)
_tmp1FC)->f1;_LL165: if(_tmp1FD->escapes)return r;return({struct Cyc_List_List*
_tmp1FE=_cycalloc(sizeof(*_tmp1FE));_tmp1FE->hd=({struct Cyc_CfFlowInfo_Reln*
_tmp1FF=_cycalloc(sizeof(*_tmp1FF));_tmp1FF->vd=v;_tmp1FF->rop=(void*)((void*)({
struct Cyc_CfFlowInfo_LessEqSize_struct*_tmp200=_cycalloc(sizeof(*_tmp200));
_tmp200[0]=({struct Cyc_CfFlowInfo_LessEqSize_struct _tmp201;_tmp201.tag=4;_tmp201.f1=
_tmp1FD;_tmp201;});_tmp200;}));_tmp1FF;});_tmp1FE->tl=r;_tmp1FE;});_LL166:;
_LL167: goto _LL15D;_LL15D:;}goto _LL142;_LL14B:;_LL14C: goto _LL142;_LL142:;}return r;}
struct Cyc_List_List*Cyc_CfFlowInfo_reln_assign_exp(struct Cyc_List_List*r,struct
Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){{void*_tmp202=(void*)e1->r;void*_tmp203;
struct Cyc_Absyn_Vardecl*_tmp204;void*_tmp205;struct Cyc_Absyn_Vardecl*_tmp206;
void*_tmp207;struct Cyc_Absyn_Vardecl*_tmp208;void*_tmp209;struct Cyc_Absyn_Vardecl*
_tmp20A;_LL169: if(*((int*)_tmp202)!= 1)goto _LL16B;_tmp203=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp202)->f2;if(_tmp203 <= (void*)1?1:*((int*)_tmp203)!= 0)goto _LL16B;_tmp204=((
struct Cyc_Absyn_Global_b_struct*)_tmp203)->f1;_LL16A: _tmp206=_tmp204;goto _LL16C;
_LL16B: if(*((int*)_tmp202)!= 1)goto _LL16D;_tmp205=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp202)->f2;if(_tmp205 <= (void*)1?1:*((int*)_tmp205)!= 2)goto _LL16D;_tmp206=((
struct Cyc_Absyn_Param_b_struct*)_tmp205)->f1;_LL16C: _tmp208=_tmp206;goto _LL16E;
_LL16D: if(*((int*)_tmp202)!= 1)goto _LL16F;_tmp207=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp202)->f2;if(_tmp207 <= (void*)1?1:*((int*)_tmp207)!= 3)goto _LL16F;_tmp208=((
struct Cyc_Absyn_Local_b_struct*)_tmp207)->f1;_LL16E: _tmp20A=_tmp208;goto _LL170;
_LL16F: if(*((int*)_tmp202)!= 1)goto _LL171;_tmp209=(void*)((struct Cyc_Absyn_Var_e_struct*)
_tmp202)->f2;if(_tmp209 <= (void*)1?1:*((int*)_tmp209)!= 4)goto _LL171;_tmp20A=((
struct Cyc_Absyn_Pat_b_struct*)_tmp209)->f1;_LL170: if(!_tmp20A->escapes)return Cyc_CfFlowInfo_reln_assign_var(
r,_tmp20A,e2);goto _LL168;_LL171:;_LL172: goto _LL168;_LL168:;}return r;}static void
Cyc_CfFlowInfo_print_reln(struct Cyc_CfFlowInfo_Reln*r){({struct Cyc_String_pa_struct
_tmp20D;_tmp20D.tag=0;_tmp20D.f1=(struct _tagged_arr)((struct _tagged_arr)Cyc_Absynpp_qvar2string((
r->vd)->name));{void*_tmp20B[1]={& _tmp20D};Cyc_fprintf(Cyc_stderr,({const char*
_tmp20C="%s";_tag_arr(_tmp20C,sizeof(char),_get_zero_arr_size(_tmp20C,3));}),
_tag_arr(_tmp20B,sizeof(void*),1));}});{void*_tmp20E=(void*)r->rop;unsigned int
_tmp20F;struct Cyc_Absyn_Vardecl*_tmp210;struct Cyc_Absyn_Vardecl*_tmp211;
unsigned int _tmp212;struct Cyc_Absyn_Vardecl*_tmp213;_LL174: if(*((int*)_tmp20E)!= 
0)goto _LL176;_tmp20F=((struct Cyc_CfFlowInfo_EqualConst_struct*)_tmp20E)->f1;
_LL175:({struct Cyc_Int_pa_struct _tmp216;_tmp216.tag=1;_tmp216.f1=(unsigned int)((
int)_tmp20F);{void*_tmp214[1]={& _tmp216};Cyc_fprintf(Cyc_stderr,({const char*
_tmp215="==%d";_tag_arr(_tmp215,sizeof(char),_get_zero_arr_size(_tmp215,5));}),
_tag_arr(_tmp214,sizeof(void*),1));}});goto _LL173;_LL176: if(*((int*)_tmp20E)!= 1)
goto _LL178;_tmp210=((struct Cyc_CfFlowInfo_LessVar_struct*)_tmp20E)->f1;_LL177:({
struct Cyc_String_pa_struct _tmp219;_tmp219.tag=0;_tmp219.f1=(struct _tagged_arr)((
struct _tagged_arr)Cyc_Absynpp_qvar2string(_tmp210->name));{void*_tmp217[1]={&
_tmp219};Cyc_fprintf(Cyc_stderr,({const char*_tmp218="<%s";_tag_arr(_tmp218,
sizeof(char),_get_zero_arr_size(_tmp218,4));}),_tag_arr(_tmp217,sizeof(void*),1));}});
goto _LL173;_LL178: if(*((int*)_tmp20E)!= 2)goto _LL17A;_tmp211=((struct Cyc_CfFlowInfo_LessSize_struct*)
_tmp20E)->f1;_LL179:({struct Cyc_String_pa_struct _tmp21C;_tmp21C.tag=0;_tmp21C.f1=(
struct _tagged_arr)((struct _tagged_arr)Cyc_Absynpp_qvar2string(_tmp211->name));{
void*_tmp21A[1]={& _tmp21C};Cyc_fprintf(Cyc_stderr,({const char*_tmp21B="<%s.size";
_tag_arr(_tmp21B,sizeof(char),_get_zero_arr_size(_tmp21B,9));}),_tag_arr(_tmp21A,
sizeof(void*),1));}});goto _LL173;_LL17A: if(*((int*)_tmp20E)!= 3)goto _LL17C;
_tmp212=((struct Cyc_CfFlowInfo_LessConst_struct*)_tmp20E)->f1;_LL17B:({struct Cyc_Int_pa_struct
_tmp21F;_tmp21F.tag=1;_tmp21F.f1=(unsigned int)((int)_tmp212);{void*_tmp21D[1]={&
_tmp21F};Cyc_fprintf(Cyc_stderr,({const char*_tmp21E="<%d";_tag_arr(_tmp21E,
sizeof(char),_get_zero_arr_size(_tmp21E,4));}),_tag_arr(_tmp21D,sizeof(void*),1));}});
goto _LL173;_LL17C: if(*((int*)_tmp20E)!= 4)goto _LL173;_tmp213=((struct Cyc_CfFlowInfo_LessEqSize_struct*)
_tmp20E)->f1;_LL17D:({struct Cyc_String_pa_struct _tmp222;_tmp222.tag=0;_tmp222.f1=(
struct _tagged_arr)((struct _tagged_arr)Cyc_Absynpp_qvar2string(_tmp213->name));{
void*_tmp220[1]={& _tmp222};Cyc_fprintf(Cyc_stderr,({const char*_tmp221="<=%s.size";
_tag_arr(_tmp221,sizeof(char),_get_zero_arr_size(_tmp221,10));}),_tag_arr(
_tmp220,sizeof(void*),1));}});goto _LL173;_LL173:;}}void Cyc_CfFlowInfo_print_relns(
struct Cyc_List_List*r){for(0;r != 0;r=r->tl){Cyc_CfFlowInfo_print_reln((struct Cyc_CfFlowInfo_Reln*)
r->hd);if(r->tl != 0)({void*_tmp223[0]={};Cyc_fprintf(Cyc_stderr,({const char*
_tmp224=",";_tag_arr(_tmp224,sizeof(char),_get_zero_arr_size(_tmp224,2));}),
_tag_arr(_tmp223,sizeof(void*),0));});}}static int Cyc_CfFlowInfo_contains_region(
struct Cyc_Absyn_Tvar*rgn,void*t){void*_tmp225=Cyc_Tcutil_compress(t);struct Cyc_Absyn_Tvar*
_tmp226;struct Cyc_Absyn_TunionInfo _tmp227;struct Cyc_List_List*_tmp228;void*
_tmp229;struct Cyc_List_List*_tmp22A;struct Cyc_Absyn_AggrInfo _tmp22B;struct Cyc_List_List*
_tmp22C;struct Cyc_Absyn_TunionFieldInfo _tmp22D;struct Cyc_List_List*_tmp22E;
struct Cyc_List_List*_tmp22F;struct Cyc_Absyn_PtrInfo _tmp230;void*_tmp231;struct
Cyc_Absyn_PtrAtts _tmp232;void*_tmp233;struct Cyc_List_List*_tmp234;struct Cyc_List_List*
_tmp235;struct Cyc_Absyn_ArrayInfo _tmp236;void*_tmp237;void*_tmp238;void*_tmp239;
void*_tmp23A;_LL17F: if((int)_tmp225 != 0)goto _LL181;_LL180: goto _LL182;_LL181: if(
_tmp225 <= (void*)3?1:*((int*)_tmp225)!= 5)goto _LL183;_LL182: goto _LL184;_LL183:
if((int)_tmp225 != 1)goto _LL185;_LL184: goto _LL186;_LL185: if(_tmp225 <= (void*)3?1:*((
int*)_tmp225)!= 6)goto _LL187;_LL186: goto _LL188;_LL187: if(_tmp225 <= (void*)3?1:*((
int*)_tmp225)!= 12)goto _LL189;_LL188: goto _LL18A;_LL189: if(_tmp225 <= (void*)3?1:*((
int*)_tmp225)!= 13)goto _LL18B;_LL18A: goto _LL18C;_LL18B: if(_tmp225 <= (void*)3?1:*((
int*)_tmp225)!= 14)goto _LL18D;_LL18C: goto _LL18E;_LL18D: if(_tmp225 <= (void*)3?1:*((
int*)_tmp225)!= 17)goto _LL18F;_LL18E: goto _LL190;_LL18F: if(_tmp225 <= (void*)3?1:*((
int*)_tmp225)!= 18)goto _LL191;_LL190: goto _LL192;_LL191: if((int)_tmp225 != 2)goto
_LL193;_LL192: goto _LL194;_LL193: if(_tmp225 <= (void*)3?1:*((int*)_tmp225)!= 0)
goto _LL195;_LL194: return 0;_LL195: if(_tmp225 <= (void*)3?1:*((int*)_tmp225)!= 1)
goto _LL197;_tmp226=((struct Cyc_Absyn_VarType_struct*)_tmp225)->f1;_LL196: return
Cyc_Absyn_tvar_cmp(_tmp226,rgn)== 0;_LL197: if(_tmp225 <= (void*)3?1:*((int*)
_tmp225)!= 2)goto _LL199;_tmp227=((struct Cyc_Absyn_TunionType_struct*)_tmp225)->f1;
_tmp228=_tmp227.targs;_tmp229=(void*)_tmp227.rgn;_LL198: if(Cyc_CfFlowInfo_contains_region(
rgn,_tmp229))return 1;_tmp22A=_tmp228;goto _LL19A;_LL199: if(_tmp225 <= (void*)3?1:*((
int*)_tmp225)!= 16)goto _LL19B;_tmp22A=((struct Cyc_Absyn_TypedefType_struct*)
_tmp225)->f2;_LL19A: _tmp22C=_tmp22A;goto _LL19C;_LL19B: if(_tmp225 <= (void*)3?1:*((
int*)_tmp225)!= 10)goto _LL19D;_tmp22B=((struct Cyc_Absyn_AggrType_struct*)_tmp225)->f1;
_tmp22C=_tmp22B.targs;_LL19C: _tmp22E=_tmp22C;goto _LL19E;_LL19D: if(_tmp225 <= (
void*)3?1:*((int*)_tmp225)!= 3)goto _LL19F;_tmp22D=((struct Cyc_Absyn_TunionFieldType_struct*)
_tmp225)->f1;_tmp22E=_tmp22D.targs;_LL19E: _tmp22F=_tmp22E;goto _LL1A0;_LL19F: if(
_tmp225 <= (void*)3?1:*((int*)_tmp225)!= 20)goto _LL1A1;_tmp22F=((struct Cyc_Absyn_JoinEff_struct*)
_tmp225)->f1;_LL1A0: return((int(*)(int(*pred)(struct Cyc_Absyn_Tvar*,void*),
struct Cyc_Absyn_Tvar*env,struct Cyc_List_List*x))Cyc_List_exists_c)(Cyc_CfFlowInfo_contains_region,
rgn,_tmp22F);_LL1A1: if(_tmp225 <= (void*)3?1:*((int*)_tmp225)!= 4)goto _LL1A3;
_tmp230=((struct Cyc_Absyn_PointerType_struct*)_tmp225)->f1;_tmp231=(void*)
_tmp230.elt_typ;_tmp232=_tmp230.ptr_atts;_tmp233=(void*)_tmp232.rgn;_LL1A2:
return Cyc_CfFlowInfo_contains_region(rgn,_tmp233)?1: Cyc_CfFlowInfo_contains_region(
rgn,_tmp231);_LL1A3: if(_tmp225 <= (void*)3?1:*((int*)_tmp225)!= 8)goto _LL1A5;
_LL1A4: return 0;_LL1A5: if(_tmp225 <= (void*)3?1:*((int*)_tmp225)!= 9)goto _LL1A7;
_tmp234=((struct Cyc_Absyn_TupleType_struct*)_tmp225)->f1;_LL1A6: for(0;_tmp234 != 
0;_tmp234=_tmp234->tl){if(Cyc_CfFlowInfo_contains_region(rgn,(*((struct _tuple4*)
_tmp234->hd)).f2))return 1;}return 0;_LL1A7: if(_tmp225 <= (void*)3?1:*((int*)
_tmp225)!= 11)goto _LL1A9;_tmp235=((struct Cyc_Absyn_AnonAggrType_struct*)_tmp225)->f2;
_LL1A8: for(0;_tmp235 != 0;_tmp235=_tmp235->tl){if(Cyc_CfFlowInfo_contains_region(
rgn,(void*)((struct Cyc_Absyn_Aggrfield*)_tmp235->hd)->type))return 1;}return 0;
_LL1A9: if(_tmp225 <= (void*)3?1:*((int*)_tmp225)!= 7)goto _LL1AB;_tmp236=((struct
Cyc_Absyn_ArrayType_struct*)_tmp225)->f1;_tmp237=(void*)_tmp236.elt_type;_LL1AA:
_tmp238=_tmp237;goto _LL1AC;_LL1AB: if(_tmp225 <= (void*)3?1:*((int*)_tmp225)!= 19)
goto _LL1AD;_tmp238=(void*)((struct Cyc_Absyn_AccessEff_struct*)_tmp225)->f1;
_LL1AC: _tmp239=_tmp238;goto _LL1AE;_LL1AD: if(_tmp225 <= (void*)3?1:*((int*)_tmp225)
!= 21)goto _LL1AF;_tmp239=(void*)((struct Cyc_Absyn_RgnsEff_struct*)_tmp225)->f1;
_LL1AE: return Cyc_CfFlowInfo_contains_region(rgn,_tmp239);_LL1AF: if(_tmp225 <= (
void*)3?1:*((int*)_tmp225)!= 15)goto _LL17E;_tmp23A=(void*)((struct Cyc_Absyn_RgnHandleType_struct*)
_tmp225)->f1;_LL1B0: return 0;_LL17E:;}struct _tuple7{struct Cyc_Dict_Dict*f1;struct
Cyc_Absyn_Tvar*f2;};static void Cyc_CfFlowInfo_kill_root(struct _tuple7*env,void*
root,void*rval){struct _tuple7 _tmp23C;struct Cyc_Dict_Dict*_tmp23D;struct Cyc_Dict_Dict**
_tmp23E;struct Cyc_Absyn_Tvar*_tmp23F;struct _tuple7*_tmp23B=env;_tmp23C=*_tmp23B;
_tmp23D=_tmp23C.f1;_tmp23E=(struct Cyc_Dict_Dict**)&(*_tmp23B).f1;_tmp23F=_tmp23C.f2;{
void*_tmp240=root;struct Cyc_Absyn_Vardecl*_tmp241;void*_tmp242;_LL1B2: if(*((int*)
_tmp240)!= 0)goto _LL1B4;_tmp241=((struct Cyc_CfFlowInfo_VarRoot_struct*)_tmp240)->f1;
_LL1B3: if(Cyc_CfFlowInfo_contains_region(_tmp23F,(void*)_tmp241->type))rval=Cyc_CfFlowInfo_typ_to_absrval((
void*)_tmp241->type,Cyc_CfFlowInfo_unknown_none);*_tmp23E=Cyc_Dict_insert(*
_tmp23E,root,rval);goto _LL1B1;_LL1B4: if(*((int*)_tmp240)!= 1)goto _LL1B6;_tmp242=(
void*)((struct Cyc_CfFlowInfo_MallocPt_struct*)_tmp240)->f2;_LL1B5: if(!Cyc_CfFlowInfo_contains_region(
_tmp23F,_tmp242))*_tmp23E=Cyc_Dict_insert(*_tmp23E,root,rval);goto _LL1B1;_LL1B6:
if(*((int*)_tmp240)!= 2)goto _LL1B1;_LL1B7: goto _LL1B1;_LL1B1:;}}static struct Cyc_Dict_Dict*
Cyc_CfFlowInfo_kill_flowdict_region(struct Cyc_Dict_Dict*fd,void*rgn){struct Cyc_Absyn_Tvar*
rgn_tvar;{void*_tmp243=Cyc_Tcutil_compress(rgn);struct Cyc_Absyn_Tvar*_tmp244;
_LL1B9: if(_tmp243 <= (void*)3?1:*((int*)_tmp243)!= 1)goto _LL1BB;_tmp244=((struct
Cyc_Absyn_VarType_struct*)_tmp243)->f1;_LL1BA: rgn_tvar=_tmp244;goto _LL1B8;_LL1BB:;
_LL1BC:(int)_throw((void*)({struct Cyc_Core_Impossible_struct*_tmp245=_cycalloc(
sizeof(*_tmp245));_tmp245[0]=({struct Cyc_Core_Impossible_struct _tmp246;_tmp246.tag=
Cyc_Core_Impossible;_tmp246.f1=({const char*_tmp247="kill_flowdict_region";
_tag_arr(_tmp247,sizeof(char),_get_zero_arr_size(_tmp247,21));});_tmp246;});
_tmp245;}));goto _LL1B8;_LL1B8:;}{struct _tuple7 env=({struct _tuple7 _tmp248;_tmp248.f1=
Cyc_Dict_empty(Cyc_CfFlowInfo_root_cmp);_tmp248.f2=rgn_tvar;_tmp248;});((void(*)(
void(*f)(struct _tuple7*,void*,void*),struct _tuple7*env,struct Cyc_Dict_Dict*d))
Cyc_Dict_iter_c)(Cyc_CfFlowInfo_kill_root,& env,fd);return env.f1;}}static struct
Cyc_List_List*Cyc_CfFlowInfo_kill_relns_region(struct Cyc_List_List*relns,void*
rgn){return 0;}void*Cyc_CfFlowInfo_kill_flow_region(void*f,void*rgn){void*_tmp249=
f;struct Cyc_Dict_Dict*_tmp24A;struct Cyc_List_List*_tmp24B;_LL1BE: if((int)_tmp249
!= 0)goto _LL1C0;_LL1BF: return f;_LL1C0: if(_tmp249 <= (void*)1?1:*((int*)_tmp249)!= 
0)goto _LL1BD;_tmp24A=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp249)->f1;
_tmp24B=((struct Cyc_CfFlowInfo_ReachableFL_struct*)_tmp249)->f2;_LL1C1: {struct
Cyc_Dict_Dict*_tmp24C=Cyc_CfFlowInfo_kill_flowdict_region(_tmp24A,rgn);struct Cyc_List_List*
_tmp24D=Cyc_CfFlowInfo_kill_relns_region(_tmp24B,rgn);return(void*)({struct Cyc_CfFlowInfo_ReachableFL_struct*
_tmp24E=_cycalloc(sizeof(*_tmp24E));_tmp24E[0]=({struct Cyc_CfFlowInfo_ReachableFL_struct
_tmp24F;_tmp24F.tag=0;_tmp24F.f1=_tmp24C;_tmp24F.f2=_tmp24D;_tmp24F;});_tmp24E;});}
_LL1BD:;}
