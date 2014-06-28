/*
 *   ciblt.c - Invertible Bloom Lookup Table
 *   (c) David Karhi <dkarhi@gmail.com>
 */

#include <stdbool.h>
#include "ruby.h"
#include "crc32.h"

#if !defined(RSTRING_LEN)
# define RSTRING_LEN(x) (RSTRING(x)->len)
# define RSTRING_PTR(x) (RSTRING(x)->ptr)
#endif

static VALUE cIBLT;

typedef struct Cell {
    int count;
    char *key_sum;
    char *value_sum;
    int key_len; // XOR may put NULLS in sums, so track length
    int value_len; // XOR may put NULLS in sums, so track length
} Cell;

struct IBLT {
    int size; // Total number of cells
    int hashes; // Number of hash functions
    int seed; // Seed value
    Cell **ptr; // Pointer to cells
};

// This functions frees up the dynamically allocated memory
void cells_free(struct IBLT *iblt) {
    int i, j;
    for (i = 0; i < iblt->hashes; i++) {
        for (j = 0; j < (int) (iblt->size/iblt->hashes); j++) {
            ruby_xfree(iblt->ptr[i][j].key_sum);
            ruby_xfree(iblt->ptr[i][j].value_sum);
        }
        ruby_xfree(iblt->ptr[i]);
    }
    ruby_xfree(iblt->ptr);
}

// Perform bitwise XOR and return result
char *bit_xor(char *x, char *y, int len_x, int len_y) {
    int i, longer_len, shorter_len;
    char *xor_result, longer_str;

    // The return string needs to be as long as the longest input, but we only
    // need to perform XOR for the length of the shortest
    if (len_x > len_y) {
        longer_len = len_x;
        shorter_len = len_y;
        longer_str = 'x';
    } 
    else {
        longer_len = len_y;
        shorter_len = len_x;
        longer_str = 'y';
    }

    xor_result = ALLOC_N(char, longer_len);
    for (i = 0; i < shorter_len; i++) {
        xor_result[i] = x[i] ^ y[i];
    }
    for (i = shorter_len; i < longer_len; i++) {
        if (longer_str == 'x') {
            xor_result[i] = x[i];
        }
        else {
            xor_result[i] = y[i];
        } 
    }
      
    return xor_result;
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
            iblt->ptr[i][j].key_sum = NULL;
            iblt->ptr[i][j].key_len = 0;
            iblt->ptr[i][j].value_sum = NULL;
            iblt->ptr[i][j].value_len = 0;
        }
    }
    return obj;
}

static VALUE iblt_insert(VALUE self, VALUE key, VALUE value) {
    struct IBLT *iblt;
    int i, seed, index;
    char *xor_string;
    Data_Get_Struct(self, struct IBLT, iblt);
    StringValue(key);
    StringValue(value);
    
    for (i = 0; i < iblt->hashes; i++) {
        // Seed each hash function differently
        seed = i + iblt->seed; 

        index = (int) (crc32((unsigned int) (seed), RSTRING_PTR(key), RSTRING_LEN(key)) % (int) (iblt->size/iblt->hashes));    
        iblt->ptr[i][index].count++;
        xor_string = bit_xor(iblt->ptr[i][index].key_sum, RSTRING_PTR(key), iblt->ptr[i][index].key_len, RSTRING_LEN(key));
        ruby_xfree(iblt->ptr[i][index].key_sum);
        iblt->ptr[i][index].key_sum = xor_string;
        xor_string = bit_xor(iblt->ptr[i][index].value_sum, RSTRING_PTR(value), iblt->ptr[i][index].value_len, RSTRING_LEN(value));
        ruby_xfree(iblt->ptr[i][index].value_sum);
        iblt->ptr[i][index].value_sum = xor_string; 
 
        if (iblt->ptr[i][index].key_len < RSTRING_LEN(key)) {
            iblt->ptr[i][index].key_len = RSTRING_LEN(key);
        }
        if (iblt->ptr[i][index].value_len < RSTRING_LEN(value)) {
            iblt->ptr[i][index].value_len = RSTRING_LEN(value);
        }
    }
    return Qnil;
}
    
