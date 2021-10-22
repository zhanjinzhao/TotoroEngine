#include "BVH.h"
#include <algorithm>
#include <stack>
#include "World/World.h"

TBVHAccelerator::TBVHAccelerator(const std::vector<TMeshComponent*>& MeshComponents)
{
	// Initialize PrimitiveInfo list
	std::vector<TBVHPrimitiveInfo> PrimitiveInfoList;
	for (size_t i = 0; i < MeshComponents.size(); i++)
	{
		TBoundingBox WorldBound;
		if (MeshComponents[i]->GetWorldBoundingBox(WorldBound))
		{
			PrimitiveInfoList.push_back({i, WorldBound});

			CachePrimitives.push_back(MeshComponents[i]);
		}
	}

	// Build BVH tree
	std::vector<TMeshComponent*> OrderedPrimitives;
	OrderedPrimitives.reserve(PrimitiveInfoList.size());

	int TotalNodes = 0;
	RootNode = RecursiveBuild(PrimitiveInfoList, 0, (int)PrimitiveInfoList.size(), TotalNodes, OrderedPrimitives);

	CachePrimitives.swap(OrderedPrimitives);

	// Compute representation of depth-first traversal of BVH tree
	LinearNodes.resize(TotalNodes);
	int Offset = 0;
	FlattenBVHTree(RootNode, Offset);
}

std::unique_ptr<TBVHBulidNode> TBVHAccelerator::RecursiveBuild(std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList, int Start, int End, int& OutTotalNodes,
	std::vector<TMeshComponent*>& OrderedPrimitives)
{
	assert(Start < End);

	std::unique_ptr<TBVHBulidNode> Node = std::make_unique<TBVHBulidNode>();

	OutTotalNodes++;

	// Compute bounds of all primitives in BVH node
	TBoundingBox Bounds;
	for (int i = Start; i < End; i++)
	{
		Bounds = TBoundingBox::Union(Bounds, PrimitiveInfoList[i].Bounds);
	}

	int PrimitiveCount = End - Start;
	if (PrimitiveCount == 1) // Create leaf node
	{
		CreateLeafNode(Node, Bounds, PrimitiveInfoList, Start, End, OrderedPrimitives);

		return Node;
	}
	else
	{
		// Compute bound of primitive centroids, choose split axis
		TBoundingBox CentroidBounds;
		for (int i = Start; i < End; i++)
		{
			CentroidBounds = TBoundingBox::Union(CentroidBounds, PrimitiveInfoList[i].Bounds);
		}
		int SplitAxis = CentroidBounds.GetWidestAxis();

		// If the width of widest axis equal to 0, then we can't split primitives to two sets
		// so create leaf node
		if (CentroidBounds.Max[SplitAxis] == CentroidBounds.Min[SplitAxis])
		{
			CreateLeafNode(Node, Bounds, PrimitiveInfoList, Start, End, OrderedPrimitives);

			return Node;
		}

		// Partition primitives into two sets and build children
		int Mid = (Start + End) / 2;
		switch (SplitMethod)
		{
			case TBVHAccelerator::ESplitMethod::Middle:
			{
				Mid = PartitionMiddleMethod(CentroidBounds, SplitAxis, PrimitiveInfoList, Start, End);

				if (Mid == Start || Mid == End) // Partition fail, use EqualCounts as an alternative
				{
					Mid = PartitionEqualCountsMethod(CentroidBounds, SplitAxis, PrimitiveInfoList, Start, End);
				}
				
				break;
			}
			case TBVHAccelerator::ESplitMethod::EqualCounts:
			{
				Mid = PartitionEqualCountsMethod(CentroidBounds, SplitAxis, PrimitiveInfoList, Start, End);

				break;
			}	
			case ESplitMethod::SAH:
			{
				if (PrimitiveCount <= 2)
				{
					Mid = PartitionEqualCountsMethod(CentroidBounds, SplitAxis, PrimitiveInfoList, Start, End);
				}
				else
				{
					if (bTryAllAxisForSAH) // Try all axis to find the best partition
					{
						float MinCost = TMath::Infinity;
						int BestAxis = -1, BestMid = -1;
						for (int CurAxis = 0; CurAxis < 3; CurAxis++) // Try all axis, compute their cost
						{
							float CurCost = 0.0f;
							int CurMid = PartitionSAHMethod(Bounds, CentroidBounds, CurAxis, PrimitiveInfoList, Start, End, CurCost);

							if (CurMid != -1 && CurCost < MinCost)
							{
								MinCost = CurCost;
								BestAxis = CurAxis;
								BestMid = CurMid;
							}
						}

						if (BestAxis != -1) // Use the least costly axis we found
						{
							SplitAxis = BestAxis;
							Mid = BestMid;
						}
						else
						{
							Mid = -1;
						}
					}
					else // Only consider the widest axis
					{
						float Cost = 0.0f;
						Mid = PartitionSAHMethod(Bounds, CentroidBounds, SplitAxis, PrimitiveInfoList, Start, End, Cost);
					}


					if (Mid == -1) // Create leaf node
					{
						CreateLeafNode(Node, Bounds, PrimitiveInfoList, Start, End, OrderedPrimitives);

						return Node;
					}

					if (Mid == Start || Mid == End) // Partition fail, use EqualCounts as an alternative
					{
						Mid = PartitionEqualCountsMethod(CentroidBounds, SplitAxis, PrimitiveInfoList, Start, End);
					}
				}

				break;
			}
			default:
				break;
		}

		Node->InitInterior(SplitAxis,
			RecursiveBuild(PrimitiveInfoList, Start, Mid,
				OutTotalNodes, OrderedPrimitives),
			RecursiveBuild(PrimitiveInfoList, Mid, End,
				OutTotalNodes, OrderedPrimitives));

		return Node;
	}
}

