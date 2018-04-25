/*
* Team Members:
*				Vedant Chaudhari 1530277
*				Aaron Hamilton
*
* Course Code: EGP-300
* Section: 01
* Project Name: HW3
* Certificate of Authenticity :
*		We certify that this work is entirely our own.  The assessor of this 
		project may reproduce this project and provide copies to other academic staff, 
		and/or communicate a copy of this project to a plagiarism-checking service, which 
		may retain a copy of the project on its database.
*/

#version 410

layout (location = 0) in vec4 aPosition;
layout (location = 2) in vec3 aNormal;
layout (location = 8) in vec2 aTexcoord;

uniform mat4 uMVP;
uniform vec4 uLightPos_obj;
uniform vec4 uEyePos_obj;

out vbPassDataBlock
{
	vec2 vPassTexcoord;
	vec3 vPassNormal;
	vec3 vPassLight;
	vec3 vPassView;
} vPassData;

void main()
{
	gl_Position = uMVP * aPosition;

	vPassData.vPassNormal = aNormal;
	vPassData.vPassTexcoord = aTexcoord;
	vPassData.vPassLight = uLightPos_obj.xyz - aPosition.xyz;
	vPassData.vPassView = uEyePos_obj.xyz - aPosition.xyz;
}