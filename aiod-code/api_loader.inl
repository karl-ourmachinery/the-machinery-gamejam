#include <foundation/api_registry.h>

// Creates all struct pointers, a `tm_api_registry_api` pointer and then creates a load function to load them all.
#define TM_LOAD_APIS(load_func_name, ...) \
    TM_API_DEF(tm_api_registry_api) \
    TM_CALL_FOR_EACH(TM_API_DEF, __VA_ARGS__) \
    static void load_func_name(struct tm_api_registry_api* reg) {\
        TM_API_GET(tm_api_registry_api)\
        TM_CALL_FOR_EACH(TM_API_GET, __VA_ARGS__)\
    }

#define TM_API_DEF(def_name) static struct def_name *def_name;

#define TM_API_GET(get_name) get_name = reg->get(#get_name);

#define TM_EXPAND(x) x

#define TM_GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, \
    _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, N, ...) N

#define TM_CALL_EXP_0(_call, ...)
#define TM_CALL_EXP_1(_call, x) _call(x)
#define TM_CALL_EXP_2(_call, x, ...) _call(x) TM_CALL_EXP_1(_call, __VA_ARGS__)
#define TM_CALL_EXP_3(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_2(_call, __VA_ARGS__))
#define TM_CALL_EXP_4(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_3(_call, __VA_ARGS__))
#define TM_CALL_EXP_5(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_4(_call, __VA_ARGS__))
#define TM_CALL_EXP_6(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_5(_call, __VA_ARGS__))
#define TM_CALL_EXP_7(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_6(_call, __VA_ARGS__))
#define TM_CALL_EXP_8(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_7(_call, __VA_ARGS__))
#define TM_CALL_EXP_9(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_8(_call, __VA_ARGS__))
#define TM_CALL_EXP_10(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_9(_call, __VA_ARGS__))
#define TM_CALL_EXP_11(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_10(_call, __VA_ARGS__))
#define TM_CALL_EXP_12(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_11(_call, __VA_ARGS__))
#define TM_CALL_EXP_13(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_12(_call, __VA_ARGS__))
#define TM_CALL_EXP_14(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_13(_call, __VA_ARGS__))
#define TM_CALL_EXP_15(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_14(_call, __VA_ARGS__))
#define TM_CALL_EXP_16(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_15(_call, __VA_ARGS__))
#define TM_CALL_EXP_17(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_16(_call, __VA_ARGS__))
#define TM_CALL_EXP_18(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_17(_call, __VA_ARGS__))
#define TM_CALL_EXP_19(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_18(_call, __VA_ARGS__))
#define TM_CALL_EXP_20(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_19(_call, __VA_ARGS__))
#define TM_CALL_EXP_21(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_20(_call, __VA_ARGS__))
#define TM_CALL_EXP_22(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_21(_call, __VA_ARGS__))
#define TM_CALL_EXP_23(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_22(_call, __VA_ARGS__))
#define TM_CALL_EXP_24(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_23(_call, __VA_ARGS__))
#define TM_CALL_EXP_25(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_24(_call, __VA_ARGS__))
#define TM_CALL_EXP_26(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_25(_call, __VA_ARGS__))
#define TM_CALL_EXP_27(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_26(_call, __VA_ARGS__))
#define TM_CALL_EXP_28(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_27(_call, __VA_ARGS__))
#define TM_CALL_EXP_29(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_28(_call, __VA_ARGS__))
#define TM_CALL_EXP_30(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_29(_call, __VA_ARGS__))
#define TM_CALL_EXP_31(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_30(_call, __VA_ARGS__))
#define TM_CALL_EXP_32(_call, x, ...) _call(x) TM_EXPAND(TM_CALL_EXP_31(_call, __VA_ARGS__))

#define TM_CALL_FOR_EACH(x, ...) \
TM_EXPAND(TM_GET_NTH_ARG(__VA_ARGS__, TM_CALL_EXP_32, TM_CALL_EXP_31, TM_CALL_EXP_30, TM_CALL_EXP_29, TM_CALL_EXP_28, \
    TM_CALL_EXP_27, TM_CALL_EXP_26, TM_CALL_EXP_25, TM_CALL_EXP_24, TM_CALL_EXP_23, TM_CALL_EXP_22, TM_CALL_EXP_21, \
    TM_CALL_EXP_20, TM_CALL_EXP_19, TM_CALL_EXP_18, TM_CALL_EXP_17, TM_CALL_EXP_16, TM_CALL_EXP_15, TM_CALL_EXP_14, \
    TM_CALL_EXP_13, TM_CALL_EXP_12, TM_CALL_EXP_11, TM_CALL_EXP_10, TM_CALL_EXP_9, TM_CALL_EXP_8, TM_CALL_EXP_7, \
    TM_CALL_EXP_6, TM_CALL_EXP_5, TM_CALL_EXP_4, TM_CALL_EXP_3, TM_CALL_EXP_2, TM_CALL_EXP_1, \
    TM_CALL_EXP_0)(x, __VA_ARGS__))
