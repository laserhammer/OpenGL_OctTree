#version 150

in vec3 pos;
in vec3 color;
in vec2 uv;

out vec3 vColor;
out vec2 UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

const float PI = 3.1415926535897932384626833;

void main()
{
	//vec3 unitNormal = pos / sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
	UV = uv;//vec2(asin(uv.x)/2/PI + 0.5, asin(uv.y)/2/PI + 0.5);
	vColor = color;
	gl_Position = proj * view * model * vec4(pos, 1.0);
}