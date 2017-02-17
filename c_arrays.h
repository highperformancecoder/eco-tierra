#define cs_binop(op,opname,name,rettype,type)\
void name##_##opname(int, void*,void*,void*);

#define C_decs(name,type)\
void *new_##name(int);\
void delete_##name(void*);\
type *cs_getlist_##name(void*);\
void cs_putlist_##name(void*,type*,int);\
type get_##type(void*,int);\
void put_##type(void*,int,type);\
void copy_##type(int,void*,void*);\
void get_##type##_array(int,void*,void*,void*);\
void put_##type##_array(int,void*,void*,void*);\
void broadcast_##type(int,void*,type);\
  cs_binop(+,plus,name,type,type)\
  cs_binop(-,minus,name,type,type)\
  cs_binop(*,mul,name,type,type)\
  cs_binop(/,div,name,type,type)\
  cs_binop(<,lt,name,int,type)\
  cs_binop(<=,le,name,int,type)\
  cs_binop(>,gt,name,int,type)\
  cs_binop(>=,ge,name,int,type)\
  cs_binop(==,eq,name,int,type)\
  cs_binop(!=,ne,name,int,type)\
void name##_cat(void*,int,void*,int,void*);\
void cs_merge_##type(int,void*,void*,void*,void*);\
type cs_max_##name(int,void*);\
type cs_masked_max_##name(int,void*,void*);\
type cs_min_##name(int,void*);\
type cs_masked_min_##name(int,void*,void*);\
type cs_sum_##name(int,void*);\
type cs_masked_sum_##name(int,void*,void*);\
type cs_prod_##name(int,void*);\
type cs_masked_prod_##name(int,void*,void*);\
void cs_abs_##name(int,void*,void*);\
void cs_sign_##name(int,void*,void*);\

C_decs(array,double)
C_decs(iarray,int)
void iarray_not(int,void*,void*);
void iarray_and(int,void*,void*,void*);
void iarray_or(int,void*,void*,void*);
void iarray_mod(int,void*,void*,void*);

/* scans etc */
/*int cs_count(int,void *);*/
void cs_pack_int(int,void*,void*,void*);
void cs_pack_double(int,void*,void*,void*);
void cs_enumerate(int,void*,void*);
void cs_log_array(int,void*,void*);
void cs_exp_array(int,void*,void*);
void cs_gen_index(int,void*,void*);
enum array_dir_t {upwards, downwards};
void iarray_rank(int size, void *r, void* x, enum array_dir_t dir);
void array_rank(int size, void *r, void* x, enum array_dir_t dir);

/* offdiaglist support */
void array_mul_iarray(int,void*,void*,void*);
void offmul(int,void*,void*,void*,void*,void*);
void iarray_to_array(int,void*,void*);
void cs_trunc(int,void*,void*);
void iarray_asg_array_round(int,void*,void*);
void iarray_addasg_array_round(int,void*,void*);
/*void cs_proj(void*,void*,void*,void*,int);*/

/* random number support */
int ROUND(double);
void cs_srand(int i);
void cs_fillrand(int,void*);
void cs_fillprand(int,void*);
float grand();
void cs_fillgrand(int,void*);
/*void cs_fill_uniq_rand(void*,int);*/



