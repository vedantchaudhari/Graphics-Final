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
	
	passPhongComponents_transform_vs4x.glsl
	Pass variables pertinent to Phong shading model.
*/

#version 410

// ****TO-DO: 
//	1) declare attributes for normal and texture coordinates
//	2) declare uniforms for variables in code pertinent to lighting
//	3) declare varyings to pass relevant data (per-vertex vs. per-fragment)
//	4) pass data to next shader

layout (location = 0) in vec4 aPosition;

uniform mat4 uMVP;

void main()
{
	gl_Position = uMVP * aPosition;
}
