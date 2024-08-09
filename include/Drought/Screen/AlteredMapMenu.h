#ifndef _DROUGHT_SCREEN_ALTEREDMAPMENU_H
#define _DROUGHT_SCREEN_ALTEREDMAPMENU_H

#include "og/newScreen/SMenu.h"
#include "Game/routeMgr.h"
#include "Drought/Pathfinder.h"

namespace Drought
{

namespace Screen
{

struct AlteredMapMenu : public og::newScreen::ObjSMenuMap
{
    AlteredMapMenu(const char*);
    virtual void doDraw(Graphics& gfx);
    virtual bool doUpdate();
    virtual bool doStart(const ::Screen::StartSceneArg*);
    virtual void doCreate(JKRArchive*);
    virtual void commonUpdate();
    virtual bool doEnd(const ::Screen::EndSceneArg*);

    void PathfindUpdate();
    void OnPathfindDone();
    void PathfindCleanup();
    void drawPath(Graphics& gfx);

    bool CheckMapMove();

    void NodeCleanup();

    Vector3f GetPositionFromTex(f32 x, f32 y);
    Vector2f GetPositionOnTex(Vector3f& pos);

    static bool CheckCanStartPathfind(Game::Navi* navi);

    void setupTextureDraw(Graphics& gfx);

    void drawArrow(Graphics& gfx);
    void drawButton(Graphics& gfx);


    void initPathfinding(bool);
    int execPathfinding();

    enum PathfindState {
        PATHFIND_INACTIVE = -1,
        PATHFIND_AWAITING = 0,
        PATHFIND_DONE     = 1,
        PATHFIND_GOHERE   = 2
    };

    bool mCanStartPathfind;
    bool mHasNoPath;
    bool mPathfindBlue;
    bool mAllPikisBlue;
    s16 mStartWPIndex;
    s16 mGoalWPIndex;
    int mStartPathFindCounter;
    PathfindState mPathfindState;
    u32 mWayPointCount;
    Path* mPath;

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
