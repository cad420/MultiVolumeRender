#version 430 core
in vec3 world_pos;
out vec4 frag_color;
uniform bool is_line;
void main() {
    if(is_line)
        frag_color=vec4(world_pos,1.f);
    else
        frag_color=vec4(world_pos,0.f);
}
