/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWASBUILTDBDATA_H
#define MUONCONDDATA_NSWASBUILTDBDATA_H

// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"
#ifndef SIMULATIONBASE
#include "MuonNSWAsBuilt/StripCalculator.h"
#include "MuonNSWAsBuilt/StgcStripCalculator.h"
#endif

class NswAsBuiltDbData {

public:
#ifndef SIMULATIONBASE
    /// Storage to the micromega as built calculator
    using MmAsBuiltPtr  = GeoModel::TransientConstSharedPtr<NswAsBuilt::StripCalculator>;
#else
    /// Define a dummy pointer object if the project is AthSimulation
    using MmAsBuiltPtr = char;
#endif
    MmAsBuiltPtr microMegaData{};

    NswAsBuiltDbData() = default;
    ~NswAsBuiltDbData() = default;

};

CLASS_DEF( NswAsBuiltDbData , 163462850 , 1 );
CONDCONT_DEF( NswAsBuiltDbData , 20792446 );

#endif
