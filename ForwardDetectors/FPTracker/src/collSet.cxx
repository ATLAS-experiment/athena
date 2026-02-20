/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FPTracker/collSet.h"
#include "FPTracker/Collimator.h"
#include "FPTracker/CollimatorData.h"
#include "FPTracker/STLHelpers.h"
#include <algorithm>
namespace FPTracker{

  Collimator::Container_t collSet(const CollimatorData& cData, Side side){

    Collimator::Container_t collimators;
    int ncoll = CollimatorData::ncoll;
    double zsign = side == beam2 ? -1.:1.;
    for(int i =0; i<ncoll; ++i){

      double zpos = zsign*cData.coll_z[side][i];
      Collimator::Ptr_t col(new Collimator(zpos,  cData.coll_xap[side][i], side));
      collimators.push_back(std::move(col));
      
    }

    std::sort(collimators.begin(), collimators.end(), absZGreater<Collimator::ConstPtr_t>);
    return collimators;
  }
    
}
