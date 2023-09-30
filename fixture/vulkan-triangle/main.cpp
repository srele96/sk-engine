// https://www.glfw.org/docs/3.3/vulkan_guide.html
// vulkan must be included before glfw
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vk_enum_string_helper.h"
#include <iostream>
#include <vector>

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

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

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

  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;

  VkInstance instance;

  if (VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
      result != VK_SUCCESS) {
    std::cerr << "Failed to create instance: " << result << "\n";
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
  // Add device extensions as needed in deviceCreateInfo.ppEnabledExtensionNames

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

  return 0;
}