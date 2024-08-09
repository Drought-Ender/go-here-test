#include "Drought/Screen/AlteredMapMenu.h"
#include "Game/Navi.h"
#include "Game/NaviState.h"
#include "Drought/Game/NaviGoHere.h"
#include "Drought/Misc.h"
#include "Game/Cave/RandMapMgr.h"
#include "LoadResource.h"
#include "Game/Piki.h"
#include "Game/CPlate.h"
#include "JSystem/J2D/J2DPrint.h"

// included in assets folder if you've downloaded this
namespace
{
	const char* cAButtonPath = "/user/Drought/GoHere/a_btn.bti";
	const char* cBlueArrowPath = "/user/Drought/GoHere/arrow.bti";
	const char* cRedArrowPath = "/user/Drought/GoHere/arrow_red.bti";
} // namespace

namespace Drought
{

namespace Screen
{

// sets whether you can rotate the map with shoulder L/R
const bool cAllowMapRotation = false;



AlteredMapMenu::AlteredMapMenu(const char* name) : og::newScreen::ObjSMenuMap(name)
{
	mStartWPIndex = -1;
}

void AlteredMapMenu::doCreate(JKRArchive* rarc) {
	mAllPikisBlue = false;
	mCanStartPathfind = false;
	mHasNoPath = false;
	mPathfindBlue = true;
	mGoalWPIndex = -1;
	mStartPathFindCounter = 0;
	mPathfindState = PATHFIND_INACTIVE;

	og::newScreen::ObjSMenuMap::doCreate(rarc);

	LoadResource::Arg first (cAButtonPath);
	LoadResource::Arg second (cBlueArrowPath);
	LoadResource::Arg third (cRedArrowPath);

	LoadResource::Node* node1 = gLoadResourceMgr->load(first);
	LoadResource::Node* node2 = gLoadResourceMgr->load(second);
	LoadResource::Node* node3 = gLoadResourceMgr->load(third);
	mAButtonTex  = static_cast<ResTIMG*>(node1->mFile);
	mArrowTex    = static_cast<ResTIMG*>(node2->mFile);
	mArrowRedTex = static_cast<ResTIMG*>(node3->mFile);

	if (!mAButtonTex) JUT_PANIC("%s missing", cAButtonPath);
	if (!mArrowTex) JUT_PANIC("%s missing", cBlueArrowPath);
	if (!mArrowRedTex) JUT_PANIC("%s missing", cRedArrowPath);


	mArrowPicture    = new JUTTexture(mArrowTex);
	mArrowRedPicture = new JUTTexture(mArrowRedTex);
	mAButton         = new JUTTexture(mAButtonTex);
	// (this would set it to north)
	// mMapAngle = 0.0f;

}

void AlteredMapMenu::commonUpdate() {
	og::newScreen::ObjSMenuMap::commonUpdate();
}

/// @brief Converts a position on the map texture to its 3D coordinates
/// @param x Map-Object's x position on the radar map
/// @param y Map-Object's y position on the radar map
/// @return The position on the game map
Vector3f AlteredMapMenu::GetPositionFromTex(f32 x, f32 y) {

	
	Vector3f inPos = Vector3f(x, y, 0.0f);

	Mtx inverse;

	PSMTXInverse(mMapTexPane->mGlobalMtx, inverse);

	Vector3f outPos;

	PSMTXMultVec(inverse, (Vec*)&inPos, (Vec*)&outPos); 

	Vector2f oldOrigin = Vector2f(outPos.x, outPos.y);

	Vector2f cPos;

	if (mDisp->mInCave) {
		cPos.y = (oldOrigin.y + 0.6f) / 0.047f;
		cPos.x = (oldOrigin.x + 0.2f) / 0.047f;
	}
	else {
		cPos.y = (oldOrigin.y + 8.85f - mMapTextureDimensions.y * 0.5f) / 0.058f;
		cPos.x = (oldOrigin.x - 24.5f - mMapTextureDimensions.x * 0.5f) / 0.058f;
		if (mDisp->mCourseIndex == 3) {
			cPos.x -= (mMapBounds.x * 1400.0f) / 4705.6f;
		}
	}

	Vector3f vec2 (cPos.x, 0.0f, cPos.y);
	vec2.y = Game::mapMgr->getMinY(vec2);
	return vec2;
	
}


/// @brief Converts a position on the game map to a point on the radar map
/// @param pos The point on the radar map
/// @return Object's position on the radar map
Vector2f AlteredMapMenu::GetPositionOnTex(Vector3f& pos) {
	Vector2f mapPosition (0.0f, 0.0f);

	if (mDisp->mInCave) {
		mapPosition.y = pos.z * 0.047f + -0.6f;
		mapPosition.x = pos.x * 0.047f + -0.2f;
	} else {
		if (mDisp->mCourseIndex == 3) {
			mapPosition.x = (mMapBounds.x * 1400.0f) / 4705.6f;
		}
		mapPosition.x += mMapTextureDimensions.x * 0.5f + pos.x * 0.058f + 24.5f;
		mapPosition.y = mMapTextureDimensions.y * 0.5f + pos.z * 0.058f + -8.85f;
		
	}

	Vector2f vec = mapPosition;	

	Vector3f testVec = Vector3f(mapPosition.x, mapPosition.y, 0.0f);

	Vector3f outVec;

	PSMTXMultVec(mMapTexPane->mGlobalMtx, (Vec*)&testVec, (Vec*)&outVec);

	return Vector2f(outVec.x, outVec.y);
}

bool AlteredMapMenu::CheckCanStartPathfind(Game::Navi* navi) {
	int stateID = navi->getStateID();

	switch (stateID)
	{
	case Game::NSID_Walk:
	case Game::NSID_GoHere:
	case Game::NSID_PathMove:
	case Game::NSID_Punch:
	case Game::NSID_CarryBomb:
	case Game::NSID_ThrowWait:
	case Game::NSID_Throw:
		return true;
	default: 
		return false;
	}
}

bool AlteredMapMenu::doStart(::Screen::StartSceneArg const* arg) {
	mAllPikisBlue = Game::CheckAllPikisBlue(Game::naviMgr->getActiveNavi());
	mCanStartPathfind = CheckCanStartPathfind(Game::naviMgr->getActiveNavi());
	return og::newScreen::ObjSMenuMap::doStart(arg);
}

bool AlteredMapMenu::doEnd(::Screen::EndSceneArg const* arg) {
	PathfindCleanup();
	NodeCleanup();

	return og::newScreen::ObjSMenuMap::doEnd(arg);
}

void AlteredMapMenu::NodeCleanup() {

}

void AlteredMapMenu::drawArrow(Graphics& gfx) {

	if (mPathfindBlue) {
		mArrowPicture->load(GX_TEXMAP0);
	}
	else {
		mArrowRedPicture->load(GX_TEXMAP0);
	}

	Vector2f center;
	og::Screen::calcGlbCenter(mPane_map, &center);

	const Rectf cArrowDrawBox = Rectf(-12.0f, -24.0f, 12.0f, 0.0f);
	

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(cArrowDrawBox.p1.x + center.x, cArrowDrawBox.p1.y + center.y, 0.0f);
    GXPosition2f32(0.0f, 0.0f);
    GXPosition3f32(cArrowDrawBox.p2.x + center.x, cArrowDrawBox.p1.y + center.y, 0.0f);
    GXPosition2f32(1.0f, 0.0f);
    GXPosition3f32(cArrowDrawBox.p2.x + center.x, cArrowDrawBox.p2.y + center.y, 0.0f);
    GXPosition2f32(1.0f, 1.0f);
    GXPosition3f32(cArrowDrawBox.p1.x + center.x, cArrowDrawBox.p2.y + center.y, 0.0f);
    GXPosition2f32(0.0f, 1.0f);
    GXEnd();
}

void AlteredMapMenu::drawButton(Graphics& gfx) {

	if (!mCanStartPathfind || mPathfindState != PATHFIND_DONE || mHasNoPath) {
		return;
	}

	mAButton->load(GX_TEXMAP0);

	Vector2f center;
	og::Screen::calcGlbCenter(mPane_map, &center);

	const Rectf cButtonDrawBox = Rectf(4.0f, -32.0f, 20.0f, -16.0f);
	

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(cButtonDrawBox.p1.x + center.x, cButtonDrawBox.p1.y + center.y, 0.0f);
    GXPosition2f32(0.0f, 0.0f);
    GXPosition3f32(cButtonDrawBox.p2.x + center.x, cButtonDrawBox.p1.y + center.y, 0.0f);
    GXPosition2f32(1.0f, 0.0f);
    GXPosition3f32(cButtonDrawBox.p2.x + center.x, cButtonDrawBox.p2.y + center.y, 0.0f);
    GXPosition2f32(1.0f, 1.0f);
    GXPosition3f32(cButtonDrawBox.p1.x + center.x, cButtonDrawBox.p2.y + center.y, 0.0f);
    GXPosition2f32(0.0f, 1.0f);
    GXEnd();
}

void AlteredMapMenu::setupTextureDraw(Graphics& gfx) {
	GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_POS_XYZ, GX_F32, 0);

    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);

    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_TEXCOORD0, 0x3c, 0, 0x7d);

    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	
	GXLoadPosMtxImm(gfx.mPerspGraph.mPosMtx, 0);
}

void AlteredMapMenu::doDraw(Graphics& gfx)
{
	J2DPerspGraph* graf = &gfx.mPerspGraph;
	drawMap(gfx);

	drawPath(gfx);

	Graphics gfx2;
	
	mIconScreen->draw(gfx2, *graf);
	
	
	setupTextureDraw(gfx);
	drawArrow(gfx);
	drawButton(gfx);
	

	if (mCompassPic && mPane_Ncompas) {
		PSMTXCopy(mPane_Ncompas->mGlobalMtx, mCompassPic->mPositionMtx);
	}

	graf->setPort();
	mIconScreen2->draw(gfx, *graf);
	graf->setPort();
	drawYaji(gfx);
}

bool AlteredMapMenu::doUpdate() {
	PathfindUpdate();

	if (cAllowMapRotation) { 
		if (mController->mPadButton->mAnalogR > 0.0f) {
			mMapAngle += 90.0f * sys->mDeltaTime * mController->mPadButton->mAnalogR;
		}
		if (mController->mPadButton->mAnalogL > 0.0f) {
			mMapAngle -= 90.0f * sys->mDeltaTime * mController->mPadButton->mAnalogL;
		}
	}

	og::newScreen::SceneSMenuBase* scene = static_cast<og::newScreen::SceneSMenuBase*>(getOwner());

	// allows the pathfind process to start if everything is setup

	bool ret = false;

	if (mCanStartPathfind && !mHasNoPath && mPathfindState == PATHFIND_DONE && scene->getGamePad()->mButton.mButtonDown & (Controller::PRESS_A)) {

		Vector2f center;
		og::Screen::calcGlbCenter(mPane_map, &center);

		Vector3f pos = GetPositionFromTex(center.x, center.y);

		P2ASSERT(mPath);

		Game::NaviGoHereStateArg arg(pos, mPath);

		mPath = nullptr; // we no longer own this, it's the navistate's

		Game::naviMgr->getActiveNavi()->transit(Game::NSID_GoHere, &arg);

		mCancelToState = MENUCLOSE_Finish;
		doUpdateCancelAction();
		mPathfindState = PATHFIND_GOHERE;
		ret = true;
	}

	return og::newScreen::ObjSMenuMap::doUpdate() || ret;
}

enum EExecPathfindingSuccess {
	PATHFINDSTATUS_FAIL = -1,
	PATHFINDSTATUS_OK = 0
};

bool AlteredMapMenu::CheckMapMove() {
	int buttons  = mController->getButton();
	f32 inputx   = mController->getMainStickX();
	f32 inputz   = mController->getMainStickY();

	if (buttons & JUTGamePad::PRESS_DPAD_UP) {
		inputz = 1.0f;
	}
	if (buttons & JUTGamePad::PRESS_DPAD_DOWN) {
		inputz = -1.0f;
	}
	if (buttons & JUTGamePad::PRESS_DPAD_LEFT) {
		inputx = -1.0f;
	}
	if (buttons & JUTGamePad::PRESS_DPAD_RIGHT) {
		inputx = 1.0f;
	}

	return ((inputx < -0.1f || inputx > 0.1f) || (inputz < -0.1f || inputz > 0.1f));
}

void AlteredMapMenu::PathfindUpdate() {
	switch (mPathfindState) {
		case PATHFIND_INACTIVE:
			mStartWPIndex = -1;
			mGoalWPIndex = -1;
			mStartPathFindCounter = 0;
			mPathfindBlue = true;
			initPathfinding(false);
			break;
		case PATHFIND_AWAITING:
			int status = execPathfinding();
			if (status == PATHFINDSTATUS_FAIL) {
				JUT_PANIC("Something went wrong lmao\n");
			}
			// no break is intentional
		case PATHFIND_DONE:
			if (CheckMapMove()) {
				mPathfindState = PATHFIND_INACTIVE;
			}
			break;
	}
	
}

void AlteredMapMenu::OnPathfindDone() {
	if (mCanStartPathfind && !mHasNoPath) {
		PSSystem::spSysIF->playSystemSe(PSSE_MP_SHIP_CALLING_01, 0);
	}
	else {
		PSSystem::spSysIF->playSystemSe(PSSE_SY_MENU_ERROR, 0);
	}
}

// setup our pathfinder and set our start and end positions
void AlteredMapMenu::initPathfinding(bool resetLinkCount) {

	mPath      = nullptr;
	mHasNoPath = false;

	Game::Navi* movingNavi = Game::naviMgr->getActiveNavi();
	P2ASSERT(movingNavi);
	P2ASSERT(Game::mapMgr);

	Vector3f naviPos = movingNavi->getPosition();
	Game::WPEdgeSearchArg searchArg(naviPos);
	s16 roomIndex    = movingNavi->mRoomIndex;
	

	if (Game::gameSystem->mIsInCave) {
		Sys::Sphere sphere;
		sphere.mPosition = naviPos;
		sphere.mRadius   = 1.0f;

		roomIndex = static_cast<Game::RoomMapMgr*>(Game::mapMgr)->findRoomIndex(sphere);
	}

	searchArg.mRoomID = roomIndex;
	searchArg.mLinks  = nullptr;

	Game::WayPoint* startWP = nullptr;
	if (Game::mapMgr->mRouteMgr->getNearestEdge(searchArg)) {
		if (searchArg.mWp1->isFlag(Game::WPF_Closed)) {
			startWP = searchArg.mWp2;
		} else {
			startWP = searchArg.mWp1;
		}
	} else {
		JUT_PANIC("zannen !\n");
	}
	

	JUT_ASSERT(startWP, "start==0");

	mStartWPIndex = startWP->mIndex;

	Vector2f center;
	og::Screen::calcGlbCenter(mPane_map, &center);
	Vector3f goalPos = GetPositionFromTex(center.x, center.y);

	if (!Drought::hasValidFloor(goalPos)) {
		mPathfindBlue = false;
		mPathfindState = PATHFIND_INACTIVE;
		OnPathfindDone();
		return;
	}

	Game::WPSearchArg searchArg2(goalPos, nullptr, false, 100.0f);
	Game::WayPoint* endWP = Game::mapMgr->mRouteMgr->getNearestWayPoint(searchArg2);
	JUT_ASSERT(endWP, "endWP=0");

	mGoalWPIndex = endWP->mIndex;

	mStartPathFindCounter = 0;


	mPathfindState = PATHFIND_AWAITING;
}

// Check on the pathfinder and update if done
int AlteredMapMenu::execPathfinding() {


	u8 flag = 0;
	mPathfindBlue = true;

	if (mStartPathFindCounter != 1 && mStartPathFindCounter < 3) {
		flag |= Game::PATHFLAG_Unk1 | Game::PATHFLAG_Unk3;
	}

	if (mAllPikisBlue || (mStartPathFindCounter >= 2 && Game::cTryRouteWater)) {
		flag |= Game::PATHFLAG_PathThroughWater;
	}

	if (mStartPathFindCounter >= 1) {
		mPathfindBlue = false;
	}

	mPath = new Drought::Path;
	mWayPointCount = Drought::Pathfinder::search_fast(mStartWPIndex, mGoalWPIndex, *mPath, flag);

	if (mWayPointCount > 0) {
		mPathfindState = PATHFIND_DONE;
		OnPathfindDone();
		return PATHFINDSTATUS_OK;
	}

	mStartPathFindCounter++;

	delete mPath;
	mPath = nullptr;

	// gives up
	if (mStartPathFindCounter >= 5) {
		mHasNoPath        = true;
		mPathfindBlue     = false;
		mPathfindState    = PATHFIND_DONE;
		OnPathfindDone();
		return PATHFINDSTATUS_OK;
	}

}

// draw the path of the Go-Here route on the 2D map
void AlteredMapMenu::drawPath(Graphics& gfx) {

	if (mPathfindState != PATHFIND_DONE || !mPath) {
		return;
	}

	J2DPerspGraph* graf = &gfx.mPerspGraph;

	const JUtility::TColor color1 = 0xffffffff; // white
	const JUtility::TColor color2 = 0xffaaaaff; // pinkish-red

	bool isImpossible = false;
	
	Vector3f naviPos = Game::naviMgr->getActiveNavi()->getPosition();
	
	Vector2f goHerePtr;
	og::Screen::calcGlbCenter(mPane_map, &goHerePtr);

	// probably unnecessary, but just in case
	const u8 oldWidth = graf->mLineWidth;

	graf->setPort();
	// GX Voodoo
	GXSetZCompLoc(GX_TRUE);
	GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);

	graf->setLineWidth(8);
	graf->setColor(color1);
	
	if (!mCanStartPathfind) {
		graf->setColor(color2);
		isImpossible = true;
	}

	JGeometry::TVec2f naviFirst = GetPositionOnTex(naviPos);

	Vector3f previousPos = naviPos;

	graf->moveTo(naviFirst);

	

	FOREACH_NODE(Drought::PathNode, mPath->mRoot, node) {
		Game::WayPoint* wp = Game::mapMgr->mRouteMgr->getWayPoint(node->mWpIdx);
		Vector3f currPos = (wp->getPosition());

		// Vector3f diffVec = currPos - previousPos;
		// f32 magnitude = diffVec.normalise2D();

		// currPos -= diffVec * wp->mRadius;

		JGeometry::TVec2f point = GetPositionOnTex(currPos);

		
		graf->lineTo(point);
		
		// if our route fails, set our color to red
		if (wp->isFlag(Game::WPF_Closed) || (wp->isFlag(Game::WPF_Water) && !mAllPikisBlue)) {
			isImpossible = true;
			graf->setColor(color2);
		}

		previousPos = currPos;
	}


	graf->lineTo(goHerePtr);

	graf->setLineWidth(oldWidth);

	if (isImpossible) {
		mPathfindBlue = false;
	}

	graf->setPort();
	// More GX Voodoo
	GXSetZCompLoc(GX_TRUE);
	GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);

}

// Ensures we're not leaving our pathfinder with a dead task
void AlteredMapMenu::PathfindCleanup() {
	if (mPathfindState != PATHFIND_GOHERE) {
		// Game::testPathfinder->release(mContextHandle);
	}

	if (mPath) {
		delete mPath;
		mPath = nullptr;
	}
}

} // namespace Screen


} // namespace Drought
