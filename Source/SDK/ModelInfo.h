#pragma once

#include <cstdint>

#include "Inconstructible.h"
#include "Pad.h"
#include "Vector.h"
#include "VirtualMethod.h"

struct StudioBbox {
    int bone;
    int group;
    Vector bbMin;
    Vector bbMax;
    int hitboxNameIndex;
    Vector offsetOrientation;
    float capsuleRadius;
    int	unused[4];
};

enum class Hitbox {
    Head,
    Neck,
    Pelvis,
    Belly,
    Thorax,
    LowerChest,
    UpperChest,
    RightThigh,
    LeftThigh,
    RightCalf,
    LeftCalf,
    RightFoot,
    LeftFoot,
    RightHand,
    LeftHand,
    RightUpperArm,
    RightForearm,
    LeftUpperArm,
    LeftForearm,
    Max
};

struct StudioHitboxSet {
    int nameIndex;
    int numHitboxes;
    int hitboxIndex;

    const char* getName() noexcept
    {
        return nameIndex ? reinterpret_cast<const char*>(std::uintptr_t(this) + nameIndex) : nullptr;
    }
	StudioHitboxSet* pHitboxSet(int i) const
	{
		(i >= 0 && i < numHitboxes);
		return (StudioHitboxSet*)(((unsigned char*)this) + hitboxIndex) + i;
	};
    StudioBbox* getHitbox(int i) noexcept
    {
        return i >= 0 && i < numHitboxes ? reinterpret_cast<StudioBbox*>(std::uintptr_t(this) + hitboxIndex) + i : nullptr;
    }

    StudioBbox* getHitbox(Hitbox i) noexcept
    {
        return static_cast<int>(i) < numHitboxes ? reinterpret_cast<StudioBbox*>(std::uintptr_t(this) + hitboxIndex) + static_cast<int>(i) : nullptr;
    }
	inline StudioBbox* pHitbox(int i) const
	{
		return (StudioBbox*)(((unsigned char*)this) + hitboxIndex) + i;
	};
};

constexpr auto MAXSTUDIOBONES = 256;
constexpr auto BONE_USED_BY_HITBOX = 0x100;

struct StudioBone {
    int nameIndex;
    int	parent;
    PAD(152)
    int flags;
    PAD(52)

    const char* getName() const noexcept
    {
        return nameIndex ? reinterpret_cast<const char*>(std::uintptr_t(this) + nameIndex) : nullptr;
    }
};
struct studiohdr_t {
	int id;
	int version;
	int checksum;        // this has to be the same in the phy and vtx files to load!
	char name[64];
	int length;

	Vector eyeposition;    // ideal eye position
	Vector illumposition;    // illumination center
	Vector hull_min;        // ideal movement hull size
	Vector hull_max;
	Vector view_bbmin;        // clipping bounding box
	Vector view_bbmax;

	int flags;
	int numbones;            // bones
	int boneindex;
	inline StudioBone* pBone(int i) const
	{
		Assert(i >= 0 && i < numbones);
		return (StudioBone*)(((unsigned char*)this) + boneindex) + i;
	};

	int RemapSeqBone(int iSequence, int iLocalBone) const;    // maps local sequence bone to global bone
	int RemapAnimBone(int iAnim, int iLocalBone) const;        // maps local animations bone to global bone
	int numbonecontrollers;        // bone controllers
	int bonecontrollerindex;
	int numhitboxsets;
	int hitboxsetindex;
	StudioHitboxSet* pHitboxSet(int i) const
	{
		(i >= 0 && i < numhitboxsets);
		return (StudioHitboxSet*)(((unsigned char*)this) + hitboxsetindex) + i;
	};
	// Look up hitbox set by index
	StudioHitboxSet* getHitboxs(int i) const
	{
		(i >= 0 && i < numhitboxsets);
		return (StudioHitboxSet*)(((unsigned char*)this) + hitboxsetindex) + i;
	};

	// Calls through to hitbox to determine size of specified set
	inline mstudiobbox_t* pHitbox(int i, int set) 
	{
		StudioHitboxSet const* s = getHitboxs(set);
		if (!s)
			return nullptr;

		return s->pHitbox(i);
	};

