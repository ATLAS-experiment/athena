/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonTrackingGeometryBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef MUONTRACKINGGEOMETRY_MUONTRACKINGGEOMETRYBUILDER_H
#define MUONTRACKINGGEOMETRY_MUONTRACKINGGEOMETRYBUILDER_H

#include "TrkDetDescrInterfaces/IGeometryBuilder.h"
//
#include "MuonTrackingGeometry/MuonInertMaterialBuilder.h"
#include "MuonTrackingGeometry/MuonTrackingGeometryBuilderImpl.h"
#include "TrkDetDescrInterfaces/IDetachedTrackingVolumeBuilderCond.h"
//
#include "GaudiKernel/ToolHandle.h"
namespace Muon {

/** @class MuonTrackingGeometryBuilder

    The Muon::MuonTrackingGeometryBuilder retrieves MuonStationBuilder and
   MuonInertMaterialBuilder for the Muon Detector sub detectors and combines the
   given Volumes to a full Trk::TrackingGeometry.

    Inheriting directly from IGeometryBuilder it can use the protected member
   functions of the IGeometryBuilder to glue Volumes together and exchange
   BoundarySurfaces

    @author Sarka.Todorova@cern.ch
  */

class MuonTrackingGeometryBuilder final
    : public MuonTrackingGeometryBuilderImpl,
      virtual public Trk::IGeometryBuilder {
   public:
    /** Constructor */
    MuonTrackingGeometryBuilder(const std::string&, const std::string&,
                                const IInterface*);
    /** Destructor */
    virtual ~MuonTrackingGeometryBuilder() = default;
    /** AlgTool initailize method.*/
    virtual StatusCode initialize() override;
    /** TrackingGeometry Interface method */
    virtual std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
        Trk::TrackingVolume* tvol = 0) const override;

    /** The unique signature */
    virtual Trk::GeometrySignature geometrySignature() const override {
        return MuonTrackingGeometryBuilderImpl::signature();
    }

   private:
    ToolHandle<Trk::IDetachedTrackingVolumeBuilder> m_stationBuilder{
        this, "MuonStationBuilder",
        "Muon::MuonStationBuilder/MuonStationBuilder"};  //!< A Tool for station
                                                         //!< type creation

    ToolHandle<Muon::MuonInertMaterialBuilder>
        m_inertBuilder{this, "InertMaterialBuilder",
                       "Muon::MuonInertMaterialBuilder/"
                       "MuonInertMaterialBuilder"};  //!< A Tool for inert
                                                     //!< object creation
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONTRACKINGGEOMETRYBUILDER_H
