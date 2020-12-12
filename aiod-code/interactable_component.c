#include "api_loader.inl"

TM_LOAD_APIS(load_apis,
    tm_entity_api,
    tm_the_truth_api,
    tm_localizer_api,
    tm_properties_view_api,
    tm_ui_api,
    tm_logger_api,
    tm_transform_component_api,
    tm_the_truth_common_types_api);

#include "interactable_component.h"
#include <foundation/macros.h>
#include <foundation/the_truth.h>
#include <foundation/the_truth_types.h>
#include <foundation/localizer.h>
#include <foundation/allocator.h>
#include <foundation/undo.h>
#include <foundation/log.h>
#include <plugins/entity/entity.h>
#include <plugins/entity/transform_component.h>
#include <plugins/ui/ui.h>
#include <plugins/editor_views/properties.h>
#include <plugins/the_machinery_shared/component_interfaces/editor_ui_interface.h>

#include <foundation/carray.inl>
#include <foundation/rect.inl>
#include <foundation/math.inl>

static const char *component_category(void)
{
    return TM_LOCALIZE("Interactables");
}

static tm_ci_editor_ui_i *editor_aspect = &(tm_ci_editor_ui_i){
    .category = component_category
};

enum lever_state {
    LEVER_STATE_LEVER_MOVING,
    LEVER_STATE_TARGET_MOVING,
};

typedef struct {
    enum lever_state state;
} active_lever_t;

typedef struct {
    double start_time;
    tm_entity_t interactable;

    union {
        active_lever_t lever;
    };
    TM_PAD(4);
} active_interaction_t;

struct tm_interactable_component_manager_o {
    tm_allocator_i allocator;
    tm_entity_context_o *ctx;
    active_interaction_t *active;
    uint32_t interactable_component_type;
    uint32_t transform_component_type;
    tm_transform_component_manager_o *trans_mgr;
};

static bool update_lever(tm_interactable_component_manager_o *mgr, float dt, double t, active_interaction_t *a, tm_interactable_component_t *c)
{
    bool res = false;
    switch (c->lever.state) {
        case TM_LEVER_STATE_CLOSED: {
            c->lever.state = TM_LEVER_STATE_LEVER_OPENING;
        } break;

        case TM_LEVER_STATE_LEVER_OPENING: {
            const float p = (float)(t - a->start_time)/c->lever.handle_open_time;
            const tm_vec4_t rot = tm_quaternion_nlerp(c->lever.handle_closed_rotation, c->lever.handle_open_rotation, tm_min(p, 1));
            tm_set_local_rotation(mgr->trans_mgr, c->lever.handle, rot);

            if (p >= 1) {
                a->start_time = t;
                c->lever.state = TM_LEVER_STATE_TARGET_OPENING;
            }
        } break;

        case TM_LEVER_STATE_TARGET_OPENING: {
            const float p = (float)(t - a->start_time)/c->lever.target_open_time;
            const tm_vec4_t rot = tm_quaternion_nlerp(c->lever.target_closed_rotation, c->lever.target_open_rotation,tm_min(p, 1));
            tm_set_local_rotation(mgr->trans_mgr, c->lever.target, rot);

            if (p >= 1) {
                res = true;
                c->lever.state = TM_LEVER_STATE_OPEN;
            }
        } break;

        case TM_LEVER_STATE_OPEN: {
            c->lever.state = TM_LEVER_STATE_LEVER_CLOSING;
        } break;

        case TM_LEVER_STATE_LEVER_CLOSING: {
            const float p = (float)(t - a->start_time)/c->lever.handle_open_time;
            const tm_vec4_t rot = tm_quaternion_nlerp(c->lever.handle_open_rotation, c->lever.handle_closed_rotation, tm_min(p, 1));
            tm_set_local_rotation(mgr->trans_mgr, c->lever.handle, rot);

            if (p >= 1) {
                a->start_time = t;
                c->lever.state = TM_LEVER_STATE_TARGET_CLOSING;
            }
        } break;

        case TM_LEVER_STATE_TARGET_CLOSING: {
            const float p = (float)(t - a->start_time)/c->lever.target_open_time;
            const tm_vec4_t rot = tm_quaternion_nlerp(c->lever.target_open_rotation, c->lever.target_closed_rotation, tm_min(p, 1));
            tm_set_local_rotation(mgr->trans_mgr, c->lever.target, rot);

            if (p >= 1) {
                res = true;
                c->lever.state = TM_LEVER_STATE_CLOSED;
            }
        } break;
    }

    return res;
}

