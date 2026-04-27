/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHCUDASERVICES_IGPUSYSTEMINFOSVC_H
#define ATHCUDASERVICES_IGPUSYSTEMINFOSVC_H

#include "GaudiKernel/IService.h"
#include "DeviceInfo.h"

#include <vector>

namespace AthCUDA {
    class IGPUSystemInfoSvc : public virtual IService {
        public:
            DeclareInterfaceID( AthCUDA::IGPUSystemInfoSvc, 1, 0 );
            virtual const std::vector<DeviceInfo>& getAvailableDevices() const =0;

   }; // class IGPUSystemInfoSvc

} // namespace AthCUDA

#endif // ATHCUDASERVICES_IGPUSYSTEMINFOSVC_H