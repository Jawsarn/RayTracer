#include "ObjLoader.h"


using namespace DirectX;

ObjLoader::ObjLoader()
{
    materialID = 0;
}

ObjLoader::~ObjLoader()
{
}

void ObjLoader::Load(const std::string &p_fileName, std::vector<Vertex> &o_vertices, std::vector<ObjMaterial> &o_material)
{
    // Values
    std::vector<XMFLOAT3> vertices;
    std::vector<XMFLOAT2> uvs;
    std::vector<XMFLOAT3> normals;

    std::vector<ObjObject> objObjects;

    ObjObject newObj;
    std::string materialFileName;

    //Load material from
    std::string path = "";

    size_t lastSlash = p_fileName.find_last_of("/");
    if (lastSlash != std::string::npos)
    {
        path = p_fileName.substr(0, lastSlash + 1);
    }


    FILE * file;
    fopen_s(&file, p_fileName.c_str(), "r");
    
    if (file == NULL)
    {
        std::runtime_error("Impossible to open the file !\n");
        return;
    }

    while (true)
    {
        char lineHeader[128];

        // read the first word of the line
        int res = fscanf_s(file, "%s", lineHeader, _countof(lineHeader));
        if (res == EOF)
        {
            break; // EOF = End Of File. Quit the loop.
        }

        if (strcmp(lineHeader, "v") == 0)
        {
            XMFLOAT3 vertex;
            fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            XMFLOAT2 uv;
            fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = 1 - uv.y;
            uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0)
        {
            XMFLOAT3 normal;
            fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0)
        {
            //std::string vertex1, vertex2, vertex3;
            uint32_t vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

            if (matches != 9) {
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return;
            }

            newObj.vertexIndices.push_back(vertexIndex[0]);
            newObj.vertexIndices.push_back(vertexIndex[1]);
            newObj.vertexIndices.push_back(vertexIndex[2]);
            newObj.uvIndices.push_back(uvIndex[0]);
            newObj.uvIndices.push_back(uvIndex[1]);
            newObj.uvIndices.push_back(uvIndex[2]);
            newObj.normalIndices.push_back(normalIndex[0]);
            newObj.normalIndices.push_back(normalIndex[1]);
            newObj.normalIndices.push_back(normalIndex[2]);
        }
        else if (strcmp(lineHeader, "mtllib") == 0)
        {
            char matFileName[128];
            int outp = fscanf_s(file, "%s\n", matFileName, _countof(matFileName));
            if (outp != 1)
            {
                return;
                //I guess? 
            }
            materialFileName = path + std::string(matFileName);
        }
        else if (strcmp(lineHeader, "usemtl") == 0)
        {
            char matFileName[128];
            int outp = fscanf_s(file, "%s\n", matFileName, _countof(matFileName));
            if (outp != 1)
            {
                return;
                //I guess? 
            }
            if (newObj.materialName != "")
            {
                objObjects.push_back(newObj);
                newObj.materialName = "";
                newObj.vertexIndices.clear();
                newObj.normalIndices.clear();
                newObj.uvIndices.clear();
            }
            newObj.materialName = std::string(matFileName);
        }
        //usemtl Sword

    }
    objObjects.push_back(newObj);

    // Read materials
    ReadMaterial(materialFileName, o_material);

    // Create the vertex list from indices
    for (size_t objNum = 0; objNum < objObjects.size(); objNum++)
    {
        int materialID = 0;
        for (size_t i = 0; i < o_material.size(); i++)
        {
            if (strcmp(objObjects[objNum].materialName.c_str(), o_material[i].name.c_str()) == 0)
            {
                materialID = o_material[i].id;
                break;
            }
        }

        size_t numVertices = objObjects[objNum].vertexIndices.size();
        if (numVertices)
        {
            for (size_t k = 0; k < numVertices; k++)
            {
                uint32_t vertexIndex = objObjects[objNum].vertexIndices[k];
                uint32_t normalIndex = objObjects[objNum].normalIndices[k];
                uint32_t uvIndex = objObjects[objNum].uvIndices[k];

                // Create new Vertex
                Vertex newVertex;
                newVertex.position = vertices[vertexIndex - 1];
                newVertex.normal = normals[normalIndex - 1];
                newVertex.texcoord = uvs[uvIndex - 1];
                newVertex.materialID = materialID;
                o_vertices.push_back(newVertex);

                
            }
        }
    }
    



    fclose(file);
}

