#include "explore.h"
#include "vulkan/vk_enum_string_helper.h" // is there alternative helper?
#include <cstring>
#include <iostream>
#include <new>
#include <stdlib.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace {

struct UserData {
  int i;
};

void *VKAPI_PTR pfnAllocation(void *pUserData, size_t size, size_t alignment,
                              VkSystemAllocationScope allocationScope) {
  std::cout << "(pfnAllocation):\n";

  // void *pMemory{::operator new(size)}; // Never gets called.

  return nullptr;
}

void *VKAPI_PTR pfnReallocation(void *pUserData, void *pOriginal, size_t size,
                                size_t alignment,
                                VkSystemAllocationScope allocationScope) {
  std::cout << "(pfnReallocation):\n";
  return nullptr;
}

void VKAPI_PTR pfnFree(void *pUserData, void *pMemory) {
  std::cout << "(pfnFree):\n"
            << "  UserData->i = " << static_cast<UserData *>(pUserData)->i
            << "\n"
            << "  pMemory = " << (pMemory == nullptr ? "nullptr" : "not null")
            << "\n";

  // My assumption is that since the pfnAllocation was never called, the memory
  // was allocated with different allocator and calling ::operator delete on it
  // crashes the program. I did not confirm that. However it makes sense. The
  // instance was created without custom allocator. It is difficult to get this
  // right.
  //
  // From all the calls I logged, pMemory was never null.
  // ::operator delete(pMemory); // Fails even though it's matched.
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// So do you mean to say it just generically say that it need this and this
// amount of memory?
//
// When the Vulkan implementation needs memory for its internal operations or
// data structures, it will request a specific amount of memory by invoking the
// pfnAllocation callback (or pfnReallocation if resizing an existing
// allocation). It doesn't specify what the memory is specifically for; it just
// says, in essence, "I need this much memory, please allocate it and give me a
// pointer to it."
//
//
// The VkSystemAllocationScope enumeration specifies the scope of the
// allocation, which can give you a hint about the purpose or lifespan of the
// memory being requested. It can be useful for fine-tuning memory allocation
// strategies, especially if you're working with custom memory allocators or
// pool-based allocation systems.
//
// Here are the possible values for VkSystemAllocationScope:
//
//     VK_SYSTEM_ALLOCATION_SCOPE_COMMAND: Indicates an allocation for a command
//     buffer.
//
//     VK_SYSTEM_ALLOCATION_SCOPE_OBJECT: Used for allocations of Vulkan objects
//     like VkBuffer, VkImage, etc.
//
//     VK_SYSTEM_ALLOCATION_SCOPE_CACHE: Indicates allocation for a cache.
//
//     VK_SYSTEM_ALLOCATION_SCOPE_DEVICE: Indicates allocations related to the
//     lifespan of the Vulkan device.
//
//     VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE: Indicates allocations related to the
//     lifespan of a Vulkan instance.
//
//     VK_SYSTEM_ALLOCATION_SCOPE_USER: This is an indication for user
//     allocations, though in practice it's rarely used.

// The idea behind providing this scope is to potentially help developers who
// might want to segregate their memory allocations based on these categories.
// For instance:

//     For short-lived allocations (like those associated with a command), you
//     might allocate memory from a fast, temporary memory pool. For
//     longer-lived allocations (like those associated with the lifespan of a
//     device or instance), you might choose to allocate from a more persistent
//     memory region.
//
// In many applications, particularly simpler ones, you might not differentiate
// between these scopes and allocate memory from the same pool or allocator
// regardless of the scope. However, for more complex applications or engines,
// or on platforms with specific memory constraints, being able to differentiate
// based on this scope might prove useful.
//
// If you're not interested in using this for a custom allocation strategy, you
// can simply ignore the allocationScope parameter.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ALIGNMENT ISSUES WHEN ALLOCATING AND DEALLOCATING MEMORY.
//
// You are right, and this is a quirk in C++'s aligned memory allocation system.
// When you use ::operator new(size_t size, std::align_val_t alignment) to
// allocate memory, and later call the corresponding ::operator delete(void*
// pMemory, std::align_val_t alignment) to deallocate that memory, there is no
// in-built mechanism in C++ to automatically infer the alignment from the
// allocated pointer. It is your responsibility to ensure that the correct
// alignment is passed to the delete operator.
//
// The traditional methods like malloc and free don't face this issue because
// they don't support alignment directly. For aligned allocations in C-style,
// you'd use functions like aligned_alloc and then free it, which does not
// require knowledge of the alignment during deallocation.
//
// But, in C++ with overloads for ::operator new and ::operator delete, the onus
// is on you to ensure correct pairing. If you don't know the alignment during
// deallocation, this becomes problematic.
//
// For such cases, you have a few solutions:
//
//     Store the Alignment with the Pointer: One approach can be to store the
//     alignment alongside the memory. This would mean custom structures to
//     handle memory or slightly over-allocating to keep the alignment as
//     metadata.
//
//     Custom Allocator: Another approach is to use a custom allocator where you
//     encapsulate this detail. Many graphics or game engines opt for custom
//     memory allocators for finer control over memory operations.
//
//     Consistent Alignment: If your allocations are always using the same
//     alignment (or a few known ones), you can always use the same aligned
//     delete.
//
//     Fallback to C-Style Allocation: As mentioned, if you use aligned_alloc
//     and then free, you won't need to pass alignment during deallocation.
//
// Remember that not passing the correct alignment to the delete operator that
// expects it is undefined behavior. In practice, this can manifest as memory
// leaks, crashes, or other unpredictable outcomes.
//
// In your specific use-case with Vulkan, which often demands specific memory
// alignments for buffers and other resources, it's essential to be meticulous
// about memory management. If the alignment requirement becomes burdensome, it
// might be worthwhile to consider a custom allocator or another solution that
// abstracts these details away safely.
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void *VKAPI_PTR pfnAllocationCreate(void *pUserData, size_t size,
                                    size_t alignment,
                                    VkSystemAllocationScope allocationScope) {
  std::cout << "(pfnAllocationCreate):\n"
            << "  size = " << size << "\n"
            << "  alignment = " << alignment << "\n";

  // void *pMemory{std::malloc(size)};
  // Using alignment makes the program crash when I try to free the memory using
  // ::operator delete(pMemory)
  //
  // It appears that memory alignment is fairly complex problem to first
  // understand, then solve.
  // https://stackoverflow.com/questions/227897/how-to-allocate-aligned-memory-only-using-the-standard-library
  void *pMemory = ::operator new(size); // works

  std::cout << "  pMemory = " << (pMemory == nullptr ? "nullptr" : "not null")
            << "\n"
            << "  sizeof(pMemory) = " << sizeof(pMemory) << "\n";

  return pMemory;
}

void *VKAPI_PTR pfnReallocationCreate(void *pUserData, void *pOriginal,
                                      size_t size, size_t alignment,
                                      VkSystemAllocationScope allocationScope) {
  std::cout << "(pfnReallocationCreate):\n";
  return nullptr;
}

void VKAPI_PTR pfnFreeCreate(void *pUserData, void *pMemory) {
  std::cout << "(pfnFreeCreate):\n"
            << "  pMemory = " << (pMemory == nullptr ? "nullptr" : "not null")
            << "\n";

  // Using ::operator delete(pMemory) crashed the program even when I matched it
  // with ::operator new(size, std::align_val_t(alignment))
  //
  // For some reason deleting memory that had alignment without specifying the
  // alignment when deleting that memory caused the program to crash. I feel
  // like I don't understand the problem completely.
  //
  // std::free(pMemory);
  ::operator delete(pMemory); // works
}

} // namespace

