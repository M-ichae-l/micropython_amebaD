/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Chester Tseng
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

#include "py/mpconfig.h"
#include "py/runtime.h"

/* mphelper */
#include "objpin.h"

/********************** Local variables ***************************************/

/********************** Local functions ***************************************/
void pin_init0(void);
void pin_validate_dir (uint dir);
void pin_validate_pull (uint pull);

STATIC mp_obj_t pin_value(mp_uint_t n_args, const mp_obj_t *args);
STATIC mp_obj_t pin_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args);
STATIC mp_obj_t pin_obj_init_helper(pin_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args);

/********************** function bodies ***************************************/

void pin_init0(void) {
    // TODO: Ameba board pin hardware init 
}

void pin_validate_dir (uint dir) {
    if (dir != PIN_INPUT && dir != PIN_OUTPUT) {
        mp_raise_ValueError("Invalid GPIO direction type");
    }
}

void pin_validate_pull (uint pull) {
    if (pull != PullNone && pull != PullUp && pull != PullDown && pull != OpenDrain) {
        mp_raise_ValueError("Invalid GPIO pull type");
    }
}

STATIC pin_obj_t *pin_find_named_pin(const mp_obj_dict_t *named_pins, mp_obj_t name) {
    mp_map_t *named_map = mp_obj_dict_get_map((mp_obj_t)named_pins);
    mp_map_elem_t *named_elem = mp_map_lookup(named_map, name, MP_MAP_LOOKUP);
    if (named_elem != NULL && named_elem->value != NULL) {
        return named_elem->value;
    }
    return NULL;
}

// C API used to convert a user-supplied pin name into an ordinal pin number.
pin_obj_t *pin_find(mp_obj_t user_obj) {
    pin_obj_t *pin_obj;

    // if a pin was provided, use it
    if (MP_OBJ_IS_TYPE(user_obj, &pin_type)) {
        pin_obj = user_obj;
        return pin_obj;
    }
    // otherwise see if the pin name matches a cpu pin
    pin_obj = pin_find_named_pin(&pin_board_pins_locals_dict, user_obj);
    if (pin_obj) {
        return pin_obj;
    }

    mp_raise_ValueError("invalid argument(s) value");
}

STATIC uint8_t pin_get_value (const pin_obj_t* self) {
    int value;
    bool setdir = false;
    if (self->dir == PullNone || self->dir == OpenDrain) {
        setdir = true;
        // configure the direction to IN for a moment in order to read the pin value
        gpio_dir((gpio_t *)&(self->obj), PIN_INPUT);
    }
    // now get the value
    value = gpio_read((gpio_t *)&(self->obj));

    if (setdir) {
        // set the direction back to output
        gpio_dir((gpio_t *)&(self->obj), PIN_OUTPUT);
        if (self->value) {
            gpio_write((gpio_t *)&(self->obj), 1);
        } else {
            gpio_write((gpio_t *)&(self->obj), 0);
        }
    }
    // return it
    return value ? 1 : 0;
}

STATIC mp_obj_t pin_obj_init_helper(pin_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    STATIC const mp_arg_t pin_init_args[] = {
        { MP_QSTR_dir,                         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pull,                        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_value,    MP_ARG_KW_ONLY  |  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(pin_init_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(pin_init_args), pin_init_args, args);

    // get the io mode, default is input
    uint dir = PIN_INPUT;
    if (args[0].u_obj != MP_OBJ_NULL) {
        dir = mp_obj_get_int(args[0].u_obj);
        pin_validate_dir(dir);
    }

    // get the pull type, default is pull none
    uint pull = PullNone;
    if (args[1].u_obj != MP_OBJ_NULL) {
        pull = mp_obj_get_int(args[1].u_obj);
        pin_validate_pull(pull);
    }

    // get the value, default is 0
    int value = -1;
    if (args[2].u_obj != MP_OBJ_NULL) {
        if (mp_obj_is_true(args[2].u_obj)) {
            value = 1;
        } else {
            value = 0;
        }
    }

    // config the pin object 
    self->dir  = dir;
    self->pull = pull;

    if (value != -1)
        self->value = value;

    // config pin hardware
    gpio_init((gpio_t *)&(self->obj), self->id);
    gpio_dir((gpio_t *)&(self->obj), self->dir);
    gpio_mode((gpio_t *)&(self->obj), self->pull);
    //
    return mp_const_none;
}

STATIC void pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pin_obj_t *self = self_in;
    uint pull = self->pull;

    // pin name
    mp_printf(print, "Pin('%q'", self->name);

    // pin mode
    qstr dir_qst;
    uint dir = self->dir;
    if (dir == PIN_INPUT) {
        dir_qst = MP_QSTR_IN;
    } else if (dir == PIN_OUTPUT) {
        dir_qst = MP_QSTR_OUT;
    } else {
        // default is input 
        dir_qst = MP_QSTR_IN;
    }

    mp_printf(print, ", dir=Pin.%q", dir_qst);

    // pin pull
    qstr pull_qst;
    if (pull == PullNone) {
        pull_qst = MP_QSTR_PULL_NONE;
    } else if (pull == PullUp) {
        pull_qst = MP_QSTR_PULL_UP;
    } else if (pull == PullDown) {
        pull_qst = MP_QSTR_PULL_DOWN;
    } else if (pull == OpenDrain) {
        pull_qst = MP_QSTR_OPEN_DRAIN;
    } else {
        // defualt is pull none
        pull_qst = MP_QSTR_PULL_NONE;
    }

    mp_printf(print, ", pull=Pin.%q", pull_qst);

    mp_printf(print, ")");
}

STATIC mp_obj_t pin_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // Run an argument through the mapper and return the result.
    pin_obj_t *pin = (pin_obj_t *)pin_find(args[0]);

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    pin_obj_init_helper(pin, n_args - 1, args + 1, &kw_args);

    return (mp_obj_t)pin;
}