static void update_active_interactables(tm_interactable_component_manager_o *mgr, float dt, double t)
{
    for (int32_t active_idx = 0; active_idx < (int32_t)tm_carray_size(mgr->active); ++active_idx) {
        active_interaction_t *a = mgr->active + active_idx;
        tm_interactable_component_t *c = tm_entity_api->get_component(mgr->ctx, a->interactable, mgr->interactable_component_type);

        if (!a->start_time)
            a->start_time = t;

        bool res = false;
        switch(c->type) {
            case TM_INTERACTABLE_TYPE_LEVER: {
                res = update_lever(mgr, dt, t, a, c);
            } break;
        }

        if (res)
            mgr->active[active_idx--] = tm_carray_pop(mgr->active);
    }
}

static void manager_init(tm_interactable_component_manager_o *mgr)
{
    mgr->transform_component_type = tm_entity_api->lookup_component(mgr->ctx, TM_TT_TYPE_HASH__TRANSFORM_COMPONENT);
    mgr->trans_mgr = (tm_transform_component_manager_o*)tm_entity_api->component_manager(mgr->ctx, mgr->transform_component_type);
}

static void manager_deinit(tm_interactable_component_manager_o *mgr)
{
    tm_carray_free(mgr->active, &mgr->allocator);
}

static void interact(tm_interactable_component_manager_o *mgr, tm_entity_t interactable)
{
    tm_carray_push(mgr->active, ((active_interaction_t) { .interactable = interactable } ), &mgr->allocator);
}

static bool can_interact(tm_interactable_component_manager_o *mgr, tm_entity_t interactable)
{
    for (int32_t active_idx = 0; active_idx < (int32_t)tm_carray_size(mgr->active); ++active_idx) {
        active_interaction_t *a = mgr->active + active_idx;

        if (a->interactable.u64 == interactable.u64)
            return false;
    }

    return true;
}

static struct tm_interactable_component_api *tm_interactable_component_api = &(struct tm_interactable_component_api) {
    .can_interact = can_interact,
    .interact = interact,
    .update_active_interactables = update_active_interactables,
};

