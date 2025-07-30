/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler



//
// includes
//

#include <FTagAnalysisAlgorithms/BTaggingTriggerEfficiencyAlg.h>
#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigAnalysisHelpers/FeatureRequestDescriptor.h"
#include "xAODBTagging/BTagging.h"

//
// method implementations
//

namespace CP
{
  BTaggingTriggerEfficiencyAlg::BTaggingTriggerEfficiencyAlg
  (const std::string &name, ISvcLocator *svcLoc) :
    EL::AnaAlgorithm(name, svcLoc),
    m_trigDecTool("Trig::TrigDecisionTool/TrigDecisionTool")
  {
    declareProperty("TrigDecisionTool", m_trigDecTool, "trigger decision tool");
  }

  StatusCode BTaggingTriggerEfficiencyAlg ::
  initialize ()
  {
    if (m_scaleFactorDecoration.empty())
    {
      ANA_MSG_ERROR ("no scale factor decoration name set");
      return StatusCode::FAILURE;
    }

    ANA_CHECK (m_offlineEfficiencyTool.retrieve());
    ANA_CHECK (m_triggerEfficiencyTool.retrieve());
    ANA_CHECK (m_conditionalEfficiencyTool.retrieve());

    ANA_CHECK (m_trigDecTool.retrieve());
    
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ATH_CHECK (m_truthFlav.initialize(m_systematicsList, m_jetHandle));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_selectionHandle.initialize (m_systematicsList, m_jetHandle, SG::AllowEmpty));
    ANA_CHECK (m_scaleFactorDecoration.initialize (m_systematicsList, m_jetHandle));

