#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Color;

uniform mat4 u_Model;      // Model Matrix
uniform mat4 u_View;       // View Matrix
uniform mat4 u_Projection; // Projection Matrix

out vec2 v_TexCoord;
out vec3 v_Color;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
    v_TexCoord = a_TexCoord;
    v_Color = a_Color;
}

#shader fragment
#version 330 core
layout(location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec3 v_Color;

uniform sampler2D u_Texture;
uniform int u_UseTexture; // 0 = use vertex color, 1 = use texture

void main()
{
    if (u_UseTexture == 1) {
        color = texture(u_Texture, v_TexCoord);
    } else {
        color = vec4(v_Color, 1.0);
    }
}
