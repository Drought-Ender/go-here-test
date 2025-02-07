#ifndef _DROUGHT_GAME_NAVIGOHERE_H_
#define _DROUGHT_GAME_NAVIGOHERE_H_

#include "Game/Navi.h"
#include "Game/MapMgr.h"
#include "Game/NaviState.h"
#include "Drought/Pathfinder.h"

namespace Game {

bool AreAllPikisBlue(Game::Navi* navi);

struct NaviGoHereStateArg : public StateArg {
	inline NaviGoHereStateArg(Vector3f pos, Drought::Path path)
	    : mPosition(pos)
	    , mPath(path)
	{
	}

	Vector3f mPosition;
	Drought::Path mPath;
};

struct NaviGoHereState : public NaviState {
	inline NaviGoHereState()
	    : NaviState(NSID_GoHere)
	    , mFinishDistanceThreshold(15.0f)
	{
	}

	virtual void init(Navi*, StateArg*);
	virtual void exec(Navi*);
	virtual void cleanup(Navi*);
	virtual void collisionCallback(Navi*, CollEvent&);
	virtual bool callable() { return true; }

	void handlePlayerChangeFix(Navi*);

	bool handleControlStick(Navi*);   // True if update should stop
	bool navigateToFinalPoint(Navi*); // True if target reached, false if not
	void navigateToWayPoint(Navi*, Game::WayPoint*);
	void changeState(Navi* player, bool isWanted);

	inline Game::WayPoint* getCurrentWaypoint()
	{
		if (mActiveRouteNodeIndex == -1 || mActiveRouteNodeIndex >= mPath.mLength) {
			return nullptr;
		}

		return Game::getWaypointAt(mPath.mWaypointList[mActiveRouteNodeIndex]);
	}

	const f32 mFinishDistanceThreshold;

	f32 mTimeoutTimer;         // The time in seconds before the player gives up trying to get there
	s32 mActiveRouteNodeIndex; // The current waypoint we're trying to get to
	Vector3f mLastPosition;    // The last position the player was at
	Vector3f mTargetPosition;  // The position we're trying to get to
	Drought::Path mPath;       // The path we're trying to follow
};

} // namespace Game

#endif