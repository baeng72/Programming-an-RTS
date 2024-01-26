#include "VulkanEx.h"
#include <fstream>
#include <stb_image.h>
#include <spirv_reflect.h>


#define DESCRIPTOR_POOL_SIZE 0x100 //allocate this for a block
namespace Vulkan{
InstanceBuilder::InstanceBuilder() {

}

InstanceBuilder InstanceBuilder::begin() {
	InstanceBuilder builder;
	return builder;
}

InstanceBuilder& InstanceBuilder::addRequiredExtension(const char* reqExt) {
	if (std::find(requiredExtensions.begin(), requiredExtensions.end(), reqExt) == requiredExtensions.end()) {
		requiredExtensions.push_back(reqExt);
	}
	return *this;
}

InstanceBuilder& InstanceBuilder::addRequiredLayer(const char* reqLayer) {
	if (std::find(requiredLayers.begin(), requiredLayers.end(), reqLayer) == requiredLayers.end()) {
		requiredLayers.push_back(reqLayer);
	}
	return *this;
}



void InstanceBuilder::build(VkInstance& instance) {
	instance = initInstance(requiredExtensions, requiredLayers);
}

VulkanInstance::VulkanInstance(VkInstance instance_) :instance(instance_) {

}
VulkanInstance::~VulkanInstance() {
	cleanupInstance(instance);
}

VulkanSurface::VulkanSurface(VkInstance instance_, VkSurfaceKHR surface_) :instance(instance_), surface(surface_) {
}
VulkanSurface::~VulkanSurface() {
	vkDestroySurfaceKHR(instance, surface, nullptr);
}

VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice physicalDevice_) :physicalDevice(physicalDevice_) {
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
}

VulkanPhysicalDevice::~VulkanPhysicalDevice() {

}

VkSurfaceCapabilitiesKHR VulkanPhysicalDevice::getSurfaceCaps(VkSurfaceKHR surface_) {
	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface_, &surfaceCaps);
	return surfaceCaps;
}

VkFormatProperties VulkanPhysicalDevice::getFormatProperties(VkFormat format_) {
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format_, &formatProperties);
	return formatProperties;
}

std::vector<VkSurfaceFormatKHR> VulkanPhysicalDevice::getSurfaceFormats(VkSurfaceKHR surface_) {
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount, surfaceFormats.data());
	return surfaceFormats;
}

std::vector<VkPresentModeKHR> VulkanPhysicalDevice::getPresentModes(VkSurfaceKHR surface_) {
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_, &presentModeCount, presentModes.data());
	return presentModes;
}

DeviceBuilder::DeviceBuilder(VkPhysicalDevice physicalDevice_, Queues& queues_) :physicalDevice(physicalDevice_), queues(queues_) {

}

DeviceBuilder DeviceBuilder::begin(VkPhysicalDevice physicalDevice_, Queues& queues_) {
	DeviceBuilder builder(physicalDevice_, queues_);
	return builder;
}

DeviceBuilder& DeviceBuilder::addDeviceExtension(const char* devExt) {
	if (std::find(deviceExtensions.begin(), deviceExtensions.end(), devExt) == deviceExtensions.end()) {
		deviceExtensions.push_back(devExt);
	}
	return *this;
}

DeviceBuilder& DeviceBuilder::setFeatures(VkPhysicalDeviceFeatures& features_) {
	features = features_;
	return *this;
}

void DeviceBuilder::build(VkDevice& device_) {
	device_ = initDevice(physicalDevice, deviceExtensions, queues, features);
}

void DeviceBuilder::build(VkDevice& device_, VkQueue& graphicsQueue_, VkQueue& computeQueue_, VkQueue& presentQueue_) {
	device_ = initDevice(physicalDevice, deviceExtensions, queues, features);
	graphicsQueue_ = getDeviceQueue(device_, queues.graphicsQueueFamily);
	computeQueue_ = getDeviceQueue(device_, queues.computeQueueFamily);
	presentQueue_ = getDeviceQueue(device_, queues.presentQueueFamily);
}

VkDevice DeviceBuilder::build() {
	VkDevice device = initDevice(physicalDevice, deviceExtensions, queues, features);
	return device;
}


VulkanDevice::VulkanDevice(VkDevice device_) :device(device_) {

}
VulkanDevice::~VulkanDevice() {
	cleanupDevice(device);
}
VkQueue VulkanDevice::getQueue(uint32_t queueFamily)const {
	return getDeviceQueue(device, queueFamily);
}

SwapchainBuilder::SwapchainBuilder(VkDevice device_, VkSurfaceKHR surface_, VkSurfaceCapabilitiesKHR& surfaceCaps_) :device(device_), surface(surface_) {
	surfaceCaps = surfaceCaps_;
}

SwapchainBuilder SwapchainBuilder::begin(VkDevice device_, VkSurfaceKHR surface_, VkSurfaceCapabilitiesKHR& surfaceCaps_) {
	SwapchainBuilder builder(device_, surface_, surfaceCaps_);
	return builder;
}

SwapchainBuilder& SwapchainBuilder::setPresentMode(VkPresentModeKHR presentMode_) {
	presentMode = presentMode_;
	return *this;
}

SwapchainBuilder& SwapchainBuilder::setFormat(VkSurfaceFormatKHR& swapChainFormat_) {
	swapchainFormat = swapChainFormat_;
	return *this;
}

SwapchainBuilder& SwapchainBuilder::setExtent(VkExtent2D& extent) {
	swapchainExtent = extent;
	return *this;
}

SwapchainBuilder& SwapchainBuilder::setExtent(uint32_t width, uint32_t height) {
	swapchainExtent = { width,height };
	return *this;
}

SwapchainBuilder& SwapchainBuilder::setImageCount(uint32_t imageCount_) {
	imageCount = imageCount_;
	return *this;
}

VkSwapchainKHR SwapchainBuilder::build() {
	VkSwapchainKHR swapchain = initSwapchain(device, surface, swapchainExtent.width, swapchainExtent.height, surfaceCaps, presentMode, swapchainFormat, swapchainExtent, imageCount);
	return swapchain;
}

VulkanSwapchain::VulkanSwapchain(VkDevice device_, VkSwapchainKHR swapchain_) :device(device_), swapchain(swapchain_) {

}

VulkanSwapchain::~VulkanSwapchain() {
	cleanupSwapchain(device, swapchain);
}


std::vector<VkImage> VulkanSwapchain::getImages() {
	std::vector<VkImage> images;
	getSwapchainImages(device, swapchain, images);
	return images;

}



VulkanSwapchainImageViews::VulkanSwapchainImageViews(VkDevice device_, std::vector<VkImageView>& imageViews_) :device(device_), swapchainImageViews(imageViews_) {

}

VulkanSwapchainImageViews::~VulkanSwapchainImageViews() {
	for (auto& imageView : swapchainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}
}

VulkanSemaphore::VulkanSemaphore(VkDevice device_, VkSemaphore semaphore_) :device(device_), semaphore(semaphore_) {

}
VulkanSemaphore::~VulkanSemaphore() {
	cleanupSemaphore(device, semaphore);
}

VulkanCommandPool::VulkanCommandPool(VkDevice device_, VkCommandPool commandPool_) :device(device_), commandPool(commandPool_) {

}
VulkanCommandPool::~VulkanCommandPool() {
	cleanupCommandPool(device, commandPool);
}

VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device_, VkCommandPool commandPool_, VkCommandBuffer commandBuffer_) :device(device_), commandPool(commandPool_), commandBuffer(commandBuffer_) {

}
VulkanCommandBuffer::~VulkanCommandBuffer() {
	cleanupCommandBuffer(device, commandPool, commandBuffer);
}

VulkanCommandBuffers::VulkanCommandBuffers(VkDevice device_, VkCommandPool commandPool_, std::vector<VkCommandBuffer>& commandBuffers_):device(device_),commandPool(commandPool_),commandBuffers(commandBuffers_) {
}

VulkanCommandBuffers::~VulkanCommandBuffers() {
	cleanupCommandBuffers(device, commandPool, commandBuffers);
}

VulkanFence::VulkanFence(VkDevice device_, VkFence fence_) :device(device_), fence(fence_) {

}
VulkanFence::~VulkanFence() {
	cleanupFence(device, fence);
}

DescriptorSetPoolCache::DescriptorSetPoolCache(VkDevice device_) :device(device_) {

}

DescriptorSetPoolCache::~DescriptorSetPoolCache() {
	for (auto& pool : freePools) {
		cleanupDescriptorPool(device, pool);
	}
	for (auto& pool : allocatedPools) {
		cleanupDescriptorPool(device, pool);
	}
}

VkDescriptorPool DescriptorSetPoolCache::getPool() {
	VkDescriptorPool descriptorPool;
	if (freePools.size() > 0) {
		descriptorPool = freePools.back();
		freePools.pop_back();

	}
	else {
		descriptorPool = createPool();
	}
	return descriptorPool;
}

VkDescriptorPool DescriptorSetPoolCache::createPool() {
	//descriptor types currently supported
	std::vector<VkDescriptorPoolSize> sizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,DESCRIPTOR_POOL_SIZE},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,DESCRIPTOR_POOL_SIZE},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,DESCRIPTOR_POOL_SIZE},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,DESCRIPTOR_POOL_SIZE},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,DESCRIPTOR_POOL_SIZE},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,DESCRIPTOR_POOL_SIZE},
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,DESCRIPTOR_POOL_SIZE},
		{VK_DESCRIPTOR_TYPE_SAMPLER,DESCRIPTOR_POOL_SIZE},
		{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,DESCRIPTOR_POOL_SIZE}
	};
	VkDescriptorPool descriptorPool = initDescriptorPool(device, sizes, DESCRIPTOR_POOL_SIZE);
	return descriptorPool;
}

void DescriptorSetPoolCache::resetPools() {
	for (auto p : allocatedPools) {
		vkResetDescriptorPool(device, p, 0);
	}
	freePools = allocatedPools;
	allocatedPools.clear();
	currentPool = VK_NULL_HANDLE;
}

bool DescriptorSetPoolCache::allocateDescriptorSets(VkDescriptorSet* pSets, VkDescriptorSetLayout* pLayouts, uint32_t count) {
	if (currentPool == VK_NULL_HANDLE) {
		currentPool = getPool();
		allocatedPools.push_back(currentPool);
	}
	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.pSetLayouts = pLayouts;
	allocInfo.descriptorSetCount = count;
	allocInfo.descriptorPool = currentPool;
	VkResult res = vkAllocateDescriptorSets(device, &allocInfo, pSets);

	if (res == VK_SUCCESS) {
		return true;
	}
	else if (res == VK_ERROR_FRAGMENTED_POOL || res == VK_ERROR_OUT_OF_POOL_MEMORY) {
		currentPool = getPool();
		allocatedPools.push_back(currentPool);
		res = vkAllocateDescriptorSets(device, &allocInfo, pSets);
		if (res == VK_SUCCESS)
			return true;
	}
	return false;
}

