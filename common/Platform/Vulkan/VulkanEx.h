#pragma once
//#pragma once
//#include <vector>
//#include <array>
#include <unordered_map>
//#include <memory>
#include <algorithm>
#include "Vulkan.h"
namespace Vulkan {

	class InstanceBuilder {
		std::vector<const char*> requiredExtensions;
		std::vector<const char*> requiredLayers;
		InstanceBuilder();
	public:
		static InstanceBuilder begin();
		InstanceBuilder& addRequiredExtension(const char* pExt);
		InstanceBuilder& addRequiredLayer(const char* pLayer);
		void build(VkInstance& instance);
	};

	class VulkanInstance {
		VkInstance instance{ VK_NULL_HANDLE };
	public:
		VulkanInstance() = delete;
		VulkanInstance(VkInstance instance_);
		const VulkanInstance& operator=(const VulkanInstance& rhs) = delete;

		~VulkanInstance();
		operator VkInstance()const { return instance; }
	};

	class VulkanSurface {
		VkInstance instance{ VK_NULL_HANDLE };
		VkSurfaceKHR surface{ VK_NULL_HANDLE };
	public:
		VulkanSurface(VkInstance instance_, VkSurfaceKHR surface_);
		VulkanSurface() = delete;
		~VulkanSurface();
		operator VkSurfaceKHR()const { return surface; }
	};

	class VulkanPhysicalDevice {
		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceMemoryProperties memoryProperties;
	public:
		VulkanPhysicalDevice(VkPhysicalDevice physicalDevice_);
		~VulkanPhysicalDevice();
		operator VkPhysicalDevice()const { return physicalDevice; }
		operator VkPhysicalDeviceProperties& () { return deviceProperties; }
		operator VkPhysicalDeviceMemoryProperties& () { return memoryProperties; }

		VkSurfaceCapabilitiesKHR getSurfaceCaps(VkSurfaceKHR surface_);
		VkFormatProperties getFormatProperties(VkFormat format_);
		std::vector<VkSurfaceFormatKHR> getSurfaceFormats(VkSurfaceKHR surface_);
		std::vector<VkPresentModeKHR> getPresentModes(VkSurfaceKHR surface_);
	};

	class DeviceBuilder {
		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		Queues queues;
		std::vector<const char*> deviceExtensions;
		VkPhysicalDeviceFeatures features{};
		DeviceBuilder(VkPhysicalDevice physicalDevice_, Queues& queues);
	public:
		static DeviceBuilder begin(VkPhysicalDevice physicalDevice, Queues& queues_);
		DeviceBuilder& addDeviceExtension(const char* devExt);
		DeviceBuilder& setFeatures(VkPhysicalDeviceFeatures& deviceFeatures_);
		void build(VkDevice& device_, VkQueue& graphicsQueue_, VkQueue& computeQueue_, VkQueue& presentQueue_);
		void build(VkDevice& device_);
		VkDevice build();
	};

	class VulkanDevice {
		VkDevice device;
		VulkanDevice() = delete;
		const VulkanDevice& operator=(const VulkanDevice& rhs) = delete;
	public:
		VulkanDevice(VkDevice device_);
		~VulkanDevice();
		operator VkDevice()const { return device; }
		VkQueue getQueue(uint32_t queueFamily)const;
	};

	class SwapchainBuilder {
		VkDevice device{ VK_NULL_HANDLE };
		VkSurfaceKHR surface{ VK_NULL_HANDLE };
		VkSurfaceCapabilitiesKHR surfaceCaps;
		VkPresentModeKHR presentMode;
		VkSurfaceFormatKHR swapchainFormat;
		VkExtent2D swapchainExtent;
		uint32_t imageCount{ UINT32_MAX };
		SwapchainBuilder(VkDevice device_, VkSurfaceKHR surface_, VkSurfaceCapabilitiesKHR& surfaceCaps_);
	public:
		static SwapchainBuilder begin(VkDevice device_, VkSurfaceKHR surface_, VkSurfaceCapabilitiesKHR& surfaceCaps_);
		SwapchainBuilder& setPresentMode(VkPresentModeKHR presentMode_);
		SwapchainBuilder& setFormat(VkSurfaceFormatKHR& format_);
		SwapchainBuilder& setExtent(VkExtent2D& extent_);
		SwapchainBuilder& setExtent(uint32_t width, uint32_t height);
		SwapchainBuilder& setImageCount(uint32_t imageCount_);
		VkSwapchainKHR build();
	};

