/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MdtComponent_H
#define MdtComponent_H

#include "MuonGeoModel/StandardComponent.h"

namespace MuonGM {

    class MdtComponent : public StandardComponent {
      public:
        MdtComponent() = default;
      
        double tubelenStepSize{0.};  // step size for tube length in endcap chambers
        double cutoutTubeXShift{0.}; // distance along tube length to shift tube center

    };
} // namespace MuonGM

#endif
