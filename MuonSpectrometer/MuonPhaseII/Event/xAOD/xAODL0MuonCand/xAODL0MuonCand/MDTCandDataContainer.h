/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef xAODL0MUONCAND_MDTCANDDATACONTAINER_H
#define xAODL0MUONCAND_MDTCANDDATACONTAINER_H

#include "xAODL0MuonCand/MDTCandData.h"
#include "xAODL0MuonCand/versions/MDTCandDataContainer_v1.h"


namespace xAOD {
    typedef MDTCandDataContainer_v1 MDTCandDataContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::MDTCandDataContainer , 1490151757 , 1 )
#endif // xAODL0MUONCAND_MDTCANDDATACONTAINER_H

