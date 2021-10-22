#pragma once
#include <vector>
#include "Mesh/BoundingBox.h"
#include "Mesh/Primitive.h"
#include <memory>

enum TKdNodeFlag
{
	SplitX,
	SplitY,
	SplitZ,
	Leaf
};

struct TkdTreeBulidNode
{
public:
	void InitLeaf(const std::vector<int>& Indices)
	{
		Flag = TKdNodeFlag::Leaf;
		PrimitiveIndices = Indices;
	}

	void InitInterior(TKdNodeFlag SplitAxis, const TBoundingBox& InBounds, float InSplitPos, std::unique_ptr<TkdTreeBulidNode>& Below,
		std::unique_ptr<TkdTreeBulidNode>& Above)
	{
		Flag = SplitAxis;
		Bounds = InBounds;
		SplitPos = InSplitPos;
		BelowChild = std::move(Below);
		AboveChild = std::move(Above);
	}

	bool IsLeafNode() const
	{
		return Flag == TKdNodeFlag::Leaf;
	}

public:
	TKdNodeFlag Flag;

	// For interior node
	TBoundingBox Bounds;

	float SplitPos;

	std::unique_ptr<TkdTreeBulidNode> BelowChild = nullptr;

	std::unique_ptr<TkdTreeBulidNode> AboveChild = nullptr;

	// For leaf node
	std::vector<int> PrimitiveIndices;
};

struct TkdTreeBoundEdge
{
public:
	enum class EdgeType 
	{ 
		Start, 
		End 
	};

	TkdTreeBoundEdge(EdgeType InType, float InPos, int InPrimitiveIndex)
		:Type(InType), Pos(InPos), PrimitiveIndex(InPrimitiveIndex)
	{}

public:
	EdgeType Type;

	float Pos;

	int PrimitiveIndex;
};

struct TKdTreeLinearNode
{
public:
	bool IsLeafNode() const
	{
		return Flag == TKdNodeFlag::Leaf;
	}

public:
	TKdNodeFlag Flag;

	// For interior node
	TBoundingBox Bounds;

	float SplitPos;

	int AboveChildOffset = -1;

	// For leaf node
	std::vector<int> PrimitiveIndices;
};

struct TkdTreeNodeToVisit
{
	TkdTreeNodeToVisit(int Index, float Min, float Max)
		:NodeIndex(Index), tMin(Min), tMax(Max)
	{}

	int NodeIndex;

	float tMin, tMax;
};

class TWorld;

class TKdTreeAccelerator
{
public:
	TKdTreeAccelerator(std::vector<std::shared_ptr<TPrimitive>> AllPrimtives, int InMaxDepth = -1);

	void DebugKdTree(TWorld* World) const;

	void DebugFlattenKdTree(TWorld* World) const;

	bool Intersect(const TRay& Ray, float& Dist, bool& bBackFace) const;

	// Just for debug
	bool IntersectBruteForce(const TRay& Ray, float& Dist, bool& bBackFace) const;

private:
	std::unique_ptr<TkdTreeBulidNode> RecursiveBuild(const TBoundingBox& NodeBounds, const std::vector<int>& PrimitiveIndices,
		int Depth, int& OutTotalNodes);

	bool FindBestSplit(const TBoundingBox& NodeBounds, const std::vector<int>& PrimitiveIndices, int& SplitAxis, int& SplitOffset, 
		std::vector<TkdTreeBoundEdge>& SplitEdges);

	int FlattenKdTree(std::unique_ptr<TkdTreeBulidNode>& Node, int& Offset);

	TColor MapDepthToColor(int Depth) const;

	void DebugBuildNode(TWorld* World, TkdTreeBulidNode* Node, int Depth) const;

private:
	std::vector<std::shared_ptr<TPrimitive>> Primitives;

	std::vector<TBoundingBox> PrimitiveBounds;

	std::unique_ptr<TkdTreeBulidNode> RootNode = nullptr;

	std::vector<TKdTreeLinearNode> LinearNodes;

	const int MaxPrimsInNode = 1;

	int MaxDepth = -1;

	const float EmptyBonus = 0.5f;

	const int TraversalCost = 1;

	const int IsectCost = 80;
};
