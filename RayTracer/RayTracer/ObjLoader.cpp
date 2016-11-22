#include "ObjLoader.h"


using namespace DirectX;

ObjLoader::ObjLoader()
{
}

ObjLoader::~ObjLoader()
{
}

void ObjLoader::Load(const std::string &p_fileName, std::vector<Vertex> &o_vertices, Material &o_material)
{
    // Values
    std::vector<XMFLOAT3> vertices;
    std::vector<XMFLOAT2> uvs;
    std::vector<XMFLOAT3> normals;

    // Indicies for later construction
    std::vector<uint32_t> vertexIndices;
    std::vector<uint32_t> uvIndices;
    std::vector<uint32_t> normalIndices;

    //Load material from
    std::string path = "";

    size_t lastSlash = p_fileName.find_last_of("/");
    if (lastSlash != std::string::npos)
    {
        path = p_fileName.substr(0, lastSlash + 1);
    }
    std::string materialFileName;


    FILE * file;
    fopen_s(&file, p_fileName.c_str(), "r");
    
    if (file == NULL)
    {
        printf("Impossible to open the file !\n");
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

            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
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
    }

    size_t numVertices = vertexIndices.size();
    if (numVertices)
    {
        for (size_t k = 0; k < numVertices; k++)
        {
            uint32_t vertexIndex = vertexIndices[k];
            uint32_t normalIndex = normalIndices[k];
            uint32_t uvIndex = uvIndices[k];

            // Create new Vertex
            Vertex newVertex;
            newVertex.position = vertices[vertexIndex - 1];
            newVertex.normal = normals[normalIndex - 1];
            newVertex.texcoord = uvs[uvIndex - 1];

            o_vertices.push_back(newVertex);
        }
    }
    


    o_material = ReadMaterial(materialFileName);
    fclose(file);
}

Material ObjLoader::ReadMaterial(std::string &p_materialFileName)
{
    Material r_material;
    r_material.Ambient = XMFLOAT3(0, 0, 0);
    r_material.Diffuse = XMFLOAT3(0, 0, 0);
    r_material.Specular = XMFLOAT3(0, 0, 0);
    r_material.diffuseTexture = "";

    FILE * file;
    fopen_s(&file, p_materialFileName.c_str(), "r");

    if (file == NULL)
    {
        printf("Impossible to open the file !\n");
        return r_material;
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
        else if (strcmp(lineHeader, "map_Kd") == 0)
        {
            char temp[128];
            int outp = fscanf_s(file, "%s\n", temp, _countof(temp));
            if (outp != 1)
            {
                return r_material;
                //I guess? 
            }
            r_material.diffuseTexture = std::string(temp);
        }
    }

    return r_material;
}
