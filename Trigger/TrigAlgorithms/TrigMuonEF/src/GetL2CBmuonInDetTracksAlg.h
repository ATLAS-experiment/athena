/*
  Get muon ID tracks from L2CB muons
  
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONEF_GETL2CBMUONINDETTRACKSOUTALG_H
#define TRIGMUONEF_GETL2CBMUONINDETTRACKSOUTALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTrigMuon/L2CombinedMuonContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

class GetL2CBmuonInDetTracksAlg : public AthReentrantAlgorithm
{
  public :

    /** Constructor **/
    GetL2CBmuonInDetTracksAlg( const std::string& name, ISvcLocator* pSvcLocator );
  
    /** initialize */
    virtual StatusCode initialize() override;
  
    /** execute the filter alg */
    virtual StatusCode execute(const EventContext& ctx) const override;


  private :

    //SG::ReadHandleKey<xAOD::TrackParticleContainer> m_fullIDtrackContainerKey{this,"FullIDTrackContainerLocation", "FullIDTracks", "Full ID Tracks Container"};
    SG::ReadHandleKey<xAOD::L2CombinedMuonContainer> m_muonL2CBContainerKey{this,"MuonL2CBContainerLocation", "MuonsL2CB", "L2CB Muon Container"};
    SG::WriteHandleKey<xAOD::TrackParticleContainer> m_idTrackOutputKey{this,"IDtrackOutputLocation", "IDTracksOut", "Output ID Tracks Container"};

};

#endif
