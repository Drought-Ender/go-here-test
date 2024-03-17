#ifndef _EBI_TITLE_ENTITIES_TPIKMIN_H
#define _EBI_TITLE_ENTITIES_TPIKMIN_H

#include "CNode.h"
#include "Vector2.h"
#include "ebi/Geometry.h"
#include "ebi/E3DGraph.h"
#include "ebi/title/TObjects.h"
#include "ebi/title/TCoordMgr.h"
#include "Parameters.h"
#include "BaseParm.h"
#include "JSystem/J3D/J3DFrameCtrl.h"

struct JKRArchive;

namespace ebi {
namespace title {
struct TObjBase;

namespace Pikmin {
#define TITLE_PIKI_TOTAL (TITLE_PIKI_COLOR_COUNT * TITLE_PIKI_TYPE_MAX) // 500 by default

struct TMgr;

struct TParam : public TParamBase {
	/**
	 * @note Address: 0x803E6F10
	 * @note Size: 0x418
	 */
	TParam()
	    : mIntScale(this, 'pk00', "�C�O�ŃX�P�[��", 2.4f, 0.0f, 10.0f)                      // 'overseas version scale'
	    , mJpnScale(this, 'pk99', "���{��ŃX�P�[��", 2.6f, 0.0f, 10.0f)                    // 'Japanese version scale'
	    , mCollRadius(this, 'pk01', "�R���W�������a", 5.0f, 0.0f, 100.0f)                   // 'collision radius'
	    , mStopDist(this, 'pk02', "��~����", 20.0f, 0.0f, 100.0f)                          // 'stopping distance'
	    , mConvDist(this, 'pk03', "��������", 50.0f, 0.0f, 100.0f)                          // 'convergence distance'
	    , mShadowX(this, 'pk05', "�e���炵X", 5.0f, 0.0f, 100.0f)                           // 'shadow offset X'
	    , mShadowZ(this, 'pk06', "�e���炵Z", 5.0f, 0.0f, 100.0f)                           // 'shadow offset Z'
	    , mAnimSpeedWalk(this, 'pk07', "�A�j���X�s�[�h�����i���x���j", 0.5f, 0.0f, 10.0f) // 'anime speed walking (speed proportional)'
	    , mAnimSpeedStyle(this, 'pk08', "�A�j���X�s�[�h��", 0.5f, 0.0f, 10.0f)              // 'anime speed wind'
	    , mAnimMaxWaitTime(this, 'pk10', "�A�j���X�s�[�hWAIT�ő�", 0.5f, 0.0f, 10.0f)       // 'anime speed WAIT max'
	    , mAnimMinWaitTime(this, 'pk09', "�A�j���X�s�[�hWAIT�ŏ�", 0.0f, 0.0f, 10.0f)       // 'anime speed WAIT min'
	    , mKogane(this, 'pk12', "�R�K�l�D���D���W��", 5.0f, -10.0f, 10.0f)                  // 'kogane attraction factor'
	    , mChappyRun(this, 'pk13', "�`���b�s�[���瓦����W��", -5.0f, -10.0f, 10.0f)        // 'chappy repulsion factor'
	    , mChaseGiveUp(this, 'pk14', "�ǂ�����������ߔ��a", 400.0f, 0.0f, 500.0f)          // 'chasing radius'
	    , mWindTimer(this, 'pk11', "���^�C�}�[(�b)", 6.0f, 0.0f, 10.0f)                     // 'wind timer (sec)'
	    , mDistSpeedFactor(this, 'pk04', "������ᑬ�x�W��", 0.2f, 0.0f, 1.0f)              // 'distance proportional speed factor'
	{
	}

