#pragma once
#include <string>
#include <vector>
#include <cassert>

#ifdef __USE__VOLK__
#include <volk/volk.h>
#define VK_NO_PROTOTYPES
#else

#include <vulkan/vulkan.h>

#endif
#define __USE__VMA__
#ifdef __USE__VMA__
#ifdef _DEBUG
#define VMA_DEBUG_LOG(format, ...) do { \
        printf(format, __VA_ARGS__); \
        printf("\n"); \
    } while(false)

#endif
#include "vk_mem_alloc.h"
#endif

#define PREFERRED_FORMAT VK_FORMAT_B8G8R8A8_UNORM
#define PREFERRED_IMAGE_FORMAT VK_FORMAT_R8G8B8A8_UNORM
struct GLFWwindow;
namespace Vulkan {

	VkInstance initInstance(std::vector<const char*>& requiredExtensions, std::vector<const char*>& requiredLayers);
	void cleanupInstance(VkInstance instance);

	/*struct HINSTANCE;
	struct HWND;
	VkSurfaceKHR initSurfaceWin32(VkInstance instance, HINSTANCE hInstance, HWND hWnd);
	*/
	VkSurfaceKHR initSurfaceGLFW(VkInstance, GLFWwindow* window);
	void cleanupSurface(VkInstance instance, VkSurfaceKHR surface);

	struct Queues {
		uint32_t graphicsQueueFamily;
		uint32_t presentQueueFamily;
		uint32_t computeQueueFamily;
	};

