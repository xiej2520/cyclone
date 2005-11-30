typedef int size_t;
extern	struct	_iobuf {
int	_cnt;
unsigned char *_ptr;
unsigned char *_base;
int	_bufsiz;
short	_flag;
char	_file;	} _iob[];
typedef struct _iobuf FILE;
extern struct _iobuf	*fopen(const char *, const char *);
extern struct _iobuf	*fdopen(int, const char *);
extern struct _iobuf	*freopen(const char *, const char *, FILE *);
extern struct _iobuf	*popen(const char *, const char *);
extern struct _iobuf	*tmpfile(void);
extern long	ftell(FILE *);
extern char	*fgets(char *, int, FILE *);
extern char	*gets(char *);
extern char	*sprintf(char *, const char *, ...);
extern char	*ctermid(char *);
extern char	*cuserid(char *);
extern char	*tempnam(const char *, const char *);
extern char	*tmpnam(char *);
typedef struct sm_element_struct sm_element;
typedef struct sm_row_struct sm_row;
typedef struct sm_col_struct sm_col;
typedef struct sm_matrix_struct sm_matrix;
struct sm_element_struct {
int row_num;	int col_num;	sm_element *next_row;	sm_element *prev_row;	sm_element *next_col;	sm_element *prev_col;	char *user_word;	};
struct sm_row_struct {
int row_num;	int length;	int flag;	sm_element *first_col;	sm_element *last_col;	sm_row *next_row;	sm_row *prev_row;	char *user_word;	};
struct sm_col_struct {
int col_num;	int length;	int flag;	sm_element *first_row;	sm_element *last_row;	sm_col *next_col;	sm_col *prev_col;	char *user_word;	};
struct sm_matrix_struct {
sm_row **rows;	int rows_size;	sm_col **cols;	int cols_size;	sm_row *first_row;	sm_row *last_row;	int nrows;	sm_col *first_col;	sm_col *last_col;	int ncols;	char *user_word;	};
extern sm_matrix *sm_alloc(), *sm_alloc_size(), *sm_dup();
extern void sm_free(), sm_delrow(), sm_delcol(), sm_resize();
extern void sm_write(), sm_print(), sm_dump(), sm_cleanup();
extern void sm_copy_row(), sm_copy_col();
extern void sm_remove(), sm_remove_element();
extern sm_element *sm_insert(), *sm_find();
extern sm_row *sm_longest_row();
extern sm_col *sm_longest_col();
extern int sm_read(), sm_read_compressed();
extern sm_row *sm_row_alloc(), *sm_row_dup(), *sm_row_and();
extern void sm_row_free(), sm_row_remove(), sm_row_print();
extern sm_element *sm_row_insert(), *sm_row_find();
extern int sm_row_contains(), sm_row_intersects();
extern int sm_row_compare(), sm_row_hash();
extern sm_col *sm_col_alloc(), *sm_col_dup(), *sm_col_and();
extern void sm_col_free(), sm_col_remove(), sm_col_print();
extern sm_element *sm_col_insert(), *sm_col_find();
extern int sm_col_contains(), sm_col_intersects();
extern int sm_col_compare(), sm_col_hash();
extern int sm_row_dominance(), sm_col_dominance(), sm_block_partition();
extern sm_row *sm_minimum_cover();
extern	char	_ctype_[];
extern struct _iobuf *popen(const char *, const char *), *tmpfile(void);
extern int pclose(FILE *);
extern void rewind(FILE *);
extern void abort(void), free(void *), exit(int), perror(const char *);
extern char *getenv(const char *), *malloc(size_t), *realloc(void *, size_t);
extern int system(const char *);
extern double atof(const char *);
extern char *strcpy(char *, const char *), *strncpy(char *, const char *, size_t), *strcat(char *, const char *), *strncat(char *, const char *, size_t), *strerror(int);
extern char *strpbrk(const char *, const char *), *strtok(char *, const char *), *strchr(const char *, int), *strrchr(const char *, int), *strstr(const char *, const char *);
extern int strcoll(const char *, const char *), strxfrm(char *, const char *, size_t), strncmp(const char *, const char *, size_t), strlen(const char *), strspn(const char *, const char *), strcspn(const char *, const char *);
extern char *memmove(void *, const void *, size_t), *memccpy(void *, const void *, int, size_t), *memchr(const void *, int, size_t), *memcpy(void *, const void *, size_t), *memset(void *, int, size_t);
extern int memcmp(const void *, const void *, size_t), strcmp(const char *, const char *);
extern long util_cpu_time();
extern int util_getopt();
extern void util_getopt_reset();
extern char *util_path_search();
extern char *util_file_search();
extern int util_pipefork();
extern void util_print_cpu_stats();
extern char *util_print_time();
extern int util_save_image();
extern char *util_strsav();
extern char *util_tilde_expand();
extern void util_restart();
extern int util_optind;
extern char *util_optarg;

