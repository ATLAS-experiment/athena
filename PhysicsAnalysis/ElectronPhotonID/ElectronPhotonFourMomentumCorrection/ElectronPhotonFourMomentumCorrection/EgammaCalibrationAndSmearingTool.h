/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMA_CALIB_TOOL_H_
#define EGAMMA_CALIB_TOOL_H_

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "AsgMessaging/AsgMessaging.h"
#include "AsgServices/ServiceHandle.h"
#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AthContainers/ConstAccessor.h"
#include "EgammaAnalysisInterfaces/IEgammaCalibrationAndSmearingTool.h"
#include "EgammaAnalysisInterfaces/IegammaMVASvc.h"
#include "ElectronPhotonFourMomentumCorrection/egammaEnergyCorrectionTool.h"
#include "PATInterfaces/ISystematicsTool.h"
#include "PATInterfaces/SystematicSet.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODEventInfo/EventInfo.h"

// Forward declarations
class egammaLayerRecalibTool;
namespace egGain {
class GainTool;
class GainUncertainty;
}  // namespace egGain
class LinearityADC;
class TH2;

namespace xAOD {
inline float get_phi_calo(const xAOD::CaloCluster& cluster, int author,
                          bool do_throw = false) {
  static const SG::ConstAccessor<float> phiCaloAcc("phiCalo");
  double phi_calo;
  if (author == xAOD::EgammaParameters::AuthorFwdElectron) {
    phi_calo = cluster.phi();
  } else if (cluster.retrieveMoment(xAOD::CaloCluster::PHICALOFRAME,
                                    phi_calo)) {
  } else if (phiCaloAcc.isAvailable(cluster)) {
    phi_calo = phiCaloAcc(cluster);
  } else {
    asg::AsgMessaging msg("get_phi_calo");
    msg.msg(MSG::ERROR) << "phiCalo not available as auxilliary variable"
                        << endmsg;
    if (do_throw) {
      throw std::runtime_error("phiCalo not available as auxilliary variable");
    }
    msg.msg(MSG::WARNING) << "using phi as phiCalo" << endmsg;
    phi_calo = cluster.phi();
  }
  return phi_calo;
}

inline float get_eta_calo(const xAOD::CaloCluster& cluster, int author,
                          bool do_throw = false) {
  double eta_calo;
  static const SG::ConstAccessor<float> etaCaloAcc("etaCalo");
  if (author == xAOD::EgammaParameters::AuthorFwdElectron) {
    eta_calo = cluster.eta();
  } else if (cluster.retrieveMoment(xAOD::CaloCluster::ETACALOFRAME,
                                    eta_calo)) {
  } else if (etaCaloAcc.isAvailable(cluster)) {
    eta_calo = etaCaloAcc(cluster);
  } else {
    asg::AsgMessaging msg("get_eta_calo");
    msg.msg(MSG::ERROR) << "etaCalo not available as auxilliary variable"
                        << endmsg;
    if (do_throw) {
      throw std::runtime_error("etaCalo not available as auxilliary variable");
    }
    msg.msg(MSG::WARNING) << "using eta as etaCalo" << endmsg;
  }
  return eta_calo;
}
}  // namespace xAOD

