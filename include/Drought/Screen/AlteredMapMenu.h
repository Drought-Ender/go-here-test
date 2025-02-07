#ifndef _DROUGHT_SCREEN_ALTEREDMAPMENU_H
#define _DROUGHT_SCREEN_ALTEREDMAPMENU_H

#include "og/Screen/ogScreen.h"
#include "og/newScreen/SMenu.h"
#include "Drought/Pathfinder.h"
#include "Game/routeMgr.h"
#include "Sys/Triangle.h"

namespace Drought {

namespace Screen {

struct GoHereMapMenu : public og::newScreen::ObjSMenuMap {
	GoHereMapMenu(const char* name)
	    : og::newScreen::ObjSMenuMap(name)
	{
		mStartIndex = -1;
	}

	virtual void doDraw(Graphics& gfx);
	virtual bool doUpdate();
	virtual bool doStart(const ::Screen::StartSceneArg*);
	virtual void doCreate(JKRArchive*);

	void PathfindUpdate();
	void OnPathfindDone();
	void RenderPath(Graphics& gfx);

	bool HasMapInputChanged();

	inline Vector2f GetTargetPosition2D()
	{
		Vector2f center;
		og::Screen::calcGlbCenter(mMapAreaPane, &center);
		return center;
	}

	inline Vector3f GetTargetPosition3D() { return GetPositionFromTex(GetTargetPosition2D()); }

	Vector3f GetPositionFromTex(const Vector2f& pos);
	Vector2f GetPositionOnTex(const Vector3f& pos);

	static bool IsPathfindingAllowed(Game::Navi* navi);

	void setupTextureDraw(Graphics& gfx);

	void drawArrow(Graphics& gfx);
	void drawButton(Graphics& gfx);

	void initPathfinding();
	void execPathfinding();
	bool validateDestinationReachable();

	enum PathfindState {
		PATHFIND_INACTIVE    = -1,
		PATHFIND_IN_PROGRESS = 0,
		PATHFIND_FINISHED    = 1,
		PATHFIND_GOHERE      = 2,
		PATHFIND_WAITING     = 3,
	};

	bool mCanStartPathfind;
	bool mFoundPath;
	bool mUseWaterNodes;
	s16 mStartIndex;
	s16 mDestinationIndex;
	PathfindState mPathfindState;
	Path mPath;

	ResTIMG* mAButtonTex;
	ResTIMG* mArrowTex;
	ResTIMG* mArrowRedTex;

	JUTTexture* mArrowPicture;
	JUTTexture* mArrowRedPicture;
	JUTTexture* mAButton;
};

} // namespace Screen

} // namespace Drought

#endif
