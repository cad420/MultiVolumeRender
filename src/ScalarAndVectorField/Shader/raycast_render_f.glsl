#version 430 core
out vec4 frag_color;
layout(binding=0,rgba32f) uniform image2D entry_pos;
layout(binding=1,rgba32f) uniform image2D exit_pos;
uniform sampler1D transfer_func;
uniform sampler3D volume_data;

uniform float ka;
uniform float kd;
uniform float shininess;
uniform float ks;
uniform vec3 light_direction;

uniform float step;
uniform float voxel;
uniform vec4 bg_color;
uniform vec4 line_color;
vec3 phongShading(vec3 samplePos,vec3 diffuseColor,vec3 ray_direction);
void main() {
    vec3 ray_entry_pos=imageLoad(entry_pos,ivec2(gl_FragCoord.xy)).xyz;
    vec4 ray_exit_pos_ =imageLoad(exit_pos, ivec2(gl_FragCoord.xy)).xyzw;
    vec3 ray_exit_pos=ray_exit_pos_.xyz;
    int line_flag=int(ray_exit_pos_.w);

    vec3 entry2exit=ray_exit_pos-ray_entry_pos;
    vec3 ray_direction=normalize(entry2exit);
    float distance=dot(ray_direction,entry2exit);
    int steps=int(distance/step);
    vec4 color=vec4(0.f);
    vec3 sample_pos=ray_entry_pos;
    for(int i=0;i<steps;i++){
        float scalar=texture(volume_data,sample_pos).r;
        if(scalar>0.f){
            vec4 sample_color=texture(transfer_func,scalar);
            sample_color.rgb=phongShading(sample_pos,sample_color.rgb,ray_direction);
            color=color + sample_color*vec4(sample_color.aaa,1.f)*(1.f-color.a);
            if(color.a>0.99f)
                break;
        }
        sample_pos+=ray_direction*step;
    }

    if(line_flag==1){
        color=(1-color.a)*line_color+color*color.a;
    }
    else{
        if(color.a==0.0f)
            discard;
        color=(1-color.a)*bg_color+color*color.a;
    }
    frag_color=color;
}
vec3 phongShading(vec3 samplePos,vec3 diffuseColor,vec3 ray_direction)
{
    vec3 N;
#define CUBIC
#ifdef CUBIC
    float value[27];
    float t1[9];
    float t2[3];
    for(int k=-1;k<2;k++){//z
        for(int j=-1;j<2;j++){//y
            for(int i=-1;i<2;i++){//x
                value[(k+1)*9+(j+1)*3+i+1]=texture(volume_data,samplePos+vec3(voxel*i,voxel*j,voxel*k)).r;
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
    N.x=(texture(volume_data,samplePos+vec3(step,0,0)).r-texture(volume_data,samplePos+vec3(-step,0,0)).r);
    N.y=(texture(volume_data,samplePos+vec3(0,step,0)).r-texture(volume_data,samplePos+vec3(0,-step,0)).r);
    N.z=(texture(volume_data,samplePos+vec3(0,0,step)).r-texture(volume_data,samplePos+vec3(0,0,-step)).r);
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
}