static float component_properties_ui(struct tm_properties_ui_args_t *args, tm_rect_t item_rect, tm_tt_id_t component_id, uint32_t indent)
{
    tm_the_truth_o *tt = args->tt;
    tm_tt_id_t desc = tm_the_truth_api->get_subobject(tt, tm_tt_read(tt, component_id), TM_TT_PROP__INTERACTABLE_COMPONENT__DESC);
    const uint64_t desc_type = desc.index ? desc.type : 0;
    const uint64_t desc_type_hash = desc_type ? tm_the_truth_api->type_name_hash(tt, desc_type) : 0;

    const uint64_t type_hashes[] = {
        0,
        TM_TT_TYPE_HASH__INTERACTABLE_LEVER,
    };

    const char *type_names[] = {
        TM_LOCALIZE("None"),
        TM_LOCALIZE("Lever"),
    };

    const tm_rect_t label_r = tm_rect_split_left(item_rect, args->metrics[TM_PROPERTIES_METRIC_LABEL_WIDTH], args->metrics[TM_PROPERTIES_METRIC_MARGIN], 0);
    const tm_rect_t dropdown_r = tm_rect_split_left(item_rect, args->metrics[TM_PROPERTIES_METRIC_LABEL_WIDTH], args->metrics[TM_PROPERTIES_METRIC_MARGIN], 1);
    
    tm_properties_view_api->ui_label(args, label_r, TM_LOCALIZE("Type"), 0);
    
    const tm_ui_dropdown_t d = {
        .rect = dropdown_r,
        .items = type_names,
        .num_items = TM_ARRAY_COUNT(type_names),
    };

    uint32_t selected_idx = 0;

    for (uint32_t type_idx = 0; type_idx < TM_ARRAY_COUNT(type_hashes); ++type_idx) {
        if (type_hashes[type_idx] == desc_type_hash) {
            selected_idx = type_idx;
            break;
        }
    }

    if (tm_ui_api->dropdown(args->ui, args->uistyle, &d, &selected_idx)) {
        const tm_tt_undo_scope_t undo_scope = tm_the_truth_api->create_undo_scope(tt, TM_LOCALIZE("Change interactable type"));
        tm_the_truth_object_o *component_w = tm_the_truth_api->write(tt, component_id);
         
        if (desc.index) {
            if (tm_the_truth_api->is_alive(tt, desc))
                tm_the_truth_api->destroy_object(tt, desc, undo_scope);
        
            tm_the_truth_api->set_subobject(tt, component_w, TM_TT_PROP__INTERACTABLE_COMPONENT__DESC, 0);
        }

        const uint64_t new_hash = type_hashes[selected_idx];

        if (new_hash) {
            const uint64_t new_type = tm_the_truth_api->optional_object_type_from_name_hash(tt, new_hash);

            if (new_type) {
                desc = tm_the_truth_api->create_object_of_type(tt, new_type, undo_scope);
                tm_the_truth_api->set_subobject_id(tt, component_w, TM_TT_PROP__INTERACTABLE_COMPONENT__DESC, desc, undo_scope);
            }
        }

        tm_the_truth_api->commit(tt, component_w, undo_scope);
        args->undo_stack->add(args->undo_stack->inst, args->tt, undo_scope);
    }

    item_rect.y += item_rect.h + args->metrics[TM_PROPERTIES_METRIC_MARGIN];
    item_rect.y = tm_properties_view_api->ui_object(args, item_rect, desc, indent);
    return item_rect.y;
}

tm_properties_aspect_i *properties_aspect = &(tm_properties_aspect_i){
    .custom_ui = component_properties_ui,
};

static void truth__create_types(struct tm_the_truth_o* tt)
{
    tm_the_truth_property_definition_t interactable_component_properties[] = {
        [TM_TT_PROP__INTERACTABLE_COMPONENT__DESC] = { "desc", TM_THE_TRUTH_PROPERTY_TYPE_SUBOBJECT, .type_hash = TM_TT_TYPE_HASH__ANYTHING },
    };

    const uint64_t interactable_component_type = tm_the_truth_api->create_object_type(tt, TM_TT_TYPE__INTERACTABLE_COMPONENT, interactable_component_properties, TM_ARRAY_COUNT(interactable_component_properties));
    tm_the_truth_api->set_default_object_to_create_subobjects(tt, interactable_component_type);
    tm_the_truth_api->set_aspect(tt, interactable_component_type, TM_CI_EDITOR_UI, editor_aspect);
    tm_the_truth_api->set_aspect(tt, interactable_component_type, TM_TT_ASPECT__PROPERTIES, properties_aspect);

    tm_the_truth_property_definition_t lever_properties[] = {
        [TM_TT_PROP__INTERACTABLE_LEVER__HANDLE] = { "handle", TM_THE_TRUTH_PROPERTY_TYPE_REFERENCE, .type_hash = TM_TT_TYPE_HASH__ENTITY },
        [TM_TT_PROP__INTERACTABLE_LEVER__HANDLE_ROTATE_AXIS] = { "handle_rotate_axis", TM_THE_TRUTH_PROPERTY_TYPE_SUBOBJECT, .type_hash = TM_TT_TYPE_HASH__VEC3, },
        [TM_TT_PROP__INTERACTABLE_LEVER__HANDLE_ROTATE_ANGLE] = { "handle_rotate_angle", TM_THE_TRUTH_PROPERTY_TYPE_FLOAT },
        [TM_TT_PROP__INTERACTABLE_LEVER__HANDLE_OPEN_TIME] = { "handle_open_time", TM_THE_TRUTH_PROPERTY_TYPE_FLOAT },
        [TM_TT_PROP__INTERACTABLE_LEVER__TARGET] = { "target", TM_THE_TRUTH_PROPERTY_TYPE_REFERENCE, .type_hash = TM_TT_TYPE_HASH__ENTITY },
        [TM_TT_PROP__INTERACTABLE_LEVER__TARGET_ROTATE_AXIS] = { "target_rotate_axis", TM_THE_TRUTH_PROPERTY_TYPE_SUBOBJECT, .type_hash = TM_TT_TYPE_HASH__VEC3, },
        [TM_TT_PROP__INTERACTABLE_LEVER__TARGET_ROTATE_ANGLE] = { "target_rotate_angle", TM_THE_TRUTH_PROPERTY_TYPE_FLOAT },
        [TM_TT_PROP__INTERACTABLE_LEVER__TARGET_OPEN_TIME] = { "target_open_time", TM_THE_TRUTH_PROPERTY_TYPE_FLOAT },
    };

    const uint64_t interactable_lever_type = tm_the_truth_api->create_object_type(tt, TM_TT_TYPE__INTERACTABLE_LEVER, lever_properties, TM_ARRAY_COUNT(lever_properties));
    tm_the_truth_api->set_property_aspect(tt, interactable_lever_type, TM_TT_PROP__INTERACTABLE_LEVER__HANDLE, TM_TT_PROP_ASPECT__PROPERTIES__USE_LOCAL_ENTITY_PICKER, (void *)1);
    tm_the_truth_api->set_property_aspect(tt, interactable_lever_type, TM_TT_PROP__INTERACTABLE_LEVER__TARGET, TM_TT_PROP_ASPECT__PROPERTIES__USE_LOCAL_ENTITY_PICKER, (void *)1);
    tm_the_truth_api->set_default_object_to_create_subobjects(tt, interactable_lever_type);
}

