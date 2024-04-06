#include "lve_descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace lve {

    // *************** Descriptor Set Layout Builder *********************

    /// <summary>
    /// Ajoute une liaison au descripteur avec les spécifications fournies
    /// </summary>
    /// <param name="binding"></param>
    /// <param name="descriptorType"></param>
    /// <param name="stageFlags"></param>
    /// <param name="count"></param>
    /// <returns></returns>
    LveDescriptorSetLayout::Builder& LveDescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }
    /// <summary>
    /// Construit un objet LveDescriptorSetLayout avec les liaisons ajoutées
    /// </summary>
    /// <returns></returns>
    std::unique_ptr<LveDescriptorSetLayout> LveDescriptorSetLayout::Builder::build() const {
        return std::make_unique<LveDescriptorSetLayout>(lveDevice, bindings);
    }

    // *************** Descriptor Set Layout *********************
    
    /// <summary>
    /// Crée un objet LveDescriptorSetLayout avec les liaisons spécifiées
    /// </summary>
    /// <param name="lveDevice"></param>
    /// <param name="bindings"></param>
    LveDescriptorSetLayout::LveDescriptorSetLayout(LveDevice& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : lveDevice{ lveDevice }, bindings{ bindings } {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(lveDevice.getDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
    /// <summary>
    ///  Libère les ressources allouées
    /// </summary>
    LveDescriptorSetLayout::~LveDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(lveDevice.getDevice(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************
    
    /// <summary>
    /// Ajoute une taille de pool pour un type de descripteur spécifié
    /// </summary>
    /// <param name="descriptorType"></param>
    /// <param name="count"></param>
    /// <returns></returns>
    LveDescriptorPool::Builder& LveDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }
    /// <summary>
    /// Définit les drapeaux du pool de descripteurs
    /// </summary>
    /// <param name="flags"></param>
    /// <returns></returns>
    LveDescriptorPool::Builder& LveDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    /// <summary>
    /// Définit le nombre maximal d'ensembles de descripteurs pouvant être alloués
    /// </summary>
    /// <param name="count"></param>
    /// <returns></returns>
    LveDescriptorPool::Builder& LveDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }
    /// <summary>
    /// Construit un objet LveDescriptorPool avec les paramètres spécifiés
    /// </summary>
    /// <returns></returns>
    std::unique_ptr<LveDescriptorPool> LveDescriptorPool::Builder::build() const {
        return std::make_unique<LveDescriptorPool>(lveDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    /// <summary>
    /// Crée un objet LveDescriptorPool avec les paramètres spécifiés
    /// </summary>
    /// <param name="lveDevice"></param>
    /// <param name="maxSets"></param>
    /// <param name="poolFlags"></param>
    /// <param name="poolSizes"></param>
    LveDescriptorPool::LveDescriptorPool(LveDevice& lveDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes) : lveDevice{ lveDevice } {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(lveDevice.getDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    /// <summary>
    /// Libère les ressources allouées
    /// </summary>
    LveDescriptorPool::~LveDescriptorPool() {
        vkDestroyDescriptorPool(lveDevice.getDevice(), descriptorPool, nullptr);
    }
    /// <summary>
    /// Alloue un ensemble de descripteurs à partir du pool
    /// </summary>
    /// <param name="descriptorSetLayout"></param>
    /// <param name="descriptor"></param>
    /// <returns></returns>
    bool LveDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(lveDevice.getDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }
    /// <summary>
    /// Libère des ensembles de descripteurs du pool
    /// </summary>
    /// <param name="descriptors"></param>
    void LveDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(lveDevice.getDevice(), descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
    }
    /// <summary>
    /// Réinitialise le pool de descripteurs
    /// </summary>
    void LveDescriptorPool::resetPool() {
        vkResetDescriptorPool(lveDevice.getDevice(), descriptorPool, 0);
    }
    /// <summary>
    /// Retourne le pool de descriptor
    /// </summary>
    /// <returns></returns>
    VkDescriptorPool LveDescriptorPool::getDescriptorPool() const {
        return descriptorPool;
    }


    // *************** Descriptor Writer *********************
    
    /// <summary>
    /// Crée un objet LveDescriptorWriter avec un layout de descripteur et un pool
    /// </summary>
    /// <param name="setLayout"></param>
    /// <param name="pool"></param>
    LveDescriptorWriter::LveDescriptorWriter(LveDescriptorSetLayout& setLayout, LveDescriptorPool& pool) : setLayout{ setLayout }, pool{ pool } {}
    /// <summary>
    /// Ajoute une écriture de descripteur pour un tampon
    /// </summary>
    /// <param name="binding"></param>
    /// <param name="bufferInfo"></param>
    /// <returns></returns>
    LveDescriptorWriter& LveDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }
    /// <summary>
    /// Ajoute une écriture de descripteur pour une image
    /// </summary>
    /// <param name="binding"></param>
    /// <param name="imageInfo"></param>
    /// <returns></returns>
    LveDescriptorWriter& LveDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }
    /// <summary>
    /// Alloue un ensemble de descripteurs à partir du pool et effectue les écritures
    /// </summary>
    /// <param name="set"></param>
    /// <returns></returns>
    bool LveDescriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }
    /// <summary>
    /// Effectue les écritures sur un ensemble de descripteurs existant
    /// </summary>
    /// <param name="set"></param>
    void LveDescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.lveDevice.getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

}  // namespace lve
