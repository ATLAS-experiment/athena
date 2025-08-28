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
#include "ColumnarCore/ColumnarTool.h"
#include "ColumnarCore/ObjectColumn.h"
#include "ColumnarCore/ColumnAccessor.h"
#include "ColumnarCluster/ClusterHelpers.h"
#include "ColumnarCore/LinkColumn.h"
#include "ColumnarCore/VectorColumn.h"
#include "ColumnarEgamma/EgammaHelpers.h"
#include "ColumnarEventInfo/EventInfoHelpers.h"
#include "ColumnarTracking/TrackDef.h"
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


// the columnar accessor variant of the above functions.  I'm not sure
// if they are used anywhere else, so I'm keeping both.
namespace columnar {
  namespace ClusterHelpers {

    template<ContainerIdConcept CI = ContainerId::cluster,typename CM=ColumnarModeDefault>
    class PhiCaloAccessor final
    {
      ColumnAccessor<CI,float,CM> m_phiCaloAcc;
      ColumnAccessor<CI,float,CM> m_phiAcc;
      ColumnAccessor<CI,float,CM> m_phicaloframeAcc;

    public:
      
      PhiCaloAccessor (ColumnarTool<CM>& columnarTool)
        : m_phiCaloAcc (columnarTool, "phiCalo", {.isOptional = true}),
          m_phiAcc (columnarTool, "calPhi"),
          m_phicaloframeAcc (columnarTool, "PHICALOFRAME", {.isOptional = true})
      {}
  
      float operator () (ClusterId cluster, int author, bool do_throw=false) const
      {
        double phi_calo;
        if(author== xAOD::EgammaParameters::AuthorFwdElectron){
          phi_calo = m_phiAcc (cluster);
        }
        else if (m_phicaloframeAcc.isAvailable (cluster)) {
          phi_calo = m_phicaloframeAcc (cluster);
        }
        else if (m_phiCaloAcc.isAvailable(cluster)) {
          phi_calo = m_phiCaloAcc(cluster);
        }
        else {
          asg::AsgMessaging msg("get_phi_calo");
          msg.msg(MSG::ERROR) << "phiCalo not available as auxilliary variable" << endmsg;
          if (do_throw) { throw std::runtime_error("phiCalo not available as auxilliary variable"); }
          msg.msg(MSG::WARNING) << "using phi as phiCalo" << endmsg;
          phi_calo = m_phiAcc (cluster);
        }
        return phi_calo;
      }
    };

    template<ContainerIdConcept CI = ContainerId::cluster,typename CM=ColumnarModeDefault>
    class EtaCaloAccessor final
    {
      ColumnAccessor<CI,float,CM> m_etaCaloAcc;
      ColumnAccessor<CI,float,CM> m_etaAcc;
      ColumnAccessor<CI,float,CM> m_etacaloframeAcc;

    public:

      EtaCaloAccessor (ColumnarTool<CM>& columnarTool)
        : m_etaCaloAcc (columnarTool, "etaCalo", {.isOptional = true}),
          m_etaAcc (columnarTool, "calEta"),
          m_etacaloframeAcc (columnarTool, "ETACALOFRAME", {.isOptional = true})
      {}

      float operator () (ClusterId cluster, int author, bool do_throw=false) const
      {
        double eta_calo;
        if(author== xAOD::EgammaParameters::AuthorFwdElectron){
              eta_calo = m_etaAcc (cluster);
        }
        else if (m_etacaloframeAcc.isAvailable(cluster)) {
          eta_calo = m_etacaloframeAcc(cluster);
        }
        else if (m_etaCaloAcc.isAvailable(cluster)) {
          eta_calo = m_etaCaloAcc(cluster);
        }
        else {
          asg::AsgMessaging msg("get_eta_calo");
          msg.msg(MSG::ERROR) << "etaCalo not available as auxilliary variable" << endmsg;
          if (do_throw) { throw std::runtime_error("etaCalo not available as auxilliary variable"); }
          msg.msg(MSG::WARNING) << "using eta as etaCalo" << endmsg;
          eta_calo = m_etaAcc (cluster);
        }
        return eta_calo;
      }
    };
  }
}

namespace CP {

class EgammaCalibrationAndSmearingTool
    : virtual public IEgammaCalibrationAndSmearingTool,
      public asg::AsgMetadataTool, public columnar::ColumnarTool<> {
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
                            columnar::EgammaId, columnar::EventInfoId)>
      IdFunction;
  typedef std::function<bool(const EgammaCalibrationAndSmearingTool&, columnar::EgammaId)> EgammaPredicate;

  EgammaCalibrationAndSmearingTool(const std::string& name);
  ~EgammaCalibrationAndSmearingTool();

  StatusCode initialize() override;

