/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgServices/AsgServiceConfig.h>
#include <AsgTools/AsgToolConfig.h>

#include <algorithm>
#include <format>
#include <memory>
#include <string>
#include <utility>

#include "AthContainers/ConstAccessor.h"
#include "PATInterfaces/SystematicRegistry.h"
#include "PathResolver/PathResolver.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaDefs.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "xAODTracking/Vertex.h"
#include "EgammaAnalysisInterfaces/GlobalEventInfo.h"

#ifndef ROOTCORE
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"
#endif

// internal (old) tool
#include <cmath>

#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"
#include "ElectronPhotonFourMomentumCorrection/GainTool.h"
#include "ElectronPhotonFourMomentumCorrection/GainUncertainty.h"
#include "ElectronPhotonFourMomentumCorrection/LinearityADC.h"
#include "ElectronPhotonFourMomentumCorrection/egammaEnergyCorrectionTool.h"
#include "egammaLayerRecalibTool/egammaLayerRecalibTool.h"

namespace CP {

const double GeV = 1000.;

std::unique_ptr<egGain::GainTool> gainToolFactory(egEnergyCorr::ESModel model) {
  switch (model) {
    case egEnergyCorr::es2011d:
    case egEnergyCorr::es2011dMedium:
    case egEnergyCorr::es2011dTight:
    case egEnergyCorr::es2012c:
    case egEnergyCorr::es2012XX:
    case egEnergyCorr::es2015PRE:
    case egEnergyCorr::es2015cPRE:
    case egEnergyCorr::es2015PRE_res_improved:
    case egEnergyCorr::es2015cPRE_res_improved:
    case egEnergyCorr::es2015c_summer:
    case egEnergyCorr::es2015_day0_3percent:
    case egEnergyCorr::es2016PRE: {
      const std::string gain_filename1 = PathResolverFindCalibFile(
          "ElectronPhotonFourMomentumCorrection/v8/FunctionsTO.root");
      const std::string gain_filename2 = PathResolverFindCalibFile(
          "ElectronPhotonFourMomentumCorrection/v8/FunctionsG_all.root");
      return std::make_unique<egGain::GainTool>(gain_filename1, gain_filename2);
    }
    case egEnergyCorr::es2017:
    case egEnergyCorr::es2017_summer:
    case egEnergyCorr::es2017_summer_improved:
    case egEnergyCorr::es2017_summer_final:
    case egEnergyCorr::es2017_R21_PRE:
    case egEnergyCorr::es2017_R21_v0:
    case egEnergyCorr::es2017_R21_v1:
    case egEnergyCorr::es2017_R21_ofc0_v1:
    case egEnergyCorr::es2018_R21_v0:
    case egEnergyCorr::es2018_R21_v1:
    case egEnergyCorr::es2022_R22_PRE:
    case egEnergyCorr::es2023_R22_Run2_v0:
    case egEnergyCorr::es2023_R22_Run2_v1:
    case egEnergyCorr::es2024_Run3_ofc0_v0:
    case egEnergyCorr::es2024_Run3_v0:
      return nullptr;
    default:
      return nullptr;
  }
}

std::string egammaMVAToolFolder(egEnergyCorr::ESModel model) {
  std::string folder;
  switch (model) {
    case egEnergyCorr::es2011d:
    case egEnergyCorr::es2011dMedium:
    case egEnergyCorr::es2011dTight:
      folder = "egammaMVACalib/v1";
      break;
    case egEnergyCorr::es2012c:
      folder = "egammaMVACalib/v1";
      break;
    case egEnergyCorr::es2012XX:
    case egEnergyCorr::es2015PRE:
    case egEnergyCorr::es2015PRE_res_improved:
    case egEnergyCorr::es2015_day0_3percent:
      folder = "egammaMVACalib/offline/v3";
      break;
    case egEnergyCorr::es2015cPRE:
    case egEnergyCorr::es2015cPRE_res_improved:
      folder = "egammaMVACalib/offline/v3_E4crack_bis";
      break;
    case egEnergyCorr::es2015c_summer:
    case egEnergyCorr::es2016PRE:
    case egEnergyCorr::es2017:
    case egEnergyCorr::es2017_summer:
    case egEnergyCorr::es2017_summer_improved:
    case egEnergyCorr::es2017_summer_final:
    case egEnergyCorr::es2015_5TeV:
      folder = "egammaMVACalib/offline/v4.0";
      break;
    case egEnergyCorr::es2017_R21_PRE:
    case egEnergyCorr::es2017_R21_v0:
    case egEnergyCorr::es2017_R21_v1:
    case egEnergyCorr::es2017_R21_ofc0_v1:
    case egEnergyCorr::es2018_R21_v0:
    case egEnergyCorr::es2018_R21_v1:
    case egEnergyCorr::es2022_R22_PRE:
    case egEnergyCorr::es2024_Run3_ofc0_v0:
      folder = "egammaMVACalib/offline/v7";
      break;
    case egEnergyCorr::es2023_R22_Run2_v0:
    case egEnergyCorr::es2023_R22_Run2_v1:
      folder = "egammaMVACalib/offline/v9";
      break;
    case egEnergyCorr::es2024_Run3_v0:
      folder = "egammaMVACalib/offline/v10";
      break;
    default:
      folder = "";
  }

  return folder;
}

std::unique_ptr<egammaLayerRecalibTool> egammaLayerRecalibToolFactory(
    egEnergyCorr::ESModel model, int enableSacc) {
  std::string tune = "";
  switch (model) {
    case egEnergyCorr::es2011d:
    case egEnergyCorr::es2011dMedium:
    case egEnergyCorr::es2011dTight:
      tune = "2011_alt_with_layer2";
      break;
    case egEnergyCorr::es2012c:
    case egEnergyCorr::es2012XX:
    case egEnergyCorr::es2015PRE:
    case egEnergyCorr::es2015cPRE:
    case egEnergyCorr::es2015PRE_res_improved:
    case egEnergyCorr::es2015cPRE_res_improved:
    case egEnergyCorr::es2015_day0_3percent:
    case egEnergyCorr::es2015c_summer:
    case egEnergyCorr::es2016PRE:
    case egEnergyCorr::es2017:
    case egEnergyCorr::es2015_5TeV:
    case egEnergyCorr::es2017_R21_PRE:
      tune = "2012_alt_with_layer2";
      break;
    case egEnergyCorr::es2017_summer:
    case egEnergyCorr::es2017_summer_improved:
      tune = "es2017_20.7_improved";
      break;
    case egEnergyCorr::es2017_summer_final:
      tune = "es2017_20.7_final";
      break;
    case egEnergyCorr::es2017_R21_v0:
    case egEnergyCorr::es2017_R21_v1:
    case egEnergyCorr::es2017_R21_ofc0_v1:
    case egEnergyCorr::es2018_R21_v0:
    case egEnergyCorr::es2022_R22_PRE:
    case egEnergyCorr::es2024_Run3_ofc0_v0:
      tune = "es2017_21.0_v0";
      break;
    case egEnergyCorr::es2018_R21_v1:
      tune = "es2018_21.0_v0";
      break;
    case egEnergyCorr::es2023_R22_Run2_v0:
      tune = "es2022_22.0_Precision";
      break;
    case egEnergyCorr::es2023_R22_Run2_v1:
      tune = "es2022_22.0_Precision_v1";
      break;
    case egEnergyCorr::es2024_Run3_v0:
      tune = "es2024_run3_extrapolate_v0";
      break;
    default:
      return nullptr;
  }
  return std::make_unique<egammaLayerRecalibTool>(tune, enableSacc);
}

bool use_intermodule_correction(egEnergyCorr::ESModel model) {
  switch (model) {
    case egEnergyCorr::es2010:
    case egEnergyCorr::es2011c:
    case egEnergyCorr::es2011d:
    case egEnergyCorr::es2011dMedium:
    case egEnergyCorr::es2011dTight:
    case egEnergyCorr::es2012a:
      return false;
    case egEnergyCorr::es2012c:
    case egEnergyCorr::es2012cMedium:
    case egEnergyCorr::es2012cTight:
    case egEnergyCorr::es2015_day0_3percent:
    case egEnergyCorr::es2012XX:
    case egEnergyCorr::es2015PRE:
    case egEnergyCorr::es2015cPRE:
    case egEnergyCorr::es2015PRE_res_improved:
    case egEnergyCorr::es2015cPRE_res_improved:
    case egEnergyCorr::es2015c_summer:
    case egEnergyCorr::es2016PRE:
    case egEnergyCorr::es2017:
    case egEnergyCorr::es2017_summer:
    case egEnergyCorr::es2017_summer_improved:
    case egEnergyCorr::es2017_summer_final:
    case egEnergyCorr::es2015_5TeV:
    case egEnergyCorr::es2017_R21_PRE:
    case egEnergyCorr::es2017_R21_v0:
    case egEnergyCorr::es2017_R21_v1:
    case egEnergyCorr::es2017_R21_ofc0_v1:
    case egEnergyCorr::es2018_R21_v0:
    case egEnergyCorr::es2018_R21_v1:
    case egEnergyCorr::es2022_R22_PRE:
    case egEnergyCorr::es2023_R22_Run2_v0:
    case egEnergyCorr::es2023_R22_Run2_v1:
    case egEnergyCorr::es2024_Run3_ofc0_v0:
    case egEnergyCorr::es2024_Run3_v0:
      return true;
    case egEnergyCorr::UNDEFINED:  // TODO: find better logic
      return false;
  }
  assert(false);
  return false;
}

bool use_phi_uniform_correction(egEnergyCorr::ESModel model) {
  return use_intermodule_correction(model);  // they are equal
}

bool is_after_run1(egEnergyCorr::ESModel model) {
  switch (model) {
    case egEnergyCorr::es2010:
    case egEnergyCorr::es2011c:
    case egEnergyCorr::es2011d:
    case egEnergyCorr::es2011dMedium:
    case egEnergyCorr::es2011dTight:
    case egEnergyCorr::es2012a:
    case egEnergyCorr::es2012c:
    case egEnergyCorr::es2012cMedium:
    case egEnergyCorr::es2012cTight:
      return false;
    case egEnergyCorr::es2015_day0_3percent:
    case egEnergyCorr::es2012XX:
    case egEnergyCorr::es2015PRE:
    case egEnergyCorr::es2015cPRE:
    case egEnergyCorr::es2015PRE_res_improved:
    case egEnergyCorr::es2015cPRE_res_improved:
    case egEnergyCorr::es2015c_summer:
    case egEnergyCorr::es2016PRE:
    case egEnergyCorr::es2017:
    case egEnergyCorr::es2017_summer:
    case egEnergyCorr::es2017_summer_improved:
    case egEnergyCorr::es2017_summer_final:
    case egEnergyCorr::es2015_5TeV:
    case egEnergyCorr::es2017_R21_PRE:
    case egEnergyCorr::es2017_R21_v0:
    case egEnergyCorr::es2017_R21_v1:
    case egEnergyCorr::es2017_R21_ofc0_v1:
    case egEnergyCorr::es2018_R21_v0:
    case egEnergyCorr::es2018_R21_v1:
    case egEnergyCorr::es2022_R22_PRE:
    case egEnergyCorr::es2023_R22_Run2_v0:
    case egEnergyCorr::es2023_R22_Run2_v1:
    case egEnergyCorr::es2024_Run3_ofc0_v0:
    case egEnergyCorr::es2024_Run3_v0:
      return true;
    case egEnergyCorr::UNDEFINED:  // TODO: find better logic
      return false;
  }
  assert(false);
  return false;
}

EgammaCalibrationAndSmearingTool::EgammaCalibrationAndSmearingTool(
    const std::string& name)
    : asg::AsgMetadataTool(name),
      m_TESModel(egEnergyCorr::UNDEFINED),
      m_TResolutionType(egEnergyCorr::Resolution::SigmaEff90),
      m_use_mapping_correction(false),
      m_currentScaleVariation_MC(egEnergyCorr::Scale::None),
      m_currentScaleVariation_data(egEnergyCorr::Scale::Nominal),
      m_currentResolutionVariation_MC(egEnergyCorr::Resolution::Nominal),
      m_currentResolutionVariation_data(egEnergyCorr::Resolution::None),
      m_set_seed_function([](const EgammaCalibrationAndSmearingTool& tool,
                             columnar::EgammaId egamma,
                             columnar::EventInfoId ei) {
        const Accessors& acc = *tool.m_accessors;
        // avoid 0 as result, see
        // https://root.cern.ch/root/html/TRandom3.html#TRandom3:SetSeed
        auto cluster = acc.caloClusterAcc(egamma)[0].value();
        return 1 + static_cast<RandomNumber>(
                       std::abs(acc.clusterPhiAcc(cluster)) * 1E6 +
                       std::abs(acc.clusterEtaAcc(cluster)) * 1E3 +
                       acc.eventNumberAcc(ei));
      }),
      m_accessors(std::make_unique<Accessors>(*this)) {

  declareProperty("ESModel", m_ESModel = "");
  declareProperty("decorrelationModel", m_decorrelation_model_name = "");
  declareProperty("decorrelationModelScale",
                  m_decorrelation_model_scale_name = "");
  declareProperty("decorrelationModelResolution",
                  m_decorrelation_model_resolution_name = "");
  declareProperty("ResolutionType", m_ResolutionType = "SigmaEff90");
  declareProperty("varSF", m_varSF = 1.0);
  declareProperty("doScaleCorrection", m_doScaleCorrection = AUTO);
  declareProperty("doSmearing", m_doSmearing = AUTO);
  declareProperty("useLayerCorrection", m_useLayerCorrection = AUTO);
  declareProperty("usePSCorrection", m_usePSCorrection = AUTO);
  declareProperty("useS12Correction", m_useS12Correction = AUTO);
  declareProperty("useSaccCorrection", m_useSaccCorrection = AUTO);
  declareProperty("useIntermoduleCorrection",
                  m_useIntermoduleCorrection = AUTO);
  declareProperty("usePhiUniformCorrection", m_usePhiUniformCorrection = AUTO);
  declareProperty("useCaloDistPhiUnifCorrection",
                  m_useCaloDistPhiUnifCorrection = AUTO);
  declareProperty("useGainCorrection", m_useGainCorrection = AUTO);
  declareProperty("useGainInterpolation", m_useGainInterpolation = AUTO);
  declareProperty("doADCLinearityCorrection",
                  m_doADCLinearityCorrection = AUTO);
  declareProperty("doLeakageCorrection", m_doLeakageCorrection = AUTO);
  declareProperty("MVAfolder", m_MVAfolder = "");
  declareProperty("layerRecalibrationTune", m_layer_recalibration_tune = "");
  declareProperty("useEPCombination", m_use_ep_combination = false);
  declareProperty("useMVACalibration", m_use_mva_calibration = AUTO);
  declareProperty("use_full_statistical_error",
                  m_use_full_statistical_error = false);
  declareProperty("use_temp_correction201215",
                  m_use_temp_correction201215 = AUTO);
  declareProperty("use_uA2MeV_2015_first2weeks_correction",
                  m_use_uA2MeV_2015_first2weeks_correction = AUTO);
  declareProperty("randomRunNumber", m_user_random_run_number = 0);
  // this is the user input, it is never changed by the tool. The tool uses
  // m_simulation.
  declareProperty("useFastSim", m_useFastSim = -1,
                  "This should be explicitly set by the user depending on the "
                  "data type (int)0=full sim, (int)1=fast sim");
  declareProperty(
      "useAFII", m_use_AFII = -1,
      "This is now deprecated. Kept for explicit error message for now");
  declareProperty("decorateEmva", m_decorateEmva = false, "whether to decorate the eMVA value");
}

EgammaCalibrationAndSmearingTool::~EgammaCalibrationAndSmearingTool() {
  ATH_MSG_DEBUG("destructor");
  delete m_layer_recalibration_tool;
  delete m_gain_tool;
}

StatusCode EgammaCalibrationAndSmearingTool::initialize() {
  ATH_MSG_INFO("Initialization");

  if (m_ESModel == "es2015XX") {
    ATH_MSG_ERROR("es2015XX is deprecated. Use es2015PRE");
  }

  if (m_ESModel == "es2010") {
    m_TESModel = egEnergyCorr::es2010;
  }  // legacy
  else if (m_ESModel == "es2011c") {
    m_TESModel = egEnergyCorr::es2011c;
  }  // mc11c : faulty G4; old geometry
  else if (m_ESModel == "es2011d") {
    m_TESModel = egEnergyCorr::es2011d;
  }  // mc11d : corrected G4; new geometry == final Run1 scheme
  else if (m_ESModel == "es2012a") {
    m_TESModel = egEnergyCorr::es2012a;
  }  // mc12a : "crude" G4 fix; old geometry
  else if (m_ESModel == "es2012c") {
    m_TESModel = egEnergyCorr::es2012c;
  }  // mc12c : corrected G4; new geometry == final Run1 scheme
  else if (m_ESModel == "es2012XX") {
    m_TESModel = egEnergyCorr::es2012XX;
  } else if (m_ESModel == "es2015PRE") {
    m_TESModel = egEnergyCorr::es2015PRE;
  } else if (m_ESModel == "es2015PRE_res_improved") {
    m_TESModel = egEnergyCorr::es2015PRE_res_improved;
  } else if (m_ESModel == "es2015cPRE") {
    m_TESModel = egEnergyCorr::es2015cPRE;
  } else if (m_ESModel == "es2015cPRE_res_improved") {
    m_TESModel = egEnergyCorr::es2015cPRE_res_improved;
  } else if (m_ESModel == "es2015c_summer") {
    m_TESModel = egEnergyCorr::es2015c_summer;
  } else if (m_ESModel == "es2016PRE") {
    m_TESModel = egEnergyCorr::es2016PRE;
  } else if (m_ESModel == "es2016data_mc15c") {
    m_TESModel = egEnergyCorr::es2017;
  } else if (m_ESModel == "es2016data_mc15c_summer") {
    m_TESModel = egEnergyCorr::es2017_summer;
  } else if (m_ESModel == "es2016data_mc15c_summer_improved") {
    m_TESModel = egEnergyCorr::es2017_summer_improved;
  } else if (m_ESModel == "es2016data_mc15c_final") {
    m_TESModel = egEnergyCorr::es2017_summer_final;
  } else if (m_ESModel == "es2015_5TeV") {
    m_TESModel = egEnergyCorr::es2015_5TeV;
  } else if (m_ESModel == "es2017_R21_PRE") {
    m_TESModel = egEnergyCorr::es2017_R21_PRE;
  } else if (m_ESModel == "es2017_R21_v0") {
    m_TESModel = egEnergyCorr::es2017_R21_v0;
  } else if (m_ESModel == "es2017_R21_v1") {
    m_TESModel = egEnergyCorr::es2017_R21_v1;
  } else if (m_ESModel == "es2017_R21_ofc0_v1") {
    m_TESModel = egEnergyCorr::es2017_R21_ofc0_v1;
  } else if (m_ESModel == "es2018_R21_v0") {
    m_TESModel = egEnergyCorr::es2018_R21_v0;
  } else if (m_ESModel == "es2018_R21_v1") {
    m_TESModel = egEnergyCorr::es2018_R21_v1;
  } else if (m_ESModel == "es2022_R22_PRE") {
    m_TESModel = egEnergyCorr::es2022_R22_PRE;
  } else if (m_ESModel == "es2023_R22_Run2_v0") {
    m_TESModel = egEnergyCorr::es2023_R22_Run2_v0;
  } else if (m_ESModel == "es2023_R22_Run2_v1") {
    m_TESModel = egEnergyCorr::es2023_R22_Run2_v1;
  } else if (m_ESModel == "es2024_Run3_ofc0_v0") {
    m_TESModel = egEnergyCorr::es2024_Run3_ofc0_v0;
  } else if (m_ESModel == "es2024_Run3_v0") {
    m_TESModel = egEnergyCorr::es2024_Run3_v0;
  } else if (m_ESModel.empty()) {
    ATH_MSG_ERROR("you must set ESModel property");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_ERROR("Cannot understand model " << m_ESModel);
    return StatusCode::FAILURE;
  }

  if (m_ResolutionType == "Gaussian") {
    m_TResolutionType = egEnergyCorr::Resolution::Gaussian;
  } else if (m_ResolutionType == "SigmaEff80") {
    m_TResolutionType = egEnergyCorr::Resolution::SigmaEff80;
  } else if (m_ResolutionType == "SigmaEff90") {
    m_TResolutionType = egEnergyCorr::Resolution::SigmaEff90;
  } else {
    ATH_MSG_ERROR("Cannot understand resolution " << m_ResolutionType);
    return StatusCode::FAILURE;
  }

  if (m_use_AFII != -1) {
    ATH_MSG_ERROR(
        "Property useAFII is deprecated. It is now replaced with useFastSim, "
        "which should be explicitly configured");
    return StatusCode::FAILURE;
  }

  if (m_useFastSim == 1) {
    m_simulation = PATCore::ParticleDataType::Fast;
  } else if (m_useFastSim == 0) {
    m_simulation = PATCore::ParticleDataType::Full;
  } else {
    ATH_MSG_ERROR("Property useFastSim should be explicitly configured");
    return StatusCode::FAILURE;
  }

  if ( (m_TESModel == egEnergyCorr::es2022_R22_PRE || m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0) &&
      m_simulation == PATCore::ParticleDataType::Fast) {
    ATH_MSG_ERROR(
        "Sample is FastSim but no AF3 calibration is supported with "
        "MC23 pre-recommendations (es2022_R22_PRE and es2024_Run3_ofc0_v0). "
        "Please swtich to Run3 consolidated recommendations (es2024_Run3_v0), " 
        "or get in touch with the EGamma CP group in case you are using this");
    return StatusCode::FAILURE;
  }

  // configure decorrelation model, translate string property to internal class
  // enum
  /*    S R SR
    0.  0 0 0     WARNING Full, Full (this is the default without configuration)
    1.  0 0 1     SR
    2.  0 1 0     FATAL
    3.  0 1 1     WARNING SR then R
    4.  1 0 0     FATAL
    5.  1 0 1     WARNING SR then S
    6.  1 1 0     S, R
    7.  1 1 1     FATAL
  */
  if (m_decorrelation_model_name.empty() and
      m_decorrelation_model_scale_name.empty() and
      m_decorrelation_model_resolution_name.empty()) {
    // case 0
    ATH_MSG_WARNING("no decorrelation model specified, assuming full model");
    m_decorrelation_model_scale = ScaleDecorrelation::FULL;
    m_decorrelation_model_resolution = ResolutionDecorrelation::FULL;
    m_decorrelation_model_name = "FULL_v1";
  } else if (not m_decorrelation_model_name.empty() and
             not m_decorrelation_model_scale_name.empty() and
             not m_decorrelation_model_resolution_name.empty()) {
    // case 7
    ATH_MSG_FATAL("too many flags for the decorrelation model");
    return StatusCode::FAILURE;
  } else {
    // set scale decorrelation model
    if (not m_decorrelation_model_scale_name.empty()) {  // case 4, 5, 6, (7)
      if (not m_decorrelation_model_name.empty()) {
        ATH_MSG_WARNING(
            "flag decorrelation model ignored for scale decorrelation model");
      }  // case 5
      if (m_decorrelation_model_scale_name == "1NP_v1")
        m_decorrelation_model_scale = ScaleDecorrelation::ONENP;
      else if (m_decorrelation_model_scale_name == "FULL_ETACORRELATED_v1")
        m_decorrelation_model_scale = ScaleDecorrelation::FULL_ETA_CORRELATED;
      else if (m_decorrelation_model_scale_name == "1NPCOR_PLUS_UNCOR")
        m_decorrelation_model_scale = ScaleDecorrelation::ONENP_PLUS_UNCONR;
      else if (m_decorrelation_model_scale_name == "FULL_v1")
        m_decorrelation_model_scale = ScaleDecorrelation::FULL;
      else {
        ATH_MSG_FATAL("cannot understand the scale decorrelation model '"
                      << m_decorrelation_model_scale_name << "'(typo?)");
        return StatusCode::FAILURE;
      }
    } else if (not m_decorrelation_model_name.empty()) {  // case 1, 3
      if (m_decorrelation_model_name == "1NP_v1")
        m_decorrelation_model_scale = ScaleDecorrelation::ONENP;
      else if (m_decorrelation_model_name == "FULL_ETACORRELATED_v1")
        m_decorrelation_model_scale = ScaleDecorrelation::FULL_ETA_CORRELATED;
      else if (m_decorrelation_model_name == "1NPCOR_PLUS_UNCOR")
        m_decorrelation_model_scale = ScaleDecorrelation::ONENP_PLUS_UNCONR;
      else if (m_decorrelation_model_name == "FULL_v1")
        m_decorrelation_model_scale = ScaleDecorrelation::FULL;
      else {
        ATH_MSG_FATAL("cannot understand the decorrelation model '"
                      << m_decorrelation_model_name << "'(typo?)");
        return StatusCode::FAILURE;
      }
    } else {  // case 2, (7)
      ATH_MSG_FATAL(
          "not information how to initialize the scale decorrelation model");
      return StatusCode::FAILURE;
    }

    // set resolution decorralation model
    if (not m_decorrelation_model_resolution_name
                .empty()) {  // case 2, 3, 6, (7)
      if (not m_decorrelation_model_name.empty()) {
        ATH_MSG_WARNING(
            "flag decorrelation model ignored for resolution decorrelation "
            "model");
      }  // case 3
      if (m_decorrelation_model_resolution_name == "1NP_v1")
        m_decorrelation_model_resolution = ResolutionDecorrelation::ONENP;
      else if (m_decorrelation_model_resolution_name == "FULL_v1")
        m_decorrelation_model_resolution = ResolutionDecorrelation::FULL;
      else {
        ATH_MSG_FATAL("cannot understand the resolution decorrelation model '"
                      << m_decorrelation_model_resolution_name << "'(typo?)");
        return StatusCode::FAILURE;
      }
    } else if (not m_decorrelation_model_name.empty()) {  // case 1, 5
      if (m_decorrelation_model_name == "1NP_v1")
        m_decorrelation_model_resolution = ResolutionDecorrelation::ONENP;
      else if (m_decorrelation_model_name == "FULL_ETACORRELATED_v1")
        m_decorrelation_model_resolution = ResolutionDecorrelation::FULL;
      else if (m_decorrelation_model_name == "1NPCOR_PLUS_UNCOR")
        m_decorrelation_model_resolution = ResolutionDecorrelation::ONENP;
      else if (m_decorrelation_model_name == "FULL_v1")
        m_decorrelation_model_resolution = ResolutionDecorrelation::FULL;
      else {
        ATH_MSG_FATAL("cannot understand the decorrelation model '"
                      << m_decorrelation_model_name << "'(typo?)");
        return StatusCode::FAILURE;
      }
    }
  }

  // create correction tool
  ATH_MSG_DEBUG("creating internal correction tool");
  m_rootTool = std::make_unique<AtlasRoot::egammaEnergyCorrectionTool>();
  if (!m_rootTool) {
    ATH_MSG_ERROR("Cannot initialize underlying tool");
    return StatusCode::FAILURE;
  }
  m_rootTool->setESModel(m_TESModel);

  if ( (m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 ||  
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 || 
        m_TESModel == egEnergyCorr::es2024_Run3_v0) &&
      (m_useGainInterpolation == AUTO || m_useGainInterpolation == 1)) {
    ATH_MSG_DEBUG(
        "Using linear interpolation in the gain tool (uncertainties only)");
    m_useGainInterpolation = 1;
    m_rootTool->setApplyL2GainInterpolation();
  }
  m_rootTool->msg().setLevel(this->msg().level());
  m_rootTool->initialize();

  // configure MVA calibration
  if (m_use_mva_calibration != 0) {
    ATH_MSG_DEBUG("creating MVA calibration tool (if needed)");
    if (m_MVAfolder.empty()) {  // automatically configure MVA tool
      m_MVAfolder = egammaMVAToolFolder(m_TESModel);
    }

    if (not m_MVAfolder.empty()) {

      // electron MVA tool
      asg::AsgToolConfig config_mva_electron(
          "egammaMVACalibTool/tool_mva_electron");
      config_mva_electron.setPropertyFromString("folder", m_MVAfolder);
      ATH_CHECK(config_mva_electron.setProperty("use_layer_corrected", true));
      ATH_CHECK(config_mva_electron.setProperty(
          "ParticleType", xAOD::EgammaParameters::electron));

      // unconverted photon MVA tool
      asg::AsgToolConfig config_mva_unconverted(
          "egammaMVACalibTool/tool_mva_unconverted");
      config_mva_unconverted.setPropertyFromString("folder", m_MVAfolder);
      ATH_CHECK(
          config_mva_unconverted.setProperty("use_layer_corrected", true));
      ATH_CHECK(config_mva_unconverted.setProperty(
          "ParticleType", xAOD::EgammaParameters::unconvertedPhoton));
      ATH_CHECK(config_mva_unconverted.setProperty("OutputLevel",
						   this->msg().level()));

      // converted photon MVA tool
      asg::AsgToolConfig config_mva_converted(
          "egammaMVACalibTool/tool_mva_converted");
      config_mva_converted.setPropertyFromString("folder", m_MVAfolder);
      ATH_CHECK(config_mva_converted.setProperty("use_layer_corrected", true));
      ATH_CHECK(config_mva_converted.setProperty(
          "ParticleType", xAOD::EgammaParameters::convertedPhoton));
      ATH_CHECK(config_mva_converted.setProperty("OutputLevel",
						 this->msg().level()));

      // initialize the ServiceHandler egammaMVASvc
      // make the name unique
      std::ostringstream mva_service_name;
      mva_service_name << "egammaMVASvc/service_mva_egamma_id"
                       << (void const*)this;
      asg::AsgServiceConfig config_mva_service(mva_service_name.str());
      ATH_CHECK(config_mva_service.addPrivateTool("ElectronTool",
                                                  config_mva_electron));
      ATH_CHECK(config_mva_service.addPrivateTool("UnconvertedPhotonTool",
                                                  config_mva_unconverted));
      ATH_CHECK(config_mva_service.addPrivateTool("ConvertedPhotonTool",
                                                  config_mva_converted));
      // fwd electron MVA tool
      if (m_doFwdCalib) {
	asg::AsgToolConfig config_mva_fwdelectron(
	    "egammaMVACalibTool/tool_mva_fwdelectron");
	config_mva_fwdelectron.setPropertyFromString("folder", m_MVAfolder);
	ATH_CHECK(config_mva_fwdelectron.setProperty(
	    "ParticleType", xAOD::EgammaParameters::AuthorFwdElectron));
	ATH_CHECK(config_mva_fwdelectron.setProperty("ShiftType", 0));
	ATH_CHECK(config_mva_fwdelectron.setProperty("OutputLevel", this->msg().level()));
	ATH_CHECK(config_mva_service.addPrivateTool("FwdElectronTool",
						    config_mva_fwdelectron));
      }
      config_mva_service.setPropertyFromString("folder", m_MVAfolder);
      ATH_CHECK(
          config_mva_service.setProperty("OutputLevel", this->msg().level()));
      ATH_CHECK(config_mva_service.makeService(m_MVACalibSvc));
    } else {
      m_use_mva_calibration = false;
    }
  }

  // configure layer recalibration tool
  // For now: layer recalibration not applied to PRE release 21 (using run 1
  // based calibration applied at reco level)
  //  for following R21 recommendations, need to apply the run2/run1 layer
  //  calibration ratio
  if (m_ESModel == "es2017_R21_PRE") {
    ATH_MSG_INFO("Layer recalibration already applied at cell level");
    m_useLayerCorrection = false;
  } else if (!m_useLayerCorrection) {
    ATH_MSG_INFO("Layer corrections disabled!");
  } else {
    ATH_MSG_DEBUG("initializing layer recalibration tool (if needed)");
    if (m_layer_recalibration_tune
            .empty()) {  // automatically configure layer recalibration tool
      m_layer_recalibration_tool =
          egammaLayerRecalibToolFactory(m_TESModel, m_useSaccCorrection)
              .release();
      if (!m_layer_recalibration_tool) {
        ATH_MSG_INFO("not using layer recalibration");
      }
    } else {
      m_layer_recalibration_tool =
          new egammaLayerRecalibTool(m_layer_recalibration_tune);
    }
    if (m_layer_recalibration_tool) {
      m_layer_recalibration_tool->msg().setLevel(this->msg().level());
      m_layer_recalibration_tool->fixForMissingCells(m_fixForMissingCells);
      if (!m_usePSCorrection) {
	ATH_MSG_INFO("PS corrections disabled!");
	m_layer_recalibration_tool->disable_PSCorrections();
      }
      if (!m_useS12Correction) {
	ATH_MSG_INFO("S12 corrections disabled!");
	m_layer_recalibration_tool->disable_S12Corrections();
      }
      if (!m_useSaccCorrection) {
	ATH_MSG_INFO("Sacc corrections disabled!");
	m_layer_recalibration_tool->disable_SaccCorrections();
      }
    }
  }

  if (m_use_temp_correction201215 != AUTO)
    m_rootTool->use_temp_correction201215(m_use_temp_correction201215);
  if (m_use_uA2MeV_2015_first2weeks_correction != AUTO)
    m_rootTool->use_uA2MeV_2015_first2weeks_correction(
        m_use_uA2MeV_2015_first2weeks_correction);
  if (not m_use_full_statistical_error and
      m_decorrelation_model_scale == ScaleDecorrelation::FULL) {
    m_rootTool->useStatErrorScaling(true);
  }

  if (m_use_ep_combination) {
    ATH_MSG_ERROR("ep combination not supported yet");
    throw std::runtime_error("ep combination not supported yet");
  }

  if (m_useIntermoduleCorrection == AUTO) {
    m_useIntermoduleCorrection = use_intermodule_correction(m_TESModel);
  }
  if (m_usePhiUniformCorrection == AUTO) {
    m_usePhiUniformCorrection = use_phi_uniform_correction(m_TESModel);
  }
  m_use_mapping_correction = not is_after_run1(m_TESModel);

  if (m_useGainCorrection == AUTO) {
    if (m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 ||
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 ||
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      m_useGainCorrection = 0;
    } 
    else {
      ATH_MSG_DEBUG("initializing gain tool");
      m_gain_tool = gainToolFactory(m_TESModel).release();
      m_useGainCorrection = bool(m_gain_tool);      
    }
  }
  else if (m_useGainCorrection == 1) {
    if (m_TESModel != egEnergyCorr::es2023_R22_Run2_v0 &&
        m_TESModel != egEnergyCorr::es2023_R22_Run2_v1 &&
        m_TESModel != egEnergyCorr::es2024_Run3_v0) {
      m_useGainCorrection = 0;
      ATH_MSG_ERROR(
          "cannot instantiate gain tool for this model (you can only disable "
          "the gain tool, but not enable it)");
    } 
    else {
      ATH_MSG_INFO(
          "initializing gain tool for run2 final precision recommendations");
      ATH_MSG_WARNING(
          "Gain corrections required but Zee scales are derived without Gain, "
          "will cause inconsistency!");
      std::string gain_tool_run_2_filename = PathResolverFindCalibFile(
          "ElectronPhotonFourMomentumCorrection/v29/"
          "gain_uncertainty_specialRun.root");
      m_gain_tool_run2 = std::make_unique<egGain::GainUncertainty>(
          gain_tool_run_2_filename, false, "GainCorrection",
          m_useGainInterpolation);
      m_gain_tool_run2->msg().setLevel(this->msg().level());
    }
  }

  if (m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 ||
      m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 ||
      m_TESModel == egEnergyCorr::es2024_Run3_v0) {
    // ADC non linearity correction
    if (m_doADCLinearityCorrection == AUTO || m_doADCLinearityCorrection == 1) {
      m_doADCLinearityCorrection = 1;
      std::string adcLinearityCorr_filename = PathResolverFindCalibFile(
          "ElectronPhotonFourMomentumCorrection/v25/linearity_ADC.root");
      m_ADCLinearity_tool = std::make_shared<LinearityADC>(adcLinearityCorr_filename);
      m_ADCLinearity_tool->msg().setLevel(this->msg().level());
      m_rootTool->setADCTool(m_ADCLinearity_tool);
    } else {
      ATH_MSG_WARNING(
          m_ESModel + " recommendations use ADC corrections for scale "
          "derivation. Disabling the ADCLinearity flag will create "
          "inconsistency!");
    }

    if (m_doLeakageCorrection == AUTO || m_doLeakageCorrection == 1) {
      m_doLeakageCorrection = 1;
      m_rootTool->setApplyLeakageCorrection(true);
    }

    // Calo distortion phi unif correction
    if (m_useCaloDistPhiUnifCorrection == AUTO ||
        m_useCaloDistPhiUnifCorrection == 1) {
      m_useCaloDistPhiUnifCorrection = 1;
      std::string phiUnifCorrfileName = PathResolverFindCalibFile(
          "ElectronPhotonFourMomentumCorrection/v33/"
          "egammaEnergyCorrectionData.root");
      std::unique_ptr<TFile> fCorr(
          TFile::Open(phiUnifCorrfileName.c_str(), "READ"));
      m_caloDistPhiUnifCorr.reset(
          dynamic_cast<TH2*>(fCorr->Get("CaloDistortionPhiUniformityCorrection/"
                                        "es2023_R22_Run2_v0/h2DcorrPhiUnif")));
      m_caloDistPhiUnifCorr->SetDirectory(nullptr);
    } else {
      ATH_MSG_WARNING(
          m_ESModel + " recommendations use CaloDistPhiUnif for scale "
          "derivation. Disabling the CaloDistPhiUnif flag will create "
          "inconsistency!");
    }
  }

  // No scale correction for release 21 ==> obsolete
  /*if (m_ESModel == "es2017_R21_PRE"){
    m_doScaleCorrection = 0;
  }
  */

  ATH_MSG_INFO("ESModel: " << m_ESModel);
  ATH_MSG_INFO("ResolutionType: " << m_ResolutionType);
  ATH_MSG_INFO("decorrelation Model: " << m_decorrelation_model_name);
  ATH_MSG_DEBUG("layer correction = " << m_useLayerCorrection);
  ATH_MSG_DEBUG("PS correction = " << m_usePSCorrection);
  ATH_MSG_DEBUG("S12 correction = " << m_useS12Correction);
  ATH_MSG_DEBUG("Sacc correction = " << m_useSaccCorrection);
  ATH_MSG_DEBUG("intermodule correction = " << m_useIntermoduleCorrection);
  ATH_MSG_DEBUG("phi uniformity correction = " << m_usePhiUniformCorrection);
  ATH_MSG_DEBUG("distorted calo phi uniformity correction = "
               << m_useCaloDistPhiUnifCorrection);
  ATH_MSG_DEBUG("gain correction = " << m_useGainCorrection);
  ATH_MSG_DEBUG("ADC non-linearity correction = " << m_doADCLinearityCorrection);
  ATH_MSG_DEBUG("leakage correction for photons = " << m_doLeakageCorrection);
  ATH_MSG_DEBUG("smearing = " << m_doSmearing);
  ATH_MSG_DEBUG("insitu scales = " << m_doScaleCorrection);
  ATH_MSG_DEBUG("ep combination = " << m_use_ep_combination);
  ATH_MSG_DEBUG("use MVA calibration = " << m_use_mva_calibration);
  ATH_MSG_DEBUG(
      "use temperature correction 2015 = " << m_use_temp_correction201215);
  ATH_MSG_DEBUG("use uA2MeV correction 2015 1/2 week = "
               << m_use_uA2MeV_2015_first2weeks_correction);

  setupSystematics();

  applySystematicVariation(CP::SystematicSet())
      .ignore();  // this set the flags for the internal tool without
                  // systematics
  CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();
  if (registry.registerSystematics(*this) != StatusCode::SUCCESS)
    return StatusCode::FAILURE;

  if (m_onlyElectrons.value() && m_onlyPhotons.value()) {
    ATH_MSG_ERROR("Cannot select both onlyElectrons and onlyPhotons");
    return StatusCode::FAILURE;
  }
  if (m_onlyElectrons.value()) {
    if (m_TESModel == egEnergyCorr::es2011c) {
      resetAccessor (m_accessors->electronTrackAcc, *this, "trackParticleLinks");
    }
  }
  if (m_onlyPhotons.value()) {
    resetAccessor (m_accessors->photonVertexAcc, *this, "vertexLinks");
  }
  if (m_decorateEmva)
    resetAccessor (m_accessors->decEmva, *this, "E_mva_only");

  ANA_CHECK (initializeColumns ());

  return StatusCode::SUCCESS;
}


PATCore::ParticleType::Type EgammaCalibrationAndSmearingTool::xAOD2ptype(columnar::EgammaId particle) const
{
  const Accessors& acc = *m_accessors;

  // this is departing from the logic below, as we are now requiring the
  // user to specify at configuration time whether we run on electrons
  // or photons.  this is necessary to configure the columns we need
  // correctly.
  if (m_onlyElectrons.value())
  {
    return PATCore::ParticleType::Electron;
  }
  if (m_onlyPhotons.value())
  {
    if (acc.photonVertexAcc(particle).size() > 0)
    {
      return PATCore::ParticleType::ConvertedPhoton;
    }
    else
    {
      return PATCore::ParticleType::UnconvertedPhoton;
    }
  }

  // this is the old logic and should not be visited in columnar mode
  // (disabled by turning on onlyElectrons or onlyPhotons)
  auto ptype = PATCore::ParticleType::Electron;
  //no ForwardElectron ptype: consider them as Electron
  if (xAOD::EgammaHelpers::isElectron(&particle.getXAODObject()) || acc.authorAcc (particle) == xAOD::EgammaParameters::AuthorFwdElectron) { ptype = PATCore::ParticleType::Electron; }
  else if (xAOD::EgammaHelpers::isPhoton(&particle.getXAODObject())) {
    if (xAOD::EgammaHelpers::isConvertedPhoton(&particle.getXAODObject())) { ptype = PATCore::ParticleType::ConvertedPhoton; }
    else { ptype = PATCore::ParticleType::UnconvertedPhoton; }
  }
  else {
    ATH_MSG_ERROR("particle is not electron of photon");
    throw std::runtime_error("particle is not electron or photon");
  }
  return ptype;
}

double EgammaCalibrationAndSmearingTool::getResolution(
    const xAOD::Egamma& particle, bool withCT) const {
  const auto ptype = xAOD2ptype(particle);
  const auto cl_etaCalo =
      xAOD::get_eta_calo(*particle.caloCluster(), particle.author());

  return m_rootTool->resolution(particle.e(), particle.caloCluster()->eta(),
                                cl_etaCalo, ptype, withCT,
                                false);  // TODO: always for full simulation
}

double EgammaCalibrationAndSmearingTool::resolution(
    double energy, double cl_eta, double cl_etaCalo,
    PATCore::ParticleType::Type ptype, bool withCT) const {
  return m_rootTool->resolution(energy, cl_eta, cl_etaCalo, ptype, withCT,
                                false);
}

CP::CorrectionCode EgammaCalibrationAndSmearingTool::applyCorrection(
    xAOD::Egamma& input) const {
  // Retrieve the event information:
  const xAOD::EventInfo* event_info = nullptr;
  if (evtStore()->retrieve(event_info, "EventInfo").isFailure()) {
    ATH_MSG_ERROR("No EventInfo object could be retrieved");
    return CP::CorrectionCode::Error;
  }
  return applyCorrection(input, *event_info);
}

CP::CorrectionCode EgammaCalibrationAndSmearingTool::correctedCopy(
    const xAOD::Electron& input, xAOD::Electron*& output) const {
  // A sanity check:
  if (output)
    ATH_MSG_WARNING(
        "Non-null pointer received. "
        "There's a possible memory leak!");

  output = new xAOD::Electron();
  output->makePrivateStore(input);
  return applyCorrection(*output);
}

CP::CorrectionCode EgammaCalibrationAndSmearingTool::correctedCopy(
    const xAOD::Photon& input, xAOD::Photon*& output) const {
  // A sanity check:
  if (output)
    ATH_MSG_WARNING(
        "Non-null pointer received. "
        "There's a possible memory leak!");

  output = new xAOD::Photon();
  output->makePrivateStore(input);
  return applyCorrection(*output);
}

double EgammaCalibrationAndSmearingTool::getEnergy(
    const xAOD::Photon& input) const {
  xAOD::Photon* new_particle = nullptr;
  ANA_CHECK_THROW(correctedCopy(input, new_particle));
  const double e = new_particle->e();
  delete new_particle;
  return e;
}

double EgammaCalibrationAndSmearingTool::getEnergy(
    const xAOD::Electron& input) const {
  xAOD::Electron* new_particle = nullptr;
  ANA_CHECK_THROW(correctedCopy(input, new_particle));
  const double e = new_particle->e();
  delete new_particle;
  return e;
}

CP::CorrectionCode EgammaCalibrationAndSmearingTool::applyCorrection(
    columnar::MutableEgammaId input, columnar::EventInfoId event_info) const {
  const Accessors& acc = *m_accessors;

  // only used in simulation (for the smearing)
  RandomNumber seed = m_set_seed_function(*this, input, event_info);

  columnar::ClusterId inputCluster = acc.caloClusterAcc (input)[0].value();

  if (m_layer_recalibration_tool && acc.authorAcc (input) !=
	xAOD::EgammaParameters::AuthorFwdElectron) {
    ATH_MSG_DEBUG("applying energy recalibration before E0|E1|E2|E3 = "
                  << acc.energyBEAcc (inputCluster, 0) << "|"
                  << acc.energyBEAcc (inputCluster, 1) << "|"
                  << acc.energyBEAcc (inputCluster, 2) << "|"
                  << acc.energyBEAcc (inputCluster, 3));
    // for now just go back to the xAOD object to access the subtool
    const CP::CorrectionCode status_layer_recalibration = m_layer_recalibration_tool->applyCorrection(input.getXAODObject(), event_info.getXAODObject());
    if (status_layer_recalibration == CP::CorrectionCode::Error) { return CP::CorrectionCode::Error; }
    ATH_MSG_DEBUG("eta|phi = " << acc.etaAcc (input) << "|" << acc.phiAcc (input));
    if (status_layer_recalibration == CP::CorrectionCode::Ok) {
      ATH_MSG_DEBUG("decoration E0|E1|E2|E3 = "
                    << acc.Es0Acc(inputCluster) << "|"
                    << acc.Es1Acc(inputCluster) << "|"
                    << acc.Es2Acc(inputCluster) << "|"
                    << acc.Es3Acc(inputCluster) << "|");
      if (acc.Es2Acc(inputCluster) == 0 and acc.Es1Acc(inputCluster) == 0 and
          acc.Es3Acc(inputCluster) == 0 and acc.Es0Acc(inputCluster) == 0 and
          (std::abs(acc.etaAcc (input)) < 1.37 or (std::abs(acc.etaAcc (input)) > 1.55 and std::abs(acc.etaAcc (input)) < 2.47)))
      {
        ATH_MSG_WARNING("all layer energies are zero");
      }
    }
  }

  double energy = acc.eAcc (input);
  // apply MVA calibration
  if (!m_MVACalibSvc.empty()) {
    egammaMVACalib::GlobalEventInfo gei;
    if (acc.authorAcc (input) ==
	xAOD::EgammaParameters::AuthorFwdElectron && m_doFwdCalib) {
      const xAOD::VertexContainer* pVtxCont = nullptr;
      if (evtStore()->retrieve(pVtxCont, m_pVtxKey).isFailure()) {
	ATH_MSG_ERROR("No primary vertex container " << m_pVtxKey << " could be retrieved");
	return CP::CorrectionCode::Error;
      }
      unsigned int npv(0);
      for (const auto *vtx : *pVtxCont) {
	if (vtx->vertexType() == xAOD::VxType::PriVtx ||
	    vtx->vertexType() == xAOD::VxType::PileUp) { ++npv; }
      }
      gei.nPV = npv;
      gei.acmu = acc.actIntPerXingAcc(event_info);
      ATH_MSG_DEBUG("Retrieved nPV = " << gei.nPV << " and mu = " << gei.acmu);
    }
    if (acc.authorAcc (input) !=
	xAOD::EgammaParameters::AuthorFwdElectron || m_doFwdCalib) {
      if (m_MVACalibSvc->getEnergy(inputCluster.getXAODObject(), input.getXAODObject(), energy, gei)
	  .isFailure()) {
	ATH_MSG_ERROR("Failure in MVACalib service");
	return CP::CorrectionCode::Error;
      }
      ATH_MSG_DEBUG("energy after MVA calibration = " << std::format("{:.2f}", energy));
    }
  }
  if (m_decorateEmva)
  {
    acc.decEmva(input) = energy;
  }

  // For the time being, it is just the MVA calib
  if (acc.authorAcc (input) ==
	xAOD::EgammaParameters::AuthorFwdElectron) {
    setPt(input, energy);
    return CP::CorrectionCode::Ok;
  }

  if (m_TESModel == egEnergyCorr::es2011c) {
    // Crack calibation correction for es2011c (calibration hits calibration)
    const auto ptype = xAOD2ptype(input);
    const double etaden =
        ptype == PATCore::ParticleType::Electron
            ? static_cast<const xAOD::Electron&>(input.getXAODObject()).trackParticle()->eta()
            : acc.clusterEtaAcc(inputCluster);
    energy *= m_rootTool->applyMCCalibration(acc.clusterEtaAcc(inputCluster),
                                             energy / cosh(etaden), ptype);
    ATH_MSG_DEBUG("energy after crack calibration es2011c = "
                  << std::format("{:.2f}", energy));
  }

  /*
   * Here we check for each event the kind of data DATA vs FullSim
   * The m_simulation flavour has already been configured
   */
  PATCore::ParticleDataType::DataType dataType =
      (acc.eventTypeAcc(event_info,xAOD::EventInfo::IS_SIMULATION))
          ? m_simulation
          : PATCore::ParticleDataType::Data;

  unsigned int runNumber_for_tool = 0;

  // apply uniformity corrections to data
  if (dataType == PATCore::ParticleDataType::Data) {
    // Get run number
    runNumber_for_tool = acc.runNumberAcc(event_info);
    // Get etaCalo, phiCalo
    const auto cl_eta = acc.clusterEtaAcc(inputCluster);
    double etaCalo = 0, phiCalo = 0;
    if (m_ADCLinearity_tool || m_gain_tool_run2 || m_usePhiUniformCorrection) {
      etaCalo = acc.etaCaloAcc(inputCluster, acc.authorAcc(input), false);
      if (m_usePhiUniformCorrection) {
        phiCalo =
            acc.phiCaloAcc(inputCluster, acc.authorAcc(input), false);
      }
    }

    // Intermodule
    if (m_useIntermoduleCorrection) {
      energy =
          intermodule_correction(energy, acc.clusterPhiAcc(inputCluster), cl_eta);
      ATH_MSG_DEBUG("energy after intermodule correction = "
                    << std::format("{:.2f}", energy));
    }

    // Calo distortion
    if ( (m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 || 
          m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 || 
          m_TESModel == egEnergyCorr::es2024_Run3_v0) &&
        m_useCaloDistPhiUnifCorrection) {
      double etaC = acc.clusterEtaAcc(inputCluster);
      double phiC = acc.clusterPhiAcc(inputCluster);
      int ieta = m_caloDistPhiUnifCorr->GetXaxis()->FindBin(etaC);
      ieta = ieta == 0 ? 1
                       : (ieta > m_caloDistPhiUnifCorr->GetNbinsX()
                              ? m_caloDistPhiUnifCorr->GetNbinsX()
                              : ieta);
      int iphi = m_caloDistPhiUnifCorr->GetYaxis()->FindBin(phiC);
      iphi = iphi == 0 ? 1
                       : (iphi > m_caloDistPhiUnifCorr->GetNbinsY()
                              ? m_caloDistPhiUnifCorr->GetNbinsY()
                              : iphi);
      energy *= m_caloDistPhiUnifCorr->GetBinContent(ieta, iphi);
      ATH_MSG_DEBUG(
          "energy after phi uniformity correction (for calo distortion) = "
          << std::format("{:.2f}", energy));
    }

    // Phi
    if (m_usePhiUniformCorrection) {
      energy *= correction_phi_unif(etaCalo, phiCalo);
      ATH_MSG_DEBUG("energy after uniformity correction = "
                    << std::format("{:.2f}", energy));
    }

    // ADC
    if (m_ADCLinearity_tool) {
      double et = energy / std::cosh(cl_eta);
      double corr =
          m_ADCLinearity_tool->getCorr(etaCalo, et, xAOD2ptype(input));
      energy *= corr;
      ATH_MSG_DEBUG("energy after ADC linearity correction = "
                    << std::format("{:.2f}", energy));
    }

    // Gain
    if (m_gain_tool) {
      const auto es2 = acc.Es2Acc.isAvailable(inputCluster)
                           ? acc.Es2Acc(inputCluster)
                           : acc.energyBEAcc (inputCluster,2);
      if (!(std::abs(cl_eta) < 1.52 and std::abs(cl_eta) > 1.37) and
          std::abs(cl_eta) < 2.4)
        energy = m_gain_tool->CorrectionGainTool(
            cl_eta, energy / GeV, es2 / GeV,
            xAOD2ptype(input));  // cl_eta ok, TODO: check corrected E2
    } else if (m_gain_tool_run2) {
      double et = energy / std::cosh(cl_eta);
      double corr = m_gain_tool_run2->getUncertainty(etaCalo, et,
                                                     xAOD2ptype(input), true);
      energy /= (1 + corr);
    }
    ATH_MSG_DEBUG("energy after gain correction = " << std::format("{:.2f}", energy));
  } else {
    if (m_user_random_run_number == 0) {
      if (acc.randomrunnumber_getter.isAvailable(event_info)) {
        runNumber_for_tool = acc.randomrunnumber_getter(event_info);
      } else {
        ATH_MSG_ERROR(
            "Pileup tool not run before using "
            "ElectronPhotonFourMomentumCorrection! Assuming it is 2016. If you "
            "want to force a specific period set the property randomRunNumber "
            "of the tool, e.g. in the job option: "
            "tool.randomRunNumber = 123456 or "
            "tool.randomRunNumber = "
            "EgammaCalibrationAndSmearingToolRunNumbersExample.run_2016");
        runNumber_for_tool = EgammaCalibPeriodRunNumbersExample::run_2016;
      }
    } else {
      runNumber_for_tool = m_user_random_run_number;
    }
  }

  const double eraw = ((acc.Es0Acc.isAvailable(inputCluster)
                            ? acc.Es0Acc(inputCluster)
                            : acc.energyBEAcc(inputCluster,0)) +
                       (acc.Es1Acc.isAvailable(inputCluster)
                            ? acc.Es1Acc(inputCluster)
                            : acc.energyBEAcc(inputCluster,1)) +
                       (acc.Es2Acc.isAvailable(inputCluster)
                            ? acc.Es2Acc(inputCluster)
                            : acc.energyBEAcc(inputCluster,2)) +
                       (acc.Es3Acc.isAvailable(inputCluster)
                            ? acc.Es3Acc(inputCluster)
                            : acc.energyBEAcc(inputCluster,3)));


  if (dataType == PATCore::ParticleDataType::Fast)
    ATH_MSG_DEBUG("is fast");
  else if (dataType == PATCore::ParticleDataType::Full)
    ATH_MSG_DEBUG("is full");
  else if (dataType == PATCore::ParticleDataType::Data)
    ATH_MSG_DEBUG("is data");

  // apply scale factors or systematics
  energy = m_rootTool->getCorrectedEnergy(
      runNumber_for_tool, dataType, xAOD2ptype(input),
      inputCluster(acc.clusterEtaAcc),
      inputCluster(acc.clusterEtaBEAcc,2),
      acc.etaCaloAcc(inputCluster, acc.authorAcc (input), false), energy,
      acc.Es2Acc.isAvailable(inputCluster)
          ? acc.Es2Acc(inputCluster)
          : inputCluster(acc.energyBEAcc,2),
      eraw, seed, oldtool_scale_flag_this_event(input, event_info),
      oldtool_resolution_flag_this_event(input, event_info), m_TResolutionType,
      m_varSF);

  ATH_MSG_DEBUG("energy after scale/systematic correction = " << std::format("{:.2f}", energy));

  // TODO: this check should be done before systematics variations
  setPt(input, energy);
  return CP::CorrectionCode::Ok;
}

void EgammaCalibrationAndSmearingTool::setPt(columnar::MutableEgammaId input, double energy) const {
  const double new_energy2 = energy * energy;
  const double m2 = m_accessors->mAcc (input) * m_accessors->mAcc (input);
  const double p2 = new_energy2 > m2 ? new_energy2 - m2 : 0.;
  m_accessors->ptOutDec (input) = sqrt(p2) / cosh(m_accessors->etaAcc (input));
  ATH_MSG_DEBUG("after setting pt, energy = " << m_accessors->eAcc (input));
}

double EgammaCalibrationAndSmearingTool::getEnergy(
    xAOD::Egamma* p, const xAOD::EventInfo* event_info) {
  ANA_CHECK_THROW(applyCorrection(*p, *event_info));
  ATH_MSG_DEBUG("returning " << p->e());
  return p->e();
}

egEnergyCorr::Scale::Variation
EgammaCalibrationAndSmearingTool::oldtool_scale_flag_this_event(
    columnar::EgammaId p, columnar::EventInfoId event_info) const {
  const Accessors& acc = *m_accessors;
  if (!acc.eventTypeAcc (event_info, xAOD::EventInfo::IS_SIMULATION))
    return m_currentScaleVariation_data;
  if (m_currentScalePredicate(*this,p))
    return m_currentScaleVariation_MC;
  else
    return egEnergyCorr::Scale::None;
}

egEnergyCorr::Resolution::Variation
EgammaCalibrationAndSmearingTool::oldtool_resolution_flag_this_event(
    columnar::EgammaId, columnar::EventInfoId event_info) const {
  const Accessors& acc = *m_accessors;
  return acc.eventTypeAcc (event_info, xAOD::EventInfo::IS_SIMULATION)
             ? m_currentResolutionVariation_MC
             : m_currentResolutionVariation_data;
}

double EgammaCalibrationAndSmearingTool::getElectronMomentum(
    const xAOD::Electron* el, const xAOD::EventInfo* event_info) {
  PATCore::ParticleDataType::DataType dataType =
      (event_info->eventType(xAOD::EventInfo::IS_SIMULATION))
          ? m_simulation
          : PATCore::ParticleDataType::Data;

  const xAOD::TrackParticle* eTrack = el->trackParticle();

  // track momentum and eta
  const float el_tracketa = eTrack->eta();
  const float el_trackmomentum = eTrack->pt() * cosh(el->eta());

  return m_rootTool->getCorrectedMomentum(
      dataType, PATCore::ParticleType::Electron, el_trackmomentum, el_tracketa,
      oldtool_scale_flag_this_event(*el, *event_info), m_varSF);
}

bool EgammaCalibrationAndSmearingTool::isAffectedBySystematic(
    const CP::SystematicVariation& systematic) const {
  CP::SystematicSet sys = affectingSystematics();
  return sys.find(systematic) != sys.end();
}

CP::SystematicSet EgammaCalibrationAndSmearingTool::affectingSystematics()
    const {
  CP::SystematicSet affecting_systematics;
  for (const auto& it : m_syst_description) {
    affecting_systematics.insert(it.first);
  }
  for (const auto& it : m_syst_description_resolution) {
    affecting_systematics.insert(it.first);
  }

  return affecting_systematics;
}

void EgammaCalibrationAndSmearingTool::setupSystematics() {
  const EgammaPredicate always = [](const EgammaCalibrationAndSmearingTool&, columnar::EgammaId) { return true; };

  // Try to simplify a bit for the ones that are fully correlate in eta,
  // whatever the model and that are not included in the macros including
  // - ADC non linearity
  // - L2Gain
  // - Leakage
  // - Conversion related
  // - TopoCluster threshold
  // - AF2
  // - PS_BARREL_B12
  // - S12EXTRALASTETABINRUN2
  // - ZEESTAT
  // - Run3 pre OFC + EXTRA
  if (m_decorrelation_model_scale == ScaleDecorrelation::FULL_ETA_CORRELATED ||
      m_decorrelation_model_scale == ScaleDecorrelation::FULL) {
    // Electron leakage, ADCLin, convReco only in final run2 recommendations
    if (m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 ||
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 ||
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      // systematic related to ADC non linearity correction. Before 2022, there
      // was not correction, nor related systematic
      if (m_doADCLinearityCorrection) {
        m_syst_description[CP::SystematicVariation("EG_SCALE_ADCLIN", +1)] =
            SysInfo{always, egEnergyCorr::Scale::ADCLinUp};
        m_syst_description[CP::SystematicVariation("EG_SCALE_ADCLIN", -1)] =
            SysInfo{always, egEnergyCorr::Scale::ADCLinDown};
      }
      // Gain splitted uncertainty
      m_syst_description[CP::SystematicVariation("EG_SCALE_L2MEDIUMGAIN", +1)] =
          SysInfo{always, egEnergyCorr::Scale::L2MediumGainUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_L2MEDIUMGAIN", -1)] =
          SysInfo{always, egEnergyCorr::Scale::L2MediumGainDown};
      m_syst_description[CP::SystematicVariation("EG_SCALE_L2LOWGAIN", +1)] =
          SysInfo{always, egEnergyCorr::Scale::L2LowGainUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_L2LOWGAIN", -1)] =
          SysInfo{always, egEnergyCorr::Scale::L2LowGainDown};

      // Electron leakage
      m_syst_description[CP::SystematicVariation("EG_SCALE_LEAKAGEELEC", +1)] =
          SysInfo{always, egEnergyCorr::Scale::LeakageElecUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_LEAKAGEELEC", -1)] =
          SysInfo{always, egEnergyCorr::Scale::LeakageElecDown};

      // Conversion related
      m_syst_description[CP::SystematicVariation("PH_SCALE_CONVRECO", +1)] =
          SysInfo{always, egEnergyCorr::Scale::ConvRecoUp};
      m_syst_description[CP::SystematicVariation("PH_SCALE_CONVRECO", -1)] =
          SysInfo{always, egEnergyCorr::Scale::ConvRecoDown};
    }
    // The equivalent of convReco (convefficiency and convfakerate) for other
    // models
    else {
      m_syst_description[CP::SystematicVariation("PH_SCALE_CONVEFFICIENCY",
                                                 +1)] =
          SysInfo{always, egEnergyCorr::Scale::ConvEfficiencyUp};
      m_syst_description[CP::SystematicVariation("PH_SCALE_CONVEFFICIENCY",
                                                 -1)] =
          SysInfo{always, egEnergyCorr::Scale::ConvEfficiencyDown};
      m_syst_description[CP::SystematicVariation("PH_SCALE_CONVFAKERATE", +1)] =
          SysInfo{always, egEnergyCorr::Scale::ConvFakeRateUp};
      m_syst_description[CP::SystematicVariation("PH_SCALE_CONVFAKERATE", -1)] =
          SysInfo{always, egEnergyCorr::Scale::ConvFakeRateDown};
    }

    // additional systematics for R22 OFC and MC21 pre and bulk
    if (m_TESModel == egEnergyCorr::es2022_R22_PRE) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_OFC", +1)] =
          SysInfo{always, egEnergyCorr::Scale::OFCUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_OFC", -1)] =
          SysInfo{always, egEnergyCorr::Scale::OFCDown};

      m_syst_description[CP::SystematicVariation("EG_SCALE_EXTRARUN3PRE", +1)] =
          SysInfo{always, egEnergyCorr::Scale::EXTRARUN3PREUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_EXTRARUN3PRE", -1)] =
          SysInfo{always, egEnergyCorr::Scale::EXTRARUN3PREDown};
    }

    if (m_TESModel == egEnergyCorr::es2017_R21_v0 or
        m_TESModel == egEnergyCorr::es2017_R21_v1 or
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v1 or
        m_TESModel == egEnergyCorr::es2022_R22_PRE or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {

      // topo clustr threshold systematics aded to release 21 recommendations
      m_syst_description[CP::SystematicVariation("EG_SCALE_TOPOCLUSTER_THRES",
                                                 +1)] =
          SysInfo{always, egEnergyCorr::Scale::topoClusterThresUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_TOPOCLUSTER_THRES",
                                                 -1)] =
          SysInfo{always, egEnergyCorr::Scale::topoClusterThresDown};

      // AF3 for run3 models: es2022_R22_PRE and esmodel >= es2023_R22_Run2_v1
      // although we prevent AF for es2022_R22_PRE and es2024_Run3_ofc0_v0
      // AF3 are still technically added in the tool
      // but normally uncertainty will be 0
      if (m_TESModel >= egEnergyCorr::es2022_R22_PRE) {
        m_syst_description[CP::SystematicVariation("EG_SCALE_AF3", +1)] =
            SysInfo{always, egEnergyCorr::Scale::afUp};
        m_syst_description[CP::SystematicVariation("EG_SCALE_AF3", -1)] =
            SysInfo{always, egEnergyCorr::Scale::afDown};
      }
      else {
      // and extra AF2 systematics for release 21 recommendations - Moriond 2018
      // - pending proper AF2 to FullSim correction with release 21
      m_syst_description[CP::SystematicVariation("EG_SCALE_AF2", +1)] =
          SysInfo{always, egEnergyCorr::Scale::afUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_AF2", -1)] =
          SysInfo{always, egEnergyCorr::Scale::afDown};
      }
    }

    // PS correlated barrel uncertainty
    if (m_TESModel == egEnergyCorr::es2017_summer_final or
        m_TESModel == egEnergyCorr::es2017_R21_v0 or
        m_TESModel == egEnergyCorr::es2017_R21_v1 or
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v1 or
        m_TESModel == egEnergyCorr::es2022_R22_PRE) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_PS_BARREL_B12",
                                                 +1)] =
          SysInfo{always, egEnergyCorr::Scale::PSb12Up};
      m_syst_description[CP::SystematicVariation("EG_SCALE_PS_BARREL_B12",
                                                 -1)] =
          SysInfo{always, egEnergyCorr::Scale::PSb12Down};
    }

    // additional systematic for S12 last eta bin run2
    if (m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2015_5TeV) {
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_S12EXTRALASTETABINRUN2", +1)] =
          SysInfo{always, egEnergyCorr::Scale::S12ExtraLastEtaBinRun2Up};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_S12EXTRALASTETABINRUN2", -1)] =
          SysInfo{always, egEnergyCorr::Scale::S12ExtraLastEtaBinRun2Down};
    }

    // Zee stat, if for FULL we do not ask for m_use_full_statistical_error
    if (m_decorrelation_model_scale ==
            ScaleDecorrelation::FULL_ETA_CORRELATED or
        !m_use_full_statistical_error) {
      // return 1 variation only, fully correlated in eta, equal to the correct
      // value but scaled by sqrt(number of bins) the scaling is done by the old
      // tool
      m_syst_description[CP::SystematicVariation("EG_SCALE_ZEESTAT", +1)] =
          SysInfo{always, egEnergyCorr::Scale::ZeeStatUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_ZEESTAT", -1)] =
          SysInfo{always, egEnergyCorr::Scale::ZeeStatDown};
    }
  }
  if (m_decorrelation_model_scale == ScaleDecorrelation::ONENP) {
    // TODO: independet implementation of ALL UP looping on all the variations
    m_syst_description[CP::SystematicVariation("EG_SCALE_ALL", +1)] =
        SysInfo{always, egEnergyCorr::Scale::AllUp};
    m_syst_description[CP::SystematicVariation("EG_SCALE_ALL", -1)] =
        SysInfo{always, egEnergyCorr::Scale::AllDown};

    // to be consistent with other schemes, we add
    // extra AF systematics in addition to the 1NP
    if (m_TESModel == egEnergyCorr::es2017_R21_v0 ||
        m_TESModel == egEnergyCorr::es2017_R21_v1 ||
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 ||
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 || 
        m_TESModel == egEnergyCorr::es2018_R21_v0 ||
        m_TESModel == egEnergyCorr::es2018_R21_v1) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_AF2", +1)] =
          SysInfo{always, egEnergyCorr::Scale::afUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_AF2", -1)] =
          SysInfo{always, egEnergyCorr::Scale::afDown};
    }
    else if (m_TESModel >= egEnergyCorr::es2022_R22_PRE) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_AF3", +1)] =
          SysInfo{always, egEnergyCorr::Scale::afUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_AF3", -1)] =
          SysInfo{always, egEnergyCorr::Scale::afDown};
    }
  } 
  else if (m_decorrelation_model_scale ==
             ScaleDecorrelation::FULL_ETA_CORRELATED) {
// all the physical effects separately, considered as fully correlated in eta
  if (m_TESModel == egEnergyCorr::es2024_Run3_v0) {
#define SYSMACRO(name, fullcorrelated, decorrelation, flagup, flagdown) \
  m_syst_description[CP::SystematicVariation(#name, +1)] =              \
      SysInfo{always, flagup};                                          \
  m_syst_description[CP::SystematicVariation(#name, -1)] =              \
      SysInfo{always, flagdown};
#include "ElectronPhotonFourMomentumCorrection/systematics_es2024_Run3_v0.def"
#undef SYSMACRO
  }
  else {
// common systematics for all the esmodels
#define SYSMACRO(name, fullcorrelated, decorrelation, flagup, flagdown) \
  m_syst_description[CP::SystematicVariation(#name, +1)] =              \
      SysInfo{always, flagup};                                          \
  m_syst_description[CP::SystematicVariation(#name, -1)] =              \
      SysInfo{always, flagdown};
#include "ElectronPhotonFourMomentumCorrection/systematics_S12_2022.def"
#undef SYSMACRO
  }

    if (m_TESModel != egEnergyCorr::es2023_R22_Run2_v0 &&
        m_TESModel != egEnergyCorr::es2023_R22_Run2_v1 &&
        m_TESModel != egEnergyCorr::es2024_Run3_v0) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_LARCALIB", +1)] =
          SysInfo{always, egEnergyCorr::Scale::LArCalibUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_LARCALIB", -1)] =
          SysInfo{always, egEnergyCorr::Scale::LArCalibDown};
      m_syst_description[CP::SystematicVariation("EG_SCALE_L2GAIN", +1)] =
          SysInfo{always, egEnergyCorr::Scale::L2GainUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_L2GAIN", -1)] =
          SysInfo{always, egEnergyCorr::Scale::L2GainDown};
    }

    // additional systematics for S12 run2
    if (m_TESModel == egEnergyCorr::es2015PRE_res_improved or
        m_TESModel == egEnergyCorr::es2015PRE or
        m_TESModel == egEnergyCorr::es2015cPRE or
        m_TESModel == egEnergyCorr::es2015c_summer or
        m_TESModel == egEnergyCorr::es2016PRE or
        m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2015_5TeV) {
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARCALIB_EXTRA2015PRE", +1)] =
          SysInfo{always, egEnergyCorr::Scale::LArCalibExtra2015PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARCALIB_EXTRA2015PRE", -1)] =
          SysInfo{always, egEnergyCorr::Scale::LArCalibExtra2015PreDown};
    }

    // additional systematics for temperature run1->run2
    if (m_TESModel == egEnergyCorr::es2015PRE_res_improved or
        m_TESModel == egEnergyCorr::es2015PRE or
        m_TESModel == egEnergyCorr::es2015cPRE or
        m_TESModel == egEnergyCorr::es2015c_summer or
        m_TESModel == egEnergyCorr::es2016PRE) {
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2015PRE", +1)] =
          SysInfo{always, egEnergyCorr::Scale::LArTemperature2015PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2015PRE", -1)] =
          SysInfo{always, egEnergyCorr::Scale::LArTemperature2015PreDown};
    }

    // additional systematic for temperature 2015->2016
    if (m_TESModel == egEnergyCorr::es2016PRE) {
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2016PRE", +1)] =
          SysInfo{always, egEnergyCorr::Scale::LArTemperature2016PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2016PRE", -1)] =
          SysInfo{always, egEnergyCorr::Scale::LArTemperature2016PreDown};
    }

    // additional systematic for PP0 region
    if (m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2017_summer_final or
        m_TESModel == egEnergyCorr::es2015_5TeV or
        m_TESModel == egEnergyCorr::es2017_R21_v0 or
        m_TESModel == egEnergyCorr::es2017_R21_v1 or
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v1 or
        m_TESModel == egEnergyCorr::es2022_R22_PRE or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_MATPP0", +1)] =
          SysInfo{always, egEnergyCorr::Scale::MatPP0Up};
      m_syst_description[CP::SystematicVariation("EG_SCALE_MATPP0", -1)] =
          SysInfo{always, egEnergyCorr::Scale::MatPP0Down};
    }

    // systematic related to wtots1
    if (m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2017_summer_final or
        m_TESModel == egEnergyCorr::es2015_5TeV or
        m_TESModel == egEnergyCorr::es2017_R21_v0 or
        m_TESModel == egEnergyCorr::es2017_R21_v1 or
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v1 or
        m_TESModel == egEnergyCorr::es2022_R22_PRE or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_WTOTS1", +1)] =
          SysInfo{always, egEnergyCorr::Scale::Wtots1Up};
      m_syst_description[CP::SystematicVariation("EG_SCALE_WTOTS1", -1)] =
          SysInfo{always, egEnergyCorr::Scale::Wtots1Down};
    }

    // systematic for the scintillators
    if (m_TESModel == egEnergyCorr::es2015cPRE or
        m_TESModel == egEnergyCorr::es2015c_summer or
        m_TESModel == egEnergyCorr::es2016PRE or
        m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2017_summer_final or
        m_TESModel == egEnergyCorr::es2015_5TeV or
        m_TESModel == egEnergyCorr::es2017_R21_v0 or
        m_TESModel == egEnergyCorr::es2017_R21_v1 or
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v1 or
        m_TESModel == egEnergyCorr::es2022_R22_PRE or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      // scintillator systematics
      m_syst_description[CP::SystematicVariation("EG_SCALE_E4SCINTILLATOR",
                                                 +1)] =
          SysInfo{always, egEnergyCorr::Scale::E4ScintillatorUp};
      m_syst_description[CP::SystematicVariation("EG_SCALE_E4SCINTILLATOR",
                                                 -1)] =
          SysInfo{always, egEnergyCorr::Scale::E4ScintillatorDown};
    }

  } else if (m_decorrelation_model_scale ==
             ScaleDecorrelation::ONENP_PLUS_UNCONR) {
// qsum of all variations correlated 8/13 TeV + uncorrelated (additional
// systematics for 2015PRE or 2016) all the physical effects separately,
// considered as fully correlated in eta
// TODO: fix for es2017
#define SYSMACRO(name, fullcorrelated, decorrelation, flagup, flagdown) \
  m_syst_description[CP::SystematicVariation(#name, +1)] =              \
      SysInfo{always, flagup};                                          \
  m_syst_description[CP::SystematicVariation(#name, -1)] =              \
      SysInfo{always, flagdown};
#include "ElectronPhotonFourMomentumCorrection/systematics_1NPCOR_PLUS_UNCOR.def"
#undef SYSMACRO

    // additional systematic for S12 last eta bin run2 - not needed anymore for
    // last 20.7 model since it is part of bin per bin E1/E2 uncertainty in root
    // file
    if (m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved) {
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_S12EXTRALASTETABINRUN2", +1)] =
          SysInfo{always, egEnergyCorr::Scale::S12ExtraLastEtaBinRun2Up};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_S12EXTRALASTETABINRUN2", -1)] =
          SysInfo{always, egEnergyCorr::Scale::S12ExtraLastEtaBinRun2Down};
    }

  } else if (m_decorrelation_model_scale == ScaleDecorrelation::FULL) {
    using pairvector = std::vector<std::pair<double, double>>;
    const pairvector decorrelation_bins_BE = {{0., 1.45}, {1.52, 2.5}};
    const std::vector<double> decorrelation_edges_TWELVE = {
        0., 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4};
    std::vector<double> decorrelation_edges_MODULE = {
        0., 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.37, 1.52, 1.8};
    const std::vector<double> decorrelation_edges_MATERIAL = {0.0, 1.1, 1.5,
                                                              2.1, 2.5};
    std::vector<double> decorrelation_edges_S12_EXTRARUN3 = {
        0., 0.8, 1.5, 2.5};

    std::vector<double> decorrelation_edges_S12;
    // for es2018_R21_v1 : 4 eta bins for muon E1/E2 uncertainty correlation
    if (m_TESModel == egEnergyCorr::es2018_R21_v1) {
      decorrelation_edges_S12.resize(5);
      decorrelation_edges_S12 = {0., 1.35, 1.5, 2.4, 2.5};
    } else if (m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
               m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 or
               m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      decorrelation_edges_S12.resize(8);
      decorrelation_edges_S12 = {0., 0.6, 1.0, 1.35, 1.5, 1.8, 2.4, 2.5};
      //
      // PS scale from muons, so "crack" is a bit different
      decorrelation_edges_MODULE[7] = 1.4;
      decorrelation_edges_MODULE[8] = 1.5;
    }
    // for previous run 2 muon calibration with 20.7, 5 eta bins for E1/E2
    // uncertainty correlation
    else {
      decorrelation_edges_S12.resize(6);
      decorrelation_edges_S12 = {0., 0.6, 1.4, 1.5, 2.4, 2.5};
    }

    if (m_TESModel != egEnergyCorr::es2017_summer_final and
        m_TESModel != egEnergyCorr::es2017_R21_v0 and
        m_TESModel != egEnergyCorr::es2017_R21_v1 and
        m_TESModel != egEnergyCorr::es2017_R21_ofc0_v1 and
        m_TESModel != egEnergyCorr::es2024_Run3_ofc0_v0 and 
        m_TESModel != egEnergyCorr::es2018_R21_v0 and
        m_TESModel != egEnergyCorr::es2018_R21_v1 and
        m_TESModel != egEnergyCorr::es2022_R22_PRE and
        m_TESModel != egEnergyCorr::es2023_R22_Run2_v0 and
        m_TESModel != egEnergyCorr::es2023_R22_Run2_v1 and
        m_TESModel != egEnergyCorr::es2024_Run3_v0) {
#define SYSMACRO(name, fullcorrelated, decorrelation, flagup, flagdown)      \
  if (bool(fullcorrelated)) {                                                \
    m_syst_description[CP::SystematicVariation(#name, +1)] =                 \
        SysInfo{always, flagup};                                             \
    m_syst_description[CP::SystematicVariation(#name, -1)] =                 \
        SysInfo{always, flagdown};                                           \
  } else {                                                                   \
    int i = 0;                                                               \
    for (const auto& p : AbsEtaCaloPredicatesFactory(decorrelation)) {       \
      m_syst_description[CP::SystematicVariation(                            \
          #name "__ETABIN" + std::to_string(i), +1)] = SysInfo{p, flagup};   \
      m_syst_description[CP::SystematicVariation(                            \
          #name "__ETABIN" + std::to_string(i), -1)] = SysInfo{p, flagdown}; \
      i += 1;                                                                \
    }                                                                        \
  }
#include "ElectronPhotonFourMomentumCorrection/systematics.def"
#undef SYSMACRO
    } else if (m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
               m_TESModel == egEnergyCorr::es2023_R22_Run2_v1) {
#define SYSMACRO(name, fullcorrelated, decorrelation, flagup, flagdown)      \
  if (bool(fullcorrelated)) {                                                \
    m_syst_description[CP::SystematicVariation(#name, +1)] =                 \
        SysInfo{always, flagup};                                             \
    m_syst_description[CP::SystematicVariation(#name, -1)] =                 \
        SysInfo{always, flagdown};                                           \
  } else {                                                                   \
    int i = 0;                                                               \
    for (const auto& p : AbsEtaCaloPredicatesFactory(decorrelation)) {       \
      m_syst_description[CP::SystematicVariation(                            \
          #name "__ETABIN" + std::to_string(i), +1)] = SysInfo{p, flagup};   \
      m_syst_description[CP::SystematicVariation(                            \
          #name "__ETABIN" + std::to_string(i), -1)] = SysInfo{p, flagdown}; \
      i += 1;                                                                \
    }                                                                        \
  }
#include "ElectronPhotonFourMomentumCorrection/systematics_S12_2022.def"
#undef SYSMACRO
    } else if (m_TESModel == egEnergyCorr::es2024_Run3_v0) {
#define SYSMACRO(name, fullcorrelated, decorrelation, flagup, flagdown)      \
  if (bool(fullcorrelated)) {                                                \
    m_syst_description[CP::SystematicVariation(#name, +1)] =                 \
        SysInfo{always, flagup};                                             \
    m_syst_description[CP::SystematicVariation(#name, -1)] =                 \
        SysInfo{always, flagdown};                                           \
  } else {                                                                   \
    int i = 0;                                                               \
    for (const auto& p : AbsEtaCaloPredicatesFactory(decorrelation)) {       \
      m_syst_description[CP::SystematicVariation(                            \
          #name "__ETABIN" + std::to_string(i), +1)] = SysInfo{p, flagup};   \
      m_syst_description[CP::SystematicVariation(                            \
          #name "__ETABIN" + std::to_string(i), -1)] = SysInfo{p, flagdown}; \
      i += 1;                                                                \
    }                                                                        \
  }
#include "ElectronPhotonFourMomentumCorrection/systematics_es2024_Run3_v0.def"
#undef SYSMACRO
    } else {
#define SYSMACRO(name, fullcorrelated, decorrelation, flagup, flagdown)      \
  if (bool(fullcorrelated)) {                                                \
    m_syst_description[CP::SystematicVariation(#name, +1)] =                 \
        SysInfo{always, flagup};                                             \
    m_syst_description[CP::SystematicVariation(#name, -1)] =                 \
        SysInfo{always, flagdown};                                           \
  } else {                                                                   \
    int i = 0;                                                               \
    for (const auto& p : AbsEtaCaloPredicatesFactory(decorrelation)) {       \
      m_syst_description[CP::SystematicVariation(                            \
          #name "__ETABIN" + std::to_string(i), +1)] = SysInfo{p, flagup};   \
      m_syst_description[CP::SystematicVariation(                            \
          #name "__ETABIN" + std::to_string(i), -1)] = SysInfo{p, flagdown}; \
      i += 1;                                                                \
    }                                                                        \
  }
#include "ElectronPhotonFourMomentumCorrection/systematics_S12.def"
#undef SYSMACRO
    }  // else

    if (m_use_full_statistical_error) {
      // statistical error, decorrelate in *all* the bins
      int i = 0;
      const TAxis& axis_statistical_error(m_rootTool->get_ZeeStat_eta_axis());
      for (int ibin = 1; ibin <= axis_statistical_error.GetNbins(); ++ibin) {
        auto p = EtaCaloPredicateFactory(
            axis_statistical_error.GetBinLowEdge(ibin),
            axis_statistical_error.GetBinLowEdge(ibin + 1));
        m_syst_description[CP::SystematicVariation(
            "EG_SCALE_ZEESTAT__ETABIN" + std::to_string(i), +1)] =
            SysInfo{p, egEnergyCorr::Scale::ZeeStatUp};
        m_syst_description[CP::SystematicVariation(
            "EG_SCALE_ZEESTAT__ETABIN" + std::to_string(i), -1)] =
            SysInfo{p, egEnergyCorr::Scale::ZeeStatDown};
        ++i;
      }
    }

    // additional systematics for S12 run2
    if (m_TESModel == egEnergyCorr::es2015PRE_res_improved or
        m_TESModel == egEnergyCorr::es2015PRE or
        m_TESModel == egEnergyCorr::es2015cPRE or
        m_TESModel == egEnergyCorr::es2015c_summer or
        m_TESModel == egEnergyCorr::es2016PRE or
        m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2015_5TeV) {
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARCALIB_EXTRA2015PRE__ETABIN0", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory({0, 1.45}),
                  egEnergyCorr::Scale::LArCalibExtra2015PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARCALIB_EXTRA2015PRE__ETABIN0", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory({0, 1.45}),
                  egEnergyCorr::Scale::LArCalibExtra2015PreDown};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARCALIB_EXTRA2015PRE__ETABIN1", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory({1.45, 2.47}),
                  egEnergyCorr::Scale::LArCalibExtra2015PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARCALIB_EXTRA2015PRE__ETABIN1", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory({1.45, 2.47}),
                  egEnergyCorr::Scale::LArCalibExtra2015PreDown};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARCALIB_EXTRA2015PRE__ETABIN2", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory({2.47, 3.2}),
                  egEnergyCorr::Scale::LArCalibExtra2015PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARCALIB_EXTRA2015PRE__ETABIN2", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory({2.47, 3.2}),
                  egEnergyCorr::Scale::LArCalibExtra2015PreDown};
    }

    // additional systematics for temperature run1->run2
    if (m_TESModel == egEnergyCorr::es2015PRE_res_improved or
        m_TESModel == egEnergyCorr::es2015PRE or
        m_TESModel == egEnergyCorr::es2015cPRE or
        m_TESModel == egEnergyCorr::es2015c_summer or
        m_TESModel == egEnergyCorr::es2016PRE) {
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2015PRE__ETABIN0", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory(decorrelation_bins_BE[0]),
                  egEnergyCorr::Scale::LArTemperature2015PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2015PRE__ETABIN0", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory(decorrelation_bins_BE[0]),
                  egEnergyCorr::Scale::LArTemperature2015PreDown};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2015PRE__ETABIN1", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory(decorrelation_bins_BE[1]),
                  egEnergyCorr::Scale::LArTemperature2015PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2015PRE__ETABIN1", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory(decorrelation_bins_BE[1]),
                  egEnergyCorr::Scale::LArTemperature2015PreDown};
    }

    // additional systematic for temperature 2015->2016
    if (m_TESModel == egEnergyCorr::es2016PRE) {
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2016PRE__ETABIN0", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory(decorrelation_bins_BE[0]),
                  egEnergyCorr::Scale::LArTemperature2016PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2016PRE__ETABIN1", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory(decorrelation_bins_BE[1]),
                  egEnergyCorr::Scale::LArTemperature2016PreUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2016PRE__ETABIN0", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory(decorrelation_bins_BE[0]),
                  egEnergyCorr::Scale::LArTemperature2016PreDown};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_LARTEMPERATURE_EXTRA2016PRE__ETABIN1", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory(decorrelation_bins_BE[1]),
                  egEnergyCorr::Scale::LArTemperature2016PreDown};
    }

    // additional systematic for PP0 region
    if (m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2017_summer_final or
        m_TESModel == egEnergyCorr::es2015_5TeV or
        m_TESModel == egEnergyCorr::es2017_R21_v0 or
        m_TESModel == egEnergyCorr::es2017_R21_v1 or
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v1 or
        m_TESModel == egEnergyCorr::es2022_R22_PRE or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_MATPP0__ETABIN0",
                                                 +1)] = SysInfo{
          AbsEtaCaloPredicateFactory(0, 1.5), egEnergyCorr::Scale::MatPP0Up};
      m_syst_description[CP::SystematicVariation("EG_SCALE_MATPP0__ETABIN1",
                                                 +1)] = SysInfo{
          AbsEtaCaloPredicateFactory(1.5, 2.5), egEnergyCorr::Scale::MatPP0Up};
      m_syst_description[CP::SystematicVariation("EG_SCALE_MATPP0__ETABIN0",
                                                 -1)] = SysInfo{
          AbsEtaCaloPredicateFactory(0, 1.5), egEnergyCorr::Scale::MatPP0Down};
      m_syst_description[CP::SystematicVariation("EG_SCALE_MATPP0__ETABIN1",
                                                 -1)] =
          SysInfo{AbsEtaCaloPredicateFactory(1.5, 2.5),
                  egEnergyCorr::Scale::MatPP0Down};
    }

    // systematic related to wtots1
    if (m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2017_summer_final or
        m_TESModel == egEnergyCorr::es2015_5TeV or
        m_TESModel == egEnergyCorr::es2017_R21_v0 or
        m_TESModel == egEnergyCorr::es2017_R21_v1 or
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v1 or
        m_TESModel == egEnergyCorr::es2022_R22_PRE) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_WTOTS1", +1)] =
          SysInfo{always, egEnergyCorr::Scale::Wtots1Up};
      m_syst_description[CP::SystematicVariation("EG_SCALE_WTOTS1", -1)] =
          SysInfo{always, egEnergyCorr::Scale::Wtots1Down};
    }

    // systematic related to wtots1, decorrelate eta bin [1.52,1.82] from rest
    if (m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      m_syst_description[CP::SystematicVariation("EG_SCALE_WTOTS1__ETABIN0",
                                                 +1)] =
          SysInfo{DoubleOrAbsEtaCaloPredicate(0, 1.52, 1.82, 2.47),
                  egEnergyCorr::Scale::Wtots1Up};
      m_syst_description[CP::SystematicVariation("EG_SCALE_WTOTS1__ETABIN0",
                                                 -1)] =
          SysInfo{DoubleOrAbsEtaCaloPredicate(0, 1.52, 1.82, 2.47),
                  egEnergyCorr::Scale::Wtots1Down};
      m_syst_description[CP::SystematicVariation("EG_SCALE_WTOTS1__ETABIN1",
                                                 +1)] =
          SysInfo{AbsEtaCaloPredicateFactory({1.52, 1.82}),
                  egEnergyCorr::Scale::Wtots1Up};
      m_syst_description[CP::SystematicVariation("EG_SCALE_WTOTS1__ETABIN1",
                                                 -1)] =
          SysInfo{AbsEtaCaloPredicateFactory({1.52, 1.82}),
                  egEnergyCorr::Scale::Wtots1Down};
    }

    // systematic for the scintillators
    if (m_TESModel == egEnergyCorr::es2015cPRE or
        m_TESModel == egEnergyCorr::es2015c_summer or
        m_TESModel == egEnergyCorr::es2016PRE or
        m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2017_summer_final or
        m_TESModel == egEnergyCorr::es2015_5TeV or
        m_TESModel == egEnergyCorr::es2017_R21_v0 or
        m_TESModel == egEnergyCorr::es2017_R21_v1 or
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v1 or
        m_TESModel == egEnergyCorr::es2022_R22_PRE or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_E4SCINTILLATOR__ETABIN0", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory(1.4, 1.46),
                  egEnergyCorr::Scale::E4ScintillatorUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_E4SCINTILLATOR__ETABIN1", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory(1.46, 1.52),
                  egEnergyCorr::Scale::E4ScintillatorUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_E4SCINTILLATOR__ETABIN2", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory(1.52, 1.6),
                  egEnergyCorr::Scale::E4ScintillatorUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_E4SCINTILLATOR__ETABIN0", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory(1.4, 1.46),
                  egEnergyCorr::Scale::E4ScintillatorDown};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_E4SCINTILLATOR__ETABIN1", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory(1.46, 1.52),
                  egEnergyCorr::Scale::E4ScintillatorDown};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_E4SCINTILLATOR__ETABIN2", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory(1.52, 1.6),
                  egEnergyCorr::Scale::E4ScintillatorDown};
      if (m_TESModel == egEnergyCorr::es2024_Run3_v0) { // extended E4 in Run 3
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_E4SCINTILLATOR__ETABIN3", +1)] =
          SysInfo{AbsEtaCaloPredicateFactory(1.6, 1.72),
                  egEnergyCorr::Scale::E4ScintillatorUp};
      m_syst_description[CP::SystematicVariation(
          "EG_SCALE_E4SCINTILLATOR__ETABIN3", -1)] =
          SysInfo{AbsEtaCaloPredicateFactory(1.6, 1.72),
                  egEnergyCorr::Scale::E4ScintillatorDown};
      }
    }
  } else {
    ATH_MSG_FATAL("scale decorrelation model invalid");
  }

  // resolution systematics
  if (m_decorrelation_model_resolution == ResolutionDecorrelation::ONENP) {
    // ALL will not include AF2/AF3 systematic
    // individual AF NP is always provided
    // linghua.guo@cern.ch 2025-04-23
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_ALL", +1)] = egEnergyCorr::Resolution::AllUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_ALL", -1)] = egEnergyCorr::Resolution::AllDown;
  } else if (m_decorrelation_model_resolution ==
             ResolutionDecorrelation::FULL) {
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_ZSMEARING", +1)] = egEnergyCorr::Resolution::ZSmearingUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_ZSMEARING", -1)] =
        egEnergyCorr::Resolution::ZSmearingDown;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_SAMPLINGTERM", +1)] =
        egEnergyCorr::Resolution::SamplingTermUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_SAMPLINGTERM", -1)] =
        egEnergyCorr::Resolution::SamplingTermDown;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_MATERIALID", +1)] =
        egEnergyCorr::Resolution::MaterialIDUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_MATERIALID", -1)] =
        egEnergyCorr::Resolution::MaterialIDDown;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_MATERIALCALO", +1)] =
        egEnergyCorr::Resolution::MaterialCaloUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_MATERIALCALO", -1)] =
        egEnergyCorr::Resolution::MaterialCaloDown;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_MATERIALGAP", +1)] =
        egEnergyCorr::Resolution::MaterialGapUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_MATERIALGAP", -1)] =
        egEnergyCorr::Resolution::MaterialGapDown;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_MATERIALCRYO", +1)] =
        egEnergyCorr::Resolution::MaterialCryoUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_MATERIALCRYO", -1)] =
        egEnergyCorr::Resolution::MaterialCryoDown;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_PILEUP", +1)] = egEnergyCorr::Resolution::PileUpUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_PILEUP", -1)] = egEnergyCorr::Resolution::PileUpDown;
    if (m_TESModel == egEnergyCorr::es2017 or
        m_TESModel == egEnergyCorr::es2017_summer or
        m_TESModel == egEnergyCorr::es2017_summer_improved or
        m_TESModel == egEnergyCorr::es2017_summer_final or
        m_TESModel == egEnergyCorr::es2015_5TeV or
        m_TESModel == egEnergyCorr::es2017_R21_v0 or
        m_TESModel == egEnergyCorr::es2017_R21_v1 or
        m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v0 or
        m_TESModel == egEnergyCorr::es2018_R21_v1 or
        m_TESModel == egEnergyCorr::es2022_R22_PRE or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 or
        m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 or
        m_TESModel == egEnergyCorr::es2024_Run3_v0) {
      m_syst_description_resolution[CP::SystematicVariation(
          "EG_RESOLUTION_MATERIALIBL", +1)] =
          egEnergyCorr::Resolution::MaterialIBLUp;
      m_syst_description_resolution[CP::SystematicVariation(
          "EG_RESOLUTION_MATERIALIBL", -1)] =
          egEnergyCorr::Resolution::MaterialIBLDown;
      m_syst_description_resolution[CP::SystematicVariation(
          "EG_RESOLUTION_MATERIALPP0", +1)] =
          egEnergyCorr::Resolution::MaterialPP0Up;
      m_syst_description_resolution[CP::SystematicVariation(
          "EG_RESOLUTION_MATERIALPP0", -1)] =
          egEnergyCorr::Resolution::MaterialPP0Down;
      
      if (m_TESModel == egEnergyCorr::es2022_R22_PRE) {  // exta sys. for Run-3
                                                         // pre-recommendations
        m_syst_description_resolution[CP::SystematicVariation(
            "EG_RESOLUTION_OFC", +1)] = egEnergyCorr::Resolution::OFCUp;
        m_syst_description_resolution[CP::SystematicVariation(
            "EG_RESOLUTION_OFC", -1)] = egEnergyCorr::Resolution::OFCDown;
      }
    }
  } else {
    ATH_MSG_FATAL("resolution decorrelation model invalid");
  }

  // Always use individual AF2/AF3 systematics for resolution
  if (m_TESModel == egEnergyCorr::es2017_R21_v1 ||
      m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 ||
      m_TESModel == egEnergyCorr::es2018_R21_v0 ||
      m_TESModel == egEnergyCorr::es2018_R21_v1) {
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_AF2", +1)] = egEnergyCorr::Resolution::afUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_AF2", -1)] = egEnergyCorr::Resolution::afDown;
  }
  else if (m_TESModel >= egEnergyCorr::es2022_R22_PRE){
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_AF3", +1)] = egEnergyCorr::Resolution::afUp;
    m_syst_description_resolution[CP::SystematicVariation(
        "EG_RESOLUTION_AF3", -1)] = egEnergyCorr::Resolution::afDown;        
  }

  // ep combination systematics
  if (m_use_ep_combination) {
    m_syst_description[CP::SystematicVariation("EL_SCALE_MOMENTUM", +1)] =
        SysInfo{always, egEnergyCorr::Scale::MomentumUp};
    m_syst_description[CP::SystematicVariation("EL_SCALE_MOMENTUM", -1)] =
        SysInfo{always, egEnergyCorr::Scale::MomentumDown};
  }
}

CP::SystematicSet EgammaCalibrationAndSmearingTool::recommendedSystematics()
    const {
  return affectingSystematics();
}

StatusCode EgammaCalibrationAndSmearingTool::applySystematicVariation(
    const CP::SystematicSet& systConfig) {

  // set the nominal one (no systematics)
  m_currentScaleVariation_MC = egEnergyCorr::Scale::None;
  m_currentScaleVariation_data = m_doScaleCorrection
                                     ? egEnergyCorr::Scale::Nominal
                                     : egEnergyCorr::Scale::None;
  m_currentResolutionVariation_MC = m_doSmearing
                                        ? egEnergyCorr::Resolution::Nominal
                                        : egEnergyCorr::Resolution::None;
  m_currentResolutionVariation_data = egEnergyCorr::Resolution::None;
  m_currentScalePredicate = [](const EgammaCalibrationAndSmearingTool&, columnar::EgammaId) { return true; };

  if (systConfig.empty())
    return StatusCode::SUCCESS;

  // the following code allows only ONE systematic variation at a time (1 for
  // scale, 1 for resolution)

  bool first_scale = true;
  bool first_resolution = true;
  for (const auto& it : systConfig) {
    const auto found_scale = m_syst_description.find(it);
    if (found_scale != m_syst_description.end()) {
      if (not first_scale) {
        ATH_MSG_ERROR("multiple scale variations not supported");
        throw std::runtime_error("multiple scale variations not supported");
      }
      first_scale = false;
      m_currentScaleVariation_MC = found_scale->second.effect;
      m_currentScalePredicate = found_scale->second.predicate;
    }

    const auto found_resolution = m_syst_description_resolution.find(it);
    if (found_resolution != m_syst_description_resolution.end()) {
      if (not first_resolution) {
        ATH_MSG_ERROR("multiple resolution variations not supported");
        throw std::runtime_error(
            "multiple resolution variations not supported");
      }
      first_resolution = false;
      m_currentResolutionVariation_MC = found_resolution->second;
    }
  }

  return StatusCode::SUCCESS;
}

double EgammaCalibrationAndSmearingTool::intermodule_correction(
    double Ecl, double phi, double eta) const {

  // Intermodule Widening Correction: E_corr = E / (a' - b' * ((1 / (1 +
  // exp((phi_mod - 2 * pi / 32) * c))) * (1 / (1 + exp((phi_mod - 2 * pi / 32)
  // * (d)))))) (phi_min, phi_max) : [a' = a / a, b' = b / a, c, d]

  double Ecl_corr = 0.;
  int DivInt = 0;
  double pi = M_PI;

  if (m_TESModel == egEnergyCorr::es2017_summer_improved ||
      m_TESModel == egEnergyCorr::es2017_summer_final ||
      m_TESModel == egEnergyCorr::es2017_R21_v0 ||
      m_TESModel == egEnergyCorr::es2017_R21_v1 ||
      m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 ||
      m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 ||
      m_TESModel == egEnergyCorr::es2018_R21_v0 ||
      m_TESModel == egEnergyCorr::es2018_R21_v1 ||
      m_TESModel == egEnergyCorr::es2022_R22_PRE ||
      m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 ||
      m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 ||
      m_TESModel == egEnergyCorr::es2024_Run3_v0) {

    double phi_mod = 0;
    if (phi < 0)
      phi_mod = fmod(phi, 2 * pi / 16.) + pi / 8.;
    else
      phi_mod = fmod(phi, 2 * pi / 16.);

    //  The correction concerns only the barrel
    if (std::abs(eta) <= 1.37) {

      if (phi < (-7 * pi / 8) && phi > (-1 * pi))
        Ecl_corr =
            Ecl /
            (1 - 0.1086 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 175.2759))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-189.3612))))));
      if (phi < (-6 * pi / 8) && phi > (-7 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.0596 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 170.8305))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-233.3782))))));
      if (phi < (-5 * pi / 8) && phi > (-6 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.0596 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 147.1451))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-139.3386))))));
      if (phi < (-4 * pi / 8) && phi > (-5 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.0583 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 168.4644))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-246.2897))))));
      if (phi < (-3 * pi / 8) && phi > (-4 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.0530 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 177.6703))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-198.3227))))));
      if (phi < (-2 * pi / 8) && phi > (-3 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.0672 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 145.0693))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-242.1771))))));
      if (phi < (-1 * pi / 8) && phi > (-2 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.0871 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 132.3303))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-166.1833))))));
      if (phi < (0 * pi / 8) && phi > (-1 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.0948 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 127.6780))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-150.0700))))));
      if (phi < (1 * pi / 8) && phi > (0 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.1166 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 172.0679))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-235.3293))))));
      if (phi < (2 * pi / 8) && phi > (1 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.1172 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 190.3524))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-198.9400))))));
      if (phi < (3 * pi / 8) && phi > (2 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.1292 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 158.0540))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-165.3893))))));
      if (phi < (4 * pi / 8) && phi > (3 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.1557 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 162.2793))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-133.5131))))));
      if (phi < (5 * pi / 8) && phi > (4 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.1659 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 180.5270))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-168.5074))))));
      if (phi < (6 * pi / 8) && phi > (5 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.1123 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 128.2277))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-154.4455))))));
      if (phi < (7 * pi / 8) && phi > (6 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.1394 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 192.1216))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-198.0727))))));
      if (phi < (8 * pi / 8) && phi > (7 * pi / 8))
        Ecl_corr =
            Ecl /
            (1 - 0.1001 *
                     ((1 / (1 + exp((phi_mod - 2 * pi / 32.) * 199.1735))) *
                      (1 / (1 + exp((phi_mod - 2 * pi / 32.) * (-176.4056))))));
    }

    //  No correction for the EC
    else {
      Ecl_corr = Ecl;
    }

  }

  else {

    //  Definitions of module folding into four quarters (top, left, bottom and
    //  right)

    DivInt = (int)(phi / ((2 * pi) / 16.));
    double phi_mod = phi - DivInt * (2 * pi / 16.);

    //  Centring on the intermodule --> phi_mod will now be in [0,0.4]
    if (phi_mod < 0)
      phi_mod += pi / 8.;

    //  The correction concerns only the barrel
    if (std::abs(eta) <= 1.4) {

      //  Top quarter
      if (phi < (3 * pi) / 4. && phi >= pi / 4.) {
        Ecl_corr =
            Ecl / (1 - 0.131 * ((1 / (1 + exp((phi_mod - 0.2) * 199.08))) *
                                (1 / (1 + exp((phi_mod - 0.2) * (-130.36))))));
      }

      //  Right quarter
      if (phi < pi / 4. && phi >= -pi / 4.) {
        Ecl_corr =
            Ecl / (1 - 0.0879 * ((1 / (1 + exp((phi_mod - 0.2) * 221.01))) *
                                 (1 / (1 + exp((phi_mod - 0.2) * (-149.51))))));
      }
      //  Bottom quarter
      if (phi < -pi / 4. && phi >= (-3 * pi) / 4.) {
        Ecl_corr =
            Ecl / (1 - 0.0605 * ((1 / (1 + exp((phi_mod - 0.2) * 281.37))) *
                                 (1 / (1 + exp((phi_mod - 0.2) * (-170.29))))));
      }
      //  Left quarter
      if ((phi < (-3 * pi) / 4.) || (phi >= (3 * pi) / 4.)) {
        Ecl_corr =
            Ecl / (1 - 0.102 * ((1 / (1 + exp((phi_mod - 0.2) * 235.37))) *
                                (1 / (1 + exp((phi_mod - 0.2) * (-219.04))))));
      }
    }

    //  No correction for the EC
    else {
      Ecl_corr = Ecl;
    }
  }

  return Ecl_corr;
}

