/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "FlavorTagInference/BTagTrackIpAccessor.h"
#include "FlavorTagInference/CustomGetterUtils.h"

#include "xAODTracking/TrackParticleFwd.h"
#include <xAODPFlow/FlowElement.h>
#include "AthContainers/AuxElement.h"
#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODEgamma/Electron.h"

#include <optional>
#include <TVector3.h>
#include "GeoPrimitives/GeoPrimitives.h"

namespace {

  using FlavorTagInference::getter_utils::SequenceGetterFunc;
  // ______________________________________________________________________
  // Custom getters for jet input features
  std::function<double(const xAOD::Jet&)> customJetGetter(
    const std::string& name)
  {
    if (name == "pt") {
      return [](const xAOD::Jet& j) -> float {return j.pt();};
    }
    if (name == "log_pt") {
      return [](const xAOD::Jet& j) -> float {return std::log(j.pt());};
    }
    if (name == "eta") {
      return [](const xAOD::Jet& j) -> float {return j.eta();};
    }
    if (name == "abs_eta") {
      return [](const xAOD::Jet& j) -> float {return std::abs(j.eta());};
    }
    if (name == "energy") {
      return [](const xAOD::Jet& j) -> float {return j.e();};
    }
    if (name == "mass") {
      return [](const xAOD::Jet& j) -> float {return j.m();};
    }

    throw std::logic_error("no match for custom getter " + name);
  }

  // _______________________________________________________________________
  // Custom getters for jet constituents 
  
  // wraps non-custom getters into sequences, also adds a name
  template <typename T, typename U>
  class NamedSeqGetter{
    private:
      SG::AuxElement::ConstAccessor<T> m_getter;
      std::string m_name;
    public:
      NamedSeqGetter(const std::string& name):
        m_getter(name),
        m_name(name)
        {}

      std::pair<std::string, std::vector<double>>
      operator()(const xAOD::Jet&, const std::vector<const U*>& constituents) const {
        std::vector<double> sequence;
        for (const U* el: constituents) {
          sequence.push_back(m_getter(*el));
        }
        return {m_name, sequence};
      }
  };

  // wraps custom getters into sequences, doesn't add a name
  template <typename Const>
  class CustomSeqGetter
  {
    using F = std::function<double(const Const&, const xAOD::Jet&)>;
    private:
      F m_getter;
    public:
      CustomSeqGetter(F getter): m_getter(getter) {}
      
      std::vector<double>
      operator()(const xAOD::Jet& jet, const std::vector<const Const*>& constituents) const {
        std::vector<double> sequence;
        sequence.reserve(constituents.size());
        for (const auto* constituent: constituents) {
          sequence.push_back(m_getter(*constituent, jet));
        }
        return sequence;
      }
  };

