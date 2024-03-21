#ifndef _DROUGHT_SCREEN_ALTEREDMAPMENU_H
#define _DROUGHT_SCREEN_ALTEREDMAPMENU_H

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


    void initPathfinding(bool);
    int execPathfinding();

    enum PathfindState {
        PATHFIND_INACTIVE = -1,
        PATHFIND_AWAITING = 0,
        PATHFIND_DONE     = 1,
        PATHFIND_GOHERE   = 2
    };

    bool mCanStartPathfind;
    bool mPathfindBlue;
    bool mAllPikisBlue;
    s16 mStartWPIndex;
    s16 mGoalWPIndex;
    u32 mContextHandle;
    int mStartPathFindCounter;
    int mPathFindCounter;
    Game::PathNode* mRootNode;
    PathfindState mPathfindState;
    int mWayPointCount;

    ResTIMG* mAButtonTex;
    ResTIMG* mArrowTex;
    ResTIMG* mArrowRedTex;

    J2DPictureEx* mArrowPicture;
    J2DPictureEx* mAButton;


    

    
};



} // namespace Screen



} // namespace Drought



#endif
