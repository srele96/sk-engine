// https://www.glfw.org/docs/3.3/vulkan_guide.html
// vulkan must be included before glfw
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
// Fixes: undefined reference to `vmaCreateAllocator'
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "vulkan/vk_enum_string_helper.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  std::cerr << "Debug(Validation Layer): " << pCallbackData->pMessage << "\n";

  // The pfnUserCallback function is called by the Vulkan implementation for
  // events that match any bit set in the VkDebugUtilsMessageSeverityFlagBitsEXT
  // and VkDebugUtilsMessageTypeFlagBitsEXT that were included in
  // VkDebugUtilsMessengerCreateInfoEXT passed when creating
  // VkDebugUtilsMessengerEXT object. The callback trigger conditions are also
  // controlled by the contents of pMessageIdName and pMessageIdNumber in
  // VkDebugUtilsMessengerCallbackDataEXT.
  //
  // The application should always return VK_FALSE. The VK_TRUE value is
  // reserved for use in layer development.
  return VK_FALSE;
}

std::vector<uint32_t> loadSPIRV(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Error. Failed to open SPIR-V file: " + filename);
  }

  file.seekg(0, std::ios::end);
  std::streampos size = file.tellg();

  if (size % 4 != 0) {
    throw std::runtime_error("Error. SPIR-V file size is not a multiple of 4");
  }

  file.seekg(0, std::ios::beg);

  std::vector<uint32_t> buffer(size / 4);
  if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
    throw std::runtime_error("Error. Failed to read SPIR-V file");
  }

  return buffer;
}

VkShaderModule createShaderModule(VkDevice device,
                                  const std::vector<uint32_t> &code) {
  VkShaderModuleCreateInfo shaderModuleCreateInfo{};
  shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  // should receive the size in bytes
  shaderModuleCreateInfo.codeSize = code.size() * sizeof(uint32_t);
  shaderModuleCreateInfo.pCode = code.data();

  VkShaderModule shaderModule;
  if (VkResult result = vkCreateShaderModule(device, &shaderModuleCreateInfo,
                                             nullptr, &shaderModule);
      result != VK_SUCCESS) {
    throw std::runtime_error(
        std::string("Error. Failed to create shader module: ") +
        string_VkResult(result));
  }

  return shaderModule;
}