void ObjLoader::ReadMaterial(std::string &p_materialFileName, std::vector<ObjMaterial> &o_material)
{
    ObjMaterial r_material;
    r_material.name = "";
    r_material.Ambient = XMFLOAT3(0, 0, 0);
    r_material.Diffuse = XMFLOAT3(0, 0, 0);
    r_material.Specular = XMFLOAT3(0, 0, 0);
    r_material.specularFactor = 0;
    r_material.transparency = 0;
    r_material.normalTexture = "";
    r_material.diffuseTexture = "";

    FILE * file;
    fopen_s(&file, p_materialFileName.c_str(), "r");

    if (file == NULL)
    {
        printf("Impossible to open the file !\n");
        return;
    }

    while (true)
    {
        /*
        
            XMFLOAT3 Ambient;
            XMFLOAT3 Diffuse;
            XMFLOAT3 Specular;
            float specular;
            float transparency; // d
            std::string diffuseTexture;
            std::string normalTexture;
        */
        char lineHeader[128];

        // read the first word of the line
        int res = fscanf_s(file, "%s", lineHeader, _countof(lineHeader));
        if (res == EOF)
        {
            break; // EOF = End Of File. Quit the loop.
        }
        else if (strcmp(lineHeader, "newmtl") == 0)
        {
            char matFileName[128];
            int outp = fscanf_s(file, "%s\n", matFileName, _countof(matFileName));
            if (outp != 1)
            {
                return;
                //I guess? 
            }
            if (r_material.name != "") // New material
            {
                o_material.push_back(r_material);
                r_material.name = "";
                r_material.Ambient = XMFLOAT3(0, 0, 0);
                r_material.Diffuse = XMFLOAT3(0, 0, 0);
                r_material.Specular = XMFLOAT3(0, 0, 0);
                r_material.specularFactor = 0;
                r_material.transparency = 0;
                r_material.normalTexture = "";
                r_material.diffuseTexture = "";
            }

            r_material.name = std::string(matFileName);
            r_material.id = materialID;
            materialID++;
        }
        else if (strcmp(lineHeader, "Ks") == 0) //diffuse color
        {
            XMFLOAT3 specCol;
            fscanf_s(file, "%f %f %f\n", &specCol.x, &specCol.y, &specCol.z);
            r_material.Diffuse = specCol;
        }
        else if (strcmp(lineHeader, "Kd") == 0) //diffuse color
        {
            XMFLOAT3 difCol;
            fscanf_s(file, "%f %f %f\n", &difCol.x, &difCol.y, &difCol.z);
            r_material.Diffuse = difCol;
        }
        else if (strcmp(lineHeader, "Ka") == 0) //ambient
        {
            XMFLOAT3 ambCol;
            fscanf_s(file, "%f %f %f\n", &ambCol.x, &ambCol.y, &ambCol.z);
            r_material.Diffuse = ambCol;
        }
        else if (strcmp(lineHeader, "Ns") == 0) //ambient
        {
            float specFac;
            fscanf_s(file, "%f\n", &specFac);
            r_material.specularFactor = specFac;
        }
        else if (strcmp(lineHeader, "d") == 0) //ambient
        {
            float transp;
            fscanf_s(file, "%f\n", &transp);
            r_material.transparency = transp;
        }
        else if (strcmp(lineHeader, "map_Kd") == 0)
        {
            char temp[128];
            int outp = fscanf_s(file, "%s\n", temp, _countof(temp));
            if (outp != 1)
            {
                return;
                //I guess? 
            }
            r_material.diffuseTexture = std::string(temp);
        }
        else if (strcmp(lineHeader, "bump") == 0)
        {
            char temp[128];
            int outp = fscanf_s(file, "%s\n", temp, _countof(temp));
            if (outp != 1)
            {
                return;
                //I guess? 
            }
            r_material.normalTexture = std::string(temp);
        }
    }
    o_material.push_back(r_material);
}
