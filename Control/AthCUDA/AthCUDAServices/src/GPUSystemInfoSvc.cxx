/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaInterprocess/Incidents.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "GPUSystemInfoSvc.h"
#include <cuda_runtime.h>

namespace AthCUDA {

    StatusCode GPUSystemInfoSvc::initialize() {
        const bool isMultiprocess = (Gaudi::Concurrency::ConcurrencyFlags::numProcs() > 0);
        if (isMultiprocess) {
            SmartIF<IIncidentSvc> incsvc{service("IncidentSvc")};
            incsvc->addListener( this, AthenaInterprocess::UpdateAfterFork::type(), 1000); // high priority- before other GPU functions

            ATH_MSG_DEBUG("In multiprocess mode the CUDA context will be only checked after fork");
            return StatusCode::SUCCESS;
        }
        else {
            readAvailableDevices();
            
            ATH_MSG_DEBUG("GPU availability check in initialize " << std::boolalpha << (m_deviceInfo.size() > 0));
            return StatusCode::SUCCESS;
        }        
    }

    void GPUSystemInfoSvc::handle(const Incident&) {  
        readAvailableDevices();
        ATH_MSG_DEBUG("GPU availability check in UpdateAfterFork " << std::boolalpha << (m_deviceInfo.size() > 0));
    }

    const std::vector<DeviceInfo>& GPUSystemInfoSvc::getAvailableDevices() const {
        if (m_wasChecked == false) {
            ATH_MSG_WARNING("Function was called before service initialization!");
        }
        return m_deviceInfo;
    }

    void GPUSystemInfoSvc::readAvailableDevices() {
        std::call_once(m_readDevicesOnceFlag, [this]() {
            std::string cudaErrorStr;

            int deviceCount = 0;
            cudaError_t error = cudaGetDeviceCount(&deviceCount);
            if (error != cudaSuccess) {
                cudaErrorStr = cudaGetErrorString(error);
                ATH_MSG_DEBUG("Error in cudaGetDeviceCount  " << cudaErrorStr);
            }

            for (int i = 0; i < deviceCount; i++) {
                cudaDeviceProp prop;
                error = cudaGetDeviceProperties(&prop, i);

                if (error != cudaSuccess) {
                    cudaErrorStr = cudaGetErrorString(error);
                    ATH_MSG_DEBUG("Error in cudaGetDeviceProperties for device " << i << " " << cudaErrorStr);
                    continue;
                }
                m_deviceInfo.emplace_back(i, prop.name, prop.major, prop.minor);
            }


            if (m_deviceInfo.empty()) {
                ATH_MSG_DEBUG("No CUDA Devices not available");
                return;
            }

            m_wasChecked = true;
        });
    }
}