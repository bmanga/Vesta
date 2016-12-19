#include "TextManager.h"

#include "opengl.h"
#include "TextureAtlas.h"
#include "TextureFont.h"
#include "shader.h"
#include "VertexBuffer.h"
mat4   model, view, projection;


Font* TextManager::getFont(float Pt)
{
	auto It = Fonts.find(unsigned(Pt));

	if (It == Fonts.end())
	{
		It = Fonts.insert({
			          unsigned(Pt),
			          std::make_unique<Font>(Atlas.get(), Pt,
			                                 Font::File{FontName.c_str()})
		          }).first;
	}

	return It->second.get();
}

void TextManager::renderText(VertexBuffer* Buffer) const
{
	Atlas->upload();

	glBindTexture(GL_TEXTURE_2D, Atlas->id());

	glUseProgram(Shader);

	glUniform1i(glGetUniformLocation(Shader, "texture"),
	                                0);
	glUniformMatrix4fv(glGetUniformLocation(Shader, "model"),
	                                       1, 0, model.data);
	glUniformMatrix4fv(glGetUniformLocation(Shader, "view"),
	                                       1, 0, view.data);
	glUniformMatrix4fv(glGetUniformLocation(Shader, "projection"),
	                                       1, 0, projection.data);

	Buffer->render(GL_TRIANGLES);
}

TextManager::TextManager():
	Atlas(std::make_unique<TextureAtlas>(256, 256, 1))
{
	Shader = shader_load("assets/shaders/v3f-t2f-c4f.vert",
	                     "assets/shaders/v3f-t2f-c4f.frag");
}


 void SetGlyphVertices(const Glyph *glyph, Position pos, Color col, Vertex *Vertices) {
	int x0 = int(pos.x + glyph->offset_x);
	int y0 = int(pos.y + glyph->offset_y);
	int x1 = int(x0 + glyph->width);
	int y1 = int(y0 - glyph->height);
	float s0 = glyph->s0;
	float t0 = glyph->t0;
	float s1 = glyph->s1;
	float t1 = glyph->t1;

	float r = col.r;
	float g = col.g;
	float b = col.b;
	float a = col.a;


	Vertices[0] = { float(x0), float(y0), pos.z,  s0, t0, r, g, b, a };
	Vertices[1] = { float(x0), float(y1), pos.z,  s0, t1, r, g, b, a };
	Vertices[2] = { float(x1), float(y1), pos.z,  s1, t1, r, g, b, a };
	Vertices[3] = { float(x1), float(y0), pos.z,  s1, t0, r, g, b, a };

}

