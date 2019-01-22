#pragma once

// Enumerated type used for indexing and referencing events
enum EventName {
	RENDERER_ADD_TO_RENDERABLES,
    RENDERER_ADD_TO_UIRENDERABLES, // Tell renderer to add renderable to list
    RENDERER_POPULATE_BUFFERS, // Tell renderer to repopulate buffers for a renderable
    RENDERER_SET_CAMERA, // Set Camera Position
    RENDERER_REPOPULATE_BUFFERS,
};
