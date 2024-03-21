#ifndef _DROUGHT_GAME_NAVIGOHERE_H_
#define _DROUGHT_GAME_NAVIGOHERE_H_

#include "Game/Navi.h"
#include "Game/NaviState.h"

namespace Game
{

/* setting this to true will have the navi still
 * attempt to route through water if it 
 * has non-blue pikmin */
const bool cTryRouteWater = false;

bool CheckAllPikisBlue(Game::Navi* navi);

struct NaviGoHereStateArg : public StateArg
{
    inline NaviGoHereStateArg(Vector3f pos, u32 contextID, PathNode* nodes) 
        : mContextID(contextID),
        mPosition(pos),
        mNodes(nodes)
    {
    }

    u32 mContextID;
    Vector3f mPosition;
    PathNode* mNodes;
};


struct NaviGoHereState : public NaviState {
	inline NaviGoHereState()
	    : NaviState(NSID_GoHere)
	{
	}

	virtual void init(Navi*, StateArg*); // _08
	virtual void exec(Navi*);            // _0C
	virtual void cleanup(Navi*);         // _10

	bool execMove(Navi*);
	bool execMoveGoal(Navi*);

	/** @fabricated */
	inline void releasePathfinder()
	{
		if (mPathfinderContextID != 0) {
			testPathfinder->release(mPathfinderContextID);
			mPathfinderContextID = 0;
		}
	}

	// _00     = VTBL
	// _00-_10 = NaviState
	u32 mPathfinderContextID; // _10
	Vector3f mPosition;       // _14
	PathNode* mCurrNode;
};
    
} // namespace Game


#endif