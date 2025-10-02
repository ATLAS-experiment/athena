/*
 *  *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 *   *   */
//
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <Eigen/Core>
//
#include "GaudiKernel/SystemOfUnits.h"
#include "Gaudi/Property.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthPileupEvent.h"
#include "xAODTruth/TruthPileupEventAuxContainer.h"
#include "PathResolver/PathResolver.h"

// ONNX Runtime include(s).
#include <onnxruntime_cxx_api.h>
//
#include "TrackOverlayDecisionAlg.h"
#include "EventBookkeeperTools/FilterReporter.h"

namespace TrackOverlayDecisionAlg {
    TrackOverlayDecisionAlg::TrackOverlayDecisionAlg( const std::string& name, ISvcLocator* pSvcLocator ) :
     ::AthReentrantAlgorithm( name, pSvcLocator )
    {
    }

StatusCode TrackOverlayDecisionAlg::initialize()
{
  ATH_CHECK(m_filterParams.initialize(false));
  ATH_CHECK( m_eventInfoContainerName.initialize() );
  ATH_CHECK( m_truthParticleName.initialize( (m_pileupSwitch == "HardScatter" or m_pileupSwitch == "All") and not m_truthParticleName.key().empty() ) );
  ATH_CHECK(m_truthSelectionTool.retrieve(EnableTool {not m_truthParticleName.key().empty()} ));
  ATH_CHECK( m_truthEventName.initialize( (m_pileupSwitch == "HardScatter" or m_pileupSwitch == "All") and not m_truthEventName.key().empty() ) );
  ATH_CHECK( m_truthPileUpEventName.initialize( (m_pileupSwitch == "PileUp" or m_pileupSwitch == "All") and not m_truthPileUpEventName.key().empty() ) );
  ATH_CHECK(m_svc.retrieve());
  std::string this_file = __FILE__;
  const std::string model_path = PathResolverFindCalibFile("TrackOverlay/TrackOverlay_J7_model.onnx");
  Ort::SessionOptions session_options;

  m_session = std::make_unique<Ort::Session>(m_svc->env(), model_path.c_str(), session_options);
  m_inputInfo = TrackOverlayDecisionAlg::GetInputNodeInfo(m_session);
  m_outputInfo = TrackOverlayDecisionAlg::GetOutputNodeInfo(m_session);
  return StatusCode::SUCCESS;
}

StatusCode TrackOverlayDecisionAlg::finalize()
{
  ATH_MSG_VERBOSE ( "Finalizing ..." );
  ATH_MSG_VERBOSE("-----------------------------------------------------------------");
  ATH_MSG_VERBOSE("m_filterParams.summary()" << m_filterParams.summary());
  ATH_MSG_VERBOSE("-----------------------------------------------------------------");
  ATH_MSG_INFO(m_filterParams.summary());
  ATH_MSG_VERBOSE(" =====================================================================");

  return StatusCode::SUCCESS;
}

const std::vector<const xAOD::TruthParticle*> TrackOverlayDecisionAlg::getTruthParticles() const {
   std::vector<const xAOD::TruthParticle*> tempVec {};
   if (m_pileupSwitch == "All") {
    if (m_truthParticleName.key().empty()) {
      return tempVec;
    }
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainer( m_truthParticleName);
    if (not truthParticleContainer.isValid()) {
      return tempVec;
    }
    tempVec.insert(tempVec.begin(), truthParticleContainer->begin(), truthParticleContainer->end());
  } else {
   if (m_pileupSwitch == "HardScatter") {
    if (not m_truthEventName.key().empty()) {
    ATH_MSG_VERBOSE("Getting TruthEvents container.");
    SG::ReadHandle<xAOD::TruthEventContainer> truthEventContainer( m_truthEventName);
    const xAOD::TruthEvent* event = (truthEventContainer.isValid()) ? truthEventContainer->at(0) : nullptr;
    if (not event) {
        return tempVec;
      }
    const auto& links = event->truthParticleLinks();
      tempVec.reserve(event->nTruthParticles());
      for (const auto& link : links) {
        if (link.isValid()){
          tempVec.push_back(*link);
        }
       }
      }
    }else if (m_pileupSwitch == "PileUp") {
      if (not m_truthPileUpEventName.key().empty()) {
      ATH_MSG_VERBOSE("getting TruthPileupEvents container");
      // get truth particles from all pileup events
      SG::ReadHandle<xAOD::TruthPileupEventContainer> truthPileupEventContainer(m_truthPileUpEventName);
      if (truthPileupEventContainer.isValid()) {
        const unsigned int nPileup = truthPileupEventContainer->size();
        tempVec.reserve(nPileup * 200); // quick initial guess, will still save some time
        for (unsigned int i(0); i != nPileup; ++i) {
          const auto *eventPileup = truthPileupEventContainer->at(i);
          // get truth particles from each pileup event
          int ntruth = eventPileup->nTruthParticles();
          ATH_MSG_VERBOSE("Adding " << ntruth << " truth particles from TruthPileupEvents container");
          const auto& links = eventPileup->truthParticleLinks();
          for (const auto& link : links) {
            if (link.isValid()){
              tempVec.push_back(*link);
            }
          }
        }
      } else {
        ATH_MSG_ERROR("no entries in TruthPileupEvents container!");
      }
      }
    } else {
      ATH_MSG_ERROR("bad value for PileUpSwitch");
    }
   }
   return tempVec;
}

StatusCode TrackOverlayDecisionAlg::execute(const EventContext &ctx) const
{
    ATH_MSG_DEBUG ("Executing ...");

    std::vector<const xAOD::TruthParticle*> truthParticlesVec = TrackOverlayDecisionAlg::getTruthParticles();

    //Access truth info for the NN input
    float eventPxSum = 0.0;
    float eventPySum = 0.0;
    float eventPt = 0.0;
    float puEvents = 0.0;

    std::vector<float> pxValues, pyValues, pzValues, eValues, etaValues, phiValues, ptValues;
    float truthMultiplicity = 0.0;
    const int truthParticles = truthParticlesVec.size();
    for (int itruth = 0; itruth < truthParticles; itruth++) {
        const xAOD::TruthParticle* thisTruth = truthParticlesVec[itruth];
        const IAthSelectionTool::CutResult accept = m_truthSelectionTool->accept(thisTruth);
        if(accept){
           pxValues.push_back((thisTruth->px()*0.001-1.46988000e+03)* px_diff); //as MinMaxScaler: 1.46988000e+03 is the lowest value of px from a J7 sample; *(0.001) is used to convert unit rather than *(1/1000) to speed up.
           pyValues.push_back((thisTruth->py()*0.001-1.35142000e+03)* py_diff); //the lowest value of py: 1.35142000e+03
           pzValues.push_back((thisTruth->pz()*0.001-1.50464000e+03)* pz_diff); //the lowest value of pz: 1.50464000e+03
           ptValues.push_back((thisTruth->pt()*0.001-5.00006000e-01)* pt_diff); //the lowest value of pt: 5.00006000e-01

           etaValues.push_back(thisTruth->eta());
           phiValues.push_back(thisTruth->phi());
           eValues.push_back((thisTruth->e()*0.001-5.08307000e-01)*e_diff); //the lowest value of energy: 5.08307000e-01

           eventPxSum += thisTruth->px();
           eventPySum += thisTruth->py();
           truthMultiplicity++;
        }//accept
    }//for itruth
    SG::ReadHandle<xAOD::TruthPileupEventContainer> truthPileupEventContainer;
    SG::ReadHandle<xAOD::EventInfo> pie = SG::ReadHandle<xAOD::EventInfo>(m_eventInfoContainerName, ctx);
    if (!m_truthPileUpEventName.key().empty()) {
         truthPileupEventContainer = SG::ReadHandle<xAOD::TruthPileupEventContainer>(m_truthPileUpEventName, ctx);
    }
    puEvents = !m_truthPileUpEventName.key().empty() and truthPileupEventContainer.isValid() ?  static_cast<int>( truthPileupEventContainer->size() ) : pie.isValid() ? pie->actualInteractionsPerCrossing() : 0;
    eventPt = std::sqrt(eventPxSum*eventPxSum + eventPySum*eventPySum)*0.001;

    std::vector<float> puEventsVec(pxValues.size(), (puEvents-1.55000000e+01)*pu_diff); //min of puEvents= 15.5, max of puEvents=84.5
    std::vector<float> truthMultiplicityVec(pxValues.size(), (truthMultiplicity-1.80000000e+01)*multi_diff);
    std::vector<float> eventPtVec(pxValues.size(), (eventPt-3.42359395e-01)*eventPt_diff);
    std::vector<float> predictions;

    //Compute the distances using Eigen for Eigen's optimized operations. Initialize matirces. Observed a significant improvement on computing calculation.
    Eigen::VectorXf ptEigen = Eigen::VectorXf::Map(ptValues.data(), ptValues.size());
    Eigen::VectorXf phiEigen = Eigen::VectorXf::Map(phiValues.data(), phiValues.size());
    Eigen::VectorXf etaEigen = Eigen::VectorXf::Map(etaValues.data(), etaValues.size());
    for (std::size_t i = 0; i < truthMultiplicity; ++i) {
        float multiplicity_0p05 = 0.0, multiplicity_0p2 = 0.0;
        float sum_0p05 = 0.0, sum_0p2 = 0.0;
        float pt_0p05 = 0.0, pt_0p2 = 0.0;
        float deltaEtaI = etaEigen[i];
        float phiI = phiEigen[i];
        for (std::size_t j = 0; j < truthMultiplicity; ++j) {
            if (i == j) continue; // Skip the particle itself
            float deltaEta = deltaEtaI - etaEigen[j];
            float deltaPhi = phiI - phiEigen[j];
            if (deltaPhi > M_PI) {
                deltaPhi -= 2.0 * M_PI;
            }
            float distances = std::sqrt(deltaEta * deltaEta + deltaPhi * deltaPhi);
            if (distances < 0.05){
                multiplicity_0p05++;
                sum_0p05 += distances;
                pt_0p05 += ptEigen[j];
            }
            if (distances < 0.2){
                multiplicity_0p2++;
                sum_0p2 += distances;
                pt_0p2 += ptEigen[j];
            }
        }// for j

    std::vector<float> featData;
    featData.push_back(pxValues[i]);
    featData.push_back(pyValues[i]);
    featData.push_back(pzValues[i]);
    featData.push_back(eValues[i]);
    featData.push_back(ptValues[i]);
    featData.push_back((multiplicity_0p2 * area0p2) * constant1);
    featData.push_back((multiplicity_0p05 * area0p05) * constant2);
    featData.push_back((sum_0p2 * area0p2) * constant3);
    featData.push_back((sum_0p05 * area0p05) * constant4);
    featData.push_back(pt_0p2 * constant5);
    featData.push_back(pt_0p05 * constant6);

    featData.push_back(puEventsVec[i]);
    featData.push_back(truthMultiplicityVec[i]);
    featData.push_back(eventPtVec[i]);

    std::vector<int64_t> input_node_dims;
    std::vector<char*> input_node_names;
    input_node_dims = std::get<0>(m_inputInfo);
    input_node_names = std::get<1>(m_inputInfo);

    std::vector<int64_t> output_node_dims;
    std::vector<char*> output_node_names;
    output_node_dims = std::get<0>(m_outputInfo);
    output_node_names = std::get<1>(m_outputInfo);

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeCPU);
    input_node_dims[0]=1;
    Ort::Value input_data = Ort::Value::CreateTensor(memoryInfo, featData.data(), featData.size(), input_node_dims.data(), input_node_dims.size());
    Ort::RunOptions run_options(nullptr);
    //Run the inference
    Ort::Session& mysession ATLAS_THREAD_SAFE = *m_session;
    auto output_values = mysession.Run(run_options, input_node_names.data(), &input_data, input_node_names.size(), output_node_names.data(), output_node_names.size());
    float* predictionData = output_values[0].GetTensorMutableData<float>();
    float prediction = predictionData[0];

    predictions.push_back(prediction);
    }//for i
    float threshold = m_MLthreshold;
    ATH_MSG_ALWAYS("ML threshold:" << threshold);
    int badTracks = 0;
    for (float prediction : predictions) {
       if (prediction > threshold) {
          badTracks++;
        }
    }
    float rouletteScore = static_cast<float>(badTracks) / static_cast<float>(truthMultiplicity);

    FilterReporter filter(m_filterParams, false, ctx);
    bool pass = false;
    int decision = rouletteScore == 0;
    if (decision==0){ //if ML decision is False, it goes to the MC-overlay workflow
      pass = true;
    }
    else{
      pass = false;
    }

    if (m_invertfilter) {
    pass =! pass;
    }
    filter.setPassed(pass);
    ATH_MSG_ALWAYS("End TrackOverlayDecisionAlg, difference in filters: "<<(pass ? "found" : "not found")<<"="<<pass<<", invert="<<m_invertfilter);
    return StatusCode::SUCCESS;
}


}// end namespace TrackOverlayDecisionAlg