STATIC mp_obj_t pin_call(mp_obj_t self_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    mp_obj_t _args[2] = {self_in, *args};
    return pin_value (n_args + 1, _args);
}

STATIC mp_obj_t pin_dir(mp_uint_t n_args, const mp_obj_t *args) {
    pin_obj_t *self = args[0];
    if (n_args == 1) {
        return mp_obj_new_int(self->dir);
    } else {
        uint dir = mp_obj_get_int(args[1]);
        pin_validate_dir(dir);
        self->dir = dir;
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pin_dir_obj, 1, 2, pin_dir);

STATIC mp_obj_t pin_pull(mp_uint_t n_args, const mp_obj_t *args) {
    pin_obj_t *self = args[0];
    if (n_args == 1) {
        return mp_obj_new_int(self->pull);
    } else {
        mp_uint_t pull = mp_obj_get_int(args[1]);
        pin_validate_pull (pull);
        self->pull = pull;
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pin_pull_obj, 1, 2, pin_pull);

STATIC mp_obj_t pin_id(mp_obj_t self_in) {
    pin_obj_t *self = self_in;
    return MP_OBJ_NEW_QSTR(self->name);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_id_obj, pin_id);

STATIC mp_obj_t pin_value(mp_uint_t n_args, const mp_obj_t *args) {
    pin_obj_t *self = args[0];
    if (n_args == 1) {
        // get the value
        return pin_get_value(self) ? mp_const_true: mp_const_false;
    } else {
        // set the pin value
        if (mp_obj_is_true(args[1])) {
            self->value = 1;
            gpio_write(&(self->obj), 1);
        } else {
            self->value = 0;
            gpio_write(&(self->obj), 0);
        }
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pin_value_obj, 1, 2, pin_value);

STATIC mp_obj_t pin_toggle(mp_obj_t self_in) {
    pin_obj_t *self = self_in;
    gpio_write(&(self->obj), ~gpio_read(&(self->obj)));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pin_toggle_obj, pin_toggle);

STATIC void pin_named_pins_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pin_named_pins_obj_t *self = self_in;
    mp_printf(print, "<Pin.%q>", self->name);
}

const mp_obj_type_t pin_board_pins_obj_type = {
    { &mp_type_type },
    .name        = MP_QSTR_board,
    .print       = pin_named_pins_obj_print,
    .locals_dict = (mp_obj_dict_t *)&pin_board_pins_locals_dict,
};

STATIC const mp_map_elem_t pin_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_id),         MP_OBJ_FROM_PTR(&pin_id_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_dir),        MP_OBJ_FROM_PTR(&pin_dir_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pull),       MP_OBJ_FROM_PTR(&pin_pull_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_value),      MP_OBJ_FROM_PTR(&pin_value_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_toggle),     MP_OBJ_FROM_PTR(&pin_toggle_obj) },

    // class attributes
    { MP_OBJ_NEW_QSTR(MP_QSTR_board),      MP_OBJ_FROM_PTR(&pin_board_pins_obj_type) },

    // class constants
    { MP_OBJ_NEW_QSTR(MP_QSTR_IN),         MP_OBJ_NEW_SMALL_INT(PIN_INPUT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_OUT),        MP_OBJ_NEW_SMALL_INT(PIN_OUTPUT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_PULL_NONE),  MP_OBJ_NEW_SMALL_INT(PullNone) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_PULL_UP),    MP_OBJ_NEW_SMALL_INT(PullUp) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_PULL_DOWN),  MP_OBJ_NEW_SMALL_INT(PullDown) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_OPEN_DRAIN), MP_OBJ_NEW_SMALL_INT(OpenDrain) },
};
STATIC MP_DEFINE_CONST_DICT(pin_locals_dict, pin_locals_dict_table);

const mp_obj_type_t pin_type = {
    { &mp_type_type },
    .name        = MP_QSTR_Pin,
    .print       = pin_print,
    .make_new    = pin_make_new,
    .call        = pin_call,
    .locals_dict = (mp_obj_dict_t *)&pin_locals_dict,
};

