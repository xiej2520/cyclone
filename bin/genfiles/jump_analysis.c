#include <setjmp.h>
/* This is a C header file to be used by the output of the Cyclone to
   C translator.  The corresponding definitions are in file lib/runtime_*.c */
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

/* Need one of these per thread (see runtime_stack.c). The runtime maintains 
   a stack that contains either _handler_cons structs or _RegionHandle structs.
   The tag is 0 for a handler_cons and 1 for a region handle.  */
struct _RuntimeStack {
  int tag; 
  struct _RuntimeStack *next;
  void (*cleanup)(struct _RuntimeStack *frame);
};

#ifndef offsetof
/* should be size_t, but int is fine. */
#define offsetof(t,n) ((int)(&(((t *)0)->n)))
#endif

/* Fat pointers */
struct _fat_ptr {
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};  

/* Regions */
struct _RegionPage
#ifdef CYC_REGION_PROFILE
{ unsigned total_bytes;
  unsigned free_bytes;
  struct _RegionPage *next;
  char data[1];
}
#endif
; // abstract -- defined in runtime_memory.c
struct _pool;
struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
  char               *offset;
  char               *last_plus_one;
  struct _DynRegionHandle *sub_regions;
  struct _pool *released_ptrs;
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
// A dynamic region is just a region handle.  The wrapper struct is for type
// abstraction.
struct Cyc_Core_DynamicRegion {
  struct _RegionHandle h;
};

struct _RegionHandle _new_region(const char*);
void* _region_malloc(struct _RegionHandle*, unsigned);
void* _region_calloc(struct _RegionHandle*, unsigned t, unsigned n);

/* Exceptions */
struct _handler_cons {
  struct _RuntimeStack s;
  jmp_buf handler;
};
void _push_handler(struct _handler_cons *);
void _push_region(struct _RegionHandle *);
void _npop_handler(int);
void _pop_handler();
void _pop_region();

#ifndef _throw
void* _throw_null_fn(const char*,unsigned);
void* _throw_arraybounds_fn(const char*,unsigned);
void* _throw_badalloc_fn(const char*,unsigned);
void* _throw_match_fn(const char*,unsigned);
void* _throw_fn(void*,const char*,unsigned);
void* _rethrow(void*);
#define _throw_null() (_throw_null_fn(__FILE__,__LINE__))
#define _throw_arraybounds() (_throw_arraybounds_fn(__FILE__,__LINE__))
#define _throw_badalloc() (_throw_badalloc_fn(__FILE__,__LINE__))
#define _throw_match() (_throw_match_fn(__FILE__,__LINE__))
#define _throw(e) (_throw_fn((e),__FILE__,__LINE__))
#endif

void* Cyc_Core_get_exn_thrown();
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
#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#define _check_null(ptr) \
  ({ typeof(ptr) _cks_null = (ptr); \
     if (!_cks_null) _throw_null(); \
     _cks_null; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_notnull(ptr,bound,elt_sz,index)\
   (((char*)ptr) + (elt_sz)*(index))
#ifdef NO_CYC_NULL_CHECKS
#define _check_known_subscript_null _check_known_subscript_notnull
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr);\
  int _index = (index);\
  if (!_cks_ptr) _throw_null(); \
  _cks_ptr + (elt_sz)*_index; })
#endif
#define _zero_arr_plus_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_char_fn _zero_arr_plus_fn
#define _zero_arr_plus_short_fn _zero_arr_plus_fn
#define _zero_arr_plus_int_fn _zero_arr_plus_fn
#define _zero_arr_plus_float_fn _zero_arr_plus_fn
#define _zero_arr_plus_double_fn _zero_arr_plus_fn
#define _zero_arr_plus_longdouble_fn _zero_arr_plus_fn
#define _zero_arr_plus_voidstar_fn _zero_arr_plus_fn
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (_cks_index >= (bound)) _throw_arraybounds(); \
  _cks_ptr + (elt_sz)*_cks_index; })
#define _check_known_subscript_notnull(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr); \
  unsigned _cks_index = (index); \
  if (_cks_index >= (bound)) _throw_arraybounds(); \
  _cks_ptr + (elt_sz)*_cks_index; })

/* _zero_arr_plus_*_fn(x,sz,i,filename,lineno) adds i to zero-terminated ptr
   x that has at least sz elements */
char* _zero_arr_plus_char_fn(char*,unsigned,int,const char*,unsigned);
short* _zero_arr_plus_short_fn(short*,unsigned,int,const char*,unsigned);
int* _zero_arr_plus_int_fn(int*,unsigned,int,const char*,unsigned);
float* _zero_arr_plus_float_fn(float*,unsigned,int,const char*,unsigned);
double* _zero_arr_plus_double_fn(double*,unsigned,int,const char*,unsigned);
long double* _zero_arr_plus_longdouble_fn(long double*,unsigned,int,const char*, unsigned);
void** _zero_arr_plus_voidstar_fn(void**,unsigned,int,const char*,unsigned);
#endif

/* _get_zero_arr_size_*(x,sz) returns the number of elements in a
   zero-terminated array that is NULL or has at least sz elements */
int _get_zero_arr_size_char(const char*,unsigned);
int _get_zero_arr_size_short(const short*,unsigned);
int _get_zero_arr_size_int(const int*,unsigned);
int _get_zero_arr_size_float(const float*,unsigned);
int _get_zero_arr_size_double(const double*,unsigned);
int _get_zero_arr_size_longdouble(const long double*,unsigned);
int _get_zero_arr_size_voidstar(const void**,unsigned);

/* _zero_arr_inplace_plus_*_fn(x,i,filename,lineno) sets
   zero-terminated pointer *x to *x + i */
