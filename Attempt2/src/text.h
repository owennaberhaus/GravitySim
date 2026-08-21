#pragma once
#include <glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <string>
#include <vector>

// Screen-space text overlay. Positions all relative to window as opposed to camera which was jerky and slow
class TextRenderer
{
public:
	enum class Anchor { TopLeft, TopRight, BottomLeft, BottomRight };

	TextRenderer();
	~TextRenderer();

	TextRenderer(const TextRenderer&) = delete;
	TextRenderer& operator=(const TextRenderer&) = delete;

	// Wrap draw calls in a Begin/End pair
	void Begin(int width, int height);
	void End();

	// x and y are pixels from the top-left corner, y growing downwards.
	void Draw(const std::string& text, float x, float y,
		float scale = 2.0f, glm::vec3 color = glm::vec3(1.0f));

	void DrawAnchored(const std::string& text, Anchor anchor, int line,
		float scale = 2.0f, glm::vec3 color = glm::vec3(1.0f), float margin = 12.0f);

	float LineHeight(float scale) const { return (kGlyphH + 3.0f) * scale; }
	float TextWidth(const std::string& text, float scale) const
	{
		return static_cast<float>(text.size()) * kGlyphW * scale;
	}

private:
	static const int kGlyphW = 8;
	static const int kGlyphH = 14;
	static const int kFirst = 32; // space
	static const int kCount = 95; // through

	void Push(std::vector<float>& out, const std::string& text, float x, float y, float scale);

	GLuint m_program{ 0 };
	GLuint m_texture{ 0 };
	GLuint m_vao{ 0 };
	GLuint m_vbo{ 0 };
	GLint m_projLoc{ -1 };
	GLint m_colorLoc{ -1 };

	int m_width{ 0 };
	int m_height{ 0 };
	GLboolean m_depthWasOn{ GL_FALSE };
};
