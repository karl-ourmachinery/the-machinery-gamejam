#include <foundation/allocator.h>
#include <foundation/api_registry.h>

#include <plugins/simulate/simulate_entry.h>
#include <plugins/simulate_common/simulate_context.h>

#include <plugins/simulate/simulate_helpers.inl>

static struct tm_api_registry_api *tm_api_registry_api;
static struct tm_simulate_context_api *tm_simulate_context_api;

struct tm_simulate_state_o {
    tm_entity_t player_camera;
    tm_simulate_helpers_context_t h;
    tm_allocator_i *allocator;
} tm_gameplay_state_o;

static tm_simulate_state_o *start(struct tm_allocator_i *allocator, struct tm_entity_context_o *entity_ctx,
    struct tm_simulate_context_o *simulate_ctx)
{
    tm_simulate_state_o *state = tm_alloc(allocator, sizeof(*state));
    *state = (tm_simulate_state_o) {
        .allocator = allocator,
    };

    tm_simulate_helpers_context_t *h = &state->h;
    tm_simulate_helpers_init_context(tm_api_registry_api, h, entity_ctx);

    state->player_camera = tm_entity_find_with_tag(TM_STATIC_HASH("player_camera", 0x689cd442a211fda4ULL), h);
    tm_simulate_context_api->set_camera(simulate_ctx, state->player_camera);
    
    return state;
}

static void stop(tm_simulate_state_o *state)
{
    // Clean up when simulation ends.

    tm_allocator_i a = *state->allocator;
    tm_free(&a, state, sizeof(*state));
}

static void update(tm_simulate_state_o *state, tm_simulate_frame_args_t *args)
{
    // Called once a frame.
}

static tm_simulate_entry_i simulate_entry_i = {
     // Change this and re-run hash.exe if you wish to change the unique identifier
    .id = TM_STATIC_HASH("An Island of Doors", 0x55b1c4a0e742df92ULL),
    .display_name = "An Island of Doors",
    .start = start,
    .stop = stop,
    .update = update,
};

TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_api* reg, bool load)
{
    tm_api_registry_api = reg;
    tm_simulate_context_api = reg->get(TM_SIMULATE_CONTEXT_API_NAME);
    tm_add_or_remove_implementation(reg, load, TM_SIMULATE_ENTRY_INTERFACE_NAME, &simulate_entry_i);
}
