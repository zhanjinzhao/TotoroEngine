
struct Data
{
	float3 v1;
	float2 v2;
};

StructuredBuffer<Data> gInputA; 
StructuredBuffer<Data> gInputB; 
RWStructuredBuffer<Data> gOutput;


[numthreads(32, 1, 1)]
void CS(int3 dtid : SV_DispatchThreadID)
{
	gOutput[dtid.x].v1 = gInputA[dtid.x].v1 + gInputB[dtid.x].v1;
	gOutput[dtid.x].v2 = gInputA[dtid.x].v2 + gInputB[dtid.x].v2;
}