bool DescriptorSetPoolCache::allocateDescriptorSets(VkDescriptorSet* pSets, VkDescriptorSetLayout layout, uint32_t count) {
	if (currentPool == VK_NULL_HANDLE) {
		currentPool = getPool();
		allocatedPools.push_back(currentPool);
	}
	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.pSetLayouts = &layout;
	allocInfo.descriptorSetCount = count;
	allocInfo.descriptorPool = currentPool;
	VkResult res = vkAllocateDescriptorSets(device, &allocInfo, pSets);

	if (res == VK_SUCCESS) {
		return true;
	}
	else if (res == VK_ERROR_FRAGMENTED_POOL || res == VK_ERROR_OUT_OF_POOL_MEMORY) {
		currentPool = getPool();
		allocatedPools.push_back(currentPool);
		res = vkAllocateDescriptorSets(device, &allocInfo, pSets);
		if (res == VK_SUCCESS)
			return true;
	}
	return false;
}

bool DescriptorSetPoolCache::allocateDescriptorSet(VkDescriptorSet* pSet, VkDescriptorSetLayout layout) {
	if (currentPool == VK_NULL_HANDLE) {
		currentPool = getPool();
		allocatedPools.push_back(currentPool);
	}
	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.pSetLayouts = &layout;
	allocInfo.descriptorSetCount = 1;
	allocInfo.descriptorPool = currentPool;
	VkResult res = vkAllocateDescriptorSets(device, &allocInfo, pSet);

	if (res == VK_SUCCESS) {
		return true;
	}
	else if (res == VK_ERROR_FRAGMENTED_POOL || res == VK_ERROR_OUT_OF_POOL_MEMORY) {
		currentPool = getPool();
		allocatedPools.push_back(currentPool);
		res = vkAllocateDescriptorSets(device, &allocInfo, pSet);
		if (res == VK_SUCCESS)
			return true;
	}
	return false;
}


DescriptorSetLayoutCache::DescriptorSetLayoutCache(VkDevice device_) :device(device_) {

}

DescriptorSetLayoutCache::~DescriptorSetLayoutCache() {
	for (auto pair : layoutCache) {
		cleanupDescriptorSetLayout(device, pair.second);
	}
}

VkDescriptorSetLayout DescriptorSetLayoutCache::create(std::vector<VkDescriptorSetLayoutBinding>& bindings) {
	DescriptorSetLayoutInfo layoutInfo;
	uint32_t bindingCount = (uint32_t)bindings.size();
	layoutInfo.bindings.reserve(bindingCount);
	bool isSorted = true;
	int32_t lastBinding = -1;
	for (uint32_t i = 0; i < bindingCount; i++) {
		layoutInfo.bindings.push_back(bindings[i]);
		//check that the bindings are in strict increasing over
		if (static_cast<int32_t>(bindings[i].binding) > lastBinding) {
			lastBinding = bindings[i].binding;
		}
		else {
			isSorted = false;
		}
	}

	if (!isSorted) {
		std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {return a.binding < b.binding; });
	}

	VkDescriptorSetLayout descriptorLayout;
	auto it = layoutCache.find(layoutInfo);
	if (it != layoutCache.end()) {
		descriptorLayout = (*it).second;
	}
	else {
		descriptorLayout = initDescriptorSetLayout(device, bindings);
		layoutCache[layoutInfo] = descriptorLayout;
	}
	return descriptorLayout;
}

bool DescriptorSetLayoutCache::DescriptorSetLayoutInfo::operator==(const DescriptorSetLayoutInfo& other)const {
	if (other.bindings.size() != bindings.size()) {
		return false;//can't be a match
	}
	else {
		//compare each binding. Bindings are sorted, so will match
		for (int i = 0; i < bindings.size(); i++) {
			if (other.bindings[i].binding != bindings[i].binding) {
				return false;
			}
			if (other.bindings[i].descriptorType != bindings[i].descriptorType) {
				return false;
			}
			if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) {
				return false;
			}
			if (other.bindings[i].stageFlags != bindings[i].stageFlags) {
				return false;
			}
		}
	}
	return true;
}

size_t DescriptorSetLayoutCache::DescriptorSetLayoutInfo::hash()const {
	size_t result = std::hash<size_t>()(bindings.size());
	for (const VkDescriptorSetLayoutBinding& b : bindings) {
		size_t bindingHash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;
		result ^= std::hash<size_t>()(bindingHash);
	}
	return result;
}

bool DescriptorSetCache::bindingsMatch(std::vector<VkWriteDescriptorSet>& writes) {
	if (writes.size() != _bindings.size())
		return false;
	for (size_t i = 0; i < writes.size(); i++) {
		if (writes[i].descriptorType != _bindings[i].descriptorType)
			return false;
		if (writes[i].descriptorCount != _bindings[i].descriptorCount)
			return false;
		if (writes[i].dstBinding != _bindings[i].binding)
			return false;
	}
	return true;
}

uint32_t DescriptorSetCache::concathash(uint32_t hash, const void* data, size_t count) {
	unsigned char* p = (unsigned char*)data;
	for (size_t i = 0; i < count; i++) {
		hash ^= p[i];
		hash *= hash * fnvprime32;
	}
	return hash;
}

uint32_t DescriptorSetCache::hashval(const void* data, size_t count) {
	unsigned char* p = (unsigned char*)data;
	uint32_t hash = fnvoffset32;	
	for (size_t i = 0; i < count; i++) {
		hash ^= p[i];
		hash *= hash * fnvprime32;
	}
	return hash;
}

uint32_t DescriptorSetCache::getHash(std::vector<VkWriteDescriptorSet>& writes) {
	uint32_t size =(uint32_t)writes.size();
	uint32_t hash = hashval(&size,sizeof(uint32_t));
	for (auto& write : writes) {
		switch (write.descriptorType) {
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			hash = concathash(hash, &write.pBufferInfo->buffer, sizeof(VkBuffer));
			break;
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		{
			for (uint32_t i = 0; i < write.descriptorCount; i++) {
				hash = concathash(hash, &write.pImageInfo[i].imageView, sizeof(VkImageView));
			}
		}
		break;
		default:
			assert(0);
		}
	}
	return hash;
}

DescriptorSetCache::DescriptorSetCache(VkDescriptorSetLayout layout, DescriptorSetPoolCache* pPoolCache, DescriptorSetLayoutCache* pLayoutCache) :_layout(layout), _pPoolCache(pPoolCache),_pLayoutCache(pLayoutCache) {
	pLayoutCache->getBindings(layout, _bindings);
}

DescriptorSetCache::DescriptorSetCache() :_layout(VK_NULL_HANDLE) {
}
void DescriptorSetCache::init(VkDescriptorSetLayout layout, DescriptorSetPoolCache* pPoolCache, DescriptorSetLayoutCache* pLayoutCache) {
	_layout = layout;
	_pPoolCache = pPoolCache;
	_pLayoutCache = pLayoutCache;
	pLayoutCache->getBindings(layout, _bindings);
}
VkDescriptorSet DescriptorSetCache::getDescriptor(std::vector<VkWriteDescriptorSet>& writes) {
	assert(bindingsMatch(writes));//will be compiled away in release build

	uint32_t hash = getHash(writes);
	VkDescriptorSet set = _cache[hash];
	if (set == VK_NULL_HANDLE) {
		if (_availableDescriptors.size() > 0) {
			set = _availableDescriptors.front();
			_availableDescriptors.pop();
		}
		else {
			_pPoolCache->allocateDescriptorSet(&set, _layout);
		}
		_cache[hash] = set;
		for (auto& write : writes) {
			write.dstSet = set;
		}

		DescriptorSetUpdater::begin(_pLayoutCache, _layout, set)
			.AddBindings(writes)
			.update();
	}
	return set;
}

void DescriptorSetCache::Reset() {
	
	for (auto& pair : _cache) {
		_availableDescriptors.emplace(pair.second);
	}
	_cache.clear();
}

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder( DescriptorSetLayoutCache* pLayout_) : pLayout(pLayout_)
{
}
DescriptorSetLayoutBuilder DescriptorSetLayoutBuilder::begin( DescriptorSetLayoutCache* pLayout_)
{
	DescriptorSetLayoutBuilder builder( pLayout_);
	return builder;
}
DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
{
	VkDescriptorSetLayoutBinding newBinding{ binding,type,descriptorCount,stageFlags,nullptr };
	bindings.push_back(newBinding);
	return *this;
}
DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::AddBinding(VkDescriptorSetLayoutBinding& binding)
{
	bindings.push_back(binding);
	return *this;
}
VkDescriptorSetLayout DescriptorSetLayoutBuilder::build()
{
	VkDescriptorSetLayout layout = pLayout->create(bindings);
	return layout;
}

DescriptorSetBuilder::DescriptorSetBuilder(DescriptorSetPoolCache* pPool_, DescriptorSetLayoutCache* pLayout_) :pPool(pPool_), pLayout(pLayout_) {

}

DescriptorSetBuilder DescriptorSetBuilder::begin(DescriptorSetPoolCache* pPool_, DescriptorSetLayoutCache* pLayout_) {
	DescriptorSetBuilder builder(pPool_, pLayout_);
	return builder;
}

DescriptorSetBuilder& DescriptorSetBuilder::AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t descriptorCount) {
	VkDescriptorSetLayoutBinding newBinding{ binding,type,descriptorCount,stageFlags,nullptr };
	bindings.push_back(newBinding);
	return *this;
}
DescriptorSetBuilder& DescriptorSetBuilder::AddBinding(VkDescriptorSetLayoutBinding&binding){	
	bindings.push_back(binding);
	return *this;
}
bool DescriptorSetBuilder::build(VkDescriptorSet& set, VkDescriptorSetLayout& layout) {
	layout = pLayout->create(bindings);
	return pPool->allocateDescriptorSet(&set, layout);
}

bool DescriptorSetBuilder::build(VkDescriptorSet& set) {
	VkDescriptorSetLayout layout;
	return build(set, layout);
}

bool DescriptorSetBuilder::build(std::vector<VkDescriptorSet>& sets, VkDescriptorSetLayout& layout, uint32_t count) {
	layout = pLayout->create(bindings);
	sets.resize(count);
	std::vector<VkDescriptorSetLayout> layouts(count, layout);
	return pPool->allocateDescriptorSets(sets.data(), layouts.data(), count);
}

bool DescriptorSetBuilder::build(std::vector<VkDescriptorSet>& sets, uint32_t count) {
	VkDescriptorSetLayout layout;
	return build(sets, layout, count);
}

