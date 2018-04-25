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
	
	drawAttribs_mrt_fs4x.glsl
	Draw all attributes to MRT as color.
*/

#version 410

// ****TO-DO: 
//	1) declare varying block to receive attribute data
//	2) declare MRT
//	3) copy inbound data to outbound render targets (might need to compress)

// out vec4 rtFragColor;

// (1)
in vbPassDataBlock
{
	vec4 vPosition;
	vec3 vNormal;
	vec2 vTexcoord;
	vec3 vTangent;
	vec3 vBitangent;
} vPassData;

// (2)
layout (location = 0) out vec4 rtPosition;
layout (location = 1) out vec4 rtNormal;
layout (location = 2) out vec4 rtTexcoord;
layout (location = 3) out vec4 rtTangent;
layout (location = 4) out vec4 rtBitangent;

void main()
{
	// DUMMY OUTPUT: all fragments are OPAQUE WHITE
	// rtFragColor = vec4(1.0, 1.0, 1.0, 1.0);

	// (3)
	rtPosition = vPassData.vPosition;
	rtNormal = vec4(vPassData.vNormal, 1.0);
	rtTexcoord = vec4(vPassData.vTexcoord, 0.0, 1.0);
	rtTangent = vec4(vPassData.vTangent, 1.0);
	rtBitangent = vec4(vPassData.vBitangent, 1.0);
}
