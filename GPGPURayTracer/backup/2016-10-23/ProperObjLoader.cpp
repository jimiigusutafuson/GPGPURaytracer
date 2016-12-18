#include "ProperObjLoader.h"

#include <windows.h>

ProperObjLoader::ProperObjLoader()
{

}


ProperObjLoader::~ProperObjLoader()
{
}

bool ProperObjLoader::loadObj(const char *path, bool forceTriangleMesh)
{
	//extract the path
	string temp = path;
	pathNoName.clear();

	int i;
	for (i = temp.size() - 1; i >= 0; i--)
	{
		if (temp.at(i) == '/')
			break;
	}
	for (int k = 0; k <= i; k++)
		pathNoName.push_back(temp.at(k));

	FILE *file = NULL;
	fopen_s(&file, path, "r");
	if (file == NULL) {
		printf("Unable to open obj file %s\n", path);
		return false;
	}

	g_indexGroups.clear();
	g_indexGroupNames.clear();
	g_indexGroups.push_back(IndexGroup());
	g_indexGroupNames.push_back("default");

	typeFlag = 0;
	currentS = 0;
	currentG = 0;
	currentM = -1;

	const int headerSize = 128;
	char lineHeader[headerSize];
	while (1)
	{
		
		// read the first word of the line
		int res = fscanf_s(file, "%s", lineHeader, headerSize);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.
		if (!readObjLine(lineHeader, headerSize, file, pathNoName, forceTriangleMesh))
			return false;
	}
	if (g_indexGroups.size() > 0)
		g_indexGroups.back().iEnd = g_indices.size() - 1;
	fclose(file);
	return true;
}

bool ProperObjLoader::loadMtl(const char *path, string pathNoName)
{
	pathNoName.append(path);
	FILE *file = NULL;
	fopen_s(&file, pathNoName.c_str(), "r");
	if (file == NULL) {
		printf("Unable to open mtl file %s\n", pathNoName);
		MessageBox(0, "Unable to open mtl file!", 0, 0);
		return false;
	}
	const int headerSize = 128;
	char lineHeader[headerSize];
	while (1)
	{

		// read the first word of the line
		int res = fscanf_s(file, "%s", lineHeader, headerSize);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.
		if (!readMtlLine(lineHeader, headerSize, file))
			return false;
	}
	fclose(file);
	return true;
}

