// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRUTH_TRUTHPARTICLEAUXCONTAINER_H
#define XAODTRUTH_TRUTHPARTICLEAUXCONTAINER_H

// Local include(s):
#include "xAODTruth/versions/TruthParticleAuxContainer_v2.h"

namespace xAOD {
   /// Declare the latest version of the truth particle auxiliary container
   typedef TruthParticleAuxContainer_v2 TruthParticleAuxContainer;
}

// Declare a CLID for the class
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::TruthParticleAuxContainer, 1107340896, 1 )

#endif // XAODTRUTH_TRUTHPARTICLEAUXCONTAINER_H
