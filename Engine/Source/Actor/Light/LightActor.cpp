#include "LightActor.h"

TLightActor::TLightActor(const std::string& Name, ELightType InType)
	:TActor(Name), Type(InType)
{

}

TLightActor::~TLightActor()
{

}

