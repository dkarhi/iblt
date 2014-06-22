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

static VALUE iblt_s_new(int argc, VALUE *argv, VALUE self) {

// We want each hash to yield a distinct location, so we divide the total 
// number of cells by the number of hashes and create that many arrays
Cell iblt[][];

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
    rb_define_method(cIBLT, "inspect", iblt_get, 0);
}

