#include "stdafx.h"
#include "MyRayTracer.h"
#include "Quad.h"
#include "Sphere.h"
#include "Light.h"

extern CCamera Camera;

bool CMyRayTracer::InitScene()
{
	if (!Floor.CreateTexture2D("floor.jpg") || !Cube.CreateTexture2D("cube.jpg") || !Earth.CreateTexture2D("earth.jpg"))
	{
		return false;
	}

	SpheresCount = 3;

	Spheres = CreateSpheres(SpheresCount);

	Spheres[0] = CSphere(vec3(-2.0f, -1.0f, 2.0f), 0.5f, vec3(0.0f, 0.5f, 1.0f), NULL, 0.875f);
	Spheres[1] = CSphere(vec3(0.0f, -1.5f, 2.0f), 0.5f, vec3(0.0f, 0.5f, 1.0f), NULL, 0.125f, 0.875f, 1.52f);
	Spheres[2] = CSphere(vec3(2.0f, -1.5f, -2.0f), 0.5f, vec3(1.0f, 1.0f, 1.0f), &Earth);

	QuadsCount = 21;

	Quads = CreateQuads(QuadsCount);

	LightsCount = 1;

	Lights = CreateLights(LightsCount);

	mat4x4 R = RotationMatrix(22.5f, vec3(0.0f, 1.0f, 0.0f));
	vec3 V = vec3(2.0f, 0.0f, 2.0f);

	Quads[0] = CQuad(R * vec3(-0.5f, -2.0f, 0.5f) + V, R * vec3(0.5f, -2.0f, 0.5f) + V, R * vec3(0.5f, -1.0f, 0.5f) + V, R * vec3(-0.5f, -1.0f, 0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[1] = CQuad(R * vec3(0.5f, -2.0f, -0.5f) + V, R * vec3(-0.5f, -2.0f, -0.5f) + V, R * vec3(-0.5f, -1.0f, -0.5f) + V, R * vec3(0.5f, -1.0f, -0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[2] = CQuad(R * vec3(0.5f, -2.0f, 0.5f) + V, R * vec3(0.5f, -2.0f, -0.5f) + V, R * vec3(0.5f, -1.0f, -0.5f) + V, R * vec3(0.5f, -1.0f, 0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[3] = CQuad(R * vec3(-0.5f, -2.0f, -0.5f) + V, R * vec3(-0.5f, -2.0f, 0.5f) + V, R * vec3(-0.5f, -1.0f, 0.5f) + V, R * vec3(-0.5f, -1.0f, -0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[4] = CQuad(R * vec3(-0.5f, -1.0f, 0.5f) + V, R * vec3(0.5f, -1.0f, 0.5f) + V, R * vec3(0.5f, -1.0f, -0.5f) + V, R * vec3(-0.5f, -1.0f, -0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[5] = CQuad(R * vec3(-0.5f, -2.0f, -0.5f) + V, R * vec3(0.5f, -2.0f, -0.5f) + V, R * vec3(0.5f, -2.0f, 0.5f) + V, R * vec3(-0.5f, -2.0f, 0.5f) + V, vec3(1.0f, 1.0f, 1.0f), &Cube);

	Quads[6] = CQuad(vec3(-0.0f, -2.0f, 4.0f), vec3(4.0f, -2.0f, 4.0f), vec3(4.0f, -2.0f, -4.0f), vec3(-0.0f, -2.0f, -4.0f), vec3(1.0f, 1.0f, 1.0f), &Floor, 0.0625f);
	Quads[7] = CQuad(vec3(-4.0f, -2.0f, 4.0f), vec3(0.0f, -2.0f, 4.0f), vec3(0.0f, -2.0f, 0.0f), vec3(-4.0f, -2.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), &Floor, 0.0625f);
	Quads[8] = CQuad(vec3(0.0f, 2.0f, -4.0f), vec3(4.0f, 2.0f, -4.0f), vec3(4.0f, 2.0f, 4.0f), vec3(0.0f, 2.0f, 4.0f), vec3(1.0f, 1.0f, 1.0f));
	Quads[9] = CQuad(vec3(-4.0f, 2.0f, 0.0f), vec3(0.0f, 2.0f, 0.0f), vec3(0.0f, 2.0f, 4.0f), vec3(-4.0f, 2.0f, 4.0f), vec3(1.0f, 1.0f, 1.0f));
	Quads[10] = CQuad(vec3(-0.0f, -2.0f, -4.0f), vec3(4.0f, -2.0f, -4.0f), vec3(4.0f, 2.0f, -4.0f), vec3(-0.0f, 2.0f, -4.0f), vec3(1.0f, 1.0f, 1.0f));
	Quads[11] = CQuad(vec3(4.0f, -2.0f, 4.0f), vec3(-4.0f, -2.0f, 4.0f), vec3(-4.0f, 2.0f, 4.0f), vec3(4.0f, 2.0f, 4.0f), vec3(1.0f, 1.0f, 1.0f));
	Quads[12] = CQuad(vec3(4.0f, -2.0f, -4.0f), vec3(4.0f, -2.0f, 4.0f), vec3(4.0f, 2.0f, 4.0f), vec3(4.0f, 2.0f, -4.0f), vec3(0.0f, 1.0f, 0.0f));
	Quads[13] = CQuad(vec3(-4.0f, -2.0f, 4.0f), vec3(-4.0f, -2.0f, -0.0f), vec3(-4.0f, 2.0f, -0.0f), vec3(-4.0f, 2.0f, 4.0f), vec3(1.0f, 0.0f, 0.0f));
	Quads[14] = CQuad(vec3(-4.0f, -2.0f, 0.0f), vec3(0.0f, -2.0f, 0.0f), vec3(0.0f, 2.0f, 0.0f), vec3(-4.0f, 2.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
	Quads[15] = CQuad(vec3(0.0f, -2.0f, 0.0f), vec3(0.0f, -2.0f, -4.0f), vec3(0.0f, 2.0f, -4.0f), vec3(0.0f, 2.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));

	vec3 S = vec3(-2.0f, 0.0f, 2.0f);

	Quads[16] = CQuad(vec3(-0.5f, 1.875f, 0.5f) + S, vec3(0.5f, 1.875f, 0.5f) + S, vec3(0.5f, 1.875f, -0.5f) + S, vec3(-0.5f, 1.875f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
	Quads[17] = CQuad(vec3(-0.5f, 1.875f - 0.125f, 0.5f) + S, vec3(0.5f, 1.875f - 0.125f, 0.5f) + S, vec3(0.5f, 2.0f - 0.125f, 0.5f) + S, vec3(-0.5f, 2.0f - 0.125f, 0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
	Quads[18] = CQuad(vec3(0.5f, 1.875f - 0.125f, -0.5f) + S, vec3(-0.5f, 1.875f - 0.125f, -0.5f) + S, vec3(-0.5f, 2.0f - 0.125f, -0.5f) + S, vec3(0.5f, 2.0f - 0.125f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
	Quads[19] = CQuad(vec3(-0.5f, 1.875f - 0.125f, -0.5f) + S, vec3(-0.5f, 1.875f - 0.125f, 0.5f) + S, vec3(-0.5f, 2.0f - 0.125f, 0.5f) + S, vec3(-0.5f, 2.0f - 0.125f, -0.5f) + S, vec3(1.0f, 1.0f, 1.0f));
	Quads[20] = CQuad(vec3(0.5f, 1.875f - 0.125f, 0.5f) + S, vec3(0.5f, 1.875f - 0.125f, -0.5f) + S, vec3(0.5f, 2.0f - 0.125f, -0.5f) + S, vec3(0.5f, 2.0f - 0.125f, 0.5f) + S, vec3(1.0f, 1.0f, 1.0f));

	Lights[0].Quad = new CQuad(vec3(-0.5f, 1.875f - 0.125f, -0.5f) + S, vec3(0.5f, 1.875f - 0.125f, -0.5f) + S, vec3(0.5f, 1.875f - 0.125f, 0.5f) + S, vec3(-0.5f, 1.875f - 0.125f, 0.5f) + S, vec3(3.0f, 3.0f, 3.0f));
	Lights[0].Ambient = 0.25f;
	Lights[0].Diffuse = 0.75f;

	Camera.LookAt(vec3(0.0f), vec3(0.0f, 0.0f, 8.75f), true);

	return true;
}

void CMyRayTracer::DestroyTextures()
{
	Floor.Destroy();
	Cube.Destroy();
	Earth.Destroy();
}

CMyRayTracer RayTracer;