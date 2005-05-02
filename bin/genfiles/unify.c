#include <setjmp.h>
/* This is a C header used by the output of the Cyclone to
   C translator.  Corresponding definitions are in file lib/runtime_*.c */
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
/* should be size_t but int is fine */
#define offsetof(t,n) ((int)(&(((t*)0)->n)))
#endif

/* Fat pointers */
struct _fat_ptr {
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};  

/* Regions */
struct _RegionPage
{ 
#ifdef CYC_REGION_PROFILE
  unsigned total_bytes;
  unsigned free_bytes;
#endif
  struct _RegionPage *next;
  char data[1];
};

struct _pool;
struct bget_region_key;
struct _RegionAllocFunctions;

struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
#if(defined(__linux__) && defined(__KERNEL__))
  struct _RegionPage *vpage;
#endif 
  struct _RegionAllocFunctions *fcns;
  char               *offset;
  char               *last_plus_one;
  struct _pool *released_ptrs;
  struct bget_region_key *key;
#ifdef CYC_REGION_PROFILE
  const char *name;
#endif
  unsigned used_bytes;
  unsigned wasted_bytes;
};


// A dynamic region is just a region handle.  The wrapper struct is for type
// abstraction.
struct Cyc_Core_DynamicRegion {
  struct _RegionHandle h;
};

/* Alias qualifier stuff */
typedef unsigned int _AliasQualHandle_t; // must match aqualt_type() in toc.cyc

struct _RegionHandle _new_region(unsigned int, const char*);
void* _region_malloc(struct _RegionHandle*, _AliasQualHandle_t, unsigned);
void* _region_calloc(struct _RegionHandle*, _AliasQualHandle_t, unsigned t, unsigned n);
void* _region_vmalloc(struct _RegionHandle*, unsigned);
void * _aqual_malloc(_AliasQualHandle_t aq, unsigned int s);
void * _aqual_calloc(_AliasQualHandle_t aq, unsigned int n, unsigned int t);
void _free_region(struct _RegionHandle*);

/* Exceptions */
struct _handler_cons {
  struct _RuntimeStack s;
  jmp_buf handler;
};
void _push_handler(struct _handler_cons*);
void _push_region(struct _RegionHandle*);
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
#define _zero_arr_plus_char_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_other_fn(t_sz,orig_x,orig_sz,orig_i,f,l)((orig_x)+(orig_i))
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
void* _zero_arr_plus_other_fn(unsigned,void*,unsigned,int,const char*,unsigned);
#endif

/* _get_zero_arr_size_*(x,sz) returns the number of elements in a
   zero-terminated array that is NULL or has at least sz elements */
unsigned _get_zero_arr_size_char(const char*,unsigned);
unsigned _get_zero_arr_size_other(unsigned,const void*,unsigned);

/* _zero_arr_inplace_plus_*_fn(x,i,filename,lineno) sets
   zero-terminated pointer *x to *x + i */
char* _zero_arr_inplace_plus_char_fn(char**,int,const char*,unsigned);
char* _zero_arr_inplace_plus_post_char_fn(char**,int,const char*,unsigned);
// note: must cast result in toc.cyc
void* _zero_arr_inplace_plus_other_fn(unsigned,void**,int,const char*,unsigned);
void* _zero_arr_inplace_plus_post_other_fn(unsigned,void**,int,const char*,unsigned);
#define _zero_arr_plus_char(x,s,i) \
  (_zero_arr_plus_char_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_inplace_plus_char(x,i) \
  _zero_arr_inplace_plus_char_fn((char**)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_char(x,i) \
  _zero_arr_inplace_plus_post_char_fn((char**)(x),(i),__FILE__,__LINE__)
#define _zero_arr_plus_other(t,x,s,i) \
  (_zero_arr_plus_other_fn(t,x,s,i,__FILE__,__LINE__))
#define _zero_arr_inplace_plus_other(t,x,i) \
  _zero_arr_inplace_plus_other_fn(t,(void**)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_other(t,x,i) \
  _zero_arr_inplace_plus_post_other_fn(t,(void**)(x),(i),__FILE__,__LINE__)

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_fat_subscript(arr,elt_sz,index) ((arr).curr + (elt_sz) * (index))
#define _untag_fat_ptr(arr,elt_sz,num_elts) ((arr).curr)
#define _check_fat_at_base(arr) (arr)
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
      _curr != (unsigned char*)0) \
    _throw_arraybounds(); \
  _curr; })
#define _check_fat_at_base(arr) ({ \
  struct _fat_ptr _arr = (arr); \
  if (_arr.base != _arr.curr) _throw_arraybounds(); \
  _arr; })
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

#ifdef CYC_GC_PTHREAD_REDIRECTS
# define pthread_create GC_pthread_create
# define pthread_sigmask GC_pthread_sigmask
# define pthread_join GC_pthread_join
# define pthread_detach GC_pthread_detach
# define dlopen GC_dlopen
#endif
/* Allocation */
void* GC_malloc(int);
void* GC_malloc_atomic(int);
void* GC_calloc(unsigned,unsigned);
void* GC_calloc_atomic(unsigned,unsigned);

#if(defined(__linux__) && defined(__KERNEL__))
void *cyc_vmalloc(unsigned);
void cyc_vfree(void*);
#endif
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

#define _CYC_MAX_REGION_CONST 0
#define _CYC_MIN_ALIGNMENT (sizeof(double))

#ifdef CYC_REGION_PROFILE
extern int rgn_total_bytes;
#endif

