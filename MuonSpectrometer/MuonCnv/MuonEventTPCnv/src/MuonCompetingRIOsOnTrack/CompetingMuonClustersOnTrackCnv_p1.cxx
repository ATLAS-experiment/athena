/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   MuonCompetingRIOsOnTrackCnv_p1.cxx
//
//-----------------------------------------------------------------------------

#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "MuonEventTPCnv/MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrackCnv_p1.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"

void 
CompetingMuonClustersOnTrackCnv_p1::persToTrans( const Muon::CompetingMuonClustersOnTrack_p1 *persObj,
                                                       Muon::CompetingMuonClustersOnTrack *transObj, 
                                                       MsgStream &log )
{
  std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>> containedChildRots{};
 
  for (const TPObjRef& ref : persObj->m_containedChildRots) {
    ITPConverterFor<Trk::MeasurementBase>  *rotCnv = nullptr;
    const Muon::MuonClusterOnTrack* mcot = dynamic_cast<const Muon::MuonClusterOnTrack*>(createTransFromPStore(&rotCnv, ref, log));
    containedChildRots.emplace_back(mcot);
  }
   
  ITPConverterFor<Trk::Surface>  *surfaceCnv = nullptr;
  Trk::Surface* associatedSurface    = createTransFromPStore(&surfaceCnv, persObj->m_associatedSurface, log);
  
  *transObj = Muon::CompetingMuonClustersOnTrack (Trk::LocalParameters(),
                                                  Amg::MatrixX(),
                                                  associatedSurface,
                                                  std::move(containedChildRots),
                                                  {}// assgnProb
                                                  );

  fillTransFromPStore( &m_cRotCnv, persObj->m_competingROT, transObj, log );
}

void 
CompetingMuonClustersOnTrackCnv_p1::transToPers( const Muon::CompetingMuonClustersOnTrack* /*transObj*/,
                                                 Muon::CompetingMuonClustersOnTrack_p1* /*persObj*/, 
                                                 MsgStream &/*log*/ ) 
{
  throw std::runtime_error( "CompetingMuonClustersOnTrackCnv_p1::transToPers is deprecated!" );
}