	// Calls through to set to get hitbox count for set
	inline int iHitboxCount(int set) const
	{
		StudioHitboxSet const* s = pHitboxSet(set);
		if (!s)
			return 0;

		return s->numHitboxes;
	};

	// file local animations? and sequences
	//private:
	int numlocalanim;            // animations/poses
	int localanimindex;        // animation descriptions
	int numlocalseq;                // sequences
	int localseqindex;

	//public:
	bool SequencesAvailable() const;

	int GetNumSeq() const;

	int iRelativeAnim(int baseseq, int relanim) const;    // maps seq local anim reference to global anim index
	int iRelativeSeq(int baseseq, int relseq) const;        // maps seq local seq reference to global seq index

	//private:
	mutable int activitylistversion;    // initialization flag - have the sequences been indexed?
	mutable int eventsindexed;

	//public:
	int GetSequenceActivity(int iSequence);

	void SetSequenceActivity(int iSequence, int iActivity);

	int GetActivityListVersion();

	void SetActivityListVersion(int version) const;

	int GetEventListVersion();

	void SetEventListVersion(int version);

	// raw textures
	int numtextures;
	int textureindex;

	// raw textures search paths
	int numcdtextures;
	int cdtextureindex;

	inline char* pCdtexture(int i) const
	{
		return (((char*)this) + *((int*)(((unsigned char*)this) + cdtextureindex) + i));
	};

	// replaceable textures tables
	int numskinref;
	int numskinfamilies;
	int skinindex;

	inline short* pSkinref(int i) const
	{
		return (short*)(((unsigned char*)this) + skinindex) + i;
	};
	int numbodyparts;
	int bodypartindex;

	// queryable attachable points
	//private:
	int numlocalattachments;
	int localattachmentindex;

	//public:
	int GetNumAttachments() const;

	int GetAttachmentBone(int i);

	// used on my tools in hlmv, not persistant
	void SetAttachmentBone(int iAttachment, int iBone);

	// animation node to animation node transition graph
	//private:
	int numlocalnodes;
	int localnodeindex;
	int localnodenameindex;

	inline char* pszLocalNodeName(int iNode) const
	{
		(iNode >= 0 && iNode < numlocalnodes);
		return (((char*)this) + *((int*)(((unsigned char*)this) + localnodenameindex) + iNode));
	}

	inline unsigned char* pLocalTransition(int i) const
	{
		(i >= 0 && i < (numlocalnodes* numlocalnodes));
		return (unsigned char*)(((unsigned char*)this) + localnodeindex) + i;
	};

	//public:
	int EntryNode(int iSequence);

	int ExitNode(int iSequence);

	char* pszNodeName(int iNode);

	int GetTransition(int iFrom, int iTo) const;

	int numflexdesc;
	int flexdescindex;
	int numflexcontrollers;
	int flexcontrollerindex;
	int numflexrules;
	int flexruleindex;
	int numikchains;
	int ikchainindex;
	int nummouths;
	int mouthindex;

	//private:
	int numlocalposeparameters;
	int localposeparamindex;

	//public:
	int GetNumPoseParameters() const;

	int GetSharedPoseParameter(int iSequence, int iLocalPose) const;

	int surfacepropindex;

	inline char* pszSurfaceProp() const
	{
		return ((char*)this) + surfacepropindex;
	}

	// Key values
	int keyvalueindex;
	int keyvaluesize;

	inline const char* KeyValueText() const
	{
		return keyvaluesize != 0 ? ((char*)this) + keyvalueindex : nullptr;
	}

	int numlocalikautoplaylocks;
	int localikautoplaylockindex;

	int GetNumIKAutoplayLocks() const;

	int CountAutoplaySequences() const;

	int CopyAutoplaySequences(unsigned short* pOut, int outCount) const;

	int GetAutoplayList(unsigned short** pOut) const;

	// The collision model mass that jay wanted
	float mass;
	int contents;

	// external animations, models, etc.
	int numincludemodels;
	int includemodelindex;

	// implementation specific call to get a named model
	const studiohdr_t* FindModel(void** cache, char const* modelname) const;

