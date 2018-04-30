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

// Simple fragment shader to pass gl_Position - Vedant

#version 410

layout (location = 0) in vec4 aPosition;

uniform mat4 uMVP;

out vec4 vPosition;

void main ()
{
	gl_Position = aPosition;

	vPosition = aPosition;
}