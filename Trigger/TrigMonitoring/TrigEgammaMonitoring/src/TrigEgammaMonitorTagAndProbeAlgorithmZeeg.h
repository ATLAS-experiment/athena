/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TrigEgammaMonitorTagAndProbeAlgorithmZeeg_H
#define TrigEgammaMonitorTagAndProbeAlgorithmZeeg_H

#include "TrigEgammaMonitorAnalysisAlgorithm.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "TriggerMatchingTool/IMatchingTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigCompositeUtils/ChainNameParser.h"
#include <sstream>






class TrigEgammaMonitorTagAndProbeAlgorithmZeeg: public TrigEgammaMonitorAnalysisAlgorithm 
{

  public:

    TrigEgammaMonitorTagAndProbeAlgorithmZeeg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~TrigEgammaMonitorTagAndProbeAlgorithmZeeg() override;

    virtual StatusCode initialize() override;
    
    virtual StatusCode fillHistograms( const EventContext& ctx) const override;


  protected:

    /*! Tag and Probe method called by derived classes */
    bool executeTandP( const EventContext& ctx, std::vector<std::shared_ptr<const xAOD::Photon>> &, std::vector<std::pair<TLorentzVector, TLorentzVector>> &tagPairs) const;

    /*! Match probes called by derived classes */
    void matchObjects(const std::string& trigItem,  std::vector<std::shared_ptr<const xAOD::Photon>>&,
                      std::vector<std::pair<const xAOD::Egamma*, const TrigCompositeUtils::Decision*>> &) const;

    void matchObjectsR3(const std::string& trigItem,const xAOD::IParticleContainer& particles,
                      std::vector<std::pair<const xAOD::Egamma*, const TrigCompositeUtils::Decision*>>& matches) const;

    std::vector<ChainNameParser::LegInfo> 
    getProbeTriggerLeg(const std::string& triggerName) const;

    std::string  createProbeTrigger(const std::vector<ChainNameParser::LegInfo>& LegParts, const std::string& signature) const;

    
    /*! List of triggers from menu after filter*/
    std::vector<std::string> m_trigList;
    std::vector<std::string> m_tagList;
    

  private:

    /*! at least one chain should pass. e28_tight_iloose? */
    bool minimalTriggerRequirement () const;

    /*! Tag Electron selection */
    bool isTagElectron(const EventContext& ctx, const ToolHandle<GenericMonitoringTool>& monGroup, const xAOD::Electron *el) const;

    /*! Di-electron selection */
    bool isGoodElectron(const ToolHandle<GenericMonitoringTool>& monGroup,const xAOD::Electron *el) const;
    bool matchDiElectronTrigger(const xAOD::Electron* e1, const xAOD::Electron* e2) const;

    /*! Probe selection */
    bool isGoodProbePhoton( const ToolHandle<GenericMonitoringTool>& monGroup, const xAOD::Photon *phot, const xAOD::JetContainer *) const;
    std::string extractElectronLeg(const std::string& chain) const;
    //Zeeg tag and probe trigger matching index
    int m_triggers_for_matching_index = -1;



    /** Properties **/
    ToolHandle<TrigEgammaMatchingToolMT> m_trigger_matching_tool{this, "TriggerMatchingTool", "TrigEgammaMatchingToolMT/TrigEgammaMatchingToolMT", "Tool to match HLT objects"};



    ToolHandle<Trig::TrigDecisionTool> m_trigger_decision_tool;






    Gaudi::Property< std::map<std::string, std::vector<std::string>> > m_TPMatchingMap{ this, "TPMatchingMap", {}, "..." };
    /*! List of triggers from menu (Probe) */
    /*! List of triggers from menu (Probe) */
    Gaudi::Property<std::vector<std::string>> m_trigInputList{this, "ProbeTriggerList", {}};
    /*! Tag trigger list (Tag)*/
    Gaudi::Property<std::vector<std::string>> m_tagTrigList{ this, "TagTriggerList", {}};
    Gaudi::Property<std::vector<int>> m_probeTriggerIndex{ this, "ProbeTriggerIndex", {}};

    /*! Zee lower mass cut */
    Gaudi::Property<float> m_ZeeMassMin{ this, "ZeeLowerMass", 80};
    /*! Zee upper mass cut */
    Gaudi::Property<float> m_ZeeMassMax{ this, "ZeeUpperMass", 100};
    /*! Define the PID for tag electron */
    //Gaudi::Property<std::string> m_offTagTightness{ this, "OfflineTagSelector", "Tight"};
    Gaudi::Property<std::string> m_offTagTightness{ this, "OfflineTagSelector", "Loose"};
    /*! define the Pid of Probe from the user */
    Gaudi::Property<std::string> m_offProbeTightness{ this, "OfflineProbeSelector", "Loose"};
    /*! Select opposite or same-sign pairs -- for background studies */
    Gaudi::Property<bool> m_oppositeCharge{ this, "OppositeCharge", true};
    /*! Minimum tag Et */
    Gaudi::Property<float> m_tagMinEt{ this, "OfflineTagMinEt", 25};
    /*! Minimum probe Et */
    Gaudi::Property<float> m_probeMinEt{this, "OfflineProbeMinEt", 4};
    /*! Probe isolation */
    Gaudi::Property<std::string> m_offProbeIsolation{ this, "OfflineProbeIsolation", "Loose"};
    /*! Remove crack region for Probe default True */
    Gaudi::Property<bool> m_rmCrack{this, "RemoveCrack", true};
    /*! Enable the requirement of triggers */
    Gaudi::Property<bool> m_applyMinimalTrigger{this, "ApplyMinimalTrigger", true};
    /*! Apply nearby jet selection */
    Gaudi::Property<bool> m_applyJetNearProbeSelection{this, "ApplyJetNearProbeSelection", true};
    /*! do jpsiee tag and probe */
    Gaudi::Property<bool> m_doJpsiee{this,"DoJpsiee", false};
    /*! analysis name */
    Gaudi::Property<std::string> m_anatype{ this, "Analysis","Zeeg"};
    /*! Event Wise offline ElectronContainer Access and end iterator */
    SG::ReadHandleKey<xAOD::ElectronContainer> m_offElectronKey{ this, "ElectronKey", "Electrons", ""};
    /*! Jet container for probe selection */
    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey{ this, "JetKey" , "AntiKt4EMPFlowJets", ""};
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_eventInfoDecorKey{this, "LArStatusFlag", "EventInfo.larFlags", "Key for EventInfo object"};
    /*! Ensure offline electron isolation decoration is retrieved after being created */
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_electronIsolationKeyPtCone20 { this, "PtCone20Decoration", m_offElectronKey, "ptcone20", "Decoration key for the ptcone20 isolation decoration" };
    SG::ReadHandleKey<xAOD::PhotonContainer> m_photonsKey{this, "PhotonKey", "Photons", ""};
    SG::ReadHandleKey<xAOD::PhotonContainer> m_hltPhotonsKey {this,"HLTPhotons","HLT_egamma_Photons","Key for HLT photon container"};
    SG::ReadHandleKeyArray<xAOD::PhotonContainer> m_offPhotonIsolationKeys {this, "PhotonIsolationKeys", {"Photons.topoetcone20", "Photons.topoetcone40"} }; 
};

#endif

