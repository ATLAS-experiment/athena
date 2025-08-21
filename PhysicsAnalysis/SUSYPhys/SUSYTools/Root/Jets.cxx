/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// This source file implements all of the functions related to Jets
// in the SUSYObjDef_xAOD class

// Local include(s):
#include "SUSYTools/SUSYObjDef_xAOD.h"

#include "xAODBase/IParticleHelpers.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetInterface/IJetSelector.h"
#include "JetCPInterfaces/ICPJetUncertaintiesTool.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "JetInterface/IJetModifier.h"
#include "JetInterface/IJetDecorator.h"
#include "JetAnalysisInterfaces/IJetJvtEfficiency.h"

#include "xAODBTagging/BTaggingUtilities.h"
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"

#include "METUtilities/METHelpers.h"

#include "AthContainers/ConstDataVector.h"
#include "FourMomUtils/xAODP4Helpers.h"
#ifndef XAOD_STANDALONE // For now metadata is Athena-only
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"
#endif

namespace ST {

  const static SG::Decorator<char>     dec_bad("bad");

  const static SG::Decorator<char>      dec_passJvt("passJvt");
  const static SG::ConstAccessor<char>  acc_passJvt("passJvt");
  const static SG::Decorator<char>      dec_passFJvt("passFJvt");
  const static SG::ConstAccessor<char>  acc_passFJvt("passFJvt");

  const static SG::Decorator<float> dec_jvt("Jvt");
  const static SG::ConstAccessor<float> acc_jvt("NNJvt");
  const static SG::Decorator<float> dec_fjvt("fJvt");
  const static SG::ConstAccessor<float> acc_fjvt("DFCommonJets_fJvt");

  const static SG::Decorator<char> dec_bjet("bjet");

  const static SG::Decorator<char> dec_bjet_jetunc("bjet_jetunc"); //added for JetUncertainties usage
  const static SG::Decorator<char> dec_bjet_loose("bjet_loose");

  const static SG::Decorator<double> dec_btag_weight("btag_weight");
  const static SG::Decorator<float> dec_btag_pb("btag_pb");
  const static SG::Decorator<float> dec_btag_pc("btag_pc");
  const static SG::Decorator<float> dec_btag_pu("btag_pu");
  const static SG::Decorator<float> dec_btag_ptau("btag_ptau");
  // for backwards compatibility
  const static SG::Decorator<float> dec_btag_dl1pb("btag_dl1pb");
  const static SG::Decorator<float> dec_btag_dl1pc("btag_dl1pc");
  const static SG::Decorator<float> dec_btag_dl1pu("btag_dl1pu");


  const static SG::Decorator<float> dec_VRradius("VRradius");
  const static SG::ConstAccessor<float> acc_VRradius("VRradius");

  const static SG::Decorator<char> dec_passDRcut("passDRcut");
  const static SG::ConstAccessor<char> acc_passDRcut("passDRcut");

  const static SG::Decorator<int> dec_wtagged("wtagged");
  const static SG::Decorator<int> dec_ztagged("ztagged");
  const static SG::Decorator<int> dec_toptagged("toptagged");

  StatusCode SUSYObjDef_xAOD::GetJets(xAOD::JetContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, bool recordSG, const std::string& jetkey, const xAOD::JetContainer* containerToBeCopied)
  {
    if (!m_tool_init) {
      ATH_MSG_ERROR("SUSYTools was not initialized!!");
      return StatusCode::FAILURE;
    }

  if (m_isPHYSLITE && jetkey.find("AnalysisJets") == std::string::npos){
    ATH_MSG_ERROR("You are running on PHYSLITE derivation. Please change the Jets container to 'AnalysisJets'");
    return StatusCode::FAILURE;
  }

    ATH_MSG_DEBUG("Default jetkey:           " << m_defaultJets);
    ATH_MSG_DEBUG("Function argument jetkey: " << jetkey);

    // load default regular & btag jet keys
    std::string jetkey_tmp = m_defaultJets;                                           // use default for regular jetkey_tmp

    // override default if user is passing a jetkey
    if (!jetkey.empty()) {
      jetkey_tmp = jetkey;
    }

    // final settings
    ATH_MSG_DEBUG("Key for retrieving jet collection:             jetkey      = " << jetkey_tmp);

    const xAOD::JetContainer* jets = nullptr;
    if (copy==nullptr) { // empty container provided
      if (containerToBeCopied != nullptr) {
        jets = containerToBeCopied;
      }
      else {
        ATH_MSG_DEBUG("Retrieve jet collection: " << jetkey_tmp);
        ATH_CHECK( evtStore()->retrieve(jets, jetkey_tmp) );
      }
      std::pair<xAOD::JetContainer*, xAOD::ShallowAuxContainer*> shallowcopy = xAOD::shallowCopyContainer(*jets);
      copy = shallowcopy.first;
      copyaux = shallowcopy.second;
      bool setLinks = xAOD::setOriginalObjectLink(*jets, *copy);
      if (!setLinks) {
        ATH_MSG_WARNING("Failed to set original object links on " << jetkey_tmp);
      }
    } else { // use the user-supplied collection instead
      ATH_MSG_DEBUG("Not retrieving jet collecton, using existing one provided by user");
      jets = copy;
    }

    // Calibrate the jets
    ATH_CHECK(m_jetCalibTool->applyCalibration(*copy));

    // Add isHS labels to jets (required for JvtEfficiencyTools)
    if (!isData()) {
      ATH_CHECK(m_jetPileupLabelingTool->decorate(*copy));
    }

    // Calculate Jvt scores (required by METSignificance)
    for (const auto jet : *copy) {
      dec_jvt(*jet) = m_jetJvtMomentTool->updateJvt(*jet);
    }

    // Re-calculate NNJvt scores
    if (m_applyJVTCut) ATH_CHECK(m_jetNNJvtMomentTool->decorate(*copy));

    // Update the jets
    for (const auto jet : *copy) {
      ATH_CHECK( this->FillJet(*jet) );
    }

    for (const auto jet : *copy) {
      // Update the JVT decorations if needed
      if( m_doFwdJVT){
        dec_passJvt(*jet) = acc_passFJvt(*jet) && acc_passJvt(*jet);
        dec_fjvt(*jet) = acc_fjvt(*jet);

        //new state for OR   .  0=non-baseline objects, 1=for baseline jets not passing JVT, 2=for any other baseline object
        if ( acc_baseline(*jet) ){
          if( acc_passJvt(*jet) )     dec_selected(*jet) = 2;
          else                        dec_selected(*jet) = 1;
        }
        else{
          dec_selected(*jet) = 0;
        }
      }
      this->IsBadJet(*jet);
      this->IsSignalJet(*jet, m_jetPt, m_jetEta);
      if (!isData())this->IsTruthBJet(*jet);
    }
    if (recordSG) {
      std::string auxname = copyaux->name();
      if (auxname.compare("UNKNOWN")==0) copyaux->setName(std::string("STCalib" + jetkey_tmp + m_currentSyst.name() + "AuxCopy").c_str());
      ATH_CHECK( evtStore()->record(copy, "STCalib" + jetkey_tmp + m_currentSyst.name()) );
      ATH_CHECK( evtStore()->record(copyaux, "STCalib" + jetkey_tmp + m_currentSyst.name() + "Aux.") );
    }
    return StatusCode::SUCCESS;
  }

