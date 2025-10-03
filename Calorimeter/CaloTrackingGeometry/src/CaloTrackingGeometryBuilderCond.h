/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CaloTrackingGeometryBuilderCond.hm (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef CALORIMETER_CALOTRACKINGGEOMETRYBUILDERCOND_H
#define CALORIMETER_CALOTRACKINGGEOMETRYBUILDERCOND_H

#include "CaloTrackingGeometryBuilderImpl.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"

/** @class CaloTrackingGeometryBuilderCond

  Retrieves the volume builders from Tile and LAr
  and combines the given volumes to a calorimeter tracking
  geometry.

  It also wraps an inner volume if provided though
  the mehtod interface.

  @author Andreas.Salzburger@cern.ch
  @author Christos Anastopoulos (Athena MT)
  */
namespace Calo {
class CaloTrackingGeometryBuilderCond final
    : public CaloTrackingGeometryBuilderImpl,
      virtual public Trk::IGeometryBuilderCond {

 public:
  /** Constructor */
  CaloTrackingGeometryBuilderCond(const std::string&, const std::string&,
                                  const IInterface*);

  /** Destructor */
  virtual ~CaloTrackingGeometryBuilderCond() = default;

  /** AlgTool initailize method.*/
  virtual StatusCode initialize() override final;

  /** TrackingGeometry Interface method */
  virtual std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
      const EventContext& ctx, Trk::TrackingVolume* innerVol,
      SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;
  /** The unique signature */
  virtual Trk::GeometrySignature geometrySignature() const override final {
    return CaloTrackingGeometryBuilderImpl::signature();
  }

 private:
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{
      this, "CaloDetDescrManager", "CaloDetDescrManager"};
    SG::ReadCondHandleKey<GeoAlignmentStore>  m_readKeyGeoAlign {
      this, "LArAlignmentStore", "LArAlignmentStore", "SG key of the GeoAlignmentStore for LAr" };
};

}  // namespace Calo

#endif  // CALORIMETER_CALOTRACKINGGEOMETRYBUILDERCOND_H

