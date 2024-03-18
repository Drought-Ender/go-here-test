#ifndef _OG_NEWSCREEN_ALTEREDMAPMENU_H
#define _OG_NEWSCREEN_ALTEREDMAPMENU_H

#include "og/newScreen/SMenu.h"
#include "Game/routeMgr.h"

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

    void PathfindUpdate();
    void OnPathfindDone();
    void drawPath(Graphics& gfx);

    bool CheckMapMove();

    Vector3f GetPositionFromTex(f32 x, f32 y);
    Vector2f GetPositionOnTex(Vector3f& pos, Vector2f& other);

    bool CheckAllPikisBlue(Game::Navi* navi);

    void initPathfinding(bool);
    int execPathfinding();

    enum PathfindState {
        PATHFIND_INACTIVE = -1,
        PATHFIND_AWAITING = 0,
        PATHFIND_DONE     = 1
    };

    WayPointLinks* mLinks;
    bool mAllPikisBlue;
    s16 mStartWPIndex;
    s16 mGoalWPIndex;
    u32 mContextHandle;
    int mStartPathFindCounter;
    int mPathFindCounter;
    Game::PathNode* mRootNode;
    PathfindState mPathfindState;
    int mWayPointCount;



    

    
};



} // namespace Screen



} // namespace Drought



#endif
