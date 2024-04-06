#include "lve_model.hpp"
#include "lve_utils.hpp"

//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

//std
#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std {
    template<>
    /// <summary>
    /// Contient des donn�es de vertex telles que position, couleur, normale et coordonn�es de texture.
    /// Poss�de des m�thodes statiques(getBindingDescriptions et getAttributeDescriptions) pour d�crire les donn�es de vertex pour Vulkan
    /// </summary>
    struct hash<lve::LveModel::Vertex> {
        size_t operator()(lve::LveModel::Vertex const& vertex) const {
            size_t seed = 0;
            lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace lve {
    /// <summary>
    /// Prend une r�f�rence � un objet LveDevice et un objet Builder en param�tre.
    ///Appelle les fonctions createVertexBuffers et createIndexBuffers pour cr�er les tampons de vertex et d'indices respectivement
    /// </summary>
    /// <param name="device"></param>
    /// <param name="builder"></param>
    LveModel::LveModel(LveDevice& device, const LveModel::Builder& builder) : lveDevice{ device } {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }
    /// <summary>
    /// D�truit l'objet LveModel.
    ///Les tampons de vertex et d'indices sont d�truits automatiquement car ce sont des objets std::unique_ptr
    /// </summary>
    LveModel::~LveModel() {}

    std::unique_ptr <LveModel> LveModel::createModelFromFile(LveDevice& device, const std::string& filePath) {
        Builder builder{};
        builder.loadModel(filePath);
        std::cout << "Vertex count: " << builder.vertices.size() << "\n";

        return std::make_unique<LveModel>(device, builder);
    }
    /// <summary>
    /// Prend un vecteur de Vertex en param�tre.
    ///Alloue un tampon de vertex sur le GPU apr�s avoir utilis� un tampon temporaire pour transf�rer les donn�es depuis le CPU
    /// </summary>
    /// <param name="vertices"></param>
    void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        LveBuffer stagingBuffer{ lveDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data());

        vertexBuffer = std::make_unique<LveBuffer>(lveDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }
    /// <summary>
    /// Prend un vecteur d'indices en param�tre.
    ///Alloue un tampon d'indices sur le GPU apr�s avoir utilis� un tampon temporaire pour transf�rer les donn�es depuis le CPU.
    /// V�rifie si l'objet LveModel a un tampon d'indices(s'il y a des indices)
    /// </summary>
    /// <param name="indices"></param>
    void LveModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;
        if (!hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        LveBuffer stagingBuffer{ lveDevice, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());

        indexBuffer = std::make_unique<LveBuffer>(lveDevice, indexSize, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }
    /// <summary>
    /// Appelle vkCmdDrawIndexed ou vkCmdDraw en fonction de la pr�sence d'un tampon d'indices
    /// </summary>
    /// <param name="commandBuffer"></param>
    void LveModel::draw(VkCommandBuffer commandBuffer) {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }
    /// <summary>
    /// Appelle vkCmdBindVertexBuffers et vkCmdBindIndexBuffer pour lier les tampons au pipeline de rendu
    /// </summary>
    /// <param name="commandBuffer"></param>
    void LveModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offset[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offset);
        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription>LveModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription>LveModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.push_back({ 0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, position) });
        attributeDescriptions.push_back({ 1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, color) });
        attributeDescriptions.push_back({ 2,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, normal) });
        attributeDescriptions.push_back({ 3,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, uv) });
        return attributeDescriptions;
    }
    /// <summary>
    /// Contient une m�thode loadModel qui utilise la biblioth�que TinyObjLoader pour charger un mod�le � partir d'un fichier OBJ.
    /// Remplit le vecteur de vertices(vertices) et d'indices (indices) � partir des donn�es du fichier OBJ.
    /// Utilise un dictionnaire std::unordered_map pour garantir l'unicit� des vertices
    /// </summary>
    /// <param name="filepath"></param>
    void LveModel::Builder::loadModel(const std::string& filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t > materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            throw std::runtime_error(warn + err);
        }
        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };
                    vertex.color =
                    {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }
                if (index.normal_index >= 0) {
                    vertex.normal =
                    {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }
                if (index.texcoord_index >= 0) {
                    vertex.uv =
                    {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
} //namespace lve