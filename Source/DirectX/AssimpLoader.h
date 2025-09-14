#pragma once
#include "Framework.h"

struct ImportSettings {
	const wchar_t* filename = nullptr;
	std::vector<class Mesh>& meshes;
	bool inverseU = false;
	bool inverseV = false;
};

class AssimpLoader {
public:
	bool Load(ImportSettings settings);

private:
	void LoadMesh(class Mesh& mesh, const struct aiMesh* aiMeshm, bool inverseU, bool inversV);
	void LoadTexture(const wchar_t* filePath, class Mesh& dst, const struct aiMaterial* src);
};
