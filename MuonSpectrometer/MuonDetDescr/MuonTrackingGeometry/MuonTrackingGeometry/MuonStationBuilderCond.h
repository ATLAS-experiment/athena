/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRACKINGGEOMETRY_MUONSTATIONBUILDERCOND_H
#define MUONTRACKINGGEOMETRY_MUONSTATIONBUILDERCOND_H

#include "MuonTrackingGeometry/MuonStationBuilderImpl.h"
#include "TrkDetDescrInterfaces/IDetachedTrackingVolumeBuilderCond.h"
#include "TrkGeometry/TrackingGeometry.h"
//
#include "StoreGate/ReadCondHandleKey.h"
namespace Muon {

/** @class MuonStationBuilderCond
    The Muon::MuonStationBuilderCond retrieves muon stations from Muon Geometry
   Tree prototypes built with help of Muon::MuonStationTypeBuilder
    by Sarka.Todorova@cern.ch
  */

class MuonStationBuilderCond final
    : public MuonStationBuilderImpl,
      virtual public Trk::IDetachedTrackingVolumeBuilderCond {
   public:
    MuonStationBuilderCond(const std::string&, const std::string&,
                           const IInterface*);
    virtual ~MuonStationBuilderCond() = default;
    virtual StatusCode initialize() override;

    virtual DetachedVolumeVec 
          buildDetachedTrackingVolumes(const EventContext& ctx,
                                       SG::WriteCondHandle<Trk::TrackingGeometry>& whandle,
                                       bool blend = false) const override;

   private:
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muonMgrReadKey{
        this, "MuonMgrReadKey", "MuonDetectorManager", "Key of input MuonMgr"};
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONSTATIONBUILDERCOND_H
