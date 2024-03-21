#ifndef _DROUGHT_MISC_H_
#define _DROUGHT_MISC_H_

#include "Vector3.h"
#include "Game/CurrTriInfo.h"
#include "Game/MapMgr.h"

namespace Drought
{

    inline bool hasValidFloor(Vector3f& vec) {
        Game::CurrTriInfo triSpawn;
        triSpawn.mTriangle = nullptr;
        triSpawn.mMaxY = FLOAT_DIST_MAX;
        triSpawn.mMinY = FLOAT_DIST_MIN;
        triSpawn.mTable = nullptr;
        triSpawn.mNormalVec = Vector3f(0.0f, 1.0f, 0.0f);
        triSpawn.mGetFullInfo = 0;
        triSpawn.mPosition = vec;
        triSpawn.mUpdateOnNewMaxY = 0;
        Game::mapMgr->getCurrTri(triSpawn);
        return triSpawn.mTriangle;
    }

} // namespace Drought


#endif