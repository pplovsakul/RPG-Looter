#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_Model;      // Model Matrix
uniform mat4 u_View;       // View Matrix
uniform mat4 u_Projection; // Projection Matrix

out vec2 v_TexCoord;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
    v_TexCoord = a_TexCoord;
}

#shader fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform sampler2D u_Texture;
uniform int u_UseTexture; // 0 = use color, 1 = use texture

void main()
{
    if (u_UseTexture == 1) {
        color = texture(u_Texture, v_TexCoord);
    } else {
        color = u_Color;
    }
}
