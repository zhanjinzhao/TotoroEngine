#pragma once

#include "D3D12/D3D12Utils.h"
#include <unordered_map>

class TInputLayoutManager
{
public:
	void AddInputLayout(const std::string& Name, const std::vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout);

	void GetInputLayout(const std::string Name, std::vector<D3D12_INPUT_ELEMENT_DESC>& OutInputLayout) const;

private:
	std::unordered_map<std::string/*Name*/, std::vector<D3D12_INPUT_ELEMENT_DESC>> InputLayoutMap;
};
