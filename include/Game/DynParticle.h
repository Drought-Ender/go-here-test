#ifndef _GAME_DYNPARTICLE_H
#define _GAME_DYNPARTICLE_H

#include "MonoObjectMgr.h"
#include "Vector3.h"

struct Matrixf;

namespace Game {
/**
 * @fabricatedName
 */
struct _DynParticleParent {
	Vector3f _00;              // _00
	Vector3f _0C;              // _0C
	f32 _18;                   // _18
	_DynParticleParent* mNext; // _1C
	Vector3f _20;              // _20
	u8 _2C;                    // _2C
};

/**
 * @size{0x34}
 * TODO: I suspect there's a common erased base interface that defines this
 * vtable.
 */
struct DynParticle : public _DynParticleParent {
	DynParticle()
	{
		_18   = 1.0f;
		_2C   = 0;
		mNext = nullptr;
		_20   = Vector3f(0.0f);
	}

	virtual void constructor() { }               // _08 (weak)
	virtual void doAnimation() { }               // _0C (weak)
	virtual void doEntry() { }                   // _10 (weak)
	virtual void doSetView(u32) { }              // _14 (weak)
	virtual void doViewCalc() { }                // _18 (weak)
	virtual void doSimulation(f32) { }           // _1C (weak)
	virtual void doDirectDraw(Graphics& gfx) { } // _20 (weak)

	DynParticle* getAt(int);

	// Unused/inlined:
	void release();
	void updateGlobal(Matrixf&);
};

struct DynParticleMgr : public MonoObjectMgr<DynParticle> {
	DynParticleMgr(int);

	// vtable 1
	// virtual ~DynParticleMgr(); // _08 (weak)
	// vtable 2
	virtual void resetMgr(); // _80 (weak)

	// _00     = VTBL
	// _00-_30 = MonoObjectMgr
};

extern DynParticleMgr* dynParticleMgr;
} // namespace Game

#endif