static void component__asset_loaded(tm_component_manager_o *mgr_in, tm_entity_t e, void *data)
{
    tm_interactable_component_manager_o *mgr = (tm_interactable_component_manager_o*)mgr_in;
    tm_interactable_component_t* c = data;
    tm_entity_context_o *ctx = mgr->ctx;
    const tm_tt_id_t entity_asset = tm_entity_api->asset(mgr->ctx, e);
    tm_the_truth_o *tt = tm_entity_api->the_truth(mgr->ctx);
    const uint64_t interactable_tt_type = tm_the_truth_api->object_type_from_name_hash(tt, TM_TT_TYPE_HASH__INTERACTABLE_COMPONENT);
    const tm_tt_id_t asset = tm_the_truth_api->find_subobject_of_type(tt, tm_tt_read(tt, entity_asset), TM_TT_PROP__ENTITY__COMPONENTS, interactable_tt_type);
    const tm_the_truth_object_o* asset_r = tm_tt_read(tt, asset);
    const tm_tt_id_t desc = tm_the_truth_api->get_subobject(tt, asset_r, TM_TT_PROP__INTERACTABLE_COMPONENT__DESC);
    const uint64_t desc_type = desc.type;
    const uint64_t desc_type_hash = tm_the_truth_api->type_name_hash(tt, desc_type);
    const tm_the_truth_object_o *desc_r = tm_tt_read(tt, desc);

    switch (desc_type_hash) {
        case TM_TT_TYPE_HASH__INTERACTABLE_LEVER: {
            c->type = TM_INTERACTABLE_TYPE_LEVER;
            tm_interactable_lever_t *l = &c->lever;
            const tm_tt_id_t handle = tm_the_truth_api->get_reference(tt, desc_r, TM_TT_PROP__INTERACTABLE_LEVER__HANDLE);
            const float handle_rotation_angle = tm_the_truth_api->get_float(tt, desc_r, TM_TT_PROP__INTERACTABLE_LEVER__HANDLE_ROTATE_ANGLE) * (TM_PI/180.f);
            const tm_vec3_t handle_rotation_axis = tm_vec3_normalize(tm_the_truth_common_types_api->get_vec3(tt, desc_r, TM_TT_PROP__INTERACTABLE_LEVER__HANDLE_ROTATE_AXIS));
            if (handle.index) {
                l->handle = tm_entity_api->resolve_asset_reference(ctx, e, handle);
                l->handle_closed_rotation = tm_get_local_rotation(mgr->trans_mgr, l->handle);
                l->handle_open_rotation = tm_quaternion_mul(c->lever.handle_closed_rotation, tm_quaternion_from_rotation(handle_rotation_axis, handle_rotation_angle));
                l->handle_open_time = tm_the_truth_api->get_float(tt, desc_r, TM_TT_PROP__INTERACTABLE_LEVER__HANDLE_OPEN_TIME);
                if (l->handle_open_time < 0.001)
                    l->handle_open_time = 1.0f;
            }
            const tm_tt_id_t target = tm_the_truth_api->get_reference(tt, desc_r, TM_TT_PROP__INTERACTABLE_LEVER__TARGET);
            const float target_rotation_angle = tm_the_truth_api->get_float(tt, desc_r, TM_TT_PROP__INTERACTABLE_LEVER__TARGET_ROTATE_ANGLE) * (TM_PI/180.f); 
            const tm_vec3_t taret_rotation_axis = tm_vec3_normalize(tm_the_truth_common_types_api->get_vec3(tt, desc_r, TM_TT_PROP__INTERACTABLE_LEVER__TARGET_ROTATE_AXIS));
            if (target.index) {
                l->target = tm_entity_api->resolve_asset_reference(ctx, e, target);
                l->target_closed_rotation = tm_get_local_rotation(mgr->trans_mgr, l->target);
                l->target_open_rotation = tm_quaternion_mul(c->lever.target_closed_rotation, tm_quaternion_from_rotation(taret_rotation_axis, target_rotation_angle));
                l->target_open_time = tm_the_truth_api->get_float(tt, desc_r, TM_TT_PROP__INTERACTABLE_LEVER__TARGET_OPEN_TIME);
                if (l->target_open_time < 0.001)
                    l->target_open_time = 1.0f;
            }
        } break;
    }
}