void DescriptorSetUpdater::setBindings() {
	bool ok=pLayout->getBindings(descriptorSetLayout, bindings);
	assert(ok);
	writes.resize(bindings.size(), { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET });
	for (size_t i = 0; i < bindings.size(); ++i) {
		writes[i].descriptorType = bindings[i].descriptorType;
		writes[i].descriptorCount = bindings[i].descriptorCount;
		writes[i].dstSet = descriptorSet;
	}
}

DescriptorSetUpdater::DescriptorSetUpdater(DescriptorSetLayoutCache* pLayout_, VkDescriptorSetLayout descriptorSetLayout_, VkDescriptorSet descriptorSet_) :pLayout(pLayout_), descriptorSetLayout(descriptorSetLayout_), descriptorSet(descriptorSet_) {

}

DescriptorSetUpdater DescriptorSetUpdater::begin(DescriptorSetLayoutCache* pLayout_, VkDescriptorSetLayout descriptorSetLayout_, VkDescriptorSet descriptorSet_) {
	DescriptorSetUpdater updater(pLayout_, descriptorSetLayout_, descriptorSet_);
	/*updater.pLayout = pLayout_;
	updater.descriptorSetLayout = descriptorSetLayout_;
	updater.descriptorSet = descriptorSet_;*/
	updater.setBindings();
	return updater;
}

DescriptorSetUpdater& DescriptorSetUpdater::AddBinding(uint32_t binding_, VkDescriptorType descriptorType_, VkDescriptorBufferInfo* bufferInfo_) {
	assert(bindings.size() > binding_);
	assert(bindings[binding_].descriptorType == descriptorType_);
	//writes[binding_].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[binding_].dstBinding = binding_;
	writes[binding_].pBufferInfo = bufferInfo_;
	return *this;
}



DescriptorSetUpdater& DescriptorSetUpdater::AddBinding(uint32_t binding_, VkDescriptorType descriptorType_, VkDescriptorImageInfo* imageInfo_, uint32_t count) {
	assert(bindings.size() > binding_);
	assert(bindings[binding_].descriptorType == descriptorType_);
	writes[binding_].pImageInfo = imageInfo_;
	writes[binding_].dstBinding = binding_;
	writes[binding_].descriptorCount = count;
	return *this;
}

DescriptorSetUpdater& DescriptorSetUpdater::AddBindings(std::vector<VkWriteDescriptorSet>& wrs) {
	for (size_t i = 0; i < wrs.size(); i++){
		writes[i].pBufferInfo = wrs[i].pBufferInfo;
		writes[i].pImageInfo = wrs[i].pImageInfo;
		writes[i].dstBinding = wrs[i].dstBinding;
	}
	
	return *this;
}

