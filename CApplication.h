#pragma once

#include "CRayTracer.h"
#include "CTexture.h"

class CApplication : public CRayTracer
{
private:
	CTexture Floor, Cube, Earth;

protected:
	bool InitScene() override;
	void DestroyTextures() override;
};