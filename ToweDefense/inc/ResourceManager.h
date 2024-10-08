#pragma once
#include "glew.h"
#include <unordered_map>
#include "glBuffer.h"
#include "glVertexArray.h"

namespace ResourceManager
{
	
	void AddBuffer(unsigned int buffID);
	void RemoveBuffer(unsigned int buffID);
	void AddVertexArray(unsigned int vaID);
	void RemoveVertexArray(unsigned int vaID);
	void Clear();
};

