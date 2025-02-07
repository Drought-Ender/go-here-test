#include "Game/Navi.h"
#include "Game/NaviState.h"
#include "Game/NaviParms.h"
#include "PSM/Navi.h"
#include "Drought/Game/NaviGoHere.h"
#include "Game/MoviePlayer.h"
#include "Game/MapMgr.h"
#include "Game/CameraMgr.h"
#include "Drought/Pathfinder.h"
#include "Game/GameLight.h"
#include "Game/CPlate.h"

namespace Game {

bool AreAllPikisBlue(Navi* navi)
{
	Iterator<Creature> iterator(navi->mCPlateMgr);
	CI_LOOP(iterator)
	{
		Piki* piki = static_cast<Piki*>(*iterator);
		if (piki->getKind() != Game::Blue) {
			return false;
		}
	}
	return true;
}

bool IsGameWorldActive()
{
	if (moviePlayer && moviePlayer->mDemoState != DEMOSTATE_Inactive) {
		return false;
	}

	if (!gameSystem->isFlag(GAMESYS_IsGameWorldActive)) {
		return false;
	}

	return !gameSystem->paused_soft();
}

void BaseGameSection::directDraw(Graphics& gfx, Viewport* vp)
{
	vp->setViewport();
	vp->setProjection();
	gfx.initPrimDraw(vp->getMatrix(true));
	doDirectDraw(gfx, vp);
	if (naviMgr) {
		Navi* player = naviMgr->getActiveNavi();
		if (player) {
			player->doDirectDraw(gfx);
		}
	}
	if (TexCaster::Mgr::sInstance) {
		gfx.initPrimDraw(vp->getMatrix(true));
		mLightMgr->mFogMgr->set(gfx);
		TexCaster::Mgr::sInstance->draw(gfx);
	}
}

void NaviFSM::init(Navi* navi)
{
	mBackupStateID = NSID_NULL;
	create(NSID_StateCount);

	registerState(new NaviWalkState);
	registerState(new NaviFollowState);
	registerState(new NaviPunchState);
	registerState(new NaviChangeState);
	registerState(new NaviGatherState);
	registerState(new NaviThrowState);
	registerState(new NaviThrowWaitState);
	registerState(new NaviDopeState);
	registerState(new NaviNukuState);
	registerState(new NaviNukuAdjustState);
	registerState(new NaviContainerState);
	registerState(new NaviAbsorbState);
	registerState(new NaviFlickState);
	registerState(new NaviDamagedState);
	registerState(new NaviPressedState);
	registerState(new NaviFallMeckState);
	registerState(new NaviKokeDamageState);
	registerState(new NaviSaraiState);
	registerState(new NaviSaraiExitState);
	registerState(new NaviDeadState);
	registerState(new NaviStuckState);
	registerState(new NaviDemo_UfoState);
	registerState(new NaviDemo_HoleInState);
	registerState(new NaviPelletState);
	registerState(new NaviCarryBombState);
	registerState(new NaviClimbState);
	registerState(new NaviPathMoveState);

	// CUSTOM STATES
	registerState(new NaviGoHereState);
}

void NaviGoHereState::init(Navi* player, StateArg* arg)
{
	P2ASSERT(arg);
	NaviGoHereStateArg* goHereArg = static_cast<NaviGoHereStateArg*>(arg);

	player->startMotion(IPikiAnims::WALK, IPikiAnims::WALK, nullptr, nullptr);
	player->setMoveRotation(true);

	mTargetPosition = goHereArg->mPosition;
	mPath           = goHereArg->mPath;

	mActiveRouteNodeIndex = 0;
	mLastPosition         = player->getPosition();
	mTimeoutTimer         = 0.0f;
}

// usually inlined, plays the navi's voice line when swapped
inline void NaviState::playChangeVoice(Navi* player)
{
	if (player->mNaviIndex == NAVIID_Olimar) { // OLIMAR
		PSSystem::spSysIF->playSystemSe(PSSE_SY_CHANGE_ORIMA, 0);

	} else if (playData->isStoryFlag(STORY_DebtPaid)) { // PRESIDENT
		PSSystem::spSysIF->playSystemSe(PSSE_SY_CHANGE_SHACHO, 0);

	} else { // LOUIE
		PSSystem::spSysIF->playSystemSe(PSSE_SY_CHANGE_LUI, 0);
	}

	if (player->mNaviIndex == NAVIID_Olimar) { // OLIMAR
		player->mSoundObj->startSound(PSSE_PL_PIKON_ORIMA, 0);

	} else if (playData->isStoryFlag(STORY_DebtPaid)) { // PRESIDENT
		player->mSoundObj->startSound(PSSE_PL_PIKON_SHACHO, 0);

	} else { // LOUIE
		player->mSoundObj->startSound(PSSE_PL_PIKON_LUI, 0);
	}
}

void NaviGoHereState::exec(Navi* player)
{
	// Handle early exit conditions (dead, frozen, etc)
	{
		if (moviePlayer && moviePlayer->mDemoState != DEMOSTATE_Inactive) {
			return;
		}

		// No idea why we're in this state and dead
		if (!player->isAlive()) {
			return;
		}
	}

	// Handle movement towards the target
	{
		WayPoint* current = getCurrentWaypoint();
		if (mActiveRouteNodeIndex != -1 && current != nullptr) {
			navigateToWayPoint(player, current);
		} else if (navigateToFinalPoint(player)) { // true if target reached, false if not
			player->GoHereSuccess();
			changeState(player, true);
			return;
		}
	}

	// Update the camera angle
	Vector3f playerPos = player->getPosition();
	{
		static f32 currentAngle = roundAng(player->getFaceDir() + PI);
		f32 targetAngle         = roundAng(player->getFaceDir() + PI);

		// Smoothly interpolate to the target angle
		currentAngle += 0.075f * angDist(roundAng(targetAngle), currentAngle);
		currentAngle = roundAng(currentAngle);

		cameraMgr->setCameraAngle(roundAng(currentAngle + gMapRotation), player->mNaviIndex);
	}

	// If we haven't moved in a while, start incrementing the giveup timer
	{
		f32 distanceBetweenLast = Vector3f::qDistance(playerPos, mLastPosition);
		mLastPosition           = playerPos;
		if (distanceBetweenLast <= 1.5f) {
			mTimeoutTimer += sys->mDeltaTime;

			// If the player presses a button, or we've been trying for a while, give up
			bool isAnyInput = player->mController1 && player->mController1->isAnyInput();
			if (isAnyInput || mTimeoutTimer >= 2.5f) {
				// The player change sound triggers if input is pressed, otherwise the damage sound triggers
				changeState(player, isAnyInput);
				return;
			}
		}
	}

	// Handle controller input
	{
		// Update the whistle's position
		player->mWhistle->update(player->mTargetVelocity, false);

		if (!player->mController1) {
			handlePlayerChangeFix(player); // Fixes the controller/camera not updating when the player changes
			return;
		}

		// Press B to exit
		if (player->mController1->isButtonDown(JUTGamePad::PRESS_B)) {
			changeState(player, true);
			return;
		}

		// Press Y to swap the captains
		if (!gameSystem->isMultiplayerMode() & playData->isDemoFlag(DEMO_Unlock_Captain_Switch)
		    && player->mController1->isButtonDown(JUTGamePad::PRESS_Y)) {

			Navi* otherPlayer = naviMgr->getAt(GET_OTHER_NAVI(player));
			if (!otherPlayer->canSwap()) {
				return;
			}

			gameSystem->mSection->pmTogglePlayer();

			playChangeVoice(otherPlayer);
			if (otherPlayer->mCurrentState->needYChangeMotion()) {
				otherPlayer->mFsm->transit(otherPlayer, NSID_Change, nullptr);
			}
		}
	}
}

void NaviGoHereState::collisionCallback(Navi* player, CollEvent& event)
{
	// Only handle collisions with an enemy.
	if (!event.mCollidingCreature->isTeki()) {
		return;
	}

	// Calculate the flat (XZ) direction from the enemy to the player.
	Vector3f enemyPos      = event.mCollidingCreature->getPosition();
	Vector3f playerPos     = player->getPosition();
	Vector3f baseDirection = playerPos;
	Vector3f::getFlatDirectionFromTo(enemyPos, baseDirection);
	baseDirection.normalise();

	// Create a perpendicular vector in the XZ plane.
	// (For (x, 0, z), a perpendicular is (-z, 0, x) assuming Y is up.)
	Vector3f perp(-baseDirection.z, 0.0f, baseDirection.x);
	perp.normalise();

	// Define the offset angle in radians (30° here).
	const float angleRad = TORADIANS(30.0f);

	// Determine which side to slip by projecting the player's current velocity onto the perpendicular.
	// Flatten the current velocity to ignore any vertical component.
	Vector3f currentVel = player->mVelocity;
	currentVel.y        = 0.0f;
	float sign          = 1.0f;
	if (currentVel.length() > 0.001f) {
		// Dot product tells us if current velocity has a component along the perpendicular.
		sign = (currentVel.dot(perp) > 0) ? 1.0f : -1.0f;
	}

	// Rotate the base direction by the offset angle. The rotation in the flat plane is:
	// newDir = baseDirection * cos(angle) + (perp * sign) * sin(angle)
	Vector3f newDirection = baseDirection * cos(angleRad) + (perp * sign) * sin(angleRad);
	newDirection.normalise();

	// Set the player's velocity in this new direction.
	player->mVelocity = newDirection * player->getMoveSpeed();
}

void NaviGoHereState::handlePlayerChangeFix(Navi* player)
{
	static f32 forceTimer = 0.0f;

	u32 playerIdx = gameSystem->mSection->mCurrentPlayerIndex;
	// Only apply the change if we're the intended active player
	if (player->mNaviIndex != playerIdx) {
		return;
	}

	PlayCamera* currentCam = playerIdx == NAVIID_Olimar ? gameSystem->mSection->mOlimarCamera : gameSystem->mSection->mLouieCamera;
	if (!currentCam) {
		// God help us, no idea how this would happen, but let's not crash
		return;
	}

	forceTimer += sys->mDeltaTime;

	// Give it 0.5 seconds to change or we do it forcefully
	if (forceTimer >= 0.5f) {
		currentCam->mChangePlayerState = CAMCHANGE_None;
		forceTimer                     = 0.0f;
	}
}

bool NaviGoHereState::handleControlStick(Navi* player)
{
	player->makeCStick(false);
	if (player->isMovieActor()) {
		return true;
	}

	if (gameSystem->isStoryMode()) {
		Navi* activePlayer = naviMgr->getActiveNavi();
		if (activePlayer != player) {
			return false;
		}

		player->mSoundObj->mRappa.playRappa(true, player->mCStickPosition.x, player->mCStickPosition.z, player->mSoundObj);

	} else {
		player->mSoundObj->mRappa.playRappa(true, player->mCStickPosition.x, player->mCStickPosition.z, player->mSoundObj);
	}

	return false;
}

// moves the navi to the nearest waypoint
void NaviGoHereState::navigateToWayPoint(Navi* player, Game::WayPoint* target)
{
	Graphics* gfx = sys->getGfx();

	Vector3f playerPos = player->getPosition();
	Vector3f targetPos = target->mPosition;

	Vector3f dirToTarget = targetPos;
	f32 distanceToTarget = Vector3f::getFlatDirectionFromTo(playerPos, dirToTarget);

	Vector3f primaryDir = targetPos - playerPos;
	primaryDir.normalise();

	Vector3f blendedDir;

	// If there is a next waypoint in the route, blend in its direction.
	if (mActiveRouteNodeIndex + 1 < mPath.mLength) {
		Game::WayPoint* nextWp = getWaypointAt(mActiveRouteNodeIndex + 1);

		// Compute the direction from the current target to the next waypoint.
		Vector3f nextDir = nextWp->mPosition - target->mPosition;
		nextDir.y        = 0.0f;
		nextDir.normalise2D();

		// Compute an interpolation factor 't' based on the distance to the current waypoint.
		// (Far away: t near 0, so use mostly primaryDir; close: t approaches 1, so blend in nextDir.)
		float t = clamp(1.0f - (distanceToTarget / target->mRadius), 0.0f, 1.0f);

		// Linearly blend the two directions:
		blendedDir = (primaryDir * (1.0f - t)) + nextDir * t;
		blendedDir.normalise();
	} else {
		// There's no next waypoint, so just use the primary direction.
		blendedDir = primaryDir;
	}

	Vector3f finalPos = targetPos + blendedDir;

	// If we are still far enough away from the target, move toward it.
	if (distanceToTarget >= target->mRadius) {
		player->makeCStick(true);
		player->mTargetVelocity = blendedDir * player->getMoveSpeed();
		return;
	}

	// If we're at the waypoint, move to the next one
	mActiveRouteNodeIndex++;
	if (mActiveRouteNodeIndex >= mPath.mLength) {
		mActiveRouteNodeIndex = -1;
		return;
	}

	WayPoint* nextWp = getCurrentWaypoint();

	// if the next waypoint is open, don't stop
	if (!nextWp->isFlag(WPF_Closed)) {
		return;
	}

	// If the waypoint is in water, and we don't have all blue pikmin, stop
	bool isWater = nextWp->isFlag(WPF_Water) && !AreAllPikisBlue(player);
	if (!isWater) {
		return;
	}

	// We're either at a closed waypoint or a water waypoint, so stop
	mTargetPosition       = nextWp->getPosition();
	mActiveRouteNodeIndex = -1;

	player->GoHereInterupted();
	if (isWater) {
		player->GoHereInteruptWater();
	} else {
		player->GoHereInteruptBlocked();
	}
}

// moves the navi to its final target destination
bool NaviGoHereState::navigateToFinalPoint(Navi* player)
{
	Vector3f direction = mTargetPosition;
	f32 distance       = Vector3f::getFlatDirectionFromTo(player->getPosition(), direction);
	if (distance < mFinishDistanceThreshold) {
		return true;
	}

	player->makeCStick(true);
	player->mTargetVelocity = direction * player->getMoveSpeed();
	return false;
}

void NaviGoHereState::cleanup(Navi* navi)
{
	mTimeoutTimer = 0.0f;
	mPath.clear();
}

void NaviGoHereState::changeState(Navi* player, bool isWanted)
{
	player->transit(isWanted ? NSID_Change : NSID_Walk, nullptr);
	PSSystem::spSysIF->playSystemSe(isWanted ? PSSE_SY_PLAYER_CHANGE : PSSE_PL_ORIMA_DAMAGE, 0);
}

void Navi::doDirectDraw(Graphics& gfx)
{
	return;

	if (getStateID() != NSID_GoHere) {
		return;
	}

	Game::NaviGoHereState* state = (Game::NaviGoHereState*)getCurrState();

	PerspPrintfInfo info;
	Vector3f pos(mPosition.x, 15.0f + mPosition.y, mPosition.z);

	info.mColorA = Color4(0xC8, 0xC8, 0xFF, 0xC8);
	info.mColorB = Color4(0x64, 0x64, 0xFF, 0xC8);
	gfx.perspPrintf(info, pos, "[%d/%d] t[%1.1f]", state->mActiveRouteNodeIndex, state->mPath.mLength, state->mTimeoutTimer);
}

bool Navi::canSwap()
{
	s32 state = getStateID();
	return isAlive() && state != NSID_Nuku && state != NSID_NukuAdjust && state != NSID_Punch;
}

f32 Navi::getMoveSpeed()
{
	f32 speed = getOlimarData()->hasItem(OlimarData::ODII_RepugnantAppendage) ? naviMgr->mNaviParms->mNaviParms.mRushBootSpeed()
	                                                                          : naviMgr->mNaviParms->mNaviParms.mMoveSpeed();

	return speed;
}

void Navi::GoHereSuccess()
{
	// your code here
}

void Navi::GoHereInterupted() { }

void Navi::GoHereInteruptBlocked() { }

void Navi::GoHereInteruptWater() { }

} // namespace Game
