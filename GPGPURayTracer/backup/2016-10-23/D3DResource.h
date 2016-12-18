#pragma once
#include "Graphics.h"
#include <sstream>

class D3DResource
{
public:
	ID3D11Resource				*g_resource;
	ID3D11ShaderResourceView	*g_srv;
	UINT g_srvStartSlot;
	virtual void setShaderResourceView(Graphics *g)
	{
		g->getDeviceContext()->CSSetShaderResources(g_srvStartSlot, 1, &g_srv);
	}
protected:

	//convert from string to wstring
	std::wstring widenString(const std::string& str)
	{
		std::wostringstream wstm;
		const std::ctype<wchar_t>& ctfacet =
			std::use_facet< std::ctype<wchar_t> >(wstm.getloc());
		for (size_t i = 0; i<str.size(); ++i)
			wstm << ctfacet.widen(str[i]);
		return wstm.str();
	}

	//convert from wstring to string
	std::string narrowString(const std::wstring& str)
	{
		std::ostringstream stm;
		const std::ctype<char>& ctfacet =
			std::use_facet< std::ctype<char> >(stm.getloc());
		for (size_t i = 0; i<str.size(); ++i)
			stm << ctfacet.narrow((std::ctype<char>::_Elem)str[i], 0);
		return stm.str();
	}

	//convert from string vector to wstring vector
	std::vector<std::wstring> widenStringArray(std::vector<std::string> strings)
	{
		std::vector<std::wstring> wstrings;
		for (unsigned int i = 0; i < strings.size(); i++)
			wstrings.push_back(widenString(strings[i]));
		return wstrings;
	}

	//convert from string vector to wstring vector
	std::vector<std::string> narrowStringArray(std::vector<std::wstring> wstrings)
	{
		std::vector<std::string> strings;
		for (unsigned int i = 0; i < wstrings.size(); i++)
			strings.push_back(narrowString(wstrings[i]));
		return strings;
	}
};