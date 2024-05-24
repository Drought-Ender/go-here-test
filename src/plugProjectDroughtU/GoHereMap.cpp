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
	mPathfindTexSetBlue = true;
	mPathfindBlue = true;
	mGoalWPIndex = -1;
	mContextHandle = 0;
	mStartPathFindCounter = 0;
	mPathFindCounter = 0;
	mRootNode = nullptr;
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


	mArrowPicture = og::Screen::CopyPictureToPane(mLouieArrow, mRootPane, 0.0f, 0.0f, 'go_here0');
	mAButton      = og::Screen::CopyPictureToPane(mLouieArrow, mRootPane, 0.0f, 0.0f, 'go_here1');

	mAButton->changeTexture(mAButtonTex, 0);
	mAButton->setAlpha(0);
	mAButton->mScale *= 0.5f;
	mArrowPicture->changeTexture(mArrowTex, 0);
	mArrowPicture->hide();
	mAButton->hide();

	// (this would set it to north)
	// mMapAngle = 0.0f;

}

void AlteredMapMenu::commonUpdate() {
	og::newScreen::ObjSMenuMap::commonUpdate();

	Vector2f center;

	og::Screen::calcGlbCenter(mPane_map, &center);

	if (mCanStartPathfind && mPathfindState == PATHFIND_DONE) {
		mAButton->setAlpha(255);
	}
	else {
		mAButton->setAlpha(0);
	}

	if (mPathfindBlue) {
		if (!mPathfindTexSetBlue) {
			mArrowPicture->changeTexture(mArrowTex, 0);
			mPathfindTexSetBlue = true;
		}
	}
	else {
		if (mPathfindTexSetBlue) {
			mArrowPicture->changeTexture(mArrowRedTex, 0);
			mPathfindTexSetBlue = false;
		}
	}


	mArrowPicture->setOffset(12.0f, -5.0f);
	mAButton->setOffset(20.0f, -20.0f);
}

