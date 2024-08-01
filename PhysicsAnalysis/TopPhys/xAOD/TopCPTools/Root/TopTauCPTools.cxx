/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TopCPTools/TopTauCPTools.h"

// Top includes
#include "TopConfiguration/TopConfig.h"
#include "TopEvent/EventTools.h"

// PathResolver include(s):
#include "PathResolver/PathResolver.h"

// Tau include(s):
#include "TauAnalysisTools/TauSelectionTool.h"
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/TauTruthMatchingTool.h"
#include "TauAnalysisTools/Enums.h"

// c++ includes
#include <map>
#include <memory>
#include <string>

namespace top {
  TauCPTools::TauCPTools(const std::string& name) :
    asg::AsgTool(name),
    m_pileupReweightingTool("PileupReweightingTool") {
    declareProperty("config", m_config);

    declareProperty("TauSmearingTool", m_tauSmearingTool);
    declareProperty("TauTruthMatchingTool", m_truthMatchingTool);
  }

  StatusCode TauCPTools::initialize() {
    ATH_MSG_INFO("top::TauCPTools initialize...");

    if (m_config->isTruthDxAOD()) {
      ATH_MSG_INFO("top::TauCPTools: no need to initialise anything on truth DxAOD");
      return StatusCode::SUCCESS;
    }

    if (!m_config->useTaus()) {
      ATH_MSG_INFO("top::TauCPTools: no need to initialise anything since not using taus");
      return StatusCode::SUCCESS;
    }

    if (m_config->makeAllCPTools()) {// skiping calibrations on mini-xAODs
      top::check(setupCalibration(), "Failed to setup Tau calibration tools");
    }
    if (m_config->isMC()) {// scale-factors are only for MC
      top::check(setupScaleFactors(), "Failed to setup Tau scale-factor tools");
    }
    return StatusCode::SUCCESS;
  }

