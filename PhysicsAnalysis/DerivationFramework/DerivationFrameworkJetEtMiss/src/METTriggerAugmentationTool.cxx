/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "METTriggerAugmentationTool.h"
#include <xAODTrigger/EnergySumRoIAuxInfo.h>
#include <xAODTrigger/JetRoI.h>
#include <PathResolver/PathResolver.h>
#include <vector>
#include <string>

#include <TFile.h>

namespace DerivationFramework {

  METTriggerAugmentationTool::METTriggerAugmentationTool(const std::string& t,
                                                         const std::string& n,
                                                         const IInterface* p) :
    base_class(t,n,p)
  {
  }

  StatusCode METTriggerAugmentationTool::initialize()
  {
    ATH_CHECK(m_L1METName.initialize());
    ATH_CHECK(m_L1JetName.initialize());
    ATH_CHECK(m_outputName.initialize());
    std::string fullLUTFileName = PathResolver::find_file(m_LUTFileName, "DATAPATH");

    if (fullLUTFileName.empty() ) {
      ATH_MSG_ERROR( "File " << m_LUTFileName << " not found!" );
      return StatusCode::FAILURE;
    }

    TFile* lutFile = TFile::Open(fullLUTFileName.c_str() );
    if (lutFile->IsZombie() ) return StatusCode::FAILURE;

    TH2* lutFromFile{};
    lutFile->GetObject("LUT", lutFromFile);
    if (!lutFromFile) {
      ATH_MSG_ERROR( "LUT file doesn't contain a 'LUT' object!" );
      return StatusCode::FAILURE;
    }

    m_LUT.reset(dynamic_cast<TH2*>(lutFromFile->Clone() ) );
    m_LUT->SetDirectory(0);
    lutFile->Close();
    return StatusCode::SUCCESS;
  }

  StatusCode METTriggerAugmentationTool::addBranches(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG(" In L1KF_METMaker::makeKFMET()" );
    // if the output has already been written we don't need to do anything
    if (evtStore()->contains<xAOD::EnergySumRoI>(m_outputName.key()) ) return StatusCode::SUCCESS; // FIXME  tool should not have been configured in this case

    SG::ReadHandle<xAOD::EnergySumRoI> originalL1{m_L1METName, ctx};
    if (!originalL1.isValid()){
      ATH_MSG_ERROR("Unable to retrieve EnergySumRoI: " << m_L1METName << "!");
      return StatusCode::FAILURE;
    }

    SG::ReadHandle<xAOD::JetRoIContainer> l1Jets{m_L1JetName, ctx};
    if (!originalL1.isValid()){
      ATH_MSG_ERROR("Unable to retrieve JetRoIContainer: " << m_L1JetName << "!");
      return StatusCode::FAILURE;
    }

    SG::WriteHandle<xAOD::EnergySumRoI> l1_kf{m_outputName, ctx};
    if (!originalL1.isValid()){
      ATH_MSG_ERROR("Invalid WriteHandle for EnergySumRoI: " << m_outputName << "!");
      return StatusCode::FAILURE;
    }
    ATH_CHECK(l1_kf.record(std::make_unique<xAOD::EnergySumRoI>(), std::make_unique<xAOD::EnergySumRoIAuxInfo>()));

    ATH_MSG_DEBUG( "Making deep copy" );
    // copy across the info
    *l1_kf = *originalL1;

    ATH_MSG_DEBUG( "Building KF MET" );
    float KFMETx = l1_kf->exMiss();
    float KFMETy = l1_kf->eyMiss();
    float KFSumEt = l1_kf->energyT();

    for (const auto jet : *l1Jets) {
      int etaBin = m_LUT->GetXaxis()->FindBin( fabs( jet->eta() ) );
      int ptBin = m_LUT->GetYaxis()->FindBin( log2( jet->et8x8() / 1000.) );
      if (ptBin==0) ptBin=1;
      float KFweight = m_LUT->GetBinContent(etaBin, ptBin);
      float jetContribution = jet->et8x8() * KFweight;

      KFMETx -= jetContribution * cos( jet->phi() );
      KFMETy -= jetContribution * sin( jet->phi() );
      KFSumEt += jetContribution;
    }
    l1_kf->setEnergyX(-KFMETx);
    l1_kf->setEnergyY(-KFMETy);
    l1_kf->setEnergyT(KFSumEt);

    ATH_MSG_DEBUG( "Built KF MET" );
    return StatusCode::SUCCESS;
  }
}