typedef unsigned int *pset;
typedef struct set_family {
int wsize; int sf_size; int capacity; int count; int active_count; pset data; struct set_family *next; } set_family_t, *pset_family;
extern int bit_count[256];

typedef struct cost_struct {
int cubes;	int in;	int out;	int mv;	int total;	int primes;	} cost_t, *pcost;
typedef struct pair_struct {
int cnt;
int *var1;
int *var2;
} pair_t, *ppair;
typedef struct symbolic_list_struct {
int variable;
int pos;
struct symbolic_list_struct *next;
} symbolic_list_t;
typedef struct symbolic_label_struct {
char *label;
struct symbolic_label_struct *next;
} symbolic_label_t;
typedef struct symbolic_struct {
symbolic_list_t *symbolic_list;	int symbolic_list_length;	symbolic_label_t *symbolic_label;	int symbolic_label_length;	struct symbolic_struct *next;
} symbolic_t;
typedef struct {
pset_family F, D, R;	char *filename; int pla_type; pset phase; ppair pair; char **label;	symbolic_t *symbolic;	symbolic_t *symbolic_output;
} PLA_t, *pPLA;

extern unsigned int debug; extern int verbose_debug; extern char *total_name[16]; extern long total_time[16]; extern int total_calls[16]; extern int echo_comments;	extern int echo_unknown_commands;	extern int force_irredundant; extern int skip_make_sparse;
extern int kiss; extern int pos; extern int print_solution; extern int recompute_onset; extern int remove_essential; extern int single_expand; extern int summary; extern int trace; extern int unwrap_onset; extern int use_random_order;	extern int use_super_gasp;	extern char *filename;	extern int debug_exact_minimization; struct pla_types_struct {
char *key;
int value;
};
struct cube_struct {
int size; int num_vars; int num_binary_vars; int *first_part; int *last_part; int *part_size; int *first_word; int *last_word; pset binary_mask; pset mv_mask; pset *var_mask; pset *temp; pset fullset; pset emptyset; unsigned int inmask; int inword; int *sparse; int num_mv_vars; int output; };
struct cdata_struct {
int *part_zeros; int *var_zeros; int *parts_active; int *is_unate; int vars_active; int vars_unate; int best; };
extern struct pla_types_struct pla_types[];
extern struct cube_struct cube, temp_cube_save;
extern struct cdata_struct cdata, temp_cdata_save;

