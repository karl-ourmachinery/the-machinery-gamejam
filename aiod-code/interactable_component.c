#include "api_loader.inl"

TM_LOAD_APIS(tm_load_apis,
    tm_entity_api,
    tm_the_truth_api,
    tm_localizer_api,
    tm_properties_view_api,
    tm_ui_api);

#include "interactable_component.h"
#include <foundation/macros.h>
#include <foundation/the_truth.h>
#include <foundation/localizer.h>
#include <foundation/undo.h>
#include <plugins/entity/entity.h>
#include <plugins/ui/ui.h>
#include <plugins/editor_views/properties.h>
#include <plugins/the_machinery_shared/component_interfaces/editor_ui_interface.h>

#include <foundation/rect.inl>

static const char *component_category(void)
{
    return TM_LOCALIZE("Interactables");
}

static tm_ci_editor_ui_i *editor_aspect = &(tm_ci_editor_ui_i){
    .category = component_category
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
        [TM_TT_PROP__INTERACTABLE_LEVER__TARGET] = { "target", TM_THE_TRUTH_PROPERTY_TYPE_REFERENCE, .type_hash = TM_TT_TYPE_HASH__ENTITY },
    };

    const uint64_t interactable_lever_type = tm_the_truth_api->create_object_type(tt, TM_TT_TYPE__INTERACTABLE_LEVER, lever_properties, TM_ARRAY_COUNT(lever_properties));
    tm_the_truth_api->set_property_aspect(tt, interactable_lever_type, TM_TT_PROP__INTERACTABLE_LEVER__HANDLE, TM_TT_PROP_ASPECT__PROPERTIES__USE_LOCAL_ENTITY_PICKER, (void *)1);
    tm_the_truth_api->set_property_aspect(tt, interactable_lever_type, TM_TT_PROP__INTERACTABLE_LEVER__TARGET, TM_TT_PROP_ASPECT__PROPERTIES__USE_LOCAL_ENTITY_PICKER, (void *)1);
    tm_the_truth_api->set_default_object_to_create_subobjects(tt, interactable_lever_type);
}

static bool component__load_asset(tm_component_manager_o* man, tm_entity_t e, void* c_vp, const tm_the_truth_o* tt, tm_tt_id_t asset)
{
    struct tm_interactable_component_t* c = c_vp;
    struct tm_entity_context_o *ctx = (struct tm_entity_context_o *)man;
    const tm_the_truth_object_o* asset_r = tm_tt_read(tt, asset);
    const tm_tt_id_t desc = tm_the_truth_api->get_subobject(tt, asset_r, TM_TT_PROP__INTERACTABLE_COMPONENT__DESC);
    const uint64_t desc_type = desc.type;
    const uint64_t desc_type_hash = tm_the_truth_api->type_name_hash(tt, desc_type);

    switch (desc_type_hash) {
        case TM_TT_TYPE_HASH__INTERACTABLE_LEVER: {
            c->type = TM_INTERACTABLE_TYPE_LEVER;
            tm_interactable_lever_t *l = &c->lever;
            const tm_tt_id_t handle = tm_the_truth_api->get_reference(tt, tm_tt_read(tt, desc), TM_TT_PROP__INTERACTABLE_LEVER__HANDLE);
            if (handle.index)
                l->handle = tm_entity_api->resolve_asset_reference(ctx, e, handle);
            const tm_tt_id_t target = tm_the_truth_api->get_reference(tt, tm_tt_read(tt, desc), TM_TT_PROP__INTERACTABLE_LEVER__TARGET);
            if (target.index)
                l->target = tm_entity_api->resolve_asset_reference(ctx, e, target);
        } break;
        default: {
            return false;
        } break;
    }

    return true;
}

static void component__create(struct tm_entity_context_o* ctx)
{
    tm_component_i component = {
        .name = TM_TT_TYPE__INTERACTABLE_COMPONENT,
        .bytes = sizeof(struct tm_interactable_component_t),
        .load_asset = component__load_asset,
        .manager = (tm_component_manager_o*)ctx,
    };

    tm_entity_api->register_component(ctx, &component);
}

void load_interactable_component(struct tm_api_registry_api* reg, bool load)
{
    tm_load_apis(reg);

    tm_add_or_remove_implementation(reg, load, TM_THE_TRUTH_CREATE_TYPES_INTERFACE_NAME, truth__create_types);
    tm_add_or_remove_implementation(reg, load, TM_ENTITY_CREATE_COMPONENT_INTERFACE_NAME, component__create);
}
