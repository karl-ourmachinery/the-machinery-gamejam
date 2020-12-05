#include <plugins/entity/entity_api_types.h>

#define TM_TT_TYPE__INTERACTABLE_COMPONENT "tm_interactable_component"
#define TM_TT_TYPE_HASH__INTERACTABLE_COMPONENT TM_STATIC_HASH("tm_interactable_component", 0x95e4f6722c966bf4ULL)

enum {
    TM_TT_PROP__INTERACTABLE_COMPONENT__DESC, // subobject(anything) -- for example TM_TT_TYPE__INTERACTABLE_LEVER
};

#define TM_TT_TYPE__INTERACTABLE_LEVER "tm_interactable_lever"
#define TM_TT_TYPE_HASH__INTERACTABLE_LEVER TM_STATIC_HASH("tm_interactable_lever", 0xb415dd3c3c35fb79ULL)

enum {
    TM_TT_PROP__INTERACTABLE_LEVER__HANDLE, // reference(entity)
    TM_TT_PROP__INTERACTABLE_LEVER__TARGET, // reference(entity)
};

enum tm_interactable_type {
    TM_INTERACTABLE_TYPE_LEVER,
};

typedef struct tm_interactable_lever_t {
    tm_entity_t handle;
    tm_entity_t target;
} tm_interactable_lever_t;

typedef struct tm_interactable_component_t {
    enum tm_interactable_type type;
    TM_PAD(4);
    union {
        tm_interactable_lever_t lever;
    };
} tm_interactable_component_t;