void DescriptorSetUpdater::update() {
	updateDescriptorSets(*pLayout, writes);
	//vkUpdateDescriptorSets(*pLayout, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

UniformBufferBuilder::UniformBufferBuilder(VkDevice device_, VkPhysicalDeviceProperties& deviceProperties_, VkPhysicalDeviceMemoryProperties& memoryProperties_, VkDescriptorType descriptorType_, bool isMapped_)
	:device(device_), deviceProperties(deviceProperties_), memoryProperties(memoryProperties_), descriptorType(descriptorType_), isMapped(isMapped_) {

}

UniformBufferBuilder UniformBufferBuilder::begin(VkDevice device_, VkPhysicalDeviceProperties& deviceProperties_, VkPhysicalDeviceMemoryProperties& memoryProperties_, VkDescriptorType descriptorType_, bool isMapped_) {
	UniformBufferBuilder builder(device_, deviceProperties_, memoryProperties_, descriptorType_, isMapped_);
	/*builder.device = device_;
	builder.deviceProperties = deviceProperties_;
	builder.memoryProperties = memoryProperties_;
	builder.descriptorType = descriptorType_;
	builder.isMapped = isMapped_;*/
	return builder;
}

UniformBufferBuilder& UniformBufferBuilder::AddBuffer(VkDeviceSize objectSize_, VkDeviceSize objectCount_, VkDeviceSize repeatCount_) {
	bufferInfo.push_back({ objectSize_,objectCount_,repeatCount_,nullptr });
	return *this;
}

void UniformBufferBuilder::build(Buffer& buffer, std::vector<UniformBufferInfo>& bufferInfo_) {
	BufferProperties props;
#ifdef __USE__VMA__
	props.usage = isMapped ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_ONLY;
#else
	props.memoryProps = bufferInfo.isMapped ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
	VkDeviceSize alignment = 256;//jsut a guess 
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	//VkDescriptorType descriptorType = descriptorType;
	switch (descriptorType) {
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		alignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
		break;

	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		alignment = deviceProperties.limits.minStorageBufferOffsetAlignment;
		break;
	}
	bufferInfo_.clear();
	VkDeviceSize totalSize = 0;
	for (auto& binfo : bufferInfo) {


		VkDeviceSize objectSize = (binfo.objectSize + alignment - 1) & ~(alignment - 1);

		VkDeviceSize bufferSize = objectSize * binfo.objectCount;
		UniformBufferInfo newInfo{};
		newInfo.objectSize = objectSize;
		newInfo.objectCount = binfo.objectCount;
		newInfo.repeatCount = binfo.repeatCount;
		bufferInfo_.push_back(newInfo);
		VkDeviceSize bufReptSize = binfo.repeatCount * bufferSize;
		totalSize += bufReptSize;
	}
	props.bufferUsage = usage;
	props.size = totalSize;
	initBuffer(device, memoryProperties, props, buffer);
	void* ptr = nullptr;
	if (isMapped) {
		ptr = mapBuffer(device, buffer);
		VkDeviceSize offset = 0;
		for (auto& binfo : bufferInfo_) {

			binfo.ptr = ((uint8_t*)ptr + offset);
			offset += binfo.objectSize * binfo.objectCount * binfo.repeatCount;
		}
	}
}

VertexBufferBuilder::VertexBufferBuilder(VkDevice device_, VkQueue queue_, VkCommandBuffer cmd_, VkPhysicalDeviceMemoryProperties& memoryProperties_) :device(device_), queue(queue_), cmd(cmd_), memoryProperties(memoryProperties_) {

}


VertexBufferBuilder VertexBufferBuilder::begin(VkDevice device_, VkQueue queue_, VkCommandBuffer cmd_, VkPhysicalDeviceMemoryProperties& memoryProperties_) {
	VertexBufferBuilder builder(device_, queue_, cmd_, memoryProperties_);
	return builder;
}

VertexBufferBuilder& VertexBufferBuilder::AddVertices(VkDeviceSize vertexSize, float* pVertexData,bool mapped) {
	vertexSizes.push_back(vertexSize);
	vertexPtrs.push_back(pVertexData);
	isMapped = mapped;
	return *this;
}

void VertexBufferBuilder::build(Buffer& buffer_, std::vector<uint32_t>& vertexLocations,void**ptrOut) {
	assert(vertexSizes.size() == vertexPtrs.size());
	vertexLocations.clear();
	VkDeviceSize totalSize = 0;// std::accumulate(vertexSizes.begin(), vertexSizes.end(), 0);
	for (auto& vertexSize : vertexSizes) {
		totalSize += vertexSize;
	}
	BufferProperties props;
#ifdef __USE__VMA__
	props.usage = isMapped ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_ONLY;
#else
	props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
	props.bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	props.size = totalSize;
	initBuffer(device, memoryProperties, props, buffer_);
	if (isMapped)
		*ptrOut = mapBuffer(device, buffer_);
	if (vertexSizes.size()) {
		if (isMapped) {
			for (size_t i = 0; i < vertexSizes.size(); ++i) {
				uint32_t offset = 0;
				void* vptr = vertexPtrs[i];
				uint8_t* ptr = (uint8_t*)*ptrOut;
				if (vptr) {
					
					VkDeviceSize vertexSize = vertexSizes[i];
					memcpy(ptr, vptr, vertexSize);
					ptr += vertexSize;
					vertexLocations.push_back(offset);
					offset += (uint32_t)vertexSize;
				}
			}
		}
		else {
			Buffer stagingBuffer;

#ifdef __USE__VMA__
			props.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
			props.memoryProps = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
#endif
			props.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			props.size = totalSize;
			initBuffer(device, memoryProperties, props, stagingBuffer);
			uint8_t* ptr = (uint8_t*)mapBuffer(device, stagingBuffer);
			uint32_t offset = 0;
			bool needCopy = false;
			for (size_t i = 0; i < vertexSizes.size(); ++i) {

				void* vptr = vertexPtrs[i];
				if (vptr) {
					needCopy = true;
					VkDeviceSize vertexSize = vertexSizes[i];
					memcpy(ptr, vptr, vertexSize);
					ptr += vertexSize;
					vertexLocations.push_back(offset);
					offset += (uint32_t)vertexSize;
				}
			}
			if (needCopy)
				CopyBufferTo(device, queue, cmd, stagingBuffer, buffer_, totalSize);
			unmapBuffer(device, stagingBuffer);
			cleanupBuffer(device, stagingBuffer);
		}
	}
}

IndexBufferBuilder::IndexBufferBuilder(VkDevice device_, VkQueue queue_, VkCommandBuffer cmd_, VkPhysicalDeviceMemoryProperties& memoryProperties_) :device(device_), queue(queue_), cmd(cmd_), memoryProperties(memoryProperties_) {

}

IndexBufferBuilder::~IndexBufferBuilder() {
	indexSizes.clear();
	indexPtrs.clear();
}

IndexBufferBuilder IndexBufferBuilder::begin(VkDevice device_, VkQueue queue_, VkCommandBuffer cmd_, VkPhysicalDeviceMemoryProperties& memoryProperties_) {
	IndexBufferBuilder builder(device_, queue_, cmd_, memoryProperties_);
	return builder;
}

IndexBufferBuilder& IndexBufferBuilder::AddIndices(VkDeviceSize indexSize, uint32_t* pindexData,bool mapped) {
	indexSizes.push_back(indexSize);
	indexPtrs.push_back(pindexData);
	isMapped = mapped;
	return *this;
}

void IndexBufferBuilder::build(Buffer& buffer_, std::vector<uint32_t>& indexLocations,void**ptrOut) {
	assert(indexSizes.size() == indexPtrs.size());
	indexLocations.clear();
	VkDeviceSize totalSize = 0;// std::accumulate(vertexSizes.begin(), vertexSizes.end(), 0);
	for (auto& indexSize : indexSizes) {
		totalSize += indexSize;
	}
	BufferProperties props;
#ifdef __USE__VMA__
	props.usage = isMapped ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_ONLY;
#else
	props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
	props.bufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	props.size = totalSize;
	initBuffer(device, memoryProperties, props, buffer_);
	if (isMapped && ptrOut) {
		*ptrOut = mapBuffer(device, buffer_);
	}
	if (indexSizes.size()) {
		if (isMapped) {
			uint8_t* ptr =(uint8_t*) *ptrOut;
			uint32_t offset = 0;
			for (size_t i = 0; i < indexSizes.size(); ++i) {
				void* vptr = indexPtrs[i];
				if (vptr) {
					VkDeviceSize indexSize = indexSizes[i];
					memcpy(ptr, vptr, indexSize);
					ptr += indexSize;
					indexLocations.push_back(offset);
					offset += (uint32_t)indexSize;
				}
			}
		}
		else {
			Buffer stagingBuffer;

#ifdef __USE__VMA__
			props.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
			props.memoryProps = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
#endif
			props.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			props.size = totalSize;
			initBuffer(device, memoryProperties, props, stagingBuffer);
			uint8_t* ptr = (uint8_t*)mapBuffer(device, stagingBuffer);
			uint32_t offset = 0;
			for (size_t i = 0; i < indexSizes.size(); ++i) {
				void* vptr = indexPtrs[i];
				VkDeviceSize indexSize = indexSizes[i];
				memcpy(ptr, vptr, indexSize);
				ptr += indexSize;
				indexLocations.push_back(offset);
				offset += (uint32_t)indexSize;
			}
			CopyBufferTo(device, queue, cmd, stagingBuffer, buffer_, totalSize);
			unmapBuffer(device, stagingBuffer);
			cleanupBuffer(device, stagingBuffer);
		}
	}
}

StagingBufferBuilder::StagingBufferBuilder(VkDevice device_, VkPhysicalDeviceMemoryProperties& memoryProperties_) :device(device_), memoryProperties(memoryProperties_) {

}

StagingBufferBuilder StagingBufferBuilder::begin(VkDevice device_, VkPhysicalDeviceMemoryProperties& memoryProperties_) {
	StagingBufferBuilder builder(device_, memoryProperties_);
	return builder;
}

StagingBufferBuilder& StagingBufferBuilder::setSize(VkDeviceSize size_) {
	size = size_;
	return *this;
}

Buffer StagingBufferBuilder::build() {
	Buffer stagingBuffer;
	BufferProperties props;
#ifdef __USE__VMA__
	props.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
	props.memoryProps = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
#endif
	props.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	props.size = size;
	initBuffer(device, memoryProperties, props, stagingBuffer);
	return stagingBuffer;
}

ImageLoader::ImageLoader(VkDevice device_, VkCommandBuffer commandBuffer_, VkQueue queue_, VkPhysicalDeviceMemoryProperties memoryProperties_) :device(device_),
commandBuffer(commandBuffer_), queue(queue_), memoryProperties(memoryProperties_) {

}

ImageLoader ImageLoader::begin(VkDevice device_, VkCommandBuffer commandBuffer_, VkQueue queue_, VkPhysicalDeviceMemoryProperties memoryProperties_) {
	ImageLoader loader(device_, commandBuffer_, queue_, memoryProperties_);
	return loader;
}

ImageLoader& ImageLoader::addImage(const char* imagePath, bool enableLod) {
	imagePaths.push_back(imagePath);
	enableLods.push_back(enableLod);
	return *this;
}

ImageLoader& ImageLoader::setIsArray(bool isArray_) {
	isArray = isArray_;
	return *this;
}

ImageLoader& ImageLoader::setIsCube(bool isCube_) {
	isCube = isCube_;
	return *this;
}

void ImageLoader::loadCubeMap(std::vector<uint8_t*>& pixelArray, uint32_t width, uint32_t height, bool enableLod, Image& image) {

	uint32_t imageCount = (uint32_t)pixelArray.size();
	assert(imageCount == 6);
	VkDeviceSize imageSize = (VkDeviceSize)(width * height * 4);
	VkDeviceSize imageArraySize = (VkDeviceSize)(imageCount * imageSize);
	TextureProperties props;
	props.format = PREFERRED_IMAGE_FORMAT;
	props.imageUsage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
#ifdef __USE__VMA__
	props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
	props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
	props.width = (uint32_t)width;
	props.height = (uint32_t)height;
	props.mipLevels = enableLod ? 0 : 1;
	props.samplerProps.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	props.layers = 6;
	props.isCubeMap = true;

	initImage(device, memoryProperties, props, image);
	transitionImage(device, queue, commandBuffer, image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image.mipLevels, imageCount);

	//we could cache the staging buffer and not allocate/deallocate each time, instead only reallaocate when larger required, but meh
	VkDeviceSize maxSize = imageSize;
	BufferProperties bufProps;
	bufProps.size = maxSize;
	bufProps.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
#ifdef __USE__VMA__
	bufProps.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
	bufProps.memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
#endif
	Buffer stagingBuffer;

	initBuffer(device, memoryProperties, bufProps, stagingBuffer);
	void* ptr = mapBuffer(device, stagingBuffer);
	//copy image data using staging buffer
	VkDeviceSize offset = 0;
	for (size_t i = 0; i < pixelArray.size(); ++i) {

		memcpy(ptr, pixelArray[i], imageSize);

		CopyBufferToImage(device, queue, commandBuffer, stagingBuffer, image, width, height, offset, (uint32_t)i);
		//offset += imageSize;
	}
	//even if lod not enabled, need to transition, so use this code.
	generateMipMaps(device, queue, commandBuffer, image);
	unmapBuffer(device, stagingBuffer);
	cleanupBuffer(device, stagingBuffer);

}


void ImageLoader::loadImageArray(std::vector<uint8_t*>& pixelArray, uint32_t width, uint32_t height, bool enableLod, Image& image) {
	uint32_t imageCount = (uint32_t)pixelArray.size();
	VkDeviceSize imageSize = (VkDeviceSize)(width * height * 4);
	VkDeviceSize imageArraySize = (VkDeviceSize)(imageCount * imageSize);
	TextureProperties props;
	props.format = PREFERRED_IMAGE_FORMAT;
	props.imageUsage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
#ifdef __USE__VMA__
	props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
	props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
	props.width = (uint32_t)width;
	props.height = (uint32_t)height;
	props.mipLevels = enableLod ? 0 : 1;
	props.samplerProps.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	props.layers = imageCount;

	initImage(device, memoryProperties, props, image);
	transitionImage(device, queue, commandBuffer, image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image.mipLevels, imageCount);

	//we could cache the staging buffer and not allocate/deallocate each time, instead only reallaocate when larger required, but meh
	VkDeviceSize maxSize = imageSize;
	BufferProperties bufProps;
	bufProps.size = maxSize;
	bufProps.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
#ifdef __USE__VMA__
	bufProps.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
	bufProps.memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
#endif
	Buffer stagingBuffer;

	initBuffer(device, memoryProperties, bufProps, stagingBuffer);
	void* ptr = mapBuffer(device, stagingBuffer);
	//copy image data using staging buffer
	VkDeviceSize offset = 0;
	for (size_t i = 0; i < pixelArray.size(); ++i) {

		memcpy(ptr, pixelArray[i], imageSize);

		CopyBufferToImage(device, queue, commandBuffer, stagingBuffer, image, width, height, offset, (uint32_t)i);

	}
	//even if lod not enabled, need to transition, so use this code.
	generateMipMaps(device, queue, commandBuffer, image);
	unmapBuffer(device, stagingBuffer);
	cleanupBuffer(device, stagingBuffer);

}

void ImageLoader::loadImage(uint8_t* pixels, uint32_t width, uint32_t height, VkFormat format, bool enableLod, Image& image) {
	VkDeviceSize imageSize = (VkDeviceSize)(width * height * 4);
	TextureProperties props;
	props.format = PREFERRED_IMAGE_FORMAT;
	props.imageUsage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
#ifdef __USE__VMA__
	props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
	props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
	props.width = (uint32_t)width;
	props.height = (uint32_t)height;
	props.mipLevels = enableLod ? 0 : 1;
	props.samplerProps.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	initImage(device, memoryProperties, props, image);
	transitionImage(device, queue, commandBuffer, image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image.mipLevels);


	//we could cache the staging buffer and not allocate/deallocate each time, instead only reallaocate when larger required, but meh
	VkDeviceSize maxSize = imageSize;
	BufferProperties bufProps;
	bufProps.size = maxSize;
	bufProps.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
#ifdef __USE__VMA__
	bufProps.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
	bufProps.memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
#endif
	Buffer stagingBuffer;

	initBuffer(device, memoryProperties, bufProps, stagingBuffer);
	void* ptr = mapBuffer(device, stagingBuffer);
	//copy image data using staging buffer
	memcpy(ptr, pixels, imageSize);

	CopyBufferToImage(device, queue, commandBuffer, stagingBuffer, image, width, height);

	//even if lod not enabled, need to transition, so use this code.
	generateMipMaps(device, queue, commandBuffer, image);
	unmapBuffer(device, stagingBuffer);
	cleanupBuffer(device, stagingBuffer);
}

bool ImageLoader::load(std::vector<Image>& images) {
	if (isArray) {
		images.resize(1);
		std::vector<int> widthArray;
		std::vector<int> heightArray;
		std::vector<uint8_t*> pixelArray;
		VkDeviceSize cubeSize = 0;
		bool enableLod = false;
		for (size_t i = 0; i < imagePaths.size(); ++i) {
			auto& imagePath = imagePaths[i];
			int texWidth, texHeight, texChannels;
			stbi_uc* texPixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			assert(texPixels != nullptr);

			VkDeviceSize imageSize = (uint64_t)texWidth * (uint64_t)texHeight * 4;
			cubeSize += imageSize;
			widthArray.push_back(texWidth);
			heightArray.push_back(texHeight);
			pixelArray.push_back(texPixels);
			enableLod |= enableLods[i];
		}
		uint32_t width = *max_element(widthArray.begin(), widthArray.end());
		uint32_t height = *max_element(heightArray.begin(), heightArray.end());
		Image image;
		loadImageArray(pixelArray, width, height, enableLod, image);
		for (auto& pixels : pixelArray) {
			stbi_image_free(pixels);
		}
		images[0] = image;

	}
	else if (isCube) {
		images.resize(1);
		std::vector<int> widthArray;
		std::vector<int> heightArray;
		std::vector<uint8_t*> pixelArray;
		VkDeviceSize cubeSize = 0;
		bool enableLod = false;
		for (size_t i = 0; i < imagePaths.size(); ++i) {
			auto& imagePath = imagePaths[i];
			int texWidth, texHeight, texChannels;
			stbi_uc* texPixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			assert(texPixels != nullptr);

			VkDeviceSize imageSize = (uint64_t)texWidth * (uint64_t)texHeight * 4;
			cubeSize += imageSize;
			widthArray.push_back(texWidth);
			heightArray.push_back(texHeight);
			pixelArray.push_back(texPixels);
			enableLod |= enableLods[i];
		}
		uint32_t width = *max_element(widthArray.begin(), widthArray.end());
		uint32_t height = *max_element(heightArray.begin(), heightArray.end());
		Image image;
		loadCubeMap(pixelArray, width, height, enableLod, image);
		for (auto& pixels : pixelArray) {
			stbi_image_free(pixels);
		}
		images[0] = image;
	}
	else {
		images.resize(imagePaths.size());
		for (size_t i = 0; i < imagePaths.size(); ++i) {
			auto& imagePath = imagePaths[i];
			int texWidth, texHeight, texChannels;
			stbi_uc* texPixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			assert(texPixels != nullptr);

			Image image;
			loadImage(texPixels, (uint32_t)texWidth, (uint32_t)texHeight, PREFERRED_FORMAT, enableLods[i], image);
			stbi_image_free(texPixels);
			images[i] = image;
		}
	}
	return true;
}




TextureLoader::TextureLoader(VkDevice device_, VkCommandBuffer commandBuffer_, VkQueue queue_, VkPhysicalDeviceMemoryProperties memoryProperties_) :device(device_),
commandBuffer(commandBuffer_), queue(queue_), memoryProperties(memoryProperties_) {

}

TextureLoader TextureLoader::begin(VkDevice device_, VkCommandBuffer commandBuffer_, VkQueue queue_, VkPhysicalDeviceMemoryProperties memoryProperties_) {
	TextureLoader loader(device_, commandBuffer_, queue_, memoryProperties_);
	return loader;
}

TextureLoader& TextureLoader::addTexture(const char* imagePath, bool enableLod) {
	imagePaths.push_back(imagePath);
	enableLods.push_back(enableLod);
	return *this;
}

TextureLoader& TextureLoader::setIsArray(bool isArray_) {
	isArray = isArray_;
	return *this;
}

TextureLoader& TextureLoader::setIsCube(bool isCube_) {
	isCube = isCube_;
	return *this;
}

TextureLoader& TextureLoader::setSamplerAddressMode(VkSamplerAddressMode addrMode_) {
	addrMode = addrMode_;
	return *this;
}

TextureLoader& TextureLoader::setSamplerFilter(VkFilter filter_) {
	filter = filter_;
	return *this;
}


void TextureLoader::loadTextureArray(std::vector<uint8_t*>& pixelArray, uint32_t width, uint32_t height, bool enableLod, Texture& texture) {
	uint32_t imageCount = (uint32_t)pixelArray.size();
	VkDeviceSize imageSize = (VkDeviceSize)(width * height * 4);
	VkDeviceSize imageArraySize = (VkDeviceSize)(imageCount * imageSize);
	TextureProperties props;
	props.format = PREFERRED_IMAGE_FORMAT;
	props.imageUsage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
#ifdef __USE__VMA__
	props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
	props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
	props.width = (uint32_t)width;
	props.height = (uint32_t)height;
	props.mipLevels = enableLod ? 0 : 1;
	props.samplerProps.addressMode = addrMode;
	props.samplerProps.filter = filter;

	props.layers = imageCount;

	initTexture(device, memoryProperties, props, texture);
	transitionImage(device, queue, commandBuffer, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mipLevels, imageCount);

	//we could cache the staging buffer and not allocate/deallocate each time, instead only reallaocate when larger required, but meh
	VkDeviceSize maxSize = imageSize;
	BufferProperties bufProps;
	bufProps.size = maxSize;
	bufProps.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
#ifdef __USE__VMA__
	bufProps.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
	bufProps.memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
#endif
	Buffer stagingBuffer;

	initBuffer(device, memoryProperties, bufProps, stagingBuffer);
	void* ptr = mapBuffer(device, stagingBuffer);
	//copy image data using staging buffer
	VkDeviceSize offset = 0;
	for (size_t i = 0; i < pixelArray.size(); ++i) {

		memcpy(ptr, pixelArray[i], imageSize);

		CopyBufferToImage(device, queue, commandBuffer, stagingBuffer, texture, width, height, offset, (uint32_t)i);

	}
	//even if lod not enabled, need to transition, so use this code.
	generateMipMaps(device, queue, commandBuffer, texture);
	unmapBuffer(device, stagingBuffer);
	cleanupBuffer(device, stagingBuffer);

}
void TextureLoader::loadCubeMap(std::vector<uint8_t*>& pixelArray, uint32_t width, uint32_t height, bool enableLod, Texture& image) {

	uint32_t imageCount = (uint32_t)pixelArray.size();
	assert(imageCount == 6);
	VkDeviceSize imageSize = (VkDeviceSize)(width * height * 4);
	VkDeviceSize imageArraySize = (VkDeviceSize)(imageCount * imageSize);
	TextureProperties props;
	props.format = PREFERRED_IMAGE_FORMAT;
	props.imageUsage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
#ifdef __USE__VMA__
	props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
	props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
	props.width = (uint32_t)width;
	props.height = (uint32_t)height;
	props.mipLevels = enableLod ? 0 : 1;
	props.samplerProps.addressMode = addrMode;
	props.samplerProps.filter = filter;
	props.layers = 6;
	props.isCubeMap = true;

	initTexture(device, memoryProperties, props, image);
	transitionImage(device, queue, commandBuffer, image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image.mipLevels, imageCount);

	//we could cache the staging buffer and not allocate/deallocate each time, instead only reallaocate when larger required, but meh
	VkDeviceSize maxSize = imageSize;
	BufferProperties bufProps;
	bufProps.size = maxSize;
	bufProps.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
#ifdef __USE__VMA__
	bufProps.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
	bufProps.memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
#endif
	Buffer stagingBuffer;

	initBuffer(device, memoryProperties, bufProps, stagingBuffer);
	void* ptr = mapBuffer(device, stagingBuffer);
	//copy image data using staging buffer
	VkDeviceSize offset = 0;
	for (size_t i = 0; i < pixelArray.size(); ++i) {

		memcpy(ptr, pixelArray[i], imageSize);

		CopyBufferToImage(device, queue, commandBuffer, stagingBuffer, image, width, height, offset, (uint32_t)i);
		//offset += imageSize;
	}
	//even if lod not enabled, need to transition, so use this code.
	generateMipMaps(device, queue, commandBuffer, image);
	unmapBuffer(device, stagingBuffer);
	cleanupBuffer(device, stagingBuffer);

}




void TextureLoader::loadTexture(uint8_t* pixels, uint32_t width, uint32_t height, VkFormat format, bool enableLod, Texture& texture) {
	VkDeviceSize imageSize = (VkDeviceSize)(width * height * 4);
	TextureProperties props;
	props.format = PREFERRED_IMAGE_FORMAT;
	props.imageUsage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
#ifdef __USE__VMA__
	props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
#else
	props.memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
#endif
	props.width = (uint32_t)width;
	props.height = (uint32_t)height;
	props.mipLevels = enableLod ? 0 : 1;
	props.samplerProps.addressMode = addrMode;
	props.samplerProps.filter = filter;

	initTexture(device, memoryProperties, props, texture);
	transitionImage(device, queue, commandBuffer, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mipLevels);


	//we could cache the staging buffer and not allocate/deallocate each time, instead only reallaocate when larger required, but meh
	VkDeviceSize maxSize = imageSize;
	BufferProperties bufProps;
	bufProps.size = maxSize;
	bufProps.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
#ifdef __USE__VMA__
	bufProps.usage = VMA_MEMORY_USAGE_CPU_ONLY;
#else
	bufProps.memoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
#endif
	Buffer stagingBuffer;

	initBuffer(device, memoryProperties, bufProps, stagingBuffer);
	void* ptr = mapBuffer(device, stagingBuffer);
	//copy image data using staging buffer
	memcpy(ptr, pixels, imageSize);

	CopyBufferToImage(device, queue, commandBuffer, stagingBuffer, texture, width, height);

	//even if lod not enabled, need to transition, so use this code.
	generateMipMaps(device, queue, commandBuffer, texture);
	unmapBuffer(device, stagingBuffer);
	cleanupBuffer(device, stagingBuffer);
}

bool TextureLoader::load(std::vector<Texture>& textures) {
	if (isArray) {
		textures.resize(1);
		std::vector<int> widthArray;
		std::vector<int> heightArray;
		std::vector<uint8_t*> pixelArray;
		VkDeviceSize cubeSize = 0;
		bool enableLod = false;
		for (size_t i = 0; i < imagePaths.size(); ++i) {
			auto& imagePath = imagePaths[i];
			int texWidth, texHeight, texChannels;
			stbi_uc* texPixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			if (texPixels == nullptr)
				return false;
			VkDeviceSize imageSize = (uint64_t)texWidth * (uint64_t)texHeight * 4;
			cubeSize += imageSize;
			widthArray.push_back(texWidth);
			heightArray.push_back(texHeight);
			pixelArray.push_back(texPixels);
			enableLod |= enableLods[i];
		}
		uint32_t width = *max_element(widthArray.begin(), widthArray.end());
		uint32_t height = *max_element(heightArray.begin(), heightArray.end());
		Texture texture;
		loadTextureArray(pixelArray, width, height, enableLod, texture);
		for (auto& pixels : pixelArray) {
			stbi_image_free(pixels);
		}
		textures[0] = texture;

	}
	else if (isCube) {
		textures.resize(1);
		std::vector<int> widthArray;
		std::vector<int> heightArray;
		std::vector<uint8_t*> pixelArray;
		VkDeviceSize cubeSize = 0;
		bool enableLod = false;
		for (size_t i = 0; i < imagePaths.size(); ++i) {
			auto& imagePath = imagePaths[i];
			int texWidth, texHeight, texChannels;
			stbi_uc* texPixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			assert(texPixels != nullptr);

			VkDeviceSize imageSize = (uint64_t)texWidth * (uint64_t)texHeight * 4;
			cubeSize += imageSize;
			widthArray.push_back(texWidth);
			heightArray.push_back(texHeight);
			pixelArray.push_back(texPixels);
			enableLod |= enableLods[i];
		}
		uint32_t width = *max_element(widthArray.begin(), widthArray.end());
		uint32_t height = *max_element(heightArray.begin(), heightArray.end());
		Texture texture;
		loadCubeMap(pixelArray, width, height, enableLod, texture);
		for (auto& pixels : pixelArray) {
			stbi_image_free(pixels);
		}
		textures[0] = texture;
	}
	else {
		textures.resize(imagePaths.size());
		for (size_t i = 0; i < imagePaths.size(); ++i) {
			auto& imagePath = imagePaths[i];
			int texWidth, texHeight, texChannels;
			stbi_uc* texPixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			if (texPixels == nullptr)
				return false;
			Texture texture;
			loadTexture(texPixels, (uint32_t)texWidth, (uint32_t)texHeight, PREFERRED_IMAGE_FORMAT, enableLods[i], texture);
			stbi_image_free(texPixels);
			textures[i] = texture;
		}
	}
	return true;
}

ShaderProgramLoader::ShaderProgramLoader(VkDevice device_) :device(device_) {

}

ShaderProgramLoader ShaderProgramLoader::begin(VkDevice device_) {
	ShaderProgramLoader loader(device_);
	return loader;
}

ShaderProgramLoader& ShaderProgramLoader::AddShaderPath(const char* shaderPath) {
	shaderPaths.push_back(shaderPath);
	return *this;
}

ShaderProgramLoader& ShaderProgramLoader::AddShaderSpirv(const std::vector<uint32_t>& spirv) {
	shaderSpirvs.push_back(spirv);
	return *this;
}

void ShaderProgramLoader::load(std::vector<ShaderModule>& shaders_) {
	//shaders_.resize(shaderPaths.size());
	for (size_t i = 0; i < shaderPaths.size(); ++i) {
		auto& shaderPath = shaderPaths[i];
		std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);
		assert(file.is_open());


		size_t fileSize = (size_t)file.tellg();
		std::vector<uint32_t> shaderData(fileSize >> 2);

		file.seekg(0);
		file.read((char*)shaderData.data(), fileSize);

		file.close();
		shaderSpirvs.push_back(shaderData);
	}
	for(size_t i=0;i<shaderSpirvs.size();i++){
		auto& spirv = shaderSpirvs[i];
		SpvReflectShaderModule module = {};
		SpvReflectResult result = spvReflectCreateShaderModule(spirv.size()*sizeof(uint32_t), spirv.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);


		VkShaderStageFlagBits stage;
		switch (module.shader_stage) {
		case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
			stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
			stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
			stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			break;
		case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
			stage = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		default:
			assert(0);
			break;
		}

		if (stage == VK_SHADER_STAGE_VERTEX_BIT) {

			vertexAttributeDescriptions.resize(module.input_variable_count);

			uint32_t offset = 0;
			std::vector<uint32_t> sizes(module.input_variable_count);
			std::vector<VkFormat> formats(module.input_variable_count);
			for (uint32_t i = 0; i < module.input_variable_count; ++i) {
				SpvReflectInterfaceVariable& inputVar = *module.input_variables[i];
				uint32_t size = 0;
				VkFormat format;
				switch (inputVar.format) {

				case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
					format = VK_FORMAT_R32G32B32A32_SFLOAT;
					size = sizeof(float) * 4;
					break;
				case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
					format = VK_FORMAT_R32G32B32_SFLOAT;
					size = sizeof(float) * 3;
					break;
				case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
					format = VK_FORMAT_R32G32_SFLOAT;
					size = sizeof(float) * 2;
					break;
				default:
					assert(0);
					break;
				}
				sizes[inputVar.location] = (uint32_t)size;
				formats[inputVar.location] = format;
			}

			for (uint32_t i = 0; i < module.input_variable_count; i++) {

				vertexAttributeDescriptions[i].location = i;
				vertexAttributeDescriptions[i].offset = offset;
				vertexAttributeDescriptions[i].format = formats[i];
				offset += sizes[i];
			}
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = offset;
			vertexInputDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		}
		spvReflectDestroyShaderModule(&module);

		VkShaderModule shader = VK_NULL_HANDLE;

		VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = spirv.size()*sizeof(uint32_t);
		createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());
		VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &shader);
		assert(res == VK_SUCCESS);
		shaders.insert(std::pair<VkShaderStageFlagBits, VkShaderModule>(stage, shader));
		//shaders.push_back(shader);
		spirv.clear();
	}
	//insert in order VERTEX,GEOMETRY,FRAGMENT
	shaders_.clear();
	if (shaders.find(VK_SHADER_STAGE_VERTEX_BIT) != shaders.end()) {
		ShaderModule vertexShader{ shaders[VK_SHADER_STAGE_VERTEX_BIT],VK_SHADER_STAGE_VERTEX_BIT };
		shaders_.push_back(vertexShader);
	}
	if (shaders.find(VK_SHADER_STAGE_GEOMETRY_BIT) != shaders.end()) {
		ShaderModule geometryShader{ shaders[VK_SHADER_STAGE_GEOMETRY_BIT],VK_SHADER_STAGE_GEOMETRY_BIT };
		shaders_.push_back(geometryShader);
	}
	if (shaders.find(VK_SHADER_STAGE_FRAGMENT_BIT) != shaders.end()) {
		ShaderModule fragmentShader{ shaders[VK_SHADER_STAGE_FRAGMENT_BIT],VK_SHADER_STAGE_FRAGMENT_BIT };
		shaders_.push_back(fragmentShader);
	}
	if (shaders.find(VK_SHADER_STAGE_COMPUTE_BIT) != shaders.end()) {
		ShaderModule computeShader{ shaders[VK_SHADER_STAGE_COMPUTE_BIT],VK_SHADER_STAGE_COMPUTE_BIT };
		shaders_.push_back(computeShader);
	}
}

