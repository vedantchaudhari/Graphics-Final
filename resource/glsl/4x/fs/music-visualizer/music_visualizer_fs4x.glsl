/*
* Team Members:
*				Vedant Chaudhari 1530277
*				Aaron Hamilton
*
* Course Code: EGP-300
* Section: 01
* Project Name: Final Project - Music Visualizer
* Certificate of Authenticity :
*		We certify that this work is entirely our own.  The assessor of this 
		project may reproduce this project and provide copies to other academic staff, 
		and/or communicate a copy of this project to a plagiarism-checking service, which 
		may retain a copy of the project on its database.
*/

// Music Visualizer Fragment Shader - Vedant

#version 410

in vec4 vPosition;

uniform mat4 uMVP;

uniform float uTime;

uniform float spectrum_data[256];
uniform float wave_data[256];

uniform int uResX;
uniform int uResY;

out vec4 rtMusicVisualizer;

#define THICCNESS = 0.05;

void interpolate(in float index, out float value)
{
	float norm = 255.0 / uResX * index;
	int floorVal = int(floor(norm));
	int ceilVal = int(ceil(norm));
	value = mix(wave_data[floorVal], wave_data[ceilVal], fract(norm));
}

void main()
{
	float time = uTime;
	float x = vPosition.x / uResX;
	float y = vPosition.y / uResY;
	float wave = 0;

	interpolate(x * uResX, wave);

	wave = 0.5 - wave / 3;

	float r = abs(0.05 /((wave - y)));
	vec4 result = vec4(r-abs(r*0.2*sin(time/5)), r-abs(r*0.2*sin(time/7)), r-abs(r*0.2*sin(time/9)), 0);

	rtMusicVisualizer = result;
}