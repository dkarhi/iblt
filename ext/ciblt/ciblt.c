/*
 *   ciblt.c - Invertible Bloom Lookup Table
 *   (c) David Karhi <dkarhi@gmail.com>
 */

#include "ruby.h"
#include "crc32.h"

#if !defined(RSTRING_LEN)
# define RSTRING_LEN(x) (RSTRING(x)->len)
# define RSTRING_PTR(x) (RSTRING(x)->ptr)
#endif

static VALUE cIBLT;

typedef struct Cell {
    int count;
    int key_sum;
    int value_sum;
} Cell;

struct IBLT {
    int size; // Total number of cells
    int hashes; // Number of hash functions
    int seed; // Seed value
    Cell **ptr; // Pointer to cells
};

// This functions frees up the dynamically allocated memory
void cells_free(struct IBLT *iblt) {
    int i;
    for (i = 0; i < iblt->hashes; i++) {
        ruby_xfree(iblt->ptr[i]);
    }
    ruby_xfree(iblt->ptr);
}

int bit_xor(int x, int y) {
    int z = x ^ y;
    return z;
}

static VALUE iblt_s_new(int argc, VALUE *argv, VALUE self) {
    struct IBLT *iblt;
    VALUE obj;
    int i, j;

    obj = Data_Make_Struct(self, struct IBLT, NULL, cells_free, iblt);

    iblt->size = NUM2INT(argv[0]);
    iblt->hashes = NUM2INT(argv[1]);
    iblt->seed = NUM2INT(argv[2]); 

    // We want hashes to yield distinct locations, so we divide the total size
    // by the number of hashes and create that many arrays of cells
    iblt->ptr = ALLOC_N(Cell *, iblt->hashes);
    for (i = 0; i < iblt->hashes; i++) {
        iblt->ptr[i] = ALLOC_N(Cell, (int) (iblt->size/iblt->hashes));
        for (j = 0; j < (int) (iblt->size/iblt->hashes); j++) {
            iblt->ptr[i][j].count = 0;
            iblt->ptr[i][j].key_sum = 0;
            iblt->ptr[i][j].value_sum = 0;
        }
    }
    return obj;
}

static VALUE iblt_insert(VALUE self, VALUE key, VALUE value) {
    struct IBLT *iblt;
    int i, seed, index;
    char ckey[15] = { 0 };
    Data_Get_Struct(self, struct IBLT, iblt);
    sprintf(ckey, "%d", NUM2INT(key)); 
    
    for (i = 0; i < iblt->hashes; i++) {
        // Seed each hash function differently
        seed = i + iblt->seed; 

        index = (int) (crc32((unsigned int) (seed), ckey, strlen(ckey)) % (int) (iblt->size/iblt->hashes));    
        iblt->ptr[i][index].count++;
        iblt->ptr[i][index].key_sum = bit_xor(iblt->ptr[i][index].key_sum, NUM2INT(key));
        iblt->ptr[i][index].value_sum = bit_xor(iblt->ptr[i][index].value_sum, NUM2INT(value));
    }
    return Qnil;
}
    
static VALUE iblt_delete(VALUE self, VALUE key, VALUE value) {
    struct IBLT *iblt;
    int i, seed, index;
    char ckey[15] = { 0 };
    Data_Get_Struct(self, struct IBLT, iblt);
    sprintf(ckey, "%d", NUM2INT(key));

    for (i = 0; i < iblt->hashes; i++) {
        // Seed each hash function differently
        seed = i + iblt->seed;

        index = (int) (crc32((unsigned int) (seed), ckey, strlen(ckey)) % (int) (iblt->size/iblt->hashes));
        iblt->ptr[i][index].count--;
        iblt->ptr[i][index].key_sum = bit_xor(iblt->ptr[i][index].key_sum, NUM2INT(key));
        iblt->ptr[i][index].value_sum = bit_xor(iblt->ptr[i][index].value_sum, NUM2INT(value));
    }
    return Qnil;

}

static VALUE iblt_get(VALUE self, VALUE key) {
    struct IBLT *iblt;
    int i, seed, index;
    char ckey[15] = { 0 };
    Data_Get_Struct(self, struct IBLT, iblt);
    sprintf(ckey, "%d", NUM2INT(key));

    for (i = 0; i < iblt->hashes; i++) {
        // Seed each hash function differently
        seed = i + iblt->seed;

        index = (int) (crc32((unsigned int) (seed), ckey, strlen(ckey)) % (int) (iblt->size/iblt->hashes));
        if (iblt->ptr[i][index].count == 0) {
            return Qnil;
        }
        else if (iblt->ptr[i][index].count == 1) {
            if (iblt->ptr[i][index].key_sum == NUM2INT(key)) {
                return INT2NUM(iblt->ptr[i][index].value_sum);
            }
            else {
                return Qnil;
            }
        }
    }
    return Qnil;
}

// This function returns the contents of the IBLT but destroys the data
static VALUE iblt_inspect_destroy(VALUE self) {
    struct IBLT *iblt; 
    VALUE str;
    int i,j;
    Data_Get_Struct(self, struct IBLT, iblt);
    
    str = rb_str_buf_new2("{");
    for (i = 0; i < iblt->hashes; i++) {
        for (j = 0; j < (int) (iblt->size/iblt->hashes); j++) {
            if (iblt->ptr[i][j].count == 1) {
               if (RSTRING_LEN(str) > 1) { 
                   rb_str_buf_cat2(str, ", ");
               }
               rb_str_buf_append(str, rb_inspect(INT2NUM(iblt->ptr[i][j].key_sum)));
               rb_str_buf_cat2(str, "=>");
               rb_str_buf_append(str, rb_inspect(INT2NUM(iblt->ptr[i][j].value_sum)));
               iblt_delete(self, INT2NUM(iblt->ptr[i][j].key_sum), INT2NUM(iblt->ptr[i][j].value_sum));
            }
        }
    }
    rb_str_buf_cat2(str, "}");
 
    return str;
}

void Init_ciblt(void) {
    cIBLT = rb_define_class("CIBLT", rb_cObject);
    rb_define_singleton_method(cIBLT, "new", iblt_s_new, -1);
    rb_define_method(cIBLT, "insert", iblt_insert, 2);
    rb_define_method(cIBLT, "delete", iblt_delete, 2);
    rb_define_method(cIBLT, "[]", iblt_get, 1);
    rb_define_method(cIBLT, "inspect!", iblt_inspect_destroy, 0);
}

