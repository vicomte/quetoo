/**
 * @brief Null vertex shader.
 */

#version 120

#define VERTEX_SHADER

#include "matrix_inc.glsl"
#include "fog_inc.glsl"

uniform vec4 GLOBAL_COLOR;

varying vec4 color;
varying vec2 texcoord;

attribute vec3 POSITION;
attribute vec2 TEXCOORD;
attribute vec4 COLOR;

uniform float TIME_FRACTION;

attribute vec3 NEXT_POSITION;

/**
 * @brief Shader entry point.
 */
void main(void) {

	// mvp transform into clip space
	gl_Position = PROJECTION_MAT * MODELVIEW_MAT * vec4(mix(POSITION, NEXT_POSITION, TIME_FRACTION), 1.0);

	texcoord = TEXCOORD;

	// pass the color through as well
	color = COLOR * GLOBAL_COLOR;

	FogVertex();
}
