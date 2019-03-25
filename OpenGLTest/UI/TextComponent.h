#pragma once

#include "UIManager.h"

class TextComponent : public UIComponent {
	public:
		TextComponent(std::string text, float fontSize, float x, float y, TextureEnum fontType = TEXTURE_FONT_SHARE_TECH_MONO);

		void Resize();
		bool IsTransparent();
		void SetImage(std::string path);

		// Change the text on this component
		void SetText(std::string text);
	private:
		glm::vec2 getUVfromChar(const char c);
		void generateVertices();

		std::string _text;
		float       _fontSize;
};