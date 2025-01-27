/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigEgammaMonitorPhotonAlgorithm_H
#define TrigEgammaMonitorPhotonAlgorithm_H


#include "TrigEgammaMonitorAnalysisAlgorithm.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/ReadDecorHandleKey.h"


class TrigEgammaMonitorPhotonAlgorithm: public TrigEgammaMonitorAnalysisAlgorithm 
{

  public:

    TrigEgammaMonitorPhotonAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~TrigEgammaMonitorPhotonAlgorithm() override;

    virtual StatusCode initialize() override;
    
    virtual StatusCode fillHistograms( const EventContext& ctx) const override;

 
  
  private:


    /*! navigation method called by executeNavigation */
    StatusCode executeNavigation(const EventContext& ctx, const TrigInfo& info,
                                 std::vector<std::pair<std::shared_ptr<const xAOD::Egamma>, const TrigCompositeUtils::Decision*>> &) const;
  
    /*! List of triggers to study */
    std::vector<std::string> m_trigList;
    /*! Photon pid word */
    Gaudi::Property<std::string> m_photonPid{this, "PhotonPid","Tight" };
    /*! doUnconverted analysis */
    Gaudi::Property<bool> m_doUnconverted{this, "DoUnconverted", false};
     /*! List of triggers from menu */
    Gaudi::Property<std::vector<std::string>> m_trigInputList{this, "TriggerList", {}};


    // Stores a map of Bootstrap triggers
    Gaudi::Property< std::map<std::string, std::string> > m_BSTrigMap{this, "BootstrapTriggerMap", {{"",""}}, "Dictionary of Triggers (as keys) and Bootstraps (value) to configure Bootstrapping of photon triggers" };
    /*! Directory name for each algorithm */
    Gaudi::Property<std::string> m_anatype{this, "Analysis", "Photon"};
    /*! force pid selection into photon navigation */
    Gaudi::Property<bool> m_forcePidSelection{ this, "ForcePidSelection", true};
    /*! Compute only final HLT decision*/
    Gaudi::Property<bool> m_onlyHLT{ this, "OnlyHLT", false};
 
    // Containers 
    /*! Event Wise offline PhotonContainer Access and end iterator */
    SG::ReadHandleKey<xAOD::PhotonContainer> m_offPhotonKey{ this, "PhotonKey", "Photons", ""};
    /*! Ensure offline photon isolation decoration is retrieved after being created */
    SG::ReadDecorHandleKeyArray<xAOD::PhotonContainer> m_offPhotonIsolationKeys {this, "PhotonIsolationKeys", {"Photons.topoetcone20", "Photons.topoetcone40"} };
   SG::ReadDecorHandleKey<xAOD::EventInfo> m_eventInfoDecorKey{this, "LArStatusFlag", "EventInfo.larFlags", "Key for EventInfo object"}; //To get data-dependencies right
};

#endif
