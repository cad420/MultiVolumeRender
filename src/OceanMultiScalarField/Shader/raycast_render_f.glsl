#version 430 core
out vec4 frag_color;

uniform sampler1D transfer_func1;
uniform sampler1D transfer_func2;
uniform sampler3D volume_data1;
uniform sampler3D volume_data2;
uniform sampler2DRect entry_pos;
uniform sampler2DRect exit_pos;


uniform float step;
uniform float min_lon;
uniform float min_lat;
uniform float min_dist;
uniform float len_lon;
uniform float len_lat;
uniform float len_dist;

vec3 TransformCoord(vec3 sample_pos){
    float dist = sqrt(sample_pos.x*sample_pos.x+sample_pos.y*sample_pos.y+sample_pos.z*sample_pos.z);
    float lat  = asin(sample_pos.z/dist);
    float lon  = atan(sample_pos.y/sample_pos.x);
    if(sample_pos.x<0.f)
        lon += 3.14159265358979323846;
    lon  = (lon-min_lon)/len_lon;
    lat  = (lat-min_lat)/len_lat;
    dist = (dist-min_dist)/len_dist;
    return vec3(lon,lat,dist);
}

void main() {

    vec3 ray_entry_pos = texture2DRect(entry_pos,ivec2(gl_FragCoord.xy)).xyz;
    vec3 ray_exit_pos  = texture2DRect(exit_pos, ivec2(gl_FragCoord.xy)).xyz;
    vec3 ray_direction=normalize(ray_exit_pos-ray_entry_pos);

    vec3 start2end=ray_exit_pos-ray_entry_pos;
    float distance=dot(ray_direction,start2end);
    int steps=int(distance/step);
    if(steps == 0) discard;

    vec4 color=vec4(0.f);
    vec3 sample_pos=ray_entry_pos;

    for(int i=0;i<steps;i++){
        vec3 sample_pos_in_tex=TransformCoord(sample_pos);
        float scalar1=texture(volume_data1,sample_pos_in_tex).r;
        float scalar2=texture(volume_data2,sample_pos_in_tex).r;
        if(scalar1>0.f || scalar2>0.f){
            vec4 sample_color1=texture(transfer_func1,scalar1);
            vec4 sample_color2=texture(transfer_func2,scalar2);
            if(sample_color1.a>0.f || sample_color2.a>0.f){
//                sample_color.rgb=phongShading(sample_pos_in_tex,sample_color.rgb,ray_direction);
                vec4 sample_color = sample_color1*sample_color1.a + sample_color2*sample_color2.a;
                color=color + sample_color*vec4(sample_color.aaa,1.f)*(1.f-color.a);
            }
        }
        if(color.a>0.99f)
            break;
        sample_pos=ray_entry_pos+(i+1)*step*ray_direction;
    }
    if(color.a==0.0f)
        discard;
    frag_color=color;
}
