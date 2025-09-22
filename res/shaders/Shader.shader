#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_MVP;

out vec2 v_TexCoord;

void main()
{
    gl_Position = u_MVP * vec4(a_Position, 1.0);
    v_TexCoord = a_TexCoord;   // Texturkoordinaten weiterreichen
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform vec4 u_color;  // optional

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    color = texColor * u_color;  // falls u_Color = (1,1,1,1), bleibt Textur unverändert
}
