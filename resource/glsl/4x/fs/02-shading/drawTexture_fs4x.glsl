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
	
	drawTexture_fs4x.glsl
	Sample from texture and output color.
*/

#version 410

// ****TO-DO: 
//	1) declare varying to receive texture coordinate from vertex shader
//	2) declare uniform variable for texture; see demo code for hints
//	3) grab sample from texture
//	4) copy sample color to output

out vec4 rtFragColor;

void main()
{
	// DUMMY OUTPUT: all fragments are OPAQUE WHITE
	rtFragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
