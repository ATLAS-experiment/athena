// -*- C++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// SUSYToolsAlg.h

#ifndef SUSYToolsAlg_H
#define SUSYToolsAlg_H

// Base class
#include "AnaAlgorithm/AnaAlgorithm.h"

// Tool handles
#include "AsgTools/AnaToolHandle.h"

// For SystInfo needs to be included
#include "SUSYTools/ISUSYObjDef_xAODTool.h"
// GRL
#include "AsgAnalysisInterfaces/IGoodRunsListSelectionTool.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAuxContainer.h"


// Timing
#include "TStopwatch.h"
#include "TEfficiency.h"

// Standard library includes
#include <string>
#include <vector>

// Need truth matching for TauJet CP tools 
namespace TauAnalysisTools {
  class ITauTruthMatchingTool;
}

// Define ConstAccessors
namespace STAlg {
  const static SG::ConstAccessor<float> acc_ptvarcone30_TTVA_LooseCone("ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000");
  const static SG::ConstAccessor<float> acc_ptvarcone30_TTVA("ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000");
  const static SG::ConstAccessor<float> acc_topoetcone20("topoetcone20");
  const static SG::ConstAccessor<float> acc_topoetcone40("topoetcone40");
  const static SG::ConstAccessor<float> acc_ptcone20("ptcone20");
  const static SG::ConstAccessor<char> acc_IsTruthMatched("IsTruthMatched");
  const static SG::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> acc_truthParticleLink("truthParticleLink");
  const static SG::ConstAccessor<float> acc_RNNJetScoreSigTrans("RNNJetScoreSigTrans");
}

class SUSYToolsAlg : public EL::AnaAlgorithm {

  public: 

    // Constructor with parameters:
    SUSYToolsAlg(const std::string& name, ISvcLocator* pSvcLocator);

    // Destructor:
    ~SUSYToolsAlg();

    // Athena algorithm's Hooks
    StatusCode  initialize();
    StatusCode  execute();
    StatusCode  finalize();

  private: 

    // Default constructor:
    SUSYToolsAlg();

    // helper functions
    void groupSysts(void);
    StatusCode bookHistograms(void);
    void stdHistsForObj(xAOD::IParticle *obj, const std::string& objtype, const std::string& objlevel, std::map<std::string,std::string> config = std::map<std::string,std::string>());

    // configuration and main tools
    std::vector<std::string> m_GRLFiles;
    std::string m_mcCampaign;
    std::map<std::string,bool> m_slices;
    std::map<std::string,TH1*> m_hists;
    std::map<std::string,TEfficiency*> m_heffs;
    std::map<std::string,std::string> m_configDict;

    asg::AnaToolHandle<ST::ISUSYObjDef_xAODTool> m_SUSYTools;
    asg::AnaToolHandle<IGoodRunsListSelectionTool> m_grl;
    asg::AnaToolHandle<TauAnalysisTools::ITauTruthMatchingTool> m_tauTruthMatchingTool;

    unsigned int m_Nevts; 
    int m_maxEvts;
    int m_lbfilter;
    bool m_isPHYSLITE;
    std::string m_kernel;
    std::string m_configFile;
    std::string m_FatJetCollection;
    std::string m_TrkJetCollection;
    std::string m_TrkJetTimeStamp;

    // systematics
    bool m_doSyst;
    std::vector<ST::SystInfo> m_sysInfoList;
    std::map<std::string,std::vector<std::string>> m_syst_all, m_syst_weights;

    // timing
    TStopwatch m_clock0;
    TStopwatch m_clock1;
    TStopwatch m_clock2;

    // triggers
    std::map<std::string,std::vector<std::string>> m_triggers;

    // histograms and counts
    std::vector<std::string> m_objects;
    std::vector<std::string> m_levels; 
    std::vector<std::string> m_vars;   
    std::map<std::string,std::map<std::string,int>> m_obj_count;

    xAOD::MissingETContainer* m_mettst_syst;
    xAOD::MissingETAuxContainer* m_mettst_syst_aux;
    xAOD::MissingETContainer* m_metcst_syst;
    xAOD::MissingETAuxContainer* m_metcst_syst_aux;

    xAOD::MissingETContainer* m_metcst_nominal;
    xAOD::MissingETAuxContainer* m_metcst_nominal_aux;
    xAOD::MissingETContainer* m_mettst_nominal;
    xAOD::MissingETAuxContainer* m_mettst_nominal_aux;

}; 

#endif
