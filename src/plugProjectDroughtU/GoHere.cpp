#include "og/newScreen/AlteredMapMenu.h"
#include "Game/Navi.h"
#include "Game/Cave/RandMapMgr.h"
#include "Game/Piki.h"
#include "Game/CPlate.h"
#include "JSystem/J2D/J2DPrint.h"

namespace Drought
{

namespace Screen
{

AlteredMapMenu::AlteredMapMenu(const char* name) : og::newScreen::ObjSMenuMap(name)
{
	mLinks = new WayPointLinks;
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
	mGoalWPIndex = -1;
	mContextHandle = 0;
	mStartPathFindCounter = 0;
	mPathFindCounter = 0;
	mRootNode = nullptr;
	mPathfindState = PATHFIND_INACTIVE;

	og::newScreen::ObjSMenuMap::doCreate(rarc);
	// mMapAngle = 0.0f;

}

// your guess is as good as mine
Vector3f AlteredMapMenu::GetPositionFromTex(f32 x, f32 y) {
	Vector2f center;

	og::Screen::calcGlbCenter(mMapTexPane, &center);

	f32 mapX = x - msVal.mMapTexOffset.x;
	f32 mapY = y - msVal.mMapTexOffset.y;

	f32 angle    = -(mMapAngle * TAU) / 360.0f;
	f32 anglecos = cosf(angle);
	f32 anglesin = sinf(angle);

	Vector2f vec = Vector2f(mapX, mapY);

	Vector2f unrotated = Vector2f(
		vec.x * anglecos - vec.y * anglesin,
		vec.y * anglecos + vec.x * anglesin
	);

	f32 scale = mCurrentZoom;
	if (mDisp->mInCave)
		scale *= 2.0f;

	Vector2f oldOrigin = (unrotated - center) * (1 / scale) + center;


	oldOrigin.x = -oldOrigin.x;
	oldOrigin.y = -oldOrigin.y;

	Vector2f cPos;

	if (mDisp->mInCave) {
		cPos.y = (oldOrigin.y + 0.6f) / 0.047f;
		cPos.x = (oldOrigin.x + 0.2f) / 0.047f;
	}
	else {
		cPos.y = (oldOrigin.y + 8.85f - mMapBounds.y * 0.5f) / 0.058f;
		cPos.x = (oldOrigin.y - 24.5f - mMapBounds.y * 0.5f) / 0.058f;
		if (mDisp->mCourseIndex == 3) {
			cPos.x -= (mMapBounds.x * 1400.0f) / 4705.6f;
		}
	}

	Vector3f vec2 (cPos.x, 0.0f, cPos.y);
	vec2.y = Game::mapMgr->getMinY(vec2);
	return vec2;
	
}



Vector2f AlteredMapMenu::GetPositionOnTex(Vector3f& pos, Vector2f& other) {
	f32 xpos, ypos;

	OSReport("Map Position %f %f\n", mMapPosition.x, mMapPosition.y);
	OSReport("Map Bounds %f %f\n", mMapBounds.x, mMapBounds.y);
	
	OSReport("Map Rotation Origin %f %f\n", mMapRotationOrigin.x, mMapRotationOrigin.y);
	// OSReport("Map Angle %f\n", mMapAngle);
	OSReport("Map Tex Offs %f %f\n", msVal.mMapTexOffset.x, msVal.mMapTexOffset.y);

	Vector2f center;

	og::Screen::calcGlbCenter(mPane_map, &center);

	


	Vector2f mapPosition;

	xpos = 0.0f;
	ypos = 0.0f;

	if (mDisp->mInCave) {
		ypos = pos.z * 0.047f + -0.6f;
		xpos = pos.x * 0.047f + -0.2f;
	} else {
		if (mDisp->mCourseIndex == 3) {
			xpos = (mMapBounds.x * 1400.0f) / 4705.6f;
		}
		xpos += mMapTextureDimensions.x * 0.5f + pos.x * 0.058f + 24.5f;
		ypos = mMapTextureDimensions.y * 0.5f + pos.z * 0.058f + -8.85f;
		
	}
	mapPosition.x = (-xpos);
	mapPosition.y = (-ypos);

	other = mapPosition;
	other.x = -other.x;
	other.y = -other.y;

	// OSReport("Guess map pos %f %f\n", mapPosition.x, mapPosition.y);

	OSReport("Zoom %f\n", mCurrentZoom);

	f32 scale = mCurrentZoom;
	if (mDisp->mInCave)
		scale *= 2.0f;

	Vector2f vec = (mMapPosition - mapPosition);

	OSReport("Add Scale %f %f\n", mMapTexPane->mScale.x, mMapTexPane->mScale.y);
	// magic numbers /shrug
	

	f32 angle    = -(mMapAngle * TAU) / 360.0f;
	f32 anglecos = cosf(angle);
	f32 anglesin = sinf(angle);

	Vector2f rotated = Vector2f(
		vec.x * anglecos - vec.y * anglesin,
		vec.y * anglecos + vec.x * anglesin
	);

	rotated.x *= mMapTexPane->mScale.x * 0.9f;
	rotated.y *= mMapTexPane->mScale.y * 0.85f;



	return rotated + center;

	

	/*
	Vector2f cPos = Vector2f(pos.x, pos.z);

	f32 xpos, ypos;

	xpos = cPos.x * 0.052355f + 22.1112399998f;
	ypos = cPos.y * 0.048157f - 7.34816666659f;


	// xpos = cPos.x * 0.052355f + mMapBounds.x * 0.5f - 97.88876f;
	// ypos = cPos.y * 0.048157f + mMapBounds.y * 0.5f - 257.651833;

	f32 angle    = -(mMapAngle * TAU) / 360.0f;
	OSReport("angle %f\n", angle);
	OSReport("Zoom %f\n", mCurrentZoom);
	OSReport("Map Bounds %f %f\n", mMapBounds.x, mMapBounds.y);
	// OSReport("factor %f\n", factor);
	f32 anglecos = cosf(angle);
	f32 anglesin = sinf(angle);

	Vector2f center;

	og::Screen::calcGlbCenter(mMapTexPane, &center);

	Vector2f rotated = Vector2f(
		xpos * anglecos - ypos * anglesin,
		ypos * anglecos + xpos * anglesin
	) * mCurrentZoom;


	// xpos = -(xpos + -0.6f);
	// ypos = -(ypos + -0.2f);

	

	return rotated + center;
	*/

}

bool AlteredMapMenu::CheckAllPikisBlue(Game::Navi* navi) {
	Iterator<Game::Creature> iterator(navi->mCPlateMgr);
	CI_LOOP(iterator)
	{
		Game::Piki* piki = static_cast<Game::Piki*>(*iterator);
		if (piki->getKind() != Game::Blue) {
			return false;
		}
	}
	return true;
}

bool AlteredMapMenu::doStart(::Screen::StartSceneArg const* arg) {
	mAllPikisBlue = CheckAllPikisBlue(Game::naviMgr->getActiveNavi());
	return og::newScreen::ObjSMenuMap::doStart(arg);
}

void AlteredMapMenu::doDraw(Graphics& gfx)
{
	J2DPerspGraph* graf = &gfx.mPerspGraph;
	drawMap(gfx);

	Graphics gfx2;
	mIconScreen->draw(gfx2, *graf);

	if (mCompassPic && mPane_Ncompas) {
		PSMTXCopy(mPane_Ncompas->mGlobalMtx, mCompassPic->mPositionMtx);
	}

	graf->setPort();
	mIconScreen2->draw(gfx, *graf);
	graf->setPort();
	drawYaji(gfx);
	drawPath(gfx);
}

bool AlteredMapMenu::doUpdate() {
	Vector2f center;

	// og::Screen::calcGlbCenter(mMapTexPane, &center);
	// Vector3f goalPos =  GetPositionFromTex(423.173004f, 225.297607f);
	// Game::naviMgr->getActiveNavi()->mPosition = goalPos;
	// PathfindUpdate();

	if (mController->mPadButton->mAnalogR > 0.0f) {
		mMapAngle += 90.0f * sys->mDeltaTime * mController->mPadButton->mAnalogR;
	}
	if (mController->mPadButton->mAnalogL > 0.0f) {
		mMapAngle -= 90.0f * sys->mDeltaTime * mController->mPadButton->mAnalogL;
	}

	return og::newScreen::ObjSMenuMap::doUpdate();
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
			initPathfinding(mLinks);
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
	OSReport("initPathfinding\n");
	if (resetLinkCount) {
		mLinks->mCount = 0;
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
	searchArg.mLinks  = mLinks;

	Game::WayPoint* startWP = nullptr;
	if (Game::mapMgr->mRouteMgr->getNearestEdge(searchArg)) {
		if (!searchArg.mWp1->isFlag(Game::WPF_Closed)) {
			startWP = searchArg.mWp1;
		} else {
			startWP = searchArg.mWp2;
		}
	} else {
		searchArg.mLinks = nullptr;
		if (Game::mapMgr->mRouteMgr->getNearestEdge(searchArg)) {
			if (searchArg.mWp1->isFlag(Game::WPF_Closed)) {
				startWP = searchArg.mWp2;
			} else {
				startWP = searchArg.mWp1;
			}
		} else {
			JUT_PANIC("zannen !\n");
		}
	}

	JUT_ASSERT(startWP, "start==0");

	mStartWPIndex = startWP->mIndex;

	
	Vector3f goalPos = GetPositionFromTex(mMapPosition.x, mMapPosition.y);
	
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
	

	OSReport("%d->...->%d\n", mStartWPIndex, mGoalWPIndex);

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

	OSReport("Path Find Start\n");

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

		if (mAllPikisBlue) {
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
	JUtility::TColor color1 = 0xffffffff;
	JUtility::TColor color2 = 0xff0000ff;
	J2DPerspGraph* graf = &gfx.mPerspGraph;
	graf->setPort();
	J2DPrint print(JFWSystem::systemFont, color1, color1);
	Vector3f naviPos = Game::naviMgr->getActiveNavi()->getPosition();  //Game::ItemOnyon::mgr->mUfo->getPosition();

	Vector2f view;

	Vector2f naviOnBoard = GetPositionOnTex(naviPos, view);
	print.print(naviOnBoard.x, naviOnBoard.y, "v"); // awesome pointer trust


	print.print(-mMapPosition.x, -mMapPosition.y, "o"); // awesome pointer trust
	print.print(view.x, view.y, "x");

	const u8 oldWidth = graf->mLineWidth;
	graf->setPort();
	graf->setLineWidth(10);
	graf->setColor(color1);
	
	

	Vector2f center;
	og::Screen::calcGlbCenter(mOlimarArrow, &center);

	Vector2f realNaviPos = center;

	og::Screen::calcGlbCenter(mMapTexPane, &center);

	graf->line(center, naviOnBoard);

	graf->setColor(color2);

	graf->line(realNaviPos, naviOnBoard);

	graf->setLineWidth(oldWidth);

	// OSReport("Ship Guess Pos %f %f\n", naviOnBoard.x, naviOnBoard.y);
	// OSReport("Ship Real Pos %f %f\n", realNaviPos.x, realNaviPos.y);
	// OSReport("Delta %f %f\n", realNaviPos.x - naviOnBoard.x, realNaviPos.y - naviOnBoard.y);


	if (mPathfindState != PATHFIND_DONE) {
		return;
	}
	OSReport("Draw Path\n");

	

	

	// const u8 oldWidth = graf->mLineWidth;

	// graf->setLineWidth(10);
	// graf->setColor(color1);

	// FOREACH_NODE_EX(Game::PathNode, mRootNode, node, node->mNext) {
	// 	Vector3f firstPos = (Game::mapMgr->mRouteMgr->getWayPoint(node->mWpIndex)->getPosition());
	// 	// im sorry
	// 	Vector3f secondPos = (Game::mapMgr->mRouteMgr->getWayPoint(static_cast<Game::PathNode*>(node->mNext)->mWpIndex)->getPosition());


	// 	JGeometry::TVec2f first = GetPositionOnTex(firstPos);
	// 	JGeometry::TVec2f second = GetPositionOnTex(secondPos);

		
	// 	graf->line(first, second);
		
	// }

	// graf->setLineWidth(oldWidth);

}

} // namespace Screen


} // namespace Drought
