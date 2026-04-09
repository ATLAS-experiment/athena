/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "FlavorTagInference/BTagTrackIpAccessor.h"
#include "FlavorTagInference/CustomGetterUtils.h"

#include "xAODMuon/Muon.h"
#include "xAODTracking/TrackParticleFwd.h"
#include <xAODPFlow/FlowElement.h>
#include "AthContainers/AuxElement.h"
#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODEgamma/Electron.h"
#include "xAODMuon/Muon.h"
#include "xAODCaloEvent/CaloCluster.h"

#include <limits>
#include <optional>
#include <TVector3.h>
#include "GeoPrimitives/GeoPrimitives.h"

namespace {

  using FlavorTagInference::getter_utils::SequenceGetterFunc;
  // ______________________________________________________________________
  // Custom getters for jet input features
  std::function<double(const xAOD::IParticle&)> customJetGetter(
    const std::string& name)
  {
    if (name == "pt") {
      return [](const xAOD::IParticle& j) -> float {return j.pt();};
    }
    if (name == "log_pt") {
      return [](const xAOD::IParticle& j) -> float {return std::log(j.pt());};
    }
    if (name == "eta") {
      return [](const xAOD::IParticle& j) -> float {return j.eta();};
    }
    if (name == "abs_eta") {
      return [](const xAOD::IParticle& j) -> float {return std::abs(j.eta());};
    }
    if (name == "energy") {
      return [](const xAOD::IParticle& j) -> float {return j.e();};
    }
    if (name == "mass") {
      return [](const xAOD::IParticle& j) -> float {return j.m();};
    }
    if (name == "phi") {
      return [](const xAOD::IParticle& j) -> float {return j.phi();};
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
      operator()(const xAOD::IParticle&, const std::vector<const U*>& constituents) const {
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
    using F = std::function<double(const Const&, const xAOD::IParticle&)>;
    private:
      F m_getter;
    public:
      CustomSeqGetter(F getter): m_getter(getter) {}
      
      std::vector<double>
      operator()(const xAOD::IParticle& jet, const std::vector<const Const*>& constituents) const {
        std::vector<double> sequence;
        sequence.reserve(constituents.size());
        for (const auto* constituent: constituents) {
          sequence.push_back(m_getter(*constituent, jet));
        }
        return sequence;
      }
  };

  template <typename F>
  SequenceGetterFunc<xAOD::Muon> muonPrimaryTrackGetter(F getter)
  {
    using Mu = xAOD::Muon;
    using Jet = xAOD::IParticle;

    return CustomSeqGetter<Mu>([getter](const Mu& mu, const Jet&) -> double {
      const xAOD::TrackParticle* track = mu.primaryTrackParticle();
      if (!track) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return static_cast<double>(getter(*track));
    });
  }

  template <typename F>
  SequenceGetterFunc<xAOD::Muon> muonIdTrackGetter(F getter)
  {
    using Mu = xAOD::Muon;
    using Jet = xAOD::IParticle;

    return CustomSeqGetter<Mu>([getter](const Mu& mu, const Jet& jet) -> double {
      const xAOD::TrackParticle* track =
        mu.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
      if (!track) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return getter(jet, {track}).front();
    });
  }

  // Getters from xAOD::TrackParticle with IP dependencies
  std::optional<SequenceGetterFunc<xAOD::TrackParticle>>
  getterFromTracksWithIpDep(
    const std::string& name,
    const std::string& prefix)
  {
    using Tp = xAOD::TrackParticle;
    using Jet = xAOD::IParticle;

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
    using Jet = xAOD::IParticle;

    if (name == "eProbabilityHT") {
      SG::AuxElement::ConstAccessor<float> eprob_acc(name);
      return CustomSeqGetter<Tp>([eprob_acc](const Tp& tp, const Jet&) {
        return eprob_acc(tp);
      });
    }
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
    if (name == "thetaVariance") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return tp.definingParametersCovMatrixDiagVec().at(3);
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
    if (name == "z0SinThetaRelativeToBeamspot") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return tp.z0() * std::sin(tp.theta());
      });
    }
    if (name == "d0RelativeToBeamspot") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return tp.d0();
      });
    }
    if (name == "d0RelativeToBeamspotVariance") {
      return CustomSeqGetter<Tp>([](const Tp& tp, const Jet&) {
          return tp.definingParametersCovMatrixDiagVec().at(0);
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
    const std::regex number_match("(numberOf|expect).*");
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
    using Jet = xAOD::IParticle;

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
    using Jet = xAOD::IParticle;
    if (name == "isCharged") {
      return CustomSeqGetter<Fl>([](const Fl& p, const Jet&) {
        return p.isCharged();
      });
    }
    return std::nullopt;
  }


  // Eigen::Vector3d getJab(const Eigen::Vector3d local_hits, const xAOD::IParticle& j)
  Eigen::Vector3d getJab(const float local_hitX, const float local_hitY, const float local_hitZ, const xAOD::IParticle& j)
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
    using Jet = xAOD::IParticle;

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
  using decorated_electron_getter_t = std::pair<
    SequenceGetterFunc<xAOD::Electron>,
    std::set<std::string>
    >;
  std::optional<decorated_electron_getter_t>
  getterFromDecoratedElectrons(const std::string& name)
  {
    using Jet = xAOD::IParticle;
    using El = xAOD::Electron;

    std::string isovar{"ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000"};
    std::set<std::string> isodeps{{isovar}};
    SG::AuxElement::ConstAccessor<float> pt_varcone30{isovar};
    if ((name == "ftag_ptVarCone30OverPt") || (name == "ptVarCone30OverPt")) {
      return decorated_electron_getter_t {
        CustomSeqGetter<El>([pt_varcone30](const El& p, const Jet&) {
          return pt_varcone30(p) / p.pt();
        }), isodeps
      };
    }
    return std::nullopt;
  }

  std::optional<SequenceGetterFunc<xAOD::Electron>>
  getterFromElectrons(const std::string& name, const std::string& prefix)
  {
    using Jet = xAOD::IParticle;
    using El = xAOD::Electron;

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
    if ((name == "ftag_energyOverP") || (name == "energyOverP")) {
      return CustomSeqGetter<El>([](const El& p, const Jet&) {
        return p.caloCluster()->e() * std::abs(p.trackParticle()->qOverP());
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

  // Getters from xAOD::Muon
  std::optional<SequenceGetterFunc<xAOD::Muon>> getterFromMuons(
      const std::string& name, const std::string& prefix
  ) {
    using Jet = xAOD::IParticle;
    using Mu = xAOD::Muon;

    if (name == "qOverPratio") {
      return CustomSeqGetter<Mu>([](const Mu& p, const Jet&) -> double {
        auto track = p.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        if ( !track ) { return -9999.0; }
        auto ms_track = p.trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
        if ( !ms_track ) { return -9999.0; }
        return track->qOverP() / ms_track->qOverP();
      });
    }

    auto track_getter_no_ipdep = getterFromTracksNoIpDep(name);
    if ( track_getter_no_ipdep ) {
      auto f = *track_getter_no_ipdep;
      return CustomSeqGetter<Mu>([f](const Mu& p, const Jet& j) -> double {
        return f(j, {p.trackParticle(xAOD::Muon::InnerDetectorTrackParticle)})[0];
      });
    }
    
    auto track_getter_ipdep = getterFromTracksWithIpDep(name, prefix);
    if ( track_getter_ipdep ) {
      auto f = *track_getter_ipdep;
      return CustomSeqGetter<Mu>([f](const Mu& p, const Jet& j) -> double {
        return f(j, {p.trackParticle(xAOD::Muon::InnerDetectorTrackParticle)})[0];
      });
    }

    // Special case of track-dependent variables that use the primary track instead of the ID track
    const std::string suffix = "_MuonPrimaryTrack";
    if (name.size() > suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
      auto extracted_name = name.substr(0, name.size() - suffix.size());
      auto primary_track_getter = getterFromTracksNoIpDep(extracted_name);
      if ( primary_track_getter ) {
        auto f = *primary_track_getter;
        return CustomSeqGetter<Mu>([f](const Mu& p, const Jet& j) -> double {
          return f(j, {p.trackParticle(xAOD::Muon::Primary)})[0];
        });
      }
    }

    return std::nullopt;
  }

  // Getters from xAOD::CaloCluster
  std::optional<SequenceGetterFunc<xAOD::CaloCluster>>
  getterFromCaloClusters(const std::string& name)
  {
    using CC = xAOD::CaloCluster;
    using Jet = xAOD::IParticle;

    // 16 moments via retrieveMoment
    static const std::map<std::string, xAOD::CaloCluster::MomentType> momentMap = {
        {"ENG_BAD_CELLS", xAOD::CaloCluster::ENG_BAD_CELLS},
        {"ISOLATION", xAOD::CaloCluster::ISOLATION},
        {"CENTER_MAG", xAOD::CaloCluster::CENTER_MAG},
        {"CELL_SIGNIFICANCE", xAOD::CaloCluster::CELL_SIGNIFICANCE},
        {"ENG_FRAC_MAX", xAOD::CaloCluster::ENG_FRAC_MAX},
        {"LATERAL", xAOD::CaloCluster::LATERAL},
        {"SIGNIFICANCE", xAOD::CaloCluster::SIGNIFICANCE},
        {"LONGITUDINAL", xAOD::CaloCluster::LONGITUDINAL},
        {"ENG_POS", xAOD::CaloCluster::ENG_POS},
        {"EM_PROBABILITY", xAOD::CaloCluster::EM_PROBABILITY},
        {"CENTER_LAMBDA", xAOD::CaloCluster::CENTER_LAMBDA},
        {"SECOND_LAMBDA", xAOD::CaloCluster::SECOND_LAMBDA},
        {"FIRST_ENG_DENS", xAOD::CaloCluster::FIRST_ENG_DENS},
        {"SECOND_R", xAOD::CaloCluster::SECOND_R},
        {"AVG_LAR_Q", xAOD::CaloCluster::AVG_LAR_Q},
        {"MASS", xAOD::CaloCluster::MASS},
    };
    auto mom_it = momentMap.find(name);
    if (mom_it != momentMap.end()) {
      auto moment_type = mom_it->second;
      return CustomSeqGetter<CC>([moment_type](const CC& c, const Jet&) {
        double val = 0;
        c.retrieveMoment(moment_type, val);
        return val;
      });
    }

    // 6 kinematics via direct methods
    if (name == "rawPhi") {
      return CustomSeqGetter<CC>([](const CC& c, const Jet&) {
        return c.rawPhi();
      });
    }
    if (name == "calPhi") {
      return CustomSeqGetter<CC>([](const CC& c, const Jet&) {
        return c.calPhi();
      });
    }
    if (name == "rawEta") {
      return CustomSeqGetter<CC>([](const CC& c, const Jet&) {
        return c.rawEta();
      });
    }
    if (name == "calEta") {
      return CustomSeqGetter<CC>([](const CC& c, const Jet&) {
        return c.calEta();
      });
    }
    if (name == "rawE") {
      return CustomSeqGetter<CC>([](const CC& c, const Jet&) {
        return c.rawE();
      });
    }
    if (name == "calE") {
      return CustomSeqGetter<CC>([](const CC& c, const Jet&) {
        return c.calE();
      });
    }

    // 22 samplings via eSample
    using CS = xAOD::CaloCluster::CaloSample;
    static const std::map<std::string, CS> sampleMap = {
        {"PreSamplerB", CS::PreSamplerB},
        {"EMB1", CS::EMB1},
        {"EMB2", CS::EMB2},
        {"EMB3", CS::EMB3},
        {"PreSamplerE", CS::PreSamplerE},
        {"EME1", CS::EME1},
        {"EME2", CS::EME2},
        {"EME3", CS::EME3},
        {"HEC0", CS::HEC0},
        {"HEC1", CS::HEC1},
        {"HEC2", CS::HEC2},
        {"HEC3", CS::HEC3},
        {"TileBar0", CS::TileBar0},
        {"TileBar1", CS::TileBar1},
        {"TileBar2", CS::TileBar2},
        {"TileGap1", CS::TileGap1},
        {"TileGap2", CS::TileGap2},
        {"TileGap3", CS::TileGap3},
        {"TileExt0", CS::TileExt0},
        {"TileExt1", CS::TileExt1},
        {"TileExt2", CS::TileExt2},
        {"FCAL0", CS::FCAL0},
        {"FCAL1", CS::FCAL1},
        {"FCAL2", CS::FCAL2},
    };
    auto samp_it = sampleMap.find(name);
    if (samp_it != sampleMap.end()) {
      auto sample = samp_it->second;
      return CustomSeqGetter<CC>([sample](const CC& c, const Jet&) {
        return c.eSample(sample);
      });
    }

    // 1 flag: usedInChargedFlow
    if (name == "usedInChargedFlow") {
      SG::AuxElement::ConstAccessor<int> accInFlow("usedInChargedFlow");
      return CustomSeqGetter<CC>([accInFlow](const CC& c, const Jet&) {
        return accInFlow.isAvailable(c) ? static_cast<double>(accInFlow(c)) : 0.0;
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
    std::function<std::pair<std::string, double>(const xAOD::IParticle&)>
    namedCustomJetGetter(const std::string& name) {
      auto getter = customJetGetter(name);
      return [name, getter](const xAOD::IParticle& j) {
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
        if (auto getterdep = getterFromDecoratedElectrons(name)) {
          return {getterdep->first, getterdep->second};
        }
        if (auto getter = getterFromElectrons(name, prefix)){
          return {*getter, {}};
        }
      }

      if constexpr (std::is_same_v<T, xAOD::Muon>) {
        if (auto getter = getterFromMuons(name, prefix)){
          return {*getter, {}};
        }
      }

      if constexpr (std::is_same_v<T, xAOD::CaloCluster>) {
        if (auto getter = getterFromCaloClusters(name)){
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
        [n=name, g=getter](const xAOD::IParticle& j, const std::vector<const T*>& t) {
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
          return getNamedCustomSeqGetter(cfg.name, prefix);
        }
        default: {
          throw std::logic_error("Unknown EDM type for constituent.");
        }
      }
    }

    template <typename T>
    SeqGetter<T>::SeqGetter(const std::vector<InputVariableConfig>& inputs, const FTagOptions& options)
    {
      std::map<std::string, std::string> remap = options.remap_scalar;
      for (const InputVariableConfig& input_cfg: inputs) {
        auto [seqGetter, seq_deps] = seqFromConsituents(input_cfg, options);

        if(input_cfg.flip_sign){
          auto seqGetter_flip=[g=seqGetter](const xAOD::IParticle&jet, const Const& constituents){
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
      const xAOD::IParticle& jet, const Const& constituents) const
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
      const xAOD::IParticle& jet, const Const& constituents) const
    {
      std::map<std::string, std::vector<double>> feats;
      for (const auto& seq_getter: m_sequence_getters){
        feats.insert(seq_getter(jet, constituents));
      }
      return feats;
    }

    template <typename T>
    const std::set<std::string>& SeqGetter<T>::getDependencies() const {
      return m_deps;
    }
    template <typename T>
    const std::set<std::string>& SeqGetter<T>::getUsedRemap() const {
      return m_used_remap;
    }


    // Explicit instantiations of supported types (IParticle, FlowElement, TrackParticle, TrackMeasurementValidation, Electron, Muon, CaloCluster)
    template class SeqGetter<xAOD::IParticle>;
    template class SeqGetter<xAOD::FlowElement>;
    template class SeqGetter<xAOD::TrackParticle>;
    template class SeqGetter<xAOD::TrackMeasurementValidation>;
    template class SeqGetter<xAOD::Electron>;
    template class SeqGetter<xAOD::Muon>;
    template class SeqGetter<xAOD::CaloCluster>;
  }
}
