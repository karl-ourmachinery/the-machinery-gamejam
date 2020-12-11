#include <foundation/api_registry.h>

// Creates API struct pointers and a function that loads them all. It supports up to 32 APIs as
// parameters. If you run TM_LOAD_APIS like this:
//
// ```c
// TM_LOAD_APIS(load_all_apis,
//     tm_allocator_api,
//     tm_entity_api)
// ```
//
// then it will output this code:
//
// ```c
// static struct tm_allocator_api *tm_allocator_api;
// static struct tm_entity_api *tm_entity_api;
//
// static void load_all_apis(struct tm_api_registry_api* reg) {
//     tm_allocator_api = reg->get("tm_allocator_api");
//     tm_entity_api = reg->get("tm_entity_api");
// }
// ```
//
// Additionally, make sure to then run `load_all_apis` whenever you plugin loads.

#define TM_LOAD_APIS(load_func_name, ...) \
    TM_CALL_FOR_EACH(TM_API_DEF, __VA_ARGS__) \
    static void load_func_name(struct tm_api_registry_api* reg) {\
        TM_CALL_FOR_EACH(TM_API_GET, __VA_ARGS__)\
    }

#define TM_API_DEF(def_name) static struct def_name *def_name;

#define TM_API_GET(get_name) get_name = reg->get(#get_name);

#define TM_EXPAND(x) x

#define TM_GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, \
    _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, N, ...) N

#define TM_CALL_EXP_0(TM_CALL, ...)
#define TM_CALL_EXP_1(TM_CALL, x) TM_CALL(x)
#define TM_CALL_EXP_2(TM_CALL, x, ...) TM_CALL(x) TM_CALL_EXP_1(TM_CALL, __VA_ARGS__)
#define TM_CALL_EXP_3(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_2(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_4(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_3(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_5(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_4(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_6(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_5(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_7(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_6(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_8(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_7(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_9(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_8(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_10(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_9(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_11(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_10(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_12(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_11(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_13(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_12(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_14(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_13(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_15(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_14(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_16(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_15(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_17(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_16(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_18(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_17(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_19(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_18(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_20(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_19(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_21(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_20(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_22(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_21(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_23(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_22(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_24(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_23(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_25(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_24(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_26(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_25(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_27(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_26(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_28(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_27(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_29(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_28(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_30(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_29(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_31(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_30(TM_CALL, __VA_ARGS__))
#define TM_CALL_EXP_32(TM_CALL, x, ...) TM_CALL(x) TM_EXPAND(TM_CALL_EXP_31(TM_CALL, __VA_ARGS__))

#define TM_CALL_FOR_EACH(x, ...) \
TM_EXPAND(TM_GET_NTH_ARG(__VA_ARGS__, TM_CALL_EXP_32, TM_CALL_EXP_31, TM_CALL_EXP_30, TM_CALL_EXP_29, TM_CALL_EXP_28, \
    TM_CALL_EXP_27, TM_CALL_EXP_26, TM_CALL_EXP_25, TM_CALL_EXP_24, TM_CALL_EXP_23, TM_CALL_EXP_22, TM_CALL_EXP_21, \
    TM_CALL_EXP_20, TM_CALL_EXP_19, TM_CALL_EXP_18, TM_CALL_EXP_17, TM_CALL_EXP_16, TM_CALL_EXP_15, TM_CALL_EXP_14, \
    TM_CALL_EXP_13, TM_CALL_EXP_12, TM_CALL_EXP_11, TM_CALL_EXP_10, TM_CALL_EXP_9, TM_CALL_EXP_8, TM_CALL_EXP_7, \
    TM_CALL_EXP_6, TM_CALL_EXP_5, TM_CALL_EXP_4, TM_CALL_EXP_3, TM_CALL_EXP_2, TM_CALL_EXP_1, \
    TM_CALL_EXP_0)(x, __VA_ARGS__))