bool ShaderProgramLoader::load(std::vector<ShaderModule>& shaders_, VkVertexInputBindingDescription& vertexInputDescription_, std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions_) {
	//shaders_.resize(shaderPaths.size());
	for (size_t i = 0; i < shaderPaths.size(); ++i) {
		auto& shaderPath = shaderPaths[i];
		std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);
		assert(file.is_open());


		size_t fileSize = (size_t)file.tellg();
		std::vector<uint32_t> shaderData(fileSize >> 2);

		file.seekg(0);
		file.read((char*)shaderData.data(), fileSize);

		file.close();
		shaderSpirvs.push_back(shaderData);
	}
	for (size_t i = 0; i < shaderSpirvs.size(); i++) {
		auto& spirv = shaderSpirvs[i];
		SpvReflectShaderModule module = {};
		SpvReflectResult result = spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);


		VkShaderStageFlagBits stage;
		switch (module.shader_stage) {
		case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
			stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
			stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
			stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			break;
		case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
			stage = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		default:
			assert(0);
			break;
		}
		for (uint32_t bindIdx = 0; bindIdx < module.descriptor_binding_count; ++bindIdx) {
			auto& binding = module.descriptor_bindings[bindIdx];
			SpvReflectDescriptorType spvDescriptorType = binding.descriptor_type;
			auto typeName = binding.type_description->type_name;
			for (uint32_t memIdx = 0; memIdx < binding.type_description->member_count; memIdx++) {
				auto member = binding.type_description->members[memIdx];
				auto memTypeName = member.type_name;
			}
		}
		for (uint32_t setIdx = 0; setIdx < module.descriptor_set_count;++setIdx) {
			auto& set = module.descriptor_sets[setIdx];
			auto setId = set.set;
			for (uint32_t bindIdx = 0; bindIdx < set.binding_count; bindIdx++) {
				auto& binding = set.bindings[bindIdx];
				auto bidningDescriptorType = binding->descriptor_type;

			}
		}
		

		if (stage == VK_SHADER_STAGE_VERTEX_BIT) {
			uint32_t count = 0;
			for (uint32_t i = 0; i < module.input_variable_count; ++i) {
				SpvReflectInterfaceVariable& inputVar = *module.input_variables[i];
				if (inputVar.built_in == -1)
					count++;
			}
			vertexAttributeDescriptions.resize(count);

			uint32_t offset = 0;
			std::vector<uint32_t> sizes(count);
			std::vector<VkFormat> formats(count);

			for (uint32_t i = 0; i < module.input_variable_count; ++i) {
				SpvReflectInterfaceVariable& inputVar = *module.input_variables[i];
				if (inputVar.built_in != -1)
					continue;
				uint32_t size = 0;
				VkFormat format;
				switch (inputVar.format) {

				case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
					format = VK_FORMAT_R32G32B32A32_SFLOAT;
					size = sizeof(float) * 4;
					break;
				case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
					format = VK_FORMAT_R32G32B32_SFLOAT;
					size = sizeof(float) * 3;
					break;
				case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
					format = VK_FORMAT_R32G32_SFLOAT;
					size = sizeof(float) * 2;
					break;
				case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
					format = VK_FORMAT_R32G32B32A32_SINT;
					size = sizeof(int32_t) * 4;
					break;
				default:
					assert(0);
					break;
				}
				sizes[inputVar.location] = (uint32_t)size;
				formats[inputVar.location] = format;

			}

			//for (uint32_t i = 0; i < module.input_variable_count; i++) {
			for (uint32_t i = 0; i < count; i++) {

				vertexAttributeDescriptions[i].location = i;
				vertexAttributeDescriptions[i].offset = offset;
				vertexAttributeDescriptions[i].format = formats[i];
				offset += sizes[i];
			}
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = offset;
			vertexInputDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		}
		spvReflectDestroyShaderModule(&module);

		VkShaderModule shader = VK_NULL_HANDLE;

		VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = spirv.size()*sizeof(uint32_t);
		createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());
		VkResult res = vkCreateShaderModule(device, &createInfo, nullptr, &shader);
		assert(res == VK_SUCCESS);
		shaders.insert(std::pair<VkShaderStageFlagBits, VkShaderModule>(stage, shader));
		//shaders.push_back(shader);
		spirv.clear();
	}
	//insert in order VERTEX,GEOMETRY,FRAGMENT
	shaders_.clear();
	if (shaders.find(VK_SHADER_STAGE_VERTEX_BIT) != shaders.end()) {
		ShaderModule vertexShader{ shaders[VK_SHADER_STAGE_VERTEX_BIT],VK_SHADER_STAGE_VERTEX_BIT };
		shaders_.push_back(vertexShader);
		vertexInputDescription_ = vertexInputDescription;
		vertexAttributeDescriptions_ = vertexAttributeDescriptions;
	}
	else {
		return false;//no input descriptions usefull for pipeline
	}
	if (shaders.find(VK_SHADER_STAGE_GEOMETRY_BIT) != shaders.end()) {
		ShaderModule geometryShader{ shaders[VK_SHADER_STAGE_GEOMETRY_BIT],VK_SHADER_STAGE_GEOMETRY_BIT };
		shaders_.push_back(geometryShader);
	}
	if (shaders.find(VK_SHADER_STAGE_FRAGMENT_BIT) != shaders.end()) {
		ShaderModule fragmentShader{ shaders[VK_SHADER_STAGE_FRAGMENT_BIT],VK_SHADER_STAGE_FRAGMENT_BIT };
		shaders_.push_back(fragmentShader);
	}
	return true;
}