void explore::vulkan_api() {
  // These logs are terribly noisy and I don't want to comment/uncomment them
  // every time I want to rerun this example. Therefore I will guard it with a
  // boolean flag which I can toggle on and off.

  if (const bool LOG_EXAMPLE_VULKAN_API{false}; LOG_EXAMPLE_VULKAN_API) {
    const std::string EXPLORE_VULKAN_API_BEGIN{R"(
~~~~~~~~~~~~~~~~~~~~~~~~
EXPLORE_VULKAN_API_BEGIN
~~~~~~~~~~~~~~~~~~~~~~~~
)"};

    const std::string EXPLORE_VULKAN_API_END{R"(
~~~~~~~~~~~~~~~~~~~~~~~~
EXPLORE_VULKAN_API_END
~~~~~~~~~~~~~~~~~~~~~~~~
)"};

    std::cout << EXPLORE_VULKAN_API_BEGIN;

    // The Vulkan's type of struct that defines the creation information for
    // vkCreateInstance. How does the vkCreateInstance use those?
    //
    // typedef struct VkInstanceCreateInfo {
    //     VkStructureType             sType;
    //     const void*                 pNext;
    //     VkInstanceCreateFlags       flags;
    //     const VkApplicationInfo*    pApplicationInfo;
    //     uint32_t                    enabledLayerCount;
    //     const char* const*          ppEnabledLayerNames;
    //     uint32_t                    enabledExtensionCount;
    //     const char* const*          ppEnabledExtensionNames;
    // } VkInstanceCreateInfo;

    VkInstanceCreateInfo pCreateInfo{};
    // This field specifies the type of the structure and is always set to
    // VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO. It's used by Vulkan to identify
    // the structure type and the version of Vulkan you are using.
    pCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    // This is a pointer to an extension-specific structure or NULL. It's a way
    // to extend the Vulkan API for future or vendor-specific functionality.
    // Typically, you'll set this to NULL unless you are using certain
    // extensions that require additional data.
    pCreateInfo.pNext = nullptr;
    // This field is reserved for future use in the Vulkan API and should be set
    // to 0. It's a placeholder for flags that might be added in future versions
    // of Vulkan.
    pCreateInfo.flags = 0;
    // This is a pointer to a VkApplicationInfo structure, which contains
    // information about your application like its name, version, and the
    // version of Vulkan it's using. This is optional but can provide useful
    // information to the driver.
    pCreateInfo.pApplicationInfo = nullptr;
    // These fields specify the number of global layers to enable and an array
    // of layer names, respectively. Layers are used to insert additional
    // functionality such as validation, debugging, and profiling. If you don’t
    // want to enable any layers, set enabledLayerCount to 0.
    pCreateInfo.enabledLayerCount = 0;
    pCreateInfo.ppEnabledLayerNames = nullptr;
    // These fields specify the number of global extensions to enable and an
    // array of extension names, respectively. Extensions are used to provide
    // additional functionality beyond what the core Vulkan specification
    // provides. If you don’t want to enable any extensions, set
    // enabledExtensionCount to 0.
    pCreateInfo.enabledExtensionCount = 0;
    pCreateInfo.ppEnabledExtensionNames = nullptr;

    VkInstance pNoAllocatorInstance{VK_NULL_HANDLE};

    // Succeeds to create vulkan instance without any information on
    // pCreateInfo. Why? I thought that it requires at least some information.

    // I love the technique vulkan uses. A structure that configures the
    // creation and more additional parameters. I haven't noticed so far such a
    // technique in JavaScript world. It is absolutely possible to use it,
    // however I haven't seen it or maybe i haven't noticed or wasn't aware of
    // it.

    if (VkResult result{vkCreateInstance(

            &pCreateInfo,
            // Allocator Callbacks:
            //
            // The allocator callbacks parameter allows you to provide custom
            // memory allocation routines for Vulkan to use when allocating
            // memory for the Vulkan instance. This is an advanced feature that
            // most applications don't need. If you're not interested in custom
            // memory allocation, you can safely pass nullptr for this
            // parameter, and Vulkan will use its default memory allocation
            // behavior. Custom allocators can be useful in scenarios where you
            // have specific memory allocation strategies, requirements or
            // constraints, such as in embedded systems or game consoles.
            nullptr,

            &pNoAllocatorInstance

            )};

        result == VK_SUCCESS) {
      std::cout
          << "(Success): Created NO_ALLOCATOR_CALLBACK Vulkan instance.\n";
    } else {
      std::cerr
          << "(Error): Failed to create NO_ALLOCATOR_CALLBACK Vulkan instance: "
          << string_VkResult(result) << "\n";
    }

    VkAllocationCallbacks pAllocatorCreate{};
    pAllocatorCreate.pfnAllocation = pfnAllocationCreate;
    pAllocatorCreate.pfnReallocation = pfnReallocationCreate;
    pAllocatorCreate.pfnFree = pfnFreeCreate;

    VkInstance pAllocatorInstance{VK_NULL_HANDLE};
    if (VkResult result{vkCreateInstance(&pCreateInfo,

                                         // When do these get called, if ever?
                                         &pAllocatorCreate,

                                         &pAllocatorInstance)};
        result == VK_SUCCESS) {
      std::cout << "(Success): Created ALLOCATOR_CALLBACK Vulkan instance.\n";
    } else {
      std::cerr
          << "(Error): Failed to create ALLOCATOR_CALLBACK Vulkan instance: "
          << string_VkResult(result) << "\n";
    }

    // There are many vkCreate<Something> functions
    // There are many vkDestroy<Something> functions
    // There are many vkAcquire<Something> functions
    // How can I use that knowledge?

    // Imagine that we don't know the lifetime of user data.
    UserData *pUserData = new UserData{20};

    // When are these called?
    VkAllocationCallbacks pAllocator{};
    pAllocator.pUserData = pUserData;
    pAllocator.pfnAllocation = pfnAllocation;
    pAllocator.pfnReallocation = pfnReallocation;
    pAllocator.pfnFree = pfnFree;

    // Without vkDestroyInstance, the pfnFree is not called.
    // With vkDestroyInstance, the pfnFree is called many times, maybe 10, 15,
    // 20 or more?
    vkDestroyInstance(pNoAllocatorInstance, &pAllocator);
    delete pUserData;

    std::cout << EXPLORE_VULKAN_API_END;
  }
}