static inline void*_fast_region_malloc(struct _RegionHandle*r, _AliasQualHandle_t aq, unsigned orig_s) {  
  if (r > (struct _RegionHandle*)_CYC_MAX_REGION_CONST && r->curr != 0) { 
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
  return _region_malloc(r,aq,orig_s); 
}

//doesn't make sense to fast malloc with reaps
#ifndef DISABLE_REAPS
#define _fast_region_malloc _region_malloc
#endif

#ifdef CYC_REGION_PROFILE
/* see macros below for usage. defined in runtime_memory.c */
void* _profile_GC_malloc(int,const char*,const char*,int);
void* _profile_GC_malloc_atomic(int,const char*,const char*,int);
void* _profile_GC_calloc(unsigned,unsigned,const char*,const char*,int);
void* _profile_GC_calloc_atomic(unsigned,unsigned,const char*,const char*,int);
void* _profile_region_malloc(struct _RegionHandle*,_AliasQualHandle_t,unsigned,const char*,const char*,int);
void* _profile_region_calloc(struct _RegionHandle*,_AliasQualHandle_t,unsigned,unsigned,const char *,const char*,int);
void * _profile_aqual_malloc(_AliasQualHandle_t aq, unsigned int s,const char *file, const char *func, int lineno);
void * _profile_aqual_calloc(_AliasQualHandle_t aq, unsigned int t1,unsigned int t2,const char *file, const char *func, int lineno);
struct _RegionHandle _profile_new_region(unsigned int i, const char*,const char*,const char*,int);
void _profile_free_region(struct _RegionHandle*,const char*,const char*,int);
#ifndef RUNTIME_CYC
#define _new_region(i,n) _profile_new_region(i,n,__FILE__,__FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__,__FUNCTION__,__LINE__)
#define _region_malloc(rh,aq,n) _profile_region_malloc(rh,aq,n,__FILE__,__FUNCTION__,__LINE__)
#define _region_calloc(rh,aq,n,t) _profile_region_calloc(rh,aq,n,t,__FILE__,__FUNCTION__,__LINE__)
#define _aqual_malloc(aq,n) _profile_aqual_malloc(aq,n,__FILE__,__FUNCTION__,__LINE__)
#define _aqual_calloc(aq,n,t) _profile_aqual_calloc(aq,n,t,__FILE__,__FUNCTION__,__LINE__)
#endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__,__FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__,__FUNCTION__,__LINE__)
#define _cyccalloc(n,s) _profile_GC_calloc(n,s,__FILE__,__FUNCTION__,__LINE__)
#define _cyccalloc_atomic(n,s) _profile_GC_calloc_atomic(n,s,__FILE__,__FUNCTION__,__LINE__)
#endif //CYC_REGION_PROFILE
#endif //_CYC_INCLUDE_H
 struct Cyc_Core_Opt{void*v;};
# 170 "core.h"
extern struct _RegionHandle*Cyc_Core_heap_region;struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 190 "list.h"
extern struct Cyc_List_List*Cyc_List_rappend(struct _RegionHandle*,struct Cyc_List_List*,struct Cyc_List_List*);
# 322
extern int Cyc_List_mem(int(*)(void*,void*),struct Cyc_List_List*,void*);struct Cyc___cycFILE;
# 53 "cycboot.h"
extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_String_pa_PrintArg_struct{int tag;struct _fat_ptr f1;};
# 73
extern struct _fat_ptr Cyc_aprintf(struct _fat_ptr,struct _fat_ptr);
# 88
extern int Cyc_fflush(struct Cyc___cycFILE*);
# 100
extern int Cyc_fprintf(struct Cyc___cycFILE*,struct _fat_ptr,struct _fat_ptr);
# 38 "string.h"
extern unsigned long Cyc_strlen(struct _fat_ptr);
# 49 "string.h"
extern int Cyc_strcmp(struct _fat_ptr,struct _fat_ptr);
extern int Cyc_strptrcmp(struct _fat_ptr*,struct _fat_ptr*);
# 46 "position.h"
extern int Cyc_Position_num_errors;
extern int Cyc_Position_max_errors;struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};struct _tuple0{union Cyc_Absyn_Nmspace f0;struct _fat_ptr*f1;};
# 135 "absyn.h"
enum Cyc_Absyn_Scope{Cyc_Absyn_Static =0U,Cyc_Absyn_Abstract =1U,Cyc_Absyn_Public =2U,Cyc_Absyn_Extern =3U,Cyc_Absyn_ExternC =4U,Cyc_Absyn_Register =5U};struct Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;unsigned loc;};
# 158
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA =0U,Cyc_Absyn_UnionA =1U};
# 160
enum Cyc_Absyn_AliasQualVal{Cyc_Absyn_Aliasable_qual =0U,Cyc_Absyn_Unique_qual =1U,Cyc_Absyn_Refcnt_qual =2U,Cyc_Absyn_Restricted_qual =3U};
# 176 "absyn.h"
enum Cyc_Absyn_AliasHint{Cyc_Absyn_UniqueHint =0U,Cyc_Absyn_RefcntHint =1U,Cyc_Absyn_RestrictedHint =2U,Cyc_Absyn_NoHint =3U};
# 182
enum Cyc_Absyn_KindQual{Cyc_Absyn_AnyKind =0U,Cyc_Absyn_MemKind =1U,Cyc_Absyn_BoxKind =2U,Cyc_Absyn_RgnKind =3U,Cyc_Absyn_EffKind =4U,Cyc_Absyn_IntKind =5U,Cyc_Absyn_BoolKind =6U,Cyc_Absyn_PtrBndKind =7U,Cyc_Absyn_AqualKind =8U};struct Cyc_Absyn_Kind{enum Cyc_Absyn_KindQual kind;enum Cyc_Absyn_AliasHint aliashint;};struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct{int tag;struct Cyc_Absyn_Kind*f1;};struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{struct _fat_ptr*name;int identity;void*kind;void*aquals_bound;};struct Cyc_Absyn_PtrLoc{unsigned ptr_loc;unsigned rgn_loc;unsigned zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;void*nullable;void*bounds;void*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;void*autoreleased;void*aqual;};struct Cyc_Absyn_PtrInfo{void*elt_type;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{struct _fat_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*qual_bnd;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;struct Cyc_Absyn_Vardecl*return_value;struct Cyc_List_List*arg_vardecls;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;void*zero_term;unsigned zt_loc;};struct Cyc_Absyn_AqualConstCon_Absyn_TyCon_struct{int tag;enum Cyc_Absyn_AliasQualVal f1;};struct Cyc_Absyn_AppType_Absyn_Type_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Evar_Absyn_Type_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;int f2;struct Cyc_List_List*f3;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct _tuple8{struct _fat_ptr*f0;struct Cyc_Absyn_Tqual f1;void*f2;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned loc;void*annot;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;unsigned varloc;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;int is_proto;};struct Cyc_Absyn_Aggrfield{struct _fat_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;};struct Cyc_Absyn_Typedefdecl{struct _tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;int extern_c;};
# 886 "absyn.h"
void*Cyc_Absyn_compress(void*);
# 921
extern void*Cyc_Absyn_var_type(struct Cyc_Absyn_Tvar*);
# 956
void*Cyc_Absyn_bounds_one (void);struct Cyc_Warn_String_Warn_Warg_struct{int tag;struct _fat_ptr f1;};
# 71 "warn.h"
void*Cyc_Warn_impos2(struct _fat_ptr);struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;};
# 53 "absynpp.h"
void Cyc_Absynpp_set_params(struct Cyc_Absynpp_Params*);
# 55
extern struct Cyc_Absynpp_Params Cyc_Absynpp_tc_params_r;
# 62
struct _fat_ptr Cyc_Absynpp_typ2string(void*);
# 75
struct _fat_ptr Cyc_Absynpp_tvar2string(struct Cyc_Absyn_Tvar*);
# 41 "evexp.h"
extern int Cyc_Evexp_same_uint_const_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*);
# 131 "relations-ap.h"
int Cyc_Relations_check_logical_implication(struct Cyc_List_List*,struct Cyc_List_List*);
# 98 "tcutil.h"
struct Cyc_Absyn_Kind*Cyc_Tcutil_type_kind(void*);
# 125
int Cyc_Tcutil_typecmp(void*,void*);
# 131
void*Cyc_Tcutil_rsubstitute(struct _RegionHandle*,struct Cyc_List_List*,void*);
# 140
int Cyc_Tcutil_subset_effect(int,void*,void*);
# 199
void*Cyc_Tcutil_normalize_effect(void*);
# 248
int Cyc_Tcutil_fast_tvar_cmp(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*);
# 252
int Cyc_Tcutil_same_rgn_po(struct Cyc_List_List*,struct Cyc_List_List*);
int Cyc_Tcutil_tycon_cmp(void*,void*);
# 30 "kinds.h"
extern struct Cyc_Absyn_Kind Cyc_Kinds_bk;
# 36
extern struct Cyc_Absyn_Kind Cyc_Kinds_aqk;
# 81 "kinds.h"
struct _fat_ptr Cyc_Kinds_kind2string(struct Cyc_Absyn_Kind*);
# 83
struct Cyc_Absyn_Kind*Cyc_Kinds_tvar_kind(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Kind*);
# 89
void*Cyc_Kinds_compress_kb(void*);
# 93
int Cyc_Kinds_kind_leq(struct Cyc_Absyn_Kind*,struct Cyc_Absyn_Kind*);
int Cyc_Kinds_kind_eq(struct Cyc_Absyn_Kind*,struct Cyc_Absyn_Kind*);
# 54 "attributes.h"
int Cyc_Atts_same_atts(struct Cyc_List_List*,struct Cyc_List_List*);
# 58
int Cyc_Atts_equiv_fn_atts(struct Cyc_List_List*,struct Cyc_List_List*);char Cyc_Unify_Unify[6U]="Unify";struct Cyc_Unify_Unify_exn_struct{char*tag;};
# 35 "unify.cyc"
struct Cyc_Unify_Unify_exn_struct Cyc_Unify_Unify_val={Cyc_Unify_Unify};struct _tuple11{void*f0;void*f1;};
# 38
static struct _tuple11 Cyc_Unify_ts_failure={.f0=0,.f1=0};struct _tuple12{int f0;int f1;};
static struct _tuple12 Cyc_Unify_tqs_const={.f0=0,.f1=0};
static const char*Cyc_Unify_failure_reason=0;
# 42
static void Cyc_Unify_fail_because(const char*reason){
Cyc_Unify_failure_reason=reason;
_throw(& Cyc_Unify_Unify_val);}
# 50
void Cyc_Unify_explain_failure (void){
if(Cyc_Position_num_errors >= Cyc_Position_max_errors)
return;
Cyc_fflush(Cyc_stderr);
# 56
if(Cyc_strcmp(_tag_fat("qualifiers don't match",sizeof(char),23U),({const char*_Tmp0=Cyc_Unify_failure_reason;_tag_fat((void*)_Tmp0,sizeof(char),_get_zero_arr_size_char((void*)_Tmp0,1U));}))==0){
({struct Cyc_String_pa_PrintArg_struct _Tmp0=({struct Cyc_String_pa_PrintArg_struct _Tmp1;_Tmp1.tag=0,_Tmp1.f1=(struct _fat_ptr)({const char*_Tmp2=Cyc_Unify_failure_reason;_tag_fat((void*)_Tmp2,sizeof(char),_get_zero_arr_size_char((void*)_Tmp2,1U));});_Tmp1;});void*_Tmp1[1];_Tmp1[0]=& _Tmp0;Cyc_fprintf(Cyc_stderr,_tag_fat("  (%s)\n",sizeof(char),8U),_tag_fat(_Tmp1,sizeof(void*),1));});
return;}
# 61
if(Cyc_strcmp(_tag_fat("function effects do not match",sizeof(char),30U),({const char*_Tmp0=Cyc_Unify_failure_reason;_tag_fat((void*)_Tmp0,sizeof(char),_get_zero_arr_size_char((void*)_Tmp0,1U));}))==0){
struct Cyc_Absynpp_Params p=Cyc_Absynpp_tc_params_r;
p.print_all_effects=1;
Cyc_Absynpp_set_params(& p);}{
# 66
void*_Tmp0;void*_Tmp1;_Tmp1=Cyc_Unify_ts_failure.f0;_Tmp0=Cyc_Unify_ts_failure.f1;{void*t1f=_Tmp1;void*t2f=_Tmp0;
struct _fat_ptr s1=(unsigned)t1f?Cyc_Absynpp_typ2string(t1f): _tag_fat("<?>",sizeof(char),4U);
struct _fat_ptr s2=(unsigned)t2f?Cyc_Absynpp_typ2string(t2f): _tag_fat("<?>",sizeof(char),4U);
int pos=2;
({struct Cyc_String_pa_PrintArg_struct _Tmp2=({struct Cyc_String_pa_PrintArg_struct _Tmp3;_Tmp3.tag=0,_Tmp3.f1=(struct _fat_ptr)((struct _fat_ptr)s1);_Tmp3;});void*_Tmp3[1];_Tmp3[0]=& _Tmp2;Cyc_fprintf(Cyc_stderr,_tag_fat("  %s",sizeof(char),5U),_tag_fat(_Tmp3,sizeof(void*),1));});
pos +=_get_fat_size(s1,sizeof(char));
if(pos + 5 >= 80){
Cyc_fprintf(Cyc_stderr,_tag_fat("\n\t",sizeof(char),3U),_tag_fat(0U,sizeof(void*),0));
pos=8;}else{
# 76
Cyc_fprintf(Cyc_stderr,_tag_fat(" ",sizeof(char),2U),_tag_fat(0U,sizeof(void*),0));
++ pos;}
# 79
Cyc_fprintf(Cyc_stderr,_tag_fat("and ",sizeof(char),5U),_tag_fat(0U,sizeof(void*),0));
pos +=4;
if((unsigned)pos + _get_fat_size(s2,sizeof(char))>= 80U){
Cyc_fprintf(Cyc_stderr,_tag_fat("\n\t",sizeof(char),3U),_tag_fat(0U,sizeof(void*),0));
pos=8;}
# 85
({struct Cyc_String_pa_PrintArg_struct _Tmp2=({struct Cyc_String_pa_PrintArg_struct _Tmp3;_Tmp3.tag=0,_Tmp3.f1=(struct _fat_ptr)((struct _fat_ptr)s2);_Tmp3;});void*_Tmp3[1];_Tmp3[0]=& _Tmp2;Cyc_fprintf(Cyc_stderr,_tag_fat("%s ",sizeof(char),4U),_tag_fat(_Tmp3,sizeof(void*),1));});
pos +=_get_fat_size(s2,sizeof(char))+ 1U;
if(pos + 17 >= 80){
Cyc_fprintf(Cyc_stderr,_tag_fat("\n\t",sizeof(char),3U),_tag_fat(0U,sizeof(void*),0));
pos=8;}
# 91
Cyc_fprintf(Cyc_stderr,_tag_fat("are not compatible. ",sizeof(char),21U),_tag_fat(0U,sizeof(void*),0));
pos +=17;
if(Cyc_Unify_failure_reason!=(const char*)0){
if(({unsigned long _Tmp2=(unsigned long)pos;_Tmp2 + Cyc_strlen(({const char*_Tmp3=Cyc_Unify_failure_reason;_tag_fat((void*)_Tmp3,sizeof(char),_get_zero_arr_size_char((void*)_Tmp3,1U));}));})>= 80U)
Cyc_fprintf(Cyc_stderr,_tag_fat("\n\t",sizeof(char),3U),_tag_fat(0U,sizeof(void*),0));
({struct Cyc_String_pa_PrintArg_struct _Tmp2=({struct Cyc_String_pa_PrintArg_struct _Tmp3;_Tmp3.tag=0,_Tmp3.f1=(struct _fat_ptr)({const char*_Tmp4=Cyc_Unify_failure_reason;_tag_fat((void*)_Tmp4,sizeof(char),_get_zero_arr_size_char((void*)_Tmp4,1U));});_Tmp3;});void*_Tmp3[1];_Tmp3[0]=& _Tmp2;Cyc_fprintf(Cyc_stderr,_tag_fat("(%s)",sizeof(char),5U),_tag_fat(_Tmp3,sizeof(void*),1));});}
# 98
Cyc_fprintf(Cyc_stderr,_tag_fat("\n",sizeof(char),2U),_tag_fat(0U,sizeof(void*),0));
Cyc_fflush(Cyc_stderr);}}}
# 104
static int Cyc_Unify_check_logical_equivalence(struct Cyc_List_List*r1,struct Cyc_List_List*r2){
# 106
if(r1==r2)return 1;
return Cyc_Relations_check_logical_implication(r1,r2)&&
 Cyc_Relations_check_logical_implication(r2,r1);}
