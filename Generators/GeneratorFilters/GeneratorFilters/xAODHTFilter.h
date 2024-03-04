/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODHTFilter_H
#define GENERATORFILTERS_XAODHTFilter_H

#include "GeneratorModules/GenFilter.h"
#include <string>
#include "GaudiKernel/ServiceHandle.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthMetaDataContainer.h"
// Defs for the particle origin
#include "MCTruthClassifier/IMCTruthClassifier.h"

#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

class MsgStream;
class StoreGateSvc;


class xAODHTFilter:public GenFilter {

public:

    xAODHTFilter(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~xAODHTFilter();
    virtual StatusCode filterInitialize();
    virtual StatusCode filterFinalize();
    virtual StatusCode filterEvent();

private:

    double m_MinJetPt;  //!< Min pT for the truth jets
    double m_MaxJetEta; //!< Max eta for the truth jets
    double m_MinHT;  //!< Min HT for events
    double m_MaxHT;  //!< Max HT for events
    double m_MinLepPt;  //!< Min pT for the truth jets
    double m_MaxLepEta; //!< Max eta for the truth jets
    bool   m_UseNu;  //!< Use neutrinos in HT
    bool   m_UseLep;   //!< Use leptons in HT

    std::string m_TruthJetContainerName;  //!< Name of the truth jet container
    std::string m_eventInfoName;

    long m_total;    //!< Total number of events tested
    long m_passed;   //!< Number of events passing all cuts
    long m_ptfailed; //!< Number of events failing the pT cuts 
       
    ToolHandle<IMCTruthClassifier> m_classif;

  SG::WriteDecorHandleKey<xAOD::EventInfo> m_mcFilterHTKey {this
      , "mcFilterHTKey"
      , "TMPEvtInfo.mcFilterHTKey"
      , "Decoration for MC Filter HT"};

};

#endif