	VkPhysicalDevice choosePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, Queues& queues);

	bool supportsDeviceExtension(VkPhysicalDevice, const char* pExtension);
	VkDevice initDevice(VkPhysicalDevice physicalDevice, std::vector<const char*> deviceExtensions, Queues queues, VkPhysicalDeviceFeatures enabledFeatures, uint32_t queueCount = 1);
	void cleanupDevice(VkDevice device);

	VkQueue getDeviceQueue(VkDevice device, uint32_t queueFamily, uint32_t queueIndex = 0);

	VkPresentModeKHR chooseSwapchainPresentMode(std::vector<VkPresentModeKHR>& presentModes);

	VkSurfaceFormatKHR chooseSwapchainFormat(std::vector<VkSurfaceFormatKHR>& formats, VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_UNORM);

	VkSurfaceTransformFlagsKHR chooseSwapchainTransform(VkSurfaceCapabilitiesKHR& surfaceCaps);

	VkCompositeAlphaFlagBitsKHR chooseSwapchainComposite(VkSurfaceCapabilitiesKHR& surfaceCaps);

	VkSemaphore initSemaphore(VkDevice device);
	void cleanupSemaphore(VkDevice device, VkSemaphore semaphore);

	VkCommandPool initCommandPool(VkDevice device, uint32_t queueFamily);
	void initCommandPools(VkDevice device, size_t size, uint32_t queueFamily, std::vector<VkCommandPool>& commandPools);
	void initCommandPools(VkDevice device, size_t size, uint32_t queueFamily, VkCommandPool* commandPools);
	VkCommandBuffer initCommandBuffer(VkDevice device, VkCommandPool commandPool);
	void initCommandBuffers(VkDevice device, std::vector<VkCommandPool>& commandPools, std::vector<VkCommandBuffer>& commandBuffers);
	void initCommandBuffers(VkDevice device, VkCommandPool* commandPools, VkCommandBuffer* commandBuffers, uint32_t count);
	void cleanupCommandBuffers(VkDevice device, std::vector<VkCommandPool>& commandPools, std::vector<VkCommandBuffer>& commandBuffers);
	void cleanupCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer);
	void cleanupCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBuffers, uint32_t count);
	void cleanupCommandPools(VkDevice device, std::vector<VkCommandPool>& commandPools);
	void cleanupCommandPools(VkDevice device, VkCommandPool* commandPools,uint32_t count);
	void cleanupCommandPool(VkDevice device, VkCommandPool commandPool);

	VkSwapchainKHR initSwapchain(VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkSurfaceCapabilitiesKHR& surfaceCaps, VkPresentModeKHR& presentMode, VkSurfaceFormatKHR& swapchainFormat, VkExtent2D& swapchainExtent, uint32_t imageCount = UINT32_MAX, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
	void getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>& images);
	void getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, VkImage* images, uint32_t& imageCount);
	void initSwapchainImageViews(VkDevice device, std::vector<VkImage>& swapchainImages, VkFormat& swapchainFormat, std::vector<VkImageView>& swapchainImageViews);
	void initSwapchainImageViews(VkDevice device, VkImage* swapchainImages, VkFormat& swapchainFormat, VkImageView* swapchainImageViews, uint32_t imageCount);
	void cleanupSwapchainImageViews(VkDevice device, std::vector<VkImageView>& imageViews);
	void cleanupSwapchainImageViews(VkDevice device, VkImageView* imageViews, uint32_t count);
	void cleanupSwapchain(VkDevice device, VkSwapchainKHR swapchain);
	uint32_t getSwapchainImageCount(VkSurfaceCapabilitiesKHR& surfaceCaps);

	uint32_t findMemoryType(uint32_t typeFilter, VkPhysicalDeviceMemoryProperties memoryProperties, VkMemoryPropertyFlags properties);
	VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDeviceProperties deviceProperties);



	struct ImageProperties {
		VkFormat format{ VK_FORMAT_R8G8B8A8_UNORM };
		VkImageUsageFlags imageUsage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
#ifdef __USE__VMA__
		VmaMemoryUsage usage;
#else
		VkMemoryPropertyFlagBits memoryProps{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
#endif
		VkSampleCountFlagBits samples{ VK_SAMPLE_COUNT_1_BIT };//set to sample count for MSAA
		VkImageAspectFlags aspect{ VK_IMAGE_ASPECT_COLOR_BIT };//change for depth
		VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };//images are usually transitions, but set if not
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint32_t mipLevels{ 1 };//0 = calculate from width/height
		uint32_t layers{ 1 };//set to 6 for cubemap		
		bool isCubeMap{ false };
	};

	inline uint32_t getMipLevels(uint32_t width, uint32_t height);
	inline uint32_t getMipLevels(ImageProperties& image);

	struct Image {
		VkImage	image{ VK_NULL_HANDLE };
#ifdef __USE__VMA__
		VmaAllocation allocation{ VK_NULL_HANDLE };
		VmaAllocationInfo allocationInfo;
#else
		VkDeviceMemory memory{ VK_NULL_HANDLE };
#endif
		VkImageView imageView{ VK_NULL_HANDLE };

		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint32_t mipLevels{ 1 };
		uint32_t layerCount{ 1 };
	};

	struct Texture :public Image {
		VkSampler sampler{ VK_NULL_HANDLE };
	};
	void initImage(VkDevice device, /*VkFormatProperties& formatProperties,*/ VkPhysicalDeviceMemoryProperties& memoryProperties, ImageProperties& props, Image& image);
	void initImage(VkDevice device, VkImageCreateInfo& pCreateInfo, Image& image, bool isMapped = false);
#ifdef __USE__VMA__
	void setImageName(Image& image, const char* pname);
#else
#define setImageName(b,p)
#endif
	struct SamplerProperties {
		VkFilter filter{ VK_FILTER_LINEAR };
		VkSamplerAddressMode addressMode{ VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE };
	};
	VkSampler initSampler(VkDevice device, SamplerProperties& samplerProps, uint32_t mipLevels = 1);
	void initSampler(VkDevice device, SamplerProperties& samplerProps, Texture& image);
	struct TextureProperties : public ImageProperties {
		SamplerProperties samplerProps;
	};
	void initTexture(VkDevice device, /*VkFormatProperties& formatProperties,*/ VkPhysicalDeviceMemoryProperties& memoryProperties, TextureProperties& props, Texture& image);
#ifdef __USE__VMA__
	void setTextureName(Texture& text, const char* pname);
