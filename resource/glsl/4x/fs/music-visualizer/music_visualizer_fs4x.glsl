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

uniform float time;

uniform float spectrum_data[256];
uniform float wave_data[256];

out vec4 rtMusicVisualizer;

#define THICCNESS = 0.05;

void interpolate(in float index, out float value)
{

}

void main()
{
	rtMusicVisualizer = vec4(1.0, 0.0, 0.0, 1.0);
}