double EgammaCalibrationAndSmearingTool::correction_phi_unif(double eta,
                                                             double phi) const {
  constexpr double PI = M_PI;
  double Fcorr = 1.0;

  if (m_use_mapping_correction) {
    // wrong mapping HV -> sectors in run1
    if (eta < -0.4 && eta > -0.6) {
      if (phi < (14 * PI / 32.) && phi > (13 * PI / 32.)) {
        Fcorr += 0.035;
      } else if (phi < (13 * PI / 32.) && phi > (12 * PI / 32.)) {
        Fcorr -= 0.035;
      }
    }
  }

  if (m_TESModel == egEnergyCorr::es2017_summer_improved ||
      m_TESModel == egEnergyCorr::es2017_summer_final ||
      m_TESModel == egEnergyCorr::es2017_R21_v0 ||
      m_TESModel == egEnergyCorr::es2017_R21_v1 or
      m_TESModel == egEnergyCorr::es2017_R21_ofc0_v1 ||
      m_TESModel == egEnergyCorr::es2024_Run3_ofc0_v0 ||
      m_TESModel == egEnergyCorr::es2018_R21_v0 ||
      m_TESModel == egEnergyCorr::es2018_R21_v1 ||
      m_TESModel == egEnergyCorr::es2022_R22_PRE ||
      m_TESModel == egEnergyCorr::es2023_R22_Run2_v0 ||
      m_TESModel == egEnergyCorr::es2023_R22_Run2_v1 ||
      m_TESModel == egEnergyCorr::es2024_Run3_v0) {

    if (eta < 0.2 && eta > 0.) {
      if (phi < (-7 * 2 * PI / 32.) && phi > (-8 * 2 * PI / 32.)) {
        Fcorr = 1.016314;
      }
    }

    else if (eta < 0.6 && eta > 0.4) {
      if (phi < 0 && phi > (-2 * PI / 32.)) {
        Fcorr = 1.041591;
      } else if (phi < (-4 * 2 * PI / 32.) && phi > (-5 * 2 * PI / 32.)) {
        Fcorr = 1.067346;
      }
    }

    else if (eta < 0.8 && eta > 0.6) {
      if (phi < (7 * 2 * PI / 32.) && phi > (6 * 2 * PI / 32.)) {
        Fcorr = 1.027980;
      }
    }

    else if (eta < 1.4 && eta > 1.2) {
      if (phi < (-9 * 2 * PI / 32.) && phi > (-10 * 2 * PI / 32.)) {
        Fcorr = 1.020299;
      } else if (phi < (-11 * 2 * PI / 32.) && phi > (-12 * 2 * PI / 32.)) {
        Fcorr = 1.051426;
      }
    }

    else if (eta < 2.3 && eta > 2.1) {
      if (phi < (-12 * 2 * PI / 32.) && phi > (-13 * 2 * PI / 32.)) {
        Fcorr = 1.071695;
      }
    }

    else if (eta < 0. && eta > -0.2) {
      if (phi < (-12 * 2 * PI / 32.) && phi > (-13 * 2 * PI / 32.)) {
        Fcorr = 1.008227;
      } else if (phi < (-8 * 2 * PI / 32.) && phi > (-9 * 2 * PI / 32.)) {
        Fcorr = 1.013929;
      }
    }

    else if (eta < -0.2 && eta > -0.4) {
      if (phi < (-9 * 2 * PI / 32.) && phi > (-10 * 2 * PI / 32.)) {
        Fcorr = 1.015749;
      }
    }

    else if (eta < -1.2 && eta > -1.4) {
      if (phi < (-6 * 2 * PI / 32.) && phi > (-7 * 2 * PI / 32.)) {
        Fcorr = 1.064954;
      }
    }

    else if (eta < -1.6 && eta > -1.8) {
      if (phi < (9 * 2 * PI / 32.) && phi > (8 * 2 * PI / 32.)) {
        Fcorr = 1.027448;
      }
    }

    else if (eta < -2.3 && eta > -2.5) {
      if (phi < (-8 * 2 * PI / 32.) && phi > (-9 * 2 * PI / 32.)) {
        Fcorr = 1.025882;
      } else if (phi < (5 * 2 * PI / 32.) && phi > (4 * 2 * PI / 32.)) {
        Fcorr = 1.036616;
      } else if (phi < (9 * 2 * PI / 32.) && phi > (8 * 2 * PI / 32.)) {
        Fcorr = 1.053838;
      } else if (phi < (10 * 2 * PI / 32.) && phi > (9 * 2 * PI / 32.)) {
        Fcorr = 1.026856;
      } else if (phi < (11 * 2 * PI / 32.) && phi > (10 * 2 * PI / 32.)) {
        Fcorr = 0.994382;
      }
    }

  }  // es2017_summer_improved end

  else {
    if (eta < 0.6 && eta > 0.4) {
      if (phi < 0 && phi > (-2 * PI / 32.)) {
        Fcorr = 1.028;
      } else if (phi < (-4 * 2 * PI / 32.) && phi > (-5 * 2 * PI / 32.)) {
        Fcorr = 1.044;
      }
    }

    else if (eta < 0.8 && eta > 0.6) {
      if (phi < (7 * 2 * PI / 32.) && phi > (6 * 2 * PI / 32.)) {
        Fcorr = 1.022;
      }
    }

    else if (eta < 1.4 && eta > 1.2) {
      if (phi < (-11 * 2 * PI / 32.) && phi > (-12 * 2 * PI / 32.)) {
        Fcorr = 1.038;
      }
    }

    else if (eta < 2.0 && eta > 1.9) {
      if (phi < (10 * 2 * PI / 32.) && phi > (9 * 2 * PI / 32.)) {
        Fcorr = 1.029;
      }
    }

    else if (eta < -1.2 && eta > -1.4) {
      if (phi < (-4 * 2 * PI / 32.) && phi > (-5 * 2 * PI / 32.)) {
        Fcorr = 1.048;
      } else if (phi < (-6 * 2 * PI / 32.) && phi > (-7 * 2 * PI / 32.)) {
        Fcorr = 1.048;
      }
    }

    else if (eta < -1.6 && eta > -1.8) {
      if (phi < (9 * 2 * PI / 32.) && phi > (8 * 2 * PI / 32.)) {
        Fcorr = 1.024;
      }
    }

    else if (eta < -2.3 && eta > -2.5) {
      if (phi < (-8 * 2 * PI / 32.) && phi > (-9 * 2 * PI / 32.)) {
        Fcorr = 1.037;
      } else if (phi < (5 * 2 * PI / 32.) && phi > (4 * 2 * PI / 32.)) {
        Fcorr = 1.031;
      } else if (phi < (9 * 2 * PI / 32.) && phi > (8 * 2 * PI / 32.)) {
        Fcorr = 1.040;
      } else if (phi < (10 * 2 * PI / 32.) && phi > (9 * 2 * PI / 32.)) {
        Fcorr = 1.030;
      } else if (phi < (11 * 2 * PI / 32.) && phi > (10 * 2 * PI / 32.)) {
        Fcorr = 1.020;
      }
    }
  }

  return Fcorr;
}

void EgammaCalibrationAndSmearingTool ::
callSingleEvent (columnar::MutableEgammaRange egammas, columnar::EventInfoId event) const
{
  for (auto egamma : egammas) {
    if (applyCorrection (egamma, event) != CP::CorrectionCode::Ok)
      throw std::runtime_error ("EgammaCalibrationAndSmearingTool::callEvents: apply failed");
  }
}

void EgammaCalibrationAndSmearingTool ::
callEvents (columnar::EventContextRange events) const
{
  const Accessors& acc = *m_accessors;
  for (auto event : events) {
    auto eventInfo = acc.m_eventHandle(event);
    callSingleEvent (acc.m_egammaHandle(event), eventInfo);
  }
}

}  // namespace CP
