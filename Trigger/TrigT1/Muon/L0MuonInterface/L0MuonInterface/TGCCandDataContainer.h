/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L0MuonInterface_TGCCANDDATACONTAINER_H
#define L0MuonInterface_TGCCANDDATACONTAINER_H

#include "L0MuonInterface/TGCCandData.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace L0Muon {

using TGCCandDataContainer = DataVector<TGCCandData>;

}  // namespace L0Muon

CLASS_DEF( L0Muon::TGCCandDataContainer , 1262868476 , 1 )

#endif  // L0MuonInterface_TGCCANDDATACONTAINER_H