    ANA_CHECK (m_systematicsList.addSystematics (*m_offlineEfficiencyTool));
    ANA_CHECK (m_systematicsList.addSystematics (*m_triggerEfficiencyTool));
    ANA_CHECK (m_systematicsList.addSystematics (*m_conditionalEfficiencyTool));

    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode BTaggingTriggerEfficiencyAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      ANA_CHECK (m_offlineEfficiencyTool->applySystematicVariation (sys));
      ANA_CHECK (m_triggerEfficiencyTool->applySystematicVariation (sys));
      ANA_CHECK (m_conditionalEfficiencyTool->applySystematicVariation (sys));
      
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.retrieve (jets, sys));
      for (const xAOD::Jet *jet : *jets)
      {
        if (m_preselection.getBool (*jet, sys))
        {
          float sf = 1.;
          CP::CorrectionCode valid;

          // For non-bjet, just get offline scale factor
          if(m_truthFlav.get(*jet, sys)!=5){
            valid = m_offlineEfficiencyTool->getScaleFactor(*jet, sf);
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);
          }

          else {
            float trigEff_MC = 0;
            float trigEff_data = 0;
            valid = m_triggerEfficiencyTool->getMCEfficiency(*jet, trigEff_MC);
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);
            valid = m_triggerEfficiencyTool->getScaleFactor(*jet, trigEff_data);
            trigEff_data *= trigEff_MC;
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);

            float condEff_MC = 0;
            float condEff_data = 0;
            valid = m_conditionalEfficiencyTool->getMCEfficiency(*jet, condEff_MC);
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);
            valid = m_conditionalEfficiencyTool->getScaleFactor(*jet, condEff_data);
            condEff_data *= condEff_MC;
            ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);

            bool passTrigger = false;
            ATH_CHECK(passTriggerBtag(jet, passTrigger));

            if(passTrigger){
              sf = condEff_data * trigEff_data;
              sf /= condEff_MC * trigEff_MC;
            }
            else{
              float offlEff_MC = 0;
              float offlEff_data = 0;
              valid = m_offlineEfficiencyTool->getMCEfficiency(*jet, offlEff_MC);
              ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);
              valid = m_offlineEfficiencyTool->getScaleFactor(*jet, offlEff_data);
              offlEff_data *= offlEff_MC;
              ANA_CHECK_CORRECTION (m_outOfValidity, *jet, valid);

              float num = offlEff_data - condEff_data * trigEff_data;
              float denom = offlEff_MC - condEff_MC * trigEff_MC;
              if(num>0 && denom>0) sf = num / denom;
              else sf = invalidScaleFactor();
            }
          }

          if (m_outOfValidity.get(*jet))
            m_scaleFactorDecoration.set (*jet, sf, sys);
          else
            m_scaleFactorDecoration.set (*jet, invalidScaleFactor(), sys);
        } else {
          m_scaleFactorDecoration.set (*jet, invalidScaleFactor(), sys);
        }
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode BTaggingTriggerEfficiencyAlg::passTriggerBtag(const xAOD::Jet* jet, bool& btag) const{
    btag = false;
    if(!m_trigDecTool->isPassed(m_trigger)){
      // No further check, btag will be false
      return StatusCode::SUCCESS;
    }

    Trig::FeatureRequestDescriptor frd;
    frd.setChainGroup(m_trigger);

    int ileg = 0;
    const xAOD::IParticle* bestHLT = nullptr;
    float minDRHLT = 0.4; // hard-coded matching distance

    for (const ChainNameParser::LegInfo& legInfo :
	   ChainNameParser::HLTChainInfo(m_trigger)){
      if (legInfo.signature == "j"){
	ATH_MSG_VERBOSE(" Leg" << ileg << ": "
			<< " " << legInfo.legName() << " "
			<< legInfo.type() << " " << legInfo.signature
			<< " " << legInfo.threshold);

	frd.setRestrictRequestToLeg(ileg);
	auto hlt_jets = m_trigDecTool->features<xAOD::IParticleContainer>(frd);

	for (const auto& hlt_jet_link : hlt_jets){
	  const xAOD::IParticle *hlt_jet = *hlt_jet_link.link;
	  float dR = jet->p4().DeltaR(hlt_jet->p4());
	  bool hasBtag = false;
	  if(m_useRun3TriggerEDM){
            // we need to access via the trigger decision tool
	    hasBtag = hlt_jet_link.source->hasObjectLink("btag");
	  }
	  else{
	    double hlt_bscore = -1.;
	    ATH_CHECK(getBtagScore(hlt_jet, hlt_bscore));
	    hasBtag = hlt_bscore > m_btagThreshold;
	  }

	  ATH_MSG_VERBOSE("  pt: "
			  << hlt_jet->pt() << " eta: " << hlt_jet->eta()
			  << " phi: " << hlt_jet->phi() << " dR: " << dR
			  << " btag: " << hasBtag);

	  if (bestHLT && isSameJet(bestHLT, hlt_jet))
	    btag |= hasBtag; // if any leg claims b-tag, then the jet is b-tagged
	  else if (dR < minDRHLT) {
	    minDRHLT = dR;
	    bestHLT = hlt_jet;
	    btag = hasBtag;
	  }
	}
      }

      ATH_MSG_VERBOSE(" =dRHLT: " << minDRHLT << " bestHLT pT: "
		      << (bestHLT ? bestHLT->pt() : -99.)
		      << " btag: " << btag);

      ileg++;
    }

    return StatusCode::SUCCESS;
  }

  bool BTaggingTriggerEfficiencyAlg::isSameJet(const xAOD::IParticle *jet1,
					       const xAOD::IParticle *jet2) const{
    // Need this function because jet1 == jet2 would return false
    // when comparing b-jet to untagged jet
    return (jet1->p4().DeltaR(jet2->p4()) < 0.01) &&
      (std::abs(jet1->pt() - jet2->pt()) < 100);
  }

  StatusCode BTaggingTriggerEfficiencyAlg::getBtagScore(const xAOD::IParticle *jet, double& hlt_bscore) const {
    SG::ConstAccessor<const xAOD::BTagging*> acc("HLTBTag");
    const xAOD::BTagging* tagInfo = acc(*jet);

    if(m_trigger.value().find("mv2c20") != std::string::npos){
      if(!tagInfo->MVx_discriminant("MV2c20", hlt_bscore)){
	ATH_MSG_ERROR("MV2c20 discriminant not accessible");
	return StatusCode::FAILURE;
      }
    }
    else if(m_trigger.value().find("mv2c10") != std::string::npos){
      if(!tagInfo->MVx_discriminant("MV2c10", hlt_bscore)){
	ATH_MSG_ERROR("MV2c10 discriminant not accessible");
	return StatusCode::FAILURE;
      }
    }
    else{
      double w1 = tagInfo->IP3D_pb() / tagInfo->IP3D_pu();
      double w2 = tagInfo->SV1_pb() / tagInfo->SV1_pu();
      double W = w1 * w2;
      if ( W/(1.0+W) < 1.0 )
	hlt_bscore = -1.0 * std::log10(1.0 - ( W / ( 1.0 + W ) ) );
      else hlt_bscore = 50;
    }
    return StatusCode::SUCCESS;
  }

}
