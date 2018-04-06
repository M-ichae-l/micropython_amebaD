#include <stdint.h>

#ifdef AMEBA1
#include "mpconfigports/rtl8195a_ports.h"
#elif AMEBAZ
#include "mpconfigports/amebaz_ports.h"
#else
#warning "Invalid CHIP set"
#endif

#define mp_type_fileio fatfs_type_fileio
#define mp_type_textio fatfs_type_textio

// type definitions for the specific machine

#define BYTES_PER_WORD (4)

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

#define UINT_FMT "%lu"
#define INT_FMT "%ld"

#define MP_HAL_PIN_FMT "%u"

typedef int32_t mp_int_t; // must be pointer size
typedef uint32_t mp_uint_t; // must be pointer size
typedef void *machine_ptr_t; // must be of pointer size
typedef const void *machine_const_ptr_t; // must be of pointer size
typedef long mp_off_t;

#define MP_STATE_PORT MP_STATE_VM
#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)

#ifdef UINT
#undef UINT
#endif
