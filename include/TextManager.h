#pragma once

#include <memory>
#include <map>
#include "mat4.h"
#include "Types.h"
using namespace ftgl;


extern mat4 projection;
extern mat4 model;
extern mat4 view;

namespace ftgl {
struct Glyph;
class VertexBuffer;
class TextureAtlas;
class Font;
}


struct Vertex
{
	float x, y, z;    // position
	float s, t;       // texture
	float r, g, b, a; // color
};


template <class T>
class TSingleton
{
public:
	TSingleton() = default;
	TSingleton(const TSingleton &) = delete;
	TSingleton &operator=(const TSingleton &) = delete;

	static T *Instance() {
		static T mInstance;

		return &mInstance;
	}

private:

};


class TextManager final : public TSingleton<TextManager>
{
	friend class TSingleton<TextManager>;

public:

	Font *getFont(float Pt);



	Font *getFont() {
		return getFont(FontPt);
	}

	void renderText(VertexBuffer* Buffer) const;


private:
	TextManager();

private:
	unsigned Shader;


	std::string FontName = "assets/fonts/Consolas.ttf";
	float FontPt = 10.0f;

	std::unique_ptr<TextureAtlas> Atlas;
	std::map<unsigned, std::unique_ptr<Font>> Fonts;
};


void SetGlyphVertices(const Glyph *glyph, Position pos, Color col, Vertex *Vertices);

