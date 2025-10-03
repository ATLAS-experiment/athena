// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRUTH_TRUTHVERTEXAUXCONTAINER_H
#define XAODTRUTH_TRUTHVERTEXAUXCONTAINER_H

// Local include(s):
#include "xAODTruth/versions/TruthVertexAuxContainer_v2.h"

namespace xAOD {
   /// Declare the latest version of the truth vertex auxiliary container
   typedef TruthVertexAuxContainer_v2 TruthVertexAuxContainer;
}

// Declare a CLID for the class
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::TruthVertexAuxContainer, 1254939514, 1 )

#endif // XAODTRUTH_TRUTHVERTEXAUXCONTAINER_H
