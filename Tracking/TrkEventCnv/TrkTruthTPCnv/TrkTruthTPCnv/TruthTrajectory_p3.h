/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

// Andrei.Gaponenko@cern.ch, 2007
// Olivier.Arnaez@cern.ch, 2015


#ifndef TRKTRUTHTPCNV_TRUTHTRAJECTORY_P3_H
#define TRKTRUTHTPCNV_TRUTHTRAJECTORY_P3_H

#include <vector>
#include "GeneratorObjectsTPCnv/HepMcParticleLink_p3.h"
#include "AthenaKernel/CLASS_DEF.h"


// Use T/P separated HepMcParticleLink

namespace Trk {
  class TruthTrajectory_p3 : public std::vector<HepMcParticleLink_p3> {};
}

CLASS_DEF( Trk::TruthTrajectory_p3 , 185011230 , 1 )

#endif/*TRKTRUTHTPCNV_TRUTHTRAJECTORY_P3_H*/