int main(int argc, char **argv) {
  uint32_t vulkanVersion;

  if (VkResult result = vkEnumerateInstanceVersion(&vulkanVersion);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to get Vulkan version: "
              << string_VkResult(result) << "\n";
  } else {
    std::cout << "Vulkan API version: " << VK_VERSION_MAJOR(vulkanVersion)
              << "." << VK_VERSION_MINOR(vulkanVersion) << "."
              << VK_VERSION_PATCH(vulkanVersion) << "\n";
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Vulkan Instance Creation:
  //
  //   Create a Vulkan instance, which is the connection between your
  //   application and the Vulkan library. Specify any desired instance-level
  //   extensions and validation layers.

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Vulkan";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  // check available layers on my machine because i dont see output from
  // validation layers
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  std::cout << "(Info): InstanceLayerProperties:\n";
  for (const auto &layerProperties : availableLayers) {
    std::cout << "  - " << layerProperties.layerName << "\n";
  }
  // end checking validation layers

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"}; // the app is freezing when creating
                                      // swapchain so i have to find out a way
                                      // to debug it and find out the reason
                                      // that happens

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
  createInfo.ppEnabledLayerNames = validationLayers.data();

  // Always initialize before querying glfw required extensions, otherwise it
  // won't query any
  if (!glfwInit()) {
    std::cerr << "Error. Failed to initialize GLFW\n";
    return EXIT_FAILURE;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // Configure GLFW for Vulkan
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // Query the required instance extensions from GLFW
  uint32_t glfwExtensionCount = 0;
  // extensions are required to use glfw for window creation with vulkan
  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::cout << "(Info): glfwExtensionCount is: " << glfwExtensionCount << "\n"
            << "(Info): glfwExtensions are:\n";
  for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
    std::cout << "  - " << glfwExtensions[i] << "\n";
  }

  // append more extensions
  std::vector<const char *> allExtensions(glfwExtensions,
                                          glfwExtensions + glfwExtensionCount);
  // layer for debug callback requires debug utils extension to be enabled
  // https://vulkan.lunarg.com/doc/view/1.3.204.1/windows/layer_configuration.html
  allExtensions.push_back("VK_EXT_debug_utils");

  std::cout << "(Info): Instance extensions:\n";
  for (const auto &extension : allExtensions) {
    std::cout << "  - " << extension << "\n";
  }

  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(allExtensions.size());
  createInfo.ppEnabledExtensionNames = allExtensions.data();

  VkInstance instance;

  if (VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to create instance: " << result << "\n";
  } else {
    std::cout << "Success. Created vulkan instance\n";
  }

  // setup debug because the app is freezing on swapchain creation
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
  debugCreateInfo.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugCreateInfo.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  debugCreateInfo.pfnUserCallback = debugCallback;

  // https://www.reddit.com/r/vulkan/comments/czluzc/undefined_reference_to/
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
      reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

  if (vkCreateDebugUtilsMessengerEXT == nullptr) {
    std::cerr << "Errror. Could not load vkCreateDebugUtilsMessengerEXT\n";
  } else {
    std::cout << "Success. Loaded vkCreateDebugUtilsMessengerEXT\n";
  }

  // Extemely usefull. Once I loaded the callback here, I saw the following
  // error log:
  // validation layer: vkCreateSwapchainKHR: Driver's function
  // pointer was NULL, returning VK_SUCCESS. Was the VK_KHR_swapchain extension
  // enabled?
  VkDebugUtilsMessengerEXT debugMessenger;
  if (VkResult result = vkCreateDebugUtilsMessengerEXT(
          instance, &debugCreateInfo, nullptr, &debugMessenger);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to create CreateDebugUtilsMessengerEXT: "
              << string_VkResult(result) << "\n";
  } else {
    std::cout << "Success. Created CreateDebugUtilsMessengerEXT\n";
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Physical Device and Queue Family Selection:
  //
  //   Enumerate and select a suitable physical device (GPU) from the available
  //   devices. Identify and select the necessary queue families (e.g.,
  //   graphics, compute, transfer).

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    std::cerr << "Error. No Vulkan-compatible GPUs found!"
              << "\n";
    // Handle this case gracefully
  } else {
    std::cout << "Success. Device count is: " << deviceCount << "\n";
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

  VkPhysicalDevice selectedDevice;
  bool suitableDeviceFound = false;
  uint32_t graphicsQueueFamilyIndex = 0;

  // look for suitable physical device
  for (const auto &physicalDevice : physicalDevices) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             nullptr);

    if (queueFamilyCount == 0) {
      // This physical device has no queue families; it's not suitable.
      continue;
    }

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                             queueFamilies);

    // Check if the device has a queue family that supports graphics operations.
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        // select the device
        selectedDevice = physicalDevice;
        suitableDeviceFound = true;
        graphicsQueueFamilyIndex = i;
        break;
      }
    }

    if (suitableDeviceFound) {
      // You've found a suitable device.
      break;
    }
  }

  if (suitableDeviceFound) {
    std::cout
        << "(Info): Found suitable GPU device with graphics queue family\n";

    // print the information about the suitable device

    // Get physical device properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(selectedDevice, &deviceProperties);

    // Get physical device features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(selectedDevice, &deviceFeatures);

    std::cout << "(Info): Selected Device Properties:\n";
    std::cout << "  - Device Name: " << deviceProperties.deviceName << "\n";
    std::cout << "  - Device Type: " << deviceProperties.deviceType << "\n";
    std::cout << "  - API Version: " << deviceProperties.apiVersion << "\n";
    std::cout << "  - Driver Version: " << deviceProperties.driverVersion
              << "\n";
    // You can access more properties as needed

    std::cout << "(Info): Selected Device Features:\n";
    std::cout << "  - Geometry Shader Support: "
              << deviceFeatures.geometryShader << "\n";
    std::cout << "  - Tessellation Shader Support: "
              << deviceFeatures.tessellationShader << "\n";
    // You can access more features as needed
  } else {
    std::cerr << "Error. No suitable GPU with graphics queue family found!\n";
    // Handle this case gracefully
  }

  // the swapchain creation was freezing the program and i found out that i have
  // to enable the swapchain extension on logical device check available device

  // extensions on my machine
  uint32_t enumerateDeviceExtensionCount = 0;
  vkEnumerateDeviceExtensionProperties(selectedDevice, nullptr,
                                       &enumerateDeviceExtensionCount, nullptr);
  std::vector<VkExtensionProperties> enumerateDeviceExtensions(
      enumerateDeviceExtensionCount);
  vkEnumerateDeviceExtensionProperties(selectedDevice, nullptr,
                                       &enumerateDeviceExtensionCount,
                                       enumerateDeviceExtensions.data());

  std::cout << "(Info): Device Extensions:\n";
  for (const auto &extension : enumerateDeviceExtensions) {
    // The list clutters terminal because it's too big. Verify the extension
    // existence.
    if (std::string(extension.extensionName) == "VK_KHR_swapchain") {
      std::cout << "  - (info): Found extension on device\n";
      std::cout << "    - " << extension.extensionName << "\n";
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Logical Device and Queue Creation:
  //
  // Create a logical device, representing the chosen physical device and
  // queue families. Retrieve handles to the queues from the logical device.

  // Define the queue family indices you need (e.g., graphicsQueueFamilyIndex).

  if (suitableDeviceFound) {
    std::cout
        << "Success. Physical device is found, graphicsQueueFamilyIndex is "
           "hopefully set correctly.\n";
  } else {
    std::cerr << "Error. Missing graphicsQueueFamilyIndex!\n";
  }

  VkDevice logicalDevice;
  VkQueue graphicsQueue; // Handle to the graphics queue

  // Specify the queue creation information.
  const float queuePriority =
      1.0f; // Priority for the queue (typically 0.0f to 1.0f)

  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex =
      graphicsQueueFamilyIndex; // Use the graphics queue family index you found
                                // earlier
  queueCreateInfo.queueCount = 1; // You can create multiple queues if needed
  queueCreateInfo.pQueuePriorities = &queuePriority;

  // Specify device features and extensions as needed.
  VkPhysicalDeviceFeatures deviceFeatures =
      {}; // You can enable specific features here.

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME, // swapchain creation was failing without
                                       // this extension
  };
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

  // Create the logical device.
  if (VkResult result = vkCreateDevice(selectedDevice, &deviceCreateInfo,
                                       nullptr, &logicalDevice);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to create logical device!"
              << string_VkResult(result) << "\n";
    // Handle this case gracefully
  } else {
    std::cout << "Success. Created logical device.\n";
  }

  // Retrieve handles to the queues from the logical device.
  vkGetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex, 0, &graphicsQueue);

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Window Surface Creation:
  //
  //   Set up a window using a windowing system/library of your choice.
  //   Create a Vulkan surface for the window, to render the graphics onto.

  // Create a GLFW window
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);
  if (!window) {
    std::cerr << "Error. Failed to create GLFW window!\n";
    glfwTerminate();
  } else {
    std::cout << "Success. Created GLFW window\n";
  }

  VkSurfaceKHR surface;
  if (VkResult result =
          glfwCreateWindowSurface(instance, window, nullptr, &surface);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to create Vulkan surface! Reason:\n"
              << string_VkResult(result) << "\n";
  } else {
    std::cout << "Success. Created vulkan surface succesfully\n";
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Swapchain Creation:
  //
  //   Create a swapchain, which handles the rendering output to the window
  //   surface.

  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;

  // BEGIN -- Querying the Swap Chain Support
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selectedDevice, surface,
                                            &capabilities);
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(selectedDevice, surface, &formatCount,
                                       nullptr);
  if (formatCount != 0) {
    std::cout << "(Info): Resizing formats to: " << formatCount << "\n";
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(selectedDevice, surface, &formatCount,
                                         formats.data());
  } else {
    std::cerr << "Error. Skipping format resizing\n";
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(selectedDevice, surface,
                                            &presentModeCount, nullptr);
  if (presentModeCount != 0) {
    std::cout << "Resizing presentModeCount to: " << presentModeCount << "\n";
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        selectedDevice, surface, &presentModeCount, presentModes.data());
  } else {
    std::cerr << "Skipping presentModeCount resizing\n";
  }
  // END -- Querying the Swap Chain Support Details

  // BEGIN -- Creating the Swap Chain

  VkSurfaceFormatKHR surfaceFormat = formats[0]; // Choose a format
  VkPresentModeKHR presentMode =
      VK_PRESENT_MODE_FIFO_KHR; // Choose a present mode

  int windowWidth, windowHeight;
  glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

  VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
  swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCreateInfo.surface = surface;
  swapchainCreateInfo.minImageCount = capabilities.minImageCount + 1;
  swapchainCreateInfo.imageFormat = surfaceFormat.format;
  swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapchainCreateInfo.imageExtent = {static_cast<uint32_t>(windowWidth),
                                     static_cast<uint32_t>(windowHeight)};
  swapchainCreateInfo.imageArrayLayers = 1;
  swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCreateInfo.preTransform = capabilities.currentTransform;
  swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainCreateInfo.presentMode = presentMode;
  swapchainCreateInfo.clipped = VK_TRUE;
  swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  VkSwapchainKHR swapchain;
  if (VkResult result = vkCreateSwapchainKHR(
          logicalDevice, &swapchainCreateInfo, nullptr, &swapchain);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to create Vulkan Swapchain: "
              << string_VkResult(result) << "\n";
  } else {
    std::cout << "Success. Vulkan Swapchain successfully\n";
  }

  // END -- Creating the Swap Chain

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Resource Creation:
  //
  //   Load or create the necessary resources like textures, shaders, and
  //   buffers.

  // Shader Creation:
  //   a. Write a vertex shader in GLSL or HLSL that specifies the position of
  //   each vertex. b. Write a fragment shader in GLSL or HLSL that specifies
  //   the color of each fragment. c. Compile these shaders into SPIR-V format.
  //   d. Load these SPIR-V bytecode files into your program.
  //   e. Create Vulkan shader module objects for each shader.

  // setup vulkan memory allocator library ...

  // Buffer Creation: (USING VULKAN MEMORY ALLOCATOR LIBRARY)
  //   a. Define the data for your triangle's vertices, including positions and
  //   colors. b. Create a Vulkan buffer to hold this data. c. Allocate memory
  //   for this buffer, and copy your vertex data into this buffer.

  // load shaders using relative path from the executable location because
  // that's the way the build system sets up the shader loading
  //
  // the problem is that relative paths use location from which the process
  // was executed while i expected the app to use the path from the executable
  // location
  std::filesystem::path executablePath(argv[0]);
  std::filesystem::path executableDirPath = executablePath.parent_path();

  VkShaderModule shaderVertexModule;
  try {
    std::string vertexShaderPath =
        (executableDirPath / "shader.vert.spv").string();

    std::cout << "(Info): Loading: " << vertexShaderPath << "\n";
    std::vector<uint32_t> shaderVertex = loadSPIRV(vertexShaderPath);

    shaderVertexModule = createShaderModule(logicalDevice, shaderVertex);

    std::cout << "Success. Created vertex shader.\n";
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
  }

  VkShaderModule shaderFragmentModule;
  try {
    std::string fragmentShaderPath =
        (executableDirPath / "shader.frag.spv").string();

    std::cout << "(Info): Loading: " << fragmentShaderPath << "\n";
    std::vector<uint32_t> fragmentShader = loadSPIRV(fragmentShaderPath);

    shaderFragmentModule = createShaderModule(logicalDevice, fragmentShader);

    std::cout << "Success. Created fragment shader\n";
  } catch (const std::exception &e) {
    std::cerr << e.what();
  }

  // Setup vulkan memory allocator before buffer creation because it is easy to
  // mess up vulkan memory allocation according to their documentation
  // https://vkguide.dev/docs/introduction/project_libs/
  VmaAllocatorCreateInfo vmaAllocatorCreateInfo{};
  vmaAllocatorCreateInfo.vulkanApiVersion = vulkanVersion;
  vmaAllocatorCreateInfo.physicalDevice = selectedDevice;
  vmaAllocatorCreateInfo.device = logicalDevice;
  vmaAllocatorCreateInfo.instance = instance;

  VmaAllocator allocator;
  if (VkResult result = vmaCreateAllocator(&vmaAllocatorCreateInfo, &allocator);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to create Vulkan Memory Allocator: "
              << string_VkResult(result) << "\n";
  } else {
    std::cout << "Success. Created Vulkan Memory Allocator.\n";
  }

  // Create vertex data
  const float vertexData[] = {
      0.0f,  1.0f,  0.0f, //
      -1.0f, -1.0f, 0.0f, //
      1.0f,  -1.0f, 0.0f, //
  };
  VkBufferCreateInfo bufferCreateInfo{};
  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.size = sizeof(vertexData);
  bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo vertexBufferAllocationCreateInfo{};
  vertexBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  VkBuffer vertexBuffer;
  VmaAllocation vertexBufferAllocation;

  if (VkResult result = vmaCreateBuffer(
          allocator, &bufferCreateInfo, &vertexBufferAllocationCreateInfo,
          &vertexBuffer, &vertexBufferAllocation, nullptr);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to create vertex buffer: "
              << string_VkResult(result) << "\n";
  } else {
    std::cout << "Success. Created vertex buffer.\n";
  }

  void *mapVertexData = nullptr;
  // You call vmaMapMemory with the pointer variable. Vulkan maps the GPU
  // buffer's memory into your application's address space, and data is now a
  // pointer to that mapped memory.
  if (VkResult result =
          vmaMapMemory(allocator, vertexBufferAllocation, &mapVertexData);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to map memory: " << string_VkResult(result)
              << "\n";
  } else {
    std::cout << "Success. Memory mapped.\n";
  }
  // You use std::copy to copy the vertex data from vertexData to the GPU buffer
  // via the data pointer. The data is now in the GPU memory, not in the data
  // pointer.
  std::copy(std::begin(vertexData), std::end(vertexData),
            static_cast<float *>(mapVertexData));
  // You call vmaUnmapMemory to unmap the GPU buffer's memory from your
  // application's address space. The data pointer is now invalid and should not
  // be used to access the memory.
  vmaUnmapMemory(allocator, vertexBufferAllocation);

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Pipeline Creation:
  //
  //   Create the graphics and/or compute pipelines.
  //   Set up the pipeline states, including shader stages, vertex input, etc.

  // Shader Stages Setup:
  //   Vertex Shader: A simple shader that passes through vertex positions.
  //   Fragment Shader: A simple shader that outputs a constant color.

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);

  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = shaderVertexModule;
  shaderStages[0].pName = "main";

  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = shaderFragmentModule;
  shaderStages[1].pName = "main";

  // Pipeline Layout Creation:
  //   Create a pipeline layout with no descriptor set layouts or push
  //   constants as you don't have any external resources to pass to the
  //   shaders.

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
  pipelineLayoutCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.setLayoutCount = 0;
  pipelineLayoutCreateInfo.pSetLayouts = nullptr;
  pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

  VkPipelineLayout pipelineLayout;
  if (VkResult result = vkCreatePipelineLayout(
          logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
      result != VK_SUCCESS) {
    std::cerr << "Error. Failed to create Pipeline Layout: "
              << string_VkResult(result) << "\n";
  } else {
    std::cout << "Success. Created Pipeline Layout.\n";
  }

  // Vertex Input Configuration:
  //     Set up the vertex input to pass vertices to the vertex shader. If your
  //     vertex data is hardcoded in the shader, this can be left empty.

  std::vector<VkVertexInputAttributeDescription>
      vertexInputAttributeDescriptions(1);
  vertexInputAttributeDescriptions[0].binding = 0;
  vertexInputAttributeDescriptions[0].location = 0;
  vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexInputAttributeDescriptions[0].offset = 0;

  VkVertexInputBindingDescription vertexInputBindingDescription{};
  vertexInputBindingDescription.binding = 0;
  vertexInputBindingDescription.stride = sizeof(float) * 3;
  vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
  pipelineVertexInputStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
  pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions =
      &vertexInputBindingDescription;
  pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(vertexInputAttributeDescriptions.size());
  pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions =
      vertexInputAttributeDescriptions.data();

  // Input Assembly Configuration:
  //     Configure the input assembly to treat vertices as a list of triangles.

  VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
  pipelineInputAssemblyStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  pipelineInputAssemblyStateCreateInfo.topology =
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

  // Viewport and Scissor Configuration:
  //   Set up a single viewport and scissor rect covering the entire
  //   framebuffer.

  int framebufferWidth, framebufferHeight;
  glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(framebufferWidth);
  viewport.height = static_cast<float>(framebufferHeight);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = {static_cast<uint32_t>(framebufferWidth),
                    static_cast<uint32_t>(framebufferHeight)};

  VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
  pipelineViewportStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  pipelineViewportStateCreateInfo.viewportCount = 1;
  pipelineViewportStateCreateInfo.pViewports = &viewport;
  pipelineViewportStateCreateInfo.scissorCount = 1;
  pipelineViewportStateCreateInfo.pScissors = &scissor;

  // Rasterizer Configuration:
  //   Configure the rasterizer to fill triangles and cull backfaces if
  //   desired.

  VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
  pipelineRasterizationStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
  pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
  pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
  pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  pipelineRasterizationStateCreateInfo.frontFace =
      VK_FRONT_FACE_COUNTER_CLOCKWISE;
  pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;

  // Multisampling Configuration:
  //   Disable multisampling if not needed.

  VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
  pipelineMultisampleStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  pipelineMultisampleStateCreateInfo.rasterizationSamples =
      VK_SAMPLE_COUNT_1_BIT;
  pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
  pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f;
  pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
  pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
  pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

  // Depth and Stencil Configuration:
  //   Disable depth and stencil testing if not needed.

  VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
  pipelineDepthStencilStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
  pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
  pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;

  // Color Blending Configuration:
  //   Set up simple alpha blending or disable blending if not needed.

  VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
  pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
  pipelineColorBlendAttachmentState.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
  pipelineColorBlendStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
  pipelineColorBlendStateCreateInfo.attachmentCount = 1;
  pipelineColorBlendStateCreateInfo.pAttachments =
      &pipelineColorBlendAttachmentState;

  // Render Pass Association:
  //     Associate the pipeline with a render pass that has a compatible color
  //     attachment format.

  // Create or Obtain a Render Pass:
  //   Before associating a pipeline with a render pass, you'll need to have a
  //   render pass created or obtained. This render pass should have the desired
  //   color, depth, and stencil attachment configurations you plan to use for
  //   rendering.

  // Creating or obtaining a render pass in Vulkan is a multi-step process that
  // involves understanding the rendering operations you wish to perform and how
  // they are to be organized within a render pass. Here's a breakdown of the
  // conceptual tasks involved:

  //   Understand Your Rendering Requirements:
  //       Determine the color, depth, and stencil attachments you will need
  //       based on what you're rendering. Decide on the format of these
  //       attachments

  //   Determine Attachment Descriptions:
  //       Create a VkAttachmentDescription structure for each attachment.
  //       Specify the format, usage, and other properties of each attachment.

  //   Determine Subpass Descriptions:
  //       Create a VkSubpassDescription structure.
  //       Specify the attachments that will be used in this subpass and how
  //       they will be used (e.g., as color attachments, depth-stencil
  //       attachments, etc.).

  //   Determine Subpass Dependencies:
  //       If you have multiple subpasses, define the dependencies between them
  //       using VkSubpassDependency structures. Specify when each subpass
  //       should execute relative to others.

  //   Create Render Pass Create Info:
  //       Create a VkRenderPassCreateInfo structure.
  //       Populate it with the attachment descriptions, subpass descriptions,
  //       and subpass dependencies.

  //   Create the Render Pass:
  //       Call vkCreateRenderPass, passing in the VkRenderPassCreateInfo
  //       structure, to create the render pass.

  //   Handle the Render Pass:
  //       Store the handle to the render pass (VkRenderPass) returned by
  //       vkCreateRenderPass for later use.

  // Not sure about this part... I'm supposed to assemble renderpass however i'm
  // boggled down with intricacies of its settings. Maybe i should not be and
  // should simply get the code to setup render pass and debug as issues arise.

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = surfaceFormat.format; // Use swapchain image format
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  return 0;
}
