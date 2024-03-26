/*
 *   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 *   */

#ifndef TRACKOVERLAYREC_TRACKOVERLAYDECISIONALG_H
#define TRACKOVERLAYREC_TRACKOVERLAYDECISIONALG_H

// STL includes
#include <string>
// FrameWork includes
#include "StoreGate/ReadHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include <EventBookkeeperTools/FilterReporterParams.h>
// local includes
#include "TrkTruthTrackInterfaces/IAthSelectionTool.h"
//#gaudi includes
#include "GaudiKernel/ToolHandle.h"
//
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthPileupEventContainer.h"
//ONNX Runtime include(s)
#include <onnxruntime_cxx_api.h>
#include "AthOnnxruntimeService/IONNXRuntimeSvc.h"
namespace  TrackOverlayDecisionAlg{
  const double M_TWOPI = 2.0 * M_PI;
  // Calculate constants only once for feature scaling
  const float px_diff = 0.00029734736; // (1/3363.07) px_max: 1893.19; px_min: -1.46988e+03
  const float py_diff = 0.000335; //(1/2979.65) py_max: 1628.23; py_min: -1.35142e+03
  const float pz_diff = 0.00025; //(1/3924.99) pz_max: 2420.35 pz_min: -1.50464e+03
  const float pt_diff = 0.0005283; //(1/1892.74) pt_max: 1893.24 pt_min: 5.00006e-01
  const float e_diff = 0.000372; //(1/2689.63) e_max: 2690.14 e_min: 5.08307e-01
  const float pu_diff = 0.01449; //(1/69) pu_max: 84.5 pu_min: 15.5
  const float multi_diff = 0.00342; //(1/292) multiplicity_max:310.0 min:1.80000000e+01
  const float eventPt_diff = 0.0004798; //(1/2084.2) eventPt_max: 2084.61 min:3.42359395e-01
  const float area0p2 =  127.388535032;//(1.0 / (M_PI * 0.05 * 0.05)) for R=0.05
  const float area0p05 = 7.961783; // 1.0 / (M_PI * 0.2 * 0.2); for R=0.2
  const float constant1 = 0.000417; //(1.0 / 2395.2); 1/max_of_multiplicity_0p2
  const float constant2 =  0.00002626749; //(1.0 / 38069.86); 1/max_of_multiplicity_0p05
  const float constant3 = 0.014797; //(1.0 / 67.58); 1/max_of_sum_0p2
  const float constant4 = 0.006934; //(1.0 / 144.21); 1/max_of_sum_0p05
  const float constant5 = 0.0004086; //(1.0 / 2447.24); 1/max_of_sumPtAroundTrack for R=0.2
  const float constant6 = 0.0004186; //(1.0 / 2388.55); 1/max_of_sumPtAroundTrack for R=0.05

  class TrackOverlayDecisionAlg : public AthReentrantAlgorithm{
  public:
    /** Constructor with parameters */
    TrackOverlayDecisionAlg( const std::string& name, ISvcLocator* pSvcLocator );
    /** Destructor */
    virtual ~TrackOverlayDecisionAlg() = default;
    /** Athena algorithm's interface method initialize() */
    virtual StatusCode  initialize() override final; /** Athena algorithm's interface method execute() */
    virtual StatusCode  execute(const EventContext& ctx) const override final;
    /** Athena algorithm's interface method finalize() */
    virtual StatusCode  finalize() override final;
  private:
    ToolHandle<IAthSelectionTool> m_truthSelectionTool{this, "TruthSelectionTool","AthTruthSelectionTool", "Truth selection tool (for efficiencies and resolutions)"};
    SG::AuxElement::Decorator<bool> m_dec_selectedByPileupSwitch{"selectedByPileupSwitch"};
    bool m_usingSpecialPileupSwitch {false};
    //set the "selectedByPileupSwitch" decoration for all particles in the passed vector
    void markSelectedByPileupSwitch(const std::vector<const xAOD::TruthParticle*> & truthParticles) const;
    BooleanProperty m_useTrackSelection {this, "useTrackSelection", false, "plot only tracks accepted by selection tool"};
    StringProperty m_pileupSwitch {this, "PileupSwitch", "HardScatter", "Pileup truth strategy to use. May be \"All\", \"HardScatter\", or \"PileUp\""};
    FloatProperty m_lowProb{this,"LowProb",0.5,"Truth match prob. cutoff for efficiency (lower bound) and fake (upper bound) classification."};
    //EventInfo container name
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoContainerName{this,"EventInfoContainerName", "EventInfo", ""};
    //TruthParticle container's name
    SG::ReadHandleKey<xAOD::TruthPileupEventContainer> m_truthPileUpEventName{this, "TruthPileupEvents", "TruthPileupEvents","Name of the truth pileup events container probably TruthPileupEvent(s)"};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleName{this, "TruthParticleContainerName",  "TruthParticles", ""};
    SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEventName{this, "TruthEvents", "TruthEvents","Name of the truth events container probably either TruthEvent or TruthEvents"};
     // Get truth particles into a vector, possibly using the pileup from the event
    const std::vector<const xAOD::TruthParticle *> getTruthParticles() const;
    FilterReporterParams m_filterParams {this, "TrackOverlayDecisionAlg", "Decides whether events should be reconstructed in track-overlay workflow or MC-overlay."};
    Gaudi::Property<bool>  m_invertfilter{this, "InvertFilter", false, "Invert filter decision."}; //!< invert filter decision at the end
    Gaudi::Property<float> m_MLthreshold{this, "MLThreshold", 0.74201, "ML threshold for bad/good tracks decision. ML scores larger than this threshold are considered as bad tracks."};
    // Set up the ONNX Runtime session
    ServiceHandle<AthONNX::IONNXRuntimeSvc> m_svc{this, "ONNXRuntimeSvc", "AthONNX::ONNXRuntimeSvc", "CaloMuonScoreTool ONNXRuntimeSvc"};
    std::tuple<std::vector<int64_t>, std::vector<char*>> m_inputInfo;
    std::tuple<std::vector<int64_t>, std::vector<char*>> m_outputInfo;
    std::unique_ptr<Ort::Session> m_session;
    inline std::tuple<std::vector<int64_t>, std::vector<char*> > GetInputNodeInfo(const std::unique_ptr< Ort::Session >& session) {
      std::vector<int64_t> input_node_dims;
      size_t num_input_nodes = session->GetInputCount();
      std::vector<char*> input_node_names(num_input_nodes);
      Ort::AllocatorWithDefaultOptions allocator;
      for( std::size_t i = 0; i < num_input_nodes; i++ ) {
        char* input_name = session->GetInputNameAllocated(i, allocator).release();
        input_node_names[i] = input_name;
        Ort::TypeInfo type_info = session->GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        input_node_dims = tensor_info.GetShape();
        }
      return std::make_tuple(input_node_dims, input_node_names);
     }

    inline std::tuple<std::vector<int64_t>, std::vector<char*> > GetOutputNodeInfo(const std::unique_ptr< Ort::Session >& session){
     std::vector<int64_t> output_node_dims;
     size_t num_output_nodes = session->GetOutputCount();
     std::vector<char*> output_node_names(num_output_nodes);
     Ort::AllocatorWithDefaultOptions allocator;
     for( std::size_t i = 0; i < num_output_nodes; i++ ) {
         char* output_name = session->GetOutputNameAllocated(i, allocator).release();
         output_node_names[i] = output_name;
         Ort::TypeInfo type_info = session->GetOutputTypeInfo(i);
         auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
         output_node_dims = tensor_info.GetShape();
     }
     return std::make_tuple(output_node_dims, output_node_names);
     }
  };

}
#endif
