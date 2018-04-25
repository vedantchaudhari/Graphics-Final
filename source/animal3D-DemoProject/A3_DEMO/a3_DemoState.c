// This file was modified by Vedant Chaudhari with permission from author

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

a3_DemoState.c/.cpp
Demo state function implementations.

********************************************
*** THIS IS YOUR DEMO'S MAIN SOURCE FILE ***
*** Implement your demo logic here.      ***
********************************************
*/


#include "a3_DemoState.h"


//-----------------------------------------------------------------------------

// OpenGL
#ifdef _WIN32
#include <Windows.h>
#include <GL/GL.h>
#else	// !_WIN32
#include <OpenGL/gl3.h>
#endif	// _WIN32


#include <stdio.h>
#include <stdlib.h>


//-----------------------------------------------------------------------------
// SETUP AND TERMINATION UTILITIES

// set default GL state
void a3demo_setDefaultGraphicsState()
{
	const float lineWidth = 2.0f;
	const float pointSize = 4.0f;

	// lines and points
	glLineWidth(lineWidth);
	glPointSize(pointSize);

	// backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// alpha blending
	// result = ( new*[new alpha] ) + ( old*[1 - new alpha] )
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// textures
	glEnable(GL_TEXTURE_2D);

	// background
	glClearColor(0.0f, 0.0f, 0.0, 0.0f);
}


//-----------------------------------------------------------------------------
// LOADING AND UNLOADING

// utility to load framebuffers
void a3demo_loadFramebuffers(a3_DemoState *demoState)
{
	a3_Framebuffer *fbo;

	fbo = demoState->fbo_scene;
	a3framebufferCreate(fbo, 8, a3fbo_colorRGBA8, a3fbo_depth24, demoState->frameWidth, demoState->frameHeight);
}

// utility to load textures
void a3demo_loadTextures(a3_DemoState *demoState)
{
	// pointer to texture
	a3_Texture *tex;
	unsigned int i;

	// list of texture files to load
	const char *texFiles[] = {
		"../../../../resource/tex/sprite/checker.png",
		"../../../../resource/tex/bg/sky_clouds.png",
		"../../../../resource/tex/bg/sky_water.png",
		"../../../../resource/tex/stone/stone_dm.png",
		"../../../../resource/tex/earth/2k/earth_dm_2k.png",
		"../../../../resource/tex/earth/2k/earth_sm_2k.png",
	};
	const unsigned int numTextures = sizeof(texFiles) / sizeof(const char *);

	for (i = 0; i < numTextures; ++i)
	{
		tex = demoState->texture + i;
		a3textureCreateFromFile(tex, texFiles[i]);
	}

	// change settings on a per-texture basis
	a3textureActivate(demoState->tex_checker, a3tex_unit00);
	a3textureDefaultSettings();	// nearest filtering, repeat on both axes

	a3textureActivate(demoState->tex_sky_clouds, a3tex_unit00);
	a3textureChangeRepeatMode(a3tex_repeatNormal, a3tex_repeatNormal);	// repeat both axes
	a3textureChangeFilterMode(a3tex_filterLinear);	// linear pixel blending

	a3textureActivate(demoState->tex_sky_water, a3tex_unit00);
	a3textureChangeRepeatMode(a3tex_repeatNormal, a3tex_repeatNormal);
	a3textureChangeFilterMode(a3tex_filterLinear);

	a3textureActivate(demoState->tex_stone_dm, a3tex_unit00);
	a3textureChangeRepeatMode(a3tex_repeatNormal, a3tex_repeatNormal);
	a3textureChangeFilterMode(a3tex_filterLinear);

	a3textureActivate(demoState->tex_earth_dm, a3tex_unit00);
	a3textureChangeRepeatMode(a3tex_repeatNormal, a3tex_repeatClamp);	// repeat on U, clamp V
	a3textureChangeFilterMode(a3tex_filterLinear);	// linear pixel blending

	a3textureActivate(demoState->tex_earth_sm, a3tex_unit00);
	a3textureChangeRepeatMode(a3tex_repeatNormal, a3tex_repeatClamp);
	a3textureChangeFilterMode(a3tex_filterLinear);


	// done
	a3textureDeactivate(a3tex_unit00);
}

