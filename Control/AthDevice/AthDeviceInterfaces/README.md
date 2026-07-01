# General Device / GPU Handling Interfaces

This package collects abstract interfaces that provide various functionalities
for interacting with accelerator devices. (Mostly GPUs.) For functionality that
can be expressed in a technology independent way.

## Interfaces Provided

  - Memory resource interfaces: These provide access to an appropriately
    configured
    [std::pmr::memory_resource](https://en.cppreference.com/cpp/memory/memory_resource)
    object. Which allows clients to manage "device memory" in an abstract way.
    * [AthDevice::IMemoryResourceProvider](AthDeviceInterfaces/IMemoryResourceProvider.h)
      is an interface to either a service or a tool. Possibly allowing some
      clients to be agnostic to whether they get set up with a service or a
      tool.
    * [AthDevice::IMemoryResourceTool](AthDeviceInterfaces/IMemoryResourceTool.h)
      is specifically a tool interface, providing the same functionality.
    * [AthDevice::IMemoryResourceSvc](AthDeviceInterfaces/IMemoryResourceSvc.h)
      is specifically a service interface, providing the same functionality.

  - "Copy object" interfaces: These provide access to an appropriately
    configured
    [vecmem::copy](https://acts-project.github.io/vecmem/classvecmem_1_1copy.html)
    object. Which would allow clients to manage memory copies between the host
    and (a) device(s) in an abstract way.
    * [AthDevice::ICopyProvider](AthDeviceInterfaces/ICopyProvider.h)
      is an interface to either a service or a tool. Possibly allowing some
      clients to be agnostic to whether they get set up with a service or a
      tool.
    * [AthDevice::ICopyTool](AthDeviceInterfaces/ICopyTool.h)
      is specifically a tool interface, providing the same functionality.
    * [AthDevice::ICopySvc](AthDeviceInterfaces/ICopySvc.h)
      is specifically a service interface, providing the same functionality.