void explore::physical_device_enumeration() {
  if (const bool LOG_EXAMPLE_PHYSICAL_DEVICE_ENUMERATION{false};
      LOG_EXAMPLE_PHYSICAL_DEVICE_ENUMERATION) {
    const std::string EXPLORE_PHYSICAL_DEVICE_ENUMERATION_BEGIN{R"(
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EXPLORE_PHYSICAL_DEVICE_ENUMERATION_BEGIN
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
)"};

    const std::string EXPLORE_PHYSICAL_DEVICE_ENUMERATION_END{R"(
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EXPLORE_PHYSICAL_DEVICE_ENUMERATION_END
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
)"};
    std::cout << EXPLORE_PHYSICAL_DEVICE_ENUMERATION_BEGIN;

    std::vector<const char *> layerNames;
    std::vector<const char *> extensionNames;

    uint32_t instanceLayerPropertiesCount{0};
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount, nullptr);
    std::vector<VkLayerProperties> instanceLayerProperties(
        instanceLayerPropertiesCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount,
                                       instanceLayerProperties.data());

    std::cout << "(info): Instance Layer Properties:\n\n";
    for (const VkLayerProperties &layerProperties : instanceLayerProperties) {
      std::cout << "  - layerProperties.layerName = "
                << layerProperties.layerName << "\n"
                << "  - layerProperties.description = "
                << layerProperties.description << "\n"
                << "  - layerProperties.specVersion = "
                << layerProperties.specVersion << "\n"
                << "  - layerProperties.implementationVersion = "
                << layerProperties.implementationVersion << "\n\n";

      uint32_t layerExtensionPropertiesCount{0};
      vkEnumerateInstanceExtensionProperties(
          layerProperties.layerName, &layerExtensionPropertiesCount, nullptr);
      std::vector<VkExtensionProperties> layerExtensionProperties(
          layerExtensionPropertiesCount);
      vkEnumerateInstanceExtensionProperties(layerProperties.layerName,
                                             &layerExtensionPropertiesCount,
                                             layerExtensionProperties.data());

      std::cout << "  (info): " << layerProperties.layerName
                << " Extension Properties:\n\n";
      if (layerExtensionPropertiesCount > 0) {
        for (const VkExtensionProperties &extensionProperties :
             layerExtensionProperties) {
          std::cout << "    - extensionProperties.extensionName = "
                    << extensionProperties.extensionName << "\n"
                    << "    - extensionProperties.specVersion = "
                    << extensionProperties.specVersion << "\n\n";
        }
      } else {
        std::cout << "      No layer extension properties.\n\n";
      }

      if (std::strcmp(layerProperties.layerName,
                      "VK_LAYER_KHRONOS_validation") == 0) {
        layerNames.push_back(layerProperties.layerName);
      }
    }

    std::cout << "(info): Enabled Instance Layers:\n\n";
    for (const char *layerName : layerNames) {
      std::cout << "  - layerName = " << layerName << "\n";
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
    createInfo.ppEnabledLayerNames = layerNames.data();
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(extensionNames.size());
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    // TODO: Dig deep to select the graphics card.
    // TODO: Dig deep to use the handle to the graphics device.

    std::cout << EXPLORE_PHYSICAL_DEVICE_ENUMERATION_END;
  }
}