PipelineLayoutBuilder::PipelineLayoutBuilder(VkDevice device_) :device(device_) {

}

PipelineLayoutBuilder PipelineLayoutBuilder::begin(VkDevice device_) {
	PipelineLayoutBuilder builder(device_);
	return builder;
}

PipelineLayoutBuilder& PipelineLayoutBuilder::AddDescriptorSetLayout(VkDescriptorSetLayout layout_) {
	descriptorSetLayouts.push_back(layout_);
	return *this;
}

PipelineLayoutBuilder& PipelineLayoutBuilder::AddDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& layouts_) {
	descriptorSetLayouts.insert(descriptorSetLayouts.end(), layouts_.begin(), layouts_.end());
	return *this;
}

PipelineLayoutBuilder& PipelineLayoutBuilder::AddPushConstants(std::vector<VkPushConstantRange>& pushConstants_) {

	pushConstants.insert(pushConstants.end(), pushConstants_.begin(), pushConstants_.end());
	return *this;
}


void PipelineLayoutBuilder::build(VkPipelineLayout& pipelineLayout) {
	pipelineLayout = initPipelineLayout(device, descriptorSetLayouts,pushConstants);
}

PipelineBuilder::PipelineBuilder(VkDevice device_, VkPipelineLayout pipelineLayout_, VkRenderPass renderPass_, std::vector<ShaderModule>& shaders_, VkVertexInputBindingDescription& vertexInputDescription_, std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions_) :
	device(device_), pipelineLayout(pipelineLayout_), shaders(shaders_), renderPass(renderPass_), vertexInputDescription(vertexInputDescription_), vertexAttributeDescriptions(vertexAttributeDescriptions_) {

}


