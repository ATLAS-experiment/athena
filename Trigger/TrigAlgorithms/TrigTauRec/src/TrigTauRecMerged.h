/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUREC_TRIGTAURECMERGED_H
#define TRIGTAUREC_TRIGTAURECMERGED_H

#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Parsers/Factory.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "tauRecTools/ITauToolBase.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrackContainer.h"

class TrigTauRecMerged: public AthReentrantAlgorithm {
public:
  TrigTauRecMerged(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
    template<class V> StatusCode deepCopy(SG::WriteHandle<DataVector<V>>& writeHandle, const DataVector<V>* oldContainer) const;

    // Error codes for calo reconstruction
    enum TAUEFCALOMON {
        NoROIDescr=0,
        NoCellCont=1,
        EmptyCellCont=2,
        NoClustCont=3,
        NoClustKey=4,
        EmptyClustCont=5,
        NoJetAttach=6,
        NoHLTtauAttach=7,
        NoHLTtauDetAttach=8,
        NoHLTtauXdetAttach=9
    };

    // Error codes for tracking
    enum TAUEFTRKMON {
        NoTrkCont=0,
        NoVtxCont=1
    };

    // Internal tools store
    // The sequence of tool execution goes as follows:
    // VertexFinderTools -> CommonToolsBeforeTF -> TrackFinderTools -> CommonTools -> VertexVarsTools -> IDTools
    const ToolHandleArray<ITauToolBase> m_commonTools {this, "CommonTools", {}, "List of ITauToolBase common tools"};
    const ToolHandleArray<ITauToolBase> m_commonToolsBeforeTF {this, "CommonToolsBeforeTF", {}, "List of ITauToolBase common tools to execute before the Track Finder tools"};
    const ToolHandleArray<ITauToolBase> m_vertexFinderTools {this, "VertexFinderTools", {}, "Vertex Finder tools"};
    const ToolHandleArray<ITauToolBase> m_trackFinderTools {this, "TrackFinderTools", {}, "Track Finder tools"};
    const ToolHandleArray<ITauToolBase> m_vertexVarsTools {this, "VertexVarsTools", {}, "Vertex Variables tools"};
    const ToolHandleArray<ITauToolBase> m_idTools {this, "IDTools", {}, "Vertex Variables tools"};

    // Monitoring tool
    const ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool"};
    Gaudi::Property<std::map<std::string, std::pair<std::string, std::string>>> m_monitoredIdScores {this, "MonitoredIDScores", {}, "Pairs of the TauID score and signal-transformed scores for each TauID algorithm to be monitored"};
    std::map<std::string, std::pair<SG::ConstAccessor<float>, SG::ConstAccessor<float>>> m_monitoredIdAccessors;

    // Inputs
    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiInputKey {this, "InputRoIs", "", "Input RoI name"};
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clustersInputKey {this, "InputCaloClusterContainer", "", "Caloclusters in view"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexInputKey {this, "InputVertexContainer", "", "Input vertex container"};
    SG::ReadHandleKey<xAOD::TauJetContainer> m_tauJetInputKey {this, "InputTauJetContainer", "", "Input TauJet container"};
    SG::ReadHandleKey<xAOD::TauTrackContainer> m_tauTrackInputKey {this, "InputTauTrackContainer", "", "Input TauTrack container" };

    // Outputs
    SG::WriteHandleKey<xAOD::JetContainer> m_tauSeedOutputKey {this, "OutputJetSeed", "", "Output jets which are seeds for tau jets"};
    SG::WriteHandleKey<xAOD::TauJetContainer> m_tauJetOutputKey {this, "OutputTauJetContainer", "", "Output TauJet container"};
    SG::WriteHandleKey<xAOD::TauTrackContainer> m_tauTrackOutputKey {this, "OutputTauTrackContainer", "", "Output TauTrack container"};

    // Helper methods
    bool doCaloReconstruction() const { return m_tauJetInputKey.key().empty() && !m_clustersInputKey.key().empty(); }
};

// Function to perform deep copy on container
template<class V> StatusCode TrigTauRecMerged::deepCopy(SG::WriteHandle<DataVector<V>>& writeHandle, const DataVector<V>* oldContainer) const {
   if(!writeHandle.isValid()) {
        ATH_MSG_FATAL("Provided with an invalid write handle");
        return StatusCode::FAILURE;
   }

   if(oldContainer){
       for(const V* v : *oldContainer) {
           // Build a new object in the new container
           writeHandle->push_back(std::make_unique<V>());
           // Copy across the aux store
           *writeHandle->back() = *v;
       }
   }
   return StatusCode::SUCCESS;
}

#endif
