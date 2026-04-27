/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHCUDASERVICES_DEVICEINFO_H
#define ATHCUDASERVICES_DEVICEINFO_H

#include "GaudiKernel/IService.h"

#include <vector>

namespace AthCUDA {

    struct DeviceInfo {
        int id{};
        std::string name;
        int smMajor{};
        int smMinor{};
    };

} // namespace AthCUDA

#endif // ATHCUDASERVICES_DEVICEINFO_H