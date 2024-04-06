#include "lve_buffer.hpp"

 // std
#include <cassert>
#include <cstring>

namespace lve {

    /**
     * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
     *
     * @param instanceSize The size of an instance
     * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
     * minUniformBufferOffsetAlignment)
     *
     * @return VkResult of the buffer mapping call
     */

    /// <summary>
    /// Retourne l'alignement minimum n�cessaire pour �tre compatible avec minOffsetAlignment
    /// </summary>
    /// <param name="instanceSize"></param>
    /// <param name="minOffsetAlignment"></param>
    /// <returns></returns>
    VkDeviceSize LveBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }
    /// <summary>
    /// Constructeur de la classe LveBuffer
    /// </summary>
    /// <param name="device"></param>
    /// <param name="instanceSize"></param>
    /// <param name="instanceCount"></param>
    /// <param name="usageFlags"></param>
    /// <param name="memoryPropertyFlags"></param>
    /// <param name="minOffsetAlignment"></param>
    LveBuffer::LveBuffer(LveDevice& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment) : lveDevice{ device }, instanceSize{ instanceSize }, instanceCount{ instanceCount }, usageFlags{ usageFlags }, memoryPropertyFlags{ memoryPropertyFlags } {
        alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        bufferSize = alignmentSize * instanceCount;
        device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
    }
    /// <summary>
    /// Destructeur de la classe LveBuffer
    /// </summary>
    LveBuffer::~LveBuffer() {
        unmap();
        vkDestroyBuffer(lveDevice.getDevice(), buffer, nullptr);
        vkFreeMemory(lveDevice.getDevice(), memory, nullptr);
    }

    /**
     * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
     *
     * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
     * buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the buffer mapping call
     */

    /// <summary>
    /// Mappe une plage de m�moire de ce tampon
    /// </summary>
    /// <param name="size"></param>
    /// <param name="offset"></param>
    /// <returns></returns>
    VkResult LveBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
        assert(buffer && memory && "Called map on buffer before create");
        return vkMapMemory(lveDevice.getDevice(), memory, offset, size, 0, &mapped);
    }

    /**
     * Unmap a mapped memory range
     *
     * @note Does not return a result as vkUnmapMemory can't fail
     */

    /// <summary>
    /// D�sapprouve une plage de m�moire mapp�e
    /// </summary>
    void LveBuffer::unmap() {
        if (mapped) {
            vkUnmapMemory(lveDevice.getDevice(), memory);
            mapped = nullptr;
        }
    }

    /**
     * Copies the specified data to the mapped buffer. Default value writes whole buffer range
     *
     * @param data Pointer to the data to copy
     * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
     * range.
     * @param offset (Optional) Byte offset from beginning of mapped region
     *
     */

    /// <summary>
    /// Copie les donn�es sp�cifi�es dans le tampon mapp�
    /// </summary>
    /// <param name="data"></param>
    /// <param name="size"></param>
    /// <param name="offset"></param>
    void LveBuffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, bufferSize);
        } else {
            char* memOffset = (char*)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    /**
     * Flush a memory range of the buffer to make it visible to the device
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
     * complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the flush call
     */

    /// <summary>
    ///  Flush une plage de m�moire du tampon pour la rendre visible pour le p�riph�rique
    /// </summary>
    /// <param name="size"></param>
    /// <param name="offset"></param>
    /// <returns></returns>
    VkResult LveBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(lveDevice.getDevice(), 1, &mappedRange);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
     * the complete buffer range.
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkResult of the invalidate call
     */

    /// <summary>
    /// Invalide une plage de m�moire du tampon pour la rendre visible pour l'h�te
    /// </summary>
    /// <param name="size"></param>
    /// <param name="offset"></param>
    /// <returns></returns>
    VkResult LveBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(lveDevice.getDevice(), 1, &mappedRange);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param size (Optional) Size of the memory range of the descriptor
     * @param offset (Optional) Byte offset from beginning
     *
     * @return VkDescriptorBufferInfo of specified offset and range
     */

    /// <summary>
    /// Cr�e une structure VkDescriptorBufferInfo pour la plage sp�cifi�e
    /// </summary>
    /// <param name="size"></param>
    /// <param name="offset"></param>
    /// <returns></returns>
    VkDescriptorBufferInfo LveBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{ buffer, offset, size, };
    }

    /**
     * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
     *
     * @param data Pointer to the data to copy
     * @param index Used in offset calculation
     *
     */

    /// <summary>
    /// Copie une quantit� sp�cifi�e de donn�es dans le tampon � un indice donn�
    /// </summary>
    /// <param name="data"></param>
    /// <param name="index"></param>
    void LveBuffer::writeToIndex(void* data, int index) {
        writeToBuffer(data, instanceSize, index * alignmentSize);
    }

    /**
     *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
     *
     * @param index Used in offset calculation
     *
     */

    /// <summary>
    /// Flush la plage de m�moire du tampon � l'indice sp�cifi�
    /// </summary>
    /// <param name="index"></param>
    /// <returns></returns>
    VkResult LveBuffer::flushIndex(int index) {
        return flush(alignmentSize, index * alignmentSize);
    }

    /**
     * Create a buffer info descriptor
     *
     * @param index Specifies the region given by index * alignmentSize
     *
     * @return VkDescriptorBufferInfo for instance at index
     */

    /// <summary>
    /// Cr�e une structure VkDescriptorBufferInfo pour l'instance � l'indice sp�cifi�
    /// </summary>
    /// <param name="index"></param>
    /// <returns></returns>
    VkDescriptorBufferInfo LveBuffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(alignmentSize, index * alignmentSize);
    }

    /**
     * Invalidate a memory range of the buffer to make it visible to the host
     *
     * @note Only required for non-coherent memory
     *
     * @param index Specifies the region to invalidate: index * alignmentSize
     *
     * @return VkResult of the invalidate call
     */

    /// <summary>
    /// Invalide la plage de m�moire du tampon � l'indice sp�cifi�
    /// </summary>
    /// <param name="index"></param>
    /// <returns></returns>
    VkResult LveBuffer::invalidateIndex(int index) {
        return invalidate(alignmentSize, index * alignmentSize);
    }

}  // namespace lve
