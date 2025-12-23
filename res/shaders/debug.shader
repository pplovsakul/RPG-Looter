#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;

uniform mat4 u_Model;      // Model Matrix
uniform mat4 u_View;       // View Matrix
uniform mat4 u_Projection; // Projection Matrix

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}

#shader fragment
#version 330 core
layout(location = 0) out vec4 color;

uniform vec3 u_Color; // Line color

void main()
{
    color = vec4(u_Color, 1.0);
}
