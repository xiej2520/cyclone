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
 struct Cyc_Core_Opt{void*v;};extern char Cyc_Core_Invalid_argument[17U];extern char Cyc_Core_Failure[8U];extern char Cyc_Core_Impossible[11U];extern char Cyc_Core_Not_found[10U];extern char Cyc_Core_Unreachable[12U];
# 171 "core.h"
extern struct _RegionHandle*Cyc_Core_unique_region;struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};extern char Cyc_List_List_mismatch[14U];
# 178 "list.h"
extern struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*);extern char Cyc_List_Nth[4U];struct Cyc_String_pa_PrintArg_struct{int tag;struct _fat_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};extern char Cyc_FileCloseError[15U];extern char Cyc_FileOpenError[14U];
# 290 "cycboot.h"
extern int isdigit(int);
# 38 "string.h"
extern unsigned long Cyc_strlen(struct _fat_ptr);
# 136 "string.h"
extern struct _fat_ptr Cyc_implode(struct Cyc_List_List*);struct Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;unsigned loc;};extern char Cyc_Absyn_EmptyAnnot[11U];
# 856 "absyn.h"
struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual(unsigned);
struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(unsigned);
# 876
void*Cyc_Absyn_new_evar(struct Cyc_Core_Opt*,struct Cyc_Core_Opt*);
# 881
extern void*Cyc_Absyn_char_type;extern void*Cyc_Absyn_uchar_type;extern void*Cyc_Absyn_ushort_type;extern void*Cyc_Absyn_uint_type;extern void*Cyc_Absyn_ulong_type;
# 883
extern void*Cyc_Absyn_schar_type;extern void*Cyc_Absyn_sshort_type;extern void*Cyc_Absyn_sint_type;extern void*Cyc_Absyn_slong_type;
# 885
extern void*Cyc_Absyn_float_type;extern void*Cyc_Absyn_double_type;extern void*Cyc_Absyn_long_double_type;
# 892
extern void*Cyc_Absyn_true_type;extern void*Cyc_Absyn_false_type;
# 933
void*Cyc_Absyn_at_type(void*,void*,struct Cyc_Absyn_Tqual,void*,void*);
# 937
void*Cyc_Absyn_fatptr_type(void*,void*,struct Cyc_Absyn_Tqual,void*,void*);
# 33 "warn.h"
void Cyc_Warn_verr(unsigned,struct _fat_ptr,struct _fat_ptr);
# 238 "tcutil.h"
void*Cyc_Tcutil_any_bool(struct Cyc_List_List*);
# 47 "kinds.h"
extern struct Cyc_Core_Opt Cyc_Kinds_rko;struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[8U];extern char Cyc_Dict_Absent[7U];extern char Cyc_Tcenv_Env_error[10U];struct Cyc_Tcenv_Genv{struct Cyc_Dict_Dict aggrdecls;struct Cyc_Dict_Dict datatypedecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict typedefs;struct Cyc_Dict_Dict ordinaries;};struct Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Tcenv_Genv*ae;struct Cyc_Tcenv_Fenv*le;int allow_valueof: 1;int in_extern_c_include: 1;int in_tempest: 1;int tempest_generalize: 1;int in_extern_c_inc_repeat: 1;};
# 99 "tcenv.h"
struct Cyc_List_List*Cyc_Tcenv_lookup_type_vars(struct Cyc_Tcenv_Tenv*);
# 31 "formatstr.cyc"
static void*Cyc_Formatstr_err_null(unsigned loc,struct _fat_ptr fmt,struct _fat_ptr ap){
# 33
Cyc_Warn_verr(loc,fmt,ap);
return 0;}struct _tuple11{struct Cyc_List_List*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_List_List*f4;char f5;int f6;};
# 43
struct Cyc_Core_Opt*Cyc_Formatstr_parse_conversionspecification(struct _RegionHandle*r,struct _fat_ptr s,int i){
# 47
unsigned long _tmp0=Cyc_strlen((struct _fat_ptr)s);unsigned long len=_tmp0;
if(i < 0 ||(unsigned long)i >= len)return 0;{
# 51
struct Cyc_List_List*_tmp1=0;struct Cyc_List_List*flags=_tmp1;
char c=' ';
for(1;(unsigned long)i < len;++ i){
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
{char _tmp2=c;switch((int)_tmp2){case 43:
 goto _LL4;case 45: _LL4:
 goto _LL6;case 32: _LL6:
 goto _LL8;case 35: _LL8:
 goto _LLA;case 48: _LLA:
 flags=({struct Cyc_List_List*_tmp3=_region_malloc(r,sizeof(*_tmp3));_tmp3->hd=(void*)((int)c),_tmp3->tl=flags;_tmp3;});continue;default:
 goto _LL0;}_LL0:;}
# 63
break;}
# 65
if((unsigned long)i >= len)return 0;
flags=({(struct Cyc_List_List*(*)(struct Cyc_List_List*))Cyc_List_imp_rev;})(flags);{
# 69
struct Cyc_List_List*_tmp4=0;struct Cyc_List_List*width=_tmp4;
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if((int)c == 42){
width=({struct Cyc_List_List*_tmp5=_region_malloc(r,sizeof(*_tmp5));_tmp5->hd=(void*)((int)c),_tmp5->tl=width;_tmp5;});
++ i;}else{
# 75
for(1;(unsigned long)i < len;++ i){
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if(isdigit((int)c))width=({struct Cyc_List_List*_tmp6=_region_malloc(r,sizeof(*_tmp6));_tmp6->hd=(void*)((int)c),_tmp6->tl=width;_tmp6;});else{
break;}}}
# 81
if((unsigned long)i >= len)return 0;
width=({(struct Cyc_List_List*(*)(struct Cyc_List_List*))Cyc_List_imp_rev;})(width);{
# 85
struct Cyc_List_List*_tmp7=0;struct Cyc_List_List*precision=_tmp7;
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if((int)c == 46){
precision=({struct Cyc_List_List*_tmp8=_region_malloc(r,sizeof(*_tmp8));_tmp8->hd=(void*)((int)c),_tmp8->tl=precision;_tmp8;});
if((unsigned long)++ i >= len)return 0;
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if((int)c == 42){
precision=({struct Cyc_List_List*_tmp9=_region_malloc(r,sizeof(*_tmp9));_tmp9->hd=(void*)((int)c),_tmp9->tl=precision;_tmp9;});
++ i;}else{
# 95
for(1;(unsigned long)i < len;++ i){
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if(isdigit((int)c))precision=({struct Cyc_List_List*_tmpA=_region_malloc(r,sizeof(*_tmpA));_tmpA->hd=(void*)((int)c),_tmpA->tl=precision;_tmpA;});else{
break;}}}}
# 101
if((unsigned long)i >= len)return 0;
precision=({(struct Cyc_List_List*(*)(struct Cyc_List_List*))Cyc_List_imp_rev;})(precision);{
# 106
struct Cyc_List_List*_tmpB=0;struct Cyc_List_List*lenmod=_tmpB;
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
{char _tmpC=c;switch((int)_tmpC){case 104:
# 110
 lenmod=({struct Cyc_List_List*_tmpD=_region_malloc(r,sizeof(*_tmpD));_tmpD->hd=(void*)((int)c),_tmpD->tl=lenmod;_tmpD;});
if((unsigned long)++ i >= len)return 0;
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if((int)c == 104){lenmod=({struct Cyc_List_List*_tmpE=_region_malloc(r,sizeof(*_tmpE));_tmpE->hd=(void*)((int)c),_tmpE->tl=lenmod;_tmpE;});++ i;}
goto _LLD;case 108:
# 116
 lenmod=({struct Cyc_List_List*_tmpF=_region_malloc(r,sizeof(*_tmpF));_tmpF->hd=(void*)((int)c),_tmpF->tl=lenmod;_tmpF;});
if((unsigned long)++ i >= len)return 0;
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if((int)c == 108){lenmod=({struct Cyc_List_List*_tmp10=_region_malloc(r,sizeof(*_tmp10));_tmp10->hd=(void*)((int)c),_tmp10->tl=lenmod;_tmp10;});++ i;}
goto _LLD;case 106:
 goto _LL15;case 122: _LL15:
 goto _LL17;case 116: _LL17:
 goto _LL19;case 76: _LL19:
# 125
 lenmod=({struct Cyc_List_List*_tmp11=_region_malloc(r,sizeof(*_tmp11));_tmp11->hd=(void*)((int)c),_tmp11->tl=lenmod;_tmp11;});
++ i;
goto _LLD;default:
 goto _LLD;}_LLD:;}
# 130
if((unsigned long)i >= len)return 0;
lenmod=({(struct Cyc_List_List*(*)(struct Cyc_List_List*))Cyc_List_imp_rev;})(lenmod);
# 134
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
{char _tmp12=c;switch((int)_tmp12){case 100:
 goto _LL20;case 105: _LL20:
 goto _LL22;case 111: _LL22:
 goto _LL24;case 117: _LL24:
 goto _LL26;case 120: _LL26:
 goto _LL28;case 88: _LL28:
 goto _LL2A;case 102: _LL2A:
 goto _LL2C;case 70: _LL2C:
 goto _LL2E;case 101: _LL2E:
 goto _LL30;case 69: _LL30:
 goto _LL32;case 103: _LL32:
 goto _LL34;case 71: _LL34:
 goto _LL36;case 97: _LL36:
 goto _LL38;case 65: _LL38:
 goto _LL3A;case 99: _LL3A:
 goto _LL3C;case 115: _LL3C:
 goto _LL3E;case 112: _LL3E:
 goto _LL40;case 110: _LL40:
 goto _LL42;case 37: _LL42:
 goto _LL1C;default:
 return 0;}_LL1C:;}
# 166 "formatstr.cyc"
return({struct Cyc_Core_Opt*_tmp14=_region_malloc(r,sizeof(*_tmp14));({struct _tuple11*_tmp117=({struct _tuple11*_tmp13=_region_malloc(r,sizeof(*_tmp13));_tmp13->f1=flags,_tmp13->f2=width,_tmp13->f3=precision,_tmp13->f4=lenmod,_tmp13->f5=c,_tmp13->f6=i + 1;_tmp13;});_tmp14->v=_tmp117;});_tmp14;});}}}}}
# 169
struct Cyc_List_List*Cyc_Formatstr_get_format_types(struct Cyc_Tcenv_Tenv*te,struct _fat_ptr s,int isCproto,unsigned loc){
# 172
unsigned long _tmp15=Cyc_strlen((struct _fat_ptr)s);unsigned long len=_tmp15;
struct Cyc_List_List*_tmp16=0;struct Cyc_List_List*typs=_tmp16;
int i;
struct _RegionHandle _tmp17=_new_region("temp");struct _RegionHandle*temp=& _tmp17;_push_region(temp);
for(i=0;(unsigned long)i < len;++ i){
if((int)*((const char*)_check_fat_subscript(s,sizeof(char),i))!= 37)continue;{
struct Cyc_Core_Opt*_tmp18=Cyc_Formatstr_parse_conversionspecification(temp,s,i + 1);struct Cyc_Core_Opt*cs=_tmp18;
if(cs == 0){
struct Cyc_List_List*_tmp1B=({void*_tmp19=0U;({struct Cyc_List_List*(*_tmp11A)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp119=loc;struct _fat_ptr _tmp118=({const char*_tmp1A="bad format string";_tag_fat(_tmp1A,sizeof(char),18U);});_tmp11A(_tmp119,_tmp118,_tag_fat(_tmp19,sizeof(void*),0));});});_npop_handler(0);return _tmp1B;}{
struct _tuple11*_tmp1C=(struct _tuple11*)cs->v;struct _tuple11*_stmttmp0=_tmp1C;struct _tuple11*_tmp1D=_stmttmp0;int _tmp23;char _tmp22;void*_tmp21;void*_tmp20;void*_tmp1F;void*_tmp1E;_tmp1E=_tmp1D->f1;_tmp1F=_tmp1D->f2;_tmp20=_tmp1D->f3;_tmp21=_tmp1D->f4;_tmp22=_tmp1D->f5;_tmp23=_tmp1D->f6;{struct Cyc_List_List*flags=_tmp1E;struct Cyc_List_List*width=_tmp1F;struct Cyc_List_List*precision=_tmp20;struct Cyc_List_List*lenmod=_tmp21;char c=_tmp22;int j=_tmp23;
i=j - 1;
{struct Cyc_List_List*_tmp24=lenmod;int _tmp25;if(_tmp24 != 0){if(((struct Cyc_List_List*)_tmp24)->tl == 0){_tmp25=(int)_tmp24->hd;if(
(_tmp25 == 106 || _tmp25 == 122)|| _tmp25 == 116){int x=_tmp25;
# 187
struct Cyc_List_List*_tmp29=({struct Cyc_Int_pa_PrintArg_struct _tmp28=({struct Cyc_Int_pa_PrintArg_struct _tmpF8;_tmpF8.tag=1,_tmpF8.f1=(unsigned long)x;_tmpF8;});void*_tmp26[1];_tmp26[0]=& _tmp28;({struct Cyc_List_List*(*_tmp11D)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp11C=loc;struct _fat_ptr _tmp11B=({const char*_tmp27="length modifier '%c' is not supported";_tag_fat(_tmp27,sizeof(char),38U);});_tmp11D(_tmp11C,_tmp11B,_tag_fat(_tmp26,sizeof(void*),1));});});_npop_handler(0);return _tmp29;}else{goto _LL6;}}else{goto _LL6;}}else{_LL6:
 goto _LL3;}_LL3:;}
# 190
{struct Cyc_List_List*_tmp2A=width;int _tmp2B;if(_tmp2A != 0){if(((struct Cyc_List_List*)_tmp2A)->tl == 0){_tmp2B=(int)_tmp2A->hd;if(_tmp2B == 42){int x=_tmp2B;
typs=({struct Cyc_List_List*_tmp2C=_cycalloc(sizeof(*_tmp2C));_tmp2C->hd=Cyc_Absyn_sint_type,_tmp2C->tl=typs;_tmp2C;});goto _LL8;}else{goto _LLB;}}else{goto _LLB;}}else{_LLB:
 goto _LL8;}_LL8:;}
# 194
{struct Cyc_List_List*_tmp2D=precision;int _tmp2F;int _tmp2E;if(_tmp2D != 0){if(((struct Cyc_List_List*)_tmp2D)->tl != 0){if(((struct Cyc_List_List*)((struct Cyc_List_List*)_tmp2D)->tl)->tl == 0){_tmp2E=(int)_tmp2D->hd;_tmp2F=(int)(_tmp2D->tl)->hd;if(
_tmp2E == 46 && _tmp2F == 42){int x=_tmp2E;int y=_tmp2F;
typs=({struct Cyc_List_List*_tmp30=_cycalloc(sizeof(*_tmp30));_tmp30->hd=Cyc_Absyn_sint_type,_tmp30->tl=typs;_tmp30;});goto _LLD;}else{goto _LL10;}}else{goto _LL10;}}else{goto _LL10;}}else{_LL10:
 goto _LLD;}_LLD:;}{
# 199
void*t;
char _tmp31=c;switch((int)_tmp31){case 100:
 goto _LL16;case 105: _LL16:
# 203
{struct Cyc_List_List*f=flags;for(0;f != 0;f=f->tl){
if((int)f->hd == 35){
struct Cyc_List_List*_tmp35=({struct Cyc_Int_pa_PrintArg_struct _tmp34=({struct Cyc_Int_pa_PrintArg_struct _tmpF9;_tmpF9.tag=1,_tmpF9.f1=(unsigned long)((int)c);_tmpF9;});void*_tmp32[1];_tmp32[0]=& _tmp34;({struct Cyc_List_List*(*_tmp120)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp11F=loc;struct _fat_ptr _tmp11E=({const char*_tmp33="flag '#' is not valid with %%%c";_tag_fat(_tmp33,sizeof(char),32U);});_tmp120(_tmp11F,_tmp11E,_tag_fat(_tmp32,sizeof(void*),1));});});_npop_handler(0);return _tmp35;}}}
{struct Cyc_List_List*_tmp36=lenmod;int _tmp38;int _tmp37;if(_tmp36 == 0){
t=Cyc_Absyn_sint_type;goto _LL3B;}else{if(((struct Cyc_List_List*)_tmp36)->tl == 0){_tmp37=(int)_tmp36->hd;if(_tmp37 == 108){int x=_tmp37;
t=Cyc_Absyn_slong_type;goto _LL3B;}else{_tmp37=(int)_tmp36->hd;if(_tmp37 == 104){int x=_tmp37;
t=Cyc_Absyn_sshort_type;goto _LL3B;}else{goto _LL44;}}}else{if(((struct Cyc_List_List*)((struct Cyc_List_List*)_tmp36)->tl)->tl == 0){_tmp37=(int)_tmp36->hd;_tmp38=(int)(_tmp36->tl)->hd;if(
_tmp37 == 104 && _tmp38 == 104){int x=_tmp37;int y=_tmp38;
t=Cyc_Absyn_schar_type;goto _LL3B;}else{goto _LL44;}}else{_LL44: {
# 213
struct Cyc_List_List*_tmp3D=({struct Cyc_String_pa_PrintArg_struct _tmp3B=({struct Cyc_String_pa_PrintArg_struct _tmpFB;_tmpFB.tag=0,({
struct _fat_ptr _tmp121=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmpFB.f1=_tmp121;});_tmpFB;});struct Cyc_Int_pa_PrintArg_struct _tmp3C=({struct Cyc_Int_pa_PrintArg_struct _tmpFA;_tmpFA.tag=1,_tmpFA.f1=(unsigned long)((int)c);_tmpFA;});void*_tmp39[2];_tmp39[0]=& _tmp3B,_tmp39[1]=& _tmp3C;({struct Cyc_List_List*(*_tmp124)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 213
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp123=loc;struct _fat_ptr _tmp122=({const char*_tmp3A="length modifier '%s' is not allowed with %%%c";_tag_fat(_tmp3A,sizeof(char),46U);});_tmp124(_tmp123,_tmp122,_tag_fat(_tmp39,sizeof(void*),2));});});_npop_handler(0);return _tmp3D;}}}}_LL3B:;}
# 216
typs=({struct Cyc_List_List*_tmp3E=_cycalloc(sizeof(*_tmp3E));_tmp3E->hd=t,_tmp3E->tl=typs;_tmp3E;});
goto _LL12;case 117:
# 219
{struct Cyc_List_List*f=flags;for(0;f != 0;f=f->tl){
if((int)f->hd == 35){
struct Cyc_List_List*_tmp41=({void*_tmp3F=0U;({struct Cyc_List_List*(*_tmp127)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp126=loc;struct _fat_ptr _tmp125=({const char*_tmp40="Flag '#' not valid with %%u";_tag_fat(_tmp40,sizeof(char),28U);});_tmp127(_tmp126,_tmp125,_tag_fat(_tmp3F,sizeof(void*),0));});});_npop_handler(0);return _tmp41;}}}
goto _LL1A;case 111: _LL1A:
 goto _LL1C;case 120: _LL1C:
 goto _LL1E;case 88: _LL1E:
# 226
{struct Cyc_List_List*_tmp42=lenmod;int _tmp44;int _tmp43;if(_tmp42 == 0){
t=Cyc_Absyn_uint_type;goto _LL46;}else{if(((struct Cyc_List_List*)_tmp42)->tl == 0){_tmp43=(int)_tmp42->hd;if(_tmp43 == 108){int x=_tmp43;
t=Cyc_Absyn_ulong_type;goto _LL46;}else{_tmp43=(int)_tmp42->hd;if(_tmp43 == 104){int x=_tmp43;
t=Cyc_Absyn_ushort_type;goto _LL46;}else{goto _LL4F;}}}else{if(((struct Cyc_List_List*)((struct Cyc_List_List*)_tmp42)->tl)->tl == 0){_tmp43=(int)_tmp42->hd;_tmp44=(int)(_tmp42->tl)->hd;if(
_tmp43 == 104 && _tmp44 == 104){int x=_tmp43;int y=_tmp44;
t=Cyc_Absyn_uchar_type;goto _LL46;}else{goto _LL4F;}}else{_LL4F: {
# 234
struct Cyc_List_List*_tmp49=({struct Cyc_String_pa_PrintArg_struct _tmp47=({struct Cyc_String_pa_PrintArg_struct _tmpFD;_tmpFD.tag=0,({
struct _fat_ptr _tmp128=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmpFD.f1=_tmp128;});_tmpFD;});struct Cyc_Int_pa_PrintArg_struct _tmp48=({struct Cyc_Int_pa_PrintArg_struct _tmpFC;_tmpFC.tag=1,_tmpFC.f1=(unsigned long)((int)c);_tmpFC;});void*_tmp45[2];_tmp45[0]=& _tmp47,_tmp45[1]=& _tmp48;({struct Cyc_List_List*(*_tmp12B)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 234
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp12A=loc;struct _fat_ptr _tmp129=({const char*_tmp46="length modifier '%s' is not allowed with %%%c";_tag_fat(_tmp46,sizeof(char),46U);});_tmp12B(_tmp12A,_tmp129,_tag_fat(_tmp45,sizeof(void*),2));});});_npop_handler(0);return _tmp49;}}}}_LL46:;}
# 237
typs=({struct Cyc_List_List*_tmp4A=_cycalloc(sizeof(*_tmp4A));_tmp4A->hd=t,_tmp4A->tl=typs;_tmp4A;});
goto _LL12;case 102:
 goto _LL22;case 70: _LL22:
 goto _LL24;case 101: _LL24:
 goto _LL26;case 69: _LL26:
 goto _LL28;case 103: _LL28:
 goto _LL2A;case 71: _LL2A:
 goto _LL2C;case 97: _LL2C:
 goto _LL2E;case 65: _LL2E:
# 253
{struct Cyc_List_List*_tmp4B=lenmod;int _tmp4C;if(_tmp4B == 0){
# 255
typs=({struct Cyc_List_List*_tmp4D=_cycalloc(sizeof(*_tmp4D));_tmp4D->hd=Cyc_Absyn_double_type,_tmp4D->tl=typs;_tmp4D;});goto _LL51;}else{if(((struct Cyc_List_List*)_tmp4B)->tl == 0){_tmp4C=(int)_tmp4B->hd;if(_tmp4C == 108){int x=_tmp4C;
# 257
typs=({struct Cyc_List_List*_tmp4E=_cycalloc(sizeof(*_tmp4E));_tmp4E->hd=Cyc_Absyn_long_double_type,_tmp4E->tl=typs;_tmp4E;});goto _LL51;}else{goto _LL56;}}else{_LL56: {
# 259
struct Cyc_List_List*_tmp53=({struct Cyc_String_pa_PrintArg_struct _tmp51=({struct Cyc_String_pa_PrintArg_struct _tmpFF;_tmpFF.tag=0,({
struct _fat_ptr _tmp12C=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmpFF.f1=_tmp12C;});_tmpFF;});struct Cyc_Int_pa_PrintArg_struct _tmp52=({struct Cyc_Int_pa_PrintArg_struct _tmpFE;_tmpFE.tag=1,_tmpFE.f1=(unsigned long)((int)c);_tmpFE;});void*_tmp4F[2];_tmp4F[0]=& _tmp51,_tmp4F[1]=& _tmp52;({struct Cyc_List_List*(*_tmp12F)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 259
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp12E=loc;struct _fat_ptr _tmp12D=({const char*_tmp50="length modifier '%s' is not allowed with %%%c";_tag_fat(_tmp50,sizeof(char),46U);});_tmp12F(_tmp12E,_tmp12D,_tag_fat(_tmp4F,sizeof(void*),2));});});_npop_handler(0);return _tmp53;}}}_LL51:;}
# 262
goto _LL12;case 99:
# 264
{struct Cyc_List_List*f=flags;for(0;f != 0;f=f->tl){
if((int)f->hd == 35 ||(int)f->hd == 48){
struct Cyc_List_List*_tmp57=({struct Cyc_Int_pa_PrintArg_struct _tmp56=({struct Cyc_Int_pa_PrintArg_struct _tmp100;_tmp100.tag=1,_tmp100.f1=(unsigned long)((int)f->hd);_tmp100;});void*_tmp54[1];_tmp54[0]=& _tmp56;({struct Cyc_List_List*(*_tmp132)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp131=loc;struct _fat_ptr _tmp130=({const char*_tmp55="flag '%c' not allowed with %%c";_tag_fat(_tmp55,sizeof(char),31U);});_tmp132(_tmp131,_tmp130,_tag_fat(_tmp54,sizeof(void*),1));});});_npop_handler(0);return _tmp57;}}}
# 269
if(lenmod != 0){
struct Cyc_List_List*_tmp5B=({struct Cyc_String_pa_PrintArg_struct _tmp5A=({struct Cyc_String_pa_PrintArg_struct _tmp101;_tmp101.tag=0,({
struct _fat_ptr _tmp133=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmp101.f1=_tmp133;});_tmp101;});void*_tmp58[1];_tmp58[0]=& _tmp5A;({struct Cyc_List_List*(*_tmp136)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 270
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp135=loc;struct _fat_ptr _tmp134=({const char*_tmp59="length modifier '%s' not allowed with %%c";_tag_fat(_tmp59,sizeof(char),42U);});_tmp136(_tmp135,_tmp134,_tag_fat(_tmp58,sizeof(void*),1));});});_npop_handler(0);return _tmp5B;}
# 272
if(precision != 0){
struct Cyc_List_List*_tmp5F=({struct Cyc_String_pa_PrintArg_struct _tmp5E=({struct Cyc_String_pa_PrintArg_struct _tmp102;_tmp102.tag=0,({
struct _fat_ptr _tmp137=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(precision));_tmp102.f1=_tmp137;});_tmp102;});void*_tmp5C[1];_tmp5C[0]=& _tmp5E;({struct Cyc_List_List*(*_tmp13A)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 273
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp139=loc;struct _fat_ptr _tmp138=({const char*_tmp5D="precision '%s' not allowed with %%c";_tag_fat(_tmp5D,sizeof(char),36U);});_tmp13A(_tmp139,_tmp138,_tag_fat(_tmp5C,sizeof(void*),1));});});_npop_handler(0);return _tmp5F;}
# 275
typs=({struct Cyc_List_List*_tmp60=_cycalloc(sizeof(*_tmp60));_tmp60->hd=Cyc_Absyn_sint_type,_tmp60->tl=typs;_tmp60;});
goto _LL12;case 115:
# 279
{struct Cyc_List_List*f=flags;for(0;f != 0;f=f->tl){
if((int)f->hd != 45){
struct Cyc_List_List*_tmp63=({void*_tmp61=0U;({struct Cyc_List_List*(*_tmp13D)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp13C=loc;struct _fat_ptr _tmp13B=({const char*_tmp62="a flag not allowed with %%s";_tag_fat(_tmp62,sizeof(char),28U);});_tmp13D(_tmp13C,_tmp13B,_tag_fat(_tmp61,sizeof(void*),0));});});_npop_handler(0);return _tmp63;}}}
# 284
if(lenmod != 0){
struct Cyc_List_List*_tmp66=({void*_tmp64=0U;({struct Cyc_List_List*(*_tmp140)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp13F=loc;struct _fat_ptr _tmp13E=({const char*_tmp65="length modifiers not allowed with %%s";_tag_fat(_tmp65,sizeof(char),38U);});_tmp140(_tmp13F,_tmp13E,_tag_fat(_tmp64,sizeof(void*),0));});});_npop_handler(0);return _tmp66;}{
# 288
void*ptr;
struct Cyc_List_List*_tmp67=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_List_List*tvs=_tmp67;
if(!isCproto)
ptr=({void*_tmp144=Cyc_Absyn_char_type;void*_tmp143=
Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmp68=_cycalloc(sizeof(*_tmp68));_tmp68->v=tvs;_tmp68;}));
# 291
struct Cyc_Absyn_Tqual _tmp142=
# 294
Cyc_Absyn_const_tqual(0U);
# 291
void*_tmp141=Cyc_Absyn_false_type;Cyc_Absyn_fatptr_type(_tmp144,_tmp143,_tmp142,_tmp141,
# 294
Cyc_Tcutil_any_bool(tvs));});else{
# 296
ptr=({void*_tmp148=Cyc_Absyn_char_type;void*_tmp147=
Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmp69=_cycalloc(sizeof(*_tmp69));_tmp69->v=tvs;_tmp69;}));
# 296
struct Cyc_Absyn_Tqual _tmp146=
# 299
Cyc_Absyn_const_tqual(0U);
# 296
void*_tmp145=Cyc_Absyn_true_type;Cyc_Absyn_at_type(_tmp148,_tmp147,_tmp146,_tmp145,
# 299
Cyc_Tcutil_any_bool(tvs));});}
typs=({struct Cyc_List_List*_tmp6A=_cycalloc(sizeof(*_tmp6A));_tmp6A->hd=ptr,_tmp6A->tl=typs;_tmp6A;});
goto _LL12;}case 112:
# 304
 typs=({struct Cyc_List_List*_tmp6B=_cycalloc(sizeof(*_tmp6B));_tmp6B->hd=Cyc_Absyn_uint_type,_tmp6B->tl=typs;_tmp6B;});