# 111
int Cyc_Unify_unify_kindbound(void*kb1,void*kb2){
struct _tuple11 _Tmp0=({struct _tuple11 _Tmp1;({void*_Tmp2=Cyc_Kinds_compress_kb(kb1);_Tmp1.f0=_Tmp2;}),({void*_Tmp2=Cyc_Kinds_compress_kb(kb2);_Tmp1.f1=_Tmp2;});_Tmp1;});void*_Tmp1;void*_Tmp2;void*_Tmp3;void*_Tmp4;switch(*((int*)_Tmp0.f0)){case 0: switch(*((int*)_Tmp0.f1)){case 0: _Tmp4=((struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct*)_Tmp0.f0)->f1;_Tmp3=((struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct*)_Tmp0.f1)->f1;{struct Cyc_Absyn_Kind*k1=_Tmp4;struct Cyc_Absyn_Kind*k2=_Tmp3;
return k1==k2;}case 2: _Tmp4=((struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct*)_Tmp0.f0)->f1;_Tmp3=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct*)_Tmp0.f1)->f1;_Tmp2=((struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct*)_Tmp0.f1)->f2;_LL8: {struct Cyc_Absyn_Kind*k1=_Tmp4;struct Cyc_Core_Opt**x=_Tmp3;struct Cyc_Absyn_Kind*k2=_Tmp2;
# 122
if(!Cyc_Kinds_kind_leq(k1,k2))
return 0;
({struct Cyc_Core_Opt*_Tmp5=({struct Cyc_Core_Opt*_Tmp6=_cycalloc(sizeof(struct Cyc_Core_Opt));_Tmp6->v=kb1;_Tmp6;});*x=_Tmp5;});
return 1;}default: goto _LLB;}case 2: switch(*((int*)_Tmp0.f1)){case 0:
# 114
 return Cyc_Unify_unify_kindbound(kb2,kb1);case 2: _Tmp4=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct*)_Tmp0.f0)->f1;_Tmp3=((struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct*)_Tmp0.f0)->f2;_Tmp2=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct*)_Tmp0.f1)->f1;_Tmp1=((struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct*)_Tmp0.f1)->f2;{struct Cyc_Core_Opt**y=_Tmp4;struct Cyc_Absyn_Kind*k1=_Tmp3;struct Cyc_Core_Opt**x=(struct Cyc_Core_Opt**)_Tmp2;struct Cyc_Absyn_Kind*k2=_Tmp1;
# 116
if(Cyc_Kinds_kind_leq(k2,k1)){
({struct Cyc_Core_Opt*_Tmp5=({struct Cyc_Core_Opt*_Tmp6=_cycalloc(sizeof(struct Cyc_Core_Opt));_Tmp6->v=kb2;_Tmp6;});*y=_Tmp5;});
return 1;}
# 120
_Tmp4=k1;_Tmp3=x;_Tmp2=k2;goto _LL8;}default: _LLB: _Tmp4=_Tmp0.f0;_Tmp3=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct*)_Tmp0.f1)->f1;_LLC: {void*y=_Tmp4;struct Cyc_Core_Opt**x=_Tmp3;
# 128
({struct Cyc_Core_Opt*_Tmp5=({struct Cyc_Core_Opt*_Tmp6=_cycalloc(sizeof(struct Cyc_Core_Opt));_Tmp6->v=y;_Tmp6;});*x=_Tmp5;});
return 1;}}default: _Tmp4=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct*)_Tmp0.f0)->f1;_Tmp3=_Tmp0.f1;{struct Cyc_Core_Opt**x=(struct Cyc_Core_Opt**)_Tmp4;void*y=_Tmp3;
# 126
_Tmp4=y;_Tmp3=x;goto _LLC;}};}
# 135
void Cyc_Unify_occurs(void*evar,struct _RegionHandle*r,struct Cyc_List_List*env,void*t){
t=Cyc_Absyn_compress(t);{
void*_Tmp0;void*_Tmp1;void*_Tmp2;void*_Tmp3;struct Cyc_Absyn_Tqual _Tmp4;struct Cyc_Absyn_PtrInfo _Tmp5;void*_Tmp6;void*_Tmp7;switch(*((int*)t)){case 2: _Tmp7=((struct Cyc_Absyn_VarType_Absyn_Type_struct*)t)->f1;{struct Cyc_Absyn_Tvar*tv=_Tmp7;
# 139
if(!({(int(*)(int(*)(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*),struct Cyc_List_List*,struct Cyc_Absyn_Tvar*))Cyc_List_mem;})(Cyc_Tcutil_fast_tvar_cmp,env,tv))
Cyc_Unify_fail_because("type variable would escape scope");
goto _LL0;}case 1: _Tmp7=(void*)((struct Cyc_Absyn_Evar_Absyn_Type_struct*)t)->f2;_Tmp6=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Evar_Absyn_Type_struct*)t)->f4;{void*rg=_Tmp7;struct Cyc_Core_Opt**sopt=_Tmp6;
# 143
if(t==evar)
Cyc_Unify_fail_because("occurs check");
if(rg!=0)
({struct Cyc_Warn_String_Warn_Warg_struct _Tmp8=({struct Cyc_Warn_String_Warn_Warg_struct _Tmp9;_Tmp9.tag=0,_Tmp9.f1=_tag_fat("occurs check: constrained Evar in compressed type",sizeof(char),50U);_Tmp9;});void*_Tmp9[1];_Tmp9[0]=& _Tmp8;({(int(*)(struct _fat_ptr))Cyc_Warn_impos2;})(_tag_fat(_Tmp9,sizeof(void*),1));});{
# 148
int problem=0;
{struct Cyc_List_List*s=(struct Cyc_List_List*)_check_null(*sopt)->v;for(0;s!=0 && !problem;s=s->tl){
if(!({(int(*)(int(*)(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*),struct Cyc_List_List*,struct Cyc_Absyn_Tvar*))Cyc_List_mem;})(Cyc_Tcutil_fast_tvar_cmp,env,(struct Cyc_Absyn_Tvar*)s->hd))
problem=1;}}
# 153
if(problem){
struct Cyc_List_List*result=0;
{struct Cyc_List_List*s=(struct Cyc_List_List*)_check_null(*sopt)->v;for(0;s!=0;s=s->tl){
if(({(int(*)(int(*)(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*),struct Cyc_List_List*,struct Cyc_Absyn_Tvar*))Cyc_List_mem;})(Cyc_Tcutil_fast_tvar_cmp,env,(struct Cyc_Absyn_Tvar*)s->hd))
result=({struct Cyc_List_List*_Tmp8=_cycalloc(sizeof(struct Cyc_List_List));_Tmp8->hd=(struct Cyc_Absyn_Tvar*)s->hd,_Tmp8->tl=result;_Tmp8;});}}
({struct Cyc_Core_Opt*_Tmp8=({struct Cyc_Core_Opt*_Tmp9=_cycalloc(sizeof(struct Cyc_Core_Opt));_Tmp9->v=result;_Tmp9;});*sopt=_Tmp8;});}
# 160
goto _LL0;}}case 3: _Tmp5=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)t)->f1;{struct Cyc_Absyn_PtrInfo pinfo=_Tmp5;
# 162
Cyc_Unify_occurs(evar,r,env,pinfo.elt_type);
Cyc_Unify_occurs(evar,r,env,pinfo.ptr_atts.rgn);
Cyc_Unify_occurs(evar,r,env,pinfo.ptr_atts.nullable);
Cyc_Unify_occurs(evar,r,env,pinfo.ptr_atts.bounds);
Cyc_Unify_occurs(evar,r,env,pinfo.ptr_atts.zero_term);
Cyc_Unify_occurs(evar,r,env,pinfo.ptr_atts.autoreleased);
goto _LL0;}case 4: _Tmp7=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)t)->f1.elt_type;_Tmp6=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)t)->f1.zero_term;{void*t2=_Tmp7;void*zt=_Tmp6;
# 171
Cyc_Unify_occurs(evar,r,env,t2);
Cyc_Unify_occurs(evar,r,env,zt);
goto _LL0;}case 5: _Tmp7=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)t)->f1.tvars;_Tmp6=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)t)->f1.effect;_Tmp4=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)t)->f1.ret_tqual;_Tmp3=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)t)->f1.ret_type;_Tmp2=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)t)->f1.args;_Tmp1=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)t)->f1.cyc_varargs;_Tmp0=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)t)->f1.rgn_po;{struct Cyc_List_List*tvs=_Tmp7;void*eff=_Tmp6;struct Cyc_Absyn_Tqual rt_tq=_Tmp4;void*rt=_Tmp3;struct Cyc_List_List*args=_Tmp2;struct Cyc_Absyn_VarargInfo*cyc_varargs=_Tmp1;struct Cyc_List_List*rgn_po=_Tmp0;
# 176
env=Cyc_List_rappend(r,tvs,env);
if(eff!=0)
Cyc_Unify_occurs(evar,r,env,eff);
Cyc_Unify_occurs(evar,r,env,rt);
for(1;args!=0;args=args->tl){
Cyc_Unify_occurs(evar,r,env,(*((struct _tuple8*)args->hd)).f2);}
if(cyc_varargs!=0)
Cyc_Unify_occurs(evar,r,env,cyc_varargs->type);
for(1;rgn_po!=0;rgn_po=rgn_po->tl){
struct _tuple11*_Tmp8=(struct _tuple11*)rgn_po->hd;void*_Tmp9;void*_TmpA;_TmpA=_Tmp8->f0;_Tmp9=_Tmp8->f1;{void*r1=_TmpA;void*r2=_Tmp9;
Cyc_Unify_occurs(evar,r,env,r1);
Cyc_Unify_occurs(evar,r,env,r2);}}
# 189
goto _LL0;}case 6: _Tmp7=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)t)->f3;{struct Cyc_List_List*fs=_Tmp7;
# 192
for(1;fs!=0;fs=fs->tl){
Cyc_Unify_occurs(evar,r,env,((struct Cyc_Absyn_Aggrfield*)fs->hd)->type);}
goto _LL0;}case 7: _Tmp7=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)t)->f2;{struct Cyc_List_List*ts=_Tmp7;
_Tmp7=ts;goto _LL10;}case 0: _Tmp7=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)t)->f2;_LL10: {struct Cyc_List_List*ts=_Tmp7;
# 197
for(1;ts!=0;ts=ts->tl){
Cyc_Unify_occurs(evar,r,env,(void*)ts->hd);}
goto _LL0;}default:
# 202
 goto _LL0;}_LL0:;}}