#else
#define setTextureName(b,p)
#endif
	void cleanupImage(VkDevice device, Image& image);
	void cleanupSampler(VkDevice device, VkSampler sampler);
	void cleanupTexture(VkDevice device, Texture& texture);

	VkFormat getSupportedDepthFormat(VkPhysicalDevice physicalDevice);
	void generateMipMaps(VkDevice device, VkQueue queue, VkCommandBuffer cmd, Image& image);
	struct Buffer;
	void CopyBufferToImage(VkDevice device, VkQueue queue, VkCommandBuffer cmd, Buffer& src, Image& dst, uint32_t width, uint32_t height, VkDeviceSize offset = 0, uint32_t arrayLayer = 0);
	void CopyBufferToImage(VkDevice device, VkQueue queue, VkCommandBuffer cmd, Buffer& src, Image& dst, std::vector<VkBufferImageCopy>& copyRegions);
	void transitionImage(VkDevice device, VkQueue queue, VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1, uint32_t layoutIndex = 0);
	void transitionImageNoSubmit(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, uint32_t layerCount = 1, uint32_t layoutIndex = 0);


	struct RenderPassProperties {
		VkFormat colorFormat{ PREFERRED_FORMAT };
		VkFormat depthFormat{ VK_FORMAT_D24_UNORM_S8_UINT };
		VkFormat resolveFormat{ VK_FORMAT_UNDEFINED };
		VkSampleCountFlagBits sampleCount{ VK_SAMPLE_COUNT_1_BIT };
		VkImageLayout colorInitialLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageLayout colorFinalLayout{ VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
		VkAttachmentLoadOp colorLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
		VkAttachmentStoreOp colorStoreOp{ VK_ATTACHMENT_STORE_OP_STORE };
		VkAttachmentLoadOp depthLoadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR };
		VkAttachmentStoreOp depthStoreOp{ VK_ATTACHMENT_STORE_OP_DONT_CARE };
		VkImageLayout depthInitialLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageLayout depthFinalLayout{ VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		std::vector<VkSubpassDependency> dependencies;
	};

	VkRenderPass initRenderPass(VkDevice device, RenderPassProperties& props);



	void cleanupRenderPass(VkDevice device, VkRenderPass renderPass);

	struct FramebufferProperties {
		VkImageView* colorAttachments;
		uint32_t colorAttachmentCount{ 0 };
		VkImageView depthAttachment{ VK_NULL_HANDLE };
		VkImageView resolveAttachment{ VK_NULL_HANDLE };
		uint32_t width{ 0 };
		uint32_t height{ 0 };

	};

	void initFramebuffers(VkDevice device, VkRenderPass renderPass, FramebufferProperties& props, std::vector<VkFramebuffer>& framebuffers);
	void initFramebuffers(VkDevice device, VkRenderPass renderPass, FramebufferProperties& props, VkFramebuffer* framebuffers);
	void cleanupFramebuffers(VkDevice device, std::vector<VkFramebuffer>& framebuffers);
	void cleanupFramebuffers(VkDevice device, VkFramebuffer* framebuffers, uint32_t count);


	struct BufferProperties {
		VkBufferUsageFlags bufferUsage{ VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
#ifdef __USE__VMA__
		VmaMemoryUsage usage{ VMA_MEMORY_USAGE_GPU_ONLY };
#else
		VkMemoryPropertyFlags memoryProps{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
#endif
		VkDeviceSize size{ 0 };
	};

	struct Buffer {
		VkBuffer	buffer{ VK_NULL_HANDLE };
#ifdef __USE__VMA__
		VmaAllocation allocation{ VK_NULL_HANDLE };
		VmaAllocationInfo allocationInfo{};
#else
		VkDeviceMemory memory{ VK_NULL_HANDLE };
#endif
		VkDeviceSize size{ 0 };

	};

	void initBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties& memoryProperties, BufferProperties& props, Buffer& buffer);
#if defined(__USE__VMA__) && defined(_DEBUG)
	void setBufferName(Buffer& buffer, const char* pname);
#else
#define setBufferName(b,p)
#endif
	void cleanupBuffer(VkDevice device, Buffer& buffer);

	void* mapBuffer(VkDevice device, Buffer& buffer);

	void unmapBuffer(VkDevice device, Buffer& buffer);
	void flushBuffer(VkDevice device, Buffer& buffer, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	void CopyBufferTo(VkDevice device, VkQueue queue, VkCommandBuffer cmd, Buffer& src, Buffer& dst, VkDeviceSize size);
	void CopyBufferTo(VkDevice device, VkQueue queue, VkCommandBuffer cmd, VkFence fence, Buffer& src, Buffer& dst, VkDeviceSize size);


	VkFence initFence(VkDevice device, VkFenceCreateFlags flags = 0);
	void cleanupFence(VkDevice device, VkFence fence);


	VkDescriptorSetLayout initDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>& descriptorBindings);
	VkDescriptorPool initDescriptorPool(VkDevice deviced, VkDescriptorPoolSize* pPoolSizes, uint32_t poolSizeCount, uint32_t maxSets);
	VkDescriptorPool initDescriptorPool(VkDevice device, std::vector<VkDescriptorPoolSize>& descriptorPoolSizes, uint32_t maxSets);
	VkDescriptorSet initDescriptorSet(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
	void initDescriptorSets(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, VkDescriptorSet* pSets, uint32_t setCount);
	void updateDescriptorSets(VkDevice device, std::vector<VkWriteDescriptorSet> descriptorWrites);
	void cleanupDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool);
	void cleanupDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);

	VkPipelineLayout initPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);
	VkPipelineLayout initPipelineLayout(VkDevice device, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
	VkPipelineLayout initPipelineLayout(VkDevice device, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, std::vector<VkPushConstantRange>& pushConstants);
	void cleanupPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout);

	struct ShaderModule {
		VkShaderModule shaderModule;
		VkShaderStageFlagBits stage;
	};

	VkShaderModule initShaderModule(VkDevice device, const char* filename);
	VkShaderModule initShaderModule(VkDevice device, const std::vector<uint32_t>& spirv);
	void cleanupShaderModule(VkDevice device, VkShaderModule shaderModule);

	struct PipelineInfo {
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkFrontFace	frontFace = VK_FRONT_FACE_CLOCKWISE;
		VkCullModeFlagBits	cullMode = VK_CULL_MODE_BACK_BIT;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkBool32	depthTest = VK_FALSE;
		VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
		VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
		VkBool32	stencilTest = VK_FALSE;
		VkStencilOpState stencil;
		VkBool32	blend = VK_FALSE;
		VkBool32	noDraw = VK_FALSE;
		std::vector<VkPipelineColorBlendAttachmentState> attachementStates;//set if using attachment buffers that aren't output framebuffer
		VkBool32 noInputState = VK_FALSE;//use for drawing creating vertices in vertex shader
		//Specialization stuff, leave nullptr if unused
		VkShaderStageFlagBits specializationStage{ VK_SHADER_STAGE_VERTEX_BIT };
		uint32_t specializationSize{ 0 };
		uint8_t* specializationData{ nullptr };
		std::vector<VkSpecializationMapEntry> specializationMap;
	};

	VkPipeline initGraphicsPipeline(VkDevice, VkRenderPass renderPass, VkPipelineLayout pipelineLayout, std::vector<ShaderModule>& shaders, VkVertexInputBindingDescription& bindingDescription, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, PipelineInfo& pipelineInfo);
	VkPipeline initComputePipeline(VkDevice device, VkPipelineLayout pipelineLayout, ShaderModule& shader);
	void cleanupPipeline(VkDevice device, VkPipeline pipeline);

	/*void saveImageJPG(VkDevice device, VkCommandBuffer cmd, VkQueue queue, VkImage srcImage, VkImageLayout srcLayout, VkPhysicalDeviceMemoryProperties& memoryProperties,

		VkFormatProperties& formatProperties, VkFormat colorFormat, uint32_t width, uint32_t height, const char* fileName);*/


}