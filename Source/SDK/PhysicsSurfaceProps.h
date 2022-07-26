#pragma once

#include "Inconstructible.h"
#include "Pad.h"
#include "VirtualMethod.h"

struct SurfaceData {
    PAD(80)
    float maxspeedfactor;
    float jumpfactor;
    float penetrationmodifier;
    float damagemodifier;
    short material;
    bool climbable;
};
struct surfacephysicsparams_t {
    float friction;
    float elasticity;
    float density;
    float thickness;
    float dampening;
};
class PhysicsSurfaceProps {
public:
    INCONSTRUCTIBLE(PhysicsSurfaceProps)

    VIRTUAL_METHOD_V(SurfaceData*, getSurfaceData, 5, (int index), (this, index))
};
class PhysicsSurfaceProps {
public:
    PhysicsSurfaceProps(void) {}

    // parses a text file containing surface prop keys
    virtual int ParseSurfaceData(const char* pFilename,
        const char* pTextfile) = 0;

    // current number of entries in the database
    virtual int SurfacePropCount(void) const = 0;

    virtual int GetSurfaceIndex(const char* pSurfacePropName) const = 0;

    virtual void GetPhysicsProperties(int surfaceDataIndex, float* density,
        float* thickness, float* friction,
        float* elasticity) const = 0;

    virtual SurfaceData* GetSurfaceData(int surfaceDataIndex) = 0;

    virtual const char* GetString(unsigned short stringTableIndex) const = 0;

    virtual const char* GetPropName(int surfaceDataIndex) const = 0;

    // sets the global index table for world materials
    // UNDONE: Make this per-CPhysCollide
    virtual void SetWorldMaterialIndexTable(int* pMapArray, int mapSize) = 0;

    // NOTE: Same as GetPhysicsProperties, but maybe more convenient
    virtual void
        GetPhysicsParameters(int surfaceDataIndex,
            surfacephysicsparams_t* pParamsOut) const = 0;
};
