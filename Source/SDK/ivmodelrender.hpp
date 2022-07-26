#pragma once


#include <SDK/Vector.h>

// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/engine/ivmodelrender.h

struct DrawModelState_t {
	void*			m_pStudioHdr;
	void*			m_pStudioHWData;
	void*		m_pRenderable;
	const matrix3x4		*m_pModelToWorld;
	void*		m_decals;
	int						m_drawFlags;
	int						m_lod;
};

struct model_t {
	char name[255];
};

struct ModelRenderInfo_t {
	Vector origin;
	QAngle angles; 
	char _padding[0x4];
	void** pRenderable;
	const model_t* pModel;
	const matrix3x4* pModelToWorld;
	const matrix3x4* pLightingOffset;
	const Vector* pLightingOrigin;
	int flags;
	int entity_index;
	int skin;
	int body;
	int hitboxset;
	void* instance;

	ModelRenderInfo_t()
	{
		pModelToWorld = NULL;
		pLightingOffset = NULL;
		pLightingOrigin = NULL;
	}
};

