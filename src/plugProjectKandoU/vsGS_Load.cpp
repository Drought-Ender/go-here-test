#include "og/newScreen/Floor.h"
#include "Game/VsGame.h"
#include "Controller.h"
#include "Game/gameStat.h"
#include "Screen/Game2DMgr.h"
#include "Radar.h"
#include "nans.h"
#include "types.h"
#include "Game/SingleGame.h"
#include "TParticle2dMgr.h"
#include "PSSystem/PSSystemIF.h"

namespace Game {
namespace VsGame {

/**
 * @note Address: 0x8022D0E8
 * @note Size: 0xF0
 */
LoadState::LoadState()
    : State(VGS_Load)
    , _20(0)
    , _24(0)
    , _28(0)
{
	mController = new Controller(JUTGamePad::PORT_0);
	mDelegate   = new Delegate<Game::VsGame::LoadState>(this, &dvdLoad);
}

/**
 * @note Address: 0x8022D1D8
 * @note Size: 0xD0
 */
void LoadState::init(Game::VsGameSection* section, StateArg* args)
{
	if (Radar::mgr) {
		Radar::mgr->clear();
	}

	Screen::gGame2DMgr->mScreenMgr->reset();
	mSection = section;
	Game::GameStat::clear();

	LoadArg* loadArg = static_cast<LoadArg*>(args);
	_20              = loadArg->_00;
	_24              = loadArg->_04;
	_28              = loadArg->_08;

	section->refreshHIO();
	_1C = 0;
	_9C = 0;
	_A0 = 15.0f;

	for (int i = 0; i < 7; i++) {
		section->mMarbleYellow[i] = nullptr;
	}
}

/**
 * @note Address: 0x8022D2A8
 * @note Size: 0x24
 */
void LoadState::dvdLoad() { mSection->setupFloatMemory(); }

/**
 * @note Address: 0x8022D2CC
 * @note Size: 0x2C4
 */
void LoadState::exec(VsGameSection* section)
{
	if (!_1C) {
		if (_28) {
			sys->heapStatusDump(true);
			section->clearHeap();
			sys->heapStatusDump(true);
		}
		og::Screen::DispMemberFloor floor;
		floor.mSublevel = section->getCurrFloor() + 1;
		ID32 id;
		if (gameSystem->isChallengeMode()) {
			sprintf(id.getStr(), "c_%02d", section->mChallengeStageData->mStageIndex);
		} else {
			sprintf(id.getStr(), "vs%02d", section->mVsStageData->mIndex2D);
		}
		id.updateID();
		floor.mCaveID = id.getID();
		Screen::gGame2DMgr->open_Floor(floor);
		sys->dvdLoadUseCallBack(&mDvdThreadCommand, mDelegate);
		_1C = true;
		_9C = false;
	} else {
		section->BaseHIOSection::doUpdate();
		if (particle2dMgr) {
			particle2dMgr->update();
		}
		Screen::gGame2DMgr->update();
		if (gameSystem->isVersusMode() && (_A0 > 0.0f)) {
			if (mDvdThreadCommand.mMode == 2) { // probably an enum
				Screen::gGame2DMgr->set_FloorVS_LoadEnd();
				if (mController->isButtonDown(JUTGamePad::PRESS_A | JUTGamePad::PRESS_START)) {
					PSSystem::spSysIF->playSystemSe(PSSE_SY_FLOOR_COMPLETE, nullptr);
					_A0 = 0.0f;
				}
			}
			_A0 -= sys->mDeltaTime;
		}
		if (!_9C && mDvdThreadCommand.mMode == 2) {
			if (gameSystem->isChallengeMode() || (gameSystem->isVersusMode() && _A0 <= 0.0f)) {
				section->postSetupFloatMemory();
				_9C = true;
				if (!_24) {
					transit(section, VGS_Game, nullptr);
				} else {
					transit(section, VGS_VS, nullptr);
				}
			}
		}
	}
}

/**
 * @note Address: 0x8022D590
 * @note Size: 0x7C
 */
void LoadState::draw(VsGameSection* section, Graphics& graphic)
{
	graphic.mPerspGraph.setPort();
	particle2dMgr->draw(1, 0); // I bet these are booleans
	Screen::gGame2DMgr->draw(graphic);
	graphic.mPerspGraph.setPort();
	particle2dMgr->draw(0, 0);
}

/**
 * @note Address: 0x8022D60C
 * @note Size: 0x4
 */
void LoadState::cleanup(VsGameSection*) { }

} // namespace VsGame
} // namespace Game
