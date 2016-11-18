/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Paul Sokolovsky
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>

#include "py/runtime.h"
#include "py/misc.h"

#include "rom/miniz.h"

typedef struct _mp_obj_compressobj_t {
    mp_obj_base_t base;
    tdefl_compressor d;
    vstr_t s;	/* FIXME: clear this vstr on deletion */
} mp_obj_compressobj_t;

static mz_bool compressobj_put(const void *pBuf, int buf_len, void *pUser) {
    mp_obj_compressobj_t *o = pUser;
    char *p = vstr_add_len(&o->s, buf_len);
    if (p) {
	memcpy(p, pBuf, buf_len);
	return MZ_TRUE;
    } else
	return MZ_FALSE;
}

STATIC mp_obj_t compressobj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    mp_obj_compressobj_t *o = m_new_obj(mp_obj_compressobj_t);
    o->base.type = type;
    vstr_init(&o->s, 0);
    tdefl_init(&o->d, compressobj_put, o, TDEFL_WRITE_ZLIB_HEADER);
    return MP_OBJ_FROM_PTR(o);
}
STATIC mp_obj_t compressobj_compress(mp_obj_t self_in, mp_obj_t data) {
    mp_obj_compressobj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_uint_t len;
    const char *buf = mp_obj_str_get_data(data, &len);
    tdefl_compress_buffer(&self->d, buf, len, TDEFL_NO_FLUSH);
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &self->s);
};
STATIC MP_DEFINE_CONST_FUN_OBJ_2(compressobj_compress_obj, compressobj_compress);
STATIC mp_obj_t compressobj_flush(mp_obj_t self_in) {
    mp_obj_compressobj_t *self = MP_OBJ_TO_PTR(self_in);
    tdefl_compress_buffer(&self->d, 0, 0, TDEFL_FINISH);
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &self->s);
};
STATIC MP_DEFINE_CONST_FUN_OBJ_1(compressobj_flush_obj, compressobj_flush);

STATIC const mp_rom_map_elem_t compressobj_locals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_compress), MP_ROM_PTR(&compressobj_compress_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&compressobj_flush_obj) },
};
STATIC MP_DEFINE_CONST_DICT(compressobj_locals_dict, compressobj_locals_table);

STATIC const mp_obj_type_t compressobj_type = {
    { &mp_type_type },
    .name = MP_QSTR_compressobj,
    .make_new = compressobj_make_new,
    .locals_dict = (void*)&compressobj_locals_dict,
};

STATIC const mp_rom_map_elem_t mp_module_miniz_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_miniz) },
    { MP_ROM_QSTR(MP_QSTR_compressobj), MP_ROM_PTR(&compressobj_type) },
/*    { MP_ROM_QSTR(MP_QSTR_decompress, MP_ROM_PTR(&mod_miniz_decompress) },*/
};
STATIC MP_DEFINE_CONST_DICT(mp_module_miniz_globals, mp_module_miniz_globals_table);

const mp_obj_module_t mp_module_miniz = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_miniz_globals,
};
