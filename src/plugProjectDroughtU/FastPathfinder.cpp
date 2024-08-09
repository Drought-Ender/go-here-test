#include "Drought/FibonacciHeap.h"
#include "Drought/Pathfinder.h"
#include "Game/pathfinder.h"

#include "Game/routeMgr.h"

namespace Drought
{

struct WaypointFloatArrayEval : FibonacciEvalStruct<s16>
{
    WaypointFloatArrayEval(f32* arr) : mArray(arr) 
    {
    }

    f32* mArray;
    virtual f32 eval(s16 idx) const {
        return mArray[idx];
    }
};

enum PathFindFlags2 {
	PATHFLAG_RequireOpen                = 0x1,
	PATHFLAG_PathThroughWater           = 0x2,
	PATHFLAG_DisallowUnfinishedBridges  = 0x4,
	PATHFLAG_Unk4                       = 0x8,
	PATHFLAG_DisallowVsRed              = 0x10,
	PATHFLAG_DisallowVsBlue             = 0x20,
	PATHFLAG_AllowUnvisited             = 0x40,
	PATHFLAG_TwoWayPathing              = 0x80 // used for Panmodoki and BlackMan
};

inline Game::WayPoint* getWaypoint(s16 idx) {
    return Game::PathfindContext::routeMgr->getWayPoint(idx);
}

static f32 heuristic(s16 first, s16 second) {
    f32 XZDistance = getWaypoint(first)->mPosition.sqrDistanceXZ(getWaypoint(second)->mPosition);
    // f32 heightDifference = getWaypoint(first)->mPosition.y - getWaypoint(second)->mPosition.y;
    // f32 sqrHeight = heightDifference;
    return XZDistance;
}

Path::Path() {
    mRoot = nullptr;
}

PathNode::PathNode(s16 wpIdx) {
    mWpIdx = wpIdx;
    mNext = nullptr;
}

void Path::prepend(PathNode* node) {
    node->mNext = mRoot;
    mRoot = node;
}

f32 Path::getDistance() {
    PathNode* node = mRoot;
    if (!node) {
        return 0.0f;
    }

    f32 travelDistance = 0.0f;

    Vector3f prevPos = getWaypoint(node->mWpIdx)->mPosition;

    node = node->mNext;

    while (node)
    {
        Vector3f newPos = getWaypoint(node->mWpIdx)->mPosition;
        travelDistance += prevPos.distance(newPos);
        prevPos = newPos;
    }
    return travelDistance;
}

u32 reconstruct_path(Path& path, s16* arr, s16 current) {
    u32 count = 0;
    while (current != -1)
    {
        path.prepend(new PathNode(current));
        current = arr[current];
        count++;
    }
    return count;
}


u32 Pathfinder::search_fast(s16 startWpID, s16 endWpID, Path& path, u8 requestFlag) {

    size_t waypointCount = Game::PathfindContext::routeMgr->mCount;

    // the real score - used to track true distance
    f32* gScore = new f32[waypointCount];
    for (u32 i = 0; i < waypointCount; i++) {
        gScore[i] = 1E38f;
    }

    gScore[startWpID] = 0.0f;

    // the heuristic score - used to nudge towards the goal node
    f32* fScore = new f32[waypointCount];
    for (u32 i = 0; i < waypointCount; i++) {
        fScore[i] = 1E38f;
    }
    fScore[endWpID] = heuristic(startWpID, endWpID);

    WaypointFloatArrayEval eval (fScore);


    bool* inTree = new bool[waypointCount];
    for (u32 i = 0; i < waypointCount; i++) {
        inTree[i] = false;
    }

    FibonacciHeap<s16> openSet(&eval);
    openSet.insert(startWpID);
    inTree[startWpID] = true;

    s16* cameFrom = new s16[waypointCount];
    for (u32 i = 0; i < waypointCount; i++) {
        cameFrom[i] = -1;
    }
    fScore[startWpID] = 0;

    while (openSet.mRoot)
    {
        s16 current = openSet.extractMin();
        inTree[current] = false;
        if (current == endWpID) {
            u32 count = reconstruct_path(path, cameFrom, current);
            delete[] gScore;
            delete[] fScore;
            delete[] cameFrom;
            delete[] inTree;
            return count;
        }

        for (u16 i = 0; i < getWaypoint(current)->mNumToLinks; i++) {
            
            s16 neighbor = getWaypoint(current)->mToLinks[i];

            Game::WayPoint* neighborWP = getWaypoint(neighbor);
            
            if ((((requestFlag & PATHFLAG_RequireOpen) && (neighborWP->mFlags & Game::WPF_Closed)) 
				|| (!(requestFlag & PATHFLAG_PathThroughWater) && (neighborWP->mFlags & Game::WPF_Water))
				|| ((requestFlag & PATHFLAG_AllowUnvisited) && (neighborWP->mFlags & Game::WPF_Unvisited))
				|| ((neighborWP->mFlags & Game::WPF_Water) && (requestFlag & PATHFLAG_DisallowUnfinishedBridges) && (neighborWP->mFlags & Game::WPF_Bridge))
				|| ((requestFlag & PATHFLAG_DisallowVsRed) && (neighborWP->mFlags & Game::WPF_VersusRed))
				|| ((requestFlag & PATHFLAG_DisallowVsBlue) && (neighborWP->mFlags & Game::WPF_VersusBlue)))) {
				continue;
			}
            f32 tentative_gScore = gScore[current] + neighborWP->mPosition.sqrDistance(getWaypoint(current)->mPosition);
            if (tentative_gScore < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                fScore[neighbor] = tentative_gScore + heuristic(neighbor, endWpID);
                if (!inTree[neighbor]) {
                    openSet.insert(neighbor);
                }
            }
        }

        // I hate this too
        if (requestFlag & PATHFLAG_TwoWayPathing == 0) {
            continue;
        }

        for (u16 i = 0; i < getWaypoint(current)->mNumFromLinks; i++) {
            
            s16 neighbor = getWaypoint(current)->mFromLinks[i];

            Game::WayPoint* neighborWP = getWaypoint(neighbor);
            
            if ((((requestFlag & PATHFLAG_RequireOpen) && (neighborWP->mFlags & Game::WPF_Closed)) 
				|| (!(requestFlag & PATHFLAG_PathThroughWater) && (neighborWP->mFlags & Game::WPF_Water))
				|| ((requestFlag & PATHFLAG_AllowUnvisited) && (neighborWP->mFlags & Game::WPF_Unvisited))
				|| ((neighborWP->mFlags & Game::WPF_Water) && (requestFlag & PATHFLAG_DisallowUnfinishedBridges) && (neighborWP->mFlags & Game::WPF_Bridge))
				|| ((requestFlag & PATHFLAG_DisallowVsRed) && (neighborWP->mFlags & Game::WPF_VersusRed))
				|| ((requestFlag & PATHFLAG_DisallowVsBlue) && (neighborWP->mFlags & Game::WPF_VersusBlue)))) {
				continue;
			}
            f32 tentative_gScore = gScore[current] + neighborWP->mPosition.sqrDistance(getWaypoint(current)->mPosition);
            if (tentative_gScore < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_gScore;
                fScore[neighbor] = tentative_gScore + heuristic(neighbor, endWpID);
                if (!inTree[neighbor]) {
                    openSet.insert(neighbor);
                }
            }
        }
        

    }
    delete[] gScore;
    delete[] fScore;
    delete[] cameFrom;
    delete[] inTree;

    return 0;
}

} // namespace Drought
