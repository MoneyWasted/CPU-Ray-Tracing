#pragma once

#include "glmath.h"

// GPU-compatible structures for compute shader
// Must match HLSL struct layouts exactly

struct GPUSphere
{
	Vector3 Position;
	float Radius;
	Vector3 Color;
	float Reflection;
	float Refraction;
	float Eta;
	float ODRadius;
	int TextureIndex;  // -1 for no texture
};

struct GPUQuad
{
	Vector3 a;
	float Reflection;
	Vector3 b;
	float Refraction;
	Vector3 c;
	float Eta;
	Vector3 d;
	float ODEta;
	Vector3 Color;
	int TextureIndex;  // -1 for no texture
	Vector3 Normal;
	float D;
	Vector3 Tangent;  // T
	float D1;
	Vector3 Binormal; // B
	float D2;
	Vector3 m;
	float D3;
	Vector3 Offset;   // O
	float D4;
};

struct GPULight
{
	Vector3 Position;
	float Radius;
	Vector3 Color;
	int IsSphere;  // 0 = quad, 1 = sphere
	int ObjectIndex;  // Index into spheres or quads array
	float Ambient;
	float Diffuse;
	float _pad;
};

struct GPURayTraceConstants
{
	unsigned int Width;
	unsigned int Height;
	unsigned int Samples;
	unsigned int GISamples;
	unsigned int FrameIndex;
	unsigned int MaxDepth;
	float AmbientOcclusionIntensity;
	int Textures;
	int SoftShadows;
	int AmbientOcclusion;
	unsigned int NumSpheres;
	unsigned int NumQuads;
	unsigned int NumLights;
	unsigned int NumTextures;
	Vector3 CameraPosition;
	float _pad1;
	Matrix4x4 CameraRayMatrix;
};

struct TextureInfo
{
	unsigned int Width;
	unsigned int Height;
	unsigned int _pad0;
	unsigned int _pad1;
};
