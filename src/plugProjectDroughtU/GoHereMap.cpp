#include "Drought/Screen/AlteredMapMenu.h"
#include "Game/Navi.h"
#include "Game/NaviState.h"
#include "Drought/Game/NaviGoHere.h"
#include "Game/Cave/RandMapMgr.h"
#include "LoadResource.h"
#include "Game/Piki.h"
#include "Game/CPlate.h"
#include "JSystem/J2D/J2DPrint.h"

namespace Drought
{

namespace Screen
{

AlteredMapMenu::AlteredMapMenu(const char* name) : og::newScreen::ObjSMenuMap(name)
{
	mStartWPIndex = -1;

	if (Game::naviMgr->getActiveNavi()->mController1->isButton(PAD_BUTTON_X)) {
		Vector3f vec = 0.0f;
		vec.y = Game::mapMgr->getMinY(vec);
		Game::naviMgr->getActiveNavi()->mPosition = vec;
		Game::naviMgr->getActiveNavi()->mFaceDir = 0.0f;
	}
	// else {
	// 	// Game::naviMgr->getActiveNavi()->mPosition.z = 0.0f;
	// 	// Game::naviMgr->getActiveNavi()->mPosition.x = 1.0f;
	// }
}

void AlteredMapMenu::doCreate(JKRArchive* rarc) {
	mAllPikisBlue = false;
	mCanStartPathfind = false;
	mGoalWPIndex = -1;
	mContextHandle = 0;
	mStartPathFindCounter = 0;
	mPathFindCounter = 0;
	mRootNode = nullptr;
	mPathfindState = PATHFIND_INACTIVE;

	og::newScreen::ObjSMenuMap::doCreate(rarc);

	LoadResource::Arg first ("/user/Drought/GoHere/a_btn.bti");
	LoadResource::Arg second ("/user/Drought/GoHere/arrow.bti");
	LoadResource::Arg third ("/user/Drought/GoHere/arrow_red.bti");

	LoadResource::Node* node1 = gLoadResourceMgr->load(first);
	LoadResource::Node* node2 = gLoadResourceMgr->load(second);
	LoadResource::Node* node3 = gLoadResourceMgr->load(third);
	mAButtonTex  = static_cast<ResTIMG*>(node1->mFile);
	mArrowTex    = static_cast<ResTIMG*>(node2->mFile);
	mArrowRedTex = static_cast<ResTIMG*>(node3->mFile);

	mArrowPicture = og::Screen::CopyPictureToPane(mLouieArrow, mRootPane, 0.0f, 0.0f, 'go_here0');
	mAButton      = og::Screen::CopyPictureToPane(mLouieArrow, mRootPane, 0.0f, 0.0f, 'go_here1');

	mAButton->changeTexture(mAButtonTex, 0);
	mAButton->mScale *= 0.5f;
	mArrowPicture->changeTexture(mArrowTex, 0);
	mArrowPicture->hide();
	mAButton->hide();

	// mMapAngle = 0.0f;

}

void AlteredMapMenu::commonUpdate() {
	og::newScreen::ObjSMenuMap::commonUpdate();

// 	mArrowPicture->setAlpha(255);
// 	mAButton->setAlpha(255);

	Vector2f center;

	og::Screen::calcGlbCenter(mPane_map, &center);


	mArrowPicture->setOffset(12.0f, -5.0f);
	mAButton->setOffset(20.0f, -20.0f);
}

// your guess is as good as mine
Vector3f AlteredMapMenu::GetPositionFromTex(f32 x, f32 y) {

	Vector2f center;

	og::Screen::calcGlbCenter(mPane_map, &center);

	Vector2f vec (x - center.x, y - center.y);

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

	// magic numbers /shrug
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
	return og::newScreen::ObjSMenuMap::doEnd(arg);
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

	// Vector2f center;

	// og::Screen::calcGlbCenter(mPane_map, &center);

	// Vector3f goalPos = GetPositionFromTex(center.x, center.y);

	// Game::naviMgr->getActiveNavi()->mPosition = goalPos;

	if (mController->mPadButton->mAnalogR > 0.0f) {
		mMapAngle += 90.0f * sys->mDeltaTime * mController->mPadButton->mAnalogR;
	}
	if (mController->mPadButton->mAnalogL > 0.0f) {
		mMapAngle -= 90.0f * sys->mDeltaTime * mController->mPadButton->mAnalogL;
	}

	og::newScreen::SceneSMenuBase* scene = static_cast<og::newScreen::SceneSMenuBase*>(getOwner());

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
	
	// Game::WPEdgeSearchArg searchArg2(goalPos);

	// if (Game::gameSystem->mIsInCave) {
	// 	Sys::Sphere sphere;
	// 	sphere.mPosition = goalPos;
	// 	sphere.mRadius   = 1.0f;


	// 	roomIndex = static_cast<Game::RoomMapMgr*>(Game::mapMgr)->findRoomIndex(sphere);
	// }

	// searchArg2.mRoomID = roomIndex;
	// searchArg2.mLinks  = mLinks;

	// Game::WayPoint* endWP = nullptr;
	// if (Game::mapMgr->mRouteMgr->getNearestEdge(searchArg2)) {
	// 	endWP = searchArg2.mWp1;
	// } else {
	// 	searchArg2.mLinks = nullptr;
	// 	if (Game::mapMgr->mRouteMgr->getNearestEdge(searchArg2)) {
	// 		endWP = searchArg2.mWp1;
	// 	} else {
	// 		JUT_PANIC("zannen !\n");
	// 	}
	// }

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


int AlteredMapMenu::execPathfinding() {
	if (mContextHandle == 0) {
		return PATHFINDSTATUS_FAIL;
	}

	// NULL context handle! (!)
	if (mContextHandle == -1) {
		return PATHFINDSTATUS_FAIL;
	}

	mPathFindCounter++;

	switch (Game::testPathfinder->check(mContextHandle)) {
	case Game::PATHFIND_MakePath:
		mPathfindState = PATHFIND_DONE;
		mWayPointCount = Game::testPathfinder->makepath(mContextHandle, &mRootNode);
		OnPathfindDone();
		break;

	case Game::PATHFIND_Start: // make a new context and start a path
		OSReport("Pathfind restart\n");
		if (mContextHandle) {
			Game::testPathfinder->release(mContextHandle);
		}

		u8 flag = Game::PATHFLAG_Unk3;

		if (mAllPikisBlue || mStartPathFindCounter >= 1) {
			flag |= Game::PATHFLAG_PathThroughWater;
		}
		mStartPathFindCounter++;

		Game::PathfindRequest request(mStartWPIndex, mGoalWPIndex, flag);

		// get a new handle
		mContextHandle = Game::testPathfinder->start(request);

		// reset our counter bc we have a new context!
		mPathFindCounter = 0;
		return PATHFINDSTATUS_OK;

		case Game::PATHFIND_Busy: // keep on keepin' on
		break;

	case Game::PATHFIND_NoHandle: // woops something happened to the handle
		JUT_PANIC("no handle %d\n", mContextHandle);
		break;
	}
	return PATHFINDSTATUS_OK;
}

void AlteredMapMenu::drawPath(Graphics& gfx) {


	if (mPathfindState != PATHFIND_DONE) {
		return;
	}

	JUtility::TColor color1 = 0xffffffff;
	JUtility::TColor color2 = 0xffaaaaff;

	bool isImpossible = false;


	J2DPerspGraph* graf = &gfx.mPerspGraph;
	
	Vector3f naviPos = Game::naviMgr->getActiveNavi()->getPosition();  //Game::ItemOnyon::mgr->mUfo->getPosition();

	Vector2f view;

	
	Vector2f goHerePtr;

	og::Screen::calcGlbCenter(mPane_map, &goHerePtr);

	


	const u8 oldWidth = graf->mLineWidth;

	graf->setPort();
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

		if (wp->isFlag(Game::WPF_Closed) || (wp->isFlag(Game::WPF_Water) && !mAllPikisBlue)) {
			isImpossible = true;
			graf->setColor(color2);
		}

		previousPos = currPos;
		
	}


	graf->lineTo(goHerePtr);

	graf->setLineWidth(oldWidth);


	// JUtility::TColor& color = (isImpossible) ? color2 : color1;

	// J2DPrint print(JFWSystem::systemFont, color, color);

	// print.print(goHerePtr.x, goHerePtr.y, "v"); // awesome pointer trust


	// graf->setPort();
	// Vector3f vec1 = mPane_map->getGlbVtx(0);
	// Vector3f vec2 = mPane_map->getGlbVtx(1);
	// Vector3f vec3 = mPane_map->getGlbVtx(2);
	// Vector3f vec4 = mPane_map->getGlbVtx(3);
	// Color4 color3(100, 0, 0, 155);
	// drawVecZ(gfx, *(Vec*)&vec1, *(Vec*)&vec2, *(Vec*)&vec3, *(Vec*)&vec4, color3, -0.999);
	// GXSetColorUpdate(GX_TRUE);
	// PSMTXCopy(mPane_map->mGlobalMtx, mRootPane->mPositionMtx);
	graf->setPort();
	GXSetZCompLoc(GX_TRUE);
	GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);

}

void AlteredMapMenu::PathfindCleanup() {
	if (mPathfindState != PATHFIND_GOHERE && mContextHandle) {
		Game::testPathfinder->release(mContextHandle);
	}
}

} // namespace Screen


} // namespace Drought
