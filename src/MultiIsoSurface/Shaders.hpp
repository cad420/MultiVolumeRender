#pragma once

namespace shader{
    const char* raycast_pos_v=R"(#version 430 core
layout(location=0) in vec3 vertex_pos;
uniform mat4 MVPMatrix;
out vec3 world_pos;
void main()
{
    gl_Position=MVPMatrix*(vec4(vertex_pos,1.0));
    world_pos=vertex_pos;
})";

    const char* raycast_pos_f=R"(#version 430 core
in vec3 world_pos;
out vec4 frag_color;
void main() {
    frag_color=vec4(world_pos,1.f);
}
)";

    const char* raycast_render_v=R"(#version 430 core
layout(location=0) in vec2 vertex_pos;
void main() {
    gl_Position=vec4(vertex_pos,0.0f,1.0f);
}
)";

    const char* raycast_render_f=R"(#version 430 core
out vec4 frag_color;
layout(binding=0,rgba32f) uniform image2D entry_pos;
layout(binding=1,rgba32f) uniform image2D exit_pos;
uniform sampler3D volume_data1;
uniform sampler3D volume_data2;

uniform float ka;
uniform float kd;
uniform float shininess;
uniform float ks;
uniform vec3 light_direction;

uniform float step;
uniform float voxel;
uniform vec3 volume_board;
uniform vec3 space_ratio;
uniform vec4 bg_color;
uniform vec4 color1;
uniform vec4 color2;
uniform float iso_value1;
uniform float iso_value2;

vec3 phongShading(vec3 samplePos,vec3 diffuseColor,vec3 ray_direction,int n);
void main() {
    vec3 ray_entry_pos=imageLoad(entry_pos,ivec2(gl_FragCoord.xy)).xyz;
    vec3 ray_exit_pos =imageLoad(exit_pos, ivec2(gl_FragCoord.xy)).xyz;
    vec3 entry2exit=ray_exit_pos-ray_entry_pos;
    vec3 ray_direction=normalize(entry2exit);
    float distance=dot(ray_direction,entry2exit);
    int steps=int(distance/step);
    vec4 color=vec4(0.f);
    vec3 sample_pos=ray_entry_pos;
    bool outside1=true,outside2=true;
    vec3 sample_pos_in_tex;
    for(int i=0;i<steps;i++){
        sample_pos_in_tex=sample_pos/volume_board;
        float scalar1=texture(volume_data1,sample_pos_in_tex).r;
        if(scalar1<iso_value1) outside1=true;
        float scalar2=texture(volume_data2,sample_pos_in_tex).r;
        if(scalar2<iso_value2) outside2=true;
        if(scalar1>=iso_value1 && outside1 && scalar2>=iso_value2 && outside2){

            color.rgb = phongShading(sample_pos_in_tex,color1.rgb,ray_direction,1)*0.5
                       +phongShading(sample_pos_in_tex,color2.rgb,ray_direction,2)*0.5;
            color.a=color1.a*0.5+color2.a*0.5;
            break;
        }
        else if(scalar1>=iso_value1 && outside1){
            color.rgb=phongShading(sample_pos_in_tex,color1.rgb,ray_direction,1);
            color.a=color1.a;
            break;
        }
        else if(scalar2>=iso_value2 && outside2){
            color.rgb=phongShading(sample_pos_in_tex,color2.rgb,ray_direction,2);
            color.a=color2.a;
            break;
        }
        sample_pos+=ray_direction*step;
    }
    if(color.a==0.0f)
        discard;
    color=(1-color.a)*bg_color+color*color.a;
    frag_color=color;
}
vec3 phongShading(vec3 samplePos,vec3 diffuseColor,vec3 ray_direction,int n)
{
    vec3 N;
#define CUBIC
#ifdef CUBIC
    float value[27];
    float t1[9];
    float t2[3];
    if(n==1)
        for(int k=-1;k<2;k++){//z
            for(int j=-1;j<2;j++){//y
                for(int i=-1;i<2;i++){//x
                    value[(k+1)*9+(j+1)*3+i+1]=texture(volume_data1,
                    samplePos+vec3(voxel*i/space_ratio.x,voxel*j/space_ratio.y,voxel*k/space_ratio.z)).r;
                }
            }
        }
    else if(n==2)
        for(int k=-1;k<2;k++){//z
            for(int j=-1;j<2;j++){//y
                for(int i=-1;i<2;i++){//x
                    value[(k+1)*9+(j+1)*3+i+1]=texture(volume_data2,
                    samplePos+vec3(voxel*i/space_ratio.x,voxel*j/space_ratio.y,voxel*k/space_ratio.z)).r;
                }
            }
        }

    int x,y,z;
    //for x-direction
    for(z=0;z<3;z++){
        for(y=0;y<3;y++){
            t1[z*3+y]=(value[18+y*3+z]-value[y*3+z])/2;
        }
    }
    for(z=0;z<3;z++)
    t2[z]=(t1[z*3+0]+4*t1[z*3+1]+t1[z*3+2])/6;
    N.x=(t2[0]+t2[1]*4+t2[2])/6;


    //for y-direction
    for(z=0;z<3;z++){
        for(x=0;x<3;x++){
            t1[z*3+x]=(value[x*9+6+z]-value[x*9+z])/2;
        }
    }
    for(z=0;z<3;z++)
    t2[z]=(t1[z*3+0]+4*t1[z*3+1]+t1[z*3+2])/6;
    N.y=(t2[0]+t2[1]*4+t2[2])/6;

    //for z-direction
    for(y=0;y<3;y++){
        for(x=0;x<3;x++){
            t1[y*3+x]=(value[x*9+y*3+2]-value[x*9+y*3])/2;
        }
    }
    for(y=0;y<3;y++)
    t2[y]=(t1[y*3+0]+4*t1[y*3+1]+t1[y*3+2])/6;
    N.z=(t2[0]+t2[1]*4+t2[2])/6;
#else
    if(n==1){
        N.x=(texture(volume_data,samplePos+vec3(voxel,0,0)).r-texture(volume_data1,samplePos+vec3(-voxel,0,0)).r);
        N.y=(texture(volume_data,samplePos+vec3(0,voxel,0)).r-texture(volume_data1,samplePos+vec3(0,-voxel,0)).r);
        N.z=(texture(volume_data,samplePos+vec3(0,0,voxel)).r-texture(volume_data1,samplePos+vec3(0,0,-voxel)).r);
    }
    else if(n==2){
        N.x=(texture(volume_data,samplePos+vec3(voxel,0,0)).r-texture(volume_data2,samplePos+vec3(-voxel,0,0)).r);
        N.y=(texture(volume_data,samplePos+vec3(0,voxel,0)).r-texture(volume_data2,samplePos+vec3(0,-voxel,0)).r);
        N.z=(texture(volume_data,samplePos+vec3(0,0,voxel)).r-texture(volume_data2,samplePos+vec3(0,0,-voxel)).r);
    }
#endif

    N=-normalize(N);

    vec3 L=-ray_direction;
    vec3 R=L;//-ray_direction;

    if(dot(N,L)<0.f)
        N=-N;

    vec3 ambient=ka*diffuseColor.rgb;
    vec3 specular=ks*pow(max(dot(N,(L+R)/2.0),0.0),shininess)*vec3(1.0f);
    vec3 diffuse=kd*max(dot(N,L),0.0)*diffuseColor.rgb;
    return ambient+specular+diffuse;
})";
}