static void component__destroy(tm_component_manager_o *mgr_in)
{
    tm_interactable_component_manager_o *mgr = (tm_interactable_component_manager_o*)mgr_in;
    manager_deinit(mgr);
    tm_allocator_i a = mgr->allocator;
    tm_entity_context_o *ctx = mgr->ctx;
    tm_free(&a, mgr, sizeof(*mgr));
    tm_entity_api->destroy_child_allocator(ctx, &a);
}

static void manager_components_created(tm_component_manager_o *man_in)
{
    tm_interactable_component_manager_o *man = (tm_interactable_component_manager_o*)man_in;
    manager_init(man);
}

static tm_interactable_component_manager_o *component__create(struct tm_entity_context_o* ctx)
{
    tm_allocator_i a;
    tm_entity_api->create_child_allocator(ctx, TM_TT_TYPE__INTERACTABLE_COMPONENT, &a);
    tm_interactable_component_manager_o *m = tm_alloc(&a, sizeof(*m));

    tm_component_i component = {
        .name = TM_TT_TYPE__INTERACTABLE_COMPONENT,
        .bytes = sizeof(struct tm_interactable_component_t),
        .asset_loaded = component__asset_loaded,
        .destroy = component__destroy,
        .manager = (tm_component_manager_o*)m,
        .components_created = manager_components_created,
    };

    const uint32_t interactable_component_type = tm_entity_api->register_component(ctx, &component);

    *m = (tm_interactable_component_manager_o) {
        .allocator = a,
        .ctx = ctx,
        .interactable_component_type = interactable_component_type,
    };

    return m;
}

#define tm_set_api(reg, load, ptr) \
    if (load)                                      \
        reg->set(#ptr, ptr, sizeof(*ptr));         \
    else                                           \
        reg->remove(ptr)

void load_interactable_component(struct tm_api_registry_api* reg, bool load)
{
    load_apis(reg);

    tm_set_api(reg, load, tm_interactable_component_api);
    tm_add_or_remove_implementation(reg, load, TM_THE_TRUTH_CREATE_TYPES_INTERFACE_NAME, truth__create_types);
    tm_add_or_remove_implementation(reg, load, TM_ENTITY_CREATE_COMPONENT_INTERFACE_NAME, component__create);
}
