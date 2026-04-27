/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHCUDASERVICES_GPUSYSTEMINFOSVC_H
#define ATHCUDASERVICES_GPUSYSTEMINFOSVC_H

#include "AthCUDAInterfaces/IGPUSystemInfoSvc.h"
#include "AthCUDAInterfaces/DeviceInfo.h"
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/IIncidentListener.h" 
#include <mutex>

namespace AthCUDA {
    // Helper service to perform a check if CUDA device is available. 
    // should happen once per process, before any other initialization of the CUDA context
    // otherwise the check of cudaGetDeviceCount fails
    class GPUSystemInfoSvc : public extends< AthService, IGPUSystemInfoSvc, IIncidentListener > {
        public:
            using extends::extends;

            virtual StatusCode initialize() override;
            virtual void handle(const Incident&) override;

            virtual const std::vector<DeviceInfo>& getAvailableDevices() const override;
        private:
            std::vector<DeviceInfo> m_deviceInfo;
            bool m_wasChecked;
            std::once_flag m_readDevicesOnceFlag;

            void readAvailableDevices();

   }; // class GPUSystemInfoSvc

} // namespace AthCUDA

#endif // ATHCUDASERVICES_GPUSYSTEMINFOSVC_H