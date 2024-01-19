/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MuonInertMaterialBuilderCond.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef MUONTRACKINGGEOMETRY_MUONINERTMATERIALBUILDERCOND_H
#define MUONTRACKINGGEOMETRY_MUONINERTMATERIALBUILDERCOND_H

#include "MuonTrackingGeometry/MuonInertMaterialBuilderImpl.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace Muon {

/** @class MuonInertMaterialBuilderCond

    The Muon::MuonInertMaterialBuilderCond retrieves muon stations from Muon Geometry Tree

    by Sarka.Todorova@cern.ch, Marcin.Wolter@cern.ch
  */

class MuonInertMaterialBuilderCond final : public MuonInertMaterialBuilderImpl {
 public:
  /** Constructor */
  MuonInertMaterialBuilderCond(const std::string&, const std::string&, const IInterface*);
  /** Destructor */
  virtual ~MuonInertMaterialBuilderCond() = default;
  /** AlgTool initialize method.*/
  virtual StatusCode initialize() override;

  /** Method returning cloned and positioned material objects */
  std::unique_ptr<std::vector<std::unique_ptr<Trk::DetachedTrackingVolume> > >
  buildDetachedTrackingVolumes(const EventContext& ctx, SG::WriteCondHandle<Trk::TrackingGeometry>& whandle,
                               bool blend = false) const;

 private:
  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muonMgrReadKey{
      this, "MuonMgrReadKey", "MuonDetectorManager", "Key of input MuonDetectorMgr"};
};

}  // namespace Muon

#endif  // MUONTRACKINGGEOMETRY_MUONINERTMATERIALBUILDERCOND_H
