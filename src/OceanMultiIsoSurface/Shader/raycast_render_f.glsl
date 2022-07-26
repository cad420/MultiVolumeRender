#version 400 core
out vec4 frag_color;

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

uniform vec4 color1;
uniform vec4 color2;
uniform float iso_value1;
uniform float iso_value2;

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
    vec3 ray_entry_pos=texture2DRect(entry_pos,ivec2(gl_FragCoord.xy)).xyz;
    vec3 ray_exit_pos =texture2DRect(exit_pos, ivec2(gl_FragCoord.xy)).xyz;
    vec3 entry2exit=ray_exit_pos-ray_entry_pos;
    vec3 ray_direction=normalize(entry2exit);
//    frag_color=vec4(ray_direction,1.f);return;
    float distance=dot(ray_direction,entry2exit);
    int steps=int(distance/step);
    if(steps==0) discard;
    vec4 color=vec4(0.f);
    vec3 sample_pos=ray_entry_pos;
    bool outside1=true,outside2=true;
    vec3 sample_pos_in_tex;
    for(int i=0;i<steps;i++){
        sample_pos_in_tex=TransformCoord(sample_pos);
        float scalar1=texture(volume_data1,sample_pos_in_tex).r;
        if(scalar1<iso_value1) outside1=true;
        float scalar2=texture(volume_data2,sample_pos_in_tex).r;
        if(scalar2<iso_value2) outside2=true;
        if(scalar1>=iso_value1 && outside1 && scalar2>=iso_value2 && outside2){
            color = color1*0.5f + color2*0.5f;
            color.a = 1.f;
            break;
        }
        else if(scalar1>=iso_value1 && outside1){
            color.rgb=color1.rgb;
            color.a= 1.f;
            break;
        }
        else if(scalar2>=iso_value2 && outside2){
            color.rgb=color2.rgb;
            color.a = 1.f;
            break;
        }
        sample_pos=ray_entry_pos+(i+1)*step*ray_direction;
    }
    if(color.a==0.0f)
        discard;
    frag_color=color;
}
