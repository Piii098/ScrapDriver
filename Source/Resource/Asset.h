#pragma once
#include <string>

class Asset {
public:


	virtual bool Load(const std::string& path) = 0;
	virtual bool Unload() = 0;

private:

};