static VALUE iblt_delete(VALUE self, VALUE key, VALUE value) {
    struct IBLT *iblt;
    int i, seed, index;
    char *xor_string;
    Data_Get_Struct(self, struct IBLT, iblt);
    StringValue(key);
    StringValue(value);

    for (i = 0; i < iblt->hashes; i++) {
        // Seed each hash function differently
        seed = i + iblt->seed;

        index = (int) (crc32((unsigned int) (seed), RSTRING_PTR(key), RSTRING_LEN(key)) % (int) (iblt->size/iblt->hashes));
        iblt->ptr[i][index].count--;
        xor_string = bit_xor(iblt->ptr[i][index].key_sum, RSTRING_PTR(key), iblt->ptr[i][index].key_len, RSTRING_LEN(key));
        ruby_xfree(iblt->ptr[i][index].key_sum);
        iblt->ptr[i][index].key_sum = xor_string;
        xor_string = bit_xor(iblt->ptr[i][index].value_sum, RSTRING_PTR(value), iblt->ptr[i][index].value_len, RSTRING_LEN(value));
        ruby_xfree(iblt->ptr[i][index].value_sum);
        iblt->ptr[i][index].value_sum = xor_string;

        if (iblt->ptr[i][index].key_len < RSTRING_LEN(key)) {
            iblt->ptr[i][index].key_len = RSTRING_LEN(key);
        }
        if (iblt->ptr[i][index].value_len < RSTRING_LEN(value)) {
            iblt->ptr[i][index].value_len = RSTRING_LEN(value);
        }

    }
    return Qnil;

}

static VALUE iblt_get(VALUE self, VALUE key) {
    struct IBLT *iblt;
    int i, seed, index;
    StringValue(key);
    Data_Get_Struct(self, struct IBLT, iblt);

    for (i = 0; i < iblt->hashes; i++) {
        // Seed each hash function differently
        seed = i + iblt->seed;

        index = (int) (crc32((unsigned int) (seed), RSTRING_PTR(key), RSTRING_LEN(key)) % (int) (iblt->size/iblt->hashes));
        if (iblt->ptr[i][index].count == 0) {
            return Qnil;
        }
        else if (iblt->ptr[i][index].count == 1) {
            if (strcmp(iblt->ptr[i][index].key_sum, RSTRING_PTR(key)) == 0) {
                return rb_str_new(iblt->ptr[i][index].value_sum, iblt->ptr[i][index].value_len);
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
    bool done;
    Data_Get_Struct(self, struct IBLT, iblt);
    
    done = false;
    str = rb_str_buf_new2("{");
    while (!done) {
        done = true;
        for (i = 0; i < iblt->hashes; i++) {
            for (j = 0; j < (int) (iblt->size/iblt->hashes); j++) {
                if (iblt->ptr[i][j].count == 1) {
                   // If we peel an item off the IBLT then we need to do 
                   // another pass to see if another cell now has a count of 1.
                   // This is incredibly time inefficient but it guarantees 
                   // complete output
                   done = false;
                   if (RSTRING_LEN(str) > 1) { 
                       rb_str_buf_cat2(str, ", ");
                   }
                   rb_str_buf_append(str, rb_str_new(iblt->ptr[i][j].key_sum, iblt->ptr[i][j].key_len));
                   rb_str_buf_cat2(str, "=>");
                   rb_str_buf_append(str, rb_str_new(iblt->ptr[i][j].value_sum, iblt->ptr[i][j].value_len));
                   iblt_delete(self, rb_str_new(iblt->ptr[i][j].key_sum, iblt->ptr[i][j].key_len), rb_str_new(iblt->ptr[i][j].value_sum, iblt->ptr[i][j].value_len));
                }
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

