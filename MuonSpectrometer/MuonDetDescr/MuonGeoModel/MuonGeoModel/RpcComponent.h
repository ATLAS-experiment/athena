/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RpcComponent_H
#define RpcComponent_H

#include "MuonGeoModel/StandardComponent.h"

namespace MuonGM {

    class RpcComponent : public StandardComponent {
      public:
        int ndivy{0};
        int ndivz{0};
    };
} // namespace MuonGM

#endif