/// @brief Converts a position on the map texture to its 3D coordinates
/// @param x Map-Object's x position on the radar map
/// @param y Map-Object's y position on the radar map
/// @return The position on the game map
Vector3f AlteredMapMenu::GetPositionFromTex(f32 x, f32 y) {

	Vector2f center;

	og::Screen::calcGlbCenter(mPane_map, &center);

	Vector2f vec (x - center.x, y - center.y);

	/* ~~~Magic Numbers, Inacuracy Here~~~ */
	vec.x /= mMapTexPane->mScale.x * 0.9f;
	vec.y /= mMapTexPane->mScale.y * 0.85f;


	f32 angle    = (mMapAngle * TAU) / 360.0f;
	f32 anglecos = cosf(angle);
	f32 anglesin = sinf(angle);

	Vector2f unrotated = Vector2f(
		vec.x * anglecos - vec.y * anglesin,
		vec.y * anglecos + vec.x * anglesin
	);


	Vector2f oldOrigin = unrotated - mMapPosition;

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

	Vector2f vec = mMapPosition + mapPosition;	

	f32 angle    = -(mMapAngle * TAU) / 360.0f;
	f32 anglecos = cosf(angle);
	f32 anglesin = sinf(angle);

	Vector2f rotated = Vector2f(
		vec.x * anglecos - vec.y * anglesin,
		vec.y * anglecos + vec.x * anglesin
	);

	/* ~~~Magic Numbers, Inacuracy Here~~~ */
	rotated.x *= mMapTexPane->mScale.x * 0.9f;
	rotated.y *= mMapTexPane->mScale.y * 0.85f;

	Vector2f center;

	og::Screen::calcGlbCenter(mPane_map, &center);

	return rotated + center;
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

void AlteredMapMenu::doDraw(Graphics& gfx)
{
	J2DPerspGraph* graf = &gfx.mPerspGraph;
	drawMap(gfx);

	drawPath(gfx);

	Graphics gfx2;
	
	mIconScreen->draw(gfx2, *graf);

	mArrowPicture->show();
	mAButton->show();

	mArrowPicture->J2DPane::draw(0.0f, 0.0f, graf, true, true);
	mAButton->J2DPane::draw(0.0f, 0.0f, graf, true, true);

	mArrowPicture->hide();
	mAButton->hide();

	

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

	if (mCanStartPathfind && mPathfindState == PATHFIND_DONE && scene->getGamePad()->mButton.mButtonDown & (Controller::PRESS_A)) {

		Vector2f center;
		og::Screen::calcGlbCenter(mPane_map, &center);

		Vector3f pos = GetPositionFromTex(center.x, center.y);

		Game::NaviGoHereStateArg arg(pos, mContextHandle, mRootNode);

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
			mPathFindCounter = 0;
			mRootNode = nullptr;
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
	PSSystem::spSysIF->playSystemSe(PSSE_MP_SHIP_CALLING_01, 0);
}

// setup our pathfinder and set our start and end positions
void AlteredMapMenu::initPathfinding(bool resetLinkCount) {
	if (resetLinkCount) {
	}
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
		if (mContextHandle) {
			Game::testPathfinder->release(mContextHandle);
		}
		
		return;
	}

	Game::WPSearchArg searchArg2(goalPos, nullptr, false, 100.0f);
	Game::WayPoint* endWP = Game::mapMgr->mRouteMgr->getNearestWayPoint(searchArg2);
	JUT_ASSERT(endWP, "endWP=0");

	mGoalWPIndex = endWP->mIndex;
	
	if (mContextHandle) {
		Game::testPathfinder->release(mContextHandle);
	}

	u8 flag = Game::PATHFLAG_Unk1 | Game::PATHFLAG_Unk3;

	if (mAllPikisBlue) {
		flag |= Game::PATHFLAG_PathThroughWater;
	}

	Game::PathfindRequest request(mStartWPIndex, mGoalWPIndex, flag);
	mContextHandle        = Game::testPathfinder->start(request);
	mStartPathFindCounter = 0;
	mPathFindCounter      = 0;
	mRootNode             = nullptr;


	mPathfindState = PATHFIND_AWAITING;
}

// Check on the pathfinder and update if done
int AlteredMapMenu::execPathfinding() {
	if (mContextHandle <= 0) {
		return PATHFINDSTATUS_FAIL;
	}
	mPathFindCounter++;

	switch (Game::testPathfinder->check(mContextHandle)) {
	case Game::PATHFIND_MakePath:
		mPathfindState = PATHFIND_DONE;
		mWayPointCount = Game::testPathfinder->makepath(mContextHandle, &mRootNode);
		if (mCanStartPathfind && mPathfindBlue) {
			OnPathfindDone();
		}
		break;

	case Game::PATHFIND_Start:
		if (mContextHandle) {
			Game::testPathfinder->release(mContextHandle);
		}

		u8 flag = Game::PATHFLAG_Unk3;

		if (mAllPikisBlue || (mStartPathFindCounter >= 1 && Game::cTryRouteWater)) {
			flag |= Game::PATHFLAG_PathThroughWater;
		}
		mStartPathFindCounter++;

		Game::PathfindRequest request(mStartWPIndex, mGoalWPIndex, flag);

		mContextHandle = Game::testPathfinder->start(request);

		mPathFindCounter = 0;
		return PATHFINDSTATUS_OK;

		case Game::PATHFIND_Busy:
		break;

	case Game::PATHFIND_NoHandle:
		JUT_PANIC("no handle %d\n", mContextHandle);
		break;
	}
	return PATHFINDSTATUS_OK;
}

// draw the path of the Go-Here route on the 2D map
void AlteredMapMenu::drawPath(Graphics& gfx) {

	if (mPathfindState != PATHFIND_DONE) {
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

	FOREACH_NODE(Game::PathNode, mRootNode, node) {
		Game::WayPoint* wp = Game::mapMgr->mRouteMgr->getWayPoint(node->mWpIndex);
		Vector3f currPos = (wp->getPosition());

		Vector3f diffVec = currPos - previousPos;
		f32 magnitude = diffVec.normalise2D();

		currPos -= diffVec * wp->mRadius;

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
	if (mPathfindState != PATHFIND_GOHERE && mContextHandle) {
		Game::testPathfinder->release(mContextHandle);
	}
}

} // namespace Screen


} // namespace Drought
