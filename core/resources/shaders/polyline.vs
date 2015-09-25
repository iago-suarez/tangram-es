#pragma tangram: extensions

#ifdef GL_ES
precision highp float;
#endif

#pragma tangram: defines

#define TANGRAM_WORLD_POSITION_WRAP vec3(100000.0)

uniform mat4 u_modelView;
uniform mat4 u_modelViewProj;
uniform mat3 u_normalMatrix;
uniform float u_tile_zoom;
uniform float u_time;
uniform float u_zoom;

#pragma tangram: uniforms

attribute vec4 a_position;
attribute vec4 a_color;
attribute vec4 a_extrude;
attribute vec2 a_texcoord;
attribute float a_layer;

varying vec4 v_world_position;
varying vec4 v_color;
varying vec3 v_eyeToPoint;
varying vec3 v_normal;
varying vec2 v_texcoord;

#ifdef TANGRAM_LIGHTING_VERTEX
    varying vec4 v_lighting;
#endif

#pragma tangram: material
#pragma tangram: lighting
#pragma tangram: global

void main() {

    vec4 position = a_position;

    {
        float width = a_extrude.z;
        float dwdz = a_extrude.w;
        float dz =  u_zoom - abs(u_tile_zoom);
        // interpolate between zoom levels
        width += dwdz * dz;
        // scale to screen-space
        width *= exp2(-dz);

        // Modify line width before extrusion
        #pragma tangram: width

        position.xy += a_extrude.xy * width;
    }

    // Modify position before camera projection
    #pragma tangram: position

    v_color = a_color;
    v_eyeToPoint = vec3(u_modelView * a_position);
    v_normal = u_normalMatrix * vec3(0.,0.,1.);
    v_texcoord = a_texcoord;
        
    #ifdef TANGRAM_LIGHTING_VERTEX
        vec4 color = v_color;
        vec3 normal = v_normal;

        // Modify normal before lighting
        #pragma tangram: normal

        v_normal = normal;

        // Modify color and material properties before lighting
        #pragma tangram: color

        v_lighting = calculateLighting(v_eyeToPoint.xyz, normal, color);
        v_color = color;
    #endif

    gl_Position = u_modelViewProj * position;
    
    // Proxy tiles have u_tile_zoom < 0, so this re-scaling will place proxy tiles deeper in
    // the depth buffer than non-proxy tiles by a distance that increases with tile zoom
    gl_Position.z /= 1. + .1 * (abs(u_tile_zoom) - u_tile_zoom);
    
    #ifdef TANGRAM_DEPTH_DELTA
        gl_Position.z -= a_layer * TANGRAM_DEPTH_DELTA * gl_Position.w;
    #endif
}
