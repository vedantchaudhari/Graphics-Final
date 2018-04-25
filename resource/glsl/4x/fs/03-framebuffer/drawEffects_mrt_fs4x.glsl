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
	vec4 vPosition;
	vec3 vNormal;
	vec2 vTexcoord;
	vec3 vTangent;
	vec3 vBitangent;
} vPassData;

// layout (location = 0) out vec4 rtPosition;
layout (location = 0) out vec4 rtNormal;
layout (location = 1) out vec4 rtTexcoord;
layout (location = 2) out vec4 rtTangent;
layout (location = 3) out vec4 rtBitangent;

// Heat map
vec3 heatMap(vec3 color)
{
	float greyVal = dot(color.rgb, vec3(0.20, 0.6, 0.11));
	vec3 heat;
	heat.r = smoothstep(0.5, 0.8, greyVal);
	if (greyVal >= 0.9)
		heat.r *= (1.1 - greyVal) * 5.0;

	if (greyVal > 0.7)	
		heat.g = smoothstep(1.0, 0.7, greyVal);
	else
		heat.g = smoothstep(0.0, 0.7, greyVal);

	heat.b = smoothstep(0.0, 0.7, greyVal);
	if (greyVal <= 0.3)
		heat.b *= greyVal / 0.3;
		
	return heat;
}

// rgb to hsl
vec3 rgb2hsl(vec3 color)
{
// (1) convert rgb values to range 0-1 by dividing value by 255
	float h = color.r / 255.0;
	float s = color.g / 255.0;
	float l = color.b / 255.0;

// (2) find min and max values of r, g and b
	float hslMin = min(h, min(s, l));
	float hslMax = max(h, max(s, l));

// (3) calculate Luminace value by adding max and min values and dividing by 2
	float L =(hslMin + hslMax) / 2;

// (4) find the saturation
//		if all rgb values are the same, there is no saturation
	float S;
	if (hslMin == hslMax)
		S = 0;
//		otherwise, check level of Lumincance to slect correct formula
	else
	{
		if (L >= 0.5)
			S = (hslMax - hslMin) / (hslMax + hslMin);
		if (L < 0.5)
			S = (hslMax - hslMin) / (2.0 - hslMax - hslMin);
	}

// (6) Calculate Hue based on max
	float H;
	if (h == hslMax)
		H = (s - l) / (hslMax - hslMin);
	if (s == hslMax)
		H = (2.0 + ((l - h) / (hslMax - hslMin)));
	if (l == hslMax)
		H = (4.0 + ((h - s) / (hslMax - hslMin)));

	return vec3(H, S, L);
}

// black and white
vec3 blackAndWhite(vec3 color)
{
	// initialize black and white
	vec3 black = vec3(1.0, 1.0, 1.0);
	vec3 white = vec3(0.0, 0.0, 0.0);

	// average color out
	float avg = (color.r + color.g + color.b) / 3.0;

	if (avg <= 0.5)
		return black;
	else 
		return white;
}

vec3 rgb2hsv(vec3 c)
{
	// Stack Overflow helped with this!
	// ****TO-DO
	// Insert link to thread here

	//float r = color.r / 255;
	//float g = color.g / 255;
	//float b = color.b / 255;

	//float hsvMin = min(r, min(g, b));
	//float hsvMax = max(r, max(g, b));

	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	// rtPosition	= vPassData.vPosition;								// position effect
	rtNormal	= vec4(heatMap(vPassData.vNormal), 1.0);			// heat map
	rtTexcoord	= vec4(rgb2hsv(vPassData.vBitangent), 1.0);			// rgb2hsv
	rtTangent	= vec4(blackAndWhite(vPassData.vTangent), 1.0);		// black and white
	rtBitangent = vec4(rgb2hsl(vPassData.vNormal), 1.0);			// rgb2hsl
}