extern int binate_split_select(pset *T, register pset cleft, register pset cright, int debug_flag);
extern pset_family cubeunlist(pset *A1);
extern pset *cofactor(pset *T, register pset c);
extern pset *cube1list(pset_family A);
extern pset *cube2list(pset_family A, pset_family B);
extern pset *cube3list(pset_family A, pset_family B, pset_family C);
extern pset *scofactor(pset *T, pset c, int var);
extern void massive_count(pset *T);
extern pset_family complement();
extern pset_family simplify();
extern void simp_comp();
extern int d1_rm_equal();
extern int rm2_contain();
extern int rm2_equal();
extern int rm_contain();
extern int rm_equal();
extern int rm_rev_contain();
extern pset *sf_list();
extern pset *sf_sort();
extern pset_family d1merge();
extern pset_family dist_merge();
extern pset_family sf_contain();
extern pset_family sf_dupl();
extern pset_family sf_ind_contain();
extern pset_family sf_ind_unlist();
extern pset_family sf_merge();
extern pset_family sf_rev_contain();
extern pset_family sf_union();
extern pset_family sf_unlist();
extern void cube_setup();
extern void restore_cube_struct();
extern void save_cube_struct();
extern void setdown_cube();
extern PLA_labels();
extern char *get_word();
extern int label_index();
extern int read_pla();
extern int read_symbolic();
extern pPLA new_PLA();
extern void PLA_summary();
extern void free_PLA();
extern void parse_pla();
extern void read_cube();
extern void skip_line();
extern foreach_output_function();
extern int cubelist_partition();
extern int so_both_do_espresso();
extern int so_both_do_exact();
extern int so_both_save();
extern int so_do_espresso();
extern int so_do_exact();
extern int so_save();
extern pset_family cof_output();
extern pset_family lex_sort();
extern pset_family mini_sort();
extern pset_family random_order();
extern pset_family size_sort();
extern pset_family sort_reduce();
extern pset_family uncof_output();
extern pset_family unravel();
extern pset_family unravel_range();
extern void so_both_espresso();
extern void so_espresso();
extern char *fmt_cost();
extern char *print_cost();
extern char *util_strsav();
extern void copy_cost();
extern void cover_cost();
extern void fatal();
extern void print_trace();
extern void size_stamp();
extern void totals();
extern char *fmt_cube();
extern char *fmt_expanded_cube();
extern char *pc1();
extern char *pc2();
extern char *pc3();
extern int makeup_labels();
extern kiss_output();
extern kiss_print_cube();
extern output_symbolic_constraints();
extern void cprint();
extern void debug1_print();
extern void debug_print();
extern void eqn_output();
extern void fpr_header();
extern void fprint_pla();
extern void pls_group();
extern void pls_label();
extern void pls_output();
extern void print_cube();
extern void print_expanded_cube();
extern void sf_debug_print();
extern find_equiv_outputs();
extern int check_equiv();
extern pset_family espresso();
extern int essen_cube();
extern pset_family cb_consensus();
extern pset_family cb_consensus_dist0();
extern pset_family essential();
extern pset_family minimize_exact();
extern pset_family minimize_exact_literals();
extern int feasibly_covered();
extern int most_frequent();
extern pset_family all_primes();
extern pset_family expand();
extern pset_family find_all_primes();
extern void elim_lowering();
extern void essen_parts();
extern void essen_raising();
extern void expand1();
extern void mincov();
extern void select_feasible();
extern void setup_BB_CC();
extern pset_family expand_gasp();
extern pset_family irred_gasp();
extern pset_family last_gasp();
extern pset_family super_gasp();
extern void expand1_gasp();
extern int util_getopt();
extern find_dc_inputs();
extern find_inputs();
extern form_bitvector();
extern map_dcset();
extern map_output_symbolic();
extern map_symbolic();
extern pset_family map_symbolic_cover();
extern symbolic_hack_labels();
extern int cube_is_covered();
extern int taut_special_cases();
extern int tautology();
extern pset_family irredundant();
extern void mark_irredundant();
extern void irred_split_cover();
extern sm_matrix *irred_derive_table();
extern pset minterms();
extern void explode();
extern void map();
extern output_phase_setup();
extern pPLA set_phase();
extern pset_family opo();
extern pset find_phase();
extern pset_family find_covers();
extern pset_family form_cover_table();
extern pset_family opo_leaf();
extern pset_family opo_recur();
extern void opoall();
extern void phase_assignment();
extern void repeated_phase_assignment();
extern generate_all_pairs();
extern int **find_pairing_cost();
extern int find_best_cost();
extern int greedy_best_cost();
extern int minimize_pair();
extern int pair_free();
extern pair_all();
extern pset_family delvar();
extern pset_family pairvar();
extern ppair pair_best_cost();
extern ppair pair_new();
extern ppair pair_save();
extern print_pair();
extern void find_optimal_pairing();
extern void set_pair();
extern void set_pair1();
extern pset_family primes_consensus();
extern int sccc_special_cases();
extern pset_family reduce();
extern pset reduce_cube();
extern pset sccc();
extern pset sccc_cube();
extern pset sccc_merge();
extern int set_andp();
extern int set_orp();
extern int setp_disjoint();
extern int setp_empty();
extern int setp_equal();
extern int setp_full();
extern int setp_implies();
extern char *pbv1();
extern char *ps1();
extern int *sf_count();
extern int *sf_count_restricted();
extern int bit_index();
extern int set_dist();
extern int set_ord();
extern void set_adjcnt();
extern pset set_and();
extern pset set_clear();
extern pset set_copy();
extern pset set_diff();
extern pset set_fill();
extern pset set_merge();
extern pset set_or();
extern pset set_xor();
extern pset sf_and();
extern pset sf_or();
extern pset_family sf_active();
extern pset_family sf_addcol();
extern pset_family sf_addset();
extern pset_family sf_append();
extern pset_family sf_bm_read();
extern pset_family sf_compress();
extern pset_family sf_copy();
extern pset_family sf_copy_col();
extern pset_family sf_delc();
extern pset_family sf_delcol();
extern pset_family sf_inactive();
extern pset_family sf_join();
extern pset_family sf_new();
extern pset_family sf_permute();
extern pset_family sf_read();
extern pset_family sf_save();
extern pset_family sf_transpose();
extern void set_write();
extern void sf_bm_print();
extern void sf_cleanup();
extern void sf_delset();
extern void sf_free();
extern void sf_print();
extern void sf_write();
extern int ccommon();
extern int cdist0();
extern int full_row();
extern int ascend();
extern int cactive();
extern int cdist();
extern int cdist01();
extern int cvolume();
extern int d1_order();
extern int d1_order_size();
extern int desc1();
extern int descend();
extern int lex_order();
extern int lex_order1();
extern pset force_lower();
extern void consensus();
extern pset_family cb1_dsharp();
extern pset_family cb_dsharp();
extern pset_family cb_recur_dsharp();
extern pset_family cb_recur_sharp();
extern pset_family cb_sharp();
extern pset_family cv_dsharp();
extern pset_family cv_intersect();
extern pset_family cv_sharp();
extern pset_family dsharp();
extern pset_family make_disjoint();
extern pset_family sharp();
pset do_sm_minimum_cover();
extern pset_family make_sparse();
extern pset_family mv_reduce();
extern qsort(void *, size_t, size_t, int (*) (const void *, const void *));
extern qst();
extern pset_family find_all_minimal_covers_petrick();
extern pset_family map_cover_to_unate();
extern pset_family map_unate_to_cover();
extern pset_family exact_minimum_cover();
extern pset_family gen_primes();
extern pset_family unate_compl();
extern pset_family unate_complement();
extern pset_family unate_intersect();
extern PLA_permute();
extern int PLA_verify();
extern int check_consistency();
extern int verify();
pset *cofactor(pset *T, register pset c)
{
pset temp = cube.temp[0], *Tc_save, *Tc, *T1;
register pset p;
int listlen;
listlen = (((pset *) T[1] - T) - 3) + 5;
Tc_save = Tc = ((pset *) malloc(sizeof(pset) * ( listlen)));
*Tc++ = set_or(set_clear(((unsigned int *) malloc(sizeof(unsigned int) * ( ((cube.size) <= 32 ? 2 : (((((cube.size)-1) >> 5) + 1) + 1))))), cube.size), T[0], set_diff(temp, cube.fullset, c));
Tc++;
for(T1 = T+2; (p = *T1++) != 0; ) {
if (p != c) {
{register int w,last;register unsigned int x;if((last=cube.inword)!=-1)
{x=p[last]&c[last];if(~(x|x>>1)&cube.inmask)goto false;for(w=1;w<last;w++)
{x=p[w]&c[w];if(~(x|x>>1)&0x55555555)goto false;}}}{register int w,var,last;
register pset mask;for(var=cube.num_binary_vars;var<cube.num_vars;var++){
mask=cube.var_mask[var];last=cube.last_word[var];for(w=cube.first_word[var
];w<=last;w++)if(p[w]&c[w]&mask[w])goto nextvar;goto false;nextvar:;}}
*Tc++ = p;
false: ;
}
}
*Tc++ = (pset) 0; Tc_save[1] = (pset) Tc; return Tc_save;
}