	class VulkanSwapchain {
		VkDevice device{ VK_NULL_HANDLE };
		VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
		VulkanSwapchain() = delete;
		const VulkanSwapchain& operator=(const VulkanSwapchain& rhs) = delete;
	public:
		VulkanSwapchain(VkDevice device_, VkSwapchainKHR swapchain_);
		~VulkanSwapchain();
		operator VkSwapchainKHR()const { return swapchain; }
		std::vector<VkImage> getImages();
	};

	class VulkanSwapchainImageViews {
		VkDevice device{ VK_NULL_HANDLE };
		std::vector<VkImageView> swapchainImageViews;
		VulkanSwapchainImageViews() = delete;
	public:
		VulkanSwapchainImageViews(VkDevice device_, std::vector<VkImageView>& swapchainImageViews_);
		~VulkanSwapchainImageViews();
		operator size_t()const { return swapchainImageViews.size(); }
		const VkImageView& operator[](int i)const { return swapchainImageViews[i]; }
	};

	class VulkanSemaphore {
		VkDevice device{ VK_NULL_HANDLE };
		VkSemaphore semaphore{ VK_NULL_HANDLE };
		VulkanSemaphore() = delete;
		const VulkanSemaphore& operator=(const VulkanSemaphore& rhs) = delete;
	public:
		VulkanSemaphore(VkDevice device_, VkSemaphore semaphore_);
		~VulkanSemaphore();
		operator VkSemaphore()const { return semaphore; }
	};

	class VulkanCommandPool {
		VkDevice device{ VK_NULL_HANDLE };
		VkCommandPool commandPool{ VK_NULL_HANDLE };
		VulkanCommandPool() = delete;
		const VulkanCommandPool& operator=(const VulkanCommandPool& rhs) = delete;
	public:
		VulkanCommandPool(VkDevice device_, VkCommandPool commandPool_);
		~VulkanCommandPool();
		operator VkCommandPool()const { return commandPool; }
	};

	class VulkanCommandBuffer {
		VkDevice device{ VK_NULL_HANDLE };
		VkCommandPool commandPool{ VK_NULL_HANDLE };
		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
		VulkanCommandBuffer() = delete;
		const VulkanCommandBuffer& operator=(const VulkanCommandBuffer& rhs) = delete;
	public:
		VulkanCommandBuffer(VkDevice device_, VkCommandPool commandPool_, VkCommandBuffer commandBuffer_);
		~VulkanCommandBuffer();
		operator VkCommandBuffer()const { return commandBuffer; }
	};

	class VulkanCommandBuffers {
		VkDevice device{ VK_NULL_HANDLE };
		VkCommandPool commandPool{ VK_NULL_HANDLE };
		std::vector<VkCommandBuffer> commandBuffers;
		VulkanCommandBuffers() = delete;
		const VulkanCommandBuffers& operator=(const VulkanCommandBuffers& rhs) = delete;
	public:
		VulkanCommandBuffers(VkDevice device_, VkCommandPool commandPool_, std::vector<VkCommandBuffer>& commandBuffers_);
		~VulkanCommandBuffers();
		operator size_t()const { return commandBuffers.size(); }
		const VkCommandBuffer& operator[](int i)const { return commandBuffers[i]; }
	};


	class VulkanFence {
		VkDevice device{ VK_NULL_HANDLE };
		VkFence fence{ VK_NULL_HANDLE };
		VulkanFence() = delete;
		const VulkanFence& operator=(const VulkanFence& rhs) = delete;
	public:
		VulkanFence(VkDevice device_, VkFence fence_);
		~VulkanFence();
		operator VkFence()const { return fence; }
	};


	class DescriptorSetPoolCache {
		VkDevice device{ VK_NULL_HANDLE };
		VkDescriptorPool getPool();
		VkDescriptorPool currentPool{ VK_NULL_HANDLE };
		std::vector<VkDescriptorPool> allocatedPools;
		std::vector<VkDescriptorPool> freePools;
		VkDescriptorPool createPool();
		DescriptorSetPoolCache() = delete;
	public:
		DescriptorSetPoolCache(VkDevice device_);
		~DescriptorSetPoolCache();
		void resetPools();
		bool allocateDescriptorSet(VkDescriptorSet* pSet, VkDescriptorSetLayout layout);
		bool allocateDescriptorSets(VkDescriptorSet* pSet, VkDescriptorSetLayout* pLayouts, uint32_t count);
		operator VkDevice()const { return device; }
	};


