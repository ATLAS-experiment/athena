/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// HGTD_DetectorElementCollection.h
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef HGTD_READOUTGEOMETRY_HGTD_DETECTORELEMENTCOLLECTION_H
#define HGTD_READOUTGEOMETRY_HGTD_DETECTORELEMENTCOLLECTION_H

#include "AthContainers/DataVector.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"

class IdentifierHash;

namespace InDetDD {
using HGTD_DetectorElementCollection = DataVector<HGTD_DetectorElement>;

namespace HGTDDetEl {

inline
const HGTD_DetectorElement* getDetectorElement(const IdentifierHash& hash,
                                               const HGTD_DetectorElementCollection& coll) {
  const unsigned int value{hash.value()};
  if (coll.size() <= value){
    return nullptr;
  }
  return coll.at(value);
}
}  // namespace HGTDDetEl
}  // namespace InDetDD
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(InDetDD::HGTD_DetectorElementCollection, 1266958207, 1)
#include "AthenaKernel/CondCont.h"
CONDCONT_MIXED_DEF(InDetDD::HGTD_DetectorElementCollection, 1258619755);

#endif  // HGTD_READOUTGEOMETRY_HGTD_DETECTORELEMENTCOLLECTION_H