// utility to load geometry
void a3demo_loadGeometry(a3_DemoState *demoState)
{
	// static model transformations
	static const a3mat4 downscale20x = {
		+0.05f, 0.0f, 0.0f, 0.0f,
		0.0f, +0.05f, 0.0f, 0.0f,
		0.0f, 0.0f, +0.05f, 0.0f,
		0.0f, 0.0f, 0.0f, +1.0f,
	};

	// pointer to shared vbo/ibo
	a3_VertexBuffer *vbo_ibo;
	a3_VertexArrayDescriptor *vao;
	a3_VertexDrawable *currentDrawable;
	unsigned int sharedVertexStorage = 0, sharedIndexStorage = 0;
	unsigned int numVerts = 0;
	unsigned int i;


	// file streaming (if requested)
	a3_FileStream fileStream[1] = { 0 };
	const char *const geometryStream = "./data/geom_data.dat";

	// geometry data
	a3_GeometryData sceneShapesData[4] = { 0 };
	a3_GeometryData proceduralShapesData[4] = { 0 };
	a3_GeometryData loadedModelsData[1] = { 0 };
	const unsigned int sceneShapesCount = sizeof(sceneShapesData) / sizeof(a3_GeometryData);
	const unsigned int proceduralShapesCount = sizeof(proceduralShapesData) / sizeof(a3_GeometryData);
	const unsigned int loadedModelsCount = sizeof(loadedModelsData) / sizeof(a3_GeometryData);

	// common index format
	a3_IndexFormatDescriptor sceneCommonIndexFormat[1] = { 0 };


	// procedural scene objects
	// attempt to load stream if requested
	if (demoState->streaming && a3fileStreamOpenRead(fileStream, geometryStream))
	{
		// read from stream

		// static scene objects
		for (i = 0; i < sceneShapesCount; ++i)
			a3fileStreamReadObject(fileStream, sceneShapesData + i, (a3_FileStreamReadFunc)a3geometryLoadDataBinary);

		// procedurally-generated objects
		for (i = 0; i < proceduralShapesCount; ++i)
			a3fileStreamReadObject(fileStream, proceduralShapesData + i, (a3_FileStreamReadFunc)a3geometryLoadDataBinary);

		// loaded model objects
		for (i = 0; i < loadedModelsCount; ++i)
			a3fileStreamReadObject(fileStream, loadedModelsData + i, (a3_FileStreamReadFunc)a3geometryLoadDataBinary);

		// done
		a3fileStreamClose(fileStream);
	}
	// not streaming or stream doesn't exist
	else if (!demoState->streaming || a3fileStreamOpenWrite(fileStream, geometryStream))
	{
		// create new data
		a3_ProceduralGeometryDescriptor sceneShapes[4] = { a3geomShape_none };
		a3_ProceduralGeometryDescriptor proceduralShapes[4] = { a3geomShape_none };
		a3_ProceduralGeometryDescriptor loadedModelShapes[1] = { a3geomShape_none };

		// static scene procedural objects
		//	(axes, grid, fsq, skybox)
		a3proceduralCreateDescriptorAxes(sceneShapes + 0, a3geomFlag_wireframe, 0.0f, 1);
		a3proceduralCreateDescriptorPlane(sceneShapes + 1, a3geomFlag_wireframe, a3geomAxis_default, 20.0f, 20.0f, 20, 20);
		a3proceduralCreateDescriptorPlane(sceneShapes + 2, a3geomFlag_texcoords, a3geomAxis_default, 2.0f, 2.0f, 1, 1);
		a3proceduralCreateDescriptorBox(sceneShapes + 3, a3geomFlag_texcoords, 100.0f, 100.0f, 100.0f, 1, 1, 1);
		for (i = 0; i < sceneShapesCount; ++i)
		{
			a3proceduralGenerateGeometryData(sceneShapesData + i, sceneShapes + i);
			a3fileStreamWriteObject(fileStream, sceneShapesData + i, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);
		}

		// other procedurally-generated objects
		a3proceduralCreateDescriptorPlane(proceduralShapes + 0, a3geomFlag_tangents, a3geomAxis_default, 40.0f, 40.0f, 40, 40);
		a3proceduralCreateDescriptorSphere(proceduralShapes + 1, a3geomFlag_tangents, a3geomAxis_default, 2.0f, 32, 24);
		a3proceduralCreateDescriptorCylinder(proceduralShapes + 2, a3geomFlag_tangents, a3geomAxis_default, 1.0f, 4.0f, 32, 1, 1);
		a3proceduralCreateDescriptorTorus(proceduralShapes + 3, a3geomFlag_tangents, a3geomAxis_default, 2.0f, 0.5f, 32, 24);
		for (i = 0; i < proceduralShapesCount; ++i)
		{
			a3proceduralGenerateGeometryData(proceduralShapesData + i, proceduralShapes + i);
			a3fileStreamWriteObject(fileStream, proceduralShapesData + i, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);
		}

		// objects loaded from mesh files
		a3modelLoadOBJ(loadedModelsData + 0, "../../../../resource/obj/teapot/teapot.obj", a3model_calculateVertexTangents, downscale20x.mm);
		for (i = 0; i < loadedModelsCount; ++i)
			a3fileStreamWriteObject(fileStream, loadedModelsData + i, (a3_FileStreamWriteFunc)a3geometrySaveDataBinary);

		// done
		a3fileStreamClose(fileStream);
	}


	// GPU data upload process: 
	//	- determine storage requirements
	//	- allocate buffer
	//	- create vertex arrays using unique formats
	//	- create drawable and upload data

	// get storage size
	sharedVertexStorage = numVerts = 0;
	for (i = 0; i < sceneShapesCount; ++i)
	{
		sharedVertexStorage += a3geometryGetVertexBufferSize(sceneShapesData + i);
		numVerts += sceneShapesData[i].numVertices;
	}
	for (i = 0; i < proceduralShapesCount; ++i)
	{
		sharedVertexStorage += a3geometryGetVertexBufferSize(proceduralShapesData + i);
		numVerts += proceduralShapesData[i].numVertices;
	}
	for (i = 0; i < loadedModelsCount; ++i)
	{
		sharedVertexStorage += a3geometryGetVertexBufferSize(loadedModelsData + i);
		numVerts += loadedModelsData[i].numVertices;
	}

	// common index format required for shapes that share vertex formats
	a3geometryCreateIndexFormat(sceneCommonIndexFormat, numVerts);
	sharedIndexStorage = 0;
	for (i = 0; i < sceneShapesCount; ++i)
		sharedIndexStorage += a3indexStorageSpaceRequired(sceneCommonIndexFormat, sceneShapesData[i].numIndices);
	for (i = 0; i < proceduralShapesCount; ++i)
		sharedIndexStorage += a3indexStorageSpaceRequired(sceneCommonIndexFormat, proceduralShapesData[i].numIndices);
	for (i = 0; i < loadedModelsCount; ++i)
		sharedIndexStorage += a3indexStorageSpaceRequired(sceneCommonIndexFormat, loadedModelsData[i].numIndices);


	// create shared buffer
	vbo_ibo = demoState->vbo_staticSceneObjectDrawBuffer;
	a3bufferCreateSplit(vbo_ibo, a3buffer_vertex, sharedVertexStorage, sharedIndexStorage, 0, 0);
	sharedVertexStorage = 0;


	// create vertex formats and drawables
	// axes
	vao = demoState->vao_position_color;
	a3geometryGenerateVertexArray(vao, sceneShapesData + 0, vbo_ibo, sharedVertexStorage);
	currentDrawable = demoState->draw_axes;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, sceneShapesData + 0, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);

	// grid: position attribute only
	// overlay objects are also just position
	vao = demoState->vao_position;
	a3geometryGenerateVertexArray(vao, sceneShapesData + 1, vbo_ibo, sharedVertexStorage);
	currentDrawable = demoState->draw_grid;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, sceneShapesData + 1, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);

	// fsq and skybox: position and texture coordinates
	vao = demoState->vao_position_texcoord;
	a3geometryGenerateVertexArray(vao, sceneShapesData + 2, vbo_ibo, sharedVertexStorage);
	currentDrawable = demoState->draw_fsq;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, sceneShapesData + 2, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_skybox;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, sceneShapesData + 3, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);

	// scene objects: full tangent basis
	vao = demoState->vao_tangent_basis;
	a3geometryGenerateVertexArray(vao, proceduralShapesData + 0, vbo_ibo, sharedVertexStorage);
	currentDrawable = demoState->draw_groundPlane;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, proceduralShapesData + 0, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_sphere;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, proceduralShapesData + 1, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_cylinder;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, proceduralShapesData + 2, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_torus;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, proceduralShapesData + 3, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);
	currentDrawable = demoState->draw_teapot;
	sharedVertexStorage += a3geometryGenerateDrawable(currentDrawable, loadedModelsData + 0, vao, vbo_ibo, sceneCommonIndexFormat, 0, 0);

	// release data when done
	for (i = 0; i < sceneShapesCount; ++i)
		a3geometryReleaseData(sceneShapesData + i);
	for (i = 0; i < proceduralShapesCount; ++i)
		a3geometryReleaseData(proceduralShapesData + i);
	for (i = 0; i < loadedModelsCount; ++i)
		a3geometryReleaseData(loadedModelsData + i);
}


