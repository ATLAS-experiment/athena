/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "details/StripSpacePointFormationAlgBase.h"

namespace ActsTrk {
    class StripSpacePointFormationAlg: public StripSpacePointFormationAlgBase<false>{
        using StripSpacePointFormationAlgBase<false>::StripSpacePointFormationAlgBase;
    };

    class StripCacheSpacePointFormationAlg: public StripSpacePointFormationAlgBase<true>{
        using StripSpacePointFormationAlgBase<true>::StripSpacePointFormationAlgBase;
    };
};