int TBVHAccelerator::PartitionMiddleMethod(const TBoundingBox& CentroidBounds, int SplitAxis,
	std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList, int Start, int End)
{
	// Partition primitives through node's midpoint
	float AxisMid = (CentroidBounds.Min[SplitAxis] + CentroidBounds.Max[SplitAxis]) / 2;

	TBVHPrimitiveInfo* MidPtr = std::partition(
		&PrimitiveInfoList[Start], &PrimitiveInfoList[End - 1] + 1,
		[SplitAxis, AxisMid](const TBVHPrimitiveInfo& PrimitiveInfo)
		{
			return PrimitiveInfo.Centroid[SplitAxis] < AxisMid;
		});

	int Mid = int(MidPtr - &PrimitiveInfoList[0]);

	return Mid;
}

int TBVHAccelerator::PartitionEqualCountsMethod(const TBoundingBox& CentroidBounds, int SplitAxis, 
	std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList, int Start, int End)
{
	// Partition primitives into equally-sized subsets
	int Mid = (Start + End) / 2;
	std::nth_element(&PrimitiveInfoList[Start], &PrimitiveInfoList[Mid],
		&PrimitiveInfoList[End - 1] + 1,
		[SplitAxis](const TBVHPrimitiveInfo& a, const TBVHPrimitiveInfo& b)
		{
			return a.Centroid[SplitAxis] < b.Centroid[SplitAxis];
		});

	return Mid;
}

int ComputeSAHBucketIndex(int BucketCount, const TBoundingBox& CentroidBounds, int SplitAxis, const TBVHPrimitiveInfo& PrimitiveInfo)
{
	float AxisWidth = CentroidBounds.Max[SplitAxis] - CentroidBounds.Min[SplitAxis];
	float Offset = PrimitiveInfo.Centroid[SplitAxis] - CentroidBounds.Min[SplitAxis];

	int BucketIdx = int(BucketCount * (Offset / AxisWidth));
	if (BucketIdx == BucketCount)
	{
		BucketIdx = BucketCount - 1;
	}
	assert(BucketIdx >= 0);
	assert(BucketIdx < BucketCount);

	return BucketIdx;
}

int TBVHAccelerator::PartitionSAHMethod(const TBoundingBox& Bounds, const TBoundingBox& CentroidBounds, int SplitAxis,
	std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList, int Start, int End, float& OutCost)
{
	// Allocate BucketInfos for SAH partition buckets
	const int BucketCount = 12;
	TBVHBucketInfo Buckets[BucketCount];

	// Initialize BucketInfos for SAH partition buckets
	for (int i = Start; i < End; ++i)
	{
		int BucketIdx = ComputeSAHBucketIndex(BucketCount, CentroidBounds, SplitAxis, PrimitiveInfoList[i]);

		Buckets[BucketIdx].Count++;
		Buckets[BucketIdx].Bounds = TBoundingBox::Union(Buckets[BucketIdx].Bounds, PrimitiveInfoList[i].Bounds);
	}

	// Compute costs for splitting after each bucket
	float Cost[BucketCount - 1];
	for (int i = 0; i < BucketCount - 1; ++i)
	{
		TBoundingBox Box0, Box1;
		int Count0 = 0, Count1 = 0;
		for (int j = 0; j <= i; ++j) 
		{
			Box0 = TBoundingBox::Union(Box0, Buckets[j].Bounds);
			Count0 += Buckets[j].Count;
		}
		for (int j = i + 1; j < BucketCount; ++j)
		{
			Box1 = TBoundingBox::Union(Box1, Buckets[j].Bounds);
			Count1 += Buckets[j].Count;
		}

		Cost[i] = 1.0f + (Count0 * Box0.GetSurfaceArea() + Count1 * Box1.GetSurfaceArea()) / Bounds.GetSurfaceArea();
	}

	// Find bucket to split at that minimizes SAH metric
	float MinCost = Cost[0];
	int MinCostSplitBucket = 0;
	for (int i = 1; i < BucketCount - 1; ++i)
	{
		if (Cost[i] < MinCost) 
		{
			MinCost = Cost[i];
			MinCostSplitBucket = i;
		}
	}

	OutCost = MinCost;

	// Either split primitives at selected SAH bucket or create leaf
	int PrimitiveCount = End - Start;
	float LeafCost = float(PrimitiveCount);
	if (PrimitiveCount > MaxPrimsInNode || MinCost < LeafCost)
	{
		TBVHPrimitiveInfo* MidPtr = std::partition(
			&PrimitiveInfoList[Start], &PrimitiveInfoList[End - 1] + 1,
			[=](const TBVHPrimitiveInfo& PrimitiveInfo)
			{
				int BucketIdx = ComputeSAHBucketIndex(BucketCount, CentroidBounds, SplitAxis, PrimitiveInfo);

				return BucketIdx <= MinCostSplitBucket;
			});

		int Mid = int(MidPtr - &PrimitiveInfoList[0]);
		return Mid;
	}
	else 
	{
		// Create leaf node

		return -1;
	}
}