pset *scofactor(pset *T, pset c, int var)
{
pset *Tc, *Tc_save;
register pset p, mask = cube.temp[1], *T1;
register int first = cube.first_word[var], last = cube.last_word[var];
int listlen;
listlen = (((pset *) T[1] - T) - 3) + 5;
Tc_save = Tc = ((pset *) malloc(sizeof(pset) * ( listlen)));
*Tc++ = set_or(set_clear(((unsigned int *) malloc(sizeof(unsigned int) * ( ((cube.size) <= 32 ? 2 : (((((cube.size)-1) >> 5) + 1) + 1))))), cube.size), T[0], set_diff(mask, cube.fullset, c));
Tc++;
(void) set_and(mask, cube.var_mask[var], c);
for(T1 = T+2; (p = *T1++) != 0; )
if (p != c) {
register int i = first;
do
if (p[i] & mask[i]) {
*Tc++ = p;
break;
}
while (++i <= last);
}
*Tc++ = (pset) 0; Tc_save[1] = (pset) Tc; return Tc_save;
}

void massive_count(pset *T)
{
int *count = cdata.part_zeros;
pset *T1;
{ register int i;
for(i = cube.size - 1; i >= 0; i--)
count[i] = 0;
}
{ register int i, *cnt;
register unsigned int val;
register pset p, cof = T[0], full = cube.fullset;
for(T1 = T+2; (p = *T1++) != 0; )
for(i = (p[0] & 0x03ff); i > 0; i--)
if (val = full[i] & ~ (p[i] | cof[i])) {
cnt = count + ((i-1) << 5);
if (val & 0xFF000000) {
if (val & 0x80000000) cnt[31]++;
if (val & 0x40000000) cnt[30]++;
if (val & 0x20000000) cnt[29]++;
if (val & 0x10000000) cnt[28]++;
if (val & 0x08000000) cnt[27]++;
if (val & 0x04000000) cnt[26]++;
if (val & 0x02000000) cnt[25]++;
if (val & 0x01000000) cnt[24]++;
}
if (val & 0x00FF0000) {
if (val & 0x00800000) cnt[23]++;
if (val & 0x00400000) cnt[22]++;
if (val & 0x00200000) cnt[21]++;
if (val & 0x00100000) cnt[20]++;
if (val & 0x00080000) cnt[19]++;
if (val & 0x00040000) cnt[18]++;
if (val & 0x00020000) cnt[17]++;
if (val & 0x00010000) cnt[16]++;
}
if (val & 0xFF00) {
if (val & 0x8000) cnt[15]++;
if (val & 0x4000) cnt[14]++;
if (val & 0x2000) cnt[13]++;
if (val & 0x1000) cnt[12]++;
if (val & 0x0800) cnt[11]++;
if (val & 0x0400) cnt[10]++;
if (val & 0x0200) cnt[ 9]++;
if (val & 0x0100) cnt[ 8]++;
}
if (val & 0x00FF) {
if (val & 0x0080) cnt[ 7]++;
if (val & 0x0040) cnt[ 6]++;
if (val & 0x0020) cnt[ 5]++;
if (val & 0x0010) cnt[ 4]++;
if (val & 0x0008) cnt[ 3]++;
if (val & 0x0004) cnt[ 2]++;
if (val & 0x0002) cnt[ 1]++;
if (val & 0x0001) cnt[ 0]++;
}
}
}
{ register int var, i, lastbit, active, maxactive;
int best = -1, mostactive = 0, mostzero = 0, mostbalanced = 32000;
cdata.vars_unate = cdata.vars_active = 0;
for(var = 0; var < cube.num_vars; var++) {
if (var < cube.num_binary_vars) { i = count[var*2];
lastbit = count[var*2 + 1];
active = (i > 0) + (lastbit > 0);
cdata.var_zeros[var] = i + lastbit;
maxactive = ((i) > ( lastbit) ? (i) : ( lastbit));
} else {
maxactive = active = cdata.var_zeros[var] = 0;
lastbit = cube.last_part[var];
for(i = cube.first_part[var]; i <= lastbit; i++) {
cdata.var_zeros[var] += count[i];
active += (count[i] > 0);
if (active > maxactive) maxactive = active;
}
}
if (active > mostactive)
best = var, mostactive = active, mostzero = cdata.var_zeros[best],
mostbalanced = maxactive;
else if (active == mostactive)
if (cdata.var_zeros[var] > mostzero)
best = var, mostzero = cdata.var_zeros[best],
mostbalanced = maxactive;
else if (cdata.var_zeros[var] == mostzero)
if (maxactive < mostbalanced)
best = var, mostbalanced = maxactive;
cdata.parts_active[var] = active;
cdata.is_unate[var] = (active == 1);
cdata.vars_active += (active > 0);
cdata.vars_unate += (active == 1);
}
cdata.best = best;
}
}

