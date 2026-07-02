/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef xAODL0MUONCAND_NSWCANDDATACONTAINER_H
#define xAODL0MUONCAND_NSWCANDDATACONTAINER_H

#include "xAODL0MuonCand/NSWCandData.h"
#include "xAODL0MuonCand/versions/NSWCandDataContainer_v1.h"

namespace xAOD {
  typedef NSWCandDataContainer_v1 NSWCandDataContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::NSWCandDataContainer , 1093508001, 1 )
#endif // xAODL0MUONCAND_NSWCANDDATACONTAINER_H

