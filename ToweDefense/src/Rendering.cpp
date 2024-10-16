#include "Rendering.h"

namespace Rendering {

	void RenderSpriteVA(VertexArray& va){
		
		va.Bind();
		glDrawElements(GL_TRIANGLES, va.indexCount, GL_UNSIGNED_INT, (void*)0);
		va.UnBind();
		
	
	}











}