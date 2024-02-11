/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonTrackingGeometryBuilderCond.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef MUONTRACKINGGEOMETRY_MUONTRACKINGGEOMETRYBUILDERCOND_H
#define MUONTRACKINGGEOMETRY_MUONTRACKINGGEOMETRYBUILDERCOND_H

#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
//
#include "MuonTrackingGeometry/MuonInertMaterialBuilderCond.h"
#include "MuonTrackingGeometry/MuonTrackingGeometryBuilderImpl.h"
#include "TrkDetDescrInterfaces/IDetachedTrackingVolumeBuilderCond.h"
#include "TrkGeometry/TrackingGeometry.h"
//
#include "GaudiKernel/ToolHandle.h"

namespace Muon {

/** @class MuonTrackingGeometryBuilderCond

    The Muon::MuonTrackingGeometryBuilderCond retrieves MuonStationBuilder and
   MuonInertMaterialBuilder for the Muon Detector sub detectors and combines the
   given Volumes to a full Trk::TrackingGeometry.

    Inheriting directly from IGeometryBuilderCond it can use the protected
   member functions of the IGeometryBuilderCond to glue Volumes together and
   exchange BoundarySurfaces

    @author Sarka.Todorova@cern.ch
  */

class MuonTrackingGeometryBuilderCond final
    : public MuonTrackingGeometryBuilderImpl,
      virtual public Trk::IGeometryBuilderCond {
   public:
    /** Constructor */
    MuonTrackingGeometryBuilderCond(const std::string&, const std::string&,
                                    const IInterface*);
    /** Destructor */
    virtual ~MuonTrackingGeometryBuilderCond() = default;
    /** AlgTool initailize method.*/
    virtual StatusCode initialize() override;
    /** TrackingGeometry Interface method */
    virtual std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
        const EventContext& ctx, Trk::TrackingVolume* tvol,
        SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override;

    /** The unique signature */
    virtual Trk::GeometrySignature geometrySignature() const override {
        return MuonTrackingGeometryBuilderImpl::signature();
    }

   private:
    ToolHandle<Trk::IDetachedTrackingVolumeBuilderCond> m_stationBuilder{
        this, "MuonStationBuilder",
        "Muon::MuonStationBuilderCond/MuonStationBuilderCond"};  //!< A Tool for
                                                                 //!< station
                                                                 //!< type
                                                                 //!< creation

    ToolHandle<Muon::MuonInertMaterialBuilderCond>
        m_inertBuilder{this, "InertMaterialBuilder",
                       "Muon::MuonInertMaterialBuilderCond/"
                       "MuonInertMaterialBuilderCond"};  //!< A Tool for inert
                                                         //!< object  creation
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONTRACKINGGEOMETRYBUILDER_H
