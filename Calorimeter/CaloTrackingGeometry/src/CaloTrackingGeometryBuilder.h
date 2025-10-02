/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CaloTrackingGeometryBuilder.hm (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef CALORIMETER_CALOTRACKINGGEOMETRYBUILDER_H
#define CALORIMETER_CALOTRACKINGGEOMETRYBUILDER_H

#include "CaloTrackingGeometryBuilderImpl.h"
#include "TrkDetDescrInterfaces/IGeometryBuilder.h"

namespace Calo {

/** @class CaloTrackingGeometryBuilder

  Retrieves the volume builders from Tile and LAr
  and combines the given volumes to a calorimeter tracking
  geometry.

  @author Andreas.Salzburger@cern.ch
  @author Christos Anastopoulos Athena MT
  */
class CaloTrackingGeometryBuilder final
    : public CaloTrackingGeometryBuilderImpl,
      virtual public Trk::IGeometryBuilder {

 public:
  /** Constructor */
  CaloTrackingGeometryBuilder(const std::string&, const std::string&,
                              const IInterface*);

  /** Destructor */
  virtual ~CaloTrackingGeometryBuilder() = default;

  /** AlgTool initailize method.*/
  virtual StatusCode initialize() override;

  /** TrackingGeometry Interface methode */
  virtual std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
      Trk::TrackingVolume* tvol = 0) const override;

  /** The unique signature */
  virtual Trk::GeometrySignature geometrySignature() const override final {
    return CaloTrackingGeometryBuilderImpl::signature();
  }
};

}  // namespace Calo

#endif  // CALORIMETER_CALOTRACKINGGEOMETRYBUILDER_H