	// _00-_0C = TParamBase
	Parm<f32> mIntScale;        // _0C, pk00
	Parm<f32> mJpnScale;        // _34, pk99
	Parm<f32> mCollRadius;      // _5C, pk01
	Parm<f32> mStopDist;        // _84, pk02
	Parm<f32> mConvDist;        // _AC, pk03
	Parm<f32> mShadowX;         // _D4, pk05
	Parm<f32> mShadowZ;         // _FC, pk06
	Parm<f32> mAnimSpeedWalk;   // _124, pk07
	Parm<f32> mAnimSpeedStyle;  // _14C, pk08
	Parm<f32> mAnimMaxWaitTime; // _174, pk10
	Parm<f32> mAnimMinWaitTime; // _19C, pk09
	Parm<f32> mKogane;          // _1C4, pk12
	Parm<f32> mChappyRun;       // _1EC, pk13
	Parm<f32> mChaseGiveUp;     // _214, pk14
	Parm<f32> mWindTimer;       // _23C, pk11
	Parm<f32> mDistSpeedFactor; // _264, pk04
};

struct TAnimFolder : public E3DAnimFolderBase {
	virtual E3DAnimRes* getAnimRes(s32); // _08 (weak)

	void load(J3DModelData*, JKRArchive*);
	E3DAnimRes* getAnimRes(int);

	// _00 = VTBL
	E3DAnimRes mAnims[4]; // _04 - move, wait, attack, wait2
};

struct TAnimator {
	TAnimator();

	void setArchive(JKRArchive*);
	J3DModel* newJ3DModel(s32 color);
	void setAnmWave(J3DModel*, f32, f32, f32);
	void setAnmWait(J3DModel*, f32);

	J3DModelData* mModelDataRed;    // _00
	J3DModelData* mModelDataYellow; // _04
	J3DModelData* mModelDataBlue;   // _08
	J3DModelData* mModelDataPurple; // _0C
	J3DModelData* mModelDataWhite;  // _10
	J3DAnmTransform* _14;           // _14
	J3DAnmTransform* _18;           // _18
	J3DMtxCalcAnmBase* _1C;         // _1C
	J3DMtxCalc* _20;                // _20
};

struct TBoidParam : public Parameters {
	TBoidParam()
	    : Parameters(nullptr, "TTitleStateParameters")
	    , mTurnMag(this, 'tsp0', "�����ς��x�N�g���̑傫��", 0.2f, 0.0f, 10.0f)        // 'turn vector magnitude'
	    , mMaxTurnVec(this, 'tsp1', "�����ς��x�N�g���ő�", 0.4f, 0.0f, 10.0f)         // 'turn vector maximum'
	    , mMaxWalkSpeed(this, 'tsp2', "�ő�������x", 2.0f, 0.0f, 10.0f)               // 'max walking speed'
	    , mBoidColl(this, 'tsp5', "BOID�Փˉ���W��", 800.0f, 0.0f, 10000.0f)          // 'BOID collision avoidance factor'
	    , mBoidSpeedMatch(this, 'tsp6', "BOID���x���킹�W��", 4.5f, 0.0f, 100.0f)      // 'BOID speed adjust factor'
	    , mBoidCenter(this, 'tsp7', "BOID���S�W���W��", 0.005f, 0.0f, 10.0f)           // 'BOID center set factor'
	    , mBoidNeighbor(this, 'tsp8', "BOID�̂̋ߏ��T�[�`���a", 30.0f, 0.0f, 500.0f) // 'BOID neighbour search radius'
	    , mGroupCenter(this, 'tsp9', "�Q�̒��S�ƌ̖̂ړI�n�̊���", 0.0f, 0.0f, 1.0f) // 'group center to individual dest ratio'
	{
	}

	// _00-_0C = Parameters
	Parm<f32> mTurnMag;        // _0C, tsp0
	Parm<f32> mMaxTurnVec;     // _34, tsp1
	Parm<f32> mMaxWalkSpeed;   // _5C, tsp2
	Parm<f32> mBoidColl;       // _84, tsp5
	Parm<f32> mBoidSpeedMatch; // _AC, tsp6
	Parm<f32> mBoidCenter;     // _D4, tsp7
	Parm<f32> mBoidNeighbor;   // _FC, tsp8
	Parm<f32> mGroupCenter;    // _124, tsp9
};

struct TBoidParamMgr : public CNode {