PipelineBuilder PipelineBuilder::begin(VkDevice device_, VkPipelineLayout pipelineLayout_, VkRenderPass renderPass_, std::vector<ShaderModule>& shaders_, VkVertexInputBindingDescription& vertexInputDescription_, std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions_) {
	PipelineBuilder builder(device_, pipelineLayout_, renderPass_, shaders_, vertexInputDescription_, vertexAttributeDescriptions_);
	return builder;
}

PipelineBuilder& PipelineBuilder::setCullMode(VkCullModeFlagBits cullMode_) {
	cullMode = cullMode_;
	return *this;
}

PipelineBuilder& PipelineBuilder::setPolygonMode(VkPolygonMode polygonMode_) {
	polygonMode = polygonMode_;
	return *this;
}

PipelineBuilder& PipelineBuilder::setFrontFace(VkFrontFace frontFace_) {
	frontFace = frontFace_;
	return *this;
}

PipelineBuilder& PipelineBuilder::setBlend(VkBool32 blend_) {
	blend = blend_;
	return *this;
}

PipelineBuilder& PipelineBuilder::setBlendState(VkBlendFactor srcBlend_, VkBlendFactor dstBlend_, VkBlendOp blendOp_, VkBlendFactor srcBlendAlpha_, VkBlendFactor dstBlendAlpha_, VkBlendOp blendOpAlpha_) {
	srcBlend = srcBlend_;
	dstBlend = dstBlend_;
	blendOp = blendOp_;
	srcBlendAlpha = srcBlendAlpha_;
	dstBlendAlpha = dstBlendAlpha_;
	blendOpAlpha = blendOpAlpha_;

	return *this;
}

PipelineBuilder& PipelineBuilder::setDepthTest(VkBool32 depthTest_) {
	depthTest = depthTest_;
	return *this;
}

PipelineBuilder& PipelineBuilder::setDepthCompareOp(VkCompareOp depthCompare_) {
	depthCompareOp = depthCompare_;
	return *this;
}

PipelineBuilder& PipelineBuilder::setStencilTest(VkBool32 stencilTest_) {
	stencilTest = stencilTest_;
	return *this;
}

PipelineBuilder& PipelineBuilder::setNoDraw(VkBool32 noDraw_) {
	noDraw = noDraw_;
	return *this;
}

PipelineBuilder& PipelineBuilder::setStencilState(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp, uint32_t compareMask, uint32_t writeMask, uint32_t reference) {
	stencil.failOp = failOp;
	stencil.passOp = passOp;
	stencil.depthFailOp = depthFailOp;
	stencil.compareOp = compareOp;
	stencil.compareMask = compareMask;
	stencil.writeMask = writeMask;
	stencil.reference = reference;
	return *this;
}

PipelineBuilder& PipelineBuilder::setTopology(VkPrimitiveTopology topology_) {
	topology = topology_;
	return *this;
}

PipelineBuilder& PipelineBuilder::setSpecializationConstant(VkShaderStageFlagBits shaderStage, uint32_t uval) {
	SpecInfo spec = { shaderStage,sizeof(uint32_t) };
	spec.val.u = uval;
	specializationInfo.push_back(spec);
	return *this;
}
PipelineBuilder& PipelineBuilder::setSpecializationConstant(VkShaderStageFlagBits shaderStage, int32_t uval) {
	SpecInfo spec = { shaderStage,sizeof(int32_t) };
	spec.val.i = uval;
	specializationInfo.push_back(spec);
	return *this;
}
PipelineBuilder& PipelineBuilder::setSpecializationConstant(VkShaderStageFlagBits shaderStage, float fval) {
	SpecInfo spec = { shaderStage,sizeof(float),fval };
	specializationInfo.push_back(spec);
	return *this;
}

PipelineBuilder& PipelineBuilder::setSampleCount(VkSampleCountFlagBits sampleCount) {
	samples = sampleCount;
	return *this;
}

void PipelineBuilder::build(VkPipeline& pipeline) {
	PipelineInfo pipelineInfo;
	pipelineInfo.cullMode = cullMode;
	pipelineInfo.depthTest = depthTest;
	pipelineInfo.depthCompareOp = depthCompareOp;
	pipelineInfo.polygonMode = polygonMode;
	pipelineInfo.blend = blend;
	pipelineInfo.stencilTest = stencilTest;
	pipelineInfo.stencil = stencil;
	pipelineInfo.frontFace = frontFace;
	pipelineInfo.noDraw = noDraw;
	pipelineInfo.topology = topology;
	pipelineInfo.samples = samples;
	if (blend) {
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachment.blendEnable = blend;
		blendAttachment.alphaBlendOp = blendOpAlpha;
		blendAttachment.srcAlphaBlendFactor = srcBlendAlpha;
		blendAttachment.dstAlphaBlendFactor = dstBlendAlpha;
		blendAttachment.colorBlendOp = blendOp;
		blendAttachment.srcColorBlendFactor = srcBlend;
		blendAttachment.dstColorBlendFactor = dstBlend;
		pipelineInfo.attachementStates.push_back(blendAttachment);
	}
	std::vector<VkSpecializationMapEntry> specMap;

	std::vector<uint8_t> specData;
	if (specializationInfo.size() > 0) {
		uint32_t offset = 0;
		uint32_t id = 0;
		VkShaderStageFlagBits shaderStage = VK_SHADER_STAGE_VERTEX_BIT;
		for (auto& specInfo : specializationInfo) {
			specMap.push_back({ id,offset,specInfo.size });
			id++;
			offset += specInfo.size;
			shaderStage = specInfo.shaderStage;

		}
		uint32_t size = offset;
		specData.resize(size);
		offset = 0;
		uint8_t* ptr = specData.data();
		for (auto& specInfo : specializationInfo) {

			memcpy(ptr, &specInfo.val, specInfo.size);
			ptr += specInfo.size;
		}
		pipelineInfo.specializationSize = size;
		pipelineInfo.specializationData = specData.data();
		pipelineInfo.specializationMap = specMap;
		pipelineInfo.specializationStage = shaderStage;

	}
	pipeline = initGraphicsPipeline(device, renderPass, pipelineLayout, shaders, vertexInputDescription, vertexAttributeDescriptions, pipelineInfo);

}

ComputePipelineBuilder::ComputePipelineBuilder(VkDevice device_, VkPipelineLayout layout_) :device(device_), pipelineLayout(layout_) {

}

ComputePipelineBuilder ComputePipelineBuilder::begin(VkDevice device_, VkPipelineLayout layout_) {
	ComputePipelineBuilder builder(device_, layout_);
	return builder;
}

ComputePipelineBuilder& ComputePipelineBuilder::setShader(const char* pShader_) {
	pShader = pShader_;
	return *this;
}

VkPipeline ComputePipelineBuilder::build() {
	VkShaderModule shader = initShaderModule(device, pShader);
	ShaderModule shaderModule = { shader,VK_SHADER_STAGE_COMPUTE_BIT };
	VkPipeline pipeline = initComputePipeline(device, pipelineLayout, shaderModule);
	cleanupShaderModule(device, shader);
	return pipeline;
}

VulkanBuffer::VulkanBuffer(VkDevice device_, Buffer& buffer_) : VulkanObject(device_), buffer(buffer_) {

}

VulkanBuffer::~VulkanBuffer() {
	cleanupBuffer(device, buffer);
}

VulkanUniformBuffer::VulkanUniformBuffer(VkDevice device_, Buffer& buffer_, std::vector<UniformBufferInfo>& bufferInfo_) :VulkanBuffer(device_, buffer_), bufferInfo(bufferInfo_) {

}

VulkanVIBuffer::VulkanVIBuffer(VkDevice device_, Buffer& buffer_, std::vector<uint32_t>& bufferLocations_) : VulkanBuffer(device_, buffer_), bufferLocations(bufferLocations_) {

}

ImageBuilder::ImageBuilder(VkDevice device_, VkPhysicalDeviceMemoryProperties& memoryProperties_) : device(device_), memoryProperties(memoryProperties_) {

}

ImageBuilder ImageBuilder::begin(VkDevice device_, VkPhysicalDeviceMemoryProperties& memoryProperties_) {
	ImageBuilder builder(device_, memoryProperties_);
	return builder;
}

ImageBuilder& ImageBuilder::setDimensions(uint32_t width_, uint32_t height_) {
	width = width_;
	height = height_;
	return *this;
}

ImageBuilder& ImageBuilder::setImageLayout(VkImageLayout imageLayout_) {
	imageLayout = imageLayout;
	return *this;
}


ImageBuilder& ImageBuilder::setFormat(VkFormat format_) {
	format = format_;
	return *this;
}

ImageBuilder& ImageBuilder::setImageAspectFlags(VkImageAspectFlags aspectFlags_) {
	aspect = aspectFlags_;
	return *this;
}


ImageBuilder& ImageBuilder::setImageUsage(VkImageUsageFlags imageUsage_) {
	imageUsage = imageUsage_;
	return *this;
}


Image ImageBuilder::build() {
	ImageProperties props;
	props.imageUsage = imageUsage;
	props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	props.aspect = aspect;
	props.layout = imageLayout;
	props.samples = samples;
	props.width = width;
	props.height = height;
	props.format = format;
	props.mipLevels = mipLevels;
	Image image;
	initImage(device, memoryProperties, props, image);
	return image;
}