# 206
static void Cyc_Unify_unify_it(void*,void*);
# 209
int Cyc_Unify_unify(void*t1,void*t2){
struct _handler_cons _Tmp0;_push_handler(& _Tmp0);{int _Tmp1=0;if(setjmp(_Tmp0.handler))_Tmp1=1;if(!_Tmp1){
Cyc_Unify_unify_it(t1,t2);{
int _Tmp2=1;_npop_handler(0);return _Tmp2;}
# 211
;_pop_handler();}else{void*_Tmp2=(void*)Cyc_Core_get_exn_thrown();void*_Tmp3;if(((struct Cyc_Unify_Unify_exn_struct*)_Tmp2)->tag==Cyc_Unify_Unify)
# 219
return 0;else{_Tmp3=_Tmp2;{void*exn=_Tmp3;_rethrow(exn);}};}}}
# 224
static void Cyc_Unify_unify_list(struct Cyc_List_List*t1,struct Cyc_List_List*t2){
for(1;t1!=0 && t2!=0;(t1=t1->tl,t2=t2->tl)){
Cyc_Unify_unify_it((void*)t1->hd,(void*)t2->hd);}
if(t1!=0 || t2!=0)
_throw(& Cyc_Unify_Unify_val);}
# 232
static void Cyc_Unify_unify_tqual(struct Cyc_Absyn_Tqual tq1,void*t1,struct Cyc_Absyn_Tqual tq2,void*t2){
if(tq1.print_const && !tq1.real_const)
({struct Cyc_Warn_String_Warn_Warg_struct _Tmp0=({struct Cyc_Warn_String_Warn_Warg_struct _Tmp1;_Tmp1.tag=0,_Tmp1.f1=_tag_fat("tq1 real_const not set.",sizeof(char),24U);_Tmp1;});void*_Tmp1[1];_Tmp1[0]=& _Tmp0;({(int(*)(struct _fat_ptr))Cyc_Warn_impos2;})(_tag_fat(_Tmp1,sizeof(void*),1));});
if(tq2.print_const && !tq2.real_const)
({struct Cyc_Warn_String_Warn_Warg_struct _Tmp0=({struct Cyc_Warn_String_Warn_Warg_struct _Tmp1;_Tmp1.tag=0,_Tmp1.f1=_tag_fat("tq2 real_const not set.",sizeof(char),24U);_Tmp1;});void*_Tmp1[1];_Tmp1[0]=& _Tmp0;({(int(*)(struct _fat_ptr))Cyc_Warn_impos2;})(_tag_fat(_Tmp1,sizeof(void*),1));});
# 238
if((tq1.real_const!=tq2.real_const || tq1.q_volatile!=tq2.q_volatile)|| tq1.q_restrict!=tq2.q_restrict){
# 241
Cyc_Unify_ts_failure=({struct _tuple11 _Tmp0;_Tmp0.f0=t1,_Tmp0.f1=t2;_Tmp0;});
Cyc_Unify_tqs_const=({struct _tuple12 _Tmp0;_Tmp0.f0=tq1.real_const,_Tmp0.f1=tq2.real_const;_Tmp0;});
Cyc_Unify_failure_reason="qualifiers don't match";
_throw(& Cyc_Unify_Unify_val);}
# 247
Cyc_Unify_tqs_const=({struct _tuple12 _Tmp0;_Tmp0.f0=0,_Tmp0.f1=0;_Tmp0;});}
# 261 "unify.cyc"
static int Cyc_Unify_unify_effect(void*e1,void*e2){
e1=Cyc_Tcutil_normalize_effect(e1);
e2=Cyc_Tcutil_normalize_effect(e2);
if(Cyc_Tcutil_subset_effect(0,e1,e2)&& Cyc_Tcutil_subset_effect(0,e2,e1))
return 1;
if(Cyc_Tcutil_subset_effect(1,e1,e2)&& Cyc_Tcutil_subset_effect(1,e2,e1))
return 1;
return 0;}
# 271
static int Cyc_Unify_unify_const_exp_opt(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
if(e1==0 && e2==0)
return 1;
if(e1==0 || e2==0)
return 0;
return Cyc_Evexp_same_uint_const_exp(e1,e2);}struct _tuple13{struct Cyc_Absyn_Tvar*f0;void*f1;};struct _tuple14{struct Cyc_Absyn_VarargInfo*f0;struct Cyc_Absyn_VarargInfo*f1;};
# 280
static void Cyc_Unify_unify_it(void*t1,void*t2){
Cyc_Unify_ts_failure=({struct _tuple11 _Tmp0;_Tmp0.f0=t1,_Tmp0.f1=t2;_Tmp0;});
Cyc_Unify_failure_reason=0;
t1=Cyc_Absyn_compress(t1);
t2=Cyc_Absyn_compress(t2);
if(t1==t2)return;{
struct _tuple11 _Tmp0=({struct _tuple11 _Tmp1;_Tmp1.f0=t2,_Tmp1.f1=t1;_Tmp1;});enum Cyc_Absyn_AggrKind _Tmp1;enum Cyc_Absyn_AggrKind _Tmp2;void*_Tmp3;void*_Tmp4;void*_Tmp5;void*_Tmp6;void*_Tmp7;void*_Tmp8;void*_Tmp9;void*_TmpA;void*_TmpB;void*_TmpC;int _TmpD;void*_TmpE;void*_TmpF;void*_Tmp10;void*_Tmp11;int _Tmp12;void*_Tmp13;void*_Tmp14;void*_Tmp15;void*_Tmp16;void*_Tmp17;void*_Tmp18;struct Cyc_Absyn_Tqual _Tmp19;void*_Tmp1A;void*_Tmp1B;void*_Tmp1C;struct Cyc_Absyn_Tqual _Tmp1D;enum Cyc_Absyn_AliasQualVal _Tmp1E;enum Cyc_Absyn_AliasQualVal _Tmp1F;void*_Tmp20;void*_Tmp21;void*_Tmp22;void*_Tmp23;void*_Tmp24;if(*((int*)_Tmp0.f0)==1){if(*((int*)_Tmp0.f1)==1){_Tmp24=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Evar_Absyn_Type_struct*)_Tmp0.f0)->f1;_Tmp23=(void**)&((struct Cyc_Absyn_Evar_Absyn_Type_struct*)_Tmp0.f0)->f2;_Tmp22=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Evar_Absyn_Type_struct*)_Tmp0.f0)->f4;_Tmp21=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Evar_Absyn_Type_struct*)_Tmp0.f1)->f1;_Tmp20=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Evar_Absyn_Type_struct*)_Tmp0.f1)->f4;{struct Cyc_Core_Opt**kind1opt=_Tmp24;void**t2r=_Tmp23;struct Cyc_Core_Opt**s1opt=_Tmp22;struct Cyc_Core_Opt**kind2opt=_Tmp21;struct Cyc_Core_Opt**s2opt=_Tmp20;
# 288
if(({struct Cyc_Absyn_Kind*_Tmp25=(struct Cyc_Absyn_Kind*)_check_null(*kind1opt)->v;Cyc_Kinds_kind_leq(_Tmp25,(struct Cyc_Absyn_Kind*)_check_null(*kind2opt)->v);}))
*kind2opt=*kind1opt;else{
if(({struct Cyc_Absyn_Kind*_Tmp25=(struct Cyc_Absyn_Kind*)_check_null(*kind2opt)->v;Cyc_Kinds_kind_leq(_Tmp25,(struct Cyc_Absyn_Kind*)_check_null(*kind1opt)->v);}))
*kind1opt=*kind2opt;else{
# 293
Cyc_Unify_fail_because("kinds are incompatible");}}
*t2r=t1;{
# 296
struct Cyc_List_List*s1=(struct Cyc_List_List*)_check_null(*s1opt)->v;
struct Cyc_List_List*s2=(struct Cyc_List_List*)_check_null(*s2opt)->v;
if(s1==s2)return;
for(1;s1!=0;s1=s1->tl){
if(!({(int(*)(int(*)(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*),struct Cyc_List_List*,struct Cyc_Absyn_Tvar*))Cyc_List_mem;})(Cyc_Tcutil_fast_tvar_cmp,s2,(struct Cyc_Absyn_Tvar*)s1->hd))
break;}
if(s1==0){
*s2opt=*s1opt;
return;}
# 306
s1=(struct Cyc_List_List*)_check_null(*s1opt)->v;
for(1;s2!=0;s2=s2->tl){
if(!({(int(*)(int(*)(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*),struct Cyc_List_List*,struct Cyc_Absyn_Tvar*))Cyc_List_mem;})(Cyc_Tcutil_fast_tvar_cmp,s1,(struct Cyc_Absyn_Tvar*)s2->hd))
break;}
if(s2==0){
*s1opt=*s2opt;
return;}
# 314
s2=(struct Cyc_List_List*)_check_null(*s2opt)->v;{
struct Cyc_List_List*ans=0;
for(1;s2!=0;s2=s2->tl){
if(({(int(*)(int(*)(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*),struct Cyc_List_List*,struct Cyc_Absyn_Tvar*))Cyc_List_mem;})(Cyc_Tcutil_fast_tvar_cmp,s1,(struct Cyc_Absyn_Tvar*)s2->hd))
ans=({struct Cyc_List_List*_Tmp25=_cycalloc(sizeof(struct Cyc_List_List));_Tmp25->hd=(struct Cyc_Absyn_Tvar*)s2->hd,_Tmp25->tl=ans;_Tmp25;});}
({struct Cyc_Core_Opt*_Tmp25=({struct Cyc_Core_Opt*_Tmp26=({struct Cyc_Core_Opt*_Tmp27=_cycalloc(sizeof(struct Cyc_Core_Opt));_Tmp27->v=ans;_Tmp27;});*s2opt=_Tmp26;});*s1opt=_Tmp25;});
return;}}}}else{
# 350
Cyc_Unify_unify_it(t2,t1);return;}}else{if(*((int*)_Tmp0.f1)==1){_Tmp24=((struct Cyc_Absyn_Evar_Absyn_Type_struct*)_Tmp0.f1)->f1;_Tmp23=(void**)&((struct Cyc_Absyn_Evar_Absyn_Type_struct*)_Tmp0.f1)->f2;_Tmp22=((struct Cyc_Absyn_Evar_Absyn_Type_struct*)_Tmp0.f1)->f4;{struct Cyc_Core_Opt*kind1=_Tmp24;void**ref1_ref=_Tmp23;struct Cyc_Core_Opt*s1opt=_Tmp22;
# 325
Cyc_Unify_occurs(t1,Cyc_Core_heap_region,(struct Cyc_List_List*)_check_null(s1opt)->v,t2);{
struct Cyc_Absyn_Kind*kind2=Cyc_Tcutil_type_kind(t2);
# 329
if(Cyc_Kinds_kind_leq(kind2,(struct Cyc_Absyn_Kind*)kind1->v)){
*ref1_ref=t2;
return;}{
# 336
struct Cyc_Absyn_PtrInfo _Tmp25;if(*((int*)t2)==3){_Tmp25=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)t2)->f1;if((int)((struct Cyc_Absyn_Kind*)kind1->v)->kind==2){struct Cyc_Absyn_PtrInfo pinfo=_Tmp25;
# 338
void*c=Cyc_Absyn_compress(pinfo.ptr_atts.bounds);
if(*((int*)c)==1){
# 341
({void*_Tmp26=c;Cyc_Unify_unify(_Tmp26,Cyc_Absyn_bounds_one());});
*ref1_ref=t2;
return;}else{
Cyc_Unify_fail_because("kinds are incompatible");};}else{goto _LL3C;}}else{_LL3C:
# 346
 Cyc_Unify_fail_because("kinds are incompatible");};}}}}else{if(*((int*)_Tmp0.f0)==0){if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f1)==10)