goto _LL12;case 110:
# 307
{struct Cyc_List_List*f=flags;for(0;f != 0;f=f->tl){
if((int)f->hd == 35 ||(int)f->hd == 48){
struct Cyc_List_List*_tmp6F=({struct Cyc_Int_pa_PrintArg_struct _tmp6E=({struct Cyc_Int_pa_PrintArg_struct _tmp103;_tmp103.tag=1,_tmp103.f1=(unsigned long)((int)f->hd);_tmp103;});void*_tmp6C[1];_tmp6C[0]=& _tmp6E;({struct Cyc_List_List*(*_tmp14B)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp14A=loc;struct _fat_ptr _tmp149=({const char*_tmp6D="flag '%c' not allowed with %%n";_tag_fat(_tmp6D,sizeof(char),31U);});_tmp14B(_tmp14A,_tmp149,_tag_fat(_tmp6C,sizeof(void*),1));});});_npop_handler(0);return _tmp6F;}}}
if(precision != 0){
struct Cyc_List_List*_tmp73=({struct Cyc_String_pa_PrintArg_struct _tmp72=({struct Cyc_String_pa_PrintArg_struct _tmp104;_tmp104.tag=0,({
struct _fat_ptr _tmp14C=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(precision));_tmp104.f1=_tmp14C;});_tmp104;});void*_tmp70[1];_tmp70[0]=& _tmp72;({struct Cyc_List_List*(*_tmp14F)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 311
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp14E=loc;struct _fat_ptr _tmp14D=({const char*_tmp71="precision '%s' not allowed with %%n";_tag_fat(_tmp71,sizeof(char),36U);});_tmp14F(_tmp14E,_tmp14D,_tag_fat(_tmp70,sizeof(void*),1));});});_npop_handler(0);return _tmp73;}{
# 313
struct Cyc_List_List*_tmp74=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_List_List*tvs=_tmp74;
{struct Cyc_List_List*_tmp75=lenmod;int _tmp77;int _tmp76;if(_tmp75 == 0){
t=Cyc_Absyn_sint_type;goto _LL58;}else{if(((struct Cyc_List_List*)_tmp75)->tl == 0){_tmp76=(int)_tmp75->hd;if(_tmp76 == 108){int x=_tmp76;
# 317
t=Cyc_Absyn_ulong_type;goto _LL58;}else{_tmp76=(int)_tmp75->hd;if(_tmp76 == 104){int x=_tmp76;
t=Cyc_Absyn_sshort_type;goto _LL58;}else{goto _LL61;}}}else{if(((struct Cyc_List_List*)((struct Cyc_List_List*)_tmp75)->tl)->tl == 0){_tmp76=(int)_tmp75->hd;_tmp77=(int)(_tmp75->tl)->hd;if(
_tmp76 == 104 && _tmp77 == 104){int x=_tmp76;int y=_tmp77;
t=Cyc_Absyn_schar_type;goto _LL58;}else{goto _LL61;}}else{_LL61: {
# 322
struct Cyc_List_List*_tmp7C=({struct Cyc_String_pa_PrintArg_struct _tmp7A=({struct Cyc_String_pa_PrintArg_struct _tmp106;_tmp106.tag=0,({
struct _fat_ptr _tmp150=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmp106.f1=_tmp150;});_tmp106;});struct Cyc_Int_pa_PrintArg_struct _tmp7B=({struct Cyc_Int_pa_PrintArg_struct _tmp105;_tmp105.tag=1,_tmp105.f1=(unsigned long)((int)c);_tmp105;});void*_tmp78[2];_tmp78[0]=& _tmp7A,_tmp78[1]=& _tmp7B;({struct Cyc_List_List*(*_tmp153)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 322
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp152=loc;struct _fat_ptr _tmp151=({const char*_tmp79="length modifier '%s' is not allowed with %%%c";_tag_fat(_tmp79,sizeof(char),46U);});_tmp153(_tmp152,_tmp151,_tag_fat(_tmp78,sizeof(void*),2));});});_npop_handler(0);return _tmp7C;}}}}_LL58:;}
# 325
t=({void*_tmp157=t;void*_tmp156=Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmp7D=_cycalloc(sizeof(*_tmp7D));_tmp7D->v=tvs;_tmp7D;}));struct Cyc_Absyn_Tqual _tmp155=Cyc_Absyn_empty_tqual(0U);void*_tmp154=Cyc_Absyn_false_type;Cyc_Absyn_at_type(_tmp157,_tmp156,_tmp155,_tmp154,
Cyc_Tcutil_any_bool(tvs));});
typs=({struct Cyc_List_List*_tmp7E=_cycalloc(sizeof(*_tmp7E));_tmp7E->hd=t,_tmp7E->tl=typs;_tmp7E;});
goto _LL12;}case 37:
# 330
 if(flags != 0){
struct Cyc_List_List*_tmp82=({struct Cyc_String_pa_PrintArg_struct _tmp81=({struct Cyc_String_pa_PrintArg_struct _tmp107;_tmp107.tag=0,({
struct _fat_ptr _tmp158=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(flags));_tmp107.f1=_tmp158;});_tmp107;});void*_tmp7F[1];_tmp7F[0]=& _tmp81;({struct Cyc_List_List*(*_tmp15B)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 331
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp15A=loc;struct _fat_ptr _tmp159=({const char*_tmp80="flags '%s' not allowed with %%%%";_tag_fat(_tmp80,sizeof(char),33U);});_tmp15B(_tmp15A,_tmp159,_tag_fat(_tmp7F,sizeof(void*),1));});});_npop_handler(0);return _tmp82;}
# 333
if(width != 0){
struct Cyc_List_List*_tmp86=({struct Cyc_String_pa_PrintArg_struct _tmp85=({struct Cyc_String_pa_PrintArg_struct _tmp108;_tmp108.tag=0,({
struct _fat_ptr _tmp15C=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(width));_tmp108.f1=_tmp15C;});_tmp108;});void*_tmp83[1];_tmp83[0]=& _tmp85;({struct Cyc_List_List*(*_tmp15F)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 334
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp15E=loc;struct _fat_ptr _tmp15D=({const char*_tmp84="width '%s' not allowed with %%%%";_tag_fat(_tmp84,sizeof(char),33U);});_tmp15F(_tmp15E,_tmp15D,_tag_fat(_tmp83,sizeof(void*),1));});});_npop_handler(0);return _tmp86;}
# 336
if(precision != 0){
struct Cyc_List_List*_tmp8A=({struct Cyc_String_pa_PrintArg_struct _tmp89=({struct Cyc_String_pa_PrintArg_struct _tmp109;_tmp109.tag=0,({
struct _fat_ptr _tmp160=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(precision));_tmp109.f1=_tmp160;});_tmp109;});void*_tmp87[1];_tmp87[0]=& _tmp89;({struct Cyc_List_List*(*_tmp163)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 337
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp162=loc;struct _fat_ptr _tmp161=({const char*_tmp88="precision '%s' not allowed with %%%%";_tag_fat(_tmp88,sizeof(char),37U);});_tmp163(_tmp162,_tmp161,_tag_fat(_tmp87,sizeof(void*),1));});});_npop_handler(0);return _tmp8A;}
# 339
if(lenmod != 0){
struct Cyc_List_List*_tmp8E=({struct Cyc_String_pa_PrintArg_struct _tmp8D=({struct Cyc_String_pa_PrintArg_struct _tmp10A;_tmp10A.tag=0,({
struct _fat_ptr _tmp164=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmp10A.f1=_tmp164;});_tmp10A;});void*_tmp8B[1];_tmp8B[0]=& _tmp8D;({struct Cyc_List_List*(*_tmp167)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 340
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp166=loc;struct _fat_ptr _tmp165=({const char*_tmp8C="length modifier '%s' not allowed with %%%%";_tag_fat(_tmp8C,sizeof(char),43U);});_tmp167(_tmp166,_tmp165,_tag_fat(_tmp8B,sizeof(void*),1));});});_npop_handler(0);return _tmp8E;}
# 342
goto _LL12;default:  {
struct Cyc_List_List*_tmp8F=0;_npop_handler(0);return _tmp8F;}}_LL12:;}}}}}{
# 346
struct Cyc_List_List*_tmp90=({(struct Cyc_List_List*(*)(struct Cyc_List_List*))Cyc_List_imp_rev;})(typs);_npop_handler(0);return _tmp90;}
# 176
;_pop_region();}struct _tuple12{int f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;char f4;int f5;};
# 357 "formatstr.cyc"
struct Cyc_Core_Opt*Cyc_Formatstr_parse_inputformat(struct _RegionHandle*r,struct _fat_ptr s,int i){
# 359
unsigned long _tmp91=Cyc_strlen((struct _fat_ptr)s);unsigned long len=_tmp91;
if(i < 0 ||(unsigned long)i >= len)return 0;{
# 362
int _tmp92=0;int suppress=_tmp92;
char _tmp93=((const char*)s.curr)[i];char c=_tmp93;
if((int)c == 42){
suppress=1;
++ i;
if((unsigned long)i >= len)return 0;}{
# 370
struct Cyc_List_List*_tmp94=0;struct Cyc_List_List*width=_tmp94;
for(1;(unsigned long)i < len;++ i){
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if(isdigit((int)c))width=({struct Cyc_List_List*_tmp95=_region_malloc(r,sizeof(*_tmp95));_tmp95->hd=(void*)((int)c),_tmp95->tl=width;_tmp95;});else{
break;}}
# 376
if((unsigned long)i >= len)return 0;
width=({(struct Cyc_List_List*(*)(struct Cyc_List_List*))Cyc_List_imp_rev;})(width);{
# 381
struct Cyc_List_List*_tmp96=0;struct Cyc_List_List*lenmod=_tmp96;
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
{char _tmp97=c;switch((int)_tmp97){case 104:
# 385
 lenmod=({struct Cyc_List_List*_tmp98=_region_malloc(r,sizeof(*_tmp98));_tmp98->hd=(void*)((int)c),_tmp98->tl=lenmod;_tmp98;});
++ i;
if((unsigned long)i >= len)return 0;
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if((int)c == 104){lenmod=({struct Cyc_List_List*_tmp99=_region_malloc(r,sizeof(*_tmp99));_tmp99->hd=(void*)((int)c),_tmp99->tl=lenmod;_tmp99;});++ i;}
goto _LL0;case 108:
# 392
 lenmod=({struct Cyc_List_List*_tmp9A=_region_malloc(r,sizeof(*_tmp9A));_tmp9A->hd=(void*)((int)c),_tmp9A->tl=lenmod;_tmp9A;});
++ i;
if((unsigned long)i >= len)return 0;
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
if((int)c == 108){lenmod=({struct Cyc_List_List*_tmp9B=_region_malloc(r,sizeof(*_tmp9B));_tmp9B->hd=(void*)((int)c),_tmp9B->tl=lenmod;_tmp9B;});++ i;}
goto _LL0;case 106:
 goto _LL8;case 122: _LL8:
 goto _LLA;case 116: _LLA:
 goto _LLC;case 76: _LLC:
# 402
 lenmod=({struct Cyc_List_List*_tmp9C=_region_malloc(r,sizeof(*_tmp9C));_tmp9C->hd=(void*)((int)c),_tmp9C->tl=lenmod;_tmp9C;});
++ i;
goto _LL0;default:
 goto _LL0;}_LL0:;}
# 407
if((unsigned long)i >= len)return 0;
lenmod=({(struct Cyc_List_List*(*)(struct Cyc_List_List*))Cyc_List_imp_rev;})(lenmod);
# 411
c=*((const char*)_check_fat_subscript(s,sizeof(char),i));
{char _tmp9D=c;switch((int)_tmp9D){case 100:
 goto _LL13;case 105: _LL13:
 goto _LL15;case 111: _LL15:
 goto _LL17;case 117: _LL17:
 goto _LL19;case 120: _LL19:
 goto _LL1B;case 88: _LL1B:
 goto _LL1D;case 102: _LL1D:
 goto _LL1F;case 70: _LL1F:
 goto _LL21;case 101: _LL21:
 goto _LL23;case 69: _LL23:
 goto _LL25;case 103: _LL25:
 goto _LL27;case 71: _LL27:
 goto _LL29;case 97: _LL29:
 goto _LL2B;case 65: _LL2B:
 goto _LL2D;case 99: _LL2D:
 goto _LL2F;case 115: _LL2F:
 goto _LL31;case 112: _LL31:
 goto _LL33;case 110: _LL33:
 goto _LL35;case 37: _LL35:
 goto _LLF;default:
 return 0;}_LLF:;}
# 434
return({struct Cyc_Core_Opt*_tmp9F=_region_malloc(r,sizeof(*_tmp9F));({struct _tuple12*_tmp168=({struct _tuple12*_tmp9E=_region_malloc(r,sizeof(*_tmp9E));_tmp9E->f1=suppress,_tmp9E->f2=width,_tmp9E->f3=lenmod,_tmp9E->f4=c,_tmp9E->f5=i + 1;_tmp9E;});_tmp9F->v=_tmp168;});_tmp9F;});}}}}
# 436
struct Cyc_List_List*Cyc_Formatstr_get_scanf_types(struct Cyc_Tcenv_Tenv*te,struct _fat_ptr s,int isCproto,unsigned loc){
# 439
unsigned long _tmpA0=Cyc_strlen((struct _fat_ptr)s);unsigned long len=_tmpA0;
struct Cyc_List_List*_tmpA1=0;struct Cyc_List_List*typs=_tmpA1;
int i;
{struct _RegionHandle _tmpA2=_new_region("temp");struct _RegionHandle*temp=& _tmpA2;_push_region(temp);
for(i=0;(unsigned long)i < len;++ i){
if((int)*((const char*)_check_fat_subscript(s,sizeof(char),i))!= 37)continue;{
struct Cyc_Core_Opt*_tmpA3=Cyc_Formatstr_parse_inputformat(temp,s,i + 1);struct Cyc_Core_Opt*x=_tmpA3;
if(x == 0){
struct Cyc_List_List*_tmpA6=({void*_tmpA4=0U;({struct Cyc_List_List*(*_tmp16B)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp16A=loc;struct _fat_ptr _tmp169=({const char*_tmpA5="bad format string";_tag_fat(_tmpA5,sizeof(char),18U);});_tmp16B(_tmp16A,_tmp169,_tag_fat(_tmpA4,sizeof(void*),0));});});_npop_handler(0);return _tmpA6;}{
struct _tuple12*_tmpA7=(struct _tuple12*)x->v;struct _tuple12*_stmttmp1=_tmpA7;struct _tuple12*_tmpA8=_stmttmp1;int _tmpAD;char _tmpAC;void*_tmpAB;void*_tmpAA;int _tmpA9;_tmpA9=_tmpA8->f1;_tmpAA=_tmpA8->f2;_tmpAB=_tmpA8->f3;_tmpAC=_tmpA8->f4;_tmpAD=_tmpA8->f5;{int suppress=_tmpA9;struct Cyc_List_List*width=_tmpAA;struct Cyc_List_List*lenmod=_tmpAB;char c=_tmpAC;int j=_tmpAD;
i=j - 1;
{struct Cyc_List_List*_tmpAE=lenmod;int _tmpAF;if(_tmpAE != 0){if(((struct Cyc_List_List*)_tmpAE)->tl == 0){_tmpAF=(int)_tmpAE->hd;if(
(_tmpAF == 106 || _tmpAF == 122)|| _tmpAF == 116){int x=_tmpAF;
# 453
struct Cyc_List_List*_tmpB3=({struct Cyc_Int_pa_PrintArg_struct _tmpB2=({struct Cyc_Int_pa_PrintArg_struct _tmp10B;_tmp10B.tag=1,_tmp10B.f1=(unsigned long)x;_tmp10B;});void*_tmpB0[1];_tmpB0[0]=& _tmpB2;({struct Cyc_List_List*(*_tmp16E)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp16D=loc;struct _fat_ptr _tmp16C=({const char*_tmpB1="length modifier '%c' is not supported";_tag_fat(_tmpB1,sizeof(char),38U);});_tmp16E(_tmp16D,_tmp16C,_tag_fat(_tmpB0,sizeof(void*),1));});});_npop_handler(0);return _tmpB3;}else{goto _LL6;}}else{goto _LL6;}}else{_LL6:
 goto _LL3;}_LL3:;}
# 456
if(suppress)continue;{
void*t;
char _tmpB4=c;switch((int)_tmpB4){case 100:
 goto _LLC;case 105: _LLC: {
# 461
struct Cyc_List_List*_tmpB5=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_List_List*tvs=_tmpB5;
{struct Cyc_List_List*_tmpB6=lenmod;int _tmpB8;int _tmpB7;if(_tmpB6 == 0){
t=Cyc_Absyn_sint_type;goto _LL33;}else{if(((struct Cyc_List_List*)_tmpB6)->tl == 0){_tmpB7=(int)_tmpB6->hd;if(_tmpB7 == 108){int x=_tmpB7;
t=Cyc_Absyn_slong_type;goto _LL33;}else{_tmpB7=(int)_tmpB6->hd;if(_tmpB7 == 104){int x=_tmpB7;
t=Cyc_Absyn_sshort_type;goto _LL33;}else{goto _LL3C;}}}else{if(((struct Cyc_List_List*)((struct Cyc_List_List*)_tmpB6)->tl)->tl == 0){_tmpB7=(int)_tmpB6->hd;_tmpB8=(int)(_tmpB6->tl)->hd;if(
_tmpB7 == 104 && _tmpB8 == 104){int x=_tmpB7;int y=_tmpB8;t=Cyc_Absyn_schar_type;goto _LL33;}else{goto _LL3C;}}else{_LL3C: {
# 468
struct Cyc_List_List*_tmpBD=({struct Cyc_String_pa_PrintArg_struct _tmpBB=({struct Cyc_String_pa_PrintArg_struct _tmp10D;_tmp10D.tag=0,({
struct _fat_ptr _tmp16F=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmp10D.f1=_tmp16F;});_tmp10D;});struct Cyc_Int_pa_PrintArg_struct _tmpBC=({struct Cyc_Int_pa_PrintArg_struct _tmp10C;_tmp10C.tag=1,_tmp10C.f1=(unsigned long)((int)c);_tmp10C;});void*_tmpB9[2];_tmpB9[0]=& _tmpBB,_tmpB9[1]=& _tmpBC;({struct Cyc_List_List*(*_tmp172)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 468
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp171=loc;struct _fat_ptr _tmp170=({const char*_tmpBA="length modifier '%s' is not allowed with %%%c";_tag_fat(_tmpBA,sizeof(char),46U);});_tmp172(_tmp171,_tmp170,_tag_fat(_tmpB9,sizeof(void*),2));});});_npop_handler(0);return _tmpBD;}}}}_LL33:;}
# 471
t=({void*_tmp176=t;void*_tmp175=Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmpBE=_cycalloc(sizeof(*_tmpBE));_tmpBE->v=tvs;_tmpBE;}));struct Cyc_Absyn_Tqual _tmp174=Cyc_Absyn_empty_tqual(0U);void*_tmp173=Cyc_Absyn_false_type;Cyc_Absyn_at_type(_tmp176,_tmp175,_tmp174,_tmp173,
Cyc_Tcutil_any_bool(tvs));});
typs=({struct Cyc_List_List*_tmpBF=_cycalloc(sizeof(*_tmpBF));_tmpBF->hd=t,_tmpBF->tl=typs;_tmpBF;});
goto _LL8;}case 117:
 goto _LL10;case 111: _LL10:
 goto _LL12;case 120: _LL12:
 goto _LL14;case 88: _LL14: {
# 479
struct Cyc_List_List*_tmpC0=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_List_List*tvs=_tmpC0;
{struct Cyc_List_List*_tmpC1=lenmod;int _tmpC3;int _tmpC2;if(_tmpC1 == 0){
t=Cyc_Absyn_uint_type;goto _LL3E;}else{if(((struct Cyc_List_List*)_tmpC1)->tl == 0){_tmpC2=(int)_tmpC1->hd;if(_tmpC2 == 108){int x=_tmpC2;
t=Cyc_Absyn_ulong_type;goto _LL3E;}else{_tmpC2=(int)_tmpC1->hd;if(_tmpC2 == 104){int x=_tmpC2;
t=Cyc_Absyn_ushort_type;goto _LL3E;}else{goto _LL47;}}}else{if(((struct Cyc_List_List*)((struct Cyc_List_List*)_tmpC1)->tl)->tl == 0){_tmpC2=(int)_tmpC1->hd;_tmpC3=(int)(_tmpC1->tl)->hd;if(
_tmpC2 == 104 && _tmpC3 == 104){int x=_tmpC2;int y=_tmpC3;t=Cyc_Absyn_uchar_type;goto _LL3E;}else{goto _LL47;}}else{_LL47: {
# 486
struct Cyc_List_List*_tmpC8=({struct Cyc_String_pa_PrintArg_struct _tmpC6=({struct Cyc_String_pa_PrintArg_struct _tmp10F;_tmp10F.tag=0,({
struct _fat_ptr _tmp177=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmp10F.f1=_tmp177;});_tmp10F;});struct Cyc_Int_pa_PrintArg_struct _tmpC7=({struct Cyc_Int_pa_PrintArg_struct _tmp10E;_tmp10E.tag=1,_tmp10E.f1=(unsigned long)((int)c);_tmp10E;});void*_tmpC4[2];_tmpC4[0]=& _tmpC6,_tmpC4[1]=& _tmpC7;({struct Cyc_List_List*(*_tmp17A)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 486
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp179=loc;struct _fat_ptr _tmp178=({const char*_tmpC5="length modifier '%s' is not allowed with %%%c";_tag_fat(_tmpC5,sizeof(char),46U);});_tmp17A(_tmp179,_tmp178,_tag_fat(_tmpC4,sizeof(void*),2));});});_npop_handler(0);return _tmpC8;}}}}_LL3E:;}
# 489
t=({void*_tmp17E=t;void*_tmp17D=Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmpC9=_cycalloc(sizeof(*_tmpC9));_tmpC9->v=tvs;_tmpC9;}));struct Cyc_Absyn_Tqual _tmp17C=Cyc_Absyn_empty_tqual(0U);void*_tmp17B=Cyc_Absyn_false_type;Cyc_Absyn_at_type(_tmp17E,_tmp17D,_tmp17C,_tmp17B,
Cyc_Tcutil_any_bool(tvs));});
typs=({struct Cyc_List_List*_tmpCA=_cycalloc(sizeof(*_tmpCA));_tmpCA->hd=t,_tmpCA->tl=typs;_tmpCA;});
goto _LL8;}case 102:
 goto _LL18;case 70: _LL18:
 goto _LL1A;case 101: _LL1A:
 goto _LL1C;case 69: _LL1C:
 goto _LL1E;case 103: _LL1E:
 goto _LL20;case 71: _LL20:
 goto _LL22;case 97: _LL22:
 goto _LL24;case 65: _LL24: {
# 501
struct Cyc_List_List*_tmpCB=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_List_List*tvs=_tmpCB;
{struct Cyc_List_List*_tmpCC=lenmod;int _tmpCD;if(_tmpCC == 0){
t=Cyc_Absyn_float_type;goto _LL49;}else{if(((struct Cyc_List_List*)_tmpCC)->tl == 0){_tmpCD=(int)_tmpCC->hd;if(_tmpCD == 108){int x=_tmpCD;
# 505
t=Cyc_Absyn_double_type;goto _LL49;}else{goto _LL4E;}}else{_LL4E: {
# 507
struct Cyc_List_List*_tmpD2=({struct Cyc_String_pa_PrintArg_struct _tmpD0=({struct Cyc_String_pa_PrintArg_struct _tmp111;_tmp111.tag=0,({
struct _fat_ptr _tmp17F=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmp111.f1=_tmp17F;});_tmp111;});struct Cyc_Int_pa_PrintArg_struct _tmpD1=({struct Cyc_Int_pa_PrintArg_struct _tmp110;_tmp110.tag=1,_tmp110.f1=(unsigned long)((int)c);_tmp110;});void*_tmpCE[2];_tmpCE[0]=& _tmpD0,_tmpCE[1]=& _tmpD1;({struct Cyc_List_List*(*_tmp182)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 507
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp181=loc;struct _fat_ptr _tmp180=({const char*_tmpCF="length modifier '%s' is not allowed with %%%c";_tag_fat(_tmpCF,sizeof(char),46U);});_tmp182(_tmp181,_tmp180,_tag_fat(_tmpCE,sizeof(void*),2));});});_npop_handler(0);return _tmpD2;}}}_LL49:;}
# 510
t=({void*_tmp186=t;void*_tmp185=Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmpD3=_cycalloc(sizeof(*_tmpD3));_tmpD3->v=tvs;_tmpD3;}));struct Cyc_Absyn_Tqual _tmp184=Cyc_Absyn_empty_tqual(0U);void*_tmp183=Cyc_Absyn_false_type;Cyc_Absyn_at_type(_tmp186,_tmp185,_tmp184,_tmp183,
Cyc_Tcutil_any_bool(tvs));});
typs=({struct Cyc_List_List*_tmpD4=_cycalloc(sizeof(*_tmpD4));_tmpD4->hd=t,_tmpD4->tl=typs;_tmpD4;});
goto _LL8;}case 99:  {
# 516
struct Cyc_List_List*_tmpD5=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_List_List*tvs=_tmpD5;
void*ptr;
if(!isCproto)
ptr=({void*_tmp18A=Cyc_Absyn_char_type;void*_tmp189=Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmpD6=_cycalloc(sizeof(*_tmpD6));_tmpD6->v=tvs;_tmpD6;}));struct Cyc_Absyn_Tqual _tmp188=
Cyc_Absyn_empty_tqual(0U);
# 519
void*_tmp187=Cyc_Absyn_false_type;Cyc_Absyn_fatptr_type(_tmp18A,_tmp189,_tmp188,_tmp187,
# 521
Cyc_Tcutil_any_bool(tvs));});else{
# 523
ptr=({void*_tmp18E=Cyc_Absyn_char_type;void*_tmp18D=
Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmpD7=_cycalloc(sizeof(*_tmpD7));_tmpD7->v=tvs;_tmpD7;}));
# 523
struct Cyc_Absyn_Tqual _tmp18C=
# 525
Cyc_Absyn_empty_tqual(0U);
# 523
void*_tmp18B=Cyc_Absyn_false_type;Cyc_Absyn_at_type(_tmp18E,_tmp18D,_tmp18C,_tmp18B,
# 525
Cyc_Tcutil_any_bool(tvs));});}
typs=({struct Cyc_List_List*_tmpD8=_cycalloc(sizeof(*_tmpD8));_tmpD8->hd=ptr,_tmpD8->tl=typs;_tmpD8;});
goto _LL8;}case 115:  {
# 529
struct Cyc_List_List*_tmpD9=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_List_List*tvs=_tmpD9;
# 531
void*ptr;
if(!isCproto)
ptr=({void*_tmp192=Cyc_Absyn_char_type;void*_tmp191=Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmpDA=_cycalloc(sizeof(*_tmpDA));_tmpDA->v=tvs;_tmpDA;}));struct Cyc_Absyn_Tqual _tmp190=
Cyc_Absyn_empty_tqual(0U);
# 533
void*_tmp18F=Cyc_Absyn_false_type;Cyc_Absyn_fatptr_type(_tmp192,_tmp191,_tmp190,_tmp18F,
Cyc_Tcutil_any_bool(tvs));});else{
# 536
ptr=({void*_tmp196=Cyc_Absyn_char_type;void*_tmp195=Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmpDB=_cycalloc(sizeof(*_tmpDB));_tmpDB->v=tvs;_tmpDB;}));struct Cyc_Absyn_Tqual _tmp194=
Cyc_Absyn_empty_tqual(0U);
# 536
void*_tmp193=Cyc_Absyn_true_type;Cyc_Absyn_at_type(_tmp196,_tmp195,_tmp194,_tmp193,
Cyc_Tcutil_any_bool(tvs));});}
typs=({struct Cyc_List_List*_tmpDC=_cycalloc(sizeof(*_tmpDC));_tmpDC->hd=ptr,_tmpDC->tl=typs;_tmpDC;});
goto _LL8;}case 91:
 goto _LL2C;case 112: _LL2C: {
# 542
struct Cyc_List_List*_tmpE0=({struct Cyc_Int_pa_PrintArg_struct _tmpDF=({struct Cyc_Int_pa_PrintArg_struct _tmp112;_tmp112.tag=1,_tmp112.f1=(unsigned long)((int)c);_tmp112;});void*_tmpDD[1];_tmpDD[0]=& _tmpDF;({struct Cyc_List_List*(*_tmp199)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp198=loc;struct _fat_ptr _tmp197=({const char*_tmpDE="%%%c is not supported";_tag_fat(_tmpDE,sizeof(char),22U);});_tmp199(_tmp198,_tmp197,_tag_fat(_tmpDD,sizeof(void*),1));});});_npop_handler(0);return _tmpE0;}case 110:  {
# 544
struct Cyc_List_List*_tmpE1=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_List_List*tvs=_tmpE1;
{struct Cyc_List_List*_tmpE2=lenmod;int _tmpE4;int _tmpE3;if(_tmpE2 == 0){
t=Cyc_Absyn_sint_type;goto _LL50;}else{if(((struct Cyc_List_List*)_tmpE2)->tl == 0){_tmpE3=(int)_tmpE2->hd;if(_tmpE3 == 108){int x=_tmpE3;
t=Cyc_Absyn_ulong_type;goto _LL50;}else{_tmpE3=(int)_tmpE2->hd;if(_tmpE3 == 104){int x=_tmpE3;
t=Cyc_Absyn_sshort_type;goto _LL50;}else{goto _LL59;}}}else{if(((struct Cyc_List_List*)((struct Cyc_List_List*)_tmpE2)->tl)->tl == 0){_tmpE3=(int)_tmpE2->hd;_tmpE4=(int)(_tmpE2->tl)->hd;if(
_tmpE3 == 104 && _tmpE4 == 104){int x=_tmpE3;int y=_tmpE4;t=Cyc_Absyn_schar_type;goto _LL50;}else{goto _LL59;}}else{_LL59: {
# 551
struct Cyc_List_List*_tmpE9=({struct Cyc_String_pa_PrintArg_struct _tmpE7=({struct Cyc_String_pa_PrintArg_struct _tmp114;_tmp114.tag=0,({
struct _fat_ptr _tmp19A=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmp114.f1=_tmp19A;});_tmp114;});struct Cyc_Int_pa_PrintArg_struct _tmpE8=({struct Cyc_Int_pa_PrintArg_struct _tmp113;_tmp113.tag=1,_tmp113.f1=(unsigned long)((int)c);_tmp113;});void*_tmpE5[2];_tmpE5[0]=& _tmpE7,_tmpE5[1]=& _tmpE8;({struct Cyc_List_List*(*_tmp19D)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 551
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp19C=loc;struct _fat_ptr _tmp19B=({const char*_tmpE6="length modifier '%s' is not allowed with %%%c";_tag_fat(_tmpE6,sizeof(char),46U);});_tmp19D(_tmp19C,_tmp19B,_tag_fat(_tmpE5,sizeof(void*),2));});});_npop_handler(0);return _tmpE9;}}}}_LL50:;}
# 554
t=({void*_tmp1A1=t;void*_tmp1A0=Cyc_Absyn_new_evar(& Cyc_Kinds_rko,({struct Cyc_Core_Opt*_tmpEA=_cycalloc(sizeof(*_tmpEA));_tmpEA->v=tvs;_tmpEA;}));struct Cyc_Absyn_Tqual _tmp19F=Cyc_Absyn_empty_tqual(0U);void*_tmp19E=Cyc_Absyn_false_type;Cyc_Absyn_at_type(_tmp1A1,_tmp1A0,_tmp19F,_tmp19E,
Cyc_Tcutil_any_bool(tvs));});
typs=({struct Cyc_List_List*_tmpEB=_cycalloc(sizeof(*_tmpEB));_tmpEB->hd=t,_tmpEB->tl=typs;_tmpEB;});
goto _LL8;}case 37:
# 559
 if(suppress){
struct Cyc_List_List*_tmpEE=({void*_tmpEC=0U;({struct Cyc_List_List*(*_tmp1A4)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp1A3=loc;struct _fat_ptr _tmp1A2=({const char*_tmpED="Assignment suppression (*) is not allowed with %%%%";_tag_fat(_tmpED,sizeof(char),52U);});_tmp1A4(_tmp1A3,_tmp1A2,_tag_fat(_tmpEC,sizeof(void*),0));});});_npop_handler(0);return _tmpEE;}
if(width != 0){
struct Cyc_List_List*_tmpF2=({struct Cyc_String_pa_PrintArg_struct _tmpF1=({struct Cyc_String_pa_PrintArg_struct _tmp115;_tmp115.tag=0,({struct _fat_ptr _tmp1A5=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(width));_tmp115.f1=_tmp1A5;});_tmp115;});void*_tmpEF[1];_tmpEF[0]=& _tmpF1;({struct Cyc_List_List*(*_tmp1A8)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp1A7=loc;struct _fat_ptr _tmp1A6=({const char*_tmpF0="width '%s' not allowed with %%%%";_tag_fat(_tmpF0,sizeof(char),33U);});_tmp1A8(_tmp1A7,_tmp1A6,_tag_fat(_tmpEF,sizeof(void*),1));});});_npop_handler(0);return _tmpF2;}
if(lenmod != 0){
struct Cyc_List_List*_tmpF6=({struct Cyc_String_pa_PrintArg_struct _tmpF5=({struct Cyc_String_pa_PrintArg_struct _tmp116;_tmp116.tag=0,({
struct _fat_ptr _tmp1A9=(struct _fat_ptr)((struct _fat_ptr)Cyc_implode(lenmod));_tmp116.f1=_tmp1A9;});_tmp116;});void*_tmpF3[1];_tmpF3[0]=& _tmpF5;({struct Cyc_List_List*(*_tmp1AC)(unsigned,struct _fat_ptr,struct _fat_ptr ap)=({
# 564
(struct Cyc_List_List*(*)(unsigned,struct _fat_ptr,struct _fat_ptr ap))Cyc_Formatstr_err_null;});unsigned _tmp1AB=loc;struct _fat_ptr _tmp1AA=({const char*_tmpF4="length modifier '%s' not allowed with %%%%";_tag_fat(_tmpF4,sizeof(char),43U);});_tmp1AC(_tmp1AB,_tmp1AA,_tag_fat(_tmpF3,sizeof(void*),1));});});_npop_handler(0);return _tmpF6;}
# 566
goto _LL8;default:  {
struct Cyc_List_List*_tmpF7=0;_npop_handler(0);return _tmpF7;}}_LL8:;}}}}}
# 443
;_pop_region();}
# 571
return({(struct Cyc_List_List*(*)(struct Cyc_List_List*))Cyc_List_imp_rev;})(typs);}
