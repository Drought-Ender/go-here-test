#ifndef _DROUGHT_GAME_NAVIGOHERE_H_
#define _DROUGHT_GAME_NAVIGOHERE_H_

#include "Game/Navi.h"
#include "Game/NaviState.h"
#include "Drought/Pathfinder.h"

namespace Game
{

/* setting this to true will have the navi still
 * attempt to route through water if it 
 * has non-blue pikmin */
const bool cTryRouteWater = false;

bool CheckAllPikisBlue(Game::Navi* navi);

struct NaviGoHereStateArg : public StateArg
{
    inline NaviGoHereStateArg(Vector3f pos, Drought::Path* path) 
        : mPosition(pos)
        , mPath(path)
    {
    }

    Vector3f mPosition;
    Drought::Path* mPath;
};


struct NaviGoHereState : public NaviState {
	inline NaviGoHereState()
	    : NaviState(NSID_GoHere)
	{
	}

	virtual void init(Navi*, StateArg*); // _08
	virtual void exec(Navi*);            // _0C
	virtual void cleanup(Navi*);         // _10
	virtual bool callable() { return true; }

	bool execMove(Navi*);
	bool execMoveGoal(Navi*);

	// _00     = VTBL
	// _00-_10 = NaviState
	Vector3f mPosition;       // _14
	Drought::PathNode* mCurrNode;
	Drought::Path* mPath;
};
    
} // namespace Game


#endif