bool ProperObjLoader::readObjLine(char *lineHeader, int size, FILE *file, string pathNoName, bool forceTriangleMesh)
{
	if (strcmp(lineHeader, "v") == 0) //vertex
	{
		XMFLOAT3 v;
		fscanf_s(file, "%f %f %f\n", &v.x, &v.y, &v.z);
		g_vertices.push_back(v);
	}
	else if (strcmp(lineHeader, "vt") == 0) //texture coordinate
	{
		XMFLOAT2 uv;
		fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
		g_UVs.push_back(uv);
	}
	else if (strcmp(lineHeader, "vn") == 0) //normal
	{
		XMFLOAT3 v;
		fscanf_s(file, "%f %f %f\n", &v.x, &v.y, &v.z);
		g_normals.push_back(v);
	}
	else if (strcmp(lineHeader, "vp") == 0) //parameter space vertex
	{
		printf("parameter space vertices not supported\n");
	}
	else if (strcmp(lineHeader, "s") == 0) //specular intensity
	{
		fscanf_s(file, "%d\n", &currentS);
	}
	else if (strcmp(lineHeader, "f") == 0) //face
	{
		int vI[6] = { 0,0,0,0,0,0 }, uvI[6] = { 0,0,0,0,0,0 }, nI[6] = { 0,0,0,0,0,0 };
		int indexAmount = 0;
		int matches;
		matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d &d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d &d/%d/%d\n",
			&vI[0], &uvI[0], &nI[0], &vI[1], &uvI[1], &nI[1], &vI[2], &uvI[2], &nI[2], &vI[3], &uvI[3], &nI[3], &vI[4], &uvI[4], &nI[4], &vI[5], &uvI[5], &nI[5]);
		if (matches == 1)
		{
			matches = fscanf_s(file, "/%d %d//%d %d//%d &d//%d %d//%d %d//%d\n",
				&nI[0], &vI[1], &nI[1], &vI[2], &nI[2], &vI[3], &nI[3], &vI[4], &nI[4], &vI[5], &nI[5]);
			matches += 1;
			indexAmount = matches / 2;
		}
		else if (matches == 2)
		{
			matches = fscanf_s(file, " %d/%d/ %d/%d/ %d/%d/ %d/%d/ %d/%d/\n",
				&vI[1], &uvI[1], &vI[2], &uvI[2], &vI[3], &uvI[3], &vI[4], &uvI[4], &vI[5], &uvI[5]);
			matches += 2;
			indexAmount = matches / 2;
		}
		else
		{
			indexAmount = matches / 3;
		}
		if (indexAmount > 4) //weird format of multiple squares or other shapes
		{
			typeFlag = -1;
			MessageBox(0, "Error reading faces!", 0, 0);
			return false;
		}
		else if (indexAmount == 4) //square type (4 indices create one face)
		{
			if (forceTriangleMesh) //force the square type into 2 triangles
			{
				typeFlag = TRIANGLE_MESH;
				Index i;
				i.pos = vI[0] - 1;
				i.tex = uvI[0] - 1;
				i.norm = nI[0] - 1;
				g_indices.push_back(i);
				i.pos = vI[1] - 1;
				i.tex = uvI[1] - 1;
				i.norm = nI[1] - 1;
				g_indices.push_back(i);
				i.pos = vI[2] - 1;
				i.tex = uvI[2] - 1;
				i.norm = nI[2] - 1;
				g_indices.push_back(i);
				i.pos = vI[2] - 1;
				i.tex = uvI[2] - 1;
				i.norm = nI[2] - 1;
				g_indices.push_back(i);
				i.pos = vI[1] - 1;
				i.tex = uvI[1] - 1;
				i.norm = nI[1] - 1;
				g_indices.push_back(i);
				i.pos = vI[3] - 1;
				i.tex = uvI[3] - 1;
				i.norm = nI[3] - 1;
				g_indices.push_back(i);

				g_smoothing.push_back(currentS); //two triangles, so 2 values for s
				g_smoothing.push_back(currentS);
			}
			else //load square normally
			{
				typeFlag = SQUARE_MESH;
				Index i;
				for (unsigned int k = 0; k < 4; k++)
				{
					i.pos = vI[k] - 1;
					i.tex = uvI[k] - 1;
					i.norm = nI[k] - 1;
					g_indices.push_back(i);
				}
				g_smoothing.push_back(currentS);
			}
		}
		else if (indexAmount == 3) //triangle mesh type (3 indices create a triangle)
		{
			typeFlag = TRIANGLE_MESH;
			Index i;
			for (unsigned int k = 0; k < 3; k++)
			{
				i.pos = vI[k] - 1;
				i.tex = uvI[k] - 1;
				i.norm = nI[k] - 1;
				g_indices.push_back(i);
			}
			g_smoothing.push_back(currentS);
		}
		else
		{
			typeFlag = -1;
			MessageBox(0, "Error reading faces!", 0, 0);
			return false;
		}
	}
	else if (strcmp(lineHeader, "mtllib") == 0) //create material
	{
		const int headerSize = 128;
		char materialPath[headerSize];
		fscanf_s(file, "%s\n", materialPath, headerSize);
		bool mtlLoaded = loadMtl(materialPath, pathNoName);
		if (!mtlLoaded)
			return false;

	}
	else if (strcmp(lineHeader, "g") == 0) //index group
	{
		g_indexGroups.back().iEnd = g_indices.size() - 1; //update previous group
		if (g_indexGroups.back().iEnd - g_indexGroups.back().iStart <= 0) //remove previous group if it is empty
		{
			g_indexGroups.pop_back();
			g_indexGroupNames.pop_back();
		}

		const int headerSize = 128;
		char groupName[headerSize];
		fscanf_s(file, "%s\n", groupName, headerSize);
		g_indexGroupNames.push_back(groupName);
		g_indexGroups.push_back(IndexGroup());
		g_indexGroups.back().iStart = g_indices.size();
		g_indexGroups.back().iEnd = g_indices.size();
		currentG = g_indexGroups.size() - 1;
	}
	else if (strcmp(lineHeader, "usemtl") == 0) //use material
	{
		const int headerSize = 128;
		char materialName[headerSize];
		fscanf_s(file, "%s\n", &materialName, headerSize);
		bool cont = true;
		for (unsigned int i = 0; i < g_materialNames.size() && cont; i++) //check all materials
		{
			if (g_materialNames[i] == materialName) //if the material name matches this name
			{
				g_indexGroups.at(currentG).material = i; //set the current group to have this material
				cont = false;
			}
		}
		if (cont)
		{
			MessageBox(0, "Error finding index group!", 0, 0);
			return false;
		}
	}
	return true;
}