	TBoidParamMgr();

	virtual ~TBoidParamMgr() { } // _08 (weak)

	void update();

	// _00     = VTBL
	// _00-_18 = CNode
	u32 _18;               // _18
	u32 _1C;               // _1C
	TBoidParam mParams[5]; // _20
	f32 _6B0;              // _6B0
	f32 _6B4;              // _6B4
	f32 _6B8;              // _6B8
	f32 _6BC;              // _6BC
	f32 _6C0;              // _6C0
	f32 _6C4;              // _6C4
	f32 _6C8;              // _6C8
	f32 _6CC;              // _6CC
	u32 mCounter;          // _6D0
	u32 mCounter2;         // _6D4
};

// @size{0x98}
struct TUnit : public TObjBase {
	enum enumState {
		STATE_Hidden = 0, // dead/off-screen
		STATE_Unk1   = 1, // wait?
		STATE_Unk2   = 2, // walk?
		STATE_Unk3   = 3, // wind blow?
		STATE_Unk4   = 4, // boid?
		STATE_Unk5   = 5,
	};

	TUnit()
	{
		mCounter       = 0;
		mCounter2      = 0;
		mDestPos       = Vector2f(0.0f, 0.0f);
		mManager       = nullptr;
		_60            = Vector2f(1.0f, 1.0f);
		mEnemyObj      = nullptr;
		_6C            = Vector2f(0.0f);
		_74            = Vector2f(0.0f);
		_7C            = Vector2f(0.0f);
		mCurrentState  = STATE_Hidden;
		mPreviousState = STATE_Hidden;
		mIsDead        = false;
	}

	~TUnit() { }

	virtual u32 getCreatureType() { return TITLECREATURE_Pikmin; } // _08 (weak)
	virtual bool isCalc();                                         // _0C

	void init(TMgr*, s32);
	void goDestination();
	bool beAttacked();
	void alive();
	bool isAssemble();
	bool isWalk();
	void startState(enumState);
	void update();
	void updateSmoothWalk_(Vector2f&);
	void updateEnemyReaction_();

	// _00     = VTBL
	// _00-_2C = TObjBase
	Vector2f mDestPos;           // _2C
	TMgr* mManager;              // _34
	J3DFrameCtrl mFrameControlA; // _38
	J3DFrameCtrl mFrameControlB; // _4C
	Vector2f _60;                // _60
	TObjBase* mEnemyObj;         // _68
	Vector2f _6C;                // _6C
	Vector2f _74;                // _74
	Vector2f _7C;                // _7C
	enumState mCurrentState;     // _84
	enumState mPreviousState;    // _88
	u32 mCounter;                // _8C
	u32 mCounter2;               // _90
	bool mIsDead;                // _94
	bool _96[2];                 // _96
};

struct TMgr : public CNode {
	TMgr();

	virtual ~TMgr() { } // _08 (weak)

	void setArchive(JKRArchive*);
	void initUnit();
	void update();
	void setStartPos(Vector2f*);
	void setDestPos(Vector2f*);
	void forceArriveDest();
	void assemble();
	void quickAssemble();
	void startBoid1(f32);
	void startBoid2(f32);
	void startBoid3(f32);
	void startWindBlow(EGEBox2f&);
	void startDemo();
	void enemyPushOut(TObjBase*);
	void updateCalcBoid_();
	bool isAssemble();
	TUnit* getUnit(s32);

	// _00     = VTBL
	// _00-_18 = CNode
	TAnimator* mAnimator;        // _18
	TParam mParams;              // _1C
	TUnit* mUnits;               // _2AC
	TBoidParamMgr mBoidParamMgr; // _2B0
	Vector2f _988;               // _988
};
} // namespace Pikmin
} // namespace title
} // namespace ebi

#endif