  // Getters from xAOD::TrackParticle with IP dependencies
  std::optional<SequenceGetterFunc<xAOD::TrackParticle>>
  getterFromTracksWithIpDep(
    const std::string& name,
    const std::string& prefix)
  {
    using Tp = xAOD::TrackParticle;
    using Jet = xAOD::Jet;

    // Note that we have two names for the lifetimeSigned variables
    // here. We should eventually remove the ones with the `IP3D_*`
    // prefix but they are used in quite a few models we're currently
    // running. We keep both because the alternative is changing the
    // metadata in every one of these models.
    BTagTrackIpAccessor a(prefix);
    if (
      name == "IP3D_signed_d0_significance" ||
      name == "lifetimeSignedD0Significance"
      ) {
      return CustomSeqGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip3d_signed_d0_significance;
      });
    }
    if (
      name == "IP3D_signed_z0_significance" ||
      name == "lifetimeSignedZ0SinThetaSignificance"
      ) {
      return CustomSeqGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip3d_signed_z0_sin_theta_significance;
      });
    }
    if (name == "IP2D_signed_d0") {
      return CustomSeqGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip2d_signed_d0;
      });
    }
    if (
      name == "IP3D_signed_d0" ||
      name == "lifetimeSignedD0"
      ) {
      return CustomSeqGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip3d_signed_d0;
      });
    }
    if (
      name == "IP3D_signed_z0" ||
      name == "lifetimeSignedZ0SinTheta"
      ) {
      return CustomSeqGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip3d_signed_z0_sin_theta;
      });
    }
    if (name == "d0" || name == "btagIp_d0") {
      return CustomSeqGetter<Tp>([a](const Tp& tp, const Jet&){
        return a.d0(tp);
      });
    }
    if (name == "z0SinTheta" || name == "btagIp_z0SinTheta") {
      return CustomSeqGetter<Tp>([a](const Tp& tp, const Jet&){
        return a.z0SinTheta(tp);
      });
    }
    if (name == "d0Uncertainty") {
      return CustomSeqGetter<Tp>([a](const Tp& tp, const Jet&){
        return a.d0Uncertainty(tp);
      });
    }
    if (name == "z0SinThetaUncertainty") {
      return CustomSeqGetter<Tp>([a](const Tp& tp, const Jet&){
        return a.z0SinThetaUncertainty(tp);
      });
    }
    return std::nullopt;
  }


  // Getters from xAOD::TrackParticle without IP dependencies
  std::optional<SequenceGetterFunc<xAOD::TrackParticle>>
  getterFromTracksNoIpDep(const std::string& name)
  {
    using Tp = xAOD::TrackParticle;
    using Jet = xAOD::Jet;

    if (name == "qOverP") {
      return CustomSeqGetter<Tp>([](const Tp& p, const Jet&) {
        return p.qOverP(); 
      });
    }
    if (name == "phiUncertainty") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::sqrt(tp.definingParametersCovMatrixDiagVec().at(2));
      });
    }
    if (name == "thetaUncertainty") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::sqrt(tp.definingParametersCovMatrixDiagVec().at(3));
      });
    }
    if (name == "qOverPUncertainty") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::sqrt(tp.definingParametersCovMatrixDiagVec().at(4));
      });
    }
    if (name == "z0RelativeToBeamspot") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return tp.z0();
      });
    }
    if (name == "d0RelativeToBeamspot") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return tp.d0();
      });
    }
    if (name == "d0RelativeToBeamspotSignificance") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return tp.d0() / std::sqrt(tp.definingParametersCovMatrixDiagVec().at(0));
      });
    }
    if (name == "log_z0RelativeToBeamspotUncertainty") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::log(std::sqrt(tp.definingParametersCovMatrixDiagVec().at(1)));
      });
    }
    if (name == "z0RelativeToBeamspotUncertainty") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::sqrt(tp.definingParametersCovMatrixDiagVec().at(1));
      });
    }
    if (name == "numberOfPixelHitsInclDead") {
      SG::AuxElement::ConstAccessor<unsigned char> pix_hits("numberOfPixelHits");
      SG::AuxElement::ConstAccessor<unsigned char> pix_dead("numberOfPixelDeadSensors");
      return CustomSeqGetter<Tp>([pix_hits, pix_dead](const Tp& tp, const Jet&) {
        return pix_hits(tp) + pix_dead(tp);
      });
    }
    if (name == "numberOfSCTHitsInclDead") {
      SG::AuxElement::ConstAccessor<unsigned char> sct_hits("numberOfSCTHits");
      SG::AuxElement::ConstAccessor<unsigned char> sct_dead("numberOfSCTDeadSensors");
      return CustomSeqGetter<Tp>([sct_hits, sct_dead](const Tp& tp, const Jet&) {
        return sct_hits(tp) + sct_dead(tp);
      });
      }
    if (name == "numberOfInnermostPixelLayerHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfInnermostPixelLayerHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfInnermostPixelLayerEndcapHits");
      return CustomSeqGetter<Tp>([barrel_hits, endcap_hits](const Tp& tp, const Jet&) {
        return barrel_hits(tp) + endcap_hits(tp);
      });
    }
    if (name == "numberOfNextToInnermostPixelLayerHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfNextToInnermostPixelLayerHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfNextToInnermostPixelLayerEndcapHits");
      return CustomSeqGetter<Tp>([barrel_hits, endcap_hits](const Tp& tp, const Jet&) {
        return barrel_hits(tp) + endcap_hits(tp);
      });
    }
    if (name == "numberOfInnermostPixelLayerSharedHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfInnermostPixelLayerSharedHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfInnermostPixelLayerSharedEndcapHits");
      return CustomSeqGetter<Tp>([barrel_hits, endcap_hits](const Tp& tp, const Jet&) {
        return barrel_hits(tp) + endcap_hits(tp);
      });
    }
    if (name == "numberOfInnermostPixelLayerSplitHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfInnermostPixelLayerSplitHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfInnermostPixelLayerSplitEndcapHits");
      return CustomSeqGetter<Tp>([barrel_hits, endcap_hits](const Tp& tp, const Jet&) {
        return barrel_hits(tp) + endcap_hits(tp);
      });
    }
    const std::regex number_match("numberOf.*");
    if (std::regex_match(name, number_match)){ 
      SG::AuxElement::ConstAccessor<unsigned char> pix_hits(name);
      return CustomSeqGetter<Tp>([pix_hits](const Tp& tp, const Jet&) {
        return pix_hits(tp);
      });
    }
    return std::nullopt;
  }


  // Getters from general xAOD::IParticle and derived classes
  template <typename T> std::optional<SequenceGetterFunc<T>>
  getterFromIParticles(const std::string& name)
  {
    using Jet = xAOD::Jet;

    if (name == "pt") {
      return CustomSeqGetter<T>([](const T& p, const Jet&) {
        return p.pt();
      });
    }
    if (name == "log_pt") {
      return CustomSeqGetter<T>([](const T& p, const Jet&) {
        return std::log(p.pt());
      });
    }
    if (name == "EFrac_wrtJet") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return p.e() / j.e();
      });
    }
    if (name == "pZ_wrtJet") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return p.p4().Vect().Dot(j.p4().Vect().Unit());
      });
    }
    if (name == "pT_wrtJet") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        double pZ = p.p4().Vect().Dot(j.p4().Vect().Unit());
        return  std::sqrt(p.p4().P()*p.p4().P() - pZ*pZ);
  
      });
    }
    if (name == "ptfrac") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return p.pt() / j.pt();
      });
    }
    if (name == "log_ptfrac") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return std::log(p.pt() / j.pt());
      });
    }
    if (name == "ptrel") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return p.p4().Vect().Perp(j.p4().Vect());
      });
    }
    if (name == "eta") {
      return CustomSeqGetter<T>([](const T& p, const Jet&) {
        return p.eta();
      });
    }
    if (name == "abs_eta") {
      return CustomSeqGetter<T>([](const T& p, const Jet&) {
          return std::abs(p.eta());
      });
    }
    if (name == "deta") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return p.eta() - j.eta();
      });
    }
    if (name == "abs_deta") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return copysign(1.0, j.eta()) * (p.eta() - j.eta());
      });
    }
    if (name == "phi") {
      return CustomSeqGetter<T>([](const T& p, const Jet&) {
        return p.phi();
      });
    }
    if (name == "dphi") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return p.p4().DeltaPhi(j.p4());
      });
    }
    if (name == "dr") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return p.p4().DeltaR(j.p4());
      });
    }
    if (name == "log_dr") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return std::log(p.p4().DeltaR(j.p4()));
      });
    }
    if (name == "log_dr_nansafe") {
      return CustomSeqGetter<T>([](const T& p, const Jet& j) {
        return std::log(p.p4().DeltaR(j.p4()) + 1e-7);
      });
    }
    if (name == "mass") {
      return CustomSeqGetter<T>([](const T& p, const Jet&) {
        return p.m();
      });
    }
    if (name == "energy") {
      return CustomSeqGetter<T>([](const T& p, const Jet&) {
        return p.e();
      });
    }    
    return std::nullopt;
  }

  // Getter from xAOD::FlowElement
  std::optional<SequenceGetterFunc<xAOD::FlowElement>>
  getterFromFlowElements(const std::string& name)
  {   
    using Fl = xAOD::FlowElement;
    using Jet = xAOD::Jet;
    if (name == "isCharged") {
      return CustomSeqGetter<Fl>([](const Fl& p, const Jet&) {
        return p.isCharged();
      });
    }
    return std::nullopt;
  }


  // Eigen::Vector3d getJab(const Eigen::Vector3d local_hits, const xAOD::Jet& j)
  Eigen::Vector3d getJab(const float local_hitX, const float local_hitY, const float local_hitZ, const xAOD::Jet& j)
  {
    // I want to compute jab coordinates: jet projection, adjacent
    // projection, beamline projection. The "adjacent" projection
    // is defined to be orthogonal to the jet and beam, but this
    // isn't a fully orthogonal basis. 

    auto p4 = j.p4();
    Eigen::Vector3d local_hits (local_hitX, local_hitY, local_hitZ);
    Eigen::Vector3d bhat(0,0,1);
    Eigen::Vector3d jet (p4.X(), p4.Y(), p4.Z());
    Eigen::Vector3d jhat = jet.normalized();
    Eigen::Vector3d a = bhat.cross(jhat);
    Eigen::Vector3d ahat = a.normalized();
    // build the matrix m that maps the jab displacement such that m*jab = detector
    Eigen::Matrix3d m;
    m << jhat, ahat, bhat;
    // now solve this for jab = m^-1 * detector
    Eigen::Vector3d jab = m.inverse() * local_hits;
    return jab;
  }


  // Getters from general xAOD::TrackMeasurementValidation and derived classes
  std::optional<SequenceGetterFunc<xAOD::TrackMeasurementValidation>>
  getterFromHits(const std::string& name)
  {
    using Tmv = xAOD::TrackMeasurementValidation;
    using Jet = xAOD::Jet;
    
    SG::AuxElement::ConstAccessor<float> local_hitX("HitsXRelToBeamspot");
    SG::AuxElement::ConstAccessor<float> local_hitY("HitsYRelToBeamspot");
    SG::AuxElement::ConstAccessor<float> local_hitZ("HitsZRelToBeamspot");

    if (name == "j") {
      return CustomSeqGetter<Tmv>([local_hitX, local_hitY, local_hitZ](const Tmv& tmv, const Jet& j) {
        return getJab(local_hitX(tmv), local_hitY(tmv), local_hitZ(tmv), j)(0);
      });
    }
    else if (name == "a") {
      return CustomSeqGetter<Tmv>([local_hitX, local_hitY, local_hitZ](const Tmv& tmv, const Jet& j) {
        return getJab(local_hitX(tmv), local_hitY(tmv), local_hitZ(tmv), j)(1);
      });
    }
    else if (name == "b") {
      return CustomSeqGetter<Tmv>([local_hitX, local_hitY, local_hitZ](const Tmv& tmv, const Jet& j) {
        return getJab(local_hitX(tmv), local_hitY(tmv), local_hitZ(tmv), j)(2);
      });
    }
    return std::nullopt;
  }


  // Getters from xAOD::Electron
  // Based on ElectronPhotonSelectorTools/AsgElectronLikelihoodTool
  std::optional<SequenceGetterFunc<xAOD::Electron>>
  getterFromElectrons(const std::string& name, const std::string& prefix)
  {
    using Jet = xAOD::Jet;
    using El = xAOD::Electron;

    SG::AuxElement::ConstAccessor<float> pt_varcone30{"ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000"};

    if ((name == "ftag_et") || (name == "et")) {
      return CustomSeqGetter<El>([](const El& p, const Jet&) {
        float energy = p.caloCluster()->e();
        return energy / std::cosh(p.trackParticle()->eta());
      });
    }
    if ((name == "ftag_deltaPOverP") || (name == "deltaPOverP")) {
      return CustomSeqGetter<El>([](const El& p, const Jet&) {
        float el_dpop = -1;
        unsigned int index;
        auto track = p.trackParticle();
        if (track->indexOfParameterAtPosition(index, xAOD::LastMeasurement))
        {
            double refittedTrack_LMqoverp = track->charge() / std::sqrt(std::pow(track->parameterPX(index), 2) +
                                                                        std::pow(track->parameterPY(index), 2) +
                                                                        std::pow(track->parameterPZ(index), 2));
            el_dpop = 1 - track->qOverP() / (refittedTrack_LMqoverp);
        }
        return el_dpop;
      });
    }
    if ((name == "ftag_ptVarCone30OverPt") || (name == "ptVarCone30OverPt")) {
      return CustomSeqGetter<El>([pt_varcone30](const El& p, const Jet&) {
        return pt_varcone30(p) / p.pt();
      });
    }
    if ((name == "ftag_energyOverP") || (name == "energyOverP")) {
      return CustomSeqGetter<El>([](const El& p, const Jet&) {
        return p.caloCluster()->e() * std::abs(p.trackParticle()->qOverP());
      });
    }
    if (name == "eProbabilityHT") {
      return CustomSeqGetter<El>([](const El& p, const Jet&) {
        float eprob = 0.0;
        p.trackParticle()->summaryValue(eprob, xAOD::eProbabilityHT);
        return eprob;
      });
    }
    auto track_getter_no_ipdep = getterFromTracksNoIpDep(name);
    if (track_getter_no_ipdep) {
      auto f = *track_getter_no_ipdep;
      return CustomSeqGetter<El>([f](const El& p, const Jet& j) -> double {
        return f(j, {p.trackParticle()})[0];
      });
    }
    auto track_getter_ipdep = getterFromTracksWithIpDep(name, prefix);
    if (track_getter_ipdep) {
      auto f = *track_getter_ipdep;
      return CustomSeqGetter<El>([f](const El& p, const Jet& j) -> double {
        return f(j, {p.trackParticle()})[0];
      });
    }
    return std::nullopt;
  }
}
  namespace FlavorTagInference {
  namespace getter_utils {
    // ________________________________________________________________
    // Interface functions
    //
    // As long as we're giving lwtnn pair<name, double> objects, we
    // can't use the raw getter functions above (which only return a
    // double). Instead we'll wrap those functions in another function,
    // which returns the pair we wanted.
    //
    // Case for jet variables
    std::function<std::pair<std::string, double>(const xAOD::Jet&)>
    namedCustomJetGetter(const std::string& name) {
      auto getter = customJetGetter(name);
      return [name, getter](const xAOD::Jet& j) {
        return std::make_pair(name, getter(j));
      };
    }

    // Case for constituent variables
    // Returns getter function with dependencies
    template <typename T>
    std::pair<SequenceGetterFunc<T>, std::set<std::string>>
    buildCustomSeqGetter(const std::string& name, const std::string& prefix) {
      if constexpr (std::is_same_v<T, xAOD::TrackParticle>) {
        if (auto getter = getterFromTracksWithIpDep(name, prefix)) {
          auto deps = BTagTrackIpAccessor(prefix).getTrackIpDataDependencyNames();
          return {*getter, deps};
        }
        if (auto getter = getterFromTracksNoIpDep(name)) {
          return {*getter, {}};
        }
      }
      
      if constexpr (std::is_same_v<T, xAOD::Electron>) {
        if (auto getter = getterFromElectrons(name, prefix)){
          return {*getter, {}};
        }
      }
      
      if constexpr (std::is_same_v<T, xAOD::FlowElement>) {
        if (auto getter = getterFromFlowElements(name)){
          return {*getter, {}};
        }
      }

      if constexpr (std::is_base_of_v<xAOD::IParticle, T>){
        if (auto getter = getterFromIParticles<T>(name)){
          return {*getter, {}};
        }
      }

      if constexpr (std::is_same_v<T, xAOD::TrackMeasurementValidation>) {
        if (auto getter = getterFromHits(name)){
          return {*getter, {"HitsXRelToBeamspot", "HitsYRelToBeamspot", "HitsZRelToBeamspot"}};
        }
      }

      throw std::logic_error("no match for custom getter " + name);
    }
    
    // ________________________________________________________________________
    // Class implementation
    //
    template <typename T>
    std::pair<typename SeqGetter<T>::InputSequence, std::set<std::string>> 
    SeqGetter<T>::getNamedCustomSeqGetter(const std::string& name, const std::string& prefix) {
      auto [getter, deps] = buildCustomSeqGetter<T>(name, prefix);
      return {
        [n=name, g=getter](const xAOD::Jet& j, const std::vector<const T*>& t) {
          return std::make_pair(n, g(j, t));
        },
        deps
      };
    }

    template <typename T>
    std::pair<typename SeqGetter<T>::InputSequence, std::set<std::string>> 
    SeqGetter<T>::seqFromConsituents(const InputVariableConfig& cfg, const FTagOptions& options){
      const std::string prefix = options.track_prefix;
      switch (cfg.type) {
        case ConstituentsEDMType::INT: return {
            NamedSeqGetter<int, T>(cfg.name), {cfg.name}
          };
        case ConstituentsEDMType::FLOAT: return {
            NamedSeqGetter<float, T>(cfg.name), {cfg.name}
          };
        case ConstituentsEDMType::CHAR: return {
            NamedSeqGetter<char, T>(cfg.name), {cfg.name}
          };
        case ConstituentsEDMType::UCHAR: return {
            NamedSeqGetter<unsigned char, T>(cfg.name), {cfg.name}
          };
        case ConstituentsEDMType::CUSTOM_GETTER: {
          return getNamedCustomSeqGetter(
            cfg.name, options.track_prefix);
        }
        default: {
          throw std::logic_error("Unknown EDM type for constituent.");
        }
      }
    }

    template <typename T>
    SeqGetter<T>::SeqGetter(std::vector<InputVariableConfig> inputs, const FTagOptions& options)
    {
      std::map<std::string, std::string> remap = options.remap_scalar;
      for (const InputVariableConfig& input_cfg: inputs) {
        auto [seqGetter, seq_deps] = seqFromConsituents(input_cfg, options);

        if(input_cfg.flip_sign){
          auto seqGetter_flip=[g=seqGetter](const xAOD::Jet&jet, const Const& constituents){
            auto [n,v] = g(jet,constituents);
            std::for_each(v.begin(), v.end(), [](double &n){ n=-1.0*n; });
            return std::make_pair(n,v);
          };
          m_sequence_getters.push_back(seqGetter_flip);
        }
        else{
          m_sequence_getters.push_back(seqGetter);
        }
        m_deps.merge(seq_deps);
        if (auto h = remap.extract(input_cfg.name)){
          m_used_remap.insert(h.key());
        }
      }
    }

    template <typename T>
    std::pair<std::vector<float>, std::vector<int64_t>> SeqGetter<T>::getFeats(
      const xAOD::Jet& jet, const Const& constituents) const
    {
      std::vector<float> cnsts_feats;
      int num_vars = m_sequence_getters.size();
      int num_cnsts = 0;

      int cnst_var_idx = 0;
      for (const auto& seq_getter: m_sequence_getters){
        auto input_sequence = seq_getter(jet, constituents).second;

        if (cnst_var_idx==0){
          num_cnsts = static_cast<int>(input_sequence.size());
          cnsts_feats.resize(num_cnsts * num_vars);
        }

        // need to transpose + flatten
        for (unsigned int cnst_idx=0; cnst_idx<input_sequence.size(); cnst_idx++){
          cnsts_feats.at(cnst_idx*num_vars + cnst_var_idx) = input_sequence.at(cnst_idx);
        }
        cnst_var_idx++;
      }
      std::vector<int64_t> cnsts_feat_dim = {num_cnsts, num_vars};
      return {cnsts_feats, cnsts_feat_dim};
    }

    template <typename T>
    std::map<std::string, std::vector<double>> SeqGetter<T>::getDL2Feats(
      const xAOD::Jet& jet, const Const& constituents) const
    {
      std::map<std::string, std::vector<double>> feats;
      for (const auto& seq_getter: m_sequence_getters){
        feats.insert(seq_getter(jet, constituents));
      }
      return feats;
    }

    template <typename T>
    std::set<std::string> SeqGetter<T>::getDependencies() const {
      return m_deps;
    }
    template <typename T>
    std::set<std::string> SeqGetter<T>::getUsedRemap() const {
      return m_used_remap;
    }


    // Explicit instantiations of supported types (IParticle, FlowElement, TrackParticle, TrackMeasurementValidation, Electron)
    template class SeqGetter<xAOD::IParticle>;
    template class SeqGetter<xAOD::FlowElement>;
    template class SeqGetter<xAOD::TrackParticle>;
    template class SeqGetter<xAOD::TrackMeasurementValidation>;
    template class SeqGetter<xAOD::Electron>;
  }
}