  StatusCode SUSYObjDef_xAOD::GetTrackJets(xAOD::JetContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, bool recordSG, const std::string& jetkey, const xAOD::JetContainer* containerToBeCopied)
  {
    if (!m_tool_init) {
      ATH_MSG_ERROR("SUSYTools was not initialized!!");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Default jetkey (trkjet):           " << m_defaultTrackJets);
    ATH_MSG_DEBUG("Function argument jetkey (trkjet): " << jetkey);

    // load default regular & btag jet keys
    std::string jetkey_tmp = m_defaultTrackJets;

    // override default if user is passing a jetkey
    if (!jetkey.empty()) {
      jetkey_tmp = jetkey;
    }

    // final settings
    ATH_MSG_DEBUG("Key for retrieving trkjet collection (as well as bjet info):      jetkey      = " << jetkey_tmp);

    const xAOD::JetContainer* jets = nullptr;
    if (copy==nullptr) { // empty container provided
      if (containerToBeCopied != nullptr) {
        jets = containerToBeCopied;
      }
      else {
        ATH_MSG_DEBUG("Retrieve jet collection: " << jetkey_tmp);
        ATH_CHECK( evtStore()->retrieve(jets, jetkey_tmp) );
      }
      std::pair<xAOD::JetContainer*, xAOD::ShallowAuxContainer*> shallowcopy = xAOD::shallowCopyContainer(*jets);
      copy = shallowcopy.first;
      copyaux = shallowcopy.second;
      bool setLinks = xAOD::setOriginalObjectLink(*jets, *copy);
      if (!setLinks) {
        ATH_MSG_WARNING("Failed to set original object links on " << jetkey_tmp);
      }
    } else { // use the user-supplied collection instead
      ATH_MSG_DEBUG("Not retrieving jet collection, using existing one provided by user");
      jets = copy;
    }

    //disable - notfortrackjets? // Calibrate the jets
    //disable - notfortrackjets? ATH_CHECK(m_jetCalibTool->applyCalibration(*copy));

    // Update the jets
    for (const auto jet : *copy) {
      ATH_CHECK( this->FillTrackJet(*jet) );
    }

    if (copy->size() > 1 && m_defaultTrackJets == "AntiKtVR30Rmax4Rmin02TrackJets") {
        // Use iterators to avoid pairing the jets twice
        for (xAOD::JetContainer::const_iterator j1 = copy->begin()+1; j1!= copy->end();++j1) {
          const xAOD::Jet* jet1 = (*j1);
          if (!acc_signal(*jet1)) continue;
          for (xAOD::JetContainer::const_iterator j2 = copy->begin(); j2 != j1; ++j2) {
            const xAOD::Jet* jet2 = (*j2);
            if (!acc_baseline(*jet2)) continue;
            //Reference to the use method in P4Helper: deltaR2( const xAOD::IParticle& p4, const xAOD::IParticle& , bool useRapidity=true )
            float dr_jets =  xAOD::P4Helpers::deltaR(jet1,jet2, false);
            const xAOD::Jet* to_check = acc_VRradius(*jet1) < acc_VRradius(*jet2) ? jet1 : jet2;
            if( dr_jets < acc_VRradius(*to_check)) dec_passDRcut(*to_check) = false;
            //break the loop at this point???
          }
        }
    }
    if (recordSG) {
      ATH_CHECK( evtStore()->record(copy, "STCalib" + jetkey_tmp + m_currentSyst.name()) );
      ATH_CHECK( evtStore()->record(copyaux, "STCalib" + jetkey_tmp + m_currentSyst.name() + "Aux.") );
    }
    return StatusCode::SUCCESS;
  }

  StatusCode SUSYObjDef_xAOD::GetFatJets(xAOD::JetContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, bool recordSG, const std::string& jetkey, const bool doLargeRdecorations, const xAOD::JetContainer* containerToBeCopied)
  {
    if (!m_tool_init) {
      ATH_MSG_ERROR("SUSYTools was not initialized!!");
      return StatusCode::FAILURE;
    }

    if (m_fatJets.empty()) {
      ATH_MSG_ERROR("JetFatCalibTool was not initialized for largeR jet!!");
      return StatusCode::FAILURE;
    }

    std::string jetkey_tmp = jetkey;
    if (jetkey.empty()) {
      jetkey_tmp = m_fatJets;
    }

    const xAOD::JetContainer* jets = nullptr;
    if (copy==nullptr) { // empty container provided
      if (containerToBeCopied != nullptr) {
        jets = containerToBeCopied;
      }
      else {
        ATH_MSG_DEBUG("Retrieve jet collection: " << jetkey_tmp);
        ATH_CHECK( evtStore()->retrieve(jets, jetkey_tmp) );
      }

      std::pair<xAOD::JetContainer*, xAOD::ShallowAuxContainer*> shallowcopy = xAOD::shallowCopyContainer(*jets);
      copy = shallowcopy.first;
      copyaux = shallowcopy.second;
      bool setLinks = xAOD::setOriginalObjectLink(*jets, *copy);
      if (!setLinks) {
        ATH_MSG_WARNING("Failed to set original object links on " << jetkey_tmp);
      }
    } else { // use the user-supplied collection instead
      ATH_MSG_DEBUG("Not retrieving jet collection, using existing one provided by user");
      jets=copy;
    }

    if(jets->size()==0) {
        ATH_MSG_DEBUG("Large R jet collection is empty");
        return StatusCode::SUCCESS;
      }

    // Calibrate the jets - only insitu for data for now
    if (isData()) ATH_CHECK(m_jetFatCalibTool->applyCalibration(*copy));


    if (!isData() && !m_JetTruthLabelName.empty()){
      ATH_MSG_DEBUG("Checking if decorator for JetTruthLabelingTool is available:");
      std::string fatjetcoll = m_fatJets;
      m_label_truthKey = fatjetcoll+"."+m_JetTruthLabelName;
      SG::ReadDecorHandle<xAOD::JetContainer, int> labelHandle_truthKey(m_label_truthKey);
      ATH_MSG_DEBUG("Reading JetTruthLabelingTool truthKey:" << m_label_truthKey << " isAvailable " << labelHandle_truthKey.isAvailable());
      // Truth Labeling (MC only)
      if (!labelHandle_truthKey.isAvailable() && !m_isPHYSLITE) ATH_CHECK(m_jetTruthLabelingTool->decorate(*copy));
    }


    for (const auto jet : *copy) {

      ATH_CHECK( this->FillJet(*jet, true, true, doLargeRdecorations) );
      //
      //  For OR, selected if it passed cuts
      if ( acc_baseline(*jet) ){
        dec_selected(*jet) = 1;
      }
      else{
        dec_selected(*jet) = 0;
      }
    }
    if (recordSG) {
      ATH_CHECK( evtStore()->record(copy, "STCalib" + jetkey_tmp + m_currentSyst.name()) );
      ATH_CHECK( evtStore()->record(copyaux, "STCalib" + jetkey_tmp + m_currentSyst.name() + "Aux.") );
    }
    return StatusCode::SUCCESS;
  }


  StatusCode SUSYObjDef_xAOD::GetJetsSyst(const xAOD::JetContainer& calibjets, xAOD::JetContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, bool recordSG, const std::string& jetkey)
  {
    if (!m_tool_init) {
      ATH_MSG_ERROR("SUSYTools was not initialized!!");
      return StatusCode::FAILURE;
    }

    std::string jetkey_tmp = jetkey;
    if (jetkey.empty()) {
      jetkey_tmp = m_defaultJets;
    }

    std::pair<xAOD::JetContainer*, xAOD::ShallowAuxContainer*> shallowcopy = xAOD::shallowCopyContainer(calibjets);
    copy = shallowcopy.first;
    copyaux = shallowcopy.second;

    bool setLinks = xAOD::setOriginalObjectLink(calibjets, *copy);
    if (!setLinks) {
      ATH_MSG_WARNING("Failed to set original object links on " << jetkey_tmp);
    }

    // ghost associate the muons to the jets (needed by MET muon-jet OR later)
    ATH_MSG_VERBOSE("Run muon-to-jet ghost association");
    const xAOD::MuonContainer* muons = nullptr;
    // Do a little guessing
    if (jetkey!="AnalysisJets"){
      ATH_CHECK( evtStore()->retrieve(muons, "Muons") );
    } else {
      ATH_CHECK( evtStore()->retrieve(muons, "AnalysisMuons") );
    }
    met::addGhostMuonsToJets(*muons, *copy);

    // Update the jets
    for (const auto jet : *copy) {
      ATH_CHECK( this->FillJet(*jet, false) );
    }

    for (const auto jet : *copy) {
      // Update the JVT decorations if needed
      if( m_doFwdJVT){
        dec_passJvt(*jet) = acc_passFJvt(*jet) && acc_passJvt(*jet);

        //new state for OR   .  0=non-baseline objects, 1=for baseline jets not passing JVT, 2=for any other baseline object
        if ( acc_baseline(*jet) ){
          if( acc_passJvt(*jet) )     dec_selected(*jet) = 2;
          else                        dec_selected(*jet) = 1;
        }
        else{
          dec_selected(*jet) = 0;
        }
      }
      this->IsBadJet(*jet);
      this->IsSignalJet(*jet, m_jetPt, m_jetEta);
      if (!isData())this->IsTruthBJet(*jet);
    }
    if (recordSG) {
      ATH_CHECK( evtStore()->record(copy, "STCalib" + jetkey_tmp + m_currentSyst.name()) );
      ATH_CHECK( evtStore()->record(copyaux, "STCalib" + jetkey_tmp + m_currentSyst.name() + "Aux.") );
    }
    return StatusCode::SUCCESS;
  }

  StatusCode SUSYObjDef_xAOD::FillJet(xAOD::Jet& input, bool doCalib, bool isFat, bool doLargeRdecorations) {

    ATH_MSG_VERBOSE( "Starting FillJet on jet with pt=" << input.pt() );
    ATH_MSG_VERBOSE(  "jet (pt,eta,phi) before calibration " << input.pt() << " " << input.eta() << " " << input.phi() );

    static const SG::ConstAccessor<bool> acc_wValidKinRange(m_WDecorName+"_ValidKinRange");
    static const SG::ConstAccessor<bool> acc_zValidKinRange(m_ZDecorName+"_ValidKinRange");
    static const SG::ConstAccessor<bool> acc_topValidKinRange(m_TopDecorName+"_ValidKinRange");
    static const SG::ConstAccessor<bool> acc_wtagged(m_WDecorName+"_Tagged");
    static const SG::ConstAccessor<bool> acc_ztagged(m_ZDecorName+"_Tagged");
    static const SG::ConstAccessor<bool> acc_toptagged(m_TopDecorName+"_Tagged");

    if (doCalib) {
      if(!isFat){
        //disable - obsoleted ATH_CHECK( m_jetCalibTool->applyCalibration(input) );
      }
      else {
        //disable - obsoleted ATH_CHECK( m_jetFatCalibTool->applyCalibration(input) );
        dec_baseline(input) = ( input.pt() > m_jetPt ) || ( input.pt() > 20e3 ); // Allows for setting m_jetPt < 20e3
        dec_bad(input) = false;
        dec_signal(input) = false;
        dec_bjet_loose(input) = false;
        dec_effscalefact(input) = 1.;
        dec_passOR(input) = true;
        dec_bjet_jetunc(input) = false;
        dec_btag_weight(input) = -999.;

        dec_wtagged(input) = -1;
        dec_ztagged(input) = -1;
        dec_toptagged(input) = -1;
        if (doLargeRdecorations) {

          // Retrieve large-R tagging results for W/Z/top
          if (!m_WtagConfig.empty()) {
            ATH_CHECK(m_WTaggerTool->tag(input));
            // Only tag jets if they are inside the kinematic range
            if ( !acc_wValidKinRange(input) ) {
              ATH_MSG_VERBOSE("Large-R W candidate jet outside of recommended tagging range. Will set score to 0.");
              dec_wtagged(input) = 0;
            }
            else dec_wtagged(input) = acc_wtagged(input);
          }
          if (!m_ZtagConfig.empty()) {
            ATH_CHECK(m_ZTaggerTool->tag(input));
            // Only tag jets if they are inside the kinematic range
            if ( !acc_zValidKinRange(input) ) {
              ATH_MSG_VERBOSE("Large-R Z candidate jet outside of recommended tagging range. Will set score to 0.");
              dec_ztagged(input) = 0;
            }
            else dec_ztagged(input) = acc_ztagged(input);
          }
          if (!m_ToptagConfig.empty()) {
            ATH_CHECK(m_TopTaggerTool->tag(input));
            // Only tag jets if they are inside the kinematic range
            if ( !acc_topValidKinRange(input) ) {
              ATH_MSG_VERBOSE("Large-R Top candidate jet outside of recommended tagging range. Will set score to 0.");
              dec_toptagged(input) = 0;
            }
            else dec_toptagged(input) = acc_toptagged(input);
          }

        }

        // If a user hasn't specified an uncertainty config, then this tool will be empty
        // for large R jets
        if (!m_WTagjetUncertaintiesTool.empty() && !m_WTagUncConfig.empty() && !m_WtagConfig.empty() && doLargeRdecorations) {
          CP::CorrectionCode result = m_WTagjetUncertaintiesTool->applyCorrection(input);
          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to apply largeR W-tag jet scale uncertainties.");
            return StatusCode::FAILURE;
            //break;
                case CP::CorrectionCode::OutOfValidityRange:
                  ATH_MSG_VERBOSE( "No valid pt/eta/m range for largeR W-tag jet scale uncertainties. ");
                  break;
          default:
            break;
          }
        } else {
          ATH_MSG_DEBUG( "No valid large-R W-tagged fat jet uncertainty, but FillJet called with a fat jet. Skipping uncertainties." );
        }

        if (!m_ZTagjetUncertaintiesTool.empty() && !m_ZTagUncConfig.empty() && !m_ZtagConfig.empty() && doLargeRdecorations) {
          CP::CorrectionCode result = m_ZTagjetUncertaintiesTool->applyCorrection(input);
          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to apply largeR Z-tag jet scale uncertainties.");
            return StatusCode::FAILURE;
            //break;
          case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE( "No valid pt/eta/m range for largeR Z-tag jet scale uncertainties. ");
            break;
          default:
            break;
          }
        } else {
          ATH_MSG_DEBUG( "No valid large-R Z-tagged fat jet uncertainty, but FillJet called with a fat jet. Skipping uncertainties." );
        }

        if (!m_TopTagjetUncertaintiesTool.empty() && !m_TopTagUncConfig.empty() && !m_ToptagConfig.empty() && doLargeRdecorations) {
          CP::CorrectionCode result = m_TopTagjetUncertaintiesTool->applyCorrection(input);
          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to apply largeR Top-tag jet scale uncertainties.");
            return StatusCode::FAILURE;
            //break;
          case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE( "No valid pt/eta/m range for largeR Top-tag jet scale uncertainties. ");
            break;
          default:
            break;
          }
        } else {
          ATH_MSG_DEBUG( "No valid large-R Top-tagged fat jet uncertainty, but FillJet called with a fat jet. Skipping uncertainties." );
        }

        if (!m_fatjetUncertaintiesTool.empty() && m_currentSyst.name().find("PseudoData") == std::string::npos) {
          CP::CorrectionCode result = m_fatjetUncertaintiesTool->applyCorrection(input);
          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to apply largeR jet scale uncertainties.");
            return StatusCode::FAILURE;
            //break;
          case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE( "No valid pt/eta/m range for largeR jet scale uncertainties. ");
            break;
          default:
            break;
          }
        } else {
          ATH_MSG_DEBUG( "No valid fat jet uncertainty, but FillJet called with a fat jet. Skipping uncertainties." );
        }

