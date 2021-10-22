#include "KdTree.h"
#include "World/World.h"
#include <algorithm>
#include <stack>
#include <cmath>

TKdTreeAccelerator::TKdTreeAccelerator(std::vector<std::shared_ptr<TPrimitive>> AllPrimtives, int InMaxDepth)
{
	// Initial Primitive and bounds
	TBoundingBox RootBounds;
	for (size_t i = 0; i < AllPrimtives.size(); i++)
	{
		TBoundingBox WorldBound;
		if (AllPrimtives[i]->GetWorldBoundingBox(WorldBound))
		{
			Primitives.push_back(AllPrimtives[i]);
			PrimitiveBounds.push_back(WorldBound);

			RootBounds = TBoundingBox::Union(RootBounds, WorldBound);
		}
	}

	MaxDepth = InMaxDepth;
	if (MaxDepth <= 0)
	{
		MaxDepth = (int)std::round(8 + 1.3f * TMath::Log2Int(int64_t(Primitives.size())));
	}

	// Init PrimitiveIndices for root node
	std::vector<int> RootPrimitiveIndices;
	RootPrimitiveIndices.reserve(Primitives.size());
	for (int i = 0; i < Primitives.size(); i++)
	{
		RootPrimitiveIndices.push_back(i);
	}

	// Recursive bulid kd-tree
	int TotalNodes = 0;
	RootNode = RecursiveBuild(RootBounds, RootPrimitiveIndices, 0, TotalNodes);

	// Compute representation of depth-first traversal of kd tree
	LinearNodes.resize(TotalNodes);
	int Offset = 0;
	FlattenKdTree(RootNode, Offset);
}


std::unique_ptr<TkdTreeBulidNode> TKdTreeAccelerator::RecursiveBuild(const TBoundingBox& NodeBounds, const std::vector<int>& PrimitiveIndices, 
	int Depth, int& OutTotalNodes)
{
	std::unique_ptr<TkdTreeBulidNode> Node = std::make_unique<TkdTreeBulidNode>();

	OutTotalNodes++;

	int PrimitiveCount = int(PrimitiveIndices.size());
	if (PrimitiveCount <= MaxPrimsInNode || Depth == MaxDepth) // Create leaf node
	{
		Node->InitLeaf(PrimitiveIndices);

		return Node;
	}
	else
	{
		// Find the best splitAxis and split position
		int SplitAxis = -1;
		int SplitOffset = -1;
		std::vector<TkdTreeBoundEdge> SplitEdges;
		bool bSuccess = FindBestSplit(NodeBounds, PrimitiveIndices, SplitAxis, SplitOffset, SplitEdges);
		
		if(!bSuccess) //Create leaf node
		{
			Node->InitLeaf(PrimitiveIndices);

			return Node;
		}

		// Classify primitives with respect to split
		std::vector<int> BelowPrimitiveIndices;	
		std::vector<int> AbovePrimitiveIndices;
		for (int i = 0; i < SplitOffset; i++)
		{
			if (SplitEdges[i].Type == TkdTreeBoundEdge::EdgeType::Start)
			{
				BelowPrimitiveIndices.push_back(SplitEdges[i].PrimitiveIndex);
			}
		}
		for (int i = SplitOffset + 1; i < SplitEdges.size(); i++)
		{
			if (SplitEdges[i].Type == TkdTreeBoundEdge::EdgeType::End)
			{
				AbovePrimitiveIndices.push_back(SplitEdges[i].PrimitiveIndex);
			}
		}

		// Recursively initialize children nodes
		float SplitPos = SplitEdges[SplitOffset].Pos;
		TBoundingBox BelowBoundingBox = NodeBounds;
		BelowBoundingBox.Max[SplitAxis] = SplitPos;
		TBoundingBox AboveBoundingBox = NodeBounds;
		AboveBoundingBox.Min[SplitAxis] = SplitPos;

		Node->InitInterior(TKdNodeFlag(SplitAxis), NodeBounds, SplitPos,
			RecursiveBuild(BelowBoundingBox, BelowPrimitiveIndices, Depth + 1, OutTotalNodes),
			RecursiveBuild(AboveBoundingBox, AbovePrimitiveIndices, Depth + 1, OutTotalNodes));
	}

	return Node;
}