// utility to load shaders
void a3demo_loadShaders(a3_DemoState *demoState)
{
	// direct to demo programs
	a3_DemoStateShaderProgram *currentDemoProg;
	int *currentUnif, uLocation;
	unsigned int i, j;

	// list of uniform names: align with uniform list in demo struct!
	const char *uniformNames[demoStateMaxCount_shaderProgramUniform] = {
		// common vertex
		"uMVP",
		"uLightPos_obj",
		"uEyePos_obj",

		// common fragment
		"uTex_dm",
		"uTex_sm",
		"uColor",
	};


	// some default uniform values
	const float defaultColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const int defaultTexUnits[] = { 0, 1, 2, 3, 4, 5, 6, 7 };


	// list of all unique shaders
	// this is a good idea to avoid multi-loading 
	//	those that are shared between programs
	union {
		struct {
			// vertex shaders
			// 03 HW
			a3_Shader passEffects_transform_vs[1];
			a3_Shader passCombined_transform_vs[1];
			// 03
			a3_Shader passAttribs_transform_vs[1];
			// 02
			a3_Shader passPhongComponents_transform_vs[1];
			a3_Shader passLambertComponents_transform_vs[1];
			a3_Shader passDiffuseComponents_transform_vs[1];
			a3_Shader passTexcoord_transform_vs[1];
			// base
			a3_Shader passthru_transform_vs[1];
			a3_Shader passColor_transform_vs[1];

			// fragment shaders
			// 03 HW
			a3_Shader drawEffects_mrt_fs[1];
			a3_Shader drawCombined_mrt_fs[1];
			// 03
			a3_Shader drawAttribs_mrt_fs[1];
			// 02
			a3_Shader drawPhong_fs[1];
			a3_Shader drawLambert_fs[1];
			a3_Shader drawDiffuse_fs[1];
			a3_Shader drawTexture_fs[1];
			// base
			a3_Shader drawColorUnif_fs[1];
			a3_Shader drawColorAttrib_fs[1];
		};
	} shaderList = { 0 };
	a3_Shader *const shaderListPtr = (a3_Shader *)(&shaderList);
	const unsigned int numUniqueShaders = sizeof(shaderList) / sizeof(a3_Shader);

	// descriptors to help load shaders; aligned with above list
	struct {
		a3_ShaderType shaderType;
		unsigned int srcCount;
		const char *filePath[8];	// max number of source files per shader
	} shaderDescriptor[] = {
		// ****REMINDER: 'Encoded' shaders are available for proof-of-concept
		//	testing ONLY!  Insert /e before file names.
		// DO NOT SUBMIT WORK USING ENCODED SHADERS OR YOU WILL GET ZERO!!!


	// CHANGE THE /e/S
	// vs
	// 03
	{ a3shader_vertex,		1,{ "../../../../resource/glsl/4x/vs/03-framebuffer/passEffects_transform_vs4x.glsl" } },
	{ a3shader_vertex,		1,{ "../../../../resource/glsl/4x/vs/03-framebuffer/passCombined_transform_vs4x.glsl" } },
	{ a3shader_vertex,		1,{ "../../../../resource/glsl/4x/vs/03-framebuffer/passAttribs_transform_vs4x.glsl" } },
		// 02
	{ a3shader_vertex,		1,{ "../../../../resource/glsl/4x/vs/02-shading/e/passPhongComponents_transform_vs4x.glsl" } },
	{ a3shader_vertex,		1,{ "../../../../resource/glsl/4x/vs/02-shading/e/passLambertComponents_transform_vs4x.glsl" } },
	{ a3shader_vertex,		1,{ "../../../../resource/glsl/4x/vs/02-shading/e/passDiffuseComponents_transform_vs4x.glsl" } },
	{ a3shader_vertex,		1,{ "../../../../resource/glsl/4x/vs/02-shading/e/passTexcoord_transform_vs4x.glsl" } },
	// base
	{ a3shader_vertex,		1,{ "../../../../resource/glsl/4x/vs/e/passthru_transform_vs4x.glsl" } },
	{ a3shader_vertex,		1,{ "../../../../resource/glsl/4x/vs/e/passColor_transform_vs4x.glsl" } },

	// fs
	// 03
	{ a3shader_fragment,	1,{ "../../../../resource/glsl/4x/fs/03-framebuffer/drawEffects_mrt_fs4x.glsl" } },
	{ a3shader_fragment,	1,{ "../../../../resource/glsl/4x/fs/03-framebuffer/drawCombined_mrt_fs4x.glsl" } },
	{ a3shader_fragment,	1,{ "../../../../resource/glsl/4x/fs/03-framebuffer/drawAttribs_mrt_fs4x.glsl" } },
	// 02
	{ a3shader_fragment,	1,{ "../../../../resource/glsl/4x/fs/02-shading/e/drawPhong_fs4x.glsl" } },
	{ a3shader_fragment,	1,{ "../../../../resource/glsl/4x/fs/02-shading/e/drawLambert_fs4x.glsl" } },
	{ a3shader_fragment,	1,{ "../../../../resource/glsl/4x/fs/02-shading/e/drawDiffuse_fs4x.glsl" } },
	{ a3shader_fragment,	1,{ "../../../../resource/glsl/4x/fs/02-shading/e/drawTexture_fs4x.glsl" } },
	// base
	{ a3shader_fragment,	1,{ "../../../../resource/glsl/4x/fs/e/drawColorUnif_fs4x.glsl" } },
	{ a3shader_fragment,	1,{ "../../../../resource/glsl/4x/fs/e/drawColorAttrib_fs4x.glsl" } },
	};

	// load unique shaders: 
	//	- load file contents
	//	- create and compile shader object
	//	- release file contents
	for (i = 0; i < numUniqueShaders; ++i)
	{
		a3shaderCreateFromFileList(shaderListPtr + i, shaderDescriptor[i].shaderType,
			shaderDescriptor[i].filePath, shaderDescriptor[i].srcCount);
	}


	// setup programs: 
	//	- create program object
	//	- attach shader objects

	// 03 programs
	// Effects HSL, HSV, etc.
	currentDemoProg = demoState->prog_drawEffectsMRT;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passEffects_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawEffects_mrt_fs);

	// Draw combined Phong, Lambert, Diffuse, Texture
	currentDemoProg = demoState->prog_drawCombinedMRT;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passCombined_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawCombined_mrt_fs);

	// draw attribs MRT
	currentDemoProg = demoState->prog_drawAttribsMRT;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passAttribs_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawAttribs_mrt_fs);


	// 02 programs
	// Phong shading
	currentDemoProg = demoState->prog_drawPhong;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passPhongComponents_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawPhong_fs);

	// Lambert shading
	currentDemoProg = demoState->prog_drawLambert;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passLambertComponents_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawLambert_fs);

	// diffuse shading
	currentDemoProg = demoState->prog_drawDiffuse;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passDiffuseComponents_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawDiffuse_fs);

	// texturing
	currentDemoProg = demoState->prog_drawTexture;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passTexcoord_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawTexture_fs);


	// base programs
	// color attrib program
	currentDemoProg = demoState->prog_drawColor;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passColor_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawColorAttrib_fs);

	// uniform color program
	currentDemoProg = demoState->prog_drawColorUnif;
	a3shaderProgramCreate(currentDemoProg->program);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.passthru_transform_vs);
	a3shaderProgramAttachShader(currentDemoProg->program, shaderList.drawColorUnif_fs);


	// activate a primitive for validation
	// makes sure the specified geometry can draw using programs
	// good idea to activate the drawable with the most attributes
	a3vertexActivateDrawable(demoState->draw_axes);

	// link and validate all programs
	for (i = 0; i < demoStateMaxCount_shaderProgram; ++i)
	{
		currentDemoProg = demoState->shaderProgram + i;
		a3shaderProgramLink(currentDemoProg->program);
		a3shaderProgramValidate(currentDemoProg->program);
	}

	// if linking fails, contingency plan goes here
	// otherwise, release shaders
	for (i = 0; i < numUniqueShaders; ++i)
		a3shaderRelease(shaderListPtr + i);


	// prepare uniforms algorithmically instead of manually for all programs
	for (i = 0; i < demoStateMaxCount_shaderProgram; ++i)
	{
		// activate program
		currentDemoProg = demoState->shaderProgram + i;
		a3shaderProgramActivate(currentDemoProg->program);

		// get uniform locations
		currentUnif = currentDemoProg->uniformLocation;
		for (j = 0; j < demoStateMaxCount_shaderProgramUniform; ++j)
			currentUnif[j] = a3shaderUniformGetLocation(currentDemoProg->program, uniformNames[j]);

		// set default values for all programs that have a uniform that will 
		//	either never change or is consistent for all programs
		if ((uLocation = currentDemoProg->uMVP) >= 0)
			a3shaderUniformSendFloatMat(a3unif_mat4, 0, uLocation, 1, a3identityMat4.mm);
		if ((uLocation = currentDemoProg->uLightPos_obj) >= 0)
			a3shaderUniformSendFloat(a3unif_vec4, uLocation, 1, a3wVec4.v);
		if ((uLocation = currentDemoProg->uEyePos_obj) >= 0)
			a3shaderUniformSendFloat(a3unif_vec4, uLocation, 1, a3wVec4.v);
		if ((uLocation = currentDemoProg->uTex_dm) >= 0)
			a3shaderUniformSendInt(a3unif_single, uLocation, 1, defaultTexUnits + 0);
		if ((uLocation = currentDemoProg->uTex_sm) >= 0)
			a3shaderUniformSendInt(a3unif_single, uLocation, 1, defaultTexUnits + 1);
		if ((uLocation = currentDemoProg->uColor) >= 0)
			a3shaderUniformSendFloat(a3unif_vec4, uLocation, 1, defaultColor);
	}

	//done
	a3shaderProgramDeactivate();
	a3vertexDeactivateDrawable();
}


