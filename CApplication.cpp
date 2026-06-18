#include "stdafx.h"

#include "CApplication.h"

#include "CQuad.h"
#include "CSphere.h"
#include "CLight.h"

extern CCamera Camera;

bool CApplication::InitScene()
{
	if (!Floor.CreateTexture2D("floor.jpg") || !Cube.CreateTexture2D("cube.jpg") || !Earth.CreateTexture2D("earth.jpg"))
	{
		return false;
	}

	Spheres = CreateSpheres(3);

	Spheres[0] = CSphere(Vector3(-2.0f, -1.0f, 2.0f), 0.5f, Vector3(0.0f, 0.5f, 1.0f), nullptr, 0.875f);
	Spheres[1] = CSphere(Vector3(0.0f, -1.5f, 2.0f), 0.5f, Vector3(0.0f, 0.5f, 1.0f), nullptr, 0.125f, 0.875f, 1.52f);
	Spheres[2] = CSphere(Vector3(2.0f, -1.5f, -2.0f), 0.5f, Vector3(1.0f, 1.0f, 1.0f), &Earth);

	Quads = CreateQuads(21);

	Lights = CreateLights(1);

	Matrix4x4 R = RotationMatrix(22.5f, Vector3(0.0f, 1.0f, 0.0f));
	Vector3 V = Vector3(2.0f, 0.0f, 2.0f);

	Quads[0] = CQuad(R * Vector3(-0.5f, -2.0f, 0.5f) + V, R * Vector3(0.5f, -2.0f, 0.5f) + V, R * Vector3(0.5f, -1.0f, 0.5f) + V, R * Vector3(-0.5f, -1.0f, 0.5f) + V, Vector3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[1] = CQuad(R * Vector3(0.5f, -2.0f, -0.5f) + V, R * Vector3(-0.5f, -2.0f, -0.5f) + V, R * Vector3(-0.5f, -1.0f, -0.5f) + V, R * Vector3(0.5f, -1.0f, -0.5f) + V, Vector3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[2] = CQuad(R * Vector3(0.5f, -2.0f, 0.5f) + V, R * Vector3(0.5f, -2.0f, -0.5f) + V, R * Vector3(0.5f, -1.0f, -0.5f) + V, R * Vector3(0.5f, -1.0f, 0.5f) + V, Vector3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[3] = CQuad(R * Vector3(-0.5f, -2.0f, -0.5f) + V, R * Vector3(-0.5f, -2.0f, 0.5f) + V, R * Vector3(-0.5f, -1.0f, 0.5f) + V, R * Vector3(-0.5f, -1.0f, -0.5f) + V, Vector3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[4] = CQuad(R * Vector3(-0.5f, -1.0f, 0.5f) + V, R * Vector3(0.5f, -1.0f, 0.5f) + V, R * Vector3(0.5f, -1.0f, -0.5f) + V, R * Vector3(-0.5f, -1.0f, -0.5f) + V, Vector3(1.0f, 1.0f, 1.0f), &Cube);
	Quads[5] = CQuad(R * Vector3(-0.5f, -2.0f, -0.5f) + V, R * Vector3(0.5f, -2.0f, -0.5f) + V, R * Vector3(0.5f, -2.0f, 0.5f) + V, R * Vector3(-0.5f, -2.0f, 0.5f) + V, Vector3(1.0f, 1.0f, 1.0f), &Cube);

	Quads[6] = CQuad(Vector3(-0.0f, -2.0f, 4.0f), Vector3(4.0f, -2.0f, 4.0f), Vector3(4.0f, -2.0f, -4.0f), Vector3(-0.0f, -2.0f, -4.0f), Vector3(1.0f, 1.0f, 1.0f), &Floor, 0.0625f);
	Quads[7] = CQuad(Vector3(-4.0f, -2.0f, 4.0f), Vector3(0.0f, -2.0f, 4.0f), Vector3(0.0f, -2.0f, 0.0f), Vector3(-4.0f, -2.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), &Floor, 0.0625f);
	Quads[8] = CQuad(Vector3(0.0f, 2.0f, -4.0f), Vector3(4.0f, 2.0f, -4.0f), Vector3(4.0f, 2.0f, 4.0f), Vector3(0.0f, 2.0f, 4.0f), Vector3(1.0f, 1.0f, 1.0f));
	Quads[9] = CQuad(Vector3(-4.0f, 2.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 2.0f, 4.0f), Vector3(-4.0f, 2.0f, 4.0f), Vector3(1.0f, 1.0f, 1.0f));
	Quads[10] = CQuad(Vector3(-0.0f, -2.0f, -4.0f), Vector3(4.0f, -2.0f, -4.0f), Vector3(4.0f, 2.0f, -4.0f), Vector3(-0.0f, 2.0f, -4.0f), Vector3(1.0f, 1.0f, 1.0f));
	Quads[11] = CQuad(Vector3(4.0f, -2.0f, 4.0f), Vector3(-4.0f, -2.0f, 4.0f), Vector3(-4.0f, 2.0f, 4.0f), Vector3(4.0f, 2.0f, 4.0f), Vector3(1.0f, 1.0f, 1.0f));
	Quads[12] = CQuad(Vector3(4.0f, -2.0f, -4.0f), Vector3(4.0f, -2.0f, 4.0f), Vector3(4.0f, 2.0f, 4.0f), Vector3(4.0f, 2.0f, -4.0f), Vector3(0.0f, 1.0f, 0.0f));
	Quads[13] = CQuad(Vector3(-4.0f, -2.0f, 4.0f), Vector3(-4.0f, -2.0f, -0.0f), Vector3(-4.0f, 2.0f, -0.0f), Vector3(-4.0f, 2.0f, 4.0f), Vector3(1.0f, 0.0f, 0.0f));
	Quads[14] = CQuad(Vector3(-4.0f, -2.0f, 0.0f), Vector3(0.0f, -2.0f, 0.0f), Vector3(0.0f, 2.0f, 0.0f), Vector3(-4.0f, 2.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
	Quads[15] = CQuad(Vector3(0.0f, -2.0f, 0.0f), Vector3(0.0f, -2.0f, -4.0f), Vector3(0.0f, 2.0f, -4.0f), Vector3(0.0f, 2.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));

	Vector3 S = Vector3(-2.0f, 0.0f, 2.0f);

	Quads[16] = CQuad(Vector3(-0.5f, 1.875f, 0.5f) + S, Vector3(0.5f, 1.875f, 0.5f) + S, Vector3(0.5f, 1.875f, -0.5f) + S, Vector3(-0.5f, 1.875f, -0.5f) + S, Vector3(1.0f, 1.0f, 1.0f));
	Quads[17] = CQuad(Vector3(-0.5f, 1.875f - 0.125f, 0.5f) + S, Vector3(0.5f, 1.875f - 0.125f, 0.5f) + S, Vector3(0.5f, 2.0f - 0.125f, 0.5f) + S, Vector3(-0.5f, 2.0f - 0.125f, 0.5f) + S, Vector3(1.0f, 1.0f, 1.0f));
	Quads[18] = CQuad(Vector3(0.5f, 1.875f - 0.125f, -0.5f) + S, Vector3(-0.5f, 1.875f - 0.125f, -0.5f) + S, Vector3(-0.5f, 2.0f - 0.125f, -0.5f) + S, Vector3(0.5f, 2.0f - 0.125f, -0.5f) + S, Vector3(1.0f, 1.0f, 1.0f));
	Quads[19] = CQuad(Vector3(-0.5f, 1.875f - 0.125f, -0.5f) + S, Vector3(-0.5f, 1.875f - 0.125f, 0.5f) + S, Vector3(-0.5f, 2.0f - 0.125f, 0.5f) + S, Vector3(-0.5f, 2.0f - 0.125f, -0.5f) + S, Vector3(1.0f, 1.0f, 1.0f));
	Quads[20] = CQuad(Vector3(0.5f, 1.875f - 0.125f, 0.5f) + S, Vector3(0.5f, 1.875f - 0.125f, -0.5f) + S, Vector3(0.5f, 2.0f - 0.125f, -0.5f) + S, Vector3(0.5f, 2.0f - 0.125f, 0.5f) + S, Vector3(1.0f, 1.0f, 1.0f));

	Lights[0].Quad = new CQuad(Vector3(-0.5f, 1.875f - 0.125f, -0.5f) + S, Vector3(0.5f, 1.875f - 0.125f, -0.5f) + S, Vector3(0.5f, 1.875f - 0.125f, 0.5f) + S, Vector3(-0.5f, 1.875f - 0.125f, 0.5f) + S, Vector3(3.0f, 3.0f, 3.0f));
	Lights[0].Ambient = 0.25f;
	Lights[0].Diffuse = 0.75f;

	Camera.LookAt(Vector3(0.0f), Vector3(0.0f, 0.0f, 8.75f), true);

	return true;
}

void CApplication::DestroyTextures()
{
	Floor.Destroy();
	Cube.Destroy();
	Earth.Destroy();
}

void CApplication::CollectSceneTextures(std::vector<CTexture*>& textures) const
{
	if (Floor.HasData()) textures.push_back(const_cast<CTexture*>(&Floor));
	if (Cube.HasData()) textures.push_back(const_cast<CTexture*>(&Cube));
	if (Earth.HasData()) textures.push_back(const_cast<CTexture*>(&Earth));
}

CApplication Application;