bool TKdTreeAccelerator::FindBestSplit(const TBoundingBox& NodeBounds, const std::vector<int>& PrimitiveIndices, int& SplitAxis, 
	int& SplitOffset, std::vector<TkdTreeBoundEdge>& SplitEdges)
{
	SplitAxis = -1;
	SplitOffset = -1;

	int PrimitiveCount = int(PrimitiveIndices.size());

	TVector3 TotalSize = NodeBounds.GetSize();
	float TotalSA = NodeBounds.GetSurfaceArea();
	float InvTotalSA = 1.0f / TotalSA;
	float BestCost = TMath::Infinity;

	for (int Axis = 0; Axis < 3; Axis++)
	{
		int OldSplitAixs = SplitAxis;

		// Initialize edges for axis
		std::vector<TkdTreeBoundEdge> Edges;
		for (int Index : PrimitiveIndices)
		{
			const TBoundingBox& Bound = PrimitiveBounds[Index];

			// Add start and end edge for this primitive
			Edges.push_back(TkdTreeBoundEdge(TkdTreeBoundEdge::EdgeType::Start, Bound.Min[Axis], Index));
			Edges.push_back(TkdTreeBoundEdge(TkdTreeBoundEdge::EdgeType::End, Bound.Max[Axis], Index));
		}

		// Sort edges for axis
		std::sort(Edges.begin(), Edges.end(),
			[](const TkdTreeBoundEdge& Edge0, const TkdTreeBoundEdge& Edge1) -> bool
			{
				if (Edge0.Pos == Edge1.Pos)
					return (int)Edge0.Type < (int)Edge1.Type;
				else
					return Edge0.Pos < Edge1.Pos;
			});

		// Compute cost of all splits for axis to find best
		int BelowCount = 0, AboveCount = PrimitiveCount;
		for (int EdgeIdx = 0; EdgeIdx < Edges.size(); EdgeIdx++)
		{
			TkdTreeBoundEdge& Edge = Edges[EdgeIdx];

			if (Edge.Type == TkdTreeBoundEdge::EdgeType::End)
			{
				AboveCount--;
			}

			float EdgePos = Edge.Pos;
			if (EdgePos > NodeBounds.Min[Axis] && EdgePos < NodeBounds.Max[Axis])
			{
				int OtherAxis0 = (Axis + 1) % 3;
				int OtherAxis1 = (Axis + 2) % 3;

				float BelowSA = 2 * (TotalSize[OtherAxis0] * TotalSize[OtherAxis1] +
					(EdgePos - NodeBounds.Min[Axis]) *
					(TotalSize[OtherAxis0] + TotalSize[OtherAxis1]));
				float AboveSA = 2 * (TotalSize[OtherAxis0] * TotalSize[OtherAxis1] +
					(NodeBounds.Max[Axis] - EdgePos) *
					(TotalSize[OtherAxis0] + TotalSize[OtherAxis1]));

				float pBelow = BelowSA * InvTotalSA;
				float pAbove = AboveSA * InvTotalSA;
				float Bonus = (AboveCount == 0 || BelowCount == 0) ? EmptyBonus : 0;
				float Cost = TraversalCost + IsectCost * (1 - Bonus) * (pBelow * BelowCount + pAbove * AboveCount);

				// Update best split if this is lowest cost so far
				if (Cost < BestCost)
				{
					BestCost = Cost;
					SplitAxis = Axis;
					SplitOffset = EdgeIdx;
				}
			}

			if (Edge.Type == TkdTreeBoundEdge::EdgeType::Start)
			{
				BelowCount++;
			}
		}
		assert(BelowCount == PrimitiveCount && AboveCount == 0);

		if (SplitAxis != OldSplitAixs)
		{
			SplitEdges = Edges;
		}
	}

	//TODO: BAD REFINE

	if ((BestCost > 4 * IsectCost * PrimitiveCount && PrimitiveCount < 16) || SplitAxis == -1) // Create leaf node
	{
		return false;
	}

	return true;
}

int TKdTreeAccelerator::FlattenKdTree(std::unique_ptr<TkdTreeBulidNode>& Node, int& Offset)
{
	TKdTreeLinearNode& LinearNode = LinearNodes[Offset];
	int MyOffset = Offset;
	Offset++;

	LinearNode.Flag = Node->Flag;
	if (Node->IsLeafNode()) // Left node
	{
		assert(Node->BelowChild == nullptr);
		assert(Node->AboveChild == nullptr);

		LinearNode.PrimitiveIndices = Node->PrimitiveIndices;
	}
	else // Interior node
	{
		LinearNode.Bounds = Node->Bounds;

		LinearNode.SplitPos = Node->SplitPos;

		FlattenKdTree(Node->BelowChild, Offset);

		LinearNode.AboveChildOffset = FlattenKdTree(Node->AboveChild, Offset);
	}

	return MyOffset;
}

void TKdTreeAccelerator::DebugKdTree(TWorld* World) const
{
	DebugBuildNode(World, RootNode.get(), 0);
}

TColor TKdTreeAccelerator::MapDepthToColor(int Depth) const
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
		Color = TColor::Cyan;
		break;
	case 4:
		Color = TColor::Blue;
		break;
	default:
		Color = TColor::Magenta;
		break;
	}

	return Color;
}