# 352
goto _LLA;else{if(*((int*)_Tmp0.f1)==0){if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)==10)goto _LL9;else{if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f1)==9)goto _LLB;else{if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)==9)goto _LLD;else{if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f1)==11)goto _LLF;else{if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)==11)goto _LL11;else{switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f1)){case 17: switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)){case 17: _Tmp1F=((struct Cyc_Absyn_AqualConstCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f1)->f1;_Tmp1E=((struct Cyc_Absyn_AqualConstCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)->f1;{enum Cyc_Absyn_AliasQualVal aqv1=_Tmp1F;enum Cyc_Absyn_AliasQualVal aqv2=_Tmp1E;
# 363
if((int)aqv1!=(int)aqv2){
Cyc_Unify_ts_failure=({struct _tuple11 _Tmp25;_Tmp25.f0=t1,_Tmp25.f1=t2;_Tmp25;});
Cyc_Unify_fail_because("(different alias qualifiers)");}
# 367
goto _LL0;}case 18: if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2!=0)switch(*((int*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2)->hd)){case 2: _LL18:
# 370
 Cyc_Unify_fail_because("(abstracted type variable doesn't unify with constant)");case 1:
# 372
 Cyc_Unify_unify_it(t2,t1);
goto _LL0;default: goto _LL27;}else{goto _LL27;}default: goto _LL27;}case 18: if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2!=0)switch(*((int*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2)->hd)){case 2: switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)){case 17:
# 368
 goto _LL18;case 18: if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2!=0)switch(*((int*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2)->hd)){case 2: _Tmp24=((struct Cyc_Absyn_VarType_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2->hd)->f1;_Tmp23=((struct Cyc_Absyn_VarType_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2->hd)->f1;{struct Cyc_Absyn_Tvar*tv2=_Tmp24;struct Cyc_Absyn_Tvar*tv1=_Tmp23;
# 380
if(tv2->identity!=tv1->identity)
Cyc_Unify_fail_because("(variable types are not the same)");
goto _LL0;}case 1:
# 388
 Cyc_Unify_unify_it(t2,t1);
goto _LL0;default: goto _LL27;}else{goto _LL27;}default: goto _LL27;}case 1: switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)){case 17: _Tmp24=((struct Cyc_Absyn_Evar_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2->hd)->f1;_Tmp23=(void**)&((struct Cyc_Absyn_Evar_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2->hd)->f2;_Tmp22=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2->tl;_Tmp1F=((struct Cyc_Absyn_AqualConstCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)->f1;{struct Cyc_Core_Opt*k=_Tmp24;void**ref=_Tmp23;struct Cyc_List_List*bnd=_Tmp22;enum Cyc_Absyn_AliasQualVal aqv=_Tmp1F;
# 375
if(!Cyc_Kinds_kind_eq((struct Cyc_Absyn_Kind*)_check_null(k)->v,& Cyc_Kinds_aqk))
Cyc_Unify_fail_because("(incompatible kind)");
*ref=t1;
goto _LL0;}case 18: if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2!=0)switch(*((int*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2)->hd)){case 2: _Tmp24=((struct Cyc_Absyn_Evar_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2->hd)->f1;_Tmp23=(void**)&((struct Cyc_Absyn_Evar_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2->hd)->f2;_Tmp22=((struct Cyc_Absyn_Evar_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2->hd)->f4;_Tmp21=((struct Cyc_Absyn_VarType_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2->hd)->f1;_Tmp20=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2->tl;{struct Cyc_Core_Opt*k=_Tmp24;void**ref=_Tmp23;struct Cyc_Core_Opt*s2_opt=_Tmp22;struct Cyc_Absyn_Tvar*tv=_Tmp21;struct Cyc_List_List*bnd=_Tmp20;
# 392
Cyc_Unify_occurs(t1,Cyc_Core_heap_region,(struct Cyc_List_List*)_check_null(s2_opt)->v,t2);
# 394
if(!({struct Cyc_Absyn_Kind*_Tmp25=(struct Cyc_Absyn_Kind*)k->v;Cyc_Kinds_kind_eq(_Tmp25,Cyc_Tcutil_type_kind(t1));}))
Cyc_Unify_fail_because("(incompatible kinds");
*ref=t1;
goto _LL0;}case 1: _Tmp24=(void*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2->hd;_Tmp23=(void*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2->hd;{void*ev2=_Tmp24;void*ev1=_Tmp23;
# 399
Cyc_Unify_unify_it(ev1,ev2);
# 405
goto _LL0;}default: goto _LL27;}else{goto _LL27;}default: goto _LL27;}case 0: if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2)->hd)->f1)==16){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2)->hd)->f2!=0){if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)==18){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2!=0){if(*((int*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2)->hd)==0){if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2)->hd)->f1)==16){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2)->hd)->f2!=0){_Tmp24=(void*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2->hd)->f2->hd;_Tmp23=(void*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2->hd)->f2->hd;{void*tv2=_Tmp24;void*tv1=_Tmp23;
# 384
if(Cyc_Tcutil_typecmp(tv2,tv1))
Cyc_Unify_fail_because("(aquals(`a) variables are not the same)");
goto _LL0;}}else{goto _LL27;}}else{goto _LL27;}}else{goto _LL27;}}else{goto _LL27;}}else{goto _LL27;}}else{goto _LL27;}}else{goto _LL27;}default: goto _LL27;}else{goto _LL27;}default: _LL27: _Tmp24=(void*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f1;_Tmp23=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2;_Tmp22=(void*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1;_Tmp21=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f2;{void*c1=_Tmp24;struct Cyc_List_List*ts1=_Tmp23;void*c2=_Tmp22;struct Cyc_List_List*ts2=_Tmp21;
# 407
if(Cyc_Tcutil_tycon_cmp(c1,c2)!=0)
Cyc_Unify_fail_because("different type constructors");
Cyc_Unify_unify_list(ts1,ts2);
return;}}}}}}}}else{switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f1)){case 9: _LLB: _LLC:
# 354
 goto _LLE;case 11: _LLF: _LL10:
# 356
 goto _LL12;case 17: goto _LL37;case 18: if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2!=0)switch(*((int*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2)->hd)){case 2: goto _LL37;case 1: goto _LL37;case 0: if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2)->hd)->f1)==16){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)((struct Cyc_List_List*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f0)->f2)->hd)->f2!=0)goto _LL37;else{goto _LL37;}}else{goto _LL37;}default: goto _LL37;}else{goto _LL37;}default: goto _LL37;}}}}else{if(*((int*)_Tmp0.f1)==0)switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp0.f1)->f1)){case 10: _LL9: _LLA:
# 353
 goto _LLC;case 9: _LLD: _LLE:
# 355
 goto _LL10;case 11: _LL11: _LL12:
# 358
 if(!Cyc_Unify_unify_effect(t1,t2))
Cyc_Unify_fail_because("effects don't unify");
return;default: switch(*((int*)_Tmp0.f0)){case 2: goto _LL37;case 3: goto _LL37;case 8: goto _LL37;case 4: goto _LL37;case 5: goto _LL37;case 6: goto _LL37;case 7: goto _LL37;default: goto _LL37;}}else{switch(*((int*)_Tmp0.f0)){case 2: if(*((int*)_Tmp0.f1)==2){_Tmp24=((struct Cyc_Absyn_VarType_Absyn_Type_struct*)_Tmp0.f0)->f1;_Tmp23=((struct Cyc_Absyn_VarType_Absyn_Type_struct*)_Tmp0.f1)->f1;{struct Cyc_Absyn_Tvar*tv2=_Tmp24;struct Cyc_Absyn_Tvar*tv1=_Tmp23;
# 414
if(tv2->identity!=tv1->identity)
Cyc_Unify_fail_because("variable types are not the same");
return;}}else{goto _LL37;}case 3: if(*((int*)_Tmp0.f1)==3){_Tmp24=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f0)->f1.elt_type;_Tmp1D=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f0)->f1.elt_tq;_Tmp23=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f0)->f1.ptr_atts.rgn;_Tmp22=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f0)->f1.ptr_atts.nullable;_Tmp21=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f0)->f1.ptr_atts.bounds;_Tmp20=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f0)->f1.ptr_atts.zero_term;_Tmp1C=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f0)->f1.ptr_atts.autoreleased;_Tmp1B=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f0)->f1.ptr_atts.aqual;_Tmp1A=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f1)->f1.elt_type;_Tmp19=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f1)->f1.elt_tq;_Tmp18=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f1)->f1.ptr_atts.rgn;_Tmp17=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f1)->f1.ptr_atts.nullable;_Tmp16=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f1)->f1.ptr_atts.bounds;_Tmp15=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f1)->f1.ptr_atts.zero_term;_Tmp14=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f1)->f1.ptr_atts.autoreleased;_Tmp13=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_Tmp0.f1)->f1.ptr_atts.aqual;{void*t2a=_Tmp24;struct Cyc_Absyn_Tqual tqual2a=_Tmp1D;void*rgn2=_Tmp23;void*null2a=_Tmp22;void*b2=_Tmp21;void*zt2=_Tmp20;void*rel2=_Tmp1C;void*aq2=_Tmp1B;void*t1a=_Tmp1A;struct Cyc_Absyn_Tqual tqual1a=_Tmp19;void*rgn1=_Tmp18;void*null1a=_Tmp17;void*b1=_Tmp16;void*zt1=_Tmp15;void*rel1=_Tmp14;void*aq1=_Tmp13;
# 420
Cyc_Unify_unify_it(t1a,t2a);
Cyc_Unify_unify_it(rgn2,rgn1);{
const char*orig_failure=Cyc_Unify_failure_reason;
Cyc_Unify_unify_it(aq2,aq1);
# 425
if(!Cyc_Unify_unify(zt1,zt2)){
Cyc_Unify_ts_failure=({struct _tuple11 _Tmp25;_Tmp25.f0=t1,_Tmp25.f1=t2;_Tmp25;});
Cyc_Unify_fail_because("not both zero terminated");}
# 429
if(!Cyc_Unify_unify(rel1,rel2)){
Cyc_Unify_ts_failure=({struct _tuple11 _Tmp25;_Tmp25.f0=t1,_Tmp25.f1=t2;_Tmp25;});
Cyc_Unify_fail_because("not both autoreleased");}
# 433
Cyc_Unify_unify_tqual(tqual1a,t1a,tqual2a,t2a);
if(!Cyc_Unify_unify(b1,b2)){
Cyc_Unify_ts_failure=({struct _tuple11 _Tmp25;_Tmp25.f0=t1,_Tmp25.f1=t2;_Tmp25;});
Cyc_Unify_fail_because("different pointer bounds");}{
# 439
void*_Tmp25=Cyc_Absyn_compress(b1);if(*((int*)_Tmp25)==0){if(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_Tmp25)->f1)==15){
# 441
Cyc_Unify_failure_reason=orig_failure;
return;}else{goto _LL46;}}else{_LL46:
# 444
 Cyc_Unify_failure_reason="incompatible pointer types";
Cyc_Unify_unify_it(null1a,null2a);
return;};}}}}else{goto _LL37;}case 8: if(*((int*)_Tmp0.f1)==8){_Tmp24=((struct Cyc_Absyn_ValueofType_Absyn_Type_struct*)_Tmp0.f0)->f1;_Tmp23=((struct Cyc_Absyn_ValueofType_Absyn_Type_struct*)_Tmp0.f1)->f1;{struct Cyc_Absyn_Exp*e1=_Tmp24;struct Cyc_Absyn_Exp*e2=_Tmp23;
# 450
if(!Cyc_Evexp_same_uint_const_exp(e1,e2))
Cyc_Unify_fail_because("cannot prove expressions are the same");
return;}}else{goto _LL37;}case 4: if(*((int*)_Tmp0.f1)==4){_Tmp24=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_Tmp0.f0)->f1.elt_type;_Tmp1D=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_Tmp0.f0)->f1.tq;_Tmp23=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_Tmp0.f0)->f1.num_elts;_Tmp22=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_Tmp0.f0)->f1.zero_term;_Tmp21=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_Tmp0.f1)->f1.elt_type;_Tmp19=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_Tmp0.f1)->f1.tq;_Tmp20=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_Tmp0.f1)->f1.num_elts;_Tmp1C=((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_Tmp0.f1)->f1.zero_term;{void*t2a=_Tmp24;struct Cyc_Absyn_Tqual tq2a=_Tmp1D;struct Cyc_Absyn_Exp*e1=_Tmp23;void*zt1=_Tmp22;void*t1a=_Tmp21;struct Cyc_Absyn_Tqual tq1a=_Tmp19;struct Cyc_Absyn_Exp*e2=_Tmp20;void*zt2=_Tmp1C;
# 456
Cyc_Unify_unify_it(t1a,t2a);
Cyc_Unify_unify_tqual(tq1a,t1a,tq2a,t2a);
Cyc_Unify_failure_reason="not both zero terminated";
Cyc_Unify_unify_it(zt1,zt2);
if(!Cyc_Unify_unify_const_exp_opt(e1,e2))
Cyc_Unify_fail_because("different array sizes");
return;}}else{goto _LL37;}case 5: if(*((int*)_Tmp0.f1)==5){_Tmp24=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.tvars;_Tmp23=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.effect;_Tmp1D=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.ret_tqual;_Tmp22=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.ret_type;_Tmp21=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.args;_Tmp12=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.c_varargs;_Tmp20=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.cyc_varargs;_Tmp1C=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.rgn_po;_Tmp1B=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.qual_bnd;_Tmp1A=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.attributes;_Tmp18=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.requires_clause;_Tmp17=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.requires_relns;_Tmp16=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.ensures_clause;_Tmp15=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.ensures_relns;_Tmp14=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.return_value;_Tmp13=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f0)->f1.arg_vardecls;_Tmp11=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.tvars;_Tmp10=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.effect;_Tmp19=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.ret_tqual;_TmpF=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.ret_type;_TmpE=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.args;_TmpD=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.c_varargs;_TmpC=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.cyc_varargs;_TmpB=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.rgn_po;_TmpA=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.qual_bnd;_Tmp9=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.attributes;_Tmp8=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.requires_clause;_Tmp7=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.requires_relns;_Tmp6=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.ensures_clause;_Tmp5=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.ensures_relns;_Tmp4=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.return_value;_Tmp3=((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_Tmp0.f1)->f1.arg_vardecls;{struct Cyc_List_List*tvs2=_Tmp24;void*eff2=_Tmp23;struct Cyc_Absyn_Tqual rt_tq2=_Tmp1D;void*rt2=_Tmp22;struct Cyc_List_List*args2=_Tmp21;int c_vararg2=_Tmp12;struct Cyc_Absyn_VarargInfo*cyc_vararg2=_Tmp20;struct Cyc_List_List*rpo2=_Tmp1C;struct Cyc_List_List*qb2=_Tmp1B;struct Cyc_List_List*atts2=_Tmp1A;struct Cyc_Absyn_Exp*req2=_Tmp18;struct Cyc_List_List*req_relns2=_Tmp17;struct Cyc_Absyn_Exp*ens2=_Tmp16;struct Cyc_List_List*ens_relns2=_Tmp15;struct Cyc_Absyn_Vardecl*argvds2=_Tmp14;struct Cyc_List_List*return_value2=_Tmp13;struct Cyc_List_List*tvs1=_Tmp11;void*eff1=_Tmp10;struct Cyc_Absyn_Tqual rt_tq1=_Tmp19;void*rt1=_TmpF;struct Cyc_List_List*args1=_TmpE;int c_vararg1=_TmpD;struct Cyc_Absyn_VarargInfo*cyc_vararg1=_TmpC;struct Cyc_List_List*rpo1=_TmpB;struct Cyc_List_List*qb1=_TmpA;struct Cyc_List_List*atts1=_Tmp9;struct Cyc_Absyn_Exp*req1=_Tmp8;struct Cyc_List_List*req_relns1=_Tmp7;struct Cyc_Absyn_Exp*ens1=_Tmp6;struct Cyc_List_List*ens_relns1=_Tmp5;struct Cyc_Absyn_Vardecl*argvds1=_Tmp4;struct Cyc_List_List*return_value1=_Tmp3;
# 466
{struct _RegionHandle _Tmp25=_new_region(0U,"rgn");struct _RegionHandle*rgn=& _Tmp25;_push_region(rgn);
{struct Cyc_List_List*inst=0;
while(tvs1!=0){
if(tvs2==0)
Cyc_Unify_fail_because("second function type has too few type variables");{
void*kb1=((struct Cyc_Absyn_Tvar*)tvs1->hd)->kind;
void*kb2=((struct Cyc_Absyn_Tvar*)tvs2->hd)->kind;
if(!Cyc_Unify_unify_kindbound(kb1,kb2))
Cyc_Unify_fail_because((const char*)_untag_fat_ptr(({struct Cyc_String_pa_PrintArg_struct _Tmp26=({struct Cyc_String_pa_PrintArg_struct _Tmp27;_Tmp27.tag=0,({struct _fat_ptr _Tmp28=(struct _fat_ptr)((struct _fat_ptr)Cyc_Absynpp_tvar2string((struct Cyc_Absyn_Tvar*)tvs1->hd));_Tmp27.f1=_Tmp28;});_Tmp27;});struct Cyc_String_pa_PrintArg_struct _Tmp27=({struct Cyc_String_pa_PrintArg_struct _Tmp28;_Tmp28.tag=0,({struct _fat_ptr _Tmp29=(struct _fat_ptr)((struct _fat_ptr)Cyc_Kinds_kind2string(Cyc_Kinds_tvar_kind((struct Cyc_Absyn_Tvar*)tvs1->hd,& Cyc_Kinds_bk)));_Tmp28.f1=_Tmp29;});_Tmp28;});struct Cyc_String_pa_PrintArg_struct _Tmp28=({struct Cyc_String_pa_PrintArg_struct _Tmp29;_Tmp29.tag=0,({struct _fat_ptr _Tmp2A=(struct _fat_ptr)((struct _fat_ptr)Cyc_Kinds_kind2string(Cyc_Kinds_tvar_kind((struct Cyc_Absyn_Tvar*)tvs2->hd,& Cyc_Kinds_bk)));_Tmp29.f1=_Tmp2A;});_Tmp29;});void*_Tmp29[3];_Tmp29[0]=& _Tmp26,_Tmp29[1]=& _Tmp27,_Tmp29[2]=& _Tmp28;Cyc_aprintf(_tag_fat("type var %s has different kinds %s and %s",sizeof(char),42U),_tag_fat(_Tmp29,sizeof(void*),3));}),sizeof(char),1U));
# 480
inst=({struct Cyc_List_List*_Tmp26=_region_malloc(rgn,0U,sizeof(struct Cyc_List_List));({struct _tuple13*_Tmp27=({struct _tuple13*_Tmp28=_region_malloc(rgn,0U,sizeof(struct _tuple13));_Tmp28->f0=(struct Cyc_Absyn_Tvar*)tvs2->hd,({void*_Tmp29=Cyc_Absyn_var_type((struct Cyc_Absyn_Tvar*)tvs1->hd);_Tmp28->f1=_Tmp29;});_Tmp28;});_Tmp26->hd=_Tmp27;}),_Tmp26->tl=inst;_Tmp26;});
tvs1=tvs1->tl;
tvs2=tvs2->tl;}}
# 484
if(tvs2!=0)
Cyc_Unify_fail_because("second function type has too many type variables");
if(inst!=0){
({void*_Tmp26=(void*)({struct Cyc_Absyn_FnType_Absyn_Type_struct*_Tmp27=_cycalloc(sizeof(struct Cyc_Absyn_FnType_Absyn_Type_struct));_Tmp27->tag=5,_Tmp27->f1.tvars=0,_Tmp27->f1.effect=eff1,_Tmp27->f1.ret_tqual=rt_tq1,_Tmp27->f1.ret_type=rt1,_Tmp27->f1.args=args1,_Tmp27->f1.c_varargs=c_vararg1,_Tmp27->f1.cyc_varargs=cyc_vararg1,_Tmp27->f1.rgn_po=rpo1,_Tmp27->f1.qual_bnd=qb1,_Tmp27->f1.attributes=atts1,_Tmp27->f1.requires_clause=req1,_Tmp27->f1.requires_relns=req_relns1,_Tmp27->f1.ensures_clause=ens1,_Tmp27->f1.ensures_relns=ens_relns1,_Tmp27->f1.return_value=argvds1,_Tmp27->f1.arg_vardecls=return_value1;_Tmp27;});Cyc_Unify_unify_it(_Tmp26,({
# 490
struct _RegionHandle*_Tmp27=rgn;struct Cyc_List_List*_Tmp28=inst;Cyc_Tcutil_rsubstitute(_Tmp27,_Tmp28,(void*)({struct Cyc_Absyn_FnType_Absyn_Type_struct*_Tmp29=_cycalloc(sizeof(struct Cyc_Absyn_FnType_Absyn_Type_struct));
_Tmp29->tag=5,_Tmp29->f1.tvars=0,_Tmp29->f1.effect=eff2,_Tmp29->f1.ret_tqual=rt_tq2,_Tmp29->f1.ret_type=rt2,_Tmp29->f1.args=args2,_Tmp29->f1.c_varargs=c_vararg2,_Tmp29->f1.cyc_varargs=cyc_vararg2,_Tmp29->f1.rgn_po=rpo2,_Tmp29->f1.qual_bnd=qb2,_Tmp29->f1.attributes=atts2,_Tmp29->f1.requires_clause=req2,_Tmp29->f1.requires_relns=req_relns2,_Tmp29->f1.ensures_clause=ens2,_Tmp29->f1.ensures_relns=ens_relns2,_Tmp29->f1.return_value=argvds2,_Tmp29->f1.arg_vardecls=return_value2;_Tmp29;}));}));});
# 495
_npop_handler(0);return;}}
# 467
;_pop_region();}
# 498
Cyc_Unify_unify_it(rt1,rt2);
Cyc_Unify_unify_tqual(rt_tq1,rt1,rt_tq2,rt2);
for(1;args1!=0 && args2!=0;(args1=args1->tl,args2=args2->tl)){
struct _tuple8 _Tmp25=*((struct _tuple8*)args1->hd);void*_Tmp26;struct Cyc_Absyn_Tqual _Tmp27;_Tmp27=_Tmp25.f1;_Tmp26=_Tmp25.f2;{struct Cyc_Absyn_Tqual tqa=_Tmp27;void*ta=_Tmp26;
struct _tuple8 _Tmp28=*((struct _tuple8*)args2->hd);void*_Tmp29;struct Cyc_Absyn_Tqual _Tmp2A;_Tmp2A=_Tmp28.f1;_Tmp29=_Tmp28.f2;{struct Cyc_Absyn_Tqual tqb=_Tmp2A;void*tb=_Tmp29;
Cyc_Unify_unify_it(ta,tb);
Cyc_Unify_unify_tqual(tqa,ta,tqb,tb);}}}
# 506
Cyc_Unify_ts_failure=({struct _tuple11 _Tmp25;_Tmp25.f0=t1,_Tmp25.f1=t2;_Tmp25;});
if(args1!=0 || args2!=0)
Cyc_Unify_fail_because("function types have different number of arguments");
if(c_vararg1!=c_vararg2)
Cyc_Unify_fail_because("only one function type takes C varargs");
# 512
{struct _tuple14 _Tmp25=({struct _tuple14 _Tmp26;_Tmp26.f0=cyc_vararg1,_Tmp26.f1=cyc_vararg2;_Tmp26;});int _Tmp26;void*_Tmp27;struct Cyc_Absyn_Tqual _Tmp28;void*_Tmp29;int _Tmp2A;void*_Tmp2B;struct Cyc_Absyn_Tqual _Tmp2C;void*_Tmp2D;if(_Tmp25.f0==0){if(_Tmp25.f1==0)
goto _LL4E;else{
goto _LL54;}}else{if(_Tmp25.f1==0){_LL54:
 Cyc_Unify_fail_because("only one function type takes varargs");}else{_Tmp2D=_Tmp25.f0->name;_Tmp2C=_Tmp25.f0->tq;_Tmp2B=_Tmp25.f0->type;_Tmp2A=_Tmp25.f0->inject;_Tmp29=_Tmp25.f1->name;_Tmp28=_Tmp25.f1->tq;_Tmp27=_Tmp25.f1->type;_Tmp26=_Tmp25.f1->inject;{struct _fat_ptr*n1=_Tmp2D;struct Cyc_Absyn_Tqual tq1=_Tmp2C;void*tp1=_Tmp2B;int i1=_Tmp2A;struct _fat_ptr*n2=_Tmp29;struct Cyc_Absyn_Tqual tq2=_Tmp28;void*tp2=_Tmp27;int i2=_Tmp26;
# 517
Cyc_Unify_unify_it(tp1,tp2);
Cyc_Unify_unify_tqual(tq1,tp1,tq2,tp2);
if(i1!=i2)
Cyc_Unify_fail_because("only one function type injects varargs");}}}_LL4E:;}{
# 524
int bad_effect;
if(eff1==0 && eff2==0)
bad_effect=0;else{
if(eff1==0 || eff2==0)
bad_effect=1;else{
# 530
bad_effect=!Cyc_Unify_unify_effect(eff1,eff2);}}
Cyc_Unify_ts_failure=({struct _tuple11 _Tmp25;_Tmp25.f0=t1,_Tmp25.f1=t2;_Tmp25;});
if(bad_effect)
Cyc_Unify_fail_because((const char*)_untag_fat_ptr(({struct Cyc_String_pa_PrintArg_struct _Tmp25=({struct Cyc_String_pa_PrintArg_struct _Tmp26;_Tmp26.tag=0,({struct _fat_ptr _Tmp27=(struct _fat_ptr)((struct _fat_ptr)((unsigned)eff1?Cyc_Absynpp_typ2string(eff1):(struct _fat_ptr)_tag_fat("-",sizeof(char),2U)));_Tmp26.f1=_Tmp27;});_Tmp26;});struct Cyc_String_pa_PrintArg_struct _Tmp26=({struct Cyc_String_pa_PrintArg_struct _Tmp27;_Tmp27.tag=0,({struct _fat_ptr _Tmp28=(struct _fat_ptr)((struct _fat_ptr)((unsigned)eff2?Cyc_Absynpp_typ2string(eff2):(struct _fat_ptr)_tag_fat("-",sizeof(char),2U)));_Tmp27.f1=_Tmp28;});_Tmp27;});void*_Tmp27[2];_Tmp27[0]=& _Tmp25,_Tmp27[1]=& _Tmp26;Cyc_aprintf(_tag_fat("function effects (%s,%s) do not match",sizeof(char),38U),_tag_fat(_Tmp27,sizeof(void*),2));}),sizeof(char),1U));
# 535
if(!Cyc_Atts_equiv_fn_atts(atts2,atts1))
Cyc_Unify_fail_because("function types have different attributes");
if(!Cyc_Tcutil_same_rgn_po(rpo2,rpo1))
Cyc_Unify_fail_because("function types have different region lifetime orderings");
# 542
if(!Cyc_Unify_check_logical_equivalence(req_relns1,req_relns2))
Cyc_Unify_fail_because("@requires clauses not equivalent");
if(!Cyc_Unify_check_logical_equivalence(ens_relns1,ens_relns2))
Cyc_Unify_fail_because("@ensures clauses not equivalent");
return;}}}else{goto _LL37;}case 6: if(*((int*)_Tmp0.f1)==6){_Tmp2=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_Tmp0.f0)->f1;_Tmp12=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_Tmp0.f0)->f2;_Tmp24=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_Tmp0.f0)->f3;_Tmp1=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_Tmp0.f1)->f1;_TmpD=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_Tmp0.f1)->f2;_Tmp23=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_Tmp0.f1)->f3;{enum Cyc_Absyn_AggrKind k2=_Tmp2;int tup2=_Tmp12;struct Cyc_List_List*fs2=_Tmp24;enum Cyc_Absyn_AggrKind k1=_Tmp1;int tup1=_TmpD;struct Cyc_List_List*fs1=_Tmp23;
# 549
if((int)k1!=(int)k2)
Cyc_Unify_fail_because("struct and union type");
for(1;fs1!=0 && fs2!=0;(fs1=fs1->tl,fs2=fs2->tl)){
struct Cyc_Absyn_Aggrfield*f1=(struct Cyc_Absyn_Aggrfield*)fs1->hd;
struct Cyc_Absyn_Aggrfield*f2=(struct Cyc_Absyn_Aggrfield*)fs2->hd;
if(Cyc_strptrcmp(f1->name,f2->name)!=0)
Cyc_Unify_fail_because("different member names");
Cyc_Unify_unify_it(f1->type,f2->type);
Cyc_Unify_unify_tqual(f1->tq,f1->type,f2->tq,f2->type);
Cyc_Unify_ts_failure=({struct _tuple11 _Tmp25;_Tmp25.f0=t1,_Tmp25.f1=t2;_Tmp25;});
if(!Cyc_Atts_same_atts(f1->attributes,f2->attributes))
Cyc_Unify_fail_because("different attributes on member");
if(!Cyc_Unify_unify_const_exp_opt(f1->width,f2->width))
Cyc_Unify_fail_because("different bitfield widths on member");
if(!Cyc_Unify_unify_const_exp_opt(f1->requires_clause,f2->requires_clause))
Cyc_Unify_fail_because("different @requires clauses on member");}
# 566
if(fs1!=0 || fs2!=0)
Cyc_Unify_fail_because("different number of members");
return;}}else{goto _LL37;}case 7: if(*((int*)_Tmp0.f1)==7){_Tmp24=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_Tmp0.f0)->f2;_Tmp23=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_Tmp0.f0)->f3;_Tmp22=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_Tmp0.f1)->f2;_Tmp21=((struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_Tmp0.f1)->f3;{struct Cyc_List_List*ts1=_Tmp24;struct Cyc_Absyn_Typedefdecl*td1=_Tmp23;struct Cyc_List_List*ts2=_Tmp22;struct Cyc_Absyn_Typedefdecl*td2=_Tmp21;
# 571
if(td1!=td2)
Cyc_Unify_fail_because("different abstract typedefs");
Cyc_Unify_failure_reason="type parameters to typedef differ";
Cyc_Unify_unify_list(ts1,ts2);
return;}}else{goto _LL37;}default: _LL37:
 _throw(& Cyc_Unify_Unify_val);}}}}}_LL0:;}}