  StatusCode TauCPTools::setupCalibration() {
    /************************************************************
    *
    * Setup the tools needed for Tau Analysis
    * ----------------------------------------
    *
    * TauSelectionTool:
    *     to select taus
    * TauEfficiencyCorrectionTool:
    *     to apply efficiency corrections
    *     (this is setup with the selection tool)
    * TauSmearingTool:
    *     to calibrate and smear taus
    *
    ************************************************************/

    //============================================================
    // Convert tau jet IDWP string to int
    // Updated: using enums from TauAnalysisTools package
    //============================================================

    auto tau_JetIDWP_to_enum = [](const std::string& s) {
      if (s == "None") return TauAnalysisTools::JETIDNONE;
      if (s == "RNNLoose") return TauAnalysisTools::JETIDRNNLOOSE;
      if (s == "RNNMedium") return TauAnalysisTools::JETIDRNNMEDIUM;
      if (s == "RNNTight") return TauAnalysisTools::JETIDRNNTIGHT;
      // If we haven't found the correct WP, then return 0
      return TauAnalysisTools::JETIDNONEUNCONFIGURED;
    };

    // convert taujetIDWP from string to int and check it's valid
    const int tauJetIDWP = tau_JetIDWP_to_enum(m_config->tauJetIDWP());

    top::check(tauJetIDWP > 0, m_config->tauJetIDWP() + " is not a supported tau Jet ID WP");

    // convert taujetIDWPLoose from string to int and check it's valid
    const int tauJetIDWPLoose = tau_JetIDWP_to_enum(m_config->tauJetIDWPLoose());
    top::check(tauJetIDWPLoose > 0, m_config->tauJetIDWPLoose() + " is not a supported tau Jet ID WP");

    auto tau_EleIDWP_to_enum = [](const std::string& s) {
      if (s == "None") return TauAnalysisTools::ELEIDNONE;
      if (s == "RNNLoose") return TauAnalysisTools::ELEIDRNNLOOSE;
      if (s == "RNNMedium") return TauAnalysisTools::ELEIDRNNMEDIUM;
      if (s == "RNNTight") return TauAnalysisTools::ELEIDRNNTIGHT;
      // If we haven't found the correct WP, then return 0
      return TauAnalysisTools::ELEIDNONEUNCONFIGURED;
    };

    // convert tauEleIDWP from string to int and check it's valid
    const int tauEleIDWP = tau_EleIDWP_to_enum(m_config->tauEleIDWP());
    top::check(tauEleIDWP > 0, m_config->tauEleIDWP() + " is not a supported tau electron ID WP");

    // convert tauEleIDWPLoose from string to int and check it's valid
    const int tauEleIDWPLoose = tau_EleIDWP_to_enum(m_config->tauEleIDWPLoose());
    top::check(tauEleIDWPLoose > 0, m_config->tauEleIDWPLoose() + " is not a supported tau electron ID WP");

    const double absCharge = 1.;
    const std::vector<size_t> nTracks = {
      1, 3
    };

    //============================================================
    // PathResolve tau config files. If set to default then
    // set to empty string.
    //============================================================

    std::string tauJetConfigFile = m_config->tauJetConfigFile();
    if (tauJetConfigFile != "Default")
      tauJetConfigFile = PathResolverFindCalibFile(tauJetConfigFile);
    else
      tauJetConfigFile.clear();
    std::string tauJetConfigFileLoose = m_config->tauJetConfigFileLoose();
    if (tauJetConfigFileLoose != "Default")
      tauJetConfigFileLoose = PathResolverFindCalibFile(tauJetConfigFileLoose);
    else
      tauJetConfigFileLoose.clear();

    // bitmap in case not using the config files
    //WARNING: if we add more configurable cuts, they need to be added in this list
    // a priori we alway apply these cuts below
    int iSelectionCuts = 0;
    iSelectionCuts |= TauAnalysisTools::CutPt;
    iSelectionCuts |= TauAnalysisTools::CutAbsEta;
    iSelectionCuts |= TauAnalysisTools::CutAbsCharge;
    iSelectionCuts |= TauAnalysisTools::CutNTrack;
    iSelectionCuts |= TauAnalysisTools::CutJetIDWP;
    iSelectionCuts |= TauAnalysisTools::CutEleIDWP;

    int iSelectionCutsLoose = iSelectionCuts;

    //============================================================
    // Nominal Tau Selection
    //============================================================


    // Alias long tool names
    using ITauSelTool = TauAnalysisTools::ITauSelectionTool;
    using ITauEffCorrTool = TauAnalysisTools::ITauEfficiencyCorrectionsTool;
    using ITauSmearTool = TauAnalysisTools::ITauSmearingTool;
    using ITauTruthMatchTool = TauAnalysisTools::ITauTruthMatchingTool;
    using TauSelTool = TauAnalysisTools::TauSelectionTool;
    using TauEffCorrTool = TauAnalysisTools::TauEfficiencyCorrectionsTool;
    using TauSmearTool = TauAnalysisTools::TauSmearingTool;
    using TauTruthMatchTool = TauAnalysisTools::TauTruthMatchingTool;

    // Names of tools here for a little clarity
    const std::string tauSelName = "TauAnalysisTools::TauSelectionTool";
    const std::string tauEffCorrName = "TauAnalysisTools::TauEfficiencyCorrectionsTool";

    ///-- Setup the tau selection tool --///
    if (asg::ToolStore::contains<ITauSelTool>(tauSelName)) {
      m_tauSelectionTool = asg::ToolStore::get<ITauSelTool>(tauSelName);
    } else {
      std::unique_ptr<ITauSelTool> tauSelectionTool = std::make_unique<TauAnalysisTools::TauSelectionTool>(tauSelName);
      if (!tauJetConfigFile.empty()) {
        top::check(asg::setProperty(tauSelectionTool, "ConfigPath", tauJetConfigFile),
                   "Failed to set tau selection tool configuration path");
        top::check(asg::setProperty(tauSelectionTool, "OutputLevel", MSG::INFO),
                   "Failed to set tau OutputLevel");
      } else {
        // set the ConfigPath to empty value is no file was provided (otherwise it creates conflicts)
        top::check(asg::setProperty(tauSelectionTool, "ConfigPath", ""),
                   "Failed to set tau selection tool configuration path");
        top::check(asg::setProperty(tauSelectionTool, "OutputLevel", MSG::INFO),
                   "Failed to set tau OutputLevel");
        //WARNING: if we add more configurable cuts, they need to be added in the iSelectionCuts (see above)
        top::check(asg::setProperty(tauSelectionTool, "SelectionCuts", iSelectionCuts),
                   "Failed to set tau SelectionCuts");
        top::check(asg::setProperty(tauSelectionTool, "PtMin", static_cast<double>(m_config->tauPtcut() / 1e3)),
                   "Failed to set tau pT cut");
        top::check(asg::setProperty(tauSelectionTool, "AbsEtaRegion", m_config->tauEtaRegions() ),
                   "Failed to set tau AbsEtaRegion");
        top::check(asg::setProperty(tauSelectionTool, "AbsCharge", absCharge),
                   "Failed to set tau AbsCharge");
        top::check(asg::setProperty(tauSelectionTool, "NTracks", nTracks),
                   "Failed to set tau NTracks");
        top::check(asg::setProperty(tauSelectionTool, "JetIDWP", tauJetIDWP),
                   "Failed to set tau JetIDWP");
        top::check(asg::setProperty(tauSelectionTool, "EleIDWP", tauEleIDWP),
		   "Failed to set tau EleIDWP");
        top::check(asg::setProperty(tauSelectionTool, "MuonOLR", m_config->tauMuOLR()),
                   "Failed to set tau MuonOLR");
      }
      top::check(tauSelectionTool->initialize(), "Failed to initialize tauSelectionTool");
      m_tauSelectionTool = tauSelectionTool.release();
    }

    ///-- Setup the EfficiencyCorrectionsTool ///
    if (asg::ToolStore::contains<ITauEffCorrTool>(tauEffCorrName)) {
      m_tauEffCorrTool = asg::ToolStore::get<ITauEffCorrTool>(tauEffCorrName);
    } else {
      std::unique_ptr<ITauEffCorrTool> tauEffCorrTool
        = std::make_unique<TauAnalysisTools::TauEfficiencyCorrectionsTool>(tauEffCorrName);
      if (m_config->isMC()) {

        if(!m_config->isDataOverlay()){
            top::check(m_pileupReweightingTool.retrieve(), "Failed to retireve pileup reweighting tool");
            top::check(asg::setProperty(tauEffCorrTool, "PileupReweightingTool", m_pileupReweightingTool),
                    "Failed to set PileupReweightingTool for " + tauEffCorrName);
        }

        top::check(asg::setProperty(tauEffCorrTool, "UseTauSubstructure", m_config->tauSubstructureSF()),
                    "Failed to set UseTauSubstructure for " + tauEffCorrName);
      }

      top::check(asg::setProperty(tauEffCorrTool, "JetIDLevel", tauJetIDWP),
                 "Failed to set JetIDLevel for " + tauEffCorrName);
      top::check(asg::setProperty(tauEffCorrTool, "EleIDLevel", tauEleIDWP),
                 "Failed to set EleIDLevel for " + tauEffCorrName);

      // Set correction types directly instead of via TauSelectionTool. See athena !70515
      // Determine Tau efficiency correction types
      std::vector<int> vEfficiencyCorrectionTypes = getTauEfficiencyCorrectionTypes(
        iSelectionCuts, tauJetIDWP, tauEleIDWP
      );
      top::check(asg::setProperty(tauEffCorrTool, "EfficiencyCorrectionTypes", vEfficiencyCorrectionTypes),
                 "Failed to set EfficiencyCorrectionTypes for " + tauEffCorrName);

      top::check(tauEffCorrTool->initialize(), "Failed to initialize");
      m_tauEffCorrTool = tauEffCorrTool.release();
    }

    //============================================================
    // 'Loose' Tau Selection
    //============================================================

    ///-- Names of tools here for a little clarity --///
    const std::string tauSelNameLoose = "TauAnalysisTools::TauSelectionToolLoose";
    const std::string tauEffCorrNameLoose = "TauAnalysisTools::TauEfficiencyCorrectionsToolLoose";

    ///-- Setup the tau selection tool --///
    if (asg::ToolStore::contains<ITauSelTool>(tauSelNameLoose)) {
      m_tauSelectionToolLoose = asg::ToolStore::get<ITauSelTool>(tauSelNameLoose);
    } else {
      std::unique_ptr<ITauSelTool> tauSelectionTool = std::make_unique<TauSelTool>(tauSelNameLoose);
      if (!tauJetConfigFileLoose.empty()) {
        top::check(asg::setProperty(tauSelectionTool, "ConfigPath", tauJetConfigFileLoose),
                   "Failed to set tau selection tool configuration path");
        top::check(asg::setProperty(tauSelectionTool, "OutputLevel", MSG::INFO),
                   "Failed to set tau OutputLevel");
      } else {
        // set the ConfigPath to empty value is no file was provided (otherwise it creates conflicts)
        top::check(asg::setProperty(tauSelectionTool, "ConfigPath", ""),
                   "Failed to set tau selection tool configuration path");
        top::check(asg::setProperty(tauSelectionTool, "OutputLevel", MSG::INFO),
                   "Failed to set tau OutputLevel");
        //WARNING: if we add more configurable cuts, they need to be added in the iSelectionCuts (see above)
        top::check(asg::setProperty(tauSelectionTool, "SelectionCuts", iSelectionCutsLoose),
                   "Failed to set tau SelectionCuts");
        top::check(asg::setProperty(tauSelectionTool, "PtMin", static_cast<double>(m_config->tauPtcut() / 1e3)),
                   "Failed to set tau pT cut");
        top::check(asg::setProperty(tauSelectionTool, "AbsEtaRegion", m_config->tauEtaRegions()),
                   "Failed to set loose tau AbsEtaRegion");
        top::check(asg::setProperty(tauSelectionTool, "AbsCharge", absCharge),
                   "Failed to set loose tau AbsCharge");
        top::check(asg::setProperty(tauSelectionTool, "NTracks", nTracks),
                   "Failed to set loose tau NTracks");
        top::check(asg::setProperty(tauSelectionTool, "JetIDWP", tauJetIDWPLoose),
                   "Failed to set loose tau JetIDWP");
        top::check(asg::setProperty(tauSelectionTool, "EleIDWP", tauEleIDWPLoose),
		   "Failed to set loose tau EleIDWP");
        top::check(asg::setProperty(tauSelectionTool, "MuonOLR", m_config->tauMuOLRLoose()),
                   "Failed to set tau MuonOLR");
      }
      top::check(tauSelectionTool->initialize(), "Failed to initialize tauSelectionTool");
      m_tauSelectionToolLoose = tauSelectionTool.release();
    }

    ///-- Setup the EfficiencyCorrectionsTool ///
    if (asg::ToolStore::contains<ITauEffCorrTool>(tauEffCorrNameLoose)) {
      m_tauEffCorrTool = asg::ToolStore::get<ITauEffCorrTool>(tauEffCorrNameLoose);
    } else {
      std::unique_ptr<ITauEffCorrTool> tauEffCorrTool
        = std::make_unique<TauEffCorrTool>(tauEffCorrNameLoose);
      if (m_config->isMC()) {

        if(!m_config->isDataOverlay()) {
           top::check(m_pileupReweightingTool.retrieve(), "Failed to retireve pileup reweighting tool");
           top::check(asg::setProperty(tauEffCorrTool, "PileupReweightingTool", m_pileupReweightingTool),
                     "Failed to set PileupReweightingTool for " + tauEffCorrNameLoose);
        }

        top::check(asg::setProperty(tauEffCorrTool, "UseTauSubstructure", m_config->tauSubstructureSFLoose()),
                   "Failed to set UseTauSubstructure for " + tauEffCorrNameLoose);
      }

      top::check(asg::setProperty(tauEffCorrTool, "JetIDLevel", tauJetIDWPLoose),
                 "Failed to set JetIDLevel for " + tauEffCorrNameLoose);
      top::check(asg::setProperty(tauEffCorrTool, "EleIDLevel", tauEleIDWPLoose),
                 "Failed to set EleIDLevel for " + tauEffCorrNameLoose);

      // Set correction types directly instead of via TauSelectionTool. See athena !70515
      // Determine Tau efficiency correction types
      std::vector<int> vEfficiencyCorrectionTypesLoose = getTauEfficiencyCorrectionTypes(
        iSelectionCutsLoose, tauJetIDWPLoose, tauEleIDWPLoose
      );
      top::check(asg::setProperty(tauEffCorrTool, "EfficiencyCorrectionTypes", vEfficiencyCorrectionTypesLoose),
                 "Failed to set EfficiencyCorrectionTypes for " + tauEffCorrNameLoose);

      top::check(tauEffCorrTool->initialize(), "Failed to initialize");
      m_tauEffCorrToolLoose = tauEffCorrTool.release();
    }

    ///-- Calibration and smearing --///
    static const std::string tauSmearName = "TauSmearingTool";
    if (asg::ToolStore::contains<ITauSmearTool>(tauSmearName)) {
      m_tauSmearingTool = asg::ToolStore::get<ITauSmearTool>(tauSmearName);
    } else {
      std::unique_ptr<TauSmearTool> tauSmearingTool = std::make_unique<TauSmearTool>(tauSmearName);
      top::check(tauSmearingTool->initialize(), "Failed to initialize");
      m_tauSmearingTool = tauSmearingTool.release();
    }
    
    ///-- Truth matching --///
    if (m_config->isMC()) {
      static const std::string tauTruthMatchingName = "TauAnalysisTools::TauTruthMatchingTool";
      if (asg::ToolStore::contains<ITauTruthMatchTool>(tauTruthMatchingName)) {
        m_truthMatchingTool = asg::ToolStore::get<ITauTruthMatchTool>(tauTruthMatchingName);
      } else {
        std::unique_ptr<TauTruthMatchTool> tauMatchingTool = std::make_unique<TauTruthMatchTool>(tauTruthMatchingName);
        top::check(tauMatchingTool->setProperty("TruthJetContainerName", "AntiKt4TruthDressedWZJets"), "Failed to set truth collection for tau truth matching tool");
        top::check(tauMatchingTool->initialize(), "Failed to initialize");
        m_truthMatchingTool = tauMatchingTool.release();
      }
    }


    return StatusCode::SUCCESS;
  }

  StatusCode TauCPTools::setupScaleFactors() {
    /**
     *
     * Fill this in at some point...
     *
     * ...or maybe we don't need to
     *
     **/
    return StatusCode::SUCCESS;
  }

  std::vector<int> TauCPTools::getTauEfficiencyCorrectionTypes(int iSelectionCuts, int iJetIDLevel, int iEleIDLevel) {
    std::vector<int> effCorrections;

    bool addedJetIDSF {false};
    bool addedEleIDSF {false};

    // add reco systematics in any case
    effCorrections.push_back(TauAnalysisTools::SFRecoHadTau);
    ATH_MSG_VERBOSE( "added SFRecoHadTau" );

    // use jet ID scale factors if TauSelectionTool applies jet ID cut
    if (iSelectionCuts & TauAnalysisTools::CutJetIDWP) {
      // add SFJetIDHadTau
      if (iJetIDLevel == TauAnalysisTools::JETIDRNNLOOSE or iJetIDLevel == TauAnalysisTools::JETIDRNNMEDIUM or iJetIDLevel == TauAnalysisTools::JETIDRNNTIGHT) {
        effCorrections.push_back(TauAnalysisTools::SFJetIDHadTau);
        ATH_MSG_VERBOSE( "added SFJetIDHadTau" );
        addedJetIDSF = true;
      } else {
        ATH_MSG_WARNING( "No JetID scale factor for working point " << iJetIDLevel );
      }
    }

    // use electron veto scale factors if applying electron veto
    if (iSelectionCuts & TauAnalysisTools::CutEleIDWP) {

      // force m_iEleIDLevel to ELEIDNONE until RNN eVeto SFs are available
      if (iEleIDLevel == TauAnalysisTools::ELEIDRNNTIGHT) {
        ATH_MSG_WARNING("Scale factors for RNN eVeto (TIGHT) are not available yet");
        iEleIDLevel = TauAnalysisTools::ELEIDNONE;
      }

      if (iEleIDLevel == TauAnalysisTools::ELEIDRNNLOOSE || iEleIDLevel == TauAnalysisTools::ELEIDRNNMEDIUM || iEleIDLevel == TauAnalysisTools::ELEIDRNNTIGHT) {
        effCorrections.push_back(TauAnalysisTools::SFEleIDHadTau);
        ATH_MSG_VERBOSE( "added SFEleIDHadTau" );
        effCorrections.push_back(TauAnalysisTools::SFEleIDElectron);
        ATH_MSG_VERBOSE( "added SFEleIDElectron" );
        addedEleIDSF = true;
      }
    }

    if (addedJetIDSF and addedEleIDSF) {
      ATH_MSG_WARNING("Both Tau JetID and EleID corrections are applied. The final SF is a product of TauID and EleID SFs, which is not correct. Need to have two TauEfficiencyCorrectionsTool instances to handle them separately!");
    }

    if (m_config->tauSubstructureSF()) {
      ATH_MSG_WARNING("Scale factors for decay mode ID are not available yet");
      //effCorrections.push_back(TauAnalysisTools::SFDecayModeHadTau);
    }

    return effCorrections;
  }
}  // namespace top