TextureBuilder::TextureBuilder(VkDevice device_, VkPhysicalDeviceMemoryProperties& memoryProperties_) : device(device_), memoryProperties(memoryProperties_) {

}

TextureBuilder TextureBuilder::begin(VkDevice device_, VkPhysicalDeviceMemoryProperties& memoryProperties_) {
	TextureBuilder builder(device_, memoryProperties_);
	return builder;
}

TextureBuilder& TextureBuilder::setDimensions(uint32_t width_, uint32_t height_) {
	width = width_;
	height = height_;
	return *this;
}

TextureBuilder& TextureBuilder::setImageLayout(VkImageLayout imageLayout_) {
	imageLayout = imageLayout;
	return *this;
}


TextureBuilder& TextureBuilder::setFormat(VkFormat format_) {
	format = format_;
	return *this;
}

TextureBuilder& TextureBuilder::setImageAspectFlags(VkImageAspectFlags aspectFlags_) {
	aspect = aspectFlags_;
	return *this;
}


TextureBuilder& TextureBuilder::setImageUsage(VkImageUsageFlags imageUsage_) {
	imageUsage = imageUsage_;
	return *this;
}

TextureBuilder& TextureBuilder::setSampleCount(VkSampleCountFlagBits samples_) {
	samples = samples_;
	return *this;
}

TextureBuilder& TextureBuilder::setFilter(VkFilter filter_) {
	filter = filter_;
	return *this;
}

TextureBuilder& TextureBuilder::setMipLevels(uint32_t m) {
	mipLevels = m;
	return *this;
}


Texture TextureBuilder::build() {
	TextureProperties props;
	props.imageUsage = imageUsage;
	props.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	props.aspect = aspect;
	props.layout = imageLayout;
	props.samples = samples;
	props.width = width;
	props.height = height;
	props.format = format;
	props.mipLevels = mipLevels;	
	Texture texture;
	initTexture(device, memoryProperties, props, texture);
	return texture;
}

RenderPassBuilder::RenderPassBuilder(VkDevice device_) :VulkanObject(device_) {
	rpProps.colorFormat = rpProps.depthFormat = rpProps.resolveFormat = VK_FORMAT_UNDEFINED;//reset format values, must specify what kinds of attachment required
}

RenderPassBuilder RenderPassBuilder::begin(VkDevice device_) {
	RenderPassBuilder builder(device_);
	return builder;
}

RenderPassBuilder& RenderPassBuilder::setColorFormat(VkFormat colorFormat_) {
	//colorFormat = colorFormat_;
	rpProps.colorFormat = colorFormat_;
	return *this;
}

RenderPassBuilder& RenderPassBuilder::setDepthFormat(VkFormat depthFormat_) {
	//depthFormat = depthFormat_;
	rpProps.depthFormat = depthFormat_;
	return *this;
}

RenderPassBuilder& RenderPassBuilder::setResolveFormat(VkFormat resolveFormat_) {
	//resolveFormat = resolveFormat;
	rpProps.resolveFormat = resolveFormat_;
	return *this;
}

RenderPassBuilder& RenderPassBuilder::setSampleCount(VkSampleCountFlagBits samples_) {
	//samples = samples_;
	rpProps.sampleCount = samples_;
	return *this;
}

RenderPassBuilder& RenderPassBuilder::setColorInitialLayout(VkImageLayout colorLayout_) {
	//initialColorLayout = colorLayout_;
	rpProps.colorInitialLayout = colorLayout_;
	return *this;
}

RenderPassBuilder& RenderPassBuilder::setColorFinalLayout(VkImageLayout colorLayout_) {
	//	finalColorLayout = colorLayout_;
	rpProps.colorFinalLayout = colorLayout_;
	return *this;
}


RenderPassBuilder& RenderPassBuilder::setDepthFinalLayout(VkImageLayout colorLayout_) {
	//	finalColorLayout = colorLayout_;
	rpProps.depthFinalLayout = colorLayout_;
	return *this;
}

RenderPassBuilder& RenderPassBuilder::setColorLoadOp(VkAttachmentLoadOp colorLoadOp_) {
	rpProps.colorLoadOp = colorLoadOp_;
	return *this;
}
RenderPassBuilder& RenderPassBuilder::setColorStoreOp(VkAttachmentStoreOp colorStoreOp_) {
	rpProps.colorStoreOp = colorStoreOp_;
	return *this;
}



RenderPassBuilder& RenderPassBuilder::setDepthInitialLayout(VkImageLayout colorLayout_) {
	//initialColorLayout = colorLayout_;
	rpProps.depthInitialLayout = colorLayout_;
	return *this;
}

RenderPassBuilder& RenderPassBuilder::setDepthLoadOp(VkAttachmentLoadOp depthLoadOp_) {
	rpProps.depthLoadOp = depthLoadOp_;
	return *this;
}

RenderPassBuilder& RenderPassBuilder::setDepthStoreOp(VkAttachmentStoreOp depthStoreOp_) {
	rpProps.depthStoreOp = depthStoreOp_;
	return *this;
}

RenderPassBuilder& RenderPassBuilder::setDependency(uint32_t srcSubpass_, uint32_t dstSubpass_, VkPipelineStageFlags srcStage_, VkPipelineStageFlags dstStage_, VkAccessFlags srcAccessFlags_, VkAccessFlags dstAccessFlags_, VkDependencyFlags dependencyFlags_) {
	rpProps.dependencies.push_back({ srcSubpass_,dstSubpass_,srcStage_,dstStage_,srcAccessFlags_,dstAccessFlags_,dependencyFlags_ });
	return *this;
}


VkRenderPass RenderPassBuilder::build() {
	/*RenderPassProperties rpProps;
	rpProps.colorFormat = colorFormat;
	rpProps.sampleCount = samples;
	rpProps.depthFormat = depthFormat;
	rpProps.resolveFormat = resolveFormat;
	rpProps.finalColorLayout = finalColorLayout;
	rpProps.initialColorLayout = initialColorLayout;*/
	return initRenderPass(device, rpProps);
}

void RenderPassBuilder::build(VkRenderPass& renderPass) {
	/*RenderPassProperties rpProps;
	rpProps.colorFormat = colorFormat;
	rpProps.sampleCount = samples;
	rpProps.depthFormat = depthFormat;
	rpProps.resolveFormat = resolveFormat;
	rpProps.finalColorLayout = finalColorLayout;
	rpProps.initialColorLayout = initialColorLayout;*/
	renderPass = initRenderPass(device, rpProps);
}

FramebufferBuilder::FramebufferBuilder(VkDevice device_) :VulkanObject(device_) {

}

FramebufferBuilder FramebufferBuilder::begin(VkDevice device_) {
	FramebufferBuilder builder(device_);
	return builder;
}

FramebufferBuilder& FramebufferBuilder::setColorImageViews(std::vector<VkImageView>& colorImageViews_) {
	colorImageViews = colorImageViews_;
	return *this;
}
FramebufferBuilder& FramebufferBuilder::setColorImageView(VkImageView colorImageView_) {
	colorImageViews.push_back(colorImageView_);
	return *this;
}

FramebufferBuilder& FramebufferBuilder::setDepthImageView(VkImageView depthImageView_) {
	depthImageView = depthImageView_;
	return *this;
}

FramebufferBuilder& FramebufferBuilder::setResolveImageView(VkImageView resolveImageView_) {
	resolveImageView = resolveImageView_;
	return *this;
}

FramebufferBuilder& FramebufferBuilder::setRenderPass(VkRenderPass renderPass_) {
	renderPass = renderPass_;
	return *this;
}

FramebufferBuilder& FramebufferBuilder::setDimensions(uint32_t width_, uint32_t height_) {
	width = width_;
	height = height_;
	return *this;
}


void FramebufferBuilder::build(std::vector<VkFramebuffer>& framebuffers) {
	assert(renderPass != VK_NULL_HANDLE);
	assert(width > 0 && height > 0);
	assert(colorImageViews.size() > 0 || depthImageView != VK_NULL_HANDLE || resolveImageView != VK_NULL_HANDLE);
	FramebufferProperties fbProps;
	fbProps.colorAttachments = colorImageViews.data();
	fbProps.colorAttachmentCount = (uint32_t)colorImageViews.size();
	fbProps.depthAttachment = depthImageView;
	fbProps.resolveAttachment = resolveImageView;
	fbProps.width = width;
	fbProps.height = height;
	initFramebuffers(device, renderPass, fbProps, framebuffers);
}
VulkanImage::VulkanImage(VkDevice device_, Image image_) : VulkanObject(device_), image(image_) {

}

VulkanImage::~VulkanImage() {
	cleanupImage(device, image);
}

void VulkanImage::Transition(VkQueue queue_, VkCommandBuffer commandBuffer_, VkImageLayout prev, VkImageLayout next) {
	transitionImage(device, queue_, commandBuffer_, image.image, prev, next, image.mipLevels, image.layerCount);
}

VulkanSampler::VulkanSampler(VkDevice device_, VkSampler sampler_) : VulkanObject(device_), sampler(sampler_) {

}
VulkanSampler::~VulkanSampler() {
	cleanupSampler(device, sampler);
}

VulkanTexture::VulkanTexture(VkDevice device_, Texture texture_) :VulkanObject(device_), texture(texture_) {

}
VulkanTexture::~VulkanTexture() {
	cleanupTexture(device, texture);
}

VulkanImageList::VulkanImageList(VkDevice device_, std::vector<Image>& images_) : VulkanObject(device_) {
	images = images_;//best way to copy?
}
VulkanImageList::~VulkanImageList() {
	for (auto& image : images) {
		cleanupImage(device, image);
	}
}


VulkanTextureList::VulkanTextureList(VkDevice device_, std::vector<Texture>& textures_) : VulkanObject(device_) {
	textures = textures_;//best way to copy?
}
VulkanTextureList::~VulkanTextureList() {
	for (auto& texture : textures) {
		cleanupTexture(device, texture);
	}
}

VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device_, VkPipelineLayout pipelineLayout_) : VulkanObject(device_), pipelineLayout(pipelineLayout_) {
}

VulkanPipelineLayout::~VulkanPipelineLayout() {
	cleanupPipelineLayout(device, pipelineLayout);
}


VulkanPipeline::VulkanPipeline(VkDevice device_, VkPipeline pipeline_) : VulkanObject(device_), pipeline(pipeline_) {
}

VulkanPipeline::~VulkanPipeline() {
	cleanupPipeline(device, pipeline);
}

VulkanDescriptorList::VulkanDescriptorList(VkDevice device_, std::vector<VkDescriptorSet>& descriptorSetList_) : VulkanObject(device_), descriptorSetList(descriptorSetList_) {

}

VulkanDescriptorList::~VulkanDescriptorList() {

}

}