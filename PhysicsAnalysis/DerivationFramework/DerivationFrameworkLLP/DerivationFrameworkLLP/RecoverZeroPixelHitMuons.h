/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DERIVATIONFRAMEWORKLLP_RECOVERZEROPIXELHITMUONS_H
#define DERIVATIONFRAMEWORKLLP_RECOVERZEROPIXELHITMUONS_H

// Framework includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#include "xAODMuon/xAODMuonEnums.h"
#include "xAODTracking/TrackParticleContainer.h"

// STL includes
#include <string>



/**
 * @class RecoverZeroPixelHitMuons
 * @brief 
 **/
class RecoverZeroPixelHitMuons : public AthReentrantAlgorithm {
public:
  RecoverZeroPixelHitMuons(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  
private:
  //Gaudi::Property<int> m_myInt{this, "MyInt", 0, "An Integer"};
  // Input & Output containers
  SG::ReadHandleKey<xAOD::MuonContainer> m_inputMuonContainerKey{this, "InputMuonContainer", "Muons"};
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputTrackContainerKey{this, "InputTrackContainer", "InDetTrackParticles"};
  SG::WriteHandleKey<xAOD::MuonContainer> m_outputMuonContainerKey{this, "OutputMuonContainer", "ZeroPixelHitMuons"}; 

  // Kinematic Variables
  Gaudi::Property<float> m_matchingDeltaR{this,"MatchingDeltaR",0.1};
  
};

#endif // DERIVATIONFRAMEWORKLLP_RECOVERZEROPIXELHITMUONS_H