  // Apply the correction on a modifyable egamma object
  virtual CP::CorrectionCode applyCorrection(xAOD::Egamma&) const override;
  CP::CorrectionCode applyCorrection(columnar::MutableEgammaId input, columnar::EventInfoId event_info) const;

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
    bool operator()(const EgammaCalibrationAndSmearingTool& tool, columnar::EgammaId p) {
      const Accessors& acc = *tool.m_accessors;
      const double eta = acc.etaCaloAcc(acc.caloClusterAcc(p)[0].value(),acc.authorAcc (p));
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
    bool operator()(const EgammaCalibrationAndSmearingTool& tool, columnar::EgammaId p) {
      const Accessors& acc = *tool.m_accessors;
      const double aeta =
          std::abs(acc.etaCaloAcc(acc.caloClusterAcc(p)[0].value(),acc.authorAcc (p)));
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

    bool operator()(const EgammaCalibrationAndSmearingTool& tool, columnar::EgammaId p) {
      const Accessors& acc = *tool.m_accessors;
      const double aeta =
          std::abs(acc.etaCaloAcc(acc.caloClusterAcc(p)[0].value(),acc.authorAcc (p)));
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

  PATCore::ParticleType::Type xAOD2ptype(columnar::EgammaId particle) const;

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
      columnar::EgammaId p, columnar::EventInfoId event_info) const;
  inline egEnergyCorr::Resolution::Variation oldtool_resolution_flag_this_event(
      columnar::EgammaId p, columnar::EventInfoId event_info) const;

  // columnar data handles
public:
  Gaudi::Property<bool> m_onlyElectrons {this, "onlyElectrons", false, "the tool will only be applied to electrons"};
  Gaudi::Property<bool> m_onlyPhotons {this, "onlyPhotons", false, "the tool will only be applied to photons"};
  struct Accessors : public columnar::ColumnarTool<>
  {
    Accessors(columnar::ColumnarTool<>& tool) : columnar::ColumnarTool<>(&tool) {}

    columnar::MutableEgammaAccessor<columnar::ObjectColumn> m_egammaHandle {*this, "EGamma"};
    columnar::EgammaHelpers::EnergyAccessor<> eAcc {*this};
    columnar::EgammaAccessor<columnar::RetypeColumn<double,float>> ptAcc {*this, "pt"};
    columnar::EgammaDecorator<float> ptOutDec {*this, "ptOut", {.replacesColumn = "pt"}};
    columnar::EgammaDecorator<float> decEmva;
    columnar::EgammaAccessor<columnar::RetypeColumn<double,float>> etaAcc {*this, "eta"};
    columnar::EgammaAccessor<columnar::RetypeColumn<double,float>> phiAcc {*this, "phi"};
    columnar::EgammaAccessor<columnar::RetypeColumn<double,float>> mAcc {*this, "m"};
    columnar::EgammaAccessor<uint16_t> authorAcc {*this, "author"};
    columnar::EgammaAccessor<std::vector<columnar::OptTrackId>> electronTrackAcc;
    columnar::EgammaAccessor<std::vector<columnar::OptVertexId>> photonVertexAcc;
    columnar::ClusterAccessor<columnar::ObjectColumn> m_clusterHandle {*this, "egammaClusters"};
    columnar::EgammaAccessor<std::vector<columnar::OptClusterId>> caloClusterAcc {*this, "caloClusterLinks"};
    columnar::ClusterAccessor<double> Es0Acc {*this, "correctedcl_Es0", {.isOptional = true}};
    columnar::ClusterAccessor<double> Es1Acc {*this, "correctedcl_Es1", {.isOptional = true}};
    columnar::ClusterAccessor<double> Es2Acc {*this, "correctedcl_Es2", {.isOptional = true}};
    columnar::ClusterAccessor<double> Es3Acc {*this, "correctedcl_Es3", {.isOptional = true}};
    columnar::ClusterAccessor<columnar::RetypeColumn<double,float>> clusterEtaAcc {*this, "calEta"};
    columnar::ClusterAccessor<columnar::RetypeColumn<double,float>> clusterPhiAcc {*this, "calPhi"};
    columnar::ClusterHelpers::EnergyBEAccessor<> energyBEAcc {*this};
    columnar::ClusterHelpers::EtaBEAccessor<> clusterEtaBEAcc {*this};
    columnar::ClusterHelpers::EtaCaloAccessor<> etaCaloAcc {*this};
    columnar::ClusterHelpers::PhiCaloAccessor<> phiCaloAcc {*this};
    columnar::EventInfoAccessor<columnar::ObjectColumn> m_eventHandle {*this, "EventInfo"};
    columnar::EventInfoHelpers::EventTypeAccessor<> eventTypeAcc {*this};
    columnar::EventInfoAccessor<uint32_t> runNumberAcc {*this, "runNumber"};
    columnar::EventInfoAccessor<uint64_t> eventNumberAcc {*this, "eventNumber"};
    columnar::EventInfoAccessor<unsigned int> randomrunnumber_getter {*this, "RandomRunNumber"};
  };
  std::unique_ptr<Accessors> m_accessors;

  void callSingleEvent (columnar::MutableEgammaRange egammas, columnar::EventInfoId event) const;
  void callEvents (columnar::EventContextRange events) const override;
};

}  // namespace CP
#endif
