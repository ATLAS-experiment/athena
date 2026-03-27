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
//#include "IPPerformance/ETAlgorithm.h"

namespace TrigConf {
  class xAODConfigTool;
}

namespace Trig {
  class TrigDecisionTool;
}

//class EventSelectorAlg : public ETAlgorithm
class EventSelectorAlg : public AthAlgorithm 
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:

    // float cutValue;
    Gaudi::Property<bool> m_applyGRLCut{this, "applyGRLCut", false, "whether to apply the GRL Cut"};
    Gaudi::Property<std::string> m_GRLxml {this, "GRLxml", "$IPPerformance_DIR/data/IPPerformance/data15_13TeV.periodAllYear_DetStatus-v89-pro21-02_Unknown_PHYS_StandardGRL_All_Good_25ns.xml", "Path to GRL XML file"};
    
    // Primary Vertex
    SG::ReadHandleKey<xAOD::VertexContainer> m_inVertexKey{this, "VetexKey", "PrimaryVertices"};
    Gaudi::Property<bool> m_applyPrimaryVertexCut{this, "applyPrimaryVertexCut", true, "whether to apply the Primary Vertex Cut"};
    Gaudi::Property<int> m_PVNTrack{this, "NTrackForPrimaryVertex", 3, "number of tracks required for a primary vertex"};

    // Event Cleaning
    Gaudi::Property<bool> m_applyEventCleaningCut{this, "applyEventCleaningCut", true, "whether to apply event cleaning cut"};

    // Trigger
    Gaudi::Property<std::string> m_triggerSelection{this, "Trigger", "HLT_j[0-9]*","list of triggers"};
    Gaudi::Property<bool> m_applyTriggerCut{this, "applyTriggerCut", false, "whether to apply trigger cut"};


private:
    ToolHandle<GoodRunsListSelectionTool>    m_grl;//{this,"grl","GoodRunsListSelectionTool"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_grlKey {this, "grlKey", "EventInfo.passGRL", "Decoration for GRL"};
    
    int m_eventCounter;     //!public:

    Gaudi::Property<bool> m_isMC{this, "isMC", false, " whether the data is Monte Carlo"};

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
