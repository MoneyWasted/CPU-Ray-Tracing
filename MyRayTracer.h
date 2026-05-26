#pragma once

#include "RayTracer.h"
#include "Texture.h"

class CMyRayTracer : public CRayTracer
{
private:
	CTexture Floor, Cube, Earth;

protected:
	bool InitScene() override;
	void DestroyTextures() override;
};