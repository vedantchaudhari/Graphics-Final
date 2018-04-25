/*
	Copyright 2011-2018 Daniel S. Buckstein

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	animal3D SDK: Minimal 3D Animation Framework
	By Daniel S. Buckstein
	
	passAttribs_transform_vs4x.glsl
	Transform position and pass all attributes down the pipeline.
*/

#version 410

// ****TO-DO: 
//	1) declare attributes
//	2) declare varying block to pass attribute data out
//	3) copy all attributes to the varying structure to be passed

layout (location = 0) in vec4 aPosition;

// (1)
layout (location = 2) in vec3 aNormal;
layout (location = 8) in vec2 aTexcoord;
layout (location = 10) in vec3 aTangent;
layout (location = 11) in vec3 aBitangent;

uniform mat4 uMVP;

// (2)
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

	// (3)
	vPassData.vPosition = aPosition;
	vPassData.vNormal = aNormal;
	vPassData.vTexcoord = aTexcoord;
	vPassData.vTangent = aTangent;
	vPassData.vBitangent = aBitangent;
}