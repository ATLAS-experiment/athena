#ifndef IPPerformance_EventSelectorAlg_H
#define IPPerformance_EventSelectorAlg_H
#include <EventBookkeeperTools/FilterReporterParams.h>
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include <AsgAnalysisInterfaces/IGoodRunsListSelectionTool.h>

// Core include(s):
#include <AnaAlgorithm/AnaAlgorithm.h>
#include "GoodRunsLists/GoodRunsListSelectionTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "PileupReweighting/PileupReweightingTool.h"
#include "xAODTracking/VertexContainer.h"
#include <AsgTools/PropertyWrapper.h>
#include "StoreGate/StoreGateSvc.h"
#include "xAODEventInfo/EventInfo.h"
// ROOT include(s):
#include "TH1D.h"

#include "AthenaBaseComps/AthAlgorithm.h"

// Local include(s):
#include "IPPerformance/ETAlgorithm.h"

namespace TrigConf {
  class xAODConfigTool;
}

namespace Trig {
  class TrigDecisionTool;
}

class EventSelectorAlg : public ETAlgorithm
//class EventSelectorAlg : public AthAlgorithm 
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:

    // float cutValue;
    Gaudi::Property<bool> m_applyGRLCut{this, "applyGRLCut", false, "whether to apply the GRL Cut"};
    Gaudi::Property<std::string> m_GRLxml {this, "GRLxml", "", "Path to GRL XML file"};
    Gaudi::Property<std::string> m_GRLExcludeList{this, "GRLExcludeList", "", "the GRL Exclude List"};
    
    // Primary Vertex
    std::string m_inVertexContName;
    Gaudi::Property<bool> m_applyPrimaryVertexCut{this, "applyPrimaryVertexCut", false, "whether to apply the Primary Vertex Cut"};
    int m_PVNTrack;

    // Event Cleaning
    Gaudi::Property<bool> m_applyEventCleaningCut{this, "applyEventCleaningCut", false, "whether to apply event cleaning cut"};

    // Trigger
    std::string m_triggerSelection;
    Gaudi::Property<bool> m_applyTriggerCut{this, "applyTriggerCut", false, "whether to apply trigger cut"};
    Gaudi::Property<bool> m_testTrigger{this, "testTrigger", false, "whether to test trigger"};

    //PU Reweighting
    Gaudi::Property<bool> m_applyPUreweighting{this, "applyPUreweighting", false, "whether to apply PU reweighting"};
    std::string m_lumiCalcFileNames;
    std::string m_PRWFileNames;
    Gaudi::Property<std::string> m_configFileName{this, "configFileName", "", "config file name"};


private:
    ToolHandle<GoodRunsListSelectionTool>    m_grl;//{this,"grl","GoodRunsListSelectionTool"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_grlKey {this, "grlKey", "EventInfo.passGRL", "Decoration for GRL"};
    //ToolHandle<CP::PileupReweightingTool>    m_pileuptool{this,"pileuptool","CP::PileupReweightingTool"};
    
   // ToolHandle<TrigConf::xAODConfigTool>     m_trigConfTool{this,"trigConfTool","TrigConf::xAODConfigTool"};
    //ToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool{this, "trigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool" };
    
    int m_eventCounter;     //!public:

    Gaudi::Property<bool> m_isMC{this, "isMC", false, " whether the data is Monte Carlo"};
    Gaudi::Property<int> m_PU_default_channel{this, "PU_default_channel", 0, "PU default channel"};
    Gaudi::Property<bool> m_debug{this,"debug",false,"whether debug"};

    //cutflow
    TH1D* m_cutflowHist = nullptr;    //!
    int m_cutflow_all;      //!
    int m_cutflow_grl;      //!
    int m_cutflow_lar;      //!
    int m_cutflow_tile;     //!
    int m_cutflow_core;     //!
    int m_cutflow_npv;      //!
    int m_cutflow_trigger;  //!

public:

    // this is a standard constructor
    EventSelectorAlg (const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~EventSelectorAlg();
    virtual StatusCode initialize();
    virtual StatusCode execute();
    virtual StatusCode finalize();

    // added functions not from Algorithm
    bool passPrimaryVertexSelection(const xAOD::VertexContainer* vertexContainer, int Ntracks);
    const xAOD::Vertex* getPrimaryVertex(const xAOD::VertexContainer* vertexContainer);


  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
 
};
#endif
