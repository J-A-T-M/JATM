#pragma once

enum ModelEnum {
	MODEL_CUBE,
	MODEL_CUBE_BEVEL,
	MODEL_SPHERE,
	MODEL_SUZANNE,
	NUM_MODELS
};

enum TextureEnum {
	TEXTURE_NONE,
	TEXTURE_UV_GRID,
	NUM_TEXTURES,
};

enum InputSourceEnum {
	INPUT_LOCAL1,
	INPUT_LOCAL2,
	INPUT_LOCAL3,
	INPUT_LOCAL4,
	INPUT_CLIENT1,
	INPUT_CLIENT2,
	INPUT_CLIENT3,
	INPUT_CLIENT4,
	INPUT_CLIENT5,
	INPUT_CLIENT6,
	INPUT_CLIENT7,
	NUM_INPUTS
};

enum ServerPacketType {
	GAME_STATE,
	HAZARD
};