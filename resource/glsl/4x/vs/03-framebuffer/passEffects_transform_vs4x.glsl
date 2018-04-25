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

uniform mat4 uMVP;

layout (location = 2) in vec3 aNormal;
layout (location = 8) in vec2 aTexcoord;
layout (location = 10) in vec3 aTangent;
layout (location = 11) in vec3 aBitangent;

out vbPassDataBlock
{
	vec4 vPosition;
	vec3 vNormal;
	vec2 vTexcoord;
	vec3 vTangent;
	vec3 vBitangent;
} vPassData;

void main()
{
	gl_Position = uMVP * aPosition;

	vPassData.vPosition	 = aPosition;
	vPassData.vNormal	 = aNormal;
	vPassData.vTexcoord	 = aTexcoord;
	vPassData.vTangent	 = aTangent;
	vPassData.vBitangent = aBitangent;
}