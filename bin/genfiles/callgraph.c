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

/* Discriminated Unions */
struct _xtunion_struct { char *tag; };

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
void   _free_region(struct _RegionHandle*);
struct _RegionHandle*_open_dynregion(struct _DynRegionFrame*,struct _DynRegionHandle*);
void   _pop_dynregion();

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

struct _xtunion_struct* Cyc_Core_get_exn_thrown();
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
#ifdef CYC_ANSI_OUTPUT
#define _INLINE  
#else
#define _INLINE inline
#endif

#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#define _check_null(ptr) \
  ({ void*_cks_null = (void*)(ptr); \
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

static _INLINE unsigned int _check_times(unsigned x, unsigned y) {
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
 struct Cyc_Core_Opt{void*v;};extern char Cyc_Core_Invalid_argument[17U];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _fat_ptr f1;};extern char Cyc_Core_Failure[8U];struct Cyc_Core_Failure_exn_struct{char*tag;struct _fat_ptr f1;};extern char Cyc_Core_Impossible[11U];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _fat_ptr f1;};extern char Cyc_Core_Not_found[10U];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12U];struct Cyc_Core_Unreachable_exn_struct{char*tag;struct _fat_ptr f1;};
# 171 "core.h"
extern struct _RegionHandle*Cyc_Core_unique_region;struct Cyc_Core_DynamicRegion;struct Cyc_Core_NewDynamicRegion{struct Cyc_Core_DynamicRegion*key;};struct Cyc_Core_ThinRes{void*arr;unsigned nelts;};struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};extern char Cyc_List_List_mismatch[14U];struct Cyc_List_List_mismatch_exn_struct{char*tag;};extern char Cyc_List_Nth[4U];struct Cyc_List_Nth_exn_struct{char*tag;};struct Cyc_Position_Error;struct Cyc_Relations_Reln;struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};struct _tuple0{union Cyc_Absyn_Nmspace f1;struct _fat_ptr*f2;};
# 149 "absyn.h"
enum Cyc_Absyn_Scope{Cyc_Absyn_Static =0U,Cyc_Absyn_Abstract =1U,Cyc_Absyn_Public =2U,Cyc_Absyn_Extern =3U,Cyc_Absyn_ExternC =4U,Cyc_Absyn_Register =5U};struct Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;unsigned loc;};
# 170
enum Cyc_Absyn_Size_of{Cyc_Absyn_Char_sz =0U,Cyc_Absyn_Short_sz =1U,Cyc_Absyn_Int_sz =2U,Cyc_Absyn_Long_sz =3U,Cyc_Absyn_LongLong_sz =4U};
enum Cyc_Absyn_Sign{Cyc_Absyn_Signed =0U,Cyc_Absyn_Unsigned =1U,Cyc_Absyn_None =2U};
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA =0U,Cyc_Absyn_UnionA =1U};
# 175
enum Cyc_Absyn_AliasQual{Cyc_Absyn_Aliasable =0U,Cyc_Absyn_Unique =1U,Cyc_Absyn_Top =2U};
# 180
enum Cyc_Absyn_KindQual{Cyc_Absyn_AnyKind =0U,Cyc_Absyn_MemKind =1U,Cyc_Absyn_BoxKind =2U,Cyc_Absyn_RgnKind =3U,Cyc_Absyn_EffKind =4U,Cyc_Absyn_IntKind =5U,Cyc_Absyn_BoolKind =6U,Cyc_Absyn_PtrBndKind =7U};struct Cyc_Absyn_Kind{enum Cyc_Absyn_KindQual kind;enum Cyc_Absyn_AliasQual aliasqual;};struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct{int tag;struct Cyc_Absyn_Kind*f1;};struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{struct _fat_ptr*name;int identity;void*kind;};struct Cyc_Absyn_PtrLoc{unsigned ptr_loc;unsigned rgn_loc;unsigned zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;void*nullable;void*bounds;void*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;void*released;};struct Cyc_Absyn_PtrInfo{void*elt_type;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{struct _fat_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;struct Cyc_Absyn_Vardecl*return_value;};struct Cyc_Absyn_UnknownDatatypeInfo{struct _tuple0*name;int is_extensible;};struct _union_DatatypeInfo_UnknownDatatype{int tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct _union_DatatypeInfo_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};union Cyc_Absyn_DatatypeInfo{struct _union_DatatypeInfo_UnknownDatatype UnknownDatatype;struct _union_DatatypeInfo_KnownDatatype KnownDatatype;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{struct _tuple0*datatype_name;struct _tuple0*field_name;int is_extensible;};struct _union_DatatypeFieldInfo_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo val;};struct _tuple1{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct _union_DatatypeFieldInfo_KnownDatatypefield{int tag;struct _tuple1 val;};union Cyc_Absyn_DatatypeFieldInfo{struct _union_DatatypeFieldInfo_UnknownDatatypefield UnknownDatatypefield;struct _union_DatatypeFieldInfo_KnownDatatypefield KnownDatatypefield;};struct _tuple2{enum Cyc_Absyn_AggrKind f1;struct _tuple0*f2;struct Cyc_Core_Opt*f3;};struct _union_AggrInfo_UnknownAggr{int tag;struct _tuple2 val;};struct _union_AggrInfo_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfo{struct _union_AggrInfo_UnknownAggr UnknownAggr;struct _union_AggrInfo_KnownAggr KnownAggr;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;void*zero_term;unsigned zt_loc;};struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_TypeDecl{void*r;unsigned loc;};struct Cyc_Absyn_VoidCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_IntCon_Absyn_TyCon_struct{int tag;enum Cyc_Absyn_Sign f1;enum Cyc_Absyn_Size_of f2;};struct Cyc_Absyn_FloatCon_Absyn_TyCon_struct{int tag;int f1;};struct Cyc_Absyn_RgnHandleCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_TagCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_HeapCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_UniqueCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_RefCntCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_AccessCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_JoinCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_RgnsCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_TrueCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_FalseCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_ThinCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_FatCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_EnumCon_Absyn_TyCon_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumCon_Absyn_TyCon_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_BuiltinCon_Absyn_TyCon_struct{int tag;struct _fat_ptr f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_DatatypeCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_DatatypeInfo f1;};struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_DatatypeFieldInfo f1;};struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AppType_Absyn_Type_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Evar_Absyn_Type_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct{int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};struct Cyc_Absyn_TypeofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};
# 392 "absyn.h"
enum Cyc_Absyn_Format_Type{Cyc_Absyn_Printf_ft =0U,Cyc_Absyn_Scanf_ft =1U};struct Cyc_Absyn_Regparm_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Stdcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Cdecl_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Fastcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Noreturn_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Const_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Aligned_att_Absyn_Attribute_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Packed_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Section_att_Absyn_Attribute_struct{int tag;struct _fat_ptr f1;};struct Cyc_Absyn_Nocommon_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Shared_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Unused_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Weak_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllimport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllexport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_instrument_function_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Constructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Destructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_check_memory_usage_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Format_att_Absyn_Attribute_struct{int tag;enum Cyc_Absyn_Format_Type f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Consume_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Pure_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Mode_att_Absyn_Attribute_struct{int tag;struct _fat_ptr f1;};struct Cyc_Absyn_Alias_att_Absyn_Attribute_struct{int tag;struct _fat_ptr f1;};struct Cyc_Absyn_Always_inline_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_throw_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_NoTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;unsigned f2;};struct Cyc_Absyn_WithTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;struct Cyc_Absyn_Exp*f6;struct Cyc_Absyn_Exp*f7;};struct Cyc_Absyn_Carray_mod_Absyn_Type_modifier_struct{int tag;void*f1;unsigned f2;};struct Cyc_Absyn_ConstArray_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;unsigned f3;};struct Cyc_Absyn_Pointer_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_Absyn_Type_modifier_struct{int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_List_List*f1;unsigned f2;int f3;};struct Cyc_Absyn_Attributes_mod_Absyn_Type_modifier_struct{int tag;unsigned f1;struct Cyc_List_List*f2;};struct _union_Cnst_Null_c{int tag;int val;};struct _tuple3{enum Cyc_Absyn_Sign f1;char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple3 val;};struct _union_Cnst_Wchar_c{int tag;struct _fat_ptr val;};struct _tuple4{enum Cyc_Absyn_Sign f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple4 val;};struct _tuple5{enum Cyc_Absyn_Sign f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple5 val;};struct _tuple6{enum Cyc_Absyn_Sign f1;long long f2;};struct _union_Cnst_LongLong_c{int tag;struct _tuple6 val;};struct _tuple7{struct _fat_ptr f1;int f2;};struct _union_Cnst_Float_c{int tag;struct _tuple7 val;};struct _union_Cnst_String_c{int tag;struct _fat_ptr val;};struct _union_Cnst_Wstring_c{int tag;struct _fat_ptr val;};union Cyc_Absyn_Cnst{struct _union_Cnst_Null_c Null_c;struct _union_Cnst_Char_c Char_c;struct _union_Cnst_Wchar_c Wchar_c;struct _union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct _union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct _union_Cnst_String_c String_c;struct _union_Cnst_Wstring_c Wstring_c;};
# 465
enum Cyc_Absyn_Primop{Cyc_Absyn_Plus =0U,Cyc_Absyn_Times =1U,Cyc_Absyn_Minus =2U,Cyc_Absyn_Div =3U,Cyc_Absyn_Mod =4U,Cyc_Absyn_Eq =5U,Cyc_Absyn_Neq =6U,Cyc_Absyn_Gt =7U,Cyc_Absyn_Lt =8U,Cyc_Absyn_Gte =9U,Cyc_Absyn_Lte =10U,Cyc_Absyn_Not =11U,Cyc_Absyn_Bitnot =12U,Cyc_Absyn_Bitand =13U,Cyc_Absyn_Bitor =14U,Cyc_Absyn_Bitxor =15U,Cyc_Absyn_Bitlshift =16U,Cyc_Absyn_Bitlrshift =17U,Cyc_Absyn_Numelts =18U};
# 472
enum Cyc_Absyn_Incrementor{Cyc_Absyn_PreInc =0U,Cyc_Absyn_PostInc =1U,Cyc_Absyn_PreDec =2U,Cyc_Absyn_PostDec =3U};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct{int tag;struct _fat_ptr*f1;};struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct{int tag;unsigned f1;};
# 490
enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion =0U,Cyc_Absyn_No_coercion =1U,Cyc_Absyn_Null_to_NonNull =2U,Cyc_Absyn_Other_coercion =3U};struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_Absyn_Designator_struct{int tag;struct _fat_ptr*f1;};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;int inline_call;};struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct{int tag;union Cyc_Absyn_Cnst f1;};struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Pragma_e_Absyn_Raw_exp_struct{int tag;struct _fat_ptr f1;};struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct{int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor f2;};struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;int f4;};struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;int f2;};struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _fat_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _fat_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct _tuple8{struct _fat_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct{int tag;struct _tuple8*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;int f3;};struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _fat_ptr*f2;};struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct{int tag;int f1;struct _fat_ptr f2;struct Cyc_List_List*f3;struct Cyc_List_List*f4;struct Cyc_List_List*f5;};struct Cyc_Absyn_Extension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Assert_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned loc;void*annot;};struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple9{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct{int tag;struct _tuple9 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct{int tag;struct _fat_ptr*f1;};struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple9 f2;struct _tuple9 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct{int tag;struct _fat_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple9 f2;};struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Stmt{void*r;unsigned loc;void*annot;};struct Cyc_Absyn_Wild_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct{int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct{int tag;union Cyc_Absyn_AggrInfo*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Null_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct{int tag;enum Cyc_Absyn_Sign f1;int f2;};struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct{int tag;struct _fat_ptr f1;int f2;};struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_Absyn_Raw_pat_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_Absyn_Raw_pat_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;void*topt;unsigned loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;unsigned loc;};struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_Global_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Local_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;unsigned varloc;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;int is_proto;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple0*name;struct Cyc_Absyn_Stmt*body;struct Cyc_Absyn_FnInfo i;void*cached_type;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;enum Cyc_Absyn_Scope orig_scope;};struct Cyc_Absyn_Aggrfield{struct _fat_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind kind;enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*impl;struct Cyc_List_List*attributes;int expected_mem_kind;};struct Cyc_Absyn_Datatypefield{struct _tuple0*name;struct Cyc_List_List*typs;unsigned loc;enum Cyc_Absyn_Scope sc;};struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct _tuple0*name;struct Cyc_Absyn_Exp*tag;unsigned loc;};struct Cyc_Absyn_Enumdecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct _tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;int extern_c;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;void*f4;};struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Datatype_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct{int tag;struct _fat_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct _tuple10{unsigned f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternCinclude_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct _tuple10*f4;};struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Tempeston_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Tempestoff_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Decl{void*r;unsigned loc;};extern char Cyc_Absyn_EmptyAnnot[11U];struct Cyc_Absyn_EmptyAnnot_Absyn_AbsynAnnot_struct{char*tag;};
# 844 "absyn.h"
int Cyc_Absyn_qvar_cmp(struct _tuple0*,struct _tuple0*);
int Cyc_Absyn_hash_qvar(struct _tuple0*);struct Cyc_Iter_Iter{void*env;int(*next)(void*,void*);};struct Cyc_Set_Set;
# 51 "set.h"
extern struct Cyc_Set_Set*Cyc_Set_empty(int(*)(void*,void*));
# 63
extern struct Cyc_Set_Set*Cyc_Set_insert(struct Cyc_Set_Set*,void*);extern char Cyc_Set_Absent[7U];struct Cyc_Set_Absent_exn_struct{char*tag;};struct Cyc___cycFILE;struct Cyc_String_pa_PrintArg_struct{int tag;struct _fat_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_PrintArg_struct{int tag;double f1;};struct Cyc_LongDouble_pa_PrintArg_struct{int tag;long double f1;};struct Cyc_ShortPtr_pa_PrintArg_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_PrintArg_struct{int tag;unsigned long*f1;};
# 100 "cycboot.h"
extern int Cyc_fprintf(struct Cyc___cycFILE*,struct _fat_ptr,struct _fat_ptr);struct Cyc_ShortPtr_sa_ScanfArg_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_ScanfArg_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_ScanfArg_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_ScanfArg_struct{int tag;unsigned*f1;};struct Cyc_StringPtr_sa_ScanfArg_struct{int tag;struct _fat_ptr f1;};struct Cyc_DoublePtr_sa_ScanfArg_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_ScanfArg_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_ScanfArg_struct{int tag;struct _fat_ptr f1;};extern char Cyc_FileCloseError[15U];struct Cyc_FileCloseError_exn_struct{char*tag;};extern char Cyc_FileOpenError[14U];struct Cyc_FileOpenError_exn_struct{char*tag;struct _fat_ptr f1;};struct Cyc_timeval{long tv_sec;long tv_usec;};struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[8U];struct Cyc_Dict_Present_exn_struct{char*tag;};extern char Cyc_Dict_Absent[7U];struct Cyc_Dict_Absent_exn_struct{char*tag;};
# 46 "graph.h"
void Cyc_Graph_print(struct Cyc___cycFILE*,struct Cyc_Dict_Dict,void(*)(struct Cyc___cycFILE*,void*));
# 50
extern struct Cyc_Dict_Dict Cyc_Graph_empty(int(*)(void*,void*));
# 53
extern struct Cyc_Dict_Dict Cyc_Graph_add_node(struct Cyc_Dict_Dict,void*);
# 59
extern struct Cyc_Dict_Dict Cyc_Graph_add_edges(struct Cyc_Dict_Dict,void*,struct Cyc_Set_Set*);struct Cyc_PP_Ppstate;struct Cyc_PP_Out;struct Cyc_PP_Doc;struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;};
# 71 "absynpp.h"
struct _fat_ptr Cyc_Absynpp_qvar2string(struct _tuple0*);struct Cyc_Hashtable_Table;
# 39 "hashtable.h"
extern struct Cyc_Hashtable_Table*Cyc_Hashtable_create(int,int(*)(void*,void*),int(*)(void*));
# 50
extern void Cyc_Hashtable_insert(struct Cyc_Hashtable_Table*,void*,void*);
# 52
extern void*Cyc_Hashtable_lookup(struct Cyc_Hashtable_Table*,void*);
# 47 "callgraph.cyc"
static struct Cyc_Set_Set*Cyc_Callgraph_cg_stmt(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Stmt*,struct Cyc_Set_Set*);
static struct Cyc_Set_Set*Cyc_Callgraph_cg_exp(struct Cyc_Hashtable_Table*,struct Cyc_Absyn_Exp*,struct Cyc_Set_Set*);
# 50
static struct Cyc_Set_Set*Cyc_Callgraph_cg_exps(struct Cyc_Hashtable_Table*fds,struct Cyc_List_List*es,struct Cyc_Set_Set*s){
for(1;es != 0;es=es->tl){
s=Cyc_Callgraph_cg_exp(fds,(struct Cyc_Absyn_Exp*)es->hd,s);}
return s;}struct _tuple11{struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};
# 56
static struct Cyc_Set_Set*Cyc_Callgraph_cg_exp(struct Cyc_Hashtable_Table*fds,struct Cyc_Absyn_Exp*e,struct Cyc_Set_Set*s){
void*_tmp0=e->r;void*_stmttmp0=_tmp0;void*_tmp1=_stmttmp0;struct Cyc_Absyn_MallocInfo _tmp4;void*_tmp5;void*_tmp3;void*_tmp2;switch(*((int*)_tmp1)){case 1: switch(*((int*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp1)->f1)){case 2: _tmp2=((struct Cyc_Absyn_Funname_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp1)->f1)->f1;{struct Cyc_Absyn_Fndecl*fd=_tmp2;
# 59
struct _handler_cons _tmp6;_push_handler(& _tmp6);{int _tmp8=0;if(setjmp(_tmp6.handler))_tmp8=1;if(!_tmp8){
# 62
{struct Cyc_Set_Set*_tmp9=({struct Cyc_Set_Set*(*_tmp2F)(struct Cyc_Set_Set*,struct Cyc_Absyn_Fndecl*)=({(struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*,struct Cyc_Absyn_Fndecl*))Cyc_Set_insert;});struct Cyc_Set_Set*_tmp2E=s;_tmp2F(_tmp2E,({(struct Cyc_Absyn_Fndecl*(*)(struct Cyc_Hashtable_Table*,struct _tuple0*))Cyc_Hashtable_lookup;})(fds,fd->name));});_npop_handler(0);return _tmp9;};_pop_handler();}else{void*_tmp7=(void*)Cyc_Core_get_exn_thrown();void*_tmpA=_tmp7;void*_tmpB;if(((struct Cyc_Core_Not_found_exn_struct*)_tmpA)->tag == Cyc_Core_Not_found)
# 64
return s;else{_tmpB=_tmpA;{void*exn=_tmpB;(int)_rethrow(exn);}};}}}case 1: _tmp2=((struct Cyc_Absyn_Global_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp1)->f1)->f1;{struct Cyc_Absyn_Vardecl*vd=_tmp2;
# 67
struct _handler_cons _tmpC;_push_handler(& _tmpC);{int _tmpE=0;if(setjmp(_tmpC.handler))_tmpE=1;if(!_tmpE){
{struct Cyc_Set_Set*_tmpF=({struct Cyc_Set_Set*(*_tmp31)(struct Cyc_Set_Set*,struct Cyc_Absyn_Fndecl*)=({(struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*,struct Cyc_Absyn_Fndecl*))Cyc_Set_insert;});struct Cyc_Set_Set*_tmp30=s;_tmp31(_tmp30,({(struct Cyc_Absyn_Fndecl*(*)(struct Cyc_Hashtable_Table*,struct _tuple0*))Cyc_Hashtable_lookup;})(fds,vd->name));});_npop_handler(0);return _tmpF;};_pop_handler();}else{void*_tmpD=(void*)Cyc_Core_get_exn_thrown();void*_tmp10=_tmpD;void*_tmp11;if(((struct Cyc_Core_Not_found_exn_struct*)_tmp10)->tag == Cyc_Core_Not_found)
# 70
return s;else{_tmp11=_tmp10;{void*exn=_tmp11;(int)_rethrow(exn);}};}}}default:
# 72
 goto _LL8;}case 0: _LL8: goto _LLA;case 2: _LLA: goto _LLC;case 17: _LLC:
 goto _LLE;case 18: _LLE: goto _LL10;case 19: _LL10: goto _LL12;case 32: _LL12:
 goto _LL14;case 33: _LL14: goto _LL16;case 39: _LL16: goto _LL18;case 40: _LL18:
 goto _LL1A;case 41: _LL1A: return s;case 3: _tmp2=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp1)->f2;{struct Cyc_List_List*es=_tmp2;
return Cyc_Callgraph_cg_exps(fds,es,s);}case 7: _tmp2=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_tmp3=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp1)->f2;{struct Cyc_Absyn_Exp*e1=_tmp2;struct Cyc_Absyn_Exp*e2=_tmp3;
_tmp2=e1;_tmp3=e2;goto _LL20;}case 8: _tmp2=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_tmp3=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_LL20: {struct Cyc_Absyn_Exp*e1=_tmp2;struct Cyc_Absyn_Exp*e2=_tmp3;
_tmp2=e1;_tmp3=e2;goto _LL22;}case 9: _tmp2=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_tmp3=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_LL22: {struct Cyc_Absyn_Exp*e1=_tmp2;struct Cyc_Absyn_Exp*e2=_tmp3;
_tmp2=e1;_tmp3=e2;goto _LL24;}case 35: _tmp2=((struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_tmp3=((struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_LL24: {struct Cyc_Absyn_Exp*e1=_tmp2;struct Cyc_Absyn_Exp*e2=_tmp3;
_tmp2=e1;_tmp3=e2;goto _LL26;}case 23: _tmp2=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_tmp3=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_LL26: {struct Cyc_Absyn_Exp*e1=_tmp2;struct Cyc_Absyn_Exp*e2=_tmp3;
_tmp2=e1;_tmp3=e2;goto _LL28;}case 27: _tmp2=((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_tmp3=((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp1)->f3;_LL28: {struct Cyc_Absyn_Exp*e1=_tmp2;struct Cyc_Absyn_Exp*e2=_tmp3;
_tmp2=e1;_tmp3=e2;goto _LL2A;}case 4: _tmp2=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_tmp3=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp1)->f3;_LL2A: {struct Cyc_Absyn_Exp*e1=_tmp2;struct Cyc_Absyn_Exp*e2=_tmp3;
return({struct Cyc_Hashtable_Table*_tmp33=fds;struct Cyc_Absyn_Exp*_tmp32=e2;Cyc_Callgraph_cg_exp(_tmp33,_tmp32,Cyc_Callgraph_cg_exp(fds,e1,s));});}case 11: _tmp2=((struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct*)_tmp1)->f1;{struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL2E;}case 12: _tmp2=((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL2E: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL30;}case 13: _tmp2=((struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL30: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL32;}case 14: _tmp2=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_LL32: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL34;}case 15: _tmp2=((struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL34: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL36;}case 20: _tmp2=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL36: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL38;}case 21: _tmp2=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL38: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL3A;}case 22: _tmp2=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL3A: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL3C;}case 38: _tmp2=((struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL3C: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL3E;}case 42: _tmp2=((struct Cyc_Absyn_Assert_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL3E: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL40;}case 28: _tmp2=((struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL40: {struct Cyc_Absyn_Exp*e=_tmp2;
_tmp2=e;goto _LL42;}case 5: _tmp2=((struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL42: {struct Cyc_Absyn_Exp*e=_tmp2;
return Cyc_Callgraph_cg_exp(fds,e,s);}case 6: _tmp2=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_tmp3=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_tmp5=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp1)->f3;{struct Cyc_Absyn_Exp*e1=_tmp2;struct Cyc_Absyn_Exp*e2=_tmp3;struct Cyc_Absyn_Exp*e3=_tmp5;
# 97
return({struct Cyc_Hashtable_Table*_tmp37=fds;struct Cyc_Absyn_Exp*_tmp36=e3;Cyc_Callgraph_cg_exp(_tmp37,_tmp36,({struct Cyc_Hashtable_Table*_tmp35=fds;struct Cyc_Absyn_Exp*_tmp34=e2;Cyc_Callgraph_cg_exp(_tmp35,_tmp34,Cyc_Callgraph_cg_exp(fds,e1,s));}));});}case 10: _tmp2=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_tmp3=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp1)->f2;{struct Cyc_Absyn_Exp*e=_tmp2;struct Cyc_List_List*es=_tmp3;
return({struct Cyc_Hashtable_Table*_tmp39=fds;struct Cyc_List_List*_tmp38=es;Cyc_Callgraph_cg_exps(_tmp39,_tmp38,Cyc_Callgraph_cg_exp(fds,e,s));});}case 34: _tmp4=((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp1)->f1;{struct Cyc_Absyn_MallocInfo mi=_tmp4;
_tmp2=mi.rgn;_tmp3=mi.num_elts;goto _LL4A;}case 16: _tmp2=((struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_tmp3=((struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_LL4A: {struct Cyc_Absyn_Exp*eopt=_tmp2;struct Cyc_Absyn_Exp*e=_tmp3;
# 101
return({struct Cyc_Hashtable_Table*_tmp3B=fds;struct Cyc_Absyn_Exp*_tmp3A=e;Cyc_Callgraph_cg_exp(_tmp3B,_tmp3A,eopt == 0?s: Cyc_Callgraph_cg_exp(fds,eopt,s));});}case 31: _tmp2=((struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct*)_tmp1)->f1;{struct Cyc_List_List*es=_tmp2;
_tmp2=es;goto _LL4E;}case 24: _tmp2=((struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL4E: {struct Cyc_List_List*es=_tmp2;
return Cyc_Callgraph_cg_exps(fds,es,s);}case 36: _tmp2=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp1)->f2;{struct Cyc_List_List*dles=_tmp2;
_tmp2=dles;goto _LL52;}case 25: _tmp2=((struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_LL52: {struct Cyc_List_List*dles=_tmp2;
_tmp2=dles;goto _LL54;}case 29: _tmp2=((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp1)->f3;_LL54: {struct Cyc_List_List*dles=_tmp2;
_tmp2=dles;goto _LL56;}case 30: _tmp2=((struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct*)_tmp1)->f2;_LL56: {struct Cyc_List_List*dles=_tmp2;
_tmp2=dles;goto _LL58;}case 26: _tmp2=((struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*)_tmp1)->f1;_LL58: {struct Cyc_List_List*dles=_tmp2;
# 109
for(1;dles != 0;dles=dles->tl){
struct _tuple11*_tmp12=(struct _tuple11*)dles->hd;struct _tuple11*_stmttmp1=_tmp12;struct _tuple11*_tmp13=_stmttmp1;void*_tmp14;_tmp14=_tmp13->f2;{struct Cyc_Absyn_Exp*e=_tmp14;
s=Cyc_Callgraph_cg_exp(fds,e,s);}}
# 113
return s;}default: _tmp2=((struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*)_tmp1)->f1;{struct Cyc_Absyn_Stmt*stmt=_tmp2;
return Cyc_Callgraph_cg_stmt(fds,stmt,s);}};}
# 118
static struct Cyc_Set_Set*Cyc_Callgraph_cg_decl(struct Cyc_Hashtable_Table*fds,struct Cyc_Absyn_Decl*d,struct Cyc_Set_Set*s){
void*_tmp15=d->r;void*_stmttmp2=_tmp15;void*_tmp16=_stmttmp2;void*_tmp18;void*_tmp17;switch(*((int*)_tmp16)){case 0: _tmp17=((struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp16)->f1;{struct Cyc_Absyn_Vardecl*vd=_tmp17;
# 121
struct Cyc_Absyn_Exp*_tmp19=vd->initializer;struct Cyc_Absyn_Exp*eopt=_tmp19;
return eopt == 0?s: Cyc_Callgraph_cg_exp(fds,eopt,s);}case 2: _tmp17=((struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct*)_tmp16)->f3;{struct Cyc_Absyn_Exp*e=_tmp17;
return Cyc_Callgraph_cg_exp(fds,e,s);}case 3: _tmp17=((struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct*)_tmp16)->f1;{struct Cyc_List_List*vds=_tmp17;
# 125
for(1;vds != 0;vds=vds->tl){
struct Cyc_Absyn_Exp*_tmp1A=((struct Cyc_Absyn_Vardecl*)vds->hd)->initializer;struct Cyc_Absyn_Exp*eopt=_tmp1A;
s=eopt == 0?s: Cyc_Callgraph_cg_exp(fds,eopt,s);}
# 129
return s;}case 4: _tmp17=((struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct*)_tmp16)->f2;_tmp18=((struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct*)_tmp16)->f3;{struct Cyc_Absyn_Vardecl*vd=_tmp17;struct Cyc_Absyn_Exp*eopt=_tmp18;
# 131
s=vd->initializer == 0?s: Cyc_Callgraph_cg_exp(fds,(struct Cyc_Absyn_Exp*)_check_null(vd->initializer),s);
return eopt == 0?s: Cyc_Callgraph_cg_exp(fds,eopt,s);}default:
 return s;};}
# 137
static struct Cyc_Set_Set*Cyc_Callgraph_cg_stmt(struct Cyc_Hashtable_Table*fds,struct Cyc_Absyn_Stmt*stmt,struct Cyc_Set_Set*s){
void*_tmp1B=stmt->r;void*_stmttmp3=_tmp1B;void*_tmp1C=_stmttmp3;void*_tmp20;void*_tmp1F;void*_tmp1E;void*_tmp1D;switch(*((int*)_tmp1C)){case 6:
 goto _LL4;case 7: _LL4:
 goto _LL6;case 8: _LL6:
 goto _LL8;case 0: _LL8:
 return s;case 1: _tmp1D=((struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;{struct Cyc_Absyn_Exp*e=_tmp1D;
return Cyc_Callgraph_cg_exp(fds,e,s);}case 2: _tmp1D=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;_tmp1E=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp1C)->f2;{struct Cyc_Absyn_Stmt*s1=_tmp1D;struct Cyc_Absyn_Stmt*s2=_tmp1E;
return({struct Cyc_Hashtable_Table*_tmp3D=fds;struct Cyc_Absyn_Stmt*_tmp3C=s2;Cyc_Callgraph_cg_stmt(_tmp3D,_tmp3C,Cyc_Callgraph_cg_stmt(fds,s1,s));});}case 3: _tmp1D=((struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;{struct Cyc_Absyn_Exp*eopt=_tmp1D;
return eopt != 0?Cyc_Callgraph_cg_exp(fds,eopt,s): s;}case 4: _tmp1D=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;_tmp1E=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp1C)->f2;_tmp1F=((struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp1C)->f3;{struct Cyc_Absyn_Exp*e=_tmp1D;struct Cyc_Absyn_Stmt*s1=_tmp1E;struct Cyc_Absyn_Stmt*s2=_tmp1F;
# 147
return({struct Cyc_Hashtable_Table*_tmp41=fds;struct Cyc_Absyn_Stmt*_tmp40=s2;Cyc_Callgraph_cg_stmt(_tmp41,_tmp40,({struct Cyc_Hashtable_Table*_tmp3F=fds;struct Cyc_Absyn_Stmt*_tmp3E=s1;Cyc_Callgraph_cg_stmt(_tmp3F,_tmp3E,Cyc_Callgraph_cg_exp(fds,e,s));}));});}case 5: _tmp1D=(((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1).f1;_tmp1E=((struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp1C)->f2;{struct Cyc_Absyn_Exp*e=_tmp1D;struct Cyc_Absyn_Stmt*s1=_tmp1E;
return({struct Cyc_Hashtable_Table*_tmp43=fds;struct Cyc_Absyn_Stmt*_tmp42=s1;Cyc_Callgraph_cg_stmt(_tmp43,_tmp42,Cyc_Callgraph_cg_exp(fds,e,s));});}case 9: _tmp1D=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;_tmp1E=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp1C)->f2).f1;_tmp1F=(((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp1C)->f3).f1;_tmp20=((struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp1C)->f4;{struct Cyc_Absyn_Exp*e1=_tmp1D;struct Cyc_Absyn_Exp*e2=_tmp1E;struct Cyc_Absyn_Exp*e3=_tmp1F;struct Cyc_Absyn_Stmt*s1=_tmp20;
# 150
return({struct Cyc_Hashtable_Table*_tmp49=fds;struct Cyc_Absyn_Stmt*_tmp48=s1;Cyc_Callgraph_cg_stmt(_tmp49,_tmp48,({struct Cyc_Hashtable_Table*_tmp47=fds;struct Cyc_Absyn_Exp*_tmp46=e3;Cyc_Callgraph_cg_exp(_tmp47,_tmp46,({struct Cyc_Hashtable_Table*_tmp45=fds;struct Cyc_Absyn_Exp*_tmp44=e2;Cyc_Callgraph_cg_exp(_tmp45,_tmp44,Cyc_Callgraph_cg_exp(fds,e1,s));}));}));});}case 10: _tmp1D=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;_tmp1E=((struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp1C)->f2;{struct Cyc_Absyn_Exp*e=_tmp1D;struct Cyc_List_List*scs=_tmp1E;
# 152
s=Cyc_Callgraph_cg_exp(fds,e,s);
for(1;scs != 0;scs=scs->tl){
# 155
s=Cyc_Callgraph_cg_stmt(fds,((struct Cyc_Absyn_Switch_clause*)scs->hd)->body,s);}
return s;}case 11: _tmp1D=((struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;{struct Cyc_List_List*es=_tmp1D;
return Cyc_Callgraph_cg_exps(fds,es,s);}case 12: _tmp1D=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;_tmp1E=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp1C)->f2;{struct Cyc_Absyn_Decl*d=_tmp1D;struct Cyc_Absyn_Stmt*s1=_tmp1E;
return({struct Cyc_Hashtable_Table*_tmp4B=fds;struct Cyc_Absyn_Stmt*_tmp4A=s1;Cyc_Callgraph_cg_stmt(_tmp4B,_tmp4A,Cyc_Callgraph_cg_decl(fds,d,s));});}case 14: _tmp1D=((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;_tmp1E=(((struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp1C)->f2).f1;{struct Cyc_Absyn_Stmt*s1=_tmp1D;struct Cyc_Absyn_Exp*e=_tmp1E;
return({struct Cyc_Hashtable_Table*_tmp4D=fds;struct Cyc_Absyn_Exp*_tmp4C=e;Cyc_Callgraph_cg_exp(_tmp4D,_tmp4C,Cyc_Callgraph_cg_stmt(fds,s1,s));});}case 15: _tmp1D=((struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct*)_tmp1C)->f1;_tmp1E=((struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct*)_tmp1C)->f2;{struct Cyc_Absyn_Stmt*s1=_tmp1D;struct Cyc_List_List*scs=_tmp1E;
# 161
s=Cyc_Callgraph_cg_stmt(fds,s1,s);
for(1;scs != 0;scs=scs->tl){
# 164
s=Cyc_Callgraph_cg_stmt(fds,((struct Cyc_Absyn_Switch_clause*)scs->hd)->body,s);}
return s;}default: _tmp1D=((struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct*)_tmp1C)->f2;{struct Cyc_Absyn_Stmt*s1=_tmp1D;
return Cyc_Callgraph_cg_stmt(fds,s1,s);}};}
# 170
static int Cyc_Callgraph_fndecl_cmp(struct Cyc_Absyn_Fndecl*fd1,struct Cyc_Absyn_Fndecl*fd2){return(int)fd1 - (int)fd2;}
# 172
static struct Cyc_Dict_Dict Cyc_Callgraph_cg_topdecls(struct Cyc_Hashtable_Table*fds,struct Cyc_Dict_Dict cg,struct Cyc_List_List*ds){
struct Cyc_Set_Set*_tmp21=({(struct Cyc_Set_Set*(*)(int(*)(struct Cyc_Absyn_Fndecl*,struct Cyc_Absyn_Fndecl*)))Cyc_Set_empty;})(Cyc_Callgraph_fndecl_cmp);struct Cyc_Set_Set*mt=_tmp21;
{struct Cyc_List_List*_tmp22=ds;struct Cyc_List_List*ds=_tmp22;for(0;ds != 0;ds=ds->tl){
void*_tmp23=((struct Cyc_Absyn_Decl*)ds->hd)->r;void*_stmttmp4=_tmp23;void*_tmp24=_stmttmp4;void*_tmp25;switch(*((int*)_tmp24)){case 1: _tmp25=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp24)->f1;{struct Cyc_Absyn_Fndecl*fd=_tmp25;
# 177
cg=({struct Cyc_Dict_Dict(*_tmp50)(struct Cyc_Dict_Dict,struct Cyc_Absyn_Fndecl*,struct Cyc_Set_Set*)=({(struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict,struct Cyc_Absyn_Fndecl*,struct Cyc_Set_Set*))Cyc_Graph_add_edges;});struct Cyc_Dict_Dict _tmp4F=cg;struct Cyc_Absyn_Fndecl*_tmp4E=fd;_tmp50(_tmp4F,_tmp4E,Cyc_Callgraph_cg_stmt(fds,fd->body,mt));});
goto _LL0;}case 10: _tmp25=((struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct*)_tmp24)->f2;{struct Cyc_List_List*ds=_tmp25;
_tmp25=ds;goto _LL6;}case 9: _tmp25=((struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct*)_tmp24)->f2;_LL6: {struct Cyc_List_List*ds=_tmp25;
cg=Cyc_Callgraph_cg_topdecls(fds,cg,ds);goto _LL0;}default:
 goto _LL0;}_LL0:;}}
# 184
return cg;}
# 188
static struct Cyc_Dict_Dict Cyc_Callgraph_enter_fndecls(struct Cyc_Hashtable_Table*fds,struct Cyc_Dict_Dict cg,struct Cyc_List_List*ds){
# 190
{struct Cyc_List_List*_tmp26=ds;struct Cyc_List_List*ds=_tmp26;for(0;ds != 0;ds=ds->tl){
void*_tmp27=((struct Cyc_Absyn_Decl*)ds->hd)->r;void*_stmttmp5=_tmp27;void*_tmp28=_stmttmp5;void*_tmp29;switch(*((int*)_tmp28)){case 1: _tmp29=((struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp28)->f1;{struct Cyc_Absyn_Fndecl*f=_tmp29;
# 193
({(void(*)(struct Cyc_Hashtable_Table*,struct _tuple0*,struct Cyc_Absyn_Fndecl*))Cyc_Hashtable_insert;})(fds,f->name,f);
cg=({(struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict,struct Cyc_Absyn_Fndecl*))Cyc_Graph_add_node;})(cg,f);
goto _LL0;}case 10: _tmp29=((struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct*)_tmp28)->f2;{struct Cyc_List_List*ds=_tmp29;
_tmp29=ds;goto _LL6;}case 9: _tmp29=((struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct*)_tmp28)->f2;_LL6: {struct Cyc_List_List*ds=_tmp29;
# 198
cg=Cyc_Callgraph_enter_fndecls(fds,cg,ds);goto _LL0;}default:
 goto _LL0;}_LL0:;}}
# 202
return cg;}
# 205
static int Cyc_Callgraph_hash_fndecl(struct Cyc_Absyn_Fndecl*fd){return(int)fd;}
# 207
struct Cyc_Dict_Dict Cyc_Callgraph_compute_callgraph(struct Cyc_List_List*ds){
# 209
struct Cyc_Hashtable_Table*fd=({(struct Cyc_Hashtable_Table*(*)(int,int(*)(struct _tuple0*,struct _tuple0*),int(*)(struct _tuple0*)))Cyc_Hashtable_create;})(51,Cyc_Absyn_qvar_cmp,Cyc_Absyn_hash_qvar);
struct Cyc_Dict_Dict cg=({(struct Cyc_Dict_Dict(*)(int(*)(struct Cyc_Absyn_Fndecl*,struct Cyc_Absyn_Fndecl*)))Cyc_Graph_empty;})(Cyc_Callgraph_fndecl_cmp);
cg=Cyc_Callgraph_enter_fndecls(fd,cg,ds);
# 213
cg=Cyc_Callgraph_cg_topdecls(fd,cg,ds);
return cg;}
# 217
static void Cyc_Callgraph_print_fndecl(struct Cyc___cycFILE*f,struct Cyc_Absyn_Fndecl*fd){
({struct Cyc_String_pa_PrintArg_struct _tmp2C=({struct Cyc_String_pa_PrintArg_struct _tmp2D;_tmp2D.tag=0,({struct _fat_ptr _tmp51=(struct _fat_ptr)((struct _fat_ptr)Cyc_Absynpp_qvar2string(fd->name));_tmp2D.f1=_tmp51;});_tmp2D;});void*_tmp2A[1];_tmp2A[0]=& _tmp2C;({struct Cyc___cycFILE*_tmp53=f;struct _fat_ptr _tmp52=({const char*_tmp2B="%s ";_tag_fat(_tmp2B,sizeof(char),4U);});Cyc_fprintf(_tmp53,_tmp52,_tag_fat(_tmp2A,sizeof(void*),1));});});}
# 221
void Cyc_Callgraph_print_callgraph(struct Cyc___cycFILE*f,struct Cyc_Dict_Dict cg){
({(void(*)(struct Cyc___cycFILE*,struct Cyc_Dict_Dict,void(*)(struct Cyc___cycFILE*,struct Cyc_Absyn_Fndecl*)))Cyc_Graph_print;})(f,cg,Cyc_Callgraph_print_fndecl);}
