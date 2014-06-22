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

static VALUE iblt_s_new(int argc, VALUE *argv, VALUE self) {
    struct IBLT *iblt;
    VALUE obj;
    int i;

    obj = Data_Make_Struct(self, struct IBLT, NULL, cells_free, iblt);
    
    iblt->size = FIX2INT(argv[0]);
    iblt->hashes = FIX2INT(argv[1]);
    iblt->seed = FIX2INT(argv[2]); 

    // We want hashes to yield distinct locations, so we divide the total size
    // by the number of hashes and create that many arrays of cells
    iblt->ptr = ALLOC_N(Cell *, iblt->hashes);
    for (i = 0; i < iblt->hashes; i++) {
        iblt->ptr[i] = ALLOC_N(Cell, iblt->size/iblt->hashes);
    }

    return obj;
}

static VALUE iblt_insert(VALUE self, VALUE key, VALUE value) {

}

static VALUE iblt_delete(VALUE self, VALUE key, VALUE value) {

}

static VALUE iblt_get(VALUE self, VALUE key) {

}

static VALUE iblt_inspect(VALUE self) {

}

void Init_ciblt(void) {
    cIBLT = rb_define_class("CIBLT", rb_cObject);
    rb_define_singleton_method(cIBLT, "new", iblt_s_new, -1);
    rb_define_method(cIBLT, "insert", iblt_insert, 2);
    rb_define_method(cIBLT, "delete", iblt_delete, 2);
    rb_define_method(cIBLT, "[]", iblt_get, 1);
    rb_define_method(cIBLT, "inspect", iblt_inspect, 0);
}