void TKdTreeAccelerator::DebugBuildNode(TWorld* World, TkdTreeBulidNode* Node, int Depth) const
{
	if (Node == nullptr)
	{
		return;
	}

	TColor Color = MapDepthToColor(Depth);

	TVector3 Offset = TVector3(0.05f) * float(std::clamp(5 - Depth, 0, 5));
	//TVector3 Offset = TVector3::Zero;

	if (Node->IsLeafNode())
	{
		for (int Index : Node->PrimitiveIndices)
		{
			const TBoundingBox& Bounds = PrimitiveBounds[Index];

			World->DrawBox3D(Bounds.Min - Offset, Bounds.Max + Offset, TColor::White);
		}
	}
	else
	{
		World->DrawBox3D(Node->Bounds.Min - Offset, Node->Bounds.Max + Offset, Color);

		DebugBuildNode(World, Node->BelowChild.get(), Depth + 1);
		DebugBuildNode(World, Node->AboveChild.get(), Depth + 1);
	}
}

void TKdTreeAccelerator::DebugFlattenKdTree(TWorld* World) const
{
	if (LinearNodes.empty())
	{
		return;
	}

	int CurrentVisitNodeIdx = 0;
	std::stack<int> NodesToVisit;

	while (true)
	{
		const TKdTreeLinearNode& Node = LinearNodes[CurrentVisitNodeIdx];

		if (Node.IsLeafNode()) // Leaf node
		{
			for (int Index : Node.PrimitiveIndices)
			{
				const TBoundingBox& Bounds = PrimitiveBounds[Index];

				World->DrawBox3D(Bounds.Min, Bounds.Max, TColor::White);
			}

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

			NodesToVisit.push(Node.AboveChildOffset);
			CurrentVisitNodeIdx++;
		}
	}
}

bool TKdTreeAccelerator::Intersect(const TRay& Ray, float& Dist, bool& bBackFace) const
{
	if (LinearNodes.empty())
	{
		return false;
	}

	// Compute initial parametric range of ray inside kd-tree extent
	float tMin, tMax;
	TBoundingBox RootBounds = LinearNodes[0].Bounds;
	if (!RootBounds.Intersect(Ray, tMin, tMax)) 
	{
		return false;
	}

	// Traverse kd-tree nodes in order for ray
	bool Hit = false;
	TVector3 InvDir(1.0f / Ray.Direction.x, 1.0f / Ray.Direction.y, 1.0f / Ray.Direction.z);

	int CurrentNodeIdx = 0;
	std::stack<TkdTreeNodeToVisit> NodesToVisit;

	while (true)
	{
		if (Ray.MaxDist < tMin)
		{
			break;
		}

		const TKdTreeLinearNode& Node = LinearNodes[CurrentNodeIdx];

		if (Node.IsLeafNode()) // Leaf node
		{
			for (int Index : Node.PrimitiveIndices)
			{
				auto Primitive = Primitives[Index];
				if (Primitive->Intersect(Ray, Dist, bBackFace))
				{
					Hit = true;
					Ray.MaxDist = Dist;
				}
			}

			if (NodesToVisit.empty())
			{
				break;
			}
			else
			{
				TkdTreeNodeToVisit NodeToVisit = NodesToVisit.top();
				NodesToVisit.pop();

				CurrentNodeIdx = NodeToVisit.NodeIndex;
				tMin = NodeToVisit.tMin;
				tMax = NodeToVisit.tMax;
			}
		}
		else // Interior node
		{
			// Compute parametric distance along ray to split plane
			int Axis = int(Node.Flag);
			float tPlane = (Node.SplitPos - Ray.Origin[Axis]) * InvDir[Axis];

			// Get node children pointers for ray
			int FirstChildIdx = -1;
			int SecondChildIdx = -1;
			int belowFirst =
				(Ray.Origin[Axis] < Node.SplitPos) ||
				(Ray.Origin[Axis] == Node.SplitPos && Ray.Direction[Axis] <= 0); //???
			if (belowFirst) 
			{
				FirstChildIdx = CurrentNodeIdx + 1;
				SecondChildIdx = Node.AboveChildOffset;
			}
			else 
			{
				FirstChildIdx = Node.AboveChildOffset;
				SecondChildIdx = CurrentNodeIdx + 1;
			}

			// Advance to next child node, possibly enqueue other child
			if (tPlane > tMax || tPlane <= 0)
			{
				CurrentNodeIdx = FirstChildIdx;
			}
			else if (tPlane < tMin)
			{
				CurrentNodeIdx = SecondChildIdx;
			}
			else 
			{	
				CurrentNodeIdx = FirstChildIdx;

				// Enqueue secondChild in todo list
				NodesToVisit.emplace(SecondChildIdx, tPlane, tMax);

				tMax = tPlane;
			}
		}
	}
	
	return Hit;
}

bool TKdTreeAccelerator::IntersectBruteForce(const TRay& Ray, float& Dist, bool& bBackFace) const
{
	bool Hit = false;

	for (auto Primitive : Primitives)
	{
		if (Primitive->Intersect(Ray, Dist, bBackFace))
		{
			Ray.MaxDist = Dist;

			Hit = true;
		}
	}

	return Hit;
}