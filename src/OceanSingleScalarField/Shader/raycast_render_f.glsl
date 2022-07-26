#version 400 core
out vec4 frag_color;
uniform sampler1D transfer_func;
uniform sampler3D volume_data;
uniform sampler2DRect entry_pos;
uniform sampler2DRect exit_pos;

uniform float ka;
uniform float kd;
uniform float shininess;
uniform float ks;
uniform vec3 light_direction;

uniform float voxel;

uniform vec3 camera_pos;
uniform float radius;
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

vec3 phongShading(vec3 samplePos,vec3 diffuseColor,vec3 ray_direction);
void main() {

    vec3 ray_entry_pos = texture2DRect(entry_pos,gl_FragCoord.xy).xyz;
    vec3 ray_exit_pos  = texture2DRect(exit_pos, gl_FragCoord.xy).xyz;
    vec3 ray_direction=normalize(ray_exit_pos-ray_entry_pos);

    vec3 start2end=ray_exit_pos-ray_entry_pos;
    float distance=dot(ray_direction,start2end);
    int steps=int(distance/step);
    if(steps == 0) discard;

    vec4 color=vec4(0.f);
    vec3 sample_pos=ray_entry_pos;

    for(int i=0;i<steps;i++){
        vec3 sample_pos_in_tex=TransformCoord(sample_pos);
        float scalar=texture(volume_data,sample_pos_in_tex).r;
        if(scalar>0.f){
            vec4 sample_color=texture(transfer_func,scalar);
            if(sample_color.a>0.f){
//                sample_color.rgb=phongShading(sample_pos_in_tex,sample_color.rgb,ray_direction);
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
vec3 phongShading(vec3 samplePos,vec3 diffuseColor,vec3 ray_direction)
{
    vec3 N;
    N.x=(texture(volume_data,samplePos+vec3(voxel,0,0)).r-texture(volume_data,samplePos+vec3(-voxel,0,0)).r);
    N.y=(texture(volume_data,samplePos+vec3(0,voxel,0)).r-texture(volume_data,samplePos+vec3(0,-voxel,0)).r);
    N.z=(texture(volume_data,samplePos+vec3(0,0,voxel)).r-texture(volume_data,samplePos+vec3(0,0,-voxel)).r);

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