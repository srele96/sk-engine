// https://www.glfw.org/docs/3.3/vulkan_guide.html
// vulkan must be included before glfw
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vk_enum_string_helper.h"
#include <iostream>
#include <vector>

// Callback function to handle messages from the validation layers
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

int main(int argc, char **argv) {
  uint32_t vulkanVersion;

  if (VkResult result = vkEnumerateInstanceVersion(&vulkanVersion);
      result == VK_SUCCESS) {
    std::cout << "Vulkan API version: " << VK_VERSION_MAJOR(vulkanVersion)
              << "." << VK_VERSION_MINOR(vulkanVersion) << "."
              << VK_VERSION_PATCH(vulkanVersion) << "\n";
  } else {
    std::cerr << "Failed to get Vulkan version: " << result << "\n";
  }

  /*

    Vulkan Instance Creation:

      Create a Vulkan instance, which is the connection between your application
      and the Vulkan library. Specify any desired instance-level extensions and
      validation layers.

    Physical Device and Queue Family Selection:

      Enumerate and select a suitable physical device (GPU) from the available
      devices. Identify and select the necessary queue families (e.g., graphics,
      compute, transfer).

    Logical Device and Queue Creation:

      Create a logical device, representing the chosen physical device and
      queue families. Retrieve handles to the queues from the logical device.

    Window Surface Creation:

      Set up a window using a windowing system/library of your choice.
      Create a Vulkan surface for the window, to render the graphics onto.

    Swapchain Creation:

      Create a swapchain, which handles the rendering output to the window
      surface.

    Resource Creation:

      Load or create the necessary resources like textures, shaders, and
      buffers.

    Pipeline Creation:

      Create the graphics and/or compute pipelines.
      Set up the pipeline states, including shader stages, vertex input, etc.

    Rendering Loop:

      Implement the rendering loop, where you record command buffers and
      submit them to the GPU. Handle window resizing, input events, and other
      such runtime concerns.

    Cleanup:

      Correctly release all Vulkan resources and cleanup before exiting the
      application.

  */

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

  std::cout << "InstanceLayerProperties:\n";
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
    std::cerr << "Failed to initialize GLFW\n";
    return EXIT_FAILURE;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // Configure GLFW for Vulkan
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // Query the required instance extensions from GLFW
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::cout << "glfwExtensionCount is: " << glfwExtensionCount
            << "\nglfwExtensions are:\n";
  for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
    std::cout << "  - " << glfwExtensions[i] << "\n";
  }

  // append more extensions
  std::vector<const char *> allExtensions(glfwExtensions,
                                          glfwExtensions + glfwExtensionCount);
  // layer for debug callback requires debug utils extension to be enabled
  // https://vulkan.lunarg.com/doc/view/1.3.204.1/windows/layer_configuration.html
  allExtensions.push_back("VK_EXT_debug_utils");

  std::cout << "Instance extensions:\n";
  for (const auto &extension : allExtensions) {
    std::cout << "  - " << extension << "\n";
  }

  // extensions are required to use glfw for window creation with vulkan
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
    std::cerr << "No Vulkan-compatible GPUs found!"
              << "\n";
    // Handle this case gracefully
  } else {
    std::cout << "Device count is: " << deviceCount << "\n";
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
    std::cout << "Found suitable GPU device with graphics queue family\n";

    // print the information about the suitable device

    // Get physical device properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(selectedDevice, &deviceProperties);

    // Get physical device features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(selectedDevice, &deviceFeatures);

    std::cout << "Selected Device Properties:\n";
    std::cout << "Device Name: " << deviceProperties.deviceName << "\n";
    std::cout << "Device Type: " << deviceProperties.deviceType << "\n";
    std::cout << "API Version: " << deviceProperties.apiVersion << "\n";
    std::cout << "Driver Version: " << deviceProperties.driverVersion << "\n";
    // You can access more properties as needed

    std::cout << "\nSelected Device Features:\n";
    std::cout << "Geometry Shader Support: " << deviceFeatures.geometryShader
              << "\n";
    std::cout << "Tessellation Shader Support: "
              << deviceFeatures.tessellationShader << "\n";
    // You can access more features as needed
  } else {
    std::cerr << "No suitable GPU with graphics queue family found!"
              << "\n";
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

  std::cout << "Device Extensions:\n";
  for (const auto &extension : enumerateDeviceExtensions) {
    // The list clutters terminal because it's too big. Verify the extension
    // existence.
    if (std::string(extension.extensionName) == "VK_KHR_swapchain") {
      std::cout << "(info): Found extension on device\n";
      std::cout << "  - " << extension.extensionName << "\n";
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
    std::cout << "Physical device is found, graphicsQueueFamilyIndex is "
                 "hopefully set correctly.\n";
  } else {
    std::cerr << "We are missing graphicsQueueFamilyIndex...\n";
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
      result == VK_SUCCESS) {
    std::cout << "Success creating logical device.\n";
  } else {
    std::cerr << "Failed to create logical device!"
              << "\n";
    // Handle this case gracefully
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
    std::cerr << "Failed to create GLFW window!" << std::endl;
    glfwTerminate();
  }

  VkSurfaceKHR surface;
  if (VkResult result =
          glfwCreateWindowSurface(instance, window, nullptr, &surface);
      result != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan surface! Reason:\n"
              << string_VkResult(result) << "\n";
  } else {
    std::cout << "Created vulkan surface succesfully\n";
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
    std::cout << "Resizing formats to: " << formatCount << "\n";
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(selectedDevice, surface, &formatCount,
                                         formats.data());
  } else {
    std::cerr << "Skipping format resizing\n";
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

  std::cout << "Created swapchainCreateInfo\n";

  // something is wrong, call to this function freezes the program...
  VkSwapchainKHR swapchain;
  if (VkResult result = vkCreateSwapchainKHR(
          logicalDevice, &swapchainCreateInfo, nullptr, &swapchain);
      result != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan Swapchain: "
              << string_VkResult(result) << "\n";
  } else {
    std::cout << "Created Vulkan Swapchain successfully\n";
  }

  // END -- Creating the Swap Chain

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Resource Creation:
  //
  //   Load or create the necessary resources like textures, shaders, and
  //   buffers.

  return 0;
}