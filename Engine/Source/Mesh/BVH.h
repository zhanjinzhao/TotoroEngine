#pragma once

#include "Component/MeshComponent.h"

struct TBVHPrimitiveInfo
{
public:
	TBVHPrimitiveInfo(size_t InPrimitiveIdx, TBoundingBox InBounds)
		:PrimitiveIdx(InPrimitiveIdx),
		Bounds(InBounds),
		Centroid(InBounds.GetCenter())
	{
	}

public:
	size_t PrimitiveIdx = 0;

	TBoundingBox Bounds;

	TVector3 Centroid = TVector3::Zero;
};

struct TBVHBulidNode
{
public:
	void InitLeaf(int First, int Count, const TBoundingBox& InBounds)
	{
		Bounds = InBounds;
		FirstPrimOffset = First;
		PrimitiveCount = Count;
	}

	void InitInterior(int Axis, std::unique_ptr<TBVHBulidNode>& Left, std::unique_ptr<TBVHBulidNode>& Right)
	{
		Bounds = TBoundingBox::Union(Left->Bounds, Right->Bounds);
		SplitAxis = Axis;
		LeftChild = std::move(Left);
		RightChild = std::move(Right);	
	}

	bool IsLeafNode() { return PrimitiveCount > 0; }

public:
	TBoundingBox Bounds;

	// For interior node
	int SplitAxis = -1;

	std::unique_ptr<TBVHBulidNode> LeftChild = nullptr;

	std::unique_ptr<TBVHBulidNode> RightChild = nullptr;

	// For leaf node
	int FirstPrimOffset = -1;

	int PrimitiveCount = -1;
};

struct TBVHBucketInfo 
{
	int Count = 0;

	TBoundingBox Bounds;
};

struct TBVHLinearNode
{
public:
	bool IsLeafNode() { return PrimitiveCount > 0; }

public:
	TBoundingBox Bounds;

	// For interior node
	int SplitAxis = -1;

	int SecondChildOffset = -1;

	// For leaf node
	int FirstPrimOffset = -1;    

	int PrimitiveCount = -1;
};

class TWorld;

class TBVHAccelerator
{
public:
	enum class ESplitMethod 
	{ 
		Middle, 
		EqualCounts,
		SAH
	};

	TBVHAccelerator(const std::vector<TMeshComponent*>& MeshComponents);

	void DebugBVHTree(TWorld* World);

	void DebugFlattenBVH(TWorld* World);

private:
	std::unique_ptr<TBVHBulidNode> RecursiveBuild(std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList, int Start, int End, int& OutTotalNodes,
		std::vector<TMeshComponent*>& OrderedPrimitives);

	int PartitionMiddleMethod(const TBoundingBox& CentroidBounds, int SplitAxis, std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList, 
		int Start, int End);

	int PartitionEqualCountsMethod(const TBoundingBox& CentroidBounds, int SplitAxis, std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList,
		int Start, int End);

	int PartitionSAHMethod(const TBoundingBox& Bounds, const TBoundingBox& CentroidBounds, int SplitAxis, std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList,
		int Start, int End, float& OutCost);

	void CreateLeafNode(std::unique_ptr<TBVHBulidNode>& OutLeftNode, const TBoundingBox& Bounds, std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList,
		int Start, int End, std::vector<TMeshComponent*>& OrderedPrimitives);

	int FlattenBVHTree(std::unique_ptr<TBVHBulidNode>& Node, int& Offset);

	TColor MapDepthToColor(int Depth);

	void DebugBuildNode(TWorld* World, TBVHBulidNode* Node, int Depth);

private:
	ESplitMethod SplitMethod = ESplitMethod::SAH;

	bool bTryAllAxisForSAH = true;

	const int MaxPrimsInNode = 1;

	std::vector<TMeshComponent*> CachePrimitives;

	std::unique_ptr<TBVHBulidNode> RootNode = nullptr;

	std::vector<TBVHLinearNode> LinearNodes;
};
