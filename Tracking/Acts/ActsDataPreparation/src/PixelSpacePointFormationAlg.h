/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "details/PixelSpacePointFormationAlgBase.h"

namespace ActsTrk {
    class PixelSpacePointFormationAlg: public PixelSpacePointFormationAlgBase<false>{
        using PixelSpacePointFormationAlgBase<false>::PixelSpacePointFormationAlgBase;
    };

    class PixelCacheSpacePointFormationAlg: public PixelSpacePointFormationAlgBase<true>{
        using PixelSpacePointFormationAlgBase<true>::PixelSpacePointFormationAlgBase;
    };
};