namespace CP {

class EgammaCalibrationAndSmearingTool
    : virtual public IEgammaCalibrationAndSmearingTool,
      public asg::AsgMetadataTool {
  // Create a proper constructor for Athena
  ASG_TOOL_CLASS3(EgammaCalibrationAndSmearingTool,
                  IEgammaCalibrationAndSmearingTool, CP::ISystematicsTool,
                  CP::IReentrantSystematicsTool)

 public:
  enum class ScaleDecorrelation {
    FULL,
    ONENP,
    FULL_ETA_CORRELATED,
    ONENP_PLUS_UNCONR
  };
  enum class ResolutionDecorrelation { FULL, ONENP };
  static const int AUTO = 2;  // this is used as a third state for boolean
                              // properties (true/false/automatic)
  typedef unsigned int RandomNumber;
  typedef std::function<int(const EgammaCalibrationAndSmearingTool&,
                            const xAOD::Egamma&, const xAOD::EventInfo&)>
      IdFunction;
  typedef std::function<bool(const xAOD::Egamma&)> EgammaPredicate;

  EgammaCalibrationAndSmearingTool(const std::string& name);
  ~EgammaCalibrationAndSmearingTool();

  StatusCode initialize() override;

  // Apply the correction on a modifyable egamma object
  virtual CP::CorrectionCode applyCorrection(xAOD::Egamma&) const override;
  virtual CP::CorrectionCode applyCorrection(
      xAOD::Egamma& input, const xAOD::EventInfo& event_info) const;

  // Create a corrected copy from a constant egamma object
  //  virtual CP::CorrectionCode correctedCopy(const xAOD::Egamma&,
  //  xAOD::Egamma*&);
  virtual CP::CorrectionCode correctedCopy(const xAOD::Electron&,
                                           xAOD::Electron*&) const override;
  virtual CP::CorrectionCode correctedCopy(const xAOD::Photon&,
                                           xAOD::Photon*&) const override;
  double getEnergy(const xAOD::Photon&) const;    // for python usage
  double getEnergy(const xAOD::Electron&) const;  // for python usage

  // systematics
  // Which systematics have an effect on the tool's behaviour?
  virtual CP::SystematicSet affectingSystematics() const override;
  // Is the tool affected by a specific systematic?
  virtual bool isAffectedBySystematic(
      const CP::SystematicVariation& systematic) const override;
  // Systematics to be used for physics analysis
  virtual CP::SystematicSet recommendedSystematics() const override;
  // Use specific systematic
  virtual StatusCode applySystematicVariation(
      const CP::SystematicSet& systConfig) override;
  virtual void setRandomSeedFunction(const IdFunction&& function) {
    m_set_seed_function = function;
  }
  const IdFunction getRandomSeedFunction() const { return m_set_seed_function; }

  virtual double resolution(
      double energy, double cl_eta, double cl_etaCalo,
      PATCore::ParticleType::Type ptype = PATCore::ParticleType::Electron,
      bool withCT = false) const override;

 private:
  static const unsigned int m_Run2Run3runNumberTransition = 400000;

  std::string m_ESModel;
  std::string m_decorrelation_model_name;
  std::string m_decorrelation_model_scale_name;
  std::string m_decorrelation_model_resolution_name;
  ScaleDecorrelation m_decorrelation_model_scale = ScaleDecorrelation::FULL;
  ResolutionDecorrelation m_decorrelation_model_resolution =
      ResolutionDecorrelation::FULL;
  egEnergyCorr::ESModel m_TESModel;
  int m_doScaleCorrection;
  int m_doSmearing;
  double m_varSF;
  std::string m_ResolutionType;
  egEnergyCorr::Resolution::resolutionType m_TResolutionType;
  int m_useFastSim;
  int m_use_AFII;
  PATCore::ParticleDataType::DataType m_simulation =
      PATCore::ParticleDataType::Full;
  // flags duplicated from the underlying ROOT tool
  int m_useIntermoduleCorrection;
  int m_usePhiUniformCorrection;
  int m_useCaloDistPhiUnifCorrection;
  int m_useGainCorrection;
  int m_doADCLinearityCorrection;
  int m_doLeakageCorrection;
  bool m_use_ep_combination;
  int m_use_mva_calibration;
  bool m_use_full_statistical_error;
  int m_use_temp_correction201215;
  int m_use_uA2MeV_2015_first2weeks_correction;
  bool m_use_mapping_correction;
  int m_user_random_run_number;
  int m_useGainInterpolation;
  int m_useLayerCorrection;
  int m_usePSCorrection;
  int m_useS12Correction;
  int m_useSaccCorrection;
  bool m_decorateEmva;

  // 2D histrogram (eta,phi) for a correction to cope with calo distortion
  // (sagging)
  std::unique_ptr<TH2> m_caloDistPhiUnifCorr;

  Gaudi::Property<bool> m_fixForMissingCells{
      this, "FixForMissingCells", true,
      "AOD fix for cell recovery in core egamma cluster"};

  void setupSystematics();

  // if using eta not abs_eta
  struct EtaCaloPredicate
  {
    EtaCaloPredicate(double eta_min, double eta_max) : m_eta_min(eta_min), m_eta_max(eta_max) {}
    bool operator()(const xAOD::Egamma& p) {
      const double eta = xAOD::get_eta_calo(*p.caloCluster(),p.author());
      return (eta >= m_eta_min and eta < m_eta_max);
    }
    private:
      float m_eta_min, m_eta_max;
  };

  const EgammaPredicate EtaCaloPredicateFactory(double eta_min, double eta_max) const
  {
    return EtaCaloPredicate(eta_min, eta_max);
  }

  // this is needed (instead of a simpler lambda since a clang bug, see
  // https://its.cern.ch/jira/browse/ATLASG-688)
  struct AbsEtaCaloPredicate {
    AbsEtaCaloPredicate(double eta_min, double eta_max)
        : m_eta_min(eta_min), m_eta_max(eta_max) {}
    bool operator()(const xAOD::Egamma& p) {
      const double aeta =
          std::abs(xAOD::get_eta_calo(*p.caloCluster(), p.author()));
      return (aeta >= m_eta_min and aeta < m_eta_max);
    }

   private:
    float m_eta_min, m_eta_max;
  };

  const EgammaPredicate AbsEtaCaloPredicateFactory(double eta_min,
                                                   double eta_max) const {
    /*return [eta_min, eta_max](const xAOD::Egamma& p) {
      const double aeta = std::abs(xAOD::get_eta_calo(*p.caloCluster()));
      return (aeta >= eta_min and aeta < eta_max); };*/
    return AbsEtaCaloPredicate(eta_min, eta_max);
  }

  const EgammaPredicate AbsEtaCaloPredicateFactory(
      std::pair<double, double> edges) const {
    return AbsEtaCaloPredicateFactory(edges.first, edges.second);
  }

  const std::vector<EgammaPredicate> AbsEtaCaloPredicatesFactory(
      const std::vector<std::pair<double, double>>& edges) const {
    std::vector<EgammaPredicate> result;
    result.reserve(edges.size());
    for (const auto& it : edges) {
      result.push_back(AbsEtaCaloPredicateFactory(it.first, it.second));
    }
    return result;
  }

  const std::vector<EgammaPredicate> AbsEtaCaloPredicatesFactory(
      const std::vector<double>& edges) const {
    std::vector<EgammaPredicate> result;
    result.reserve(edges.size() - 1);
    auto it2 = edges.begin();
    auto it = it2++;
    for (; it2 != edges.end(); ++it, ++it2) {
      result.push_back(AbsEtaCaloPredicateFactory(*it, *it2));
    }
    return result;
  }

  struct DoubleOrAbsEtaCaloPredicate {
    DoubleOrAbsEtaCaloPredicate(double eta1_min, double eta1_max,
                                double eta2_min, double eta2_max)
        : m_eta1_min(eta1_min),
          m_eta1_max(eta1_max),
          m_eta2_min(eta2_min),
          m_eta2_max(eta2_max) {}

    bool operator()(const xAOD::Egamma& p) {
      const double aeta =
          std::abs(xAOD::get_eta_calo(*p.caloCluster(), p.author()));
      return ((aeta >= m_eta1_min and aeta < m_eta1_max) or
              (aeta >= m_eta2_min and aeta < m_eta2_max));
    }

   private:
    float m_eta1_min, m_eta1_max, m_eta2_min, m_eta2_max;
  };

  const EgammaPredicate DoubleOrAbsEtaCaloPredicateFactory(
      double eta1_min, double eta1_max, double eta2_min,
      double eta2_max) const {
    return DoubleOrAbsEtaCaloPredicate(eta1_min, eta1_max, eta2_min, eta2_max);
  }

  PATCore::ParticleType::Type xAOD2ptype(const xAOD::Egamma& particle) const;

 public:
  virtual double getEnergy(xAOD::Egamma*, const xAOD::EventInfo*);
  virtual double getElectronMomentum(const xAOD::Electron*,
                                     const xAOD::EventInfo*);
  double getResolution(const xAOD::Egamma& particle,
                       bool withCT = true) const override;
  double intermodule_correction(double Ecl, double phi, double eta) const;
  double correction_phi_unif(double eta, double phi) const;

 private:
  ServiceHandle<IegammaMVASvc> m_MVACalibSvc{this, "MVACalibSvc", "",
                                             "calibration service"};
  std::unique_ptr<egGain::GainUncertainty> m_gain_tool_run2;
  std::shared_ptr<LinearityADC> m_ADCLinearity_tool;
  egGain::GainTool* m_gain_tool = nullptr;                       //!
  egammaLayerRecalibTool* m_layer_recalibration_tool = nullptr;  //!
  std::string m_layer_recalibration_tune;                        //!

  // A pointer to the underlying ROOT tool
  std::unique_ptr<AtlasRoot::egammaEnergyCorrectionTool> m_rootTool;
  std::string m_MVAfolder;

  struct SysInfo {
    EgammaPredicate predicate;
    egEnergyCorr::Scale::Variation effect;
  };

  std::map<CP::SystematicVariation, SysInfo> m_syst_description;
  std::map<CP::SystematicVariation, egEnergyCorr::Resolution::Variation>
      m_syst_description_resolution;

  // These are modified by the ISystematicsTool methods
  egEnergyCorr::Scale::Variation m_currentScaleVariation_MC;
  egEnergyCorr::Scale::Variation m_currentScaleVariation_data;
  egEnergyCorr::Resolution::Variation m_currentResolutionVariation_MC;
  egEnergyCorr::Resolution::Variation m_currentResolutionVariation_data;

  EgammaPredicate m_currentScalePredicate;

  IdFunction m_set_seed_function;

  inline egEnergyCorr::Scale::Variation oldtool_scale_flag_this_event(
      const xAOD::Egamma& p, const xAOD::EventInfo& event_info) const;
  inline egEnergyCorr::Resolution::Variation oldtool_resolution_flag_this_event(
      const xAOD::Egamma& p, const xAOD::EventInfo& event_info) const;
};

}  // namespace CP
#endif