char* _zero_arr_inplace_plus_char_fn(char**,int,const char*,unsigned);
short* _zero_arr_inplace_plus_short_fn(short**,int,const char*,unsigned);
int* _zero_arr_inplace_plus_int(int**,int,const char*,unsigned);
float* _zero_arr_inplace_plus_float_fn(float**,int,const char*,unsigned);
double* _zero_arr_inplace_plus_double_fn(double**,int,const char*,unsigned);
long double* _zero_arr_inplace_plus_longdouble_fn(long double**,int,const char*,unsigned);
void** _zero_arr_inplace_plus_voidstar_fn(void***,int,const char*,unsigned);
/* like the previous functions, but does post-addition (as in e++) */
char* _zero_arr_inplace_plus_post_char_fn(char**,int,const char*,unsigned);
short* _zero_arr_inplace_plus_post_short_fn(short**x,int,const char*,unsigned);
int* _zero_arr_inplace_plus_post_int_fn(int**,int,const char*,unsigned);
float* _zero_arr_inplace_plus_post_float_fn(float**,int,const char*,unsigned);
double* _zero_arr_inplace_plus_post_double_fn(double**,int,const char*,unsigned);
long double* _zero_arr_inplace_plus_post_longdouble_fn(long double**,int,const char *,unsigned);
void** _zero_arr_inplace_plus_post_voidstar_fn(void***,int,const char*,unsigned);
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
#define _zero_arr_inplace_plus_char(x,i) \
  _zero_arr_inplace_plus_char_fn((char **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_short(x,i) \
  _zero_arr_inplace_plus_short_fn((short **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_int(x,i) \
  _zero_arr_inplace_plus_int_fn((int **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_float(x,i) \
  _zero_arr_inplace_plus_float_fn((float **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_double(x,i) \
  _zero_arr_inplace_plus_double_fn((double **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_longdouble(x,i) \
  _zero_arr_inplace_plus_longdouble_fn((long double **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_voidstar(x,i) \
  _zero_arr_inplace_plus_voidstar_fn((void ***)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_char(x,i) \
  _zero_arr_inplace_plus_post_char_fn((char **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_short(x,i) \
  _zero_arr_inplace_plus_post_short_fn((short **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_int(x,i) \
  _zero_arr_inplace_plus_post_int_fn((int **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_float(x,i) \
  _zero_arr_inplace_plus_post_float_fn((float **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_double(x,i) \
  _zero_arr_inplace_plus_post_double_fn((double **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_longdouble(x,i) \
  _zero_arr_inplace_plus_post_longdouble_fn((long double **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_voidstar(x,i) \
  _zero_arr_inplace_plus_post_voidstar_fn((void***)(x),(i),__FILE__,__LINE__)

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_fat_subscript(arr,elt_sz,index) ((arr).curr + (elt_sz) * (index))
#define _untag_fat_ptr(arr,elt_sz,num_elts) ((arr).curr)
#else
#define _check_fat_subscript(arr,elt_sz,index) ({ \
  struct _fat_ptr _cus_arr = (arr); \
  unsigned char *_cus_ans = _cus_arr.curr + (elt_sz) * (index); \
  /* JGM: not needed! if (!_cus_arr.base) _throw_null();*/ \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#define _untag_fat_ptr(arr,elt_sz,num_elts) ({ \
  struct _fat_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if ((_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one) &&\
      _curr != (unsigned char *)0) \
    _throw_arraybounds(); \
  _curr; })
#endif

#define _tag_fat(tcurr,elt_sz,num_elts) ({ \
  struct _fat_ptr _ans; \
  unsigned _num_elts = (num_elts);\
  _ans.base = _ans.curr = (void*)(tcurr); \
  /* JGM: if we're tagging NULL, ignore num_elts */ \
  _ans.last_plus_one = _ans.base ? (_ans.base + (elt_sz) * _num_elts) : 0; \
  _ans; })

#define _get_fat_size(arr,elt_sz) \
  ({struct _fat_ptr _arr = (arr); \
    unsigned char *_arr_curr=_arr.curr; \
    unsigned char *_arr_last=_arr.last_plus_one; \
    (_arr_curr < _arr.base || _arr_curr >= _arr_last) ? 0 : \
    ((_arr_last - _arr_curr) / (elt_sz));})

#define _fat_ptr_plus(arr,elt_sz,change) ({ \
  struct _fat_ptr _ans = (arr); \
  int _change = (change);\
  _ans.curr += (elt_sz) * _change;\
  _ans; })
#define _fat_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _fat_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += (elt_sz) * (change);\
  *_arr_ptr; })
#define _fat_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _fat_ptr * _arr_ptr = (arr_ptr); \
  struct _fat_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += (elt_sz) * (change);\
  _ans; })

//Not a macro since initialization order matters. Defined in runtime_zeroterm.c.
struct _fat_ptr _fat_ptr_decrease_size(struct _fat_ptr,unsigned sz,unsigned numelts);

/* Allocation */
void* GC_malloc(int);
void* GC_malloc_atomic(int);
void* GC_calloc(unsigned,unsigned);
void* GC_calloc_atomic(unsigned,unsigned);
// bound the allocation size to be < MAX_ALLOC_SIZE. See macros below for usage.
#define MAX_MALLOC_SIZE (1 << 28)
void* _bounded_GC_malloc(int,const char*,int);
void* _bounded_GC_malloc_atomic(int,const char*,int);
void* _bounded_GC_calloc(unsigned,unsigned,const char*,int);
void* _bounded_GC_calloc_atomic(unsigned,unsigned,const char*,int);
/* these macros are overridden below ifdef CYC_REGION_PROFILE */
#ifndef CYC_REGION_PROFILE
#define _cycalloc(n) _bounded_GC_malloc(n,__FILE__,__LINE__)
#define _cycalloc_atomic(n) _bounded_GC_malloc_atomic(n,__FILE__,__LINE__)
#define _cyccalloc(n,s) _bounded_GC_calloc(n,s,__FILE__,__LINE__)
#define _cyccalloc_atomic(n,s) _bounded_GC_calloc_atomic(n,s,__FILE__,__LINE__)
#endif

static inline unsigned int _check_times(unsigned x, unsigned y) {
  unsigned long long whole_ans = 
    ((unsigned long long) x)*((unsigned long long)y);
  unsigned word_ans = (unsigned)whole_ans;
  if(word_ans < whole_ans || word_ans > MAX_MALLOC_SIZE)
    _throw_badalloc();
  return word_ans;
}

#define _CYC_MAX_REGION_CONST 2
#define _CYC_MIN_ALIGNMENT (sizeof(double))

#ifdef CYC_REGION_PROFILE
extern int rgn_total_bytes;
#endif

static inline void *_fast_region_malloc(struct _RegionHandle *r, unsigned orig_s) {  
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

#ifdef CYC_REGION_PROFILE
/* see macros below for usage. defined in runtime_memory.c */
void* _profile_GC_malloc(int,const char*,const char*,int);
void* _profile_GC_malloc_atomic(int,const char*,const char*,int);
void* _profile_GC_calloc(unsigned,unsigned,const char*,const char*,int);
void* _profile_GC_calloc_atomic(unsigned,unsigned,const char*,const char*,int);
void* _profile_region_malloc(struct _RegionHandle*,unsigned,const char*,const char*,int);
void* _profile_region_calloc(struct _RegionHandle*,unsigned,unsigned,const char *,const char*,int);
struct _RegionHandle _profile_new_region(const char*,const char*,const char*,int);
void _profile_free_region(struct _RegionHandle*,const char*,const char*,int);
#ifndef RUNTIME_CYC
#define _new_region(n) _profile_new_region(n,__FILE__,__FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__,__FUNCTION__,__LINE__)
#define _region_malloc(rh,n) _profile_region_malloc(rh,n,__FILE__,__FUNCTION__,__LINE__)
#define _region_calloc(rh,n,t) _profile_region_calloc(rh,n,t,__FILE__,__FUNCTION__,__LINE__)
#  endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__,__FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__,__FUNCTION__,__LINE__)
#define _cyccalloc(n,s) _profile_GC_calloc(n,s,__FILE__,__FUNCTION__,__LINE__)
#define _cyccalloc_atomic(n,s) _profile_GC_calloc_atomic(n,s,__FILE__,__FUNCTION__,__LINE__)
#endif
#endif
 struct Cyc_Core_Opt{void*v;};
# 123 "core.h"
int Cyc_Core_ptrcmp(void*,void*);extern char Cyc_Core_Invalid_argument[17U];extern char Cyc_Core_Failure[8U];extern char Cyc_Core_Impossible[11U];extern char Cyc_Core_Not_found[10U];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12U];
# 171
extern struct _RegionHandle*Cyc_Core_unique_region;struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};extern char Cyc_List_List_mismatch[14U];extern char Cyc_List_Nth[4U];extern char Cyc_FileCloseError[15U];extern char Cyc_FileOpenError[14U];
# 50 "string.h"
extern int Cyc_strptrcmp(struct _fat_ptr*,struct _fat_ptr*);struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};struct _tuple0{union Cyc_Absyn_Nmspace f1;struct _fat_ptr*f2;};
# 149 "absyn.h"
enum Cyc_Absyn_Scope{Cyc_Absyn_Static =0U,Cyc_Absyn_Abstract =1U,Cyc_Absyn_Public =2U,Cyc_Absyn_Extern =3U,Cyc_Absyn_ExternC =4U,Cyc_Absyn_Register =5U};struct Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;unsigned loc;};struct Cyc_Absyn_Tvar{struct _fat_ptr*name;int identity;void*kind;};struct Cyc_Absyn_VarargInfo{struct _fat_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;struct Cyc_Absyn_Vardecl*return_value;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned loc;void*annot;};struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple9{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct{int tag;struct _tuple9 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct{int tag;struct _fat_ptr*f1;};struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple9 f2;struct _tuple9 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct{int tag;struct _fat_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple9 f2;};struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Stmt{void*r;unsigned loc;void*annot;};struct Cyc_Absyn_Pat{void*r;void*topt;unsigned loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;unsigned loc;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;unsigned varloc;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;int is_proto;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple0*name;struct Cyc_Absyn_Stmt*body;struct Cyc_Absyn_FnInfo i;void*cached_type;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;enum Cyc_Absyn_Scope orig_scope;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;void*f4;};struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct{int tag;struct _fat_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Decl{void*r;unsigned loc;};extern char Cyc_Absyn_EmptyAnnot[11U];
# 1132 "absyn.h"
void Cyc_Absyn_do_nested_statement(struct Cyc_Absyn_Exp*,void*,void(*)(void*,struct Cyc_Absyn_Stmt*),int);struct Cyc_Warn_String_Warn_Warg_struct{int tag;struct _fat_ptr f1;};
# 67 "warn.h"
void Cyc_Warn_err2(unsigned,struct _fat_ptr);
# 30 "flags.h"
extern int Cyc_Flags_no_regions;extern char Cyc_Dict_Present[8U];extern char Cyc_Dict_Absent[7U];extern char Cyc_Tcenv_Env_error[10U];
# 112 "tcpat.h"
int Cyc_Tcpat_has_vars(struct Cyc_Core_Opt*);struct Cyc_Hashtable_Table;
# 39 "hashtable.h"
extern struct Cyc_Hashtable_Table*Cyc_Hashtable_create(int,int(*)(void*,void*),int(*)(void*));
# 50
extern void Cyc_Hashtable_insert(struct Cyc_Hashtable_Table*,void*,void*);
# 52
extern void*Cyc_Hashtable_lookup(struct Cyc_Hashtable_Table*,void*);
# 86
extern int Cyc_Hashtable_hash_stringptr(struct _fat_ptr*);struct Cyc_JumpAnalysis_Jump_Anal_Result{struct Cyc_Hashtable_Table*pop_tables;struct Cyc_Hashtable_Table*succ_tables;struct Cyc_Hashtable_Table*pat_pop_tables;};struct Cyc_JumpAnalysis_NotLoop_j_JumpAnalysis_Jumpee_struct{int tag;};struct Cyc_JumpAnalysis_CaseEnd_j_JumpAnalysis_Jumpee_struct{int tag;};struct Cyc_JumpAnalysis_FnEnd_j_JumpAnalysis_Jumpee_struct{int tag;};struct Cyc_JumpAnalysis_Stmt_j_JumpAnalysis_Jumpee_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_JumpAnalysis_Fallthru_j_JumpAnalysis_Jumpee_struct{int tag;struct Cyc_Absyn_Switch_clause*f1;};struct Cyc_JumpAnalysis_Return_j_JumpAnalysis_Jumpee_struct{int tag;};struct Cyc_JumpAnalysis_Lab_j_JumpAnalysis_Jumpee_struct{int tag;struct _fat_ptr*f1;};
# 54 "jump_analysis.cyc"
struct Cyc_JumpAnalysis_NotLoop_j_JumpAnalysis_Jumpee_struct Cyc_JumpAnalysis_NotLoop_j_val={0};
struct Cyc_JumpAnalysis_CaseEnd_j_JumpAnalysis_Jumpee_struct Cyc_JumpAnalysis_CaseEnd_j_val={1};
struct Cyc_JumpAnalysis_FnEnd_j_JumpAnalysis_Jumpee_struct Cyc_JumpAnalysis_FnEnd_j_val={2};
struct Cyc_JumpAnalysis_Return_j_JumpAnalysis_Jumpee_struct Cyc_JumpAnalysis_Return_j_val={5};struct Cyc_JumpAnalysis_SaveEnv{int try_depth;int pat_var_depth;struct Cyc_List_List*enclosers;};struct Cyc_JumpAnalysis_FnEnv{struct Cyc_JumpAnalysis_Jump_Anal_Result*tables;struct Cyc_Hashtable_Table*stmt_info;struct Cyc_List_List*jumpers;struct Cyc_Hashtable_Table*labels;};struct Cyc_JumpAnalysis_StmtEnv{struct Cyc_JumpAnalysis_FnEnv*fenv;void*continue_stmt;void*break_stmt;void*next_stmt;struct Cyc_JumpAnalysis_SaveEnv save_env;};
# 81
static void Cyc_JumpAnalysis_anal_exp(struct Cyc_JumpAnalysis_StmtEnv,struct Cyc_Absyn_Exp*);
static void Cyc_JumpAnalysis_anal_stmt(struct Cyc_JumpAnalysis_StmtEnv,struct Cyc_Absyn_Stmt*);
static void Cyc_JumpAnalysis_anal_fd(struct Cyc_JumpAnalysis_Jump_Anal_Result*,struct Cyc_Absyn_Fndecl*);
# 85
static int Cyc_JumpAnalysis_hash_ptr(void*s){
return(int)s;}
# 88
static struct Cyc_Hashtable_Table*Cyc_JumpAnalysis_make_ptr_table (void){
return({(struct Cyc_Hashtable_Table*(*)(int,int(*)(void*,void*),int(*)(void*)))Cyc_Hashtable_create;})(33,Cyc_Core_ptrcmp,({(int(*)(void*))Cyc_JumpAnalysis_hash_ptr;}));}
# 91
static struct Cyc_Hashtable_Table*Cyc_JumpAnalysis_make_var_table (void){
return({(struct Cyc_Hashtable_Table*(*)(int,int(*)(struct _fat_ptr*,struct _fat_ptr*),int(*)(struct _fat_ptr*)))Cyc_Hashtable_create;})(33,Cyc_strptrcmp,Cyc_Hashtable_hash_stringptr);}
# 94
static struct Cyc_JumpAnalysis_StmtEnv Cyc_JumpAnalysis_add_encloser(struct Cyc_Absyn_Stmt*s,struct Cyc_JumpAnalysis_StmtEnv env){
struct Cyc_JumpAnalysis_StmtEnv _tmp0=env;struct Cyc_JumpAnalysis_StmtEnv env1=_tmp0;
({struct Cyc_List_List*_tmp6F=({struct Cyc_List_List*_tmp1=_cycalloc(sizeof(*_tmp1));_tmp1->hd=s,_tmp1->tl=(env1.save_env).enclosers;_tmp1;});(env1.save_env).enclosers=_tmp6F;});
return env1;}
# 99
static void Cyc_JumpAnalysis_anal_exps(struct Cyc_JumpAnalysis_StmtEnv env,struct Cyc_List_List*es){
for(1;es != 0;es=es->tl){
Cyc_JumpAnalysis_anal_exp(env,(struct Cyc_Absyn_Exp*)es->hd);}}
# 103
static void Cyc_JumpAnalysis_anal_stmt_exp(struct Cyc_JumpAnalysis_StmtEnv*env,struct Cyc_Absyn_Stmt*s){
({struct Cyc_JumpAnalysis_StmtEnv _tmp70=Cyc_JumpAnalysis_add_encloser(s,*env);Cyc_JumpAnalysis_anal_stmt(_tmp70,s);});}
# 106
static void Cyc_JumpAnalysis_anal_exp(struct Cyc_JumpAnalysis_StmtEnv env,struct Cyc_Absyn_Exp*e){
({(void(*)(struct Cyc_Absyn_Exp*,struct Cyc_JumpAnalysis_StmtEnv*,void(*)(struct Cyc_JumpAnalysis_StmtEnv*,struct Cyc_Absyn_Stmt*),int))Cyc_Absyn_do_nested_statement;})(e,& env,Cyc_JumpAnalysis_anal_stmt_exp,0);}
# 109
static void Cyc_JumpAnalysis_anal_scs(struct Cyc_JumpAnalysis_StmtEnv env,struct Cyc_List_List*scs){
# 112
env.break_stmt=env.next_stmt;
env.next_stmt=(void*)& Cyc_JumpAnalysis_CaseEnd_j_val;
for(1;scs != 0;scs=scs->tl){
struct Cyc_Absyn_Switch_clause*_tmp2=(struct Cyc_Absyn_Switch_clause*)scs->hd;struct Cyc_Absyn_Switch_clause*sc=_tmp2;
struct Cyc_Absyn_Exp*_tmp3=sc->where_clause;struct Cyc_Absyn_Exp*wc=_tmp3;
if(wc != 0)
Cyc_JumpAnalysis_anal_exp(env,wc);{
struct Cyc_JumpAnalysis_StmtEnv _tmp4=env;struct Cyc_JumpAnalysis_StmtEnv env1=_tmp4;
if(Cyc_Tcpat_has_vars(sc->pat_vars)){
env1=Cyc_JumpAnalysis_add_encloser(sc->body,env1);
++(env1.save_env).pat_var_depth;}
# 124
Cyc_JumpAnalysis_anal_stmt(env1,sc->body);}}}struct _tuple12{struct Cyc_Absyn_Stmt*f1;void*f2;};
# 127
static void Cyc_JumpAnalysis_anal_stmt(struct Cyc_JumpAnalysis_StmtEnv env,struct Cyc_Absyn_Stmt*s){
({void(*_tmp73)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*)=({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*))Cyc_Hashtable_insert;});struct Cyc_Hashtable_Table*_tmp72=(env.fenv)->stmt_info;struct Cyc_Absyn_Stmt*_tmp71=s;_tmp73(_tmp72,_tmp71,({struct Cyc_JumpAnalysis_SaveEnv*_tmp5=_cycalloc(sizeof(*_tmp5));*_tmp5=env.save_env;_tmp5;}));});{
void*_tmp6=s->r;void*_stmttmp0=_tmp6;void*_tmp7=_stmttmp0;void*_tmpD;void*_tmpC;void*_tmpB;void*_tmpA;void*_tmp9;void*_tmp8;switch(*((int*)_tmp7)){case 0:
 goto _LL0;case 1: _tmp8=((struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;{struct Cyc_Absyn_Exp*e=_tmp8;
Cyc_JumpAnalysis_anal_exp(env,e);goto _LL0;}case 2: _tmp8=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;_tmp9=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp7)->f2;{struct Cyc_Absyn_Stmt*s1=_tmp8;struct Cyc_Absyn_Stmt*s2=_tmp9;
# 133
struct Cyc_JumpAnalysis_StmtEnv _tmpE=env;struct Cyc_JumpAnalysis_StmtEnv env1=_tmpE;
({void*_tmp74=(void*)({struct Cyc_JumpAnalysis_Stmt_j_JumpAnalysis_Jumpee_struct*_tmpF=_cycalloc(sizeof(*_tmpF));_tmpF->tag=3,_tmpF->f1=s2;_tmpF;});env1.next_stmt=_tmp74;});
Cyc_JumpAnalysis_anal_stmt(env1,s1);
Cyc_JumpAnalysis_anal_stmt(env,s2);
goto _LL0;}case 4: _tmp8=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;_tmp9=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp7)->f2;_tmpA=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp7)->f3;{struct Cyc_Absyn_Exp*e=_tmp8;struct Cyc_Absyn_Stmt*s1=_tmp9;struct Cyc_Absyn_Stmt*s2=_tmpA;
# 139
Cyc_JumpAnalysis_anal_exp(env,e);
Cyc_JumpAnalysis_anal_stmt(env,s1);
Cyc_JumpAnalysis_anal_stmt(env,s2);
goto _LL0;}case 3: _tmp8=((struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;{struct Cyc_Absyn_Exp*eopt=_tmp8;
# 145
if(eopt != 0)Cyc_JumpAnalysis_anal_exp(env,eopt);
({struct Cyc_List_List*_tmp76=({struct Cyc_List_List*_tmp11=_cycalloc(sizeof(*_tmp11));({struct _tuple12*_tmp75=({struct _tuple12*_tmp10=_cycalloc(sizeof(*_tmp10));_tmp10->f1=s,_tmp10->f2=(void*)& Cyc_JumpAnalysis_Return_j_val;_tmp10;});_tmp11->hd=_tmp75;}),_tmp11->tl=(env.fenv)->jumpers;_tmp11;});(env.fenv)->jumpers=_tmp76;});
goto _LL0;}case 5: _tmp8=(((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp7)->f1).f1;_tmp9=(((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp7)->f1).f2;_tmpA=((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp7)->f2;{struct Cyc_Absyn_Exp*e=_tmp8;struct Cyc_Absyn_Stmt*cont_s=_tmp9;struct Cyc_Absyn_Stmt*s=_tmpA;
# 150
({void(*_tmp79)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*)=({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*))Cyc_Hashtable_insert;});struct Cyc_Hashtable_Table*_tmp78=(env.fenv)->stmt_info;struct Cyc_Absyn_Stmt*_tmp77=cont_s;_tmp79(_tmp78,_tmp77,({struct Cyc_JumpAnalysis_SaveEnv*_tmp12=_cycalloc(sizeof(*_tmp12));*_tmp12=env.save_env;_tmp12;}));});
Cyc_JumpAnalysis_anal_exp(env,e);{
struct Cyc_JumpAnalysis_StmtEnv _tmp13=env;struct Cyc_JumpAnalysis_StmtEnv env_body=_tmp13;
({void*_tmp7A=(void*)({struct Cyc_JumpAnalysis_Stmt_j_JumpAnalysis_Jumpee_struct*_tmp14=_cycalloc(sizeof(*_tmp14));_tmp14->tag=3,_tmp14->f1=cont_s;_tmp14;});env_body.continue_stmt=_tmp7A;});
env_body.next_stmt=env_body.continue_stmt;
env_body.break_stmt=env.next_stmt;
Cyc_JumpAnalysis_anal_stmt(env_body,s);
goto _LL0;}}case 9: _tmp8=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;_tmp9=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp7)->f2).f1;_tmpA=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp7)->f2).f2;_tmpB=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp7)->f3).f1;_tmpC=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp7)->f3).f2;_tmpD=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp7)->f4;{struct Cyc_Absyn_Exp*e1=_tmp8;struct Cyc_Absyn_Exp*e2=_tmp9;struct Cyc_Absyn_Stmt*guard_s=_tmpA;struct Cyc_Absyn_Exp*e3=_tmpB;struct Cyc_Absyn_Stmt*cont_s=_tmpC;struct Cyc_Absyn_Stmt*s=_tmpD;
# 159
({void(*_tmp7D)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*)=({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*))Cyc_Hashtable_insert;});struct Cyc_Hashtable_Table*_tmp7C=(env.fenv)->stmt_info;struct Cyc_Absyn_Stmt*_tmp7B=guard_s;_tmp7D(_tmp7C,_tmp7B,({struct Cyc_JumpAnalysis_SaveEnv*_tmp15=_cycalloc(sizeof(*_tmp15));*_tmp15=env.save_env;_tmp15;}));});
({void(*_tmp80)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*)=({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*))Cyc_Hashtable_insert;});struct Cyc_Hashtable_Table*_tmp7F=(env.fenv)->stmt_info;struct Cyc_Absyn_Stmt*_tmp7E=cont_s;_tmp80(_tmp7F,_tmp7E,({struct Cyc_JumpAnalysis_SaveEnv*_tmp16=_cycalloc(sizeof(*_tmp16));*_tmp16=env.save_env;_tmp16;}));});
Cyc_JumpAnalysis_anal_exp(env,e1);
Cyc_JumpAnalysis_anal_exp(env,e2);{
# 164
struct Cyc_JumpAnalysis_StmtEnv _tmp17=env;struct Cyc_JumpAnalysis_StmtEnv env_body=_tmp17;
({void*_tmp81=(void*)({struct Cyc_JumpAnalysis_Stmt_j_JumpAnalysis_Jumpee_struct*_tmp18=_cycalloc(sizeof(*_tmp18));_tmp18->tag=3,_tmp18->f1=cont_s;_tmp18;});env_body.continue_stmt=_tmp81;});
env_body.next_stmt=env_body.continue_stmt;
env_body.break_stmt=env.next_stmt;
Cyc_JumpAnalysis_anal_stmt(env_body,s);
Cyc_JumpAnalysis_anal_exp(env_body,e3);
goto _LL0;}}case 14: _tmp8=((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;_tmp9=(((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp7)->f2).f1;_tmpA=(((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp7)->f2).f2;{struct Cyc_Absyn_Stmt*s=_tmp8;struct Cyc_Absyn_Exp*e=_tmp9;struct Cyc_Absyn_Stmt*cont_s=_tmpA;
# 172
({void(*_tmp84)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*)=({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_JumpAnalysis_SaveEnv*))Cyc_Hashtable_insert;});struct Cyc_Hashtable_Table*_tmp83=(env.fenv)->stmt_info;struct Cyc_Absyn_Stmt*_tmp82=cont_s;_tmp84(_tmp83,_tmp82,({struct Cyc_JumpAnalysis_SaveEnv*_tmp19=_cycalloc(sizeof(*_tmp19));*_tmp19=env.save_env;_tmp19;}));});{
struct Cyc_JumpAnalysis_StmtEnv _tmp1A=env;struct Cyc_JumpAnalysis_StmtEnv env_body=_tmp1A;
({void*_tmp85=(void*)({struct Cyc_JumpAnalysis_Stmt_j_JumpAnalysis_Jumpee_struct*_tmp1B=_cycalloc(sizeof(*_tmp1B));_tmp1B->tag=3,_tmp1B->f1=cont_s;_tmp1B;});env_body.continue_stmt=_tmp85;});
env_body.next_stmt=env_body.continue_stmt;
env_body.break_stmt=env.next_stmt;
Cyc_JumpAnalysis_anal_stmt(env_body,s);
Cyc_JumpAnalysis_anal_exp(env,e);
goto _LL0;}}case 6:
# 182
({struct Cyc_List_List*_tmp87=({struct Cyc_List_List*_tmp1D=_cycalloc(sizeof(*_tmp1D));({struct _tuple12*_tmp86=({struct _tuple12*_tmp1C=_cycalloc(sizeof(*_tmp1C));_tmp1C->f1=s,_tmp1C->f2=env.break_stmt;_tmp1C;});_tmp1D->hd=_tmp86;}),_tmp1D->tl=(env.fenv)->jumpers;_tmp1D;});(env.fenv)->jumpers=_tmp87;});
goto _LL0;case 7:
# 185
({struct Cyc_List_List*_tmp89=({struct Cyc_List_List*_tmp1F=_cycalloc(sizeof(*_tmp1F));({struct _tuple12*_tmp88=({struct _tuple12*_tmp1E=_cycalloc(sizeof(*_tmp1E));_tmp1E->f1=s,_tmp1E->f2=env.continue_stmt;_tmp1E;});_tmp1F->hd=_tmp88;}),_tmp1F->tl=(env.fenv)->jumpers;_tmp1F;});(env.fenv)->jumpers=_tmp89;});
goto _LL0;case 8: _tmp8=((struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;{struct _fat_ptr*lab=_tmp8;
# 188
({struct Cyc_List_List*_tmp8C=({struct Cyc_List_List*_tmp22=_cycalloc(sizeof(*_tmp22));({struct _tuple12*_tmp8B=({struct _tuple12*_tmp21=_cycalloc(sizeof(*_tmp21));_tmp21->f1=s,({void*_tmp8A=(void*)({struct Cyc_JumpAnalysis_Lab_j_JumpAnalysis_Jumpee_struct*_tmp20=_cycalloc(sizeof(*_tmp20));_tmp20->tag=6,_tmp20->f1=lab;_tmp20;});_tmp21->f2=_tmp8A;});_tmp21;});_tmp22->hd=_tmp8B;}),_tmp22->tl=(env.fenv)->jumpers;_tmp22;});(env.fenv)->jumpers=_tmp8C;});
goto _LL0;}case 13: _tmp8=((struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;_tmp9=((struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct*)_tmp7)->f2;{struct _fat_ptr*lab=_tmp8;struct Cyc_Absyn_Stmt*s1=_tmp9;
# 192
{struct _handler_cons _tmp23;_push_handler(& _tmp23);{int _tmp25=0;if(setjmp(_tmp23.handler))_tmp25=1;if(!_tmp25){
({(struct Cyc_Absyn_Stmt*(*)(struct Cyc_Hashtable_Table*,struct _fat_ptr*))Cyc_Hashtable_lookup;})((env.fenv)->labels,lab);
({struct Cyc_Warn_String_Warn_Warg_struct _tmp27=({struct Cyc_Warn_String_Warn_Warg_struct _tmp67;_tmp67.tag=0,({struct _fat_ptr _tmp8D=({const char*_tmp28="repated label";_tag_fat(_tmp28,sizeof(char),14U);});_tmp67.f1=_tmp8D;});_tmp67;});void*_tmp26[1];_tmp26[0]=& _tmp27;Cyc_Warn_err2(s->loc,_tag_fat(_tmp26,sizeof(void*),1));});
# 193
;_pop_handler();}else{void*_tmp24=(void*)Cyc_Core_get_exn_thrown();void*_tmp29=_tmp24;void*_tmp2A;if(((struct Cyc_Core_Not_found_exn_struct*)_tmp29)->tag == Cyc_Core_Not_found)
# 195
goto _LL21;else{_tmp2A=_tmp29;{void*exn=_tmp2A;(int)_rethrow(exn);}}_LL21:;}}}
({(void(*)(struct Cyc_Hashtable_Table*,struct _fat_ptr*,struct Cyc_Absyn_Stmt*))Cyc_Hashtable_insert;})((env.fenv)->labels,lab,s);
Cyc_JumpAnalysis_anal_stmt(env,s1);
goto _LL0;}case 11: _tmp8=((struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;_tmp9=((struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct*)_tmp7)->f2;{struct Cyc_List_List*es=_tmp8;struct Cyc_Absyn_Switch_clause**clauseopt=_tmp9;
# 202
Cyc_JumpAnalysis_anal_exps(env,es);
({struct Cyc_List_List*_tmp90=({struct Cyc_List_List*_tmp2D=_cycalloc(sizeof(*_tmp2D));({struct _tuple12*_tmp8F=({struct _tuple12*_tmp2C=_cycalloc(sizeof(*_tmp2C));_tmp2C->f1=s,({void*_tmp8E=(void*)({struct Cyc_JumpAnalysis_Fallthru_j_JumpAnalysis_Jumpee_struct*_tmp2B=_cycalloc(sizeof(*_tmp2B));_tmp2B->tag=4,_tmp2B->f1=*_check_null(clauseopt);_tmp2B;});_tmp2C->f2=_tmp8E;});_tmp2C;});_tmp2D->hd=_tmp8F;}),_tmp2D->tl=(env.fenv)->jumpers;_tmp2D;});(env.fenv)->jumpers=_tmp90;});
# 205
goto _LL0;}case 10: _tmp8=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;_tmp9=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp7)->f2;{struct Cyc_Absyn_Exp*e=_tmp8;struct Cyc_List_List*scs=_tmp9;
# 207
Cyc_JumpAnalysis_anal_exp(env,e);
Cyc_JumpAnalysis_anal_scs(env,scs);
goto _LL0;}case 15: _tmp8=((struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;_tmp9=((struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct*)_tmp7)->f2;{struct Cyc_Absyn_Stmt*s=_tmp8;struct Cyc_List_List*scs=_tmp9;
# 211
struct Cyc_JumpAnalysis_StmtEnv _tmp2E=Cyc_JumpAnalysis_add_encloser(s,env);struct Cyc_JumpAnalysis_StmtEnv env1=_tmp2E;
++(env1.save_env).try_depth;
Cyc_JumpAnalysis_anal_stmt(env1,s);
Cyc_JumpAnalysis_anal_scs(env,scs);
goto _LL0;}default: _tmp8=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp7)->f1;_tmp9=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp7)->f2;{struct Cyc_Absyn_Decl*d=_tmp8;struct Cyc_Absyn_Stmt*s1=_tmp9;
# 220
env=Cyc_JumpAnalysis_add_encloser(s,env);
{void*_tmp2F=d->r;void*_stmttmp1=_tmp2F;void*_tmp30=_stmttmp1;void*_tmp31;switch(*((int*)_tmp30)){case 4: _tmp31=((struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct*)_tmp30)->f3;{struct Cyc_Absyn_Exp*eo=_tmp31;
# 226
if(eo != 0){
Cyc_JumpAnalysis_anal_exp(env,eo);
++(env.save_env).pat_var_depth;}else{
# 231
if(!Cyc_Flags_no_regions)
++(env.save_env).try_depth;}
goto _LL26;}case 0: _tmp31=(((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp30)->f1)->initializer;{struct Cyc_Absyn_Exp*eo=_tmp31;
# 235
if(eo != 0)Cyc_JumpAnalysis_anal_exp(env,eo);
goto _LL26;}case 1: _tmp31=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp30)->f1;{struct Cyc_Absyn_Fndecl*fd=_tmp31;
# 238
Cyc_JumpAnalysis_anal_fd((env.fenv)->tables,fd);
goto _LL26;}case 2: _tmp31=((struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct*)_tmp30)->f3;{struct Cyc_Absyn_Exp*e=_tmp31;
# 241
Cyc_JumpAnalysis_anal_exp(env,e);
++(env.save_env).pat_var_depth;
goto _LL26;}case 3:
# 245
 goto _LL32;case 8: _LL32:
 goto _LL34;case 5: _LL34:
 goto _LL36;case 6: _LL36:
 goto _LL38;case 7: _LL38:
 goto _LL3A;case 13: _LL3A:
 goto _LL3C;case 14: _LL3C:
 goto _LL3E;case 15: _LL3E:
 goto _LL40;case 16: _LL40:
 goto _LL42;case 11: _LL42:
 goto _LL44;case 12: _LL44:
 goto _LL46;case 9: _LL46:
 goto _LL48;default: _LL48:
 goto _LL26;}_LL26:;}
# 259
Cyc_JumpAnalysis_anal_stmt(env,s1);
goto _LL0;}}_LL0:;}}
# 265
static void Cyc_JumpAnalysis_anal_fd(struct Cyc_JumpAnalysis_Jump_Anal_Result*tables,struct Cyc_Absyn_Fndecl*fd){
struct Cyc_JumpAnalysis_FnEnv*_tmp32=({struct Cyc_JumpAnalysis_FnEnv*_tmp61=_cycalloc(sizeof(*_tmp61));_tmp61->tables=tables,({
struct Cyc_Hashtable_Table*_tmp92=({(struct Cyc_Hashtable_Table*(*)(void))Cyc_JumpAnalysis_make_ptr_table;})();_tmp61->stmt_info=_tmp92;}),_tmp61->jumpers=0,({
# 269
struct Cyc_Hashtable_Table*_tmp91=({(struct Cyc_Hashtable_Table*(*)(void))Cyc_JumpAnalysis_make_var_table;})();_tmp61->labels=_tmp91;});_tmp61;});
# 266
struct Cyc_JumpAnalysis_FnEnv*fenv=_tmp32;
# 270
struct Cyc_JumpAnalysis_StmtEnv _tmp33=({struct Cyc_JumpAnalysis_StmtEnv _tmp6E;_tmp6E.fenv=fenv,_tmp6E.continue_stmt=(void*)& Cyc_JumpAnalysis_NotLoop_j_val,_tmp6E.break_stmt=(void*)& Cyc_JumpAnalysis_NotLoop_j_val,_tmp6E.next_stmt=(void*)& Cyc_JumpAnalysis_FnEnd_j_val,
# 274
(_tmp6E.save_env).try_depth=0,(_tmp6E.save_env).pat_var_depth=0,(_tmp6E.save_env).enclosers=0;_tmp6E;});
# 270
struct Cyc_JumpAnalysis_StmtEnv senv=_tmp33;
# 277
Cyc_JumpAnalysis_anal_stmt(senv,fd->body);{
# 279
struct Cyc_Hashtable_Table*pop_table=({(struct Cyc_Hashtable_Table*(*)(void))Cyc_JumpAnalysis_make_ptr_table;})();
struct Cyc_Hashtable_Table*succ_table=({(struct Cyc_Hashtable_Table*(*)(void))Cyc_JumpAnalysis_make_ptr_table;})();
struct Cyc_Hashtable_Table*pat_pop_table=({(struct Cyc_Hashtable_Table*(*)(void))Cyc_JumpAnalysis_make_ptr_table;})();
# 283
struct Cyc_Hashtable_Table*_tmp34=fenv->stmt_info;struct Cyc_Hashtable_Table*stmt_info=_tmp34;
{struct Cyc_List_List*_tmp35=fenv->jumpers;struct Cyc_List_List*jumpers=_tmp35;for(0;jumpers != 0;jumpers=jumpers->tl){
struct _tuple12*_tmp36=(struct _tuple12*)jumpers->hd;struct _tuple12*_stmttmp2=_tmp36;struct _tuple12*_tmp37=_stmttmp2;void*_tmp39;void*_tmp38;_tmp38=_tmp37->f1;_tmp39=_tmp37->f2;{struct Cyc_Absyn_Stmt*src_stmt=_tmp38;void*dest=_tmp39;
struct Cyc_JumpAnalysis_SaveEnv*_tmp3A=
# 288
({(struct Cyc_JumpAnalysis_SaveEnv*(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*))Cyc_Hashtable_lookup;})(stmt_info,src_stmt);
# 286
struct Cyc_JumpAnalysis_SaveEnv*_stmttmp3=_tmp3A;struct Cyc_JumpAnalysis_SaveEnv*_tmp3B=_stmttmp3;void*_tmp3E;int _tmp3D;int _tmp3C;_tmp3C=_tmp3B->try_depth;_tmp3D=_tmp3B->pat_var_depth;_tmp3E=_tmp3B->enclosers;{int src_trydepth=_tmp3C;int src_pvdepth=_tmp3D;struct Cyc_List_List*src_enclose=_tmp3E;
# 289
void*_tmp3F=dest;void*_tmp40;switch(*((int*)_tmp3F)){case 0:
# 291
({struct Cyc_Warn_String_Warn_Warg_struct _tmp42=({struct Cyc_Warn_String_Warn_Warg_struct _tmp68;_tmp68.tag=0,({struct _fat_ptr _tmp93=({const char*_tmp43="jump has no target";_tag_fat(_tmp43,sizeof(char),19U);});_tmp68.f1=_tmp93;});_tmp68;});void*_tmp41[1];_tmp41[0]=& _tmp42;Cyc_Warn_err2(src_stmt->loc,_tag_fat(_tmp41,sizeof(void*),1));});
goto _LL6;case 1:
# 294
({struct Cyc_Warn_String_Warn_Warg_struct _tmp45=({struct Cyc_Warn_String_Warn_Warg_struct _tmp69;_tmp69.tag=0,({
struct _fat_ptr _tmp94=({const char*_tmp46="break causes outer switch to implicitly fallthru";_tag_fat(_tmp46,sizeof(char),49U);});_tmp69.f1=_tmp94;});_tmp69;});void*_tmp44[1];_tmp44[0]=& _tmp45;Cyc_Warn_err2(src_stmt->loc,_tag_fat(_tmp44,sizeof(void*),1));});
goto _LL6;case 2:
# 298
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_Absyn_Stmt*))Cyc_Hashtable_insert;})(succ_table,src_stmt,0);
goto _LLE;case 5: _LLE:
# 301
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,int))Cyc_Hashtable_insert;})(pop_table,src_stmt,src_trydepth);
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,int))Cyc_Hashtable_insert;})(pat_pop_table,src_stmt,src_pvdepth);
goto _LL6;case 6: _tmp40=((struct Cyc_JumpAnalysis_Lab_j_JumpAnalysis_Jumpee_struct*)_tmp3F)->f1;{struct _fat_ptr*lab=_tmp40;
# 305
struct Cyc_Absyn_Stmt*dest_stmt;
{struct _handler_cons _tmp47;_push_handler(& _tmp47);{int _tmp49=0;if(setjmp(_tmp47.handler))_tmp49=1;if(!_tmp49){dest_stmt=({(struct Cyc_Absyn_Stmt*(*)(struct Cyc_Hashtable_Table*,struct _fat_ptr*))Cyc_Hashtable_lookup;})(fenv->labels,lab);;_pop_handler();}else{void*_tmp48=(void*)Cyc_Core_get_exn_thrown();void*_tmp4A=_tmp48;void*_tmp4B;if(((struct Cyc_Core_Not_found_exn_struct*)_tmp4A)->tag == Cyc_Core_Not_found){
# 308
({struct Cyc_Warn_String_Warn_Warg_struct _tmp4D=({struct Cyc_Warn_String_Warn_Warg_struct _tmp6C;_tmp6C.tag=0,({struct _fat_ptr _tmp95=({const char*_tmp51="label ";_tag_fat(_tmp51,sizeof(char),7U);});_tmp6C.f1=_tmp95;});_tmp6C;});struct Cyc_Warn_String_Warn_Warg_struct _tmp4E=({struct Cyc_Warn_String_Warn_Warg_struct _tmp6B;_tmp6B.tag=0,_tmp6B.f1=*lab;_tmp6B;});struct Cyc_Warn_String_Warn_Warg_struct _tmp4F=({struct Cyc_Warn_String_Warn_Warg_struct _tmp6A;_tmp6A.tag=0,({struct _fat_ptr _tmp96=({const char*_tmp50=" undefined";_tag_fat(_tmp50,sizeof(char),11U);});_tmp6A.f1=_tmp96;});_tmp6A;});void*_tmp4C[3];_tmp4C[0]=& _tmp4D,_tmp4C[1]=& _tmp4E,_tmp4C[2]=& _tmp4F;Cyc_Warn_err2(src_stmt->loc,_tag_fat(_tmp4C,sizeof(void*),3));});
goto L;}else{_tmp4B=_tmp4A;{void*exn=_tmp4B;(int)_rethrow(exn);}};}}}{
# 311
struct Cyc_JumpAnalysis_SaveEnv*_tmp52=
({(struct Cyc_JumpAnalysis_SaveEnv*(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*))Cyc_Hashtable_lookup;})(stmt_info,dest_stmt);
# 311
struct Cyc_JumpAnalysis_SaveEnv*_stmttmp4=_tmp52;struct Cyc_JumpAnalysis_SaveEnv*_tmp53=_stmttmp4;void*_tmp54;_tmp54=_tmp53->enclosers;{struct Cyc_List_List*dest_enclose=_tmp54;
# 313
if(dest_enclose != 0){
struct Cyc_Absyn_Stmt*_tmp55=(struct Cyc_Absyn_Stmt*)dest_enclose->hd;struct Cyc_Absyn_Stmt*dest_scope=_tmp55;
for(1;src_enclose != 0;src_enclose=src_enclose->tl){
if((struct Cyc_Absyn_Stmt*)src_enclose->hd == dest_scope)
break;}
if(src_enclose == 0){
({struct Cyc_Warn_String_Warn_Warg_struct _tmp57=({struct Cyc_Warn_String_Warn_Warg_struct _tmp6D;_tmp6D.tag=0,({
struct _fat_ptr _tmp97=({const char*_tmp58="goto enters local scope or exception handler";_tag_fat(_tmp58,sizeof(char),45U);});_tmp6D.f1=_tmp97;});_tmp6D;});void*_tmp56[1];_tmp56[0]=& _tmp57;Cyc_Warn_err2(src_stmt->loc,_tag_fat(_tmp56,sizeof(void*),1));});
goto _LL6;}}
# 324
_tmp40=dest_stmt;goto _LL12;}}}case 3: _tmp40=((struct Cyc_JumpAnalysis_Stmt_j_JumpAnalysis_Jumpee_struct*)_tmp3F)->f1;_LL12: {struct Cyc_Absyn_Stmt*dest_stmt=_tmp40;
# 326
struct Cyc_JumpAnalysis_SaveEnv*_tmp59=
({(struct Cyc_JumpAnalysis_SaveEnv*(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*))Cyc_Hashtable_lookup;})(stmt_info,dest_stmt);
# 326
struct Cyc_JumpAnalysis_SaveEnv*_stmttmp5=_tmp59;struct Cyc_JumpAnalysis_SaveEnv*_tmp5A=_stmttmp5;int _tmp5C;int _tmp5B;_tmp5B=_tmp5A->try_depth;_tmp5C=_tmp5A->pat_var_depth;{int dest_trydepth=_tmp5B;int dest_pvdepth=_tmp5C;
# 328
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,int))Cyc_Hashtable_insert;})(pop_table,src_stmt,src_trydepth - dest_trydepth);
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,int))Cyc_Hashtable_insert;})(pat_pop_table,src_stmt,src_pvdepth - dest_pvdepth);
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_Absyn_Stmt*))Cyc_Hashtable_insert;})(succ_table,src_stmt,dest_stmt);
goto _LL6;}}default: _tmp40=((struct Cyc_JumpAnalysis_Fallthru_j_JumpAnalysis_Jumpee_struct*)_tmp3F)->f1;{struct Cyc_Absyn_Switch_clause*dest_clause=_tmp40;
# 333
struct Cyc_JumpAnalysis_SaveEnv*_tmp5D=
({(struct Cyc_JumpAnalysis_SaveEnv*(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*))Cyc_Hashtable_lookup;})(stmt_info,dest_clause->body);
# 333
struct Cyc_JumpAnalysis_SaveEnv*_stmttmp6=_tmp5D;struct Cyc_JumpAnalysis_SaveEnv*_tmp5E=_stmttmp6;int _tmp60;int _tmp5F;_tmp5F=_tmp5E->try_depth;_tmp60=_tmp5E->pat_var_depth;{int dest_trydepth=_tmp5F;int dest_pvdepth=_tmp60;
# 335
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,int))Cyc_Hashtable_insert;})(pop_table,src_stmt,src_trydepth - dest_trydepth);{
int pv_pop;
if(Cyc_Tcpat_has_vars(dest_clause->pat_vars))
pv_pop=(1 + src_pvdepth)- dest_pvdepth;else{
# 340
pv_pop=src_pvdepth - dest_pvdepth;}
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,int))Cyc_Hashtable_insert;})(pat_pop_table,src_stmt,pv_pop);
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_Absyn_Stmt*))Cyc_Hashtable_insert;})(succ_table,src_stmt,dest_clause->body);
goto _LL6;}}}}_LL6:;}}}}
# 346
L:({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Fndecl*,struct Cyc_Hashtable_Table*))Cyc_Hashtable_insert;})(tables->pop_tables,fd,pop_table);
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Fndecl*,struct Cyc_Hashtable_Table*))Cyc_Hashtable_insert;})(tables->succ_tables,fd,succ_table);
({(void(*)(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Fndecl*,struct Cyc_Hashtable_Table*))Cyc_Hashtable_insert;})(tables->pat_pop_tables,fd,pat_pop_table);}}
# 351
static void Cyc_JumpAnalysis_jump_analysis_iter(struct Cyc_JumpAnalysis_Jump_Anal_Result*tables,struct Cyc_List_List*tds){
# 354
for(1;tds != 0;tds=tds->tl){
void*_tmp62=((struct Cyc_Absyn_Decl*)tds->hd)->r;void*_stmttmp7=_tmp62;void*_tmp63=_stmttmp7;void*_tmp64;switch(*((int*)_tmp63)){case 2:
 goto _LL4;case 3: _LL4:
 goto _LL6;case 4: _LL6:
 goto _LL8;case 0: _LL8:
 goto _LLA;case 8: _LLA:
 goto _LLC;case 5: _LLC:
 goto _LLE;case 6: _LLE:
 goto _LL10;case 7: _LL10:
 goto _LL12;case 13: _LL12:
 goto _LL14;case 14: _LL14:
 goto _LL16;case 15: _LL16:
 goto _LL18;case 16: _LL18:
 goto _LL1A;case 11: _LL1A:
 goto _LL1C;case 12: _LL1C:
 continue;case 9: _tmp64=((struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct*)_tmp63)->f2;{struct Cyc_List_List*tds2=_tmp64;
_tmp64=tds2;goto _LL20;}case 10: _tmp64=((struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct*)_tmp63)->f2;_LL20: {struct Cyc_List_List*tds2=_tmp64;
Cyc_JumpAnalysis_jump_analysis_iter(tables,tds2);goto _LL0;}default: _tmp64=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp63)->f1;{struct Cyc_Absyn_Fndecl*fd=_tmp64;
Cyc_JumpAnalysis_anal_fd(tables,fd);goto _LL0;}}_LL0:;}}
# 376
struct Cyc_JumpAnalysis_Jump_Anal_Result*Cyc_JumpAnalysis_jump_analysis(struct Cyc_List_List*tds){
struct Cyc_JumpAnalysis_Jump_Anal_Result*_tmp65=({struct Cyc_JumpAnalysis_Jump_Anal_Result*_tmp66=_cycalloc(sizeof(*_tmp66));({struct Cyc_Hashtable_Table*_tmp9A=({(struct Cyc_Hashtable_Table*(*)(void))Cyc_JumpAnalysis_make_ptr_table;})();_tmp66->pop_tables=_tmp9A;}),({
struct Cyc_Hashtable_Table*_tmp99=({(struct Cyc_Hashtable_Table*(*)(void))Cyc_JumpAnalysis_make_ptr_table;})();_tmp66->succ_tables=_tmp99;}),({
struct Cyc_Hashtable_Table*_tmp98=({(struct Cyc_Hashtable_Table*(*)(void))Cyc_JumpAnalysis_make_ptr_table;})();_tmp66->pat_pop_tables=_tmp98;});_tmp66;});
# 377
struct Cyc_JumpAnalysis_Jump_Anal_Result*tables=_tmp65;
# 380
Cyc_JumpAnalysis_jump_analysis_iter(tables,tds);
return tables;}
