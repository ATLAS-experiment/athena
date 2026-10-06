/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MuonInterface_TGCCANDDATACONTAINER_H
#define L1MuonInterface_TGCCANDDATACONTAINER_H

#include "L1MuonInterface/TGCCandData.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace L1Muon {

using TGCCandDataContainer = DataVector<TGCCandData>;

}  // namespace L1Muon

CLASS_DEF( L1Muon::TGCCandDataContainer , 1262868476 , 1 )

#endif  // L1MuonInterface_TGCCANDDATACONTAINER_H