	class DescriptorSetLayoutCache {
		struct DescriptorSetLayoutInfo {
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			bool operator==(const DescriptorSetLayoutInfo& k)const;
			size_t hash()const;
		};
		struct DescriptorSetLayoutHash {
			size_t operator()(const DescriptorSetLayoutInfo& k)const {
				return k.hash();
			}
		};
		std::unordered_map<DescriptorSetLayoutInfo, VkDescriptorSetLayout, DescriptorSetLayoutHash> layoutCache;
		VkDevice device;
		DescriptorSetLayoutCache() = delete;
	public:
		DescriptorSetLayoutCache(VkDevice device_);
		~DescriptorSetLayoutCache();
		VkDescriptorSetLayout create(std::vector<VkDescriptorSetLayoutBinding>& bindings);
		operator VkDevice()const { return device; }
		bool getBindings(VkDescriptorSetLayout layout, std::vector<VkDescriptorSetLayoutBinding>& bindings) {
			bindings.clear();
			for (auto& pair : layoutCache) {
				if (pair.second == layout) {
					bindings = std::vector<VkDescriptorSetLayoutBinding>(pair.first.bindings);
					return true;
				}
			}
			return false;
		}
	};


	class DescriptorSetBuilder {
		DescriptorSetPoolCache* pPool{ nullptr };
		DescriptorSetLayoutCache* pLayout{ nullptr };
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		DescriptorSetBuilder(DescriptorSetPoolCache* pPool_, DescriptorSetLayoutCache* pLayout_);
	public:
		static DescriptorSetBuilder begin(DescriptorSetPoolCache* pPool_, DescriptorSetLayoutCache* pLayout_);
		DescriptorSetBuilder& AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1);
		bool build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
		bool build(VkDescriptorSet& set);
		bool build(std::vector<VkDescriptorSet>& sets, VkDescriptorSetLayout& layout, uint32_t count);
		bool build(std::vector<VkDescriptorSet>& sets, uint32_t count);
	};


	class DescriptorSetUpdater {
		DescriptorSetLayoutCache* pLayout{ nullptr };
		VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<VkWriteDescriptorSet> writes;
		void setBindings();
		DescriptorSetUpdater(DescriptorSetLayoutCache* pLayout_, VkDescriptorSetLayout descriptorSetLayout_, VkDescriptorSet descriptorSet_);
	public:
		static DescriptorSetUpdater begin(DescriptorSetLayoutCache* pLayout_, VkDescriptorSetLayout descriptorSetLayout_, VkDescriptorSet descriptorSet_);
		DescriptorSetUpdater& AddBinding(uint32_t binding, VkDescriptorType type, VkDescriptorBufferInfo* bufferInfo);
		DescriptorSetUpdater& AddBinding(uint32_t binding, VkDescriptorType type, VkDescriptorImageInfo* imageInfo, uint32_t count = 1);
		void update();
	};


	struct UniformBufferInfo {
		VkDeviceSize objectSize{ 0 };
		VkDeviceSize objectCount{ 0 };
		VkDeviceSize repeatCount{ 0 };
		void* ptr{ nullptr };
	};


	class UniformBufferBuilder {

		VkDevice device{ VK_NULL_HANDLE };
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceMemoryProperties memoryProperties;
		VkDescriptorType descriptorType;
		bool isMapped{ false };
		std::vector<UniformBufferInfo> bufferInfo;
		Buffer buffer;
		UniformBufferBuilder(VkDevice device_, VkPhysicalDeviceProperties& devicePropertes_, VkPhysicalDeviceMemoryProperties& memoryProperties_, VkDescriptorType descriptorType_, bool isMapped_);
	public:
		static UniformBufferBuilder begin(VkDevice device_, VkPhysicalDeviceProperties& deviceProperties_, VkPhysicalDeviceMemoryProperties& memoryProperties_, VkDescriptorType descriptorType_, bool isMapped_);
		UniformBufferBuilder& AddBuffer(VkDeviceSize objectSize_, VkDeviceSize objectCount_, VkDeviceSize repeatCount_);
		void build(Buffer& buffer_, std::vector<UniformBufferInfo>& bufferInfo_);
	};

	class VertexBufferBuilder {
		VkDevice device{ VK_NULL_HANDLE };
		VkQueue queue{ VK_NULL_HANDLE };
		VkCommandBuffer cmd{ VK_NULL_HANDLE };
		VkPhysicalDeviceMemoryProperties memoryProperties;
		std::vector<VkDeviceSize> vertexSizes;
		std::vector<float*> vertexPtrs;
		std::vector<uint32_t> vertexLocations;
		bool isMapped{ false };
		VertexBufferBuilder(VkDevice device_, VkQueue queue_, VkCommandBuffer cmd_, VkPhysicalDeviceMemoryProperties& memoryProperties_);
	public:
		static VertexBufferBuilder begin(VkDevice device_, VkQueue queue_, VkCommandBuffer cmd_, VkPhysicalDeviceMemoryProperties& memoryProperties_);
		VertexBufferBuilder& AddVertices(VkDeviceSize vertexSize, float* pVertexData, bool mapped = false);
		void build(Buffer& buffer_, std::vector<uint32_t>& vertexLocations, void** ptr = nullptr);

	};

	class IndexBufferBuilder {
		VkDevice device{ VK_NULL_HANDLE };
		VkQueue queue{ VK_NULL_HANDLE };
		VkCommandBuffer cmd{ VK_NULL_HANDLE };

		VkPhysicalDeviceMemoryProperties memoryProperties;
		std::vector<VkDeviceSize> indexSizes;
		std::vector<uint32_t*> indexPtrs;
		std::vector<uint32_t> indexLocations;
		bool isMapped{ false };
		IndexBufferBuilder(VkDevice device_, VkQueue queue_, VkCommandBuffer cmd_, VkPhysicalDeviceMemoryProperties& memoryProperties_);
	public:
		static IndexBufferBuilder begin(VkDevice device_, VkQueue queue_, VkCommandBuffer cmd_, VkPhysicalDeviceMemoryProperties& memoryProperties_);
		IndexBufferBuilder& AddIndices(VkDeviceSize indexSize, uint32_t* pIndexData, bool mapped = false);
		void build(Buffer& buffer_, std::vector<uint32_t>& indexLocations, void** ptr = nullptr);

	};


	class StagingBufferBuilder {
		VkDevice device{ VK_NULL_HANDLE };
		VkPhysicalDeviceMemoryProperties memoryProperties;
		VkDeviceSize size{ 0 };
		StagingBufferBuilder(VkDevice device_, VkPhysicalDeviceMemoryProperties& properties_);
	public:
		static StagingBufferBuilder begin(VkDevice device_, VkPhysicalDeviceMemoryProperties& properties_);
		StagingBufferBuilder& setSize(VkDeviceSize size);
		Buffer build();
	};

	class ImageLoader {
		VkDevice device{ VK_NULL_HANDLE };
		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
		VkQueue queue{ VK_NULL_HANDLE };
		VkPhysicalDeviceMemoryProperties memoryProperties;
		std::vector<std::string> imagePaths;
		std::vector<bool> enableLods;
		bool isArray{ false };
		bool isCube{ false };
		ImageLoader(VkDevice device_, VkCommandBuffer commandBuffer_, VkQueue queue_, VkPhysicalDeviceMemoryProperties memoryProperties_);
		void loadImage(uint8_t* pixels, uint32_t width, uint32_t height, VkFormat format, bool enableLod, Image& image);
		void loadImageArray(std::vector<uint8_t*>& pixelArray, uint32_t width, uint32_t height, bool enableLod, Image& image);
		void loadCubeMap(std::vector<uint8_t*>& cubeArray, uint32_t width, uint32_t heigth, bool enableLod, Image& image);
	public:
		static ImageLoader begin(VkDevice device_, VkCommandBuffer commandBuffer_, VkQueue queue_, VkPhysicalDeviceMemoryProperties memoryProperties_);
		ImageLoader& addImage(const char* imagePath, bool enableLod = false);
		ImageLoader& setIsCube(bool isCube_);
		ImageLoader& setIsArray(bool isArray_);
		bool load(std::vector<Image>& images);
	};

	class TextureLoader {
		VkDevice device{ VK_NULL_HANDLE };
		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
		VkQueue queue{ VK_NULL_HANDLE };
		VkPhysicalDeviceMemoryProperties memoryProperties;
		std::vector<std::string> imagePaths;
		std::vector<bool> enableLods;
		bool isArray{ false };
		bool isCube{ false };

		TextureLoader(VkDevice device_, VkCommandBuffer commandBuffer_, VkQueue queue_, VkPhysicalDeviceMemoryProperties memoryProperties_);
		void loadTexture(uint8_t* pixels, uint32_t width, uint32_t height, VkFormat format, bool enableLod, Texture& texture);
		void loadCubeMap(std::vector<uint8_t*>& cubeArray, uint32_t width, uint32_t heigth, bool enableLod, Texture& image);
		void loadTextureArray(std::vector<uint8_t*>& pixelArray, uint32_t width, uint32_t height, bool enableLod, Texture& texture);
	public:
		static TextureLoader begin(VkDevice device_, VkCommandBuffer commandBuffer_, VkQueue queue_, VkPhysicalDeviceMemoryProperties memoryProperties_);
		TextureLoader& addTexture(const char* imagePath, bool enableLod = false);
		TextureLoader& setIsArray(bool isArray_);
		TextureLoader& setIsCube(bool isCube_);
		bool load(std::vector<Texture>& textures);

	};


	class ShaderProgramLoader {
		VkDevice device{ VK_NULL_HANDLE };
		std::vector<std::string> shaderPaths;
		std::vector<std::vector<uint32_t>> shaderSpirvs;
		VkVertexInputBindingDescription vertexInputDescription;
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
		std::unordered_map<VkShaderStageFlagBits, VkShaderModule> shaders;
		ShaderProgramLoader(VkDevice device_);
	public:
		static ShaderProgramLoader begin(VkDevice device_);
		ShaderProgramLoader& AddShaderPath(const char* shaderPath);
		ShaderProgramLoader& AddShaderSpirv(const std::vector<uint32_t>& spirv);
		void load(std::vector<ShaderModule>& shaders_);
		bool load(std::vector<ShaderModule>& shaders_, VkVertexInputBindingDescription& vertexInputDescription_, std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions_);
	};



	class PipelineLayoutBuilder {
		VkDevice device{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<VkPushConstantRange> pushConstants;
		PipelineLayoutBuilder(VkDevice device_);
	public:
		static PipelineLayoutBuilder begin(VkDevice device_);
		PipelineLayoutBuilder& AddDescriptorSetLayout(VkDescriptorSetLayout layout_);
		PipelineLayoutBuilder& AddDescriptorSetLayouts(std::vector<VkDescriptorSetLayout>& layouts_);
		PipelineLayoutBuilder& AddPushConstants(std::vector<VkPushConstantRange>& pushConstants);
		void build(VkPipelineLayout& pipelineLayout);
	};


	class PipelineBuilder {
		struct SpecInfo {
			VkShaderStageFlagBits shaderStage{ VK_SHADER_STAGE_VERTEX_BIT };
			uint32_t size;
			union {
				float f;
				uint32_t u;
				int32_t i;
			}val;
		};
		VkDevice device{ VK_NULL_HANDLE };
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		VkRenderPass renderPass{ VK_NULL_HANDLE };
		std::vector<ShaderModule>& shaders;
		VkVertexInputBindingDescription vertexInputDescription;
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
		VkPrimitiveTopology topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
		VkCullModeFlagBits cullMode{ VK_CULL_MODE_BACK_BIT };
		VkPolygonMode polygonMode{ VK_POLYGON_MODE_FILL };
		VkBool32 depthTest{ VK_FALSE };
		VkCompareOp depthCompareOp{ VK_COMPARE_OP_LESS };
		VkBool32 stencilTest{ VK_FALSE };
		VkStencilOpState stencil;

		VkBool32 blend{ VK_FALSE };
		VkBlendFactor srcBlend{ VK_BLEND_FACTOR_SRC_ALPHA };
		VkBlendFactor dstBlend{ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
		VkBlendOp blendOp{ VK_BLEND_OP_ADD };
		VkBlendFactor srcBlendAlpha{ VK_BLEND_FACTOR_ONE };
		VkBlendFactor dstBlendAlpha{ VK_BLEND_FACTOR_ZERO };
		VkBlendOp blendOpAlpha{ VK_BLEND_OP_ADD };
		VkFrontFace frontFace{ VK_FRONT_FACE_COUNTER_CLOCKWISE };
		VkBool32 noDraw{ VK_FALSE };
		std::vector<SpecInfo> specializationInfo;
		VkSampleCountFlagBits samples{ VK_SAMPLE_COUNT_1_BIT };
		PipelineBuilder(VkDevice device_, VkPipelineLayout pipelineLayout_, VkRenderPass renderPass_, std::vector<ShaderModule>& shaders_, VkVertexInputBindingDescription& vertexInputDescription_, std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions_);
	public:
		static PipelineBuilder begin(VkDevice device_, VkPipelineLayout pipelineLayout_, VkRenderPass renderPass_, std::vector<ShaderModule>& shaders_, VkVertexInputBindingDescription& vertexInputDescription_, std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions_);
		PipelineBuilder& setCullMode(VkCullModeFlagBits cullMode_);
		PipelineBuilder& setPolygonMode(VkPolygonMode polygonMode_);
		PipelineBuilder& setFrontFace(VkFrontFace frontFace_);
		PipelineBuilder& setDepthTest(VkBool32 depthTest_);
		PipelineBuilder& setDepthCompareOp(VkCompareOp depthCompare_);
		PipelineBuilder& setBlend(VkBool32 blend_);
		PipelineBuilder& setBlendState(VkBlendFactor srcBlend, VkBlendFactor dstBlend, VkBlendOp blendOp, VkBlendFactor srcBlendAlpha, VkBlendFactor dstBlendAlpha, VkBlendOp blendOpAlpha);
		PipelineBuilder& setStencilTest(VkBool32 stencilTest_);
		PipelineBuilder& setNoDraw(VkBool32 noDraw_);
		PipelineBuilder& setStencilState(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp, uint32_t compareMask, uint32_t writeMask, uint32_t reference);
		PipelineBuilder& setTopology(VkPrimitiveTopology topology_);
		PipelineBuilder& setSpecializationConstant(VkShaderStageFlagBits shaderStage, uint32_t uval);
		PipelineBuilder& setSpecializationConstant(VkShaderStageFlagBits shaderStage, int32_t ival);
		PipelineBuilder& setSpecializationConstant(VkShaderStageFlagBits shaderStage, float fval);
		PipelineBuilder& setSampleCount(VkSampleCountFlagBits samples);
		void build(VkPipeline& pipeline);
	};

	class ComputePipelineBuilder {
		VkDevice device{ VK_NULL_HANDLE };
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
		const char* pShader{ nullptr };
		ComputePipelineBuilder(VkDevice device_, VkPipelineLayout pipelineLayout_);
	public:
		static ComputePipelineBuilder begin(VkDevice device_, VkPipelineLayout pipelineLayout_);
		ComputePipelineBuilder& setShader(const char* pShader_);
		VkPipeline build();
	};

	class VulkanObject {
	protected:
		VkDevice device{ VK_NULL_HANDLE };
		VulkanObject(VkDevice device_) :device(device_) {}
	public:
		operator VkDevice()const { return device; }
	};

	class VulkanBuffer : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		Vulkan::Buffer buffer;
	public:
		VulkanBuffer() = delete;
		VulkanBuffer(VkDevice device_, Vulkan::Buffer& buffer_);
		~VulkanBuffer();
		operator VkBuffer()const { return buffer.buffer; }
		VkBuffer getBuffer()const { return buffer.buffer; }
		Vulkan::Buffer getVulkanBuffer()const { return buffer; }
		operator Vulkan::Buffer()const { return buffer; }
	};

	class VulkanUniformBuffer :public VulkanBuffer {
		std::vector<UniformBufferInfo> bufferInfo;
	public:
		VulkanUniformBuffer() = delete;
		VulkanUniformBuffer(VkDevice device_, Vulkan::Buffer& buffer_, std::vector<UniformBufferInfo>& bufferInfo_);
		operator size_t()const { return bufferInfo.size(); }
		UniformBufferInfo operator[](int i)const { return bufferInfo[i]; }
	};

	class VulkanVIBuffer : public VulkanBuffer {
		std::vector<uint32_t> bufferLocations;
	public:
		VulkanVIBuffer() = delete;
		VulkanVIBuffer(VkDevice device_, Vulkan::Buffer& buffer_, std::vector<uint32_t>& bufferLocations_);
		operator size_t()const { return bufferLocations.size(); }
		uint32_t operator[](int i)const { return bufferLocations[i]; }
	};

	class ImageBuilder {
		VkDevice device{ VK_NULL_HANDLE };
		VkPhysicalDeviceMemoryProperties memoryProperties;
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint32_t mipLevels{ 1 };
		VkFormat format{ PREFERRED_IMAGE_FORMAT };
		VkImageUsageFlags imageUsage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
		VkImageAspectFlags aspect{ VK_IMAGE_ASPECT_COLOR_BIT };
		VkImageLayout imageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkSampleCountFlagBits samples{ VK_SAMPLE_COUNT_1_BIT };
		ImageBuilder(VkDevice device_, VkPhysicalDeviceMemoryProperties& memoryProperties_);
	public:
		static ImageBuilder begin(VkDevice device_, VkPhysicalDeviceMemoryProperties& memorProperties_);
		ImageBuilder& setDimensions(uint32_t width_, uint32_t height_);
		ImageBuilder& setFormat(VkFormat format_);
		ImageBuilder& setImageUsage(VkImageUsageFlags imageUsage_);
		ImageBuilder& setImageAspectFlags(VkImageAspectFlags aspectFlags_);
		ImageBuilder& setImageLayout(VkImageLayout layout_);
		ImageBuilder& setSampleCount(VkSampleCountFlagBits samples_);
		ImageBuilder& setMipLevels(uint32_t mipLevels);
		Image build();
	};

	class TextureBuilder {
		VkDevice device{ VK_NULL_HANDLE };
		VkPhysicalDeviceMemoryProperties memoryProperties;
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint32_t mipLevels{ 1 };
		VkFormat format{ PREFERRED_IMAGE_FORMAT };
		VkFilter filter{ VK_FILTER_LINEAR };
		VkImageUsageFlags imageUsage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
		VkImageAspectFlags aspect{ VK_IMAGE_ASPECT_COLOR_BIT };
		VkImageLayout imageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkSampleCountFlagBits samples{ VK_SAMPLE_COUNT_1_BIT };
		TextureBuilder(VkDevice device_, VkPhysicalDeviceMemoryProperties& memoryProperties_);
	public:
		static TextureBuilder begin(VkDevice device_, VkPhysicalDeviceMemoryProperties& memorProperties_);
		TextureBuilder& setDimensions(uint32_t width_, uint32_t height_);
		TextureBuilder& setFormat(VkFormat format_);
		TextureBuilder& setImageUsage(VkImageUsageFlags imageUsage_);
		TextureBuilder& setImageAspectFlags(VkImageAspectFlags aspectFlags_);
		TextureBuilder& setImageLayout(VkImageLayout layout_);
		TextureBuilder& setSampleCount(VkSampleCountFlagBits samples_);
		TextureBuilder& setMipLevels(uint32_t mipLevels);
		TextureBuilder& setFilter(VkFilter filter_);
		Vulkan::Texture build();
	};


	class RenderPassBuilder : public VulkanObject {
		RenderPassProperties rpProps;
		/*VkFormat colorFormat{ VK_FORMAT_UNDEFINED };
		VkSampleCountFlagBits samples{ VK_SAMPLE_COUNT_1_BIT };
		VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
		VkFormat resolveFormat{ VK_FORMAT_UNDEFINED };
		VkImageLayout colorinitialColorLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageLayout finalColorLayout{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };*/
		RenderPassBuilder(VkDevice device_);
	public:
		static RenderPassBuilder begin(VkDevice device_);
		RenderPassBuilder& setColorFormat(VkFormat colorFormat);
		RenderPassBuilder& setDepthFormat(VkFormat depthFormat);
		RenderPassBuilder& setResolveFormat(VkFormat resolveFormat);
		RenderPassBuilder& setSampleCount(VkSampleCountFlagBits samples);
		RenderPassBuilder& setColorFinalLayout(VkImageLayout colorLayout_);
		RenderPassBuilder& setColorInitialLayout(VkImageLayout colorLayout_);
		RenderPassBuilder& setColorLoadOp(VkAttachmentLoadOp colorLoadOp_);
		RenderPassBuilder& setColorStoreOp(VkAttachmentStoreOp colorStoreOp_);
		RenderPassBuilder& setDepthInitialLayout(VkImageLayout depthLayout_);
		RenderPassBuilder& setDepthFinalLayout(VkImageLayout depthLayout_);
		RenderPassBuilder& setDepthLoadOp(VkAttachmentLoadOp depthLoadOp_);
		RenderPassBuilder& setDepthStoreOp(VkAttachmentStoreOp depthStoreOp_);
		RenderPassBuilder& setDependency(uint32_t srcSubpass_, uint32_t dstSubpass_, VkPipelineStageFlags srcStage_, VkPipelineStageFlags dstStage_, VkAccessFlags srcAccessFlags_, VkAccessFlags dstAccessFlags_, VkDependencyFlags dependencyFlags_);
		VkRenderPass build();
		void build(VkRenderPass& renderPass_);
	};

	class FramebufferBuilder :public VulkanObject {
		std::vector<VkImageView> colorImageViews;
		VkImageView depthImageView{ VK_NULL_HANDLE };
		VkImageView resolveImageView{ VK_NULL_HANDLE };
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		VkRenderPass renderPass{ VK_NULL_HANDLE };
		FramebufferBuilder(VkDevice device_);
	public:
		static FramebufferBuilder begin(VkDevice device);
		FramebufferBuilder& setColorImageViews(std::vector<VkImageView>& colorImageViews_);
		FramebufferBuilder& setColorImageView(VkImageView colorImageView_);
		FramebufferBuilder& setDepthImageView(VkImageView depthImageView_);
		FramebufferBuilder& setResolveImageView(VkImageView resolveImageView_);
		FramebufferBuilder& setDimensions(uint32_t width_, uint32_t height_);
		FramebufferBuilder& setRenderPass(VkRenderPass renderPass_);

		void build(std::vector<VkFramebuffer>& framebuffers);
	};

	class VulkanImage : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		Image image;
		VulkanImage() = delete;
		const VulkanImage& operator=(const VulkanImage& rhs) = delete;
	public:
		VulkanImage(VkDevice device_, Image image_);
		~VulkanImage();
		operator Image()const { return image; }
		operator VkImage()const { return image.image; }
		operator VkImageView()const { return image.imageView; }
		void Transition(VkQueue queue_, VkCommandBuffer commandBuffer_, VkImageLayout prev, VkImageLayout next);
	};

	class VulkanSampler : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		VkSampler sampler{ VK_NULL_HANDLE };
	public:
		VulkanSampler() = delete;
		const VulkanSampler& operator=(const VulkanSampler& rhs) = delete;
		VulkanSampler(VkDevice device_, VkSampler sampler_);
		~VulkanSampler();
		operator VkSampler()const { return sampler; }
	};

	class VulkanTexture : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		Texture texture;
	public:
		VulkanTexture(VkDevice device_, Texture texture_);
		~VulkanTexture();
		operator VkImageView()const { return texture.imageView; }
		operator VkSampler()const { return texture.sampler; };
		operator VkImage()const { return texture.image; }
		operator Image()const { return texture; }
		operator Texture()const { return texture; }

	};

	class VulkanImageList : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		std::vector<Image> images;
	public:
		VulkanImageList() = delete;
		const VulkanImageList& operator=(const VulkanImageList& rhs) = delete;
		VulkanImageList(VkDevice device_, std::vector<Image>& images_);
		~VulkanImageList();
		operator size_t()const { return images.size(); }
		const Image& operator[](int n)const { return images[n]; }

	};

	class VulkanTextureList : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		std::vector<Texture> textures;
	public:
		VulkanTextureList() = delete;
		VulkanTextureList(VkDevice device_, std::vector<Texture>& textures_);
		~VulkanTextureList();
		operator size_t()const { return textures.size(); }
		const Texture& operator[](int i)const { return textures[i]; }
	};

	class VulkanDescriptor : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		VkDescriptorSet descriptor{ VK_NULL_HANDLE };
	public:
		VulkanDescriptor(VkDevice device_, VkDescriptorSet descriptorSet_) :VulkanObject(device_), descriptor(descriptorSet_) {}

		operator VkDescriptorSet()const { return descriptor; }
		VkDescriptorSet getDescriptor()const { return descriptor; }
	};


	class VulkanDescriptorList : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSet> descriptorSetList;
	public:
		VulkanDescriptorList(VkDevice device_, std::vector<VkDescriptorSet>& descriptorSetList_);
		~VulkanDescriptorList();
		VkDescriptorSet operator[](int i) { return descriptorSetList[i]; }
	};

	class VulkanPipelineLayout : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	public:
		VulkanPipelineLayout() = delete;
		VulkanPipelineLayout(VkDevice device_, VkPipelineLayout pipelineLayout_);
		~VulkanPipelineLayout();
		operator VkPipelineLayout()const {
			return pipelineLayout;
		}
	};

	class VulkanPipeline : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		VkPipeline pipeline{ VK_NULL_HANDLE };
	public:
		VulkanPipeline() = delete;
		VulkanPipeline(VkDevice device_, VkPipeline pipeline_);
		~VulkanPipeline();
		operator VkPipeline()const {
			return pipeline;
		}
	};

	class VulkanFramebuffer : public VulkanObject {
		VkFramebuffer frameBuffer{ VK_NULL_HANDLE };
	public:
		VulkanFramebuffer(VkDevice device_, VkFramebuffer frameBuffer_) :VulkanObject(device_), frameBuffer(frameBuffer_) {}
		~VulkanFramebuffer() { std::vector<VkFramebuffer> frameBuffers{ frameBuffer }; cleanupFramebuffers(device, frameBuffers); }
		operator VkFramebuffer()const { return frameBuffer; }
	};

	class VulkanFramebufferList : public VulkanObject {
		std::vector<VkFramebuffer> frameBuffers;
	public:
		VulkanFramebufferList(VkDevice device_, std::vector<VkFramebuffer>& frameBuffers_) :VulkanObject(device_), frameBuffers(frameBuffers_) {}
		~VulkanFramebufferList() { cleanupFramebuffers(device, frameBuffers); }
		const VkFramebuffer& operator [](uint32_t i)const { return frameBuffers[i]; }
	};


	class VulkanRenderPass : public VulkanObject {
		//VkDevice device{ VK_NULL_HANDLE };
		VkRenderPass renderPass{ VK_NULL_HANDLE };
	public:
		VulkanRenderPass(VkDevice device_, VkRenderPass renderPass_) :VulkanObject(device_), renderPass(renderPass_) {}
		~VulkanRenderPass() { cleanupRenderPass(device, renderPass); }
		operator VkRenderPass()const { return renderPass; }
	};
}