        // Use the PDSmeared uncertainties tool on the systematic with PseudoData in the name
        if (!m_fatjetUncertaintiesPDSmearTool.empty() && m_fatJetUncertaintiesPDsmearing && m_currentSyst.name().find("PseudoData")) {
          CP::CorrectionCode result = m_fatjetUncertaintiesPDSmearTool->applyCorrection(input);
          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to apply largeR jet scale PD uncertainties.");
            return StatusCode::FAILURE;
            //break;
          case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE( "No valid pt/eta/m range for largeR jet scale uncertainties. ");
            break;
          default:
            break;
          }
        } else {
          ATH_MSG_DEBUG( "No valid fat jet uncertainty, but FillJet called with a fat jet. Skipping uncertainties." );
        }

        if (!m_fatjetFFSmearingTool.empty() && std::find(m_fatjetFFSmearingSyst.begin(), m_fatjetFFSmearingSyst.end(), m_currentSyst) != m_fatjetFFSmearingSyst.end()) {
          CP::CorrectionCode result = m_fatjetFFSmearingTool->applyCorrection(input);
          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to apply largeR jet scale uncertainties.");
            return StatusCode::FAILURE;
            //break;
          case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE( "No valid pt/eta/m range for largeR jet scale uncertainties. ");
            break;
          default:
            break;
          }
        } else {
          ATH_MSG_DEBUG( "No valid fat jet uncertainty, but FillJet called with a fat jet. Skipping uncertainties." );
        }

        ATH_MSG_VERBOSE(  "Large-R jet (pt,eta,phi) after calibration " << input.pt() << " " << input.eta() << " " << input.phi() );

        return StatusCode::SUCCESS;
      }
      ATH_MSG_VERBOSE(  "jet (pt,eta,phi) after calibration " << input.pt() << " " << input.eta() << " " << input.phi() );

    }

    dec_passOR(input) = true;
    dec_bjet_jetunc(input) = false;

    if (m_useBtagging) {
      if (m_BtagWP != "Continuous") this->IsBJet(input);
      else this->IsBJetContinuous(input);
    }

   if ( (input.pt() > m_jetPt) || (input.pt() > 15e3) ) {
     if(!isFat && m_currentSyst.name().find("PseudoData") == std::string::npos) {
       // Use the normal jet uncertainties tool for this systematic and do not use the PDSmeared initialised tool
       CP::CorrectionCode result = m_jetUncertaintiesTool->applyCorrection(input);
       switch (result) {
         case CP::CorrectionCode::Error:
           ATH_MSG_ERROR( "Failed to apply JES correction" );
           break;
         case CP::CorrectionCode::OutOfValidityRange:
           ATH_MSG_WARNING( "JES correction OutOfValidity range."); // Jet (pt,eta,phi) = (" << input.pt() << ", " << input.eta() << ", " << input.phi() << ")");
           break;
         default:
                break;
       }
     }
   }

   if (m_jetUncertaintiesPDsmearing) {
     if ( (input.pt() > m_jetPt) || (input.pt() > 15e3) ) {
       if(!isFat && m_currentSyst.name().find("PseudoData")){
         // Use the PDSmeared uncertainties tool on the systematic with PseudoData in the name
         CP::CorrectionCode result = m_jetUncertaintiesPDSmearTool->applyCorrection(input);
         switch (result) {
           case CP::CorrectionCode::Error:
             ATH_MSG_ERROR( "Failed to apply JES correction" );
             break;
           case CP::CorrectionCode::OutOfValidityRange:
             ATH_MSG_WARNING( "JES correction OutOfValidity range."); // Jet (pt,eta,phi) = (" << input.pt() << ", " << input.eta() << ", " << input.phi() << ")");
             break;
           default:
             break;
         }
       }
     }
   }


    ATH_MSG_VERBOSE(  "jet (pt,eta,phi) after JES correction " << input.pt() << " " << input.eta() << " " << input.phi() );

    dec_passJvt(input) = !m_applyJVTCut || m_jetNNJvtSelectionTool->accept(&input);
    dec_passFJvt(input) = !m_doFwdJVT ||  m_jetfJvtSelectionTool->accept(&input);
    dec_baseline(input) = ( input.pt() > m_jetPt ) || ( input.pt() > 20e3 ); // Allows for setting m_jetPt < 20e3
    dec_bad(input) = false;
    dec_signal_less_JVT(input) = false;
    dec_signal(input) = false;
    dec_bjet_loose(input) = false;
    dec_effscalefact(input) = 1.;

    //new state for OR   .  0=non-baseline objects, 1=for baseline jets not passing JVT, 2=for any other baseline object
    if (acc_baseline(input) ){
      if( acc_passJvt(input) ) dec_selected(input) = 2;
      else                     dec_selected(input) = 1;
    }
    else{
      dec_selected(input) = 0;
    }

    if (m_useBtagging && !m_orBtagWP.empty()) {
      dec_bjet_loose(input) = this->IsBJetLoose(input);
    }

    if (m_debug) {
      ATH_MSG_INFO( "JET pt: " << input.pt() );
      ATH_MSG_INFO( "JET eta: " << input.eta() );
      ATH_MSG_INFO( "JET phi: " << input.phi() );
      ATH_MSG_INFO( "JET E: " << input.e() );
      ATH_MSG_INFO( "JET Ceta: " << input.jetP4(xAOD::JetConstitScaleMomentum).eta() );
      ATH_MSG_INFO( "JET Cphi: " << input.jetP4(xAOD::JetConstitScaleMomentum).phi() );
      ATH_MSG_INFO( "JET CE: " << input.jetP4(xAOD::JetConstitScaleMomentum).e() );
      ATH_MSG_INFO( "JET Cm: " << input.jetP4(xAOD::JetConstitScaleMomentum).M() ); // fix-me M

    }

    return StatusCode::SUCCESS;
  }

  StatusCode SUSYObjDef_xAOD::FillTrackJet(xAOD::Jet& input) {

    ATH_MSG_VERBOSE( "Starting FillTrackJet on jet with pt=" << input.pt() );

    dec_btag_weight(input) = -999.;
    dec_effscalefact(input) = 1.;

    if (m_defaultTrackJets == "AntiKtVR30Rmax4Rmin02TrackJets") {
      // VR recommendation
      // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTagCalib2017#Recommendations_for_variable_rad
      dec_baseline(input) = input.pt() >= 5e3 && input.numConstituents() >= 2;
      if (m_trkJetPt < 10e3)
        ATH_MSG_WARNING ("The pt threshold of VR jets you set is: " << m_trkJetPt/1000. << " GeV. But VR jets with pt < 10GeV are uncalibrated.");
      dec_signal(input) = acc_baseline(input) && input.pt() >= m_trkJetPt && std::abs(input.eta()) <= m_trkJetEta;
      dec_VRradius(input) = std::max(0.02,std::min(0.4,30000./input.pt()));
      dec_passDRcut(input) = acc_signal(input);
    } else {
      dec_baseline(input) = input.pt() >= m_trkJetPt && std::abs(input.eta()) <= m_trkJetEta;
      dec_signal(input) = acc_baseline(input);
    }

    if (m_useBtagging_trkJet) {
      if (m_BtagWP_trkJet != "Continuous") this->IsTrackBJet(input);
      else this->IsTrackBJetContinuous(input);
    }

    if (m_debug) {
      ATH_MSG_INFO( "TRK JET pt: " << input.pt() );
      ATH_MSG_INFO( "TRK JET eta: " << input.eta() );
      ATH_MSG_INFO( "TRK JET phi: " << input.phi() );
      ATH_MSG_INFO( "TRK JET E: " << input.e() );
    }

    return StatusCode::SUCCESS;
  }


  bool SUSYObjDef_xAOD::IsBJetLoose(const xAOD::Jet& input) const {
    bool isbjet_loose = false;
    if (m_orBJetPtUpperThres < 0 || m_orBJetPtUpperThres > input.pt())
      isbjet_loose = bool(m_btagSelTool_OR->accept(input)); //note : b-tag applies only to jet with eta < 2.5
    return isbjet_loose;
  }

  bool SUSYObjDef_xAOD::JetPassJVT(xAOD::Jet& input) {
    char pass_jvt = !m_applyJVTCut || m_jetNNJvtSelectionTool->accept(&input);
    dec_passJvt(input) = pass_jvt;
    return pass_jvt;
  }

  bool SUSYObjDef_xAOD::IsSignalJet(const xAOD::Jet& input, float ptcut, float etacut) const {
    if ( !acc_baseline(input)  || !acc_passOR(input) ) return false;

    if ( input.pt() <= ptcut || std::abs(input.eta()) >= etacut) return false;

    bool isgoodjet = !acc_bad(input) && acc_passJvt(input);

    dec_signal(input) = isgoodjet;

    // For JVT calculation
    dec_signal_less_JVT(input) = !acc_bad(input);


    if (m_debug) {
      float emfrac, hecf, LArQuality, HECQuality, Timing,  fracSamplingMax, NegativeE, AverageLArQF;
      std::vector<float> sumpttrk_vec;

      input.getAttribute(xAOD::JetAttribute::EMFrac, emfrac);
      input.getAttribute(xAOD::JetAttribute::HECFrac, hecf);
      input.getAttribute(xAOD::JetAttribute::LArQuality, LArQuality);
      input.getAttribute(xAOD::JetAttribute::HECQuality, HECQuality);
      input.getAttribute(xAOD::JetAttribute::Timing, Timing);
      input.getAttribute(xAOD::JetAttribute::SumPtTrkPt500, sumpttrk_vec);
      input.getAttribute(xAOD::JetAttribute::FracSamplingMax, fracSamplingMax);
      input.getAttribute(xAOD::JetAttribute::NegativeE, NegativeE);
      input.getAttribute(xAOD::JetAttribute::AverageLArQF, AverageLArQF);

      float sumpttrk;
      if (!sumpttrk_vec.empty() && this->GetPrimVtx()) {
        sumpttrk = sumpttrk_vec[this->GetPrimVtx()->index()];
      } else {
        sumpttrk = 0.;
      }

      ATH_MSG_INFO( "JET pt: " << input.pt() );
      ATH_MSG_INFO( "JET eta: " << input.eta() );
      ATH_MSG_INFO( "JET emfrac: " << emfrac );
      ATH_MSG_INFO( "JET hecfrac: " << hecf );
      ATH_MSG_INFO( "JET LArQuality: " << LArQuality );
      ATH_MSG_INFO( "JET HECQuality: " << HECQuality );
      ATH_MSG_INFO( "JET Timing: " << Timing );
      ATH_MSG_INFO( "JET sumpttrk: " << sumpttrk );
      ATH_MSG_INFO( "JET fracSamplingMax: " << fracSamplingMax );
      ATH_MSG_INFO( "JET AverageLArQF: " << AverageLArQF );
    }

    ATH_MSG_VERBOSE( "JET isbad?: " << static_cast<int>(acc_bad(input)));

    return isgoodjet;
  }


  bool SUSYObjDef_xAOD::IsBadJet(const xAOD::Jet& input) const {

    if ( !acc_passOR(input) ) return false;

    float ptcut = 20e3;
    if ( m_jetPt < ptcut ) ptcut = m_jetPt;

    bool  isPileup = !acc_passJvt(input);

    if ( input.pt() <= ptcut || isPileup ) return false;

    if (m_acc_jetClean.isAvailable(input)) {
      dec_bad(input) = !m_acc_jetClean(input);
    } else {
      ATH_MSG_VERBOSE("DFCommon jet cleaning variable not available ... setting 'bad' decorator to 0.");
      dec_bad(input) = 0;
      // the jet cleaning decisions are only calculable at AOD now
      // dec_bad(input) = m_jetCleaningTool.empty() ? false : !m_jetCleaningTool->keep(input);
    }

    ATH_MSG_VERBOSE( "JET isbad?: " << static_cast<int>(acc_bad(input)));

    return acc_bad(input);
  }


  bool SUSYObjDef_xAOD::IsBJet(const xAOD::Jet& input) const {

    bool isbjet = bool(m_btagSelTool->accept(input));
    dec_bjet(input) = isbjet;

    if (SetBtagWeightDecorations(input, m_btagSelTool, m_BtagTagger).isFailure())
       ANA_MSG_ERROR("Couldn't set b-tag decorations for jet, is-b = " << (isbjet?"true":"false") << ", pT = " << input.pt()/1000.);

    return isbjet;
  }

  bool SUSYObjDef_xAOD::IsTrackBJet(const xAOD::Jet& input) const {

    bool isbjet = bool(m_btagSelTool_trkJet->accept(input));
    dec_bjet(input) = isbjet;

    if(SetBtagWeightDecorations(input, m_btagSelTool_trkJet, m_BtagTagger_trkJet).isFailure())
       ANA_MSG_ERROR("Couldn't set b-tag decorations for trackjet, is-b = " << (isbjet?"true":"false") << ", pT = " << input.pt()/1000.);

    return isbjet;
  }

  int SUSYObjDef_xAOD::IsBJetContinuous(const xAOD::Jet& input) const {
    //////////////////////
    // Cheatsheet:
    // returns 6 if between 65% and 0%
    // returns 5 if between 70% and 65%
    // returns 4 if between 77% and 70%
    // returns 3 if between 85% and 77%
    // returns 2 if between 90% and 85%
    // returns 1 if between 100% and 90%
    // returns 0 if smaller than -1e4-> should never happen
    // return -1 if bigger than 1e4 or not in b-tagging acceptance
    //////////////////////

    int isbjet = m_btagSelTool->getQuantile(input);
    dec_bjet(input) = isbjet;

    if(SetBtagWeightDecorations(input, m_btagSelTool, m_BtagTagger).isFailure())
       ANA_MSG_ERROR("Couldn't set continuous b-tag decorations for jet, is-b = " << isbjet << ", pT = " << input.pt()/1000.);

    return isbjet;
  }

  int SUSYObjDef_xAOD::IsTrackBJetContinuous(const xAOD::Jet& input) const {

    int isbjet = m_btagSelTool_trkJet->getQuantile(input);
    dec_bjet(input) = isbjet;

    if(SetBtagWeightDecorations(input, m_btagSelTool_trkJet, m_BtagTagger_trkJet).isFailure())
       ANA_MSG_ERROR("Couldn't set continuous b-tag decorations for trackjet, is-b = " << isbjet << ", pT = " << input.pt()/1000.);

    return isbjet;
  }

  float SUSYObjDef_xAOD::BtagSF(const xAOD::JetContainer* jets) {

    float totalSF = 1.;
    for ( const xAOD::Jet* jet : *jets ) {

      float sf = 1.;

      if ( std::abs(jet->eta()) > 2.5 ) {
        ATH_MSG_VERBOSE( "Trying to retrieve b-tagging SF for jet with |eta|>2.5 (jet eta=" << jet->eta() << "), jet will be skipped");
      } else if ( jet->pt() < 20e3 ){
        ATH_MSG_VERBOSE( "Trying to retrieve b-tagging SF for jet with invalid pt (jet pt=" << jet->pt() << "), jet will be skipped");
      } else {

        CP::CorrectionCode result;
        int truthlabel(-1);
        if (!jet->getAttribute("HadronConeExclTruthLabelID", truthlabel)) {
          ATH_MSG_ERROR("Failed to get jet truth label!");
        }
        ATH_MSG_VERBOSE("This jet is " << (acc_bjet(*jet) ? "" : "not ") << "b-tagged.");
        ATH_MSG_VERBOSE("This jet's truth label is " << truthlabel);

        if ( acc_bjet(*jet) or m_BtagWP == "Continuous") {
          result = m_btagEffTool->getScaleFactor(*jet, sf);

          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to retrieve SF for b-tagged jets in SUSYTools_xAOD::BtagSF" );
            break;
          case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE( "No valid SF for b-tagged jets in SUSYTools_xAOD::BtagSF" );
            break;
          default:
            ATH_MSG_VERBOSE( "Retrieve SF for b-tagged jets in SUSYTools_xAOD::BtagSF with value " << sf );
          }
        } else {

          result = m_btagEffTool->getInefficiencyScaleFactor(*jet, sf);

          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to retrieve SF for non-b-tagged jets in SUSYTools_xAOD::BtagSF" );
            break;
          case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE( "No valid inefficiency SF for non-b-tagged jets in SUSYTools_xAOD::BtagSF" );
            break;
          default:
            ATH_MSG_VERBOSE( "Retrieve SF for non-b-tagged jets in SUSYTools_xAOD::BtagSF with value " << sf );
          }
        }
      }

      dec_effscalefact(*jet) = sf;

      if( acc_signal(*jet) && acc_passOR(*jet) ) totalSF *= sf; //consider goodjets only

    }

    return totalSF;
  }


  float SUSYObjDef_xAOD::BtagSFsys(const xAOD::JetContainer* jets, const CP::SystematicSet& systConfig)
  {
    float totalSF = 1.;

    //Set the new systematic variation
    StatusCode ret = m_btagEffTool->applySystematicVariation(systConfig);
    if ( ret != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("Cannot configure BTaggingEfficiencyTool for systematic var. " << systConfig.name() );
    }

    totalSF = BtagSF( jets );

    ret = m_btagEffTool->applySystematicVariation(m_currentSyst);
    if ( ret != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("Cannot configure BTaggingEfficiencyTool for systematic var. " << systConfig.name() );
    }

    return totalSF;
  }

  float SUSYObjDef_xAOD::BtagSF_trkJet(const xAOD::JetContainer* trkjets) {

    float totalSF = 1.;
    for ( const xAOD::Jet* trkjet : *trkjets ) {

      float sf = 1.;

      if ( std::abs(trkjet->eta()) > 2.5 ) {
        ATH_MSG_VERBOSE( "Trying to retrieve b-tagging SF for trkjet with |eta|>2.5 (trkjet eta=" << trkjet->eta() << "), trkjet will be skipped");
      } else if ( trkjet->pt() < 10e3 ){
        ATH_MSG_VERBOSE( "Trying to retrieve b-tagging SF for trkjet with invalid pt (trkjet pt=" << trkjet->pt() << "), jet will be skipped");
      } else {

        CP::CorrectionCode result;
        int truthlabel(-1);
        if (!trkjet->getAttribute("HadronConeExclTruthLabelID", truthlabel)) {
          ATH_MSG_ERROR("Failed to get jet truth label!");
        }
        ATH_MSG_VERBOSE("This jet is " << (acc_bjet(*trkjet) ? "" : "not ") << "b-tagged.");
        ATH_MSG_VERBOSE("This jet's truth label is " << truthlabel);

        if ( acc_bjet(*trkjet) ) {
          result = m_btagEffTool_trkJet->getScaleFactor(*trkjet, sf);

          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to retrieve SF for b-tagged trk jets in SUSYTools_xAOD::BtagSF_trkJet" );
            break;
          case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE( "No valid SF for b-tagged trk jets in SUSYTools_xAOD::BtagSF_trkJet" );
            break;
          default:
            ATH_MSG_VERBOSE( "Retrieve SF for b-tagged trk jets in SUSYTools_xAOD::BtagSF_trkJet with value " << sf );
          }
        } else {

          result = m_btagEffTool_trkJet->getInefficiencyScaleFactor(*trkjet, sf);

          switch (result) {
          case CP::CorrectionCode::Error:
            ATH_MSG_ERROR( "Failed to retrieve SF for non-b-tagged trk jets in SUSYTools_xAOD::BtagSF_trkJet" );
            break;
          case CP::CorrectionCode::OutOfValidityRange:
            ATH_MSG_VERBOSE( "No valid inefficiency SF for non-b-tagged trk jets in SUSYTools_xAOD::BtagSF_trkJet" );
            break;
          default:
            ATH_MSG_VERBOSE( "Retrieve SF for non-b-tagged trk jets in SUSYTools_xAOD::BtagSF_trkJet with value " << sf );
          }
        }
      }

      dec_effscalefact(*trkjet) = sf;

      if( acc_signal(*trkjet) ) totalSF *= sf;

    }

    return totalSF;
  }


  float SUSYObjDef_xAOD::BtagSFsys_trkJet(const xAOD::JetContainer* trkjets, const CP::SystematicSet& systConfig)
  {
    float totalSF = 1.;

    //Set the new systematic variation
    StatusCode ret = m_btagEffTool_trkJet->applySystematicVariation(systConfig);
    if ( ret != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("Cannot configure BTaggingEfficiencyTool (track jets) for systematic var. " << systConfig.name() );
    }

    totalSF = BtagSF_trkJet( trkjets );

    ret = m_btagEffTool_trkJet->applySystematicVariation(m_currentSyst);
    if ( ret != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("Cannot configure BTaggingEfficiencyTool (track jets) for systematic var. " << systConfig.name() );
    }

    return totalSF;
  }

  double SUSYObjDef_xAOD::JVT_SF(const xAOD::JetContainer* jets) {

    float totalSF = 1.;
    if (!m_applyJVTCut) return totalSF;

    ConstDataVector<xAOD::JetContainer> jvtjets(SG::VIEW_ELEMENTS);
    for (const xAOD::Jet* jet : *jets) {
      // Only jets that were good for every cut except JVT
      if (acc_signal_less_JVT(*jet) && acc_passOR(*jet)) {
        jvtjets.push_back(jet);
      }
    }

    for (const xAOD::Jet* jet : jvtjets) {
      float current_sf = 0;

      // the SF are only applied for HS jets and implicitely requires the presence of the isHS decoration
      CP::CorrectionCode result;
      if (acc_passJvt(*jet)) {
        result = m_jetNNJvtEfficiencyTool->getEfficiencyScaleFactor(*jet,current_sf);
      }
      else {
        result = m_jetNNJvtEfficiencyTool->getInefficiencyScaleFactor(*jet,current_sf);
      }

      switch (result) {
        case CP::CorrectionCode::Error:
          // this is probably not right, should report an error here
          ATH_MSG_ERROR("Inexplicably failed JVT calibration" );
          break;
        case CP::CorrectionCode::OutOfValidityRange:
          // no NNJvt SF for jet, that is ok e.g. for jets with |eta| > 2.5
          ATH_MSG_VERBOSE( "Skip SF application in SUSYTools_xAOD::JVT_SF as jet outside validate range" );
          break;
        default:
          ATH_MSG_VERBOSE( "Retrieve SF for jet in SUSYTools_xAOD::JVT_SF with value " << current_sf );
          totalSF *= current_sf;
      }

    }

    ATH_MSG_VERBOSE( "Retrieve total SF for jet container in SUSYTools_xAOD::JVT_SF with value " << totalSF );

    return totalSF;
  }


  double SUSYObjDef_xAOD::JVT_SFsys(const xAOD::JetContainer* jets, const CP::SystematicSet& systConfig) {

    float totalSF = 1.;
    if (!m_applyJVTCut) return totalSF;

    //Set the new systematic variation
    StatusCode ret = m_jetNNJvtEfficiencyTool->applySystematicVariation(systConfig);
    if ( ret != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("Cannot configure NNjvtEfficiencyTool for systematic var. " << systConfig.name() );
    }

    // Delegate
    totalSF = SUSYObjDef_xAOD::JVT_SF( jets );

    // }
    if (m_applyJVTCut) {
      ret = m_jetNNJvtEfficiencyTool->applySystematicVariation(m_currentSyst);
      if ( ret != StatusCode::SUCCESS) {
        ATH_MSG_ERROR("Cannot configure NNjvtEfficiencyTool for systematic var. " << systConfig.name() );
      }
    }

    return totalSF;
  }

  double SUSYObjDef_xAOD::FJVT_SF(const xAOD::JetContainer* jets) {

    float totalSF = 1.;
    if (!m_doFwdJVT) return totalSF;

    ConstDataVector<xAOD::JetContainer> fjvtjets(SG::VIEW_ELEMENTS);
    for (const xAOD::Jet* jet : *jets) {
      // Only jets that were good for every cut except JVT
      if (acc_signal_less_JVT(*jet) && acc_passOR(*jet)) {
        fjvtjets.push_back(jet);
      }
    }

    for (const xAOD::Jet* jet : fjvtjets) {
      float current_sf = 0;

      // the SF are only applied for HS jets and implicitely requires the presense of the isHS decoration
      CP::CorrectionCode result;
      if (acc_passFJvt(*jet)) {
        result = m_jetfJvtEfficiencyTool->getEfficiencyScaleFactor(*jet,current_sf);
      }
      else {
        result = m_jetfJvtEfficiencyTool->getInefficiencyScaleFactor(*jet,current_sf);
      }

      switch (result) {
        case CP::CorrectionCode::Error:
          // this is probably not right, should report an error here
          ATH_MSG_ERROR("Inexplicably failed fJVT calibration" );
          break;
        case CP::CorrectionCode::OutOfValidityRange:
          // no fJvt SF for jet, that is ok e.g. for jets with |eta| < 2.5
          ATH_MSG_VERBOSE( "Skip SF application in SUSYTools_xAOD::FJVT_SF as jet outside validate range" );
          break;
        default:
          ATH_MSG_VERBOSE( "Retrieve SF for jet in SUSYTools_xAOD::FJVT_SF with value " << current_sf );
          totalSF *= current_sf;
      }

    }

    ATH_MSG_VERBOSE( "Retrieve total SF for jet container in SUSYTools_xAOD::FJVT_SF with value " << totalSF );

    return totalSF;
  }

  double SUSYObjDef_xAOD::FJVT_SFsys(const xAOD::JetContainer* jets, const CP::SystematicSet& systConfig) {

    float totalSF = 1.;
    if (!m_doFwdJVT) return totalSF;

    //Set the new systematic variation
    StatusCode ret = m_jetfJvtEfficiencyTool->applySystematicVariation(systConfig);
    if ( ret != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("Cannot configure fJvtEfficiencyTool for systematic var. " << systConfig.name() );
    }

    // Delegate
    totalSF = SUSYObjDef_xAOD::FJVT_SF( jets );

    if (m_doFwdJVT) {
      ret = m_jetfJvtEfficiencyTool->applySystematicVariation(m_currentSyst);
      if ( ret != StatusCode::SUCCESS) {
        ATH_MSG_ERROR("Cannot configure fJvtEfficiencyTool for systematic var. " << systConfig.name() );
      }
    }

    return totalSF;
  }

  double SUSYObjDef_xAOD::GetTotalJetSF(const xAOD::JetContainer* jets, const bool btagSF, const bool jvtSF, const bool fjvtSF) {

    double totalSF = 1.;
    if (btagSF) totalSF *= BtagSF(jets);

    if (jvtSF && m_applyJVTCut) totalSF *= JVT_SF(jets);

    if (fjvtSF) totalSF *= FJVT_SF(jets);

    return totalSF;
  }


  double SUSYObjDef_xAOD::GetTotalJetSFsys(const xAOD::JetContainer* jets, const CP::SystematicSet& systConfig, const bool btagSF, const bool jvtSF, const bool fjvtSF) {

    double totalSF = 1.;
    if (btagSF) totalSF *= BtagSFsys(jets, systConfig);

    if (jvtSF && m_applyJVTCut) totalSF *= JVT_SFsys(jets, systConfig);

    if (fjvtSF) totalSF *= FJVT_SFsys(jets, systConfig);

    return totalSF;
  }

  StatusCode SUSYObjDef_xAOD::SetBtagWeightDecorations(const xAOD::Jet& input, const asg::AnaToolHandle<IBTaggingSelectionTool>& btagSelTool, const std::string& btagTagger) const {
    double weight = 0.;
    if ( btagSelTool->getTaggerWeight(input, weight, false/*useVetoWP=false*/) != CP::CorrectionCode::Ok ) {
      ATH_MSG_ERROR( btagSelTool->name() << ": could not retrieve b-tag weight (" << btagTagger << ")." );
      return StatusCode::FAILURE;
    }
    dec_btag_weight(input) = weight;
    ATH_MSG_DEBUG( btagSelTool->name() << " b-tag weight: " << weight );

    double btag_pb(-10), btag_pc(-10), btag_pu(-10), btag_ptau(-10);
    // following name change is needed given different name is used for GN2v00 in derivation and in CDI
    std::string actualTagger = btagTagger;
    if (btagTagger == "GN2v00LegacyWP" || btagTagger == "GN2v00NewAliasWP"){
      actualTagger = "GN2v00";
    }
    xAOD::BTaggingUtilities::getBTagging(input)->pb(actualTagger, btag_pb);
    xAOD::BTaggingUtilities::getBTagging(input)->pc(actualTagger, btag_pc);
    xAOD::BTaggingUtilities::getBTagging(input)->pu(actualTagger, btag_pu);
    xAOD::BTaggingUtilities::getBTagging(input)->pu(actualTagger, btag_ptau);
    dec_btag_pb(input) = btag_pb;
    dec_btag_pc(input) = btag_pc;
    dec_btag_pu(input) = btag_pu;
    dec_btag_ptau(input) = btag_ptau;
    ATH_MSG_DEBUG( btagSelTool->name() << " b-tag " << btagTagger << "-type pb:   " << btag_pb );
    ATH_MSG_DEBUG( btagSelTool->name() << " b-tag " << btagTagger << "-type pc:   " << btag_pc );
    ATH_MSG_DEBUG( btagSelTool->name() << " b-tag " << btagTagger << "-type pu:   " << btag_pu );
    ATH_MSG_DEBUG( btagSelTool->name() << " b-tag " << btagTagger << "-type ptau: " << btag_ptau );
    // backwards compatibility
    if ( btagSelTool->name().find("DL1")!=std::string::npos ) {
       dec_btag_dl1pb(input) = btag_pb;
       dec_btag_dl1pc(input) = btag_pc;
       dec_btag_dl1pu(input) = btag_pu;
    }
    else {
       dec_btag_dl1pb(input) = -10;
       dec_btag_dl1pc(input) = -10;
       dec_btag_dl1pu(input) = -10;
    }
    return StatusCode::SUCCESS;
  }
}
