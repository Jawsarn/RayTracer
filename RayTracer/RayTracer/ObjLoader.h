#pragma once
#include "GraphicStructures.h"
#include <string>
#include <vector>
#include <stdint.h>

struct ObjObject
{
    std::vector<uint32_t> vertexIndices;
    std::vector<uint32_t> uvIndices;
    std::vector<uint32_t> normalIndices;
    std::string materialName;
};

class ObjLoader
{
public:
    ObjLoader();
    ~ObjLoader();
    void Load(const std::string & p_fileName, std::vector<Vertex>& o_vertices, std::vector<ObjMaterial>& o_material);




private:
    void ReadMaterial(std::string & p_materialFileName, std::vector<ObjMaterial>& o_material);
    int materialID;
};
