///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// AthExIParticles.h 
// Header file for class AthExIParticles
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHEXTHINNING_ATHEXIPARTICLES_H 
#define ATHEXTHINNING_ATHEXIPARTICLES_H 

// STL includes

// Gaudi includes

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

// AthExThinning includes
#include "AthExThinning/AthExIParticle.h"


using AthExIParticles = DataVector<AthExIParticle>;


CLASS_DEF( AthExIParticles , 141971559, 1 )

#endif //> ATHEXTHINNING_ATHEXIPARTICLES_H