int binate_split_select(pset *T, register pset cleft, register pset cright, int debug_flag)
{
int best = cdata.best;
register int i, lastbit = cube.last_part[best], halfbit = 0;
register pset cof=T[0];
set_diff(cleft, cube.fullset, cube.var_mask[best]);
set_diff(cright, cube.fullset, cube.var_mask[best]);
for(i = cube.first_part[best]; i <= lastbit; i++)
if (! (cof[(((i) >> 5) + 1)] & (1 << ((i) & (32-1)))))
halfbit++;
for(i = cube.first_part[best], halfbit = halfbit/2; halfbit > 0; i++)
if (! (cof[(((i) >> 5) + 1)] & (1 << ((i) & (32-1)))))
halfbit--, (cleft[((( i) >> 5) + 1)] |= 1 << (( i) & (32-1)));
for(; i <= lastbit; i++)
if (! (cof[(((i) >> 5) + 1)] & (1 << ((i) & (32-1)))))
(cright[((( i) >> 5) + 1)] |= 1 << (( i) & (32-1)));
if (debug & debug_flag) {
printf("BINATE_SPLIT_SELECT: split against %d\n", best);
if (verbose_debug)
printf("cl=%s\ncr=%s\n", pc1(cleft), pc2(cright));
}
return best;
}
pset *cube1list(pset_family A)
{
register pset last, p, *plist, *list;
list = plist = ((pset *) malloc(sizeof(pset) * ( A->count + 3)));
*plist++ = set_clear(((unsigned int *) malloc(sizeof(unsigned int) * ( ((cube.size) <= 32 ? 2 : (((((cube.size)-1) >> 5) + 1) + 1))))), cube.size);
plist++;
for( p=A->data, last= p+A->count*A->wsize; p< last; p+=A->wsize) {
*plist++ = p;
}
*plist++ = 0; list[1] = (pset) plist;
return list;
}
pset *cube2list(pset_family A, pset_family B)
{
register pset last, p, *plist, *list;
list = plist = ((pset *) malloc(sizeof(pset) * ( A->count + B->count + 3)));
*plist++ = set_clear(((unsigned int *) malloc(sizeof(unsigned int) * ( ((cube.size) <= 32 ? 2 : (((((cube.size)-1) >> 5) + 1) + 1))))), cube.size);
plist++;
for( p=A->data, last= p+A->count*A->wsize; p< last; p+=A->wsize) {
*plist++ = p;
}
for( p=B->data, last= p+B->count*B->wsize; p< last; p+=B->wsize) {
*plist++ = p;
}
*plist++ = 0;
list[1] = (pset) plist;
return list;
}
pset *cube3list(pset_family A, pset_family B, pset_family C)
{
register pset last, p, *plist, *list;
plist = ((pset *) malloc(sizeof(pset) * ( A->count + B->count + C->count + 3)));
list = plist;
*plist++ = set_clear(((unsigned int *) malloc(sizeof(unsigned int) * ( ((cube.size) <= 32 ? 2 : (((((cube.size)-1) >> 5) + 1) + 1))))), cube.size);
plist++;
for( p=A->data, last= p+A->count*A->wsize; p< last; p+=A->wsize) {
*plist++ = p;
}
for( p=B->data, last= p+B->count*B->wsize; p< last; p+=B->wsize) {
*plist++ = p;
}
for( p=C->data, last= p+C->count*C->wsize; p< last; p+=C->wsize) {
*plist++ = p;
}
*plist++ = 0;
list[1] = (pset) plist;
return list;
}
pset_family cubeunlist(pset *A1)
{
register int i;
register pset p, pdest, cof = A1[0];
register pset_family A;
A = sf_new((((pset *) A1[1] - A1) - 3), cube.size);
for(i = 2; (p = A1[i]) != 0; i++) {
pdest = ((A)->data + (A)->wsize * ( i-2));
{register int i_=( p[0] & 0x03ff); (pdest[0] &= ~0x03ff, pdest[0] |= (i_)); do pdest[i_] = p[i_] | cof[i_]; while (--i_>0);};
}
A->count = (((pset *) A1[1] - A1) - 3);
return A;
}

simplify_cubelist(pset *T)
{
register pset *Tdest;
register int i, ncubes;
set_copy(cube.temp[0], T[0]);	ncubes = (((pset *) T[1] - T) - 3);
qsort((char *) (T+2), ncubes, sizeof(pset), d1_order);
Tdest = T+2;
for(i = 3; i < ncubes; i++) {
if (d1_order(&T[i-1], &T[i]) != 0) {
*Tdest++ = T[i];
}
}
*Tdest++ = 0;	Tdest[1] = (pset) Tdest;	}
