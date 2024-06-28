/*
  Merge inside-out, outside-in and L2mt inner detector track containers
  
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONEF_MERGEMUONINDETTRACKSOUTALG_H
#define TRIGMUONEF_MERGEMUONINDETTRACKSOUTALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTrigMuon/L2CombinedMuonContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

class MergeMuonInDetTracksAlg : public AthReentrantAlgorithm
{
  public :

    /** Constructor **/
    MergeMuonInDetTracksAlg( const std::string& name, ISvcLocator* pSvcLocator );
  
    /** initialize */
    virtual StatusCode initialize() override;
  
    /** execute the filter alg */
    virtual StatusCode execute(const EventContext& ctx) const override;


  private :

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_fullIDtrackContainerKey{this,"FullIDTrackContainerLocation", "FullIDTracks", "Full ID Tracks Container"};
    SG::ReadHandleKey<xAOD::L2CombinedMuonContainer> m_muonCBContainerKey{this,"MuonCBContainerLocation", "MuonsCB", "CB Muon Container"};
    SG::ReadHandleKey<xAOD::L2CombinedMuonContainer> m_muonInsideOutContainerKey{this,"MuonInsideOutContainerLocation", "MuonsInsideOut", "InsideOut Muon Container"};
    SG::ReadHandleKey<xAOD::L2CombinedMuonContainer> m_muonL2mtContainerKey{this,"MuonL2mtContainerLocation", "MuonsL2mt", "L2mt Muons Container"};
    SG::WriteHandleKey<xAOD::TrackParticleContainer> m_idTrackOutputKey{this,"IDtrackOutputLocation", "IDTracksOut", "Output ID Tracks Container"};

};

#endif
