#include "Drought/FibonacciHeap.h"
#include "Drought/Pathfinder.h"
#include "Game/pathfinder.h"
#include "Game/MapMgr.h"
#include "Game/routeMgr.h"

namespace Drought {
PathNode::PathNode(s16 idx, PathNode* p, f32 g, f32 h)
    : mWaypointIndex(idx)
    , mParent(p)
    , mGroundCost(g)
    , mHeuristicCost(h)
    , mFinalCost(g + h)
{
}

Path::Path()
    : mWaypointList(nullptr)
    , mLength(0)
{
}

Path::~Path()
{
	if (mWaypointList) {
		delete[] mWaypointList;
	}
}

void Path::allocate(u16 size)
{
	clear();

	mWaypointList = new s16[size];
	mLength       = size;
}

NodeList::NodeList(u32 initialCapacity)
    : mCapacity(initialCapacity)
    , mSize(0)
{
	mNodeList = new PathNode*[mCapacity];
}

NodeList::~NodeList() { delete[] mNodeList; }

void NodeList::add(PathNode* node)
{
	if (mSize >= mCapacity) {
		// Grow array if needed
		u32 newCapacity     = mCapacity * 2;
		PathNode** newNodes = new PathNode*[newCapacity];
		for (u32 i = 0; i < mSize; i++) {
			newNodes[i] = mNodeList[i];
		}
		delete[] mNodeList;
		mNodeList = newNodes;
		mCapacity = newCapacity;
	}
	mNodeList[mSize++] = node;
}

void NodeList::remove(PathNode* node)
{
	for (u32 i = 0; i < mSize; i++) {
		if (mNodeList[i] == node) {
			// Shift remaining elements left
			for (u32 j = i; j < mSize - 1; j++) {
				mNodeList[j] = mNodeList[j + 1];
			}
			mSize--;
			return;
		}
	}
}

PathNode* NodeList::findLowestFCost()
{
	if (mSize == 0)
		return nullptr;

	PathNode* lowest = mNodeList[0];
	for (u32 i = 1; i < mSize; i++) {
		if (mNodeList[i]->mFinalCost < lowest->mFinalCost) {
			lowest = mNodeList[i];
		}
	}
	return lowest;
}

NodeMap::NodeMap(u32 size)
    : mLength(size)
{
	mBucketList = new NodeMapEntry*[mLength];
	for (u32 i = 0; i < mLength; i++) {
		mBucketList[i] = nullptr;
	}
}

NodeMap::~NodeMap()
{
	for (u32 i = 0; i < mLength; i++) {
		NodeMapEntry* entry = mBucketList[i];
		while (entry) {
			NodeMapEntry* next = entry->mNext;
			delete entry;
			entry = next;
		}
	}
	delete[] mBucketList;
}

void NodeMap::put(s16 key, PathNode* value)
{
	u32 bucket          = (u32)key % mLength;
	NodeMapEntry* entry = mBucketList[bucket];

	while (entry) {
		if (entry->mKey == key) {
			entry->mValue = value;
			return;
		}
		entry = entry->mNext;
	}

	// Create new entry
	entry               = new NodeMapEntry;
	entry->mKey         = key;
	entry->mValue       = value;
	entry->mNext        = mBucketList[bucket];
	mBucketList[bucket] = entry;
}

PathNode* NodeMap::get(s16 key)
{
	u32 bucket          = (u32)key % mLength;
	NodeMapEntry* entry = mBucketList[bucket];

	while (entry) {
		if (entry->mKey == key) {
			return entry->mValue;
		}
		entry = entry->mNext;
	}

	return nullptr;
}

f32 WaypointPathfinder::calculateHeuristic(Game::WayPoint* from, Game::WayPoint* to)
{
	Vector3f diff = to->mPosition - from->mPosition;
	diff.y *= 2500.0f; // Weight Y axis more heavily
	return diff.length();
}

void WaypointPathfinder::reconstructPath(PathNode* endNode, Path& outPath)
{
	// First count nodes
	u16 pathLength    = 0;
	PathNode* current = endNode;
	while (current) {
		pathLength++;
		current = current->mParent;
	}

	// Allocate and fill path
	outPath.allocate(pathLength);
	current = endNode;
	for (s16 i = pathLength - 1; i >= 0; i--) {
		outPath.mWaypointList[i] = current->mWaypointIndex;
		current                  = current->mParent;
	}
}

void WaypointPathfinder::cleanup(NodeList& openList, NodeList& closedList)
{
	for (u32 i = 0; i < openList.mSize; i++) {
		delete openList.mNodeList[i];
	}
	for (u32 i = 0; i < closedList.mSize; i++) {
		delete closedList.mNodeList[i];
	}
}

u16 WaypointPathfinder::findPath(s16 startIdx, s16 destIdx, u32 allowedFlags, bool isDesperate, Path& outPath)
{
	if (startIdx == destIdx) {
		outPath.allocate(1);
		outPath.mWaypointList[0] = startIdx;
		return 1;
	}

	NodeList openList(128);
	NodeList closedList(128);
	NodeMap nodeMap(256);

	// Create start node
	Game::WayPoint* startWP = Game::getWaypointAt(startIdx);
	PathNode* startNode     = new PathNode(startIdx, nullptr, 0.0f, calculateHeuristic(startWP, Game::getWaypointAt(destIdx)));
	openList.add(startNode);
	nodeMap.put(startIdx, startNode);

	while (openList.mSize > 0) {
		PathNode* current = openList.findLowestFCost();

		if (current->mWaypointIndex == destIdx) {
			reconstructPath(current, outPath);
			cleanup(openList, closedList);
			return outPath.mLength;
		}

		openList.remove(current);
		closedList.add(current);

		// Process neighbors using WayPointIterator
		Game::WayPoint* currentWP = Game::getWaypointAt(current->mWaypointIndex);
		Game::WayPointIterator wpIter(currentWP, true); // true to consider both directions

		CI_LOOP(wpIter)
		{
			s16 neighborIdx = *wpIter;

			// Skip if in closed list
			if (closedList.contains(neighborIdx)) {
				continue;
			}

			Game::WayPoint* neighborWP = Game::getWaypointAt(neighborIdx);
			if (!neighborWP)
				continue;

			// Is this waypoint on a slippery slope?
			if (!isDesperate) {
				Game::CurrTriInfo triangleAtWp;
				triangleAtWp.mPosition          = neighborWP->mPosition;
				triangleAtWp.mGetTopPolygonInfo = false;
				Game::mapMgr->getCurrTri(triangleAtWp);
				if (!triangleAtWp.mTriangle || triangleAtWp.mTriangle->mCode.getSlipCode() != MapCode::Code::SlipCode_NoSlip) {
					continue;
				}

				// Check path restrictions
				if ((allowedFlags & Game::PATHFLAG_RequireOpen) && (neighborWP->mFlags & Game::WPF_Closed)) {
					continue; // Skip closed waypoints when open is required
				}

				if ((allowedFlags & Game::PATHFLAG_AllowUnvisited) && (neighborWP->mFlags & Game::WPF_Unvisited)) {
					continue; // Skip unvisited waypoints when only allowing unvisited
				}
			}

			if (!(allowedFlags & Game::PATHFLAG_PathThroughWater) && (neighborWP->mFlags & Game::WPF_Water)) {
				continue; // Skip water waypoints when water paths are disallowed
			}

			if ((neighborWP->mFlags & Game::WPF_Water) && (allowedFlags & Game::PATHFLAG_DisallowUnfinishedBridges)
			    && (neighborWP->mFlags & Game::WPF_Bridge)) {
				continue; // Skip unfinished bridges in water
			}

			if ((allowedFlags & Game::PATHFLAG_DisallowVsRed) && (neighborWP->mFlags & Game::WPF_VersusRed)) {
				continue; // Skip red versus waypoints when disallowed
			}

			if ((allowedFlags & Game::PATHFLAG_DisallowVsBlue) && (neighborWP->mFlags & Game::WPF_VersusBlue)) {
				continue; // Skip blue versus waypoints when disallowed
			}

			f32 newGCost           = current->mGroundCost + calculateHeuristic(currentWP, neighborWP);
			PathNode* neighborNode = nodeMap.get(neighborIdx);

			if (!neighborNode || newGCost < neighborNode->mGroundCost) {
				f32 hCost = calculateHeuristic(neighborWP, Game::getWaypointAt(destIdx));

				if (!neighborNode) {
					neighborNode = new PathNode(neighborIdx, current, newGCost, hCost);
					nodeMap.put(neighborIdx, neighborNode);
					openList.add(neighborNode);
				} else {
					neighborNode->mParent        = current;
					neighborNode->mGroundCost    = newGCost;
					neighborNode->mHeuristicCost = hCost;
					neighborNode->mFinalCost     = newGCost + hCost;
				}
			}
		}
	}

	// No path found at all
	outPath.allocate(0);
	return 0;
}
} // namespace Drought