void TBVHAccelerator::CreateLeafNode(std::unique_ptr<TBVHBulidNode>& OutLeftNode, const TBoundingBox& Bounds,
	std::vector<TBVHPrimitiveInfo>& PrimitiveInfoList, int Start, int End, std::vector<TMeshComponent*>& OrderedPrimitives)
{
	int FirstPrimOffset = (int)OrderedPrimitives.size();
	for (int i = Start; i < End; ++i)
	{
		size_t PrimitiveIdx = PrimitiveInfoList[i].PrimitiveIdx;
		OrderedPrimitives.push_back(CachePrimitives[PrimitiveIdx]);
	}

	int PrimitiveCount = End - Start;
	OutLeftNode->InitLeaf(FirstPrimOffset, PrimitiveCount, Bounds);
}

int TBVHAccelerator::FlattenBVHTree(std::unique_ptr<TBVHBulidNode>& Node, int& Offset)
{
	TBVHLinearNode& LinearNode = LinearNodes[Offset];
	int MyOffset = Offset;
	Offset++;

	LinearNode.Bounds = Node->Bounds;
	if (Node->IsLeafNode()) // Left node
	{
		assert(Node->LeftChild == nullptr);
		assert(Node->RightChild == nullptr);

		LinearNode.FirstPrimOffset = Node->FirstPrimOffset;
		LinearNode.PrimitiveCount = Node->PrimitiveCount;
	}
	else // Interior node
	{
		LinearNode.SplitAxis = Node->SplitAxis;

		FlattenBVHTree(Node->LeftChild, Offset);

		LinearNode.SecondChildOffset = FlattenBVHTree(Node->RightChild, Offset);
	}

	return MyOffset;
}

void TBVHAccelerator::DebugBVHTree(TWorld* World)
{
	DebugBuildNode(World, RootNode.get(), 0);
}

TColor TBVHAccelerator::MapDepthToColor(int Depth)
{
	TColor Color;
	switch (Depth)
	{
	case 0:
		Color = TColor::Red;
		break;
	case 1:
		Color = TColor::Yellow;
		break;
	case 2:
		Color = TColor::Green;
		break;
	case 3:
		Color = TColor::Blue;
		break;
	default:
		Color = TColor::White;
		break;
	}

	return Color;
}

void TBVHAccelerator::DebugBuildNode(TWorld* World, TBVHBulidNode* Node, int Depth)
{
	if (Node == nullptr)
	{
		return;
	}

	TColor Color = MapDepthToColor(Depth);

	TVector3 Offset = TVector3(0.1f) * float(std::clamp(5 - Depth, 0, 5));

	World->DrawBox3D(Node->Bounds.Min - Offset, Node->Bounds.Max + Offset, Color);

	DebugBuildNode(World, Node->LeftChild.get(), Depth + 1);
	DebugBuildNode(World, Node->RightChild.get(), Depth + 1);
}

void TBVHAccelerator::DebugFlattenBVH(TWorld* World)
{
	if (LinearNodes.empty())
	{
		return;
	}

	int CurrentVisitNodeIdx = 0;
	std::stack<int> NodesToVisit;

	while (true)
	{
		TBVHLinearNode& Node = LinearNodes[CurrentVisitNodeIdx];

		if (Node.IsLeafNode()) // Leaf node
		{
			World->DrawBox3D(Node.Bounds.Min, Node.Bounds.Max, TColor::White);

			if (NodesToVisit.empty())
			{
				break;
			}
			else
			{
				CurrentVisitNodeIdx = NodesToVisit.top();
				NodesToVisit.pop();
			}
		}
		else // Interior node
		{
			World->DrawBox3D(Node.Bounds.Min, Node.Bounds.Max, TColor::Red);

			NodesToVisit.push(Node.SecondChildOffset);
			CurrentVisitNodeIdx++;
		}
	}
}