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
    TM_TT_PROP__INTERACTABLE_LEVER__HANDLE_ROTATE_AXIS, // vec3
    TM_TT_PROP__INTERACTABLE_LEVER__HANDLE_ROTATE_ANGLE, // float
    TM_TT_PROP__INTERACTABLE_LEVER__HANDLE_OPEN_TIME, // float
    TM_TT_PROP__INTERACTABLE_LEVER__TARGET, // reference(entity)
    TM_TT_PROP__INTERACTABLE_LEVER__TARGET_ROTATE_AXIS, // vec3
    TM_TT_PROP__INTERACTABLE_LEVER__TARGET_ROTATE_ANGLE, // float
    TM_TT_PROP__INTERACTABLE_LEVER__TARGET_OPEN_TIME, // float
};

enum tm_interactable_type {
    TM_INTERACTABLE_TYPE_LEVER,
};

enum tm_lever_state {
    TM_LEVER_STATE_CLOSED,
    TM_LEVER_STATE_LEVER_OPENING,
    TM_LEVER_STATE_TARGET_OPENING,
    TM_LEVER_STATE_OPEN,
    TM_LEVER_STATE_LEVER_CLOSING,
    TM_LEVER_STATE_TARGET_CLOSING,
};

typedef struct tm_interactable_lever_t {
    tm_entity_t handle;
    tm_entity_t target;
    tm_vec4_t handle_closed_rotation;
    tm_vec4_t handle_open_rotation;
    tm_vec4_t target_closed_rotation;
    tm_vec4_t target_open_rotation;
    float target_open_time;
    float handle_open_time;
    enum tm_lever_state state;
    TM_PAD(4);
} tm_interactable_lever_t;

typedef struct tm_interactable_component_t {
    enum tm_interactable_type type;
    TM_PAD(4);
    union {
        tm_interactable_lever_t lever;
    };
} tm_interactable_component_t;

typedef struct tm_interactable_component_manager_o tm_interactable_component_manager_o;

struct tm_interactable_component_api {
    bool (*can_interact)(tm_interactable_component_manager_o *mgr, tm_entity_t interactable);
    void (*interact)(tm_interactable_component_manager_o *mgr, tm_entity_t interactable);
    void (*update_active_interactables)(tm_interactable_component_manager_o *mgr, float dt, double t);
};