//-----------------------------------------------------------------------------
// release objects
// this is where the union array style comes in handy; don't need a single 
//	release statement for each and every object... just iterate and release!

// utility to unload framebuffers
void a3demo_unloadFramebuffers(a3_DemoState *demoState)
{
	a3_Framebuffer *currentFBO = demoState->framebuffer,
		*const endFBO = currentFBO + demoStateMaxCount_framebuffer;

	while (currentFBO < endFBO)
		a3framebufferRelease(currentFBO++);
}

// utility to unload textures
void a3demo_unloadTextures(a3_DemoState *demoState)
{
	a3_Texture *currentTex = demoState->texture,
		*const endTex = currentTex + demoStateMaxCount_texture;

	while (currentTex < endTex)
		a3textureRelease(currentTex++);
}

// utility to unload geometry
void a3demo_unloadGeometry(a3_DemoState *demoState)
{
	a3_BufferObject *currentBuff = demoState->drawDataBuffer,
		*const endBuff = currentBuff + demoStateMaxCount_drawDataBuffer;
	a3_VertexArrayDescriptor *currentVAO = demoState->vertexArray,
		*const endVAO = currentVAO + demoStateMaxCount_vertexArray;
	a3_VertexDrawable *currentDraw = demoState->drawable,
		*const endDraw = currentDraw + demoStateMaxCount_drawable;

	while (currentBuff < endBuff)
		a3bufferRelease(currentBuff++);
	while (currentVAO < endVAO)
		a3vertexArrayReleaseDescriptor(currentVAO++);
	while (currentDraw < endDraw)
		a3vertexReleaseDrawable(currentDraw++);
}


// utility to unload shaders
void a3demo_unloadShaders(a3_DemoState *demoState)
{
	a3_DemoStateShaderProgram *currentProg = demoState->shaderProgram,
		*const endProg = currentProg + demoStateMaxCount_shaderProgram;

	while (currentProg < endProg)
		a3shaderProgramRelease((currentProg++)->program);
}


//-----------------------------------------------------------------------------

// initialize non-asset objects
void a3demo_initScene(a3_DemoState *demoState)
{
	unsigned int i;
	const float cameraAxisPos = 15.0f;

	// all objects
	for (i = 0; i < demoStateMaxCount_sceneObject; ++i)
		a3demo_initSceneObject(demoState->sceneObject + i);

	a3demo_setCameraSceneObject(demoState->sceneCamera, demoState->cameraObject);
	for (i = 0; i < demoStateMaxCount_camera; ++i)
		a3demo_initCamera(demoState->sceneCamera + i);

	// cameras
	demoState->sceneCamera->znear = 1.00f;
	demoState->sceneCamera->zfar = 100.0f;
	demoState->sceneCamera->ctrlMoveSpeed = 10.0f;
	demoState->sceneCamera->ctrlRotateSpeed = 5.0f;
	demoState->sceneCamera->ctrlZoomSpeed = 5.0f;


	// camera's starting orientation depends on "vertical" axis
	// we want the exact same view in either case
	if (demoState->verticalAxis)
	{
		// vertical axis is Y
		demoState->sceneCamera->sceneObject->position.x = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->position.y = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->position.z = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->euler.x = -30.0f;
		demoState->sceneCamera->sceneObject->euler.y = 45.0f;
		demoState->sceneCamera->sceneObject->euler.z = 0.0f;
	}
	else
	{
		// vertical axis is Z
		demoState->sceneCamera->sceneObject->position.x = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->position.y = -cameraAxisPos;
		demoState->sceneCamera->sceneObject->position.z = +cameraAxisPos;
		demoState->sceneCamera->sceneObject->euler.x = 60.0f;
		demoState->sceneCamera->sceneObject->euler.y = 0.0f;
		demoState->sceneCamera->sceneObject->euler.z = 45.0f;
	}

	// same fovy to start
	demoState->sceneCamera->fovy = a3realSixty;

	// demo modes
	demoState->demoMode = 0;
	demoState->demoModeCount = 4;

	demoState->displayDepth = 0;
	demoState->displayGrid = 1;
	demoState->displayAxes = 1;


	// initialize other objects 
	// e.g. light
	a3real4Set(demoState->lightPos_world.v, 20.0f, 0.0f, 0.0f, 1.0f);
}


//-----------------------------------------------------------------------------

// the handle release callbacks are no longer valid; since the library was 
//	reloaded, old function pointers are out of scope!
// could reload everything, but that would mean rebuilding GPU data...
//	...or just set new function pointers!
void a3demo_refresh(a3_DemoState *demoState)
{
	a3_Framebuffer *currentFBO = demoState->framebuffer,
		*const endFBO = currentFBO + demoStateMaxCount_framebuffer;
	a3_Texture *currentTex = demoState->texture,
		*const endTex = currentTex + demoStateMaxCount_texture;
	a3_BufferObject *currentBuff = demoState->drawDataBuffer,
		*const endBuff = currentBuff + demoStateMaxCount_drawDataBuffer;
	a3_VertexArrayDescriptor *currentVAO = demoState->vertexArray,
		*const endVAO = currentVAO + demoStateMaxCount_vertexArray;
	a3_DemoStateShaderProgram *currentProg = demoState->shaderProgram,
		*const endProg = currentProg + demoStateMaxCount_shaderProgram;

	while (currentFBO < endFBO)
		a3framebufferHandleUpdateReleaseCallback(currentFBO++);
	while (currentTex < endTex)
		a3textureHandleUpdateReleaseCallback(currentTex++);
	while (currentBuff < endBuff)
		a3bufferHandleUpdateReleaseCallback(currentBuff++);
	while (currentVAO < endVAO)
		a3vertexArrayHandleUpdateReleaseCallback(currentVAO++);
	while (currentProg < endProg)
		a3shaderProgramHandleUpdateReleaseCallback((currentProg++)->program);
}


// confirm that all graphics objects were unloaded
void a3demo_validateUnload(const a3_DemoState *demoState)
{
	unsigned int handle;
	const a3_Framebuffer *currentFBO = demoState->framebuffer,
		*const endFBO = currentFBO + demoStateMaxCount_framebuffer;
	const a3_Texture *currentTex = demoState->texture,
		*const endTex = currentTex + demoStateMaxCount_texture;
	const a3_BufferObject *currentBuff = demoState->drawDataBuffer,
		*const endBuff = currentBuff + demoStateMaxCount_drawDataBuffer;
	const a3_VertexArrayDescriptor *currentVAO = demoState->vertexArray,
		*const endVAO = currentVAO + demoStateMaxCount_vertexArray;
	const a3_DemoStateShaderProgram *currentProg = demoState->shaderProgram,
		*const endProg = currentProg + demoStateMaxCount_shaderProgram;

	handle = 0;
	currentFBO = demoState->framebuffer;
	while (currentFBO < endFBO)
		handle += (currentFBO++)->handle->handle;
	if (handle)
		printf("\n A3 Warning: One or more framebuffers not released.");

	handle = 0;
	currentTex = demoState->texture;
	while (currentTex < endTex)
		handle += (currentTex++)->handle->handle;
	if (handle)
		printf("\n A3 Warning: One or more textures not released.");

	handle = 0;
	currentBuff = demoState->drawDataBuffer;
	while (currentBuff < endBuff)
		handle += (currentBuff++)->handle->handle;
	if (handle)
		printf("\n A3 Warning: One or more draw data buffers not released.");

	handle = 0;
	currentVAO = demoState->vertexArray;
	while (currentVAO < endVAO)
		handle += (currentVAO++)->handle->handle;
	if (handle)
		printf("\n A3 Warning: One or more vertex arrays not released.");

	handle = 0;
	currentProg = demoState->shaderProgram;
	while (currentProg < endProg)
		handle += (currentProg++)->program->handle->handle;
	if (handle)
		printf("\n A3 Warning: One or more shader programs not released.");
}


//-----------------------------------------------------------------------------
// MAIN LOOP

void a3demo_input(a3_DemoState *demoState, double dt)
{
	a3real ctrlRotateSpeed = 1.0f;
	a3real azimuth = 0.0f;
	a3real elevation = 0.0f;
	int rotatingCamera = 0, movingCamera = 0, changingParam = 0;

	// using Xbox controller
	if (a3XboxControlIsConnected(demoState->xcontrol))
	{
		// move and rotate camera using joysticks
		double lJoystick[2], rJoystick[2], lTrigger[1], rTrigger[1];
		a3XboxControlGetJoysticks(demoState->xcontrol, lJoystick, rJoystick);
		a3XboxControlGetTriggers(demoState->xcontrol, lTrigger, rTrigger);

		movingCamera = a3demo_moveSceneObject(demoState->camera->sceneObject, (float)dt * demoState->camera->ctrlMoveSpeed,
			(a3real)(rJoystick[0]),
			(a3real)(*rTrigger - *lTrigger),
			(a3real)(-rJoystick[1])
		);
		// rotate
		{
			ctrlRotateSpeed = 10.0f;
			azimuth = (a3real)(-lJoystick[0]);
			elevation = (a3real)(lJoystick[1]);

			// this really defines which way is "up"
			// mouse's Y motion controls pitch, but X can control yaw or roll
			// controlling yaw makes Y axis seem "up", roll makes Z seem "up"
			rotatingCamera = a3demo_rotateSceneObject(demoState->camera->sceneObject,
				ctrlRotateSpeed * (float)dt * demoState->camera->ctrlRotateSpeed,
				// pitch: vertical tilt
				elevation,
				// yaw/roll depends on "vertical" axis: if y, yaw; if z, roll
				demoState->verticalAxis ? azimuth : a3realZero,
				demoState->verticalAxis ? a3realZero : azimuth);
		}
	}

	// using mouse and keyboard
	else
	{
		// Check if switching shader programs

		if (a3keyboardIsChanged(demoState->keyboard, a3key_L) > 0)
		{
			demoState->programType = !demoState->programType;
		}

		// move using WASDEQ
		movingCamera = a3demo_moveSceneObject(demoState->camera->sceneObject, (float)dt * demoState->camera->ctrlMoveSpeed,
			(a3real)a3keyboardGetDifference(demoState->keyboard, a3key_D, a3key_A),
			(a3real)a3keyboardGetDifference(demoState->keyboard, a3key_E, a3key_Q),
			(a3real)a3keyboardGetDifference(demoState->keyboard, a3key_S, a3key_W)
		);
		if (a3mouseIsHeld(demoState->mouse, a3mouse_left))
		{
			azimuth = -(a3real)a3mouseGetDeltaX(demoState->mouse);
			elevation = -(a3real)a3mouseGetDeltaY(demoState->mouse);

			// this really defines which way is "up"
			// mouse's Y motion controls pitch, but X can control yaw or roll
			// controlling yaw makes Y axis seem "up", roll makes Z seem "up"
			rotatingCamera = a3demo_rotateSceneObject(demoState->camera->sceneObject,
				ctrlRotateSpeed * (float)dt * demoState->camera->ctrlRotateSpeed,
				// pitch: vertical tilt
				elevation,
				// yaw/roll depends on "vertical" axis: if y, yaw; if z, roll
				demoState->verticalAxis ? azimuth : a3realZero,
				demoState->verticalAxis ? a3realZero : azimuth);
		}
	}
}

void a3demo_update(a3_DemoState *demoState, double dt)
{
	unsigned int i;

	const float dr = (float)dt * 15.0f;

	// rotate and position objects
	if (demoState->verticalAxis)
	{
		// y-up
		demoState->sphereObject->euler.y = a3trigValid_sind(demoState->sphereObject->euler.y + dr);
		demoState->cylinderObject->euler.y = a3trigValid_sind(demoState->cylinderObject->euler.y + dr);
		demoState->torusObject->euler.y = a3trigValid_sind(demoState->torusObject->euler.y + dr);
		demoState->teapotObject->euler.y = a3trigValid_sind(demoState->teapotObject->euler.y + dr);

		demoState->sphereObject->position.x = +5.0f;
		demoState->cylinderObject->position.z = -5.0f;
		demoState->torusObject->position.x = -5.0f;
		demoState->teapotObject->position.z = +5.0f;

		demoState->groundObject->position.y = -4.0f;
		demoState->groundObject->euler.x = -90.0f;
	}
	else
	{
		// z-up
		demoState->sphereObject->euler.z = a3trigValid_sind(demoState->sphereObject->euler.z + dr);
		demoState->cylinderObject->euler.z = a3trigValid_sind(demoState->cylinderObject->euler.z + dr);
		demoState->torusObject->euler.z = a3trigValid_sind(demoState->torusObject->euler.z + dr);
		demoState->teapotObject->euler.z = a3trigValid_sind(demoState->teapotObject->euler.z + dr);

		demoState->sphereObject->position.x = +5.0f;
		demoState->cylinderObject->position.y = +5.0f;
		demoState->torusObject->position.x = -5.0f;
		demoState->teapotObject->position.y = -5.0f;

		demoState->groundObject->position.z = -4.0f;
	}


	// update scene objects
	for (i = 0; i < demoStateMaxCount_sceneObject; ++i)
		a3demo_updateSceneObject(demoState->sceneObject + i);

	// update cameras
	for (i = 0; i < demoStateMaxCount_camera; ++i)
		a3demo_updateCameraViewProjection(demoState->camera + i);
}

void a3demo_render(const a3_DemoState *demoState)
{
	const a3_VertexDrawable *currentDrawable;
	const a3_DemoStateShaderProgram *currentDemoProgram;

	const int useVerticalY = demoState->verticalAxis;


	// grid lines highlight
	// if Y axis is up, give it a greenish hue
	// if Z axis is up, a bit of blue
	const float gridColor[] = {
		0.15f,
		useVerticalY ? 0.25f : 0.20f,
		useVerticalY ? 0.20f : 0.25f,
		1.0f
	};


	// RGB
	const float rgba4[] = {
		1.0f, 0.0f, 0.0f, 1.0f,	// red
		0.0f, 1.0f, 0.0f, 1.0f,	// green
		0.0f, 0.0f, 1.0f, 1.0f,	// blue
		0.0f, 1.0f, 1.0f, 1.0f,	// cyan
		1.0f, 0.0f, 1.0f, 1.0f,	// magenta
		1.0f, 1.0f, 0.0f, 1.0f,	// yellow
		1.0f, 0.5f, 0.0f, 1.0f,	// orange
		0.0f, 0.5f, 1.0f, 1.0f,	// sky blue
		0.5f, 0.5f, 0.5f, 1.0f,	// solid grey
		0.5f, 0.5f, 0.5f, 0.5f,	// translucent grey
	},
	*const red = rgba4 + 0, *const green = rgba4 + 4, *const blue = rgba4 + 8,
	*const cyan = rgba4 + 12, *const magenta = rgba4 + 16, *const yellow = rgba4 + 20,
	*const orange = rgba4 + 24, *const skyblue = rgba4 + 28,
	*const grey = rgba4 + 32, *const grey_t = rgba4 + 36;


	// model transformations (if needed)
	const a3mat4 convertY2Z = {
		+1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, +1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, +1.0f,
	};
	const a3mat4 convertZ2Y = {
		+1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, +1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, +1.0f,
	};
	const a3mat4 convertZ2X = {
		0.0f, 0.0f, -1.0f, 0.0f,
		0.0f, +1.0f, 0.0f, 0.0f,
		+1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, +1.0f,
	};

	// final model matrix and full matrix stack
	a3mat4 modelMat = a3identityMat4, modelMatInv = a3identityMat4, modelMatOrig = a3identityMat4,
		modelViewProjectionMat = a3identityMat4;

	// tmp lighting vectors
	a3vec4 lightPos_obj, eyePos_obj;

	// current scene object being rendered, for convenience
	const a3_DemoSceneObject *currentSceneObject;


	// ****TO-DO: 
	//	- activate framebuffer (...instead of...)
	// reset viewport and clear buffers
	//	a3framebufferDeactivateSetViewport(a3fbo_depth24, -demoState->frameBorder, -demoState->frameBorder, demoState->frameWidth, demoState->frameHeight);
	a3framebufferActivate(demoState->fbo_scene);

	// clearing is expensive!
	// instead, draw skybox and force depth to farthest possible value in scene
	// we could call this a "skybox clear" because it serves both purposes
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// draw skybox with texture, inverted
	currentDemoProgram = demoState->prog_drawTexture;
	a3shaderProgramActivate(currentDemoProgram->program);
	currentDrawable = demoState->draw_skybox;
	modelMatInv = demoState->camera->sceneObject->modelMatInv;
	modelMatInv.v3 = a3wVec4;
	a3real4x4Product(modelViewProjectionMat.m, demoState->camera->projectionMat.m, modelMatInv.m);
	if (!useVerticalY)	// need to rotate box if Z-up
		a3real4x4ConcatL(modelViewProjectionMat.m, convertY2Z.m);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, skyblue);
	a3textureActivate(demoState->tex_sky_clouds, a3tex_unit00);

	glDepthFunc(GL_ALWAYS);
	glCullFace(GL_FRONT);
	a3vertexActivateAndRenderDrawable(currentDrawable);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);


	if (demoState->displayGrid)
	{
		// draw grid aligned to world
		currentDemoProgram = demoState->prog_drawColorUnif;
		a3shaderProgramActivate(currentDemoProgram->program);
		currentDrawable = demoState->draw_grid;
		modelViewProjectionMat = demoState->camera->viewProjectionMat;
		if (useVerticalY)
			a3real4x4ConcatL(modelViewProjectionMat.m, convertZ2Y.m);
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
		a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uColor, 1, gridColor);
		a3vertexActivateAndRenderDrawable(currentDrawable);
	}


	// draw objects: 
	//	- correct "up" axis if needed
	//	- calculate full MVP matrix
	//	- move lighting objects' positions into object space
	//	- send uniforms
	//	- draw

	// draw models
	//	currentDemoProgram = demoState->prog_drawPhong;

	// ****TO-DO
	// Add some sort of if statement here to switch between the two shader programs
	
	if (demoState->programType == 0)
		currentDemoProgram = demoState->prog_drawCombinedMRT;
	else
		currentDemoProgram = demoState->prog_drawEffectsMRT;
	
	// currentDemoProgram = demoState->prog_drawAttribsMRT;
	a3shaderProgramActivate(currentDemoProgram->program);

	// ground
	currentDrawable = demoState->draw_groundPlane;
	currentSceneObject = demoState->groundObject;

	modelMat = currentSceneObject->modelMat;
	a3real4x4TransformInverseIgnoreScale(modelMatInv.m, modelMat.m);
	a3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
	a3real4Real4x4Product(lightPos_obj.v, modelMatInv.m, demoState->lightPos_world.v);
	a3real4Real4x4Product(eyePos_obj.v, modelMatInv.m, demoState->cameraObject->modelMat.v3.v);

	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uLightPos_obj, 1, lightPos_obj.v);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uEyePos_obj, 1, eyePos_obj.v);
	a3textureActivate(demoState->tex_stone_dm, a3tex_unit00);
	a3textureActivate(demoState->tex_stone_dm, a3tex_unit01);
	a3vertexActivateAndRenderDrawable(currentDrawable);

	// sphere
	currentDrawable = demoState->draw_sphere;
	currentSceneObject = demoState->sphereObject;

	modelMatOrig = currentSceneObject->modelMat;
	if (useVerticalY)	// sphere's axis is Z
		a3real4x4Product(modelMat.m, modelMatOrig.m, convertZ2Y.m);
	else
		modelMat = modelMatOrig;
	a3real4x4TransformInverseIgnoreScale(modelMatInv.m, modelMat.m);
	a3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
	a3real4Real4x4Product(lightPos_obj.v, modelMatInv.m, demoState->lightPos_world.v);
	a3real4Real4x4Product(eyePos_obj.v, modelMatInv.m, demoState->cameraObject->modelMat.v3.v);

	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uLightPos_obj, 1, lightPos_obj.v);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uEyePos_obj, 1, eyePos_obj.v);
	a3textureActivate(demoState->tex_earth_dm, a3tex_unit00);
	a3textureActivate(demoState->tex_earth_sm, a3tex_unit01);
	a3vertexActivateAndRenderDrawable(currentDrawable);

	// cylinder
	currentDrawable = demoState->draw_cylinder;
	currentSceneObject = demoState->cylinderObject;

	modelMatOrig = currentSceneObject->modelMat;
	a3real4x4Product(modelMat.m, modelMatOrig.m, convertZ2X.m);
	a3real4x4TransformInverseIgnoreScale(modelMatInv.m, modelMat.m);
	a3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
	a3real4Real4x4Product(lightPos_obj.v, modelMatInv.m, demoState->lightPos_world.v);
	a3real4Real4x4Product(eyePos_obj.v, modelMatInv.m, demoState->cameraObject->modelMat.v3.v);

	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uLightPos_obj, 1, lightPos_obj.v);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uEyePos_obj, 1, eyePos_obj.v);
	a3textureActivate(demoState->tex_checker, a3tex_unit00);
	a3textureActivate(demoState->tex_checker, a3tex_unit01);
	a3vertexActivateAndRenderDrawable(currentDrawable);

	// torus
	currentDrawable = demoState->draw_torus;
	currentSceneObject = demoState->torusObject;

	modelMatOrig = currentSceneObject->modelMat;
	a3real4x4Product(modelMat.m, modelMatOrig.m, convertZ2X.m);
	a3real4x4TransformInverseIgnoreScale(modelMatInv.m, modelMat.m);
	a3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
	a3real4Real4x4Product(lightPos_obj.v, modelMatInv.m, demoState->lightPos_world.v);
	a3real4Real4x4Product(eyePos_obj.v, modelMatInv.m, demoState->cameraObject->modelMat.v3.v);

	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uLightPos_obj, 1, lightPos_obj.v);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uEyePos_obj, 1, eyePos_obj.v);
	a3textureActivate(demoState->tex_earth_dm, a3tex_unit00);
	a3textureActivate(demoState->tex_earth_sm, a3tex_unit01);
	a3vertexActivateAndRenderDrawable(currentDrawable);

	// teapot
	currentDrawable = demoState->draw_teapot;
	currentSceneObject = demoState->teapotObject;

	modelMatOrig = currentSceneObject->modelMat;
	if (!useVerticalY)	// teapot's axis is Y
		a3real4x4Product(modelMat.m, modelMatOrig.m, convertY2Z.m);
	else
		modelMat = modelMatOrig;
	a3real4x4TransformInverseIgnoreScale(modelMatInv.m, modelMat.m);
	a3real4x4Product(modelViewProjectionMat.m, demoState->camera->viewProjectionMat.m, modelMat.m);
	a3real4Real4x4Product(lightPos_obj.v, modelMatInv.m, demoState->lightPos_world.v);
	a3real4Real4x4Product(eyePos_obj.v, modelMatInv.m, demoState->cameraObject->modelMat.v3.v);

	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uLightPos_obj, 1, lightPos_obj.v);
	a3shaderUniformSendFloat(a3unif_vec4, currentDemoProgram->uEyePos_obj, 1, eyePos_obj.v);
	a3textureActivate(demoState->tex_checker, a3tex_unit00);
	a3textureActivate(demoState->tex_checker, a3tex_unit01);
	a3vertexActivateAndRenderDrawable(currentDrawable);

	glDisable(GL_STENCIL_TEST);


	// scene is rendered, draw other modes using resulting textures
	// deactivate active FBO, use full frame
	a3framebufferDeactivateSetViewport(a3fbo_depthDisable, -demoState->frameBorder, -demoState->frameBorder, demoState->frameWidth, demoState->frameHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	a3textureActivate(demoState->tex_checker, a3tex_unit00);	// ****TO-DO: change this
	if (demoState->displayDepth)
		a3framebufferBindDepthTexture(demoState->fbo_scene, a3tex_unit00);
	else
		a3framebufferBindColorTexture(demoState->fbo_scene, a3tex_unit00, demoState->demoMode);

	// display previously rendered scene on FSQ
	// do not clear since we are drawing a FULL-SCREEN QUAD
	// render FSQ with texture
	// use simple texturing program
	currentDemoProgram = demoState->prog_drawTexture;
	a3shaderProgramActivate(currentDemoProgram->program);
	a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, a3identityMat4.mm);
	currentDrawable = demoState->draw_fsq;
	a3vertexActivateAndRenderDrawable(currentDrawable);

	if (demoState->displayAxes)
	{
		glDisable(GL_DEPTH_TEST);

		// draw coordinate axes in front of everything
		currentDemoProgram = demoState->prog_drawColor;
		a3shaderProgramActivate(currentDemoProgram->program);
		currentDrawable = demoState->draw_axes;
		a3vertexActivateDrawable(currentDrawable);

		// center of world
		modelViewProjectionMat = demoState->camera->viewProjectionMat;
		a3shaderUniformSendFloatMat(a3unif_mat4, 0, currentDemoProgram->uMVP, 1, modelViewProjectionMat.mm);
		a3vertexRenderActiveDrawable();

		glEnable(GL_DEPTH_TEST);
	}


	// deactivate things
	a3vertexDeactivateDrawable();
	a3shaderProgramDeactivate();
	a3textureDeactivate(a3tex_unit01);
	a3textureDeactivate(a3tex_unit00);


	// HUD
	if (demoState->textInit && demoState->showText)
	{
		// display mode info
		//const char *demoModeText[] = {
		//	"Attribute as color 0: position",
		//	"Attribute as color 1: normal",
		//	"Attribute as color 2: color",
		//	"Attribute as color 3: texture coordinate",
		//	"Attribute as color 4: tangent",
		//	"Attribute as color 5: bitangent",
		//	"Attribute as color 6: blend weights",
		//	"Attribute as color 7: blend indices",
		//};

		const char *demoModeText[] = {
			"Attribute as shader 0: Texture or rgb2hsv",
			"Attribute as shader 1: Diffuse or rgb2hsl",
			"Attribute as shader 2: Lambert or black&white",
			"Attribute as shader 3: Phong or heatmap",
		};

		const float col = (!demoState->displayDepth) ? 1.0f : 0.0f;


		glDisable(GL_DEPTH_TEST);

		a3textDraw(demoState->text, -0.98f, +0.90f, -1.0f, col, col, col, 1.0f,
			"Demo mode (%u / %u): ", demoState->demoMode + 1, demoState->demoModeCount);
		a3textDraw(demoState->text, -0.98f, +0.80f, -1.0f, col, col, col, 1.0f,
			"    %s", demoModeText[demoState->demoMode]);

		if (demoState->displayDepth)
			a3textDraw(demoState->text, -0.98f, +0.70f, -1.0f, col, col, col, 1.0f,
				"Showing DEPTH BUFFER as texture (show color = 'f')");
		else
			a3textDraw(demoState->text, -0.98f, +0.70f, -1.0f, col, col, col, 1.0f,
				"Showing COLOR TARGET %u as texture (show depth = 'f')", demoState->demoMode);

		a3textDraw(demoState->text, -0.98f, +0.60f, -1.0f, col, col, col, 1.0f,
			"GRID in scene (toggle = 'g') %d | AXES overlay ('x') %d", demoState->displayGrid, demoState->displayAxes);


		// display controls
		if (a3XboxControlIsConnected(demoState->xcontrol))
		{
			a3textDraw(demoState->text, -0.98f, -0.50f, -1.0f, col, col, col, 1.0f,
				"Xbox controller camera control: ");
			a3textDraw(demoState->text, -0.98f, -0.60f, -1.0f, col, col, col, 1.0f,
				"    Left joystick = rotate | Right joystick, triggers = move");
		}
		else
		{
			a3textDraw(demoState->text, -0.98f, -0.50f, -1.0f, col, col, col, 1.0f,
				"Keyboard/mouse camera control: ");
			a3textDraw(demoState->text, -0.98f, -0.60f, -1.0f, col, col, col, 1.0f,
				"    Left click & drag = rotate | WASDEQ = move | wheel = zoom");
		}

		a3textDraw(demoState->text, -0.98f, -0.40f, -1.0f, col, col, col, 1.0f,
			"    Switch between shader programs: 'L'");
		a3textDraw(demoState->text, -0.98f, -0.70f, -1.0f, col, col, col, 1.0f,
			"    Toggle demo mode:           ',' prev | next '.' ");
		a3textDraw(demoState->text, -0.98f, -0.80f, -1.0f, col, col, col, 1.0f,
			"    Toggle text display:        't' (toggle) | 'T' (alloc/dealloc) ");
		a3textDraw(demoState->text, -0.98f, -0.90f, -1.0f, col, col, col, 1.0f,
			"    Reload all shader programs: 'P' ****CHECK CONSOLE FOR ERRORS!**** ");

		glEnable(GL_DEPTH_TEST);
	}
}


//-----------------------------------------------------------------------------
