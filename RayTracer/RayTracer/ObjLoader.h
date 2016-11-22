#pragma once
#include "GraphicStructures.h"
#include <string>
#include <vector>
#include <stdint.h>

class ObjLoader
{
public:
    ObjLoader();
    ~ObjLoader();
    void Load(const std::string & p_fileName, std::vector<Vertex>& o_vertices, Material & o_material);


private:
    Material ReadMaterial(std::string & p_materialFileName);
};
