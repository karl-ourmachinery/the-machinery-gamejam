#include "api_loader.inl"

TM_LOAD_APIS(tm_load_apis,
    tm_simulate_context_api,
    tm_input_api,
    tm_ui_api,
    tm_error_api,
    tm_entity_api,
    tm_application_api,
    tm_draw2d_api,
    tm_temp_allocator_api,
    tm_physx_scene_api);

#include "interactable_component.h"

#include <foundation/allocator.h>
#include <foundation/api_registry.h>
#include <foundation/input.h>
#include <foundation/application.h>
#include <foundation/temp_allocator.h>

#include <plugins/simulate/simulate_entry.h>
#include <plugins/simulate_common/simulate_context.h>
#include <plugins/ui/ui.h>
#include <plugins/ui/draw2d.h>
#include <plugins/entity/entity.h>
#include <plugins/physics/physics_mover_component.h>
#include <plugins/physx/physx_scene.h>

#include <plugins/simulate/simulate_helpers.inl>

typedef struct input_state_t {
    bool held_keys[TM_INPUT_KEYBOARD_ITEM_COUNT];
    bool left_mouse_held;
    bool left_mouse_pressed;
    TM_PAD(1);
    tm_vec2_t mouse_delta;
} input_state_t;

typedef struct component_indices_t {
    uint32_t interactable;
} component_indices_t;

struct tm_simulate_state_o {
    tm_entity_t player;
    input_state_t input;

    tm_entity_t player_camera;
    tm_entity_t door;
    tm_entity_t lever;

    tm_tt_id_t player_collision_type;
    tm_vec4_t door_initial_rot;
    tm_vec4_t lever_initial_rot;

    bool mouse_captured;
    TM_PAD(3);
    float look_yaw;
    float look_pitch;
    
    float door_end_angle;
    double door_start_open_time;

    float lever_end_angle;
    TM_PAD(4);
    double lever_start_move_time;

    uint64_t processed_events;
    tm_simulate_helpers_context_t h;
    tm_entity_context_o *entity_ctx;
    tm_simulate_context_o *simulate_ctx;
    tm_allocator_i *allocator;

    component_indices_t comp_idx;
    TM_PAD(4);
} tm_gameplay_state_o;

static tm_simulate_state_o *start(struct tm_allocator_i *allocator, tm_entity_context_o *entity_ctx,
    tm_simulate_context_o *simulate_ctx)
{
    tm_simulate_state_o *state = tm_alloc(allocator, sizeof(*state));
    *state = (tm_simulate_state_o) {
        .allocator = allocator,
        .entity_ctx = entity_ctx,
        .simulate_ctx = simulate_ctx,
    };

    tm_simulate_helpers_context_t *h = &state->h;
    tm_simulate_helpers_init_context(tm_api_registry_api, h, entity_ctx);

    state->comp_idx.interactable = tm_entity_api->lookup_component(entity_ctx, TM_TT_TYPE_HASH__INTERACTABLE_COMPONENT);

    state->player_camera = tm_entity_find_with_tag(TM_STATIC_HASH("player_camera", 0x689cd442a211fda4ULL), h);
    state->player = tm_entity_find_with_tag(TM_STATIC_HASH("player", 0xafff68de8a0598dfULL), h);
    tm_simulate_context_api->set_camera(simulate_ctx, state->player_camera);
    
    TM_INIT_TEMP_ALLOCATOR_WITH_ADAPTER(ta, a);
    const tm_hash_u64_to_id_t collision_types = tm_physics_get_collison_type_lookup(a, h);
    state->player_collision_type = tm_hash_get_rv(&collision_types, TM_STATIC_HASH("player", 0xafff68de8a0598dfULL));
    TM_SHUTDOWN_TEMP_ALLOCATOR(ta);
    
    return state;
}

static void stop(tm_simulate_state_o *state)
{
    // Clean up when simulation ends.

    tm_allocator_i a = *state->allocator;
    tm_free(&a, state, sizeof(*state));
}

static float door_open_speed = 1.0f;

static void open_door(tm_simulate_state_o *state, tm_simulate_frame_args_t *args)
{
    if (!state->door_start_open_time)
        return;

    const float t = (float)(args->time - state->door_start_open_time)/door_open_speed;
    const tm_vec4_t rot = tm_quaternion_mul(state->door_initial_rot, tm_quaternion_from_rotation((tm_vec3_t){ 0, 1, 0}, tm_lerp(0, state->door_end_angle, tm_min(t, 1))));
    tm_entity_set_local_rotation(state->door, rot, &state->h);
}

static float lever_move_speed = 0.3f;

static void open_lever(tm_simulate_state_o *state, tm_simulate_frame_args_t *args)
{
    if (!state->lever_start_move_time)
        return;

    const float t = (float)(args->time - state->lever_start_move_time)/lever_move_speed;
    const tm_vec4_t rot = tm_quaternion_mul(state->lever_initial_rot, tm_quaternion_from_rotation((tm_vec3_t){ 1, 0, 0}, tm_lerp(0, state->lever_end_angle, tm_min(t, 1))));
    tm_entity_set_local_rotation(state->lever, rot, &state->h);

    if (t >= 1 && !state->door_start_open_time) {
        state->door_end_angle = -TM_PI/2;
        state->door_start_open_time = args->time;
    }
}

static void update(tm_simulate_state_o *state, tm_simulate_frame_args_t *args)
{
    // Reset per-frame-input
    state->input.mouse_delta.x = state->input.mouse_delta.y = 0;
    state->input.left_mouse_pressed = false;

    // Read input
    tm_input_event_t events[32];
    bool mouse_captured_this_frame = state->mouse_captured;
    while (true) {
        uint64_t n = tm_input_api->events(state->processed_events, events, 32);

        for (uint64_t i = 0; i < n; ++i) {
            const tm_input_event_t* e = events + i;
            if (mouse_captured_this_frame) {
                if (e->source && e->source->controller_type == TM_INPUT_CONTROLLER_TYPE_MOUSE) {
                    if (e->item_id == TM_INPUT_MOUSE_ITEM_BUTTON_LEFT) {
                        const bool down = e->data.f.x > 0.5f;
                        state->input.left_mouse_pressed = down && !state->input.left_mouse_held;
                        state->input.left_mouse_held = down;
                    } else if (e->item_id == TM_INPUT_MOUSE_ITEM_MOVE) {
                        state->input.mouse_delta.x += e->data.f.x;
                        state->input.mouse_delta.y += e->data.f.y;
                    }
                }
                if (e->source && e->source->controller_type == TM_INPUT_CONTROLLER_TYPE_KEYBOARD) {
                    if (e->type == TM_INPUT_EVENT_TYPE_DATA_CHANGE) {
                        state->input.held_keys[e->item_id] = e->data.f.x == 1.0f;
                    }
                }
            } else {
                if (e->source && e->source->controller_type == TM_INPUT_CONTROLLER_TYPE_MOUSE) {
                    if (e->item_id == TM_INPUT_MOUSE_ITEM_BUTTON_LEFT) {
                        const bool down = e->data.f.x > 0.5f;
                        if (down && !state->input.left_mouse_held) {
                            if (!args->running_in_editor || (tm_ui_api->is_hovering(args->ui, args->rect, 0))) {
                                state->mouse_captured = true;
                            }
                        }
                        state->input.left_mouse_held = down;
                    }
                }

                if (e->source && e->source->controller_type == TM_INPUT_CONTROLLER_TYPE_KEYBOARD) {
                    if (e->item_id == TM_INPUT_KEYBOARD_ITEM_ESCAPE && e->type == TM_INPUT_EVENT_TYPE_DATA_CHANGE) {
                        state->input.held_keys[e->item_id] = e->data.f.x == 1.0f;
                    }
                }
            }
        }

        state->processed_events += n;
        if (n < 32)
            break;
    }

    // Capture mouse
    {
        if ((args->running_in_editor && state->input.held_keys[TM_INPUT_KEYBOARD_ITEM_ESCAPE]) || !tm_ui_api->window_has_focus(args->ui)) {
            state->mouse_captured = false;
            tm_application_api->set_cursor_hidden(tm_application_api->application(), false);
        }

        if (state->mouse_captured)
            tm_application_api->set_cursor_hidden(tm_application_api->application(), true);
    }

    open_lever(state, args);
    open_door(state, args);

    tm_simulate_helpers_context_t *h = &state->h;
    const tm_vec3_t camera_pos = tm_entity_get_position(state->player_camera, h);
    const tm_vec4_t camera_rot = tm_entity_get_rotation(state->player_camera, h);
    struct tm_physx_mover_component_t* player_mover = tm_entity_api->get_component_by_hash(state->entity_ctx, state->player, TM_TT_TYPE_HASH__PHYSX_MOVER_COMPONENT);

    if (!TM_ASSERT(player_mover, tm_error_api->def, "Invalid player"))
        return;


    // Process input if mouse is captured.
    if (state->mouse_captured) {
        // Exit on ESC
        if (!args->running_in_editor && state->input.held_keys[TM_INPUT_KEYBOARD_ITEM_ESCAPE])
            tm_application_api->exit(tm_application_api->application());

        tm_vec3_t local_movement = { 0 };
        if (state->input.held_keys[TM_INPUT_KEYBOARD_ITEM_A])
            local_movement.x -= 1.0f;
        if (state->input.held_keys[TM_INPUT_KEYBOARD_ITEM_D])
            local_movement.x += 1.0f;
        if (state->input.held_keys[TM_INPUT_KEYBOARD_ITEM_W])
            local_movement.z -= 1.0f;
        if (state->input.held_keys[TM_INPUT_KEYBOARD_ITEM_S])
            local_movement.z += 1.0f;

        // Move
        if (tm_vec3_length(local_movement) != 0) {
            tm_vec3_t rotated_movement = tm_quaternion_rotate_vec3(camera_rot, local_movement);
            rotated_movement.y = 0;
            const tm_vec3_t normalized_rotated_movement = tm_vec3_normalize(rotated_movement);
            const tm_vec3_t final_movement = tm_vec3_mul(normalized_rotated_movement, 5);
            player_mover->velocity.x = final_movement.x;
            player_mover->velocity.z = final_movement.z;
        } else {
            player_mover->velocity.x = 0;
            player_mover->velocity.z = 0;
        }

        // Look
        const float mouse_sens = 0.1f * args->dt;
        state->look_yaw -= state->input.mouse_delta.x * mouse_sens;
        state->look_pitch -= state->input.mouse_delta.y * mouse_sens;
        state->look_pitch = tm_clamp(state->look_pitch, -TM_PI / 3, TM_PI / 3);
        const tm_vec4_t yawq = tm_quaternion_from_rotation((tm_vec3_t){ 0, 1, 0 }, state->look_yaw);
        const tm_vec3_t local_sideways = tm_quaternion_rotate_vec3(yawq, (tm_vec3_t){ 1, 0, 0 });
        const tm_vec4_t pitchq = tm_quaternion_from_rotation(local_sideways, state->look_pitch);
        tm_entity_set_local_rotation(state->player_camera, tm_quaternion_mul(pitchq, yawq), h);

        // Jump
        if (state->input.held_keys[TM_INPUT_KEYBOARD_ITEM_SPACE] && player_mover->is_standing)
            player_mover->velocity.y = 3.5;
    }

    // Modified if the raycast below hits the box.
    tm_color_srgb_t crosshair_color = { 70, 80, 70, 255 };

    const tm_vec3_t camera_forward = tm_quaternion_rotate_vec3(camera_rot, (tm_vec3_t){ 0, 0, -1 });
    const tm_physx_raycast_t r = tm_physx_scene_api->raycast(args->physx_scene, camera_pos, camera_forward, 2.5f, state->player_collision_type, (tm_physx_raycast_flags_t){ 0 }, 0, 0);

    if (r.has_block) {
        const tm_entity_t hit = r.block.body;
        const tm_component_mask_t *hit_mask = tm_entity_api->component_mask(state->entity_ctx, hit);
        if (tm_entity_mask_has_component(hit_mask, state->comp_idx.interactable)) {
            crosshair_color = (tm_color_srgb_t){ 255, 255, 255, 255 };
            if (state->input.left_mouse_pressed) {
                tm_interactable_component_t *ic = tm_entity_api->get_component(state->entity_ctx, hit, state->comp_idx.interactable);

                if (ic->type == TM_INTERACTABLE_TYPE_LEVER) {
                    state->lever_start_move_time = 0;
                    state->door_start_open_time = 0;
                    state->door = ic->lever.target; 
                    state->door_initial_rot = tm_entity_get_local_rotation(state->door, h);

                    state->lever = ic->lever.handle;
                    state->lever_initial_rot = tm_entity_get_local_rotation(state->lever, h);
                    state->lever_end_angle = TM_PI/5;
                    state->lever_start_move_time = args->time;
                }
            }
        }
    }

    // UI: Crosshair
    tm_ui_buffers_t uib = tm_ui_api->buffers(args->ui);
    tm_vec2_t crosshair_pos = { args->rect.w / 2, args->rect.h / 2 };
    tm_draw2d_style_t style[1] = { 0 };
    tm_ui_api->to_draw_style(args->ui, style, args->uistyle);
    style->color = crosshair_color;
    tm_draw2d_api->fill_circle(uib.vbuffer, uib.ibuffers[TM_UI_BUFFER_MAIN], style, crosshair_pos, 3);

}

static tm_simulate_entry_i simulate_entry_i = {
     // Change this and re-run hash.exe if you wish to change the unique identifier
    .id = TM_STATIC_HASH("An Island of Doors", 0x55b1c4a0e742df92ULL),
    .display_name = "An Island of Doors",
    .start = start,
    .stop = stop,
    .update = update,
};

extern void load_interactable_component(struct tm_api_registry_api* reg, bool load);

TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_api* reg, bool load)
{
    tm_load_apis(reg);
    tm_add_or_remove_implementation(reg, load, TM_SIMULATE_ENTRY_INTERFACE_NAME, &simulate_entry_i);
    load_interactable_component(reg, load);
}