bool ProperObjLoader::readMtlLine(char *lineHeader, int size, FILE *file)
{
	if (strcmp(lineHeader, "Ka") == 0)
		fscanf_s(file, "%f %f %f\n", &g_materials[currentM].ka.x, &g_materials[currentM].ka.y, &g_materials[currentM].ka.z);
	else if (strcmp(lineHeader, "Kd") == 0)
		fscanf_s(file, "%f %f %f\n", &g_materials[currentM].kd.x, &g_materials[currentM].kd.y, &g_materials[currentM].kd.z);
	else if (strcmp(lineHeader, "Ks") == 0)
		fscanf_s(file, "%f %f %f\n", &g_materials[currentM].ks.x, &g_materials[currentM].ks.y, &g_materials[currentM].ks.z);
	else if (strcmp(lineHeader, "Ke") == 0)
		fscanf_s(file, "%f %f %f\n", &g_materials[currentM].ke.x, &g_materials[currentM].ke.y, &g_materials[currentM].ke.z);
	else if (strcmp(lineHeader, "Tf") == 0)
		fscanf_s(file, "%f %f %f\n", &g_materials[currentM].tf.x, &g_materials[currentM].tf.y, &g_materials[currentM].tf.z);
	else if (strcmp(lineHeader, "Tr") == 0)
		fscanf_s(file, "%f\n", &g_materials[currentM].tr);
	else if (strcmp(lineHeader, "Ni") == 0)
		fscanf_s(file, "%f\n", &g_materials[currentM].ni);
	else if (strcmp(lineHeader, "Ns") == 0)
		fscanf_s(file, "%f\n", &g_materials[currentM].ns);
	else if (strcmp(lineHeader, "map_Ka") == 0)
	{
		const int headerSize = 128;
		char map_ka[headerSize];
		fscanf_s(file, "%s\n", &map_ka, headerSize);
		g_map_kaPaths.push_back(pathNoName+map_ka);
		g_materials[currentM].map_ka = g_map_kaPaths.size() - 1;
	}
	else if (strcmp(lineHeader, "map_Kd") == 0)
	{
		const int headerSize = 128;
		char map_kd[headerSize];
		fscanf_s(file, "%s\n", &map_kd, headerSize);
		g_map_kdPaths.push_back(pathNoName + map_kd);
		g_materials[currentM].map_kd = g_map_kdPaths.size() - 1;
	}
	else if (strcmp(lineHeader, "map_Ks") == 0)
	{
		const int headerSize = 128;
		char map_ks[headerSize];
		fscanf_s(file, "%s\n", &map_ks, headerSize);
		g_map_ksPaths.push_back(pathNoName + map_ks);
		g_materials[currentM].map_ks = g_map_ksPaths.size() - 1;
	}
	else if (strcmp(lineHeader, "map_Ke") == 0)
	{
		const int headerSize = 128;
		char map_ke[headerSize];
		fscanf_s(file, "%s\n", &map_ke, headerSize);
		g_map_kePaths.push_back(pathNoName + map_ke);
		g_materials[currentM].map_ke = g_map_kePaths.size() - 1;
	}
	else if (strcmp(lineHeader, "map_Ns") == 0)
	{
		const int headerSize = 128;
		char map_Ns[headerSize];
		fscanf_s(file, "%s\n", &map_Ns, headerSize);
		g_map_nsPaths.push_back(pathNoName + map_Ns);
		g_materials[currentM].map_ns = g_map_nsPaths.size() - 1;
	}
	else if (strcmp(lineHeader, "map_Tr") == 0)
	{
		const int headerSize = 128;
		char map_tr[headerSize];
		fscanf_s(file, "%s\n", &map_tr, headerSize);
		g_map_trPaths.push_back(pathNoName + map_tr);
		g_materials[currentM].map_tr = g_map_trPaths.size() - 1;
	}
	else if (strcmp(lineHeader, "map_Bump") == 0 || strcmp(lineHeader, "map_bump") == 0)
	{
		const int headerSize = 128;
		char map_bump[headerSize];
		fscanf_s(file, "%s\n", &map_bump, headerSize);
		g_map_bumpPaths.push_back(pathNoName + map_bump);
		g_materials[currentM].map_bump = g_map_bumpPaths.size() - 1;
	}
	else if (strcmp(lineHeader, "map_Disp") == 0 || strcmp(lineHeader, "map_disp") == 0)
	{
		const int headerSize = 128;
		char map_disp[headerSize];
		fscanf_s(file, "%s\n", &map_disp, headerSize);
		g_map_dispPaths.push_back(pathNoName + map_disp);
		g_materials[currentM].map_disp = g_map_dispPaths.size() - 1;
	}
	else if (strcmp(lineHeader, "map_Decal") == 0 || strcmp(lineHeader, "map_decal") == 0)
	{
		const int headerSize = 128;
		char map_decal[headerSize];
		fscanf_s(file, "%s\n", &map_decal, headerSize);
		g_map_decalPaths.push_back(pathNoName + map_decal);
		g_materials[currentM].map_decal = g_map_decalPaths.size() - 1;
	}
	else if (strcmp(lineHeader, "map_Normal") == 0 || strcmp(lineHeader, "map_normal") == 0)
	{
		const int headerSize = 128;
		char map_normal[headerSize];
		fscanf_s(file, "%s\n", &map_normal, headerSize);
		g_map_normalPaths.push_back(pathNoName + map_normal);
		g_materials[currentM].map_normal = g_map_normalPaths.size() - 1;
	}
	else if (strcmp(lineHeader, "illum") == 0)
		fscanf_s(file, "%f\n", &g_materials[currentM].illum);
	if (strcmp(lineHeader, "newmtl") == 0)
	{
		const int headerSize = 128;
		char materialName[headerSize];
		fscanf_s(file, "%s\n", materialName, headerSize);
		g_materialNames.push_back(materialName);
		g_materials.push_back(Material());
		currentM++;
	}
	return true;
}

