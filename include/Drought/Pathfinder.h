#ifndef _DROUGHT_PATHFINDER_H_
#define _DROUGHT_PATHFINDER_H_

#include "Game/routeMgr.h"
#include "Game/pathfinder.h"

namespace Drought {
struct PathNode;
struct Path;
class WaypointPathfinder;

struct PathNode {
	s16 mWaypointIndex;
	PathNode* mParent;
	f32 mGroundCost;    // Cost from start to this node
	f32 mHeuristicCost; // Estimated cost from this node to goal
	f32 mFinalCost;     // gCost + hCost

	PathNode(s16 idx, PathNode* p, f32 g, f32 h);
};

struct Path {
	s16* mWaypointList; // Array of waypoint indices
	u16 mLength;        // Number of waypoints in path

	Path();
	~Path();

	Path(const Path& other)
	{
		mLength       = other.mLength;
		mWaypointList = new s16[mLength];
		for (u16 i = 0; i < mLength; ++i) {
			mWaypointList[i] = other.mWaypointList[i];
		}
	}

	inline bool hasPath() const { return mLength != 0 && mWaypointList; }

	void allocate(u16 size);
	void clear()
	{
		if (mWaypointList) {
			delete[] mWaypointList;
			mWaypointList = nullptr;
		}

		mLength = 0;
	}

	// The pathfinding is working backwards from the destination to the start.
	// Because of that, we need to reverse the path once we've found it.
	void reverse()
	{
		if (!hasPath()) {
			return;
		}

		for (u16 i = 0; i < mLength / 2; ++i) {
			s16 temp                       = mWaypointList[i];
			mWaypointList[i]               = mWaypointList[mLength - i - 1];
			mWaypointList[mLength - i - 1] = temp;
		}
	}
};

// Replacement for std::vector<PathNode*>
struct NodeList {
	PathNode** mNodeList;
	u32 mCapacity;
	u32 mSize;

	NodeList(u32 initialCapacity);
	~NodeList();

	void add(PathNode* node);
	void remove(PathNode* node);
	PathNode* findLowestFCost();
	bool contains(s16 waypointIndex)
	{
		for (u32 i = 0; i < mSize; i++) {
			if (mNodeList[i]->mWaypointIndex == waypointIndex) {
				return true;
			}
		}
		return false;
	}
};

struct NodeMapEntry {
	s16 mKey;
	PathNode* mValue;
	NodeMapEntry* mNext;
};

struct NodeMap {
	NodeMapEntry** mBucketList;
	u32 mLength;

	NodeMap(u32 size);
	~NodeMap();

	void put(s16 key, PathNode* value);
	PathNode* get(s16 key);
};

class WaypointPathfinder {
public:
	static u16 findPath(s16 startIdx, s16 destIdx, u32 allowedFlags, bool isDesperate, Path& outPath);

private:
	static inline f32 calculateHeuristic(Game::WayPoint* from, Game::WayPoint* to);
	static void reconstructPath(PathNode* endNode, Path& outPath);
	static void cleanup(NodeList& openList, NodeList& closedList);
};

} // namespace Drought

#endif
