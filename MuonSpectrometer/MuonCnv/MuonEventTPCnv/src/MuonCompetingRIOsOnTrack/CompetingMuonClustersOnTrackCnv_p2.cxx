/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   MuonCompetingRIOsOnTrackCnv_p1.cxx
//
//-----------------------------------------------------------------------------

#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "MuonEventTPCnv/MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrackCnv_p2.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"

void 
CompetingMuonClustersOnTrackCnv_p2::persToTrans( const Muon::CompetingMuonClustersOnTrack_p1 *persObj,
                                                       Muon::CompetingMuonClustersOnTrack *transObj, 
                                                       MsgStream &log )
{
  std::vector<std::unique_ptr<const Muon::MuonClusterOnTrack>> containedChildRots{};
 
  for (const TPObjRef& ref : persObj->m_containedChildRots) {
    ITPConverterFor<Trk::MeasurementBase>  *rotCnv = nullptr;
    const Muon::MuonClusterOnTrack* mcot = dynamic_cast<const Muon::MuonClusterOnTrack*>(createTransFromPStore(&rotCnv, ref, log));
    containedChildRots.emplace_back( mcot );
  }
   
  Trk::Surface* associatedSurface = this->createTransFromPStore( &m_surfCnv, persObj->m_associatedSurface, log );

  *transObj = Muon::CompetingMuonClustersOnTrack (Trk::LocalParameters(),
                                                  Amg::MatrixX(),
                                                  associatedSurface,
                                                  std::move(containedChildRots),
                                                  {} // assgnProb
                                                  );

   fillTransFromPStore( &m_cRotCnv, persObj->m_competingROT, transObj, log );
}

void 
CompetingMuonClustersOnTrackCnv_p2::transToPers( const Muon::CompetingMuonClustersOnTrack *transObj,
                                                        Muon::CompetingMuonClustersOnTrack_p1 *persObj, 
                                                        MsgStream &log ) 
{
  persObj->m_competingROT = baseToPersistent( &m_cRotCnv,  transObj, log );

  for (const auto& p : transObj->containedROTs()) {
    ITPConverterFor<Trk::MeasurementBase>  *rotCnv = nullptr;
    persObj->m_containedChildRots.push_back( toPersistent(&rotCnv, p.get(), log) );
  }
    
  persObj->m_associatedSurface     = toPersistent(&m_surfCnv, transObj->associatedSurfaceRaw(), log);
}


