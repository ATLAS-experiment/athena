/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOADAPTORS_GEOTRTUNCOMPRESSEDHIT_H
#define GEOADAPTORS_GEOTRTUNCOMPRESSEDHIT_H

/**
 * @file   GeoTRTUncompressedHit.h
 * @class  GeoTRTUncompressedHit
 * @author Joe Boudreau
 * @brief  This adaptor class allows TRT_UncompressedHits to behave
 *          as if they knew which detector they were in
 */

#include "CLHEP/Geometry/Point3D.h"
class TRTUncompressedHit;

namespace InDetDD {
  class TRT_DetectorManager;
}

class GeoTRTUncompressedHit {

 public:

  // Constructor:
  GeoTRTUncompressedHit(const TRTUncompressedHit & h);

  // Get the absolute global position:
  HepGeom::Point3D<double> getGlobalPosition(const InDetDD::TRT_DetectorManager* mgr) const;

  // Is this hit OK? 
  operator bool () const { return true; }

 private:
  const TRTUncompressedHit* m_hit{nullptr};
};

#include "GeoAdaptors/GeoTRTUncompressedHit.icc"

#endif
