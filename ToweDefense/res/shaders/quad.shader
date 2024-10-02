#shader vertex
#version 330 core
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Colour;
layout(location = 2) in vec2 in_TexCoords;
out vec4 ex_Color;
out vec2 ex_TexCoords;

uniform mat4 ModelMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;



void main()
{

	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(in_Position, 1.0);
	ex_Color = vec4(in_Colour, 1.0);
	ex_TexCoords = in_TexCoords;
};

#shader fragment
#version 330 core

in vec4 ex_Color;
in vec2 ex_TexCoords;
out vec4 out_Color;

uniform sampler2D diffuseMap;

void main()
{
	out_Color = texture(diffuseMap, ex_TexCoords);
};