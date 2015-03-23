#version 150

in vec3 vColor;
in vec2 UV;

out vec4 outColor;

uniform sampler2D tex;
uniform sampler2D collideTex;
uniform bool colliding;

void main()
{
	if(colliding)
	{
		vec4 texColor = texture(collideTex, UV);
		outColor = vec4(vColor, 1.0) * texColor;
	}
	else
	{
		vec4 texColor = texture(tex, UV);
		outColor = vec4(vColor, 1.0) * texColor;
	}
}