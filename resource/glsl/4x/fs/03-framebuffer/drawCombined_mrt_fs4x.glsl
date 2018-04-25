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

in vbPassDataBlock
{
	vec2 vPassTexcoord;
	vec3 vPassNormal;
	vec3 vPassLight;
	vec3 vPassView;
} vPassData;

uniform sampler2D uTex_dm;
uniform sampler2D uTex_sm;

layout (location = 0) out vec4 rtTexture;	// Just the texture
layout (location = 1) out vec4 rtDiffuse;	// Just the diffuse
layout (location = 2) out vec4 rtLambert;	// Just the lambert
layout (location = 3) out vec4 rtPhong;		// Just the phong

void main()
{
	vec4 diffuseSample = texture(uTex_dm, vPassData.vPassTexcoord);
	vec4 specularSample = texture(uTex_sm, vPassData.vPassTexcoord);

	// Calculate Diffuse
	vec3 L = normalize(vPassData.vPassLight);
	vec3 N = normalize(vPassData.vPassNormal);
	float diffuse = dot(N, L);

	// Calculate Specular
	vec3 V = normalize(vPassData.vPassView);
	vec3 R = (diffuse + diffuse) * N - L;
	float specular = dot(V, R);

	diffuse = max(0.0, diffuse);
	specular = max(0.0, specular);

	specular *= specular;
	specular *= specular;
	specular *= specular;
	specular *= specular;

	// Calculate Lambert
	vec3 Lambert = diffuseSample.rgb * diffuse;
	// Calculate Phong
	vec3 Phong = Lambert + specularSample.rgb * specular + vec3(0.01, 0.0, 0.02);

	rtTexture = diffuseSample;
	rtDiffuse = vec4(diffuse, diffuse, diffuse, 1.0);
	rtLambert = vec4(Lambert, diffuseSample.a);
	rtPhong = vec4(Phong, diffuseSample.a);
}