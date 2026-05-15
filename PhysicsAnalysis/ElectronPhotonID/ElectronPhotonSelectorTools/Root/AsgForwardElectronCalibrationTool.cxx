/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
    @class AsgForwardElectronCalibrationTool
    @brief Tool to apply DNN-based pT calibration to forward electrons for Run 4 using ITk and HGTD variables.
    Internal Note: https://cds.cern.ch/record/2922184

    Workflow:
      1. Extract 25 input variables from the forward electron container
      2. Run DNN via lwtnn (MinMax scaler already into JSON as a batch norm layer)
      3. Apply softplus manually (not supported natively by lwtnn)
      4. Undo MinMax pT scaling -> calibrated pT in MeV

    @author Mathis Dubau (LAPP)
    @date   Feb 2026

    Based on ElectronDNNCalculator and AsgForwardElectronLikelihoodTool

*/


#include "ElectronPhotonSelectorTools/AsgForwardElectronCalibrationTool.h"

#include "PathResolver/PathResolver.h"
#include "xAODEgamma/Electron.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "CaloGeoHelpers/CaloSampling.h"

#include "lwtnn/parse_json.hh"

#include <fstream>
#include <cmath>





//=============================================================================
// Standard constructor
//=============================================================================
AsgForwardElectronCalibrationTool::AsgForwardElectronCalibrationTool(
  const std::string& myname)
  : AsgTool(myname)
{
  // Declare the needed properties
  declareProperty("ModelFiles", m_modelFiles,
                  "lwtnn JSON files, one per eta bin (in eta order)");
  declareProperty("pTMin", m_pTMin,
                  "Lower bound of pT MinMax scaling [MeV]");
  declareProperty("pTMax", m_pTMax,
                  "Upper bound of pT MinMax scaling [MeV]");
}

//=============================================================================
// Standard destructor
//=============================================================================
AsgForwardElectronCalibrationTool::~AsgForwardElectronCalibrationTool() = default;

// ============================================================================
// Initialise
// ============================================================================
StatusCode AsgForwardElectronCalibrationTool::initialize()
{
  // Sanity checks
  if (m_modelFiles.size() != 3) {
    ATH_MSG_ERROR("Exactly 3 model files expected (one per eta bin): "
                  "[2.5,2.7], [2.7,3.2], [3.2,4.0]). Only got "
                  << m_modelFiles.size());
    return StatusCode::FAILURE;
  }

  // Fixed variable names - must match the lwtnn JSON input
  m_variables = {
    // Calo eta and Phi
    "x1_calo_eta",          
    "x2_calo_phi",
    // ITk eta and Phi
    "x3_track_eta", 
    "x4_track_phi",
    // HGTD time
    "x5_time",
    // ITk hit counts
    "x6_pixels",            
    "x7_strips",
    // Shower shape moments 
    "x8_ENG_FRAC_MAX", 
    "x9_LONGITUDINAL",
    "x10_SECOND_LAMBDA",    
    "x11_LATERAL",
    "x12_SECOND_R",         
    "x13_CENTER_LAMBDA",
    "x14_SECOND_ENG_DENS",
    // Track-cluster matching variables
    "x15_delta_eta2",       
    "x16_delta_phi2",
    "x17_delta_phi_rescaled2", 
    "x18_delta_phi_last",
    // Calo energy fractions in each layer
    "x19_calo_frac_EM_1",
    "x20_calo_frac_EM_2",  
    "x21_calo_frac_EM_3",
    "x22_calo_frac_HAD_0",  
    "x23_calo_frac_HAD_1",
    "x24_calo_frac_HAD_2",  
    "x25_calo_frac_HAD_3"
  };
  // More informations on Table 3 in Internal Note: https://cds.cern.ch/record/2922184 

  // Load one JSON per eta bin
  m_graphs.reserve(3);
  for (const auto& model : m_modelFiles) {
    const std::string path = PathResolverFindCalibFile(model);
    if (path.empty()) {
      ATH_MSG_ERROR("Could not locate: " << model);
      return StatusCode::FAILURE;
    }
    std::ifstream dnn_json(path);
    auto parsed = lwt::parse_json_graph(dnn_json);
    m_graphs.emplace_back(
      std::make_unique<lwt::LightweightGraph>(parsed));
    ATH_MSG_INFO("Loaded calibration model for bin "
                 << m_graphs.size() - 1 << ": " << path);
  }

  ATH_MSG_INFO("AsgForwardElectronCalibrationTool initialised");
  return StatusCode::SUCCESS;
}

// ============================================================================
// Calibration
// ============================================================================
double AsgForwardElectronCalibrationTool::calibrate(const EventContext& /*ctx*/,
                                                    const xAOD::Electron* eg) const
{
  if (!eg) {
    ATH_MSG_ERROR("Failed, no Electron object.");
    return -999.;
  }

  const xAOD::CaloCluster* cluster = eg->caloCluster();
  if (!cluster) {
    ATH_MSG_WARNING("Failed, no cluster.");
    return -999.;
  }

  // ITk electron must have a track
  const xAOD::TrackParticle* track = eg->trackParticle();
  if (!track) {
    ATH_MSG_WARNING("Failed, no track.");
    return -999.;
  }

  const double absEta = std::abs(cluster->eta());
  const int etaBin = getEtaBin(absEta);
  if (etaBin < 0) {
    ATH_MSG_WARNING("Electron |eta|=" << absEta
                    << " is outside allowed range.");
    return -999.;
  }


  // Get input variables
  std::vector<float> inputs;
  if (!getInputs(eg, inputs)) return -999.;


  // Compute the DNN output
  std::map<std::string, std::map<std::string, double>> inputMap;
  for (size_t i = 0; i < m_variables.size(); ++i)
    inputMap["node_0"][m_variables[i]] = static_cast<float>(inputs[i]);

  const auto outputs  = m_graphs[etaBin]->compute(inputMap);
  const double rawOut = outputs.begin()->second;
  const double calibratedPt = unscalePt(softplus(rawOut));

  // Round to 1 MeV precision due to lwtnn mismatch
  double calibratedPt_rounded = std::round(calibratedPt);

  return calibratedPt_rounded;
}

// ============================================================================
// getEtaBin
// ============================================================================
int AsgForwardElectronCalibrationTool::getEtaBin(double absEta) const
{
  // Convention: x1 < |eta| <= x2
  if (absEta > 2.5 && absEta <= 2.7) return 0;
  if (absEta > 2.7 && absEta <= 3.2) return 1;
  if (absEta > 3.2 && absEta <= 4.0) return 2;
  return -1;
}

// ============================================================================
// getInputs 
// ============================================================================
bool AsgForwardElectronCalibrationTool::getInputs(const xAOD::Electron* eg,
                                                  std::vector<float>& inputs) const
{
  inputs.clear();
  inputs.reserve(25);

  const xAOD::CaloCluster*   cluster = eg->caloCluster();
  const xAOD::TrackParticle* track   = eg->trackParticle();

  if (!cluster) { ATH_MSG_ERROR("No CaloCluster.");   return false; }
  if (!track)   { ATH_MSG_ERROR("No TrackParticle."); return false; }

  // x1, x2 = calo eta and phi
  inputs.push_back(static_cast<float>(cluster->eta()));
  inputs.push_back(static_cast<float>(cluster->phi()));

  // x3, x4 = track eta and phi
  inputs.push_back(static_cast<float>(track->eta()));
  inputs.push_back(static_cast<float>(track->phi()));

  // x5 = HGTD time
   static const SG::AuxElement::Accessor<uint8_t> accValid("hasValidTime");
   if (track && accValid.isAvailable(*track) && accValid(*track))
     {
       inputs.push_back(static_cast<float>(track->time()));
     }
   else
     {
       ATH_MSG_WARNING("No valid time for the track while doing track->time(): " );
       inputs.push_back(-99);
     }
   
  // x6, x7 = ITk hit counts
  inputs.push_back(static_cast<float>(eg->trackParticleSummaryIntValue(xAOD::numberOfPixelHits)));
  inputs.push_back(static_cast<float>(eg->trackParticleSummaryIntValue(xAOD::numberOfSCTHits)));

  // x8 to x14 = 7 calorimeter shower-shape moments
  auto getMoment = [&](xAOD::CaloCluster::MomentType type,
                       const char* name) -> float {
    double val{0.};
    if (!cluster->retrieveMoment(type, val))
      ATH_MSG_WARNING("Could not retrieve calo moment: " << name);
    return val;
  };

  inputs.push_back(getMoment(xAOD::CaloCluster::ENG_FRAC_MAX,    "ENG_FRAC_MAX"));
  inputs.push_back(getMoment(xAOD::CaloCluster::LONGITUDINAL,    "LONGITUDINAL"));
  inputs.push_back(getMoment(xAOD::CaloCluster::SECOND_LAMBDA,   "SECOND_LAMBDA"));
  inputs.push_back(getMoment(xAOD::CaloCluster::LATERAL,         "LATERAL"));
  inputs.push_back(getMoment(xAOD::CaloCluster::SECOND_R,        "SECOND_R"));
  inputs.push_back(getMoment(xAOD::CaloCluster::CENTER_LAMBDA,   "CENTER_LAMBDA"));
  inputs.push_back(getMoment(xAOD::CaloCluster::SECOND_ENG_DENS, "SECOND_ENG_DENS"));

  // x15 to x18 = track-calo matching
  auto getMatch = [&](xAOD::EgammaParameters::TrackCaloMatchType type,
                      const char* name) -> float {
    float val{0.f};
    if (!eg->trackCaloMatchValue(val, type))
      ATH_MSG_WARNING("Could not retrieve track-calo match: " << name);
    return val;
  };

  inputs.push_back(getMatch(xAOD::EgammaParameters::deltaEta2,
                            "delta_eta2"));
  inputs.push_back(getMatch(xAOD::EgammaParameters::deltaPhi2,
                            "delta_phi2"));
  inputs.push_back(getMatch(xAOD::EgammaParameters::deltaPhiRescaled2,
                            "delta_phi_rescaled2"));
  inputs.push_back(getMatch(xAOD::EgammaParameters::deltaPhiFromLastMeasurement,
                            "delta_phi_last"));

  // x19 to x25 = Derived calorimeter energy fractions
  // Prevent division by zero
  const double caloE   = cluster->e();
  const double inv_E   = (caloE != 0.) ? 1. / caloE : 0.;

  using CS = CaloSampling::CaloSample;

  // EM fractions: 
  //   f_i^EM = energyBE(i) / caloE  for i in {1,2,3}
  inputs.push_back(static_cast<float>(cluster->energyBE(1) * inv_E));
  inputs.push_back(static_cast<float>(cluster->energyBE(2) * inv_E));
  inputs.push_back(static_cast<float>(cluster->energyBE(3) * inv_E));

  // HAD fractions:
  //   f_0^HAD = HEC0 / caloE
  //   f_i^HAD = (HEC_i + FCAL_(i-1)) / caloE  for i in {1,2,3}
  inputs.push_back(static_cast<float>(
    cluster->eSample(static_cast<CS>(CaloSampling::HEC0)) * inv_E));
  inputs.push_back(static_cast<float>(
    (cluster->eSample(static_cast<CS>(CaloSampling::HEC1)) +
     cluster->eSample(static_cast<CS>(CaloSampling::FCAL0))) * inv_E));
  inputs.push_back(static_cast<float>(
    (cluster->eSample(static_cast<CS>(CaloSampling::HEC2)) +
     cluster->eSample(static_cast<CS>(CaloSampling::FCAL1))) * inv_E));
  inputs.push_back(static_cast<float>(
    (cluster->eSample(static_cast<CS>(CaloSampling::HEC3)) +
     cluster->eSample(static_cast<CS>(CaloSampling::FCAL2))) * inv_E));

  return true;
}

// ============================================================================
// Helpers
// ============================================================================

double AsgForwardElectronCalibrationTool::softplus(double x) const
{
  // Avoids overflow for large x
  return (x > 20.) ? x : std::log1p(std::exp(x));
}

double AsgForwardElectronCalibrationTool::unscalePt(double x) const
{
  // Inverse MinMaxScaler with feature_range=(0,1)
  // Trained with: 
  //              pTMin =  10 GeV
  //              pTMax = 255 GeV
  return x * (m_pTMax - m_pTMin) + m_pTMin;
}