	// implementation specific back pointer to virtual data
	mutable void* virtualModel;
	//virtualmodel_t		GetVirtualModel() const;

	// for demand loaded animation blocks
	int szanimblocknameindex;

	inline char* pszAnimBlockName() const
	{
		return ((char*)this) + szanimblocknameindex;
	}

	int numanimblocks;
	int animblockindex;
	mutable void* animblockModel;

	unsigned char* GetAnimBlock(int i) const;

	int bonetablebynameindex;

	inline const unsigned char* GetBoneTableSortedByName() const
	{
		return (unsigned char*)this + bonetablebynameindex;
	}

	// used by tools only that don't cache, but persist mdl's peer data
	// engine uses virtualModel to back link to cache pointers
	void* pVertexBase;
	void* pIndexBase;

	// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
	// this value is used to calculate directional components of lighting
	// on static props
	unsigned char constdirectionallightdot;

	// set during load of mdl data to track* desired* lod configuration (not actual)
	// the* actual* clamped root lod is found in studiohwdata
	// this is stored here as a global store to ensure the staged loading matches the rendering
	unsigned char rootLOD;

	// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
	// to be set as root LOD:
	//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
	//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
	unsigned char numAllowedRootLODs;
	unsigned char unused[1];
	int unused4; // zero out if version < 47
	int numflexcontrollerui;
	int flexcontrolleruiindex;
	int unused3[2];

	// FIXME: Remove when we up the model version. Move all fields of studiohdr2_t into studiohdr_t.
	int studiohdr2index;

	// NOTE: No room to add stuff? Up the .mdl file format version
	// [and move all fields in studiohdr2_t into studiohdr_t and kill studiohdr2_t],
	// or add your stuff to studiohdr2_t. See NumSrcBoneTransforms/SrcBoneTransform for the pattern to use.
	int unused2[1];

	studiohdr_t() { }
private:
	// No copy constructors allowed
	studiohdr_t(const studiohdr_t& vOther);

	friend struct virtualmodel_t;
};
struct mstudiobbox_t {
	int bone;
	int group;
	Vector bbmin;
	Vector bbmax;
	int hitboxnameindex;
	Vector rotation;
	float radius;
	int pad2[4];

	char* pszHitboxName()
	{
		if (hitboxnameindex == 0)
			return nullptr;

		return ((char*)this) + hitboxnameindex;
	}
};
struct StudioHdr {
    int id;
    int version;
    int checksum;
    char name[64];
    int length;
    Vector eyePosition;
    Vector illumPosition;
    Vector hullMin;
    Vector hullMax;
    Vector bbMin;
    Vector bbMax;
    int flags;
    int numBones;
    int boneIndex;
    int numBoneControllers;
    int boneControllerIndex;
    int numHitboxSets;
    int hitboxSetIndex;

    const StudioBone* getBone(int i) const noexcept
    {
        return i >= 0 && i < numBones ? reinterpret_cast<StudioBone*>(std::uintptr_t(this) + boneIndex) + i : nullptr;
    }
	StudioHitboxSet* pHitboxSet(int i) const
	{
		(i >= 0 && i < numHitboxSets);
		return (StudioHitboxSet*)(((unsigned char*)this) + hitboxSetIndex) + i;
	};
    StudioHitboxSet* getHitboxSet(int i) noexcept
    {
        return i >= 0 && i < numHitboxSets ? reinterpret_cast<StudioHitboxSet*>(std::uintptr_t(this) + hitboxSetIndex) + i : nullptr;
    }
    StudioBbox* pHitbox(int i, int set) const
    {
        StudioHitboxSet const* s = pHitboxSet(set);
        if (!s)
            return nullptr;

        return s->pHitbox(i);
    };

};

struct Model;

class ModelInfo {
public:
    INCONSTRUCTIBLE(ModelInfo)

    VIRTUAL_METHOD(int, getModelIndex, WIN32_LINUX(2, 3), (const char* name), (this, name))
    VIRTUAL_METHOD(studiohdr_t*, getStudioModel, WIN32_LINUX(32, 31), (const Model* model), (this, model))
};
