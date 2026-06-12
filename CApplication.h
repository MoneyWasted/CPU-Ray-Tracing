#pragma once

#include "CRayTracer.h"
#include "CTexture.h"

class CApplication : public CRayTracer
{
private:
	CTexture Floor, Cube, Earth;

	bool InitScene() override;
	void DestroyTextures() override;

public:
	CApplication() = default;
	virtual ~CApplication() = default;

	CApplication(const CApplication&) = delete;
	CApplication& operator=(const CApplication&) = delete;
	CApplication(CApplication&&) = delete;
	CApplication& operator=(CApplication&&) = delete;
};