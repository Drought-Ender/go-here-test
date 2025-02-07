#include "Drought/Game/NaviGoHere.h"
#include "Drought/Screen/AlteredMapMenu.h"
#include "Game/Cave/RandMapMgr.h"
#include "Game/CPlate.h"
#include "Game/MapMgr.h"
#include "Game/Navi.h"
#include "Game/NaviState.h"
#include "Game/Piki.h"
#include "JSystem/J2D/J2DPrint.h"
#include "LoadResource.h"

// included in assets folder if you've downloaded this
const char* cAButtonPath   = "/user/Drought/GoHere/a_btn.bti";
const char* cBlueArrowPath = "/user/Drought/GoHere/arrow.bti";
const char* cRedArrowPath  = "/user/Drought/GoHere/arrow_red.bti";

namespace Drought {

namespace Screen {
const bool cAllowMapRotation = false;

void GoHereMapMenu::doCreate(JKRArchive* rarc)
{
	mUseWaterNodes    = false;
	mCanStartPathfind = false;
	mFoundPath        = true;
	mDestinationIndex = -1;
	mPathfindState    = PATHFIND_INACTIVE;

	og::newScreen::ObjSMenuMap::doCreate(rarc);

	LoadResource::Arg first(cAButtonPath);
	LoadResource::Node* node1 = gLoadResourceMgr->load(first);
	mAButtonTex               = static_cast<ResTIMG*>(node1->mFile);
	if (!mAButtonTex) {
		JUT_PANIC("%s missing", cAButtonPath);
	}

	LoadResource::Arg second(cBlueArrowPath);
	LoadResource::Node* node2 = gLoadResourceMgr->load(second);
	mArrowTex                 = static_cast<ResTIMG*>(node2->mFile);
	if (!mArrowTex) {
		JUT_PANIC("%s missing", cBlueArrowPath);
	}

	LoadResource::Arg third(cRedArrowPath);
	LoadResource::Node* node3 = gLoadResourceMgr->load(third);
	mArrowRedTex              = static_cast<ResTIMG*>(node3->mFile);
	if (!mArrowRedTex) {
		JUT_PANIC("%s missing", cRedArrowPath);
	}

	mArrowPicture    = new JUTTexture(mArrowTex);
	mArrowRedPicture = new JUTTexture(mArrowRedTex);
	mAButton         = new JUTTexture(mAButtonTex);
}

Vector3f GoHereMapMenu::GetPositionFromTex(const Vector2f& pos)
{
	Vector3f inPos = Vector3f(pos.x, pos.y, 0.0f);

	Mtx inverse;
	PSMTXInverse(mMapTexPane->mGlobalMtx, inverse);

	Vector3f origin;
	PSMTXMultVec(inverse, (Vec*)&inPos, (Vec*)&origin);

	Vector2f cPos;
	if (mDisp->mInCave) {
		cPos.x = (origin.x + 0.2f) / 0.047f;
		cPos.y = (origin.y + 0.6f) / 0.047f;
	} else {
		cPos.x = (origin.x - 24.5f - mMapTextureDimensions.x * 0.5f) / 0.058f;
		cPos.y = (origin.y + 8.85f - mMapTextureDimensions.y * 0.5f) / 0.058f;
		if (mDisp->mCourseIndex == 3) {
			cPos.x -= (mMapBounds.x * 1400.0f) / 4705.6f;
		}
	}

	Vector3f finalPosition(cPos.x, 0.0f, cPos.y);
	finalPosition.y = Game::mapMgr->getMinY(finalPosition);
	return finalPosition;
}

Vector2f GoHereMapMenu::GetPositionOnTex(const Vector3f& pos)
{
	Vector2f mapPosition(0.0f, 0.0f);

	if (mDisp->mInCave) {
		mapPosition.x = pos.x * 0.047f + -0.2f;
		mapPosition.y = pos.z * 0.047f + -0.6f;
	} else {
		if (mDisp->mCourseIndex == 3) {
			mapPosition.x = (mMapBounds.x * 1400.0f) / 4705.6f;
		}

		mapPosition.y = mMapTextureDimensions.y * 0.5f + pos.z * 0.058f + -8.85f;
		mapPosition.x += mMapTextureDimensions.x * 0.5f + pos.x * 0.058f + 24.5f;
	}

	Vector3f testVec = Vector3f(mapPosition.x, mapPosition.y, 0.0f);
	Vector3f outVec;
	PSMTXMultVec(mMapTexPane->mGlobalMtx, (Vec*)&testVec, (Vec*)&outVec);

	return Vector2f(outVec.x, outVec.y);
}

bool GoHereMapMenu::IsPathfindingAllowed(Game::Navi* navi)
{
	int stateID = navi->getStateID();

	switch (stateID) {
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

bool GoHereMapMenu::doStart(::Screen::StartSceneArg const* arg)
{
	mUseWaterNodes    = Game::AreAllPikisBlue(Game::naviMgr->getActiveNavi());
	mCanStartPathfind = IsPathfindingAllowed(Game::naviMgr->getActiveNavi());
	return og::newScreen::ObjSMenuMap::doStart(arg);
}

void GoHereMapMenu::drawArrow(Graphics& gfx)
{
	if (mFoundPath) {
		mArrowPicture->load(GX_TEXMAP0);
	} else {
		mArrowRedPicture->load(GX_TEXMAP0);
	}

	const Rectf cArrowDrawBox = Rectf(-12.0f, -24.0f, 12.0f, 0.0f);
	Vector2f center           = GetTargetPosition2D();

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

void GoHereMapMenu::drawButton(Graphics& gfx)
{

	if (!mCanStartPathfind || mPathfindState != PATHFIND_FINISHED || !mFoundPath) {
		return;
	}

	mAButton->load(GX_TEXMAP0);

	const Rectf cButtonDrawBox = Rectf(4.0f, -32.0f, 20.0f, -16.0f);
	Vector2f center            = GetTargetPosition2D();

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

void GoHereMapMenu::setupTextureDraw(Graphics& gfx)
{
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_POS_XYZ, GX_F32, 0);

	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3X4, GX_TG_TEXCOORD0, 0x3c, GX_FALSE, 0x7d);

	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);

	GXLoadPosMtxImm(gfx.mPerspGraph.mPosMtx, 0);

	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 1, GX_DF_CLAMP, GX_AF_NONE);

	GXSetCullMode(GX_CULL_BACK);
	GXSetZMode(GX_FALSE, GX_NEVER, GX_FALSE);
}

void GoHereMapMenu::doDraw(Graphics& gfx)
{
	J2DPerspGraph* graf = &gfx.mPerspGraph;
	drawMap(gfx);

	Graphics gfx2;
	mIconScreen->hide();
	mIconScreen->draw(gfx2, *graf);
	mIconScreen->show();

	{
		J2DGrafContext tmpGraf = *graf;
		mMapTexPane->drawSelf(0.0f, 0.0f, &tmpGraf.mPosMtx);
	}

	graf->setPort();
	GXSetZCompLoc(GX_TRUE);
	GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);

	RenderPath(gfx);

	Graphics gfx3;
	JUtility::TColor clear(0xff, 0xff, 0xff, 0x00);
	mMapTexPane->setWhite(clear);
	mIconScreen->draw(gfx3, *graf);
	mMapTexPane->setWhite(TCOLOR_WHITE_U8);

	setupTextureDraw(gfx);
	drawArrow(gfx);
	drawButton(gfx);

	if (mCompassPic && mCompassPane) {
		PSMTXCopy(mCompassPane->mGlobalMtx, mCompassPic->mPositionMtx);
	}

	graf->setPort();
	mIconScreen2->draw(gfx, *graf);
	graf->setPort();
	drawYaji(gfx);
}

bool GoHereMapMenu::doUpdate()
{
	PathfindUpdate();

	og::newScreen::SceneSMenuBase* scene = static_cast<og::newScreen::SceneSMenuBase*>(getOwner());

	if (cAllowMapRotation) {
		if (mController->mPadButton->mAnalogR > 0.0f) {
			mMapAngle += 90.0f * sys->mDeltaTime * mController->mPadButton->mAnalogR;
		}
		if (mController->mPadButton->mAnalogL > 0.0f) {
			mMapAngle -= 90.0f * sys->mDeltaTime * mController->mPadButton->mAnalogL;
		}
	}

	// allows the pathfind process to start if everything is setup
	bool ret = false;
	if (mCanStartPathfind && mFoundPath && mPathfindState == PATHFIND_FINISHED
	    && scene->getGamePad()->mButton.mButtonDown & (Controller::PRESS_A)) {
		Vector3f pos = GetTargetPosition3D();

		Game::NaviGoHereStateArg arg(pos, mPath);
		Game::naviMgr->getActiveNavi()->transit(Game::NSID_GoHere, &arg);

		mCancelToState = MENUCLOSE_Finish;
		doUpdateCancelAction();
		mPathfindState = PATHFIND_GOHERE;
		ret            = true;
	}

	return og::newScreen::ObjSMenuMap::doUpdate() || ret;
}

bool GoHereMapMenu::HasMapInputChanged()
{
	int buttons = mController->getButton();
	f32 inputx  = mController->getMainStickX();
	f32 inputz  = mController->getMainStickY();

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

void GoHereMapMenu::PathfindUpdate()
{
	static f32 cooldown = 0.0f;

	switch (mPathfindState) {
	case PATHFIND_INACTIVE:
		initPathfinding();
		break;

	case PATHFIND_IN_PROGRESS:
		if (HasMapInputChanged()) {
			cooldown       = 0.1f;
			mPathfindState = PATHFIND_WAITING;
			break;
		}

		execPathfinding();
		break;
	case PATHFIND_FINISHED:
		if (HasMapInputChanged()) {
			mPathfindState = PATHFIND_INACTIVE;
		}
		break;
	case PATHFIND_WAITING:
		if (cooldown > 0.0f) {
			cooldown -= sys->mDeltaTime;
		} else {
			mPathfindState = PATHFIND_INACTIVE;
		}
		break;
	}
}

void GoHereMapMenu::OnPathfindDone()
{
	if (mCanStartPathfind && mFoundPath) {
		PSSystem::spSysIF->playSystemSe(PSSE_MP_SHIP_CALLING_01, 0);
	} else {
		PSSystem::spSysIF->playSystemSe(PSSE_SY_MENU_ERROR, 0);
	}
}

// setup our pathfinder and set our start and end positions
void GoHereMapMenu::initPathfinding()
{
	// Reset our pathfinding state
	mStartIndex       = 0;
	mDestinationIndex = 0;
	mFoundPath        = false;
	mPath.clear();

	// First let's calculate if the player wants to go to a valid polygon
	Vector3f goalPos = GetTargetPosition3D();

	// Is there a valid polygon there?
	if (!Game::isTriangleAt(goalPos)) {
		// No valid polygon, fuck
		mFoundPath     = false;
		mPathfindState = PATHFIND_INACTIVE;
		OnPathfindDone();
		return;
	}

	Game::Navi* player      = Game::naviMgr->getActiveNavi();
	Vector3f playerPosition = player->getPosition();

	s16 roomIndex = player->mRoomIndex;
	if (Game::gameSystem->mIsInCave) {
		Sys::Sphere sphere(playerPosition, 1.0f);
		roomIndex = static_cast<Game::RoomMapMgr*>(Game::mapMgr)->findRoomIndex(sphere);
	}

	Game::WPEdgeSearchArg searchArg(playerPosition);
	searchArg.mRoomID = roomIndex;
	searchArg.mLinks  = nullptr;

	// If there is water at the goalposition, we are in water!
	// Sys::Sphere waterSphere(goalPos, 1.0f);
	// searchArg.mInWater = Game::mapMgr->findWater(waterSphere) != nullptr;

	Game::WayPoint* startWP = nullptr;
	if (Game::mapMgr->mRouteMgr->getNearestEdge(searchArg)) {
		startWP = searchArg.mWp1->isFlag(Game::WPF_Closed) ? searchArg.mWp2 : searchArg.mWp1;
	} else {
		searchArg.mLinks = nullptr;
		if (Game::mapMgr->mRouteMgr->getNearestEdge(searchArg)) {
			startWP = searchArg.mWp1->isFlag(Game::WPF_Closed) ? searchArg.mWp2 : searchArg.mWp1;
		}
	}

	JUT_ASSERT(startWP, "No start waypoint found!");
	mStartIndex = startWP->mIndex;

	Game::WPSearchArg searchArg2(goalPos, nullptr, false, 100.0f);
	Game::WayPoint* endWP = Game::mapMgr->mRouteMgr->getNearestWayPoint(searchArg2);

	JUT_ASSERT(endWP, "No end waypoint found!");
	mDestinationIndex = endWP->mIndex;
	mPathfindState    = PATHFIND_IN_PROGRESS;
}

// Check on the pathfinder and update if done
void GoHereMapMenu::execPathfinding()
{
	// Routes are all waypoints in it are either:
	// - Open and not in water (unless all Pikis are blue)
	// - Goes over a FINISHED bridge
	u8 flag = Game::PATHFLAG_RequireOpen | Game::PATHFLAG_DisallowUnfinishedBridges | Game::PATHFLAG_AllowUnvisited;
	if (mUseWaterNodes) {
		flag |= Game::PATHFLAG_PathThroughWater;
	}

	mPath.clear();

	// Try end-to-start (works better for some cases)
	Drought::WaypointPathfinder::findPath(mDestinationIndex, mStartIndex, flag, mPath);
	if (!mPath.hasPath()) {
		// First try: no path, let's try the other way
		Drought::WaypointPathfinder::findPath(mStartIndex, mDestinationIndex, flag, mPath);
		if (!mPath.hasPath()) {
			// Second try: we searched front and back, but nothing, damn
			mFoundPath     = false;
			mPathfindState = PATHFIND_FINISHED;
			OnPathfindDone();
			return;
		}
	} else {
		// First try: we found a path, let's check if the destination is reachable
		mPath.reverse();
	}

	// We aren't just done, we need to check if the final destination is reachable
	if (validateDestinationReachable()) {
		mFoundPath = true;
	} else {
		// The destination is not reachable, so we can't do this
		mFoundPath = false;
		mPath.clear();
	}

	mPathfindState = PATHFIND_FINISHED;
	OnPathfindDone();
}

static bool checkSegment(const Vector3f& p0, const Vector3f& p1, f32 stepHeight, int maxDepth)
{
	// Check the height difference
	f32 dy = p1.y - p0.y;
	if (dy <= stepHeight) {
		return true;
	}

	// Bail out if we've recursed too far
	if (maxDepth <= 0) {
		return false;
	}

	// Check the midpoint
	Vector3f mid = Vector3f::middle(p0, p1);
	mid.y        = Game::mapMgr->getMinY(mid);

	// Check the two halves
	return checkSegment(p0, mid, stepHeight, maxDepth - 1) && checkSegment(mid, p1, stepHeight, maxDepth - 1);
}

bool GoHereMapMenu::validateDestinationReachable()
{
	// If no path exists, we can't do this
	if (!mPath.hasPath()) {
		return false;
	}

	// If the destination index is invalid, we can't do this
	Game::WayPoint* finalWp = Game::getWaypointAt(mDestinationIndex);
	if (!finalWp) {
		return false;
	}

	const f32 cMaxStepHeight     = 3.0f; // How high can we step up?
	const int cMaxRecursionDepth = 4;    // How many subdivisions can we check?

	// Get the starting and target positions and adjust their y-values to the terrain's minimum.
	Vector3f startPos = finalWp->getPosition();
	startPos.y        = Game::mapMgr->getMinY(startPos);

	// If the distance from the waypoint to the player is less than 10
	Vector3f playerPosition = Game::naviMgr->getActiveNavi()->getPosition();
	if (startPos.sqrDistance(playerPosition) < SQUARE(25.0f)) {
		// If the waypoint is more than cMaxStepHeight above the player, we can't do this
		if (startPos.y - playerPosition.y > cMaxStepHeight) {
			return false;
		}
	}

	// Get target position and adjust y.
	Vector3f targetPos = GetTargetPosition3D();
	targetPos.y        = Game::mapMgr->getMinY(targetPos);

	// Use adaptive sampling to check the vertical difference
	// If the height is too different between the points, recursively subdivide
	if (!checkSegment(startPos, targetPos, cMaxStepHeight, cMaxRecursionDepth)) {
		return false;
	}

	// The difference in Y wasn't too big, so we can do this
	return true;
}

// draw the path of the Go-Here route on the 2D map
void GoHereMapMenu::RenderPath(Graphics& gfx)
{
	if (mPathfindState != PATHFIND_FINISHED || !mCanStartPathfind || !mPath.hasPath()) {
		return;
	}

	J2DPerspGraph* graf = &gfx.mPerspGraph;

	// Red-ish for Olimar, Blue-ish for Louie
	const JUtility::TColor successOlimarCol(0xFF, 0xCC, 0xCC, 0xAA);
	const JUtility::TColor successLouieCol(0xCC, 0xCC, 0xFF, 0xAA);

	const u8 oldWidth = graf->mLineWidth;

	graf->setPort();
	graf->setLineWidth(8);
	GXSetZCompLoc(GX_TRUE);
	GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);

	Game::Navi* player = Game::naviMgr->getActiveNavi();
	graf->setColor(player->getNaviID() == NAVIID_Olimar ? successOlimarCol : successLouieCol);

	Vector3f playerPos = player->getPosition();
	Vector3f previous  = playerPos;
	graf->moveTo(GetPositionOnTex(playerPos));
	for (u16 i = 0; i < mPath.mLength; i++) {
		Vector3f current = Game::getWaypointAt(mPath.mWaypointList[i])->getPosition();
		graf->lineTo(GetPositionOnTex(current));

		previous = current;
	}

	Vector2f goHerePtr = GetTargetPosition2D();
	graf->lineTo(goHerePtr);

	graf->setLineWidth(oldWidth);
	graf->setPort();
	GXSetZCompLoc(GX_TRUE);
	GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
}

} // namespace Screen

} // namespace Drought