void ProperObjLoader::generateVertexBufferNoIndices(vector<MeshVertex> *pVertices)
{
	pVertices->clear();
	if (g_indices.size() > 0)
	{
		if (g_normals.size() == 0)
			g_normals.push_back(XMFLOAT3(0, 0, 0));
		if (g_UVs.size() == 0)
			g_UVs.push_back(XMFLOAT2(0, 0));
		printf("indices found! Generating vertex vector according to them.\n");
		for (unsigned int i = 0; i < g_indices.size(); i++)
		{
			MeshVertex v;
			v.pos = g_vertices[g_indices[i].pos];
			v.normal = g_normals[max(0,g_indices[i].norm)];
			v.texC = g_UVs[max(0,g_indices[i].tex)];
			pVertices->push_back(v);
		}
	}
	else
	{
		printf("no indices detected. generating based on pure vertex data.\n");
		for (unsigned int i = 0; i < g_vertices.size(); i++)
		{
			MeshVertex v;
			v.pos = g_vertices[i];
			pVertices->push_back(v);
		}
		for (unsigned int i = 0; i < g_normals.size() && i < pVertices->size(); i++)
		{
			pVertices->at(i).normal = g_normals[i];
		}
		for (unsigned int i = 0; i < g_UVs.size() && i < pVertices->size(); i++)
		{
			pVertices->at(i).texC = g_UVs[i];
		}
	}
}

void ProperObjLoader::clear()
{
	g_indexGroupNames.clear();
	g_indexGroups.clear();
	g_indices.clear();
	g_map_bumpPaths.clear();
	g_map_decalPaths.clear();
	g_map_dispPaths.clear();
	g_map_kaPaths.clear();
	g_map_kdPaths.clear();
	g_map_kePaths.clear();
	g_map_ksPaths.clear();
	g_map_normalPaths.clear();
	g_map_nsPaths.clear();
	g_map_trPaths.clear();
	g_materialNames.clear();
	g_materials.clear();
	g_normals.clear();
	g_smoothing.clear();
	g_UVs.clear();
	g_vertices.clear();
	typeFlag = 0;
	currentS = 0;
	currentG = 0;
	currentM = -1;
}