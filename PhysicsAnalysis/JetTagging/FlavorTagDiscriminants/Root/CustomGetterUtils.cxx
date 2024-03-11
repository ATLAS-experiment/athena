/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/CustomGetterUtils.h"

#include <optional>

namespace {

  using FlavorTagDiscriminants::getter_utils::SequenceFromTracks;
  using FlavorTagDiscriminants::getter_utils::SequenceFromIParticles;
  // ______________________________________________________________________
  // Custom getters for jet-wise quantities
  //
  // this function is not at all optimized, but then it doesn't have
  // to be since it should only be called in the initialization stage.
  //
  std::function<double(const xAOD::Jet&)> customGetter(
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
  // Custom getters for constituent variables (CJGetter -> Constituent and Jet Getter)
  template <typename T>
  class CJGetter
  {
    using F = std::function<double(const T&, const xAOD::Jet&)>;
    private:
      F m_getter;
    public:
        CJGetter(F getter):
        m_getter(getter)
        {}
      std::vector<double> operator()(
        const xAOD::Jet& jet,
        const std::vector<const T*>& particles) const {
        std::vector<double> sequence;
        sequence.reserve(particles.size());
        for (const auto* particle: particles) {
          sequence.push_back(m_getter(*particle, jet));
        }
        return sequence;
      }
  };


  // The sequence getter takes in constituents and calculates arrays of
  // values which are better suited for inputs to the NNs
  template <typename T, typename U>
  class SequenceGetter{
    private:
      SG::AuxElement::ConstAccessor<T> m_getter;
      std::string m_name;
    public:
      SequenceGetter(const std::string& name):
        m_getter(name),
        m_name(name)
        {
        }
      std::pair<std::string, std::vector<double>> operator()(const xAOD::Jet&, const std::vector<const U*>& consts) const {
        std::vector<double> seq;
        for (const U* el: consts) {
          seq.push_back(m_getter(*el));
        }
        return {m_name, seq};
      }
  };


  // Getters from xAOD::TrackParticle with IP dependencies
  std::optional<SequenceFromTracks>
  getterFromTracksWithIpDep(
    const std::string& name,
    const std::string& prefix)
  {
    using Tp = xAOD::TrackParticle;
    using Jet = xAOD::Jet;

    BTagTrackIpAccessor a(prefix);
    if (name == "IP3D_signed_d0_significance") {
      return CJGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip3d_signed_d0_significance;
      });
    }
    if (name == "IP3D_signed_z0_significance") {
      return CJGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip3d_signed_z0_significance;
      });
    }
    if (name == "IP2D_signed_d0") {
      return CJGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip2d_signed_d0;
      });
    }
    if (name == "IP3D_signed_d0") {
      return CJGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip3d_signed_d0;
      });
    }
    if (name == "IP3D_signed_z0") {
      return CJGetter<Tp>([a](const Tp& tp, const Jet& j){
        return a.getSignedIp(tp, j).ip3d_signed_z0;
      });
    }
    if (name == "d0" || name == "btagIp_d0") {
      return CJGetter<Tp>([a](const Tp& tp, const Jet&){
        return a.d0(tp);
      });
    }
    if (name == "z0SinTheta" || name == "btagIp_z0SinTheta") {
      return CJGetter<Tp>([a](const Tp& tp, const Jet&){
        return a.z0SinTheta(tp);
      });
    }
    if (name == "d0Uncertainty") {
      return CJGetter<Tp>([a](const Tp& tp, const Jet&){
        return a.d0Uncertainty(tp);
      });
    }
    if (name == "z0SinThetaUncertainty") {
      return CJGetter<Tp>([a](const Tp& tp, const Jet&){
        return a.z0SinThetaUncertainty(tp);
      });
    }
    return std::nullopt;
  }

  // Getters from xAOD::TrackParticle without IP dependencies
  std::optional<SequenceFromTracks>
  getterFromTracksNoIpDep(const std::string& name)
  {
    using Tp = xAOD::TrackParticle;
    using Jet = xAOD::Jet;

    if (name == "phiUncertainty") {
      return CJGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::sqrt(tp.definingParametersCovMatrixDiagVec().at(2));
      });
    }
    if (name == "thetaUncertainty") {
      return CJGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::sqrt(tp.definingParametersCovMatrixDiagVec().at(3));
      });
    }
    if (name == "qOverPUncertainty") {
      return CJGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::sqrt(tp.definingParametersCovMatrixDiagVec().at(4));
      });
    }
    if (name == "z0RelativeToBeamspot") {
      return CJGetter<Tp>([](const Tp& tp, const Jet&) {
          return tp.z0();
      });
    }
    if (name == "log_z0RelativeToBeamspotUncertainty") {
      return CJGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::log(std::sqrt(tp.definingParametersCovMatrixDiagVec().at(1)));
      });
    }
    if (name == "z0RelativeToBeamspotUncertainty") {
      return CJGetter<Tp>([](const Tp& tp, const Jet&) {
          return std::sqrt(tp.definingParametersCovMatrixDiagVec().at(1));
      });
    }
    if (name == "numberOfPixelHitsInclDead") {
      SG::AuxElement::ConstAccessor<unsigned char> pix_hits("numberOfPixelHits");
      SG::AuxElement::ConstAccessor<unsigned char> pix_dead("numberOfPixelDeadSensors");
      return CJGetter<Tp>([pix_hits, pix_dead](const Tp& tp, const Jet&) {
        return pix_hits(tp) + pix_dead(tp);
      });
    }
    if (name == "numberOfSCTHitsInclDead") {
      SG::AuxElement::ConstAccessor<unsigned char> sct_hits("numberOfSCTHits");
      SG::AuxElement::ConstAccessor<unsigned char> sct_dead("numberOfSCTDeadSensors");
      return CJGetter<Tp>([sct_hits, sct_dead](const Tp& tp, const Jet&) {
        return sct_hits(tp) + sct_dead(tp);
      });
      }
    if (name == "numberOfInnermostPixelLayerHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfInnermostPixelLayerHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfInnermostPixelLayerEndcapHits");
      return CJGetter<Tp>([barrel_hits, endcap_hits](const Tp& tp, const Jet&) {
        return barrel_hits(tp) + endcap_hits(tp);
      });
    }
    if (name == "numberOfNextToInnermostPixelLayerHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfNextToInnermostPixelLayerHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfNextToInnermostPixelLayerEndcapHits");
      return CJGetter<Tp>([barrel_hits, endcap_hits](const Tp& tp, const Jet&) {
        return barrel_hits(tp) + endcap_hits(tp);
      });
    }
    if (name == "numberOfInnermostPixelLayerSharedHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfInnermostPixelLayerSharedHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfInnermostPixelLayerSharedEndcapHits");
      return CJGetter<Tp>([barrel_hits, endcap_hits](const Tp& tp, const Jet&) {
        return barrel_hits(tp) + endcap_hits(tp);
      });
    }
    if (name == "numberOfInnermostPixelLayerSplitHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfInnermostPixelLayerSplitHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfInnermostPixelLayerSplitEndcapHits");
      return CJGetter<Tp>([barrel_hits, endcap_hits](const Tp& tp, const Jet&) {
        return barrel_hits(tp) + endcap_hits(tp);
      });
    }
    return std::nullopt;
  }


  // Getters from general xAOD::IParticle and derived classes
  template <typename T>
  std::optional<
  std::function<std::vector<double>(
    const xAOD::Jet&,
    const std::vector<const T*>&)>
  >
  getterFromIParticles(const std::string& name)
  {
    using Jet = xAOD::Jet;

    if (name == "pt") {
      return CJGetter<T>([](const T& p, const Jet&) {
        return p.pt();
      });
    }
    if (name == "log_pt") {
      return CJGetter<T>([](const T& p, const Jet&) {
        return std::log(p.pt());
      });
    }
    if (name == "ptfrac") {
      return CJGetter<T>([](const T& p, const Jet& j) {
        return p.pt() / j.pt();
      });
    }
    if (name == "log_ptfrac") {
      return CJGetter<T>([](const T& p, const Jet& j) {
        return std::log(p.pt() / j.pt());
      });
    }

    if (name == "eta") {
      return CJGetter<T>([](const T& p, const Jet&) {
        return p.eta();
      });
    }
    if (name == "deta") {
      return CJGetter<T>([](const T& p, const Jet& j) {
        return p.eta() - j.eta();
      });
    }
    if (name == "abs_deta") {
      return CJGetter<T>([](const T& p, const Jet& j) {
        return copysign(1.0, j.eta()) * (p.eta() - j.eta());
      });
    }

    if (name == "phi") {
      return CJGetter<T>([](const T& p, const Jet&) {
        return p.phi();
      });
    }
    if (name == "dphi") {
      return CJGetter<T>([](const T& p, const Jet& j) {
        return p.p4().DeltaPhi(j.p4());
      });
    }

    if (name == "dr") {
      return CJGetter<T>([](const T& p, const Jet& j) {
        return p.p4().DeltaR(j.p4());
      });
    }
    if (name == "log_dr") {
      return CJGetter<T>([](const T& p, const Jet& j) {
        return std::log(p.p4().DeltaR(j.p4()));
      });
    }
    if (name == "log_dr_nansafe") {
      return CJGetter<T>([](const T& p, const Jet& j) {
        return std::log(p.p4().DeltaR(j.p4()) + 1e-7);
      });
    }

    if (name == "mass") {
      return CJGetter<T>([](const T& p, const Jet&) {
        return p.m();
      });
    }
    if (name == "energy") {
      return CJGetter<T>([](const T& p, const Jet&) {
        return p.e();
      });
    }
    return std::nullopt;
  }

}
  namespace FlavorTagDiscriminants {
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
    customGetterAndName(const std::string& name) {
      auto getter = customGetter(name);
      return [name, getter](const xAOD::Jet& j) {
               return std::make_pair(name, getter(j));
             };
    }

    // Case for constituent variables
    // Returns getter function with dependencies
    template <typename T>
    std::pair<
    std::function<std::vector<double>(
      const xAOD::Jet&,
      const std::vector<const T*>&)>,
    std::set<std::string>>
    customSequenceGetterWithDeps(const std::string& name,
                                const std::string& prefix) {

      if constexpr (std::is_same_v<T, xAOD::TrackParticle>) {
        if (auto getter = getterFromTracksWithIpDep(name, prefix)) {
          auto deps = BTagTrackIpAccessor(prefix).getTrackIpDataDependencyNames();
          return {*getter, deps};
        }
        if (auto getter = getterFromTracksNoIpDep(name)) {
          return {*getter, {}};
        }
      }
      if (auto getter = getterFromIParticles<T>(name)){
        return {*getter, {}};
      }
      throw std::logic_error("no match for custom getter " + name);
    }
    
    // ________________________________________________________________________
    // Class implementation
    //
    template <typename T>
    std::pair<typename CustomSequenceGetter<T>::NamedSequenceFromConstituents, std::set<std::string>> 
    CustomSequenceGetter<T>::customNamedSeqGetterWithDeps(const std::string& name,
                                 const std::string& prefix) {
      auto [getter, deps] = customSequenceGetterWithDeps<T>(name, prefix);
      return {
        [n=name, g=getter](const xAOD::Jet& j,
                       const std::vector<const T*>& t) {
          return std::make_pair(n, g(j, t));
        },
        deps
      };
    }

    template <typename T>
    std::pair<typename CustomSequenceGetter<T>::NamedSequenceFromConstituents, std::set<std::string>> 
    CustomSequenceGetter<T>::seqFromConsituents(
        const InputVariableConfig& cfg, 
        const FTagOptions& options){
      const std::string prefix = options.track_prefix;
      switch (cfg.type) {
        case ConstituentsEDMType::INT: return {
            SequenceGetter<int, T>(cfg.name), {cfg.name}
          };
        case ConstituentsEDMType::FLOAT: return {
            SequenceGetter<float, T>(cfg.name), {cfg.name}
          };
        case ConstituentsEDMType::CHAR: return {
            SequenceGetter<char, T>(cfg.name), {cfg.name}
          };
        case ConstituentsEDMType::UCHAR: return {
            SequenceGetter<unsigned char, T>(cfg.name), {cfg.name}
          };
        case ConstituentsEDMType::CUSTOM_GETTER: {
          return customNamedSeqGetterWithDeps(
            cfg.name, options.track_prefix);
        }
        default: {
          throw std::logic_error("Unknown EDM type for constituent.");
        }
      }
    }

    template <typename T>
    CustomSequenceGetter<T>::CustomSequenceGetter(
      std::vector<InputVariableConfig> inputs,
      const FTagOptions& options)
    {
        std::map<std::string, std::string> remap = options.remap_scalar;
        for (const InputVariableConfig& input_cfg: inputs) {
          auto [seqGetter, seq_deps] = seqFromConsituents(
          input_cfg, options);

          if(input_cfg.flip_sign){
            auto seqGetter_flip=[g=seqGetter](const xAOD::Jet&jet, const Constituents& constituents){
              auto [n,v] = g(jet,constituents);
              std::for_each(v.begin(), v.end(), [](double &n){ n=-1.0*n; });
              return std::make_pair(n,v);
            };
            m_sequencesFromConstituents.push_back(seqGetter_flip);
          }
          else{
            m_sequencesFromConstituents.push_back(seqGetter);
          }
          m_deps.merge(seq_deps);
          if (auto h = remap.extract(input_cfg.name)){
            m_used_remap.insert(h.key());
          }
        }
    }

    template <typename T>
    std::pair<std::vector<float>, std::vector<int64_t>> CustomSequenceGetter<T>::getFeats(
      const xAOD::Jet& jet, const Constituents& constituents) const
    {
      std::vector<float> cnsts_feats;
      int num_vars = m_sequencesFromConstituents.size();
      int num_cnsts = 0;

      int cnst_var_idx = 0;
      for (const auto& seq_builder: m_sequencesFromConstituents){
        auto double_vec = seq_builder(jet, constituents).second;

        if (cnst_var_idx==0){
            num_cnsts = static_cast<int>(double_vec.size());
            cnsts_feats.resize(num_cnsts * num_vars);
        }

        // need to transpose + flatten
        for (unsigned int cnst_idx=0; cnst_idx<double_vec.size(); cnst_idx++){
          cnsts_feats.at(cnst_idx*num_vars + cnst_var_idx)
              = double_vec.at(cnst_idx);
        }
        cnst_var_idx++;
      }
      std::vector<int64_t> cnsts_feat_dim = {num_cnsts, num_vars};
      return {cnsts_feats, cnsts_feat_dim};
    }

    template <typename T>
    std::map<std::string, std::vector<double>> CustomSequenceGetter<T>::getDL2Feats(
      const xAOD::Jet& jet, const Constituents& constituents) const
    {
      std::map<std::string, std::vector<double>> feats;
      for (const auto& seq_builder: m_sequencesFromConstituents){
        feats.insert(seq_builder(jet, constituents));
      }
      return feats;
    }

    template <typename T>
    std::set<std::string> CustomSequenceGetter<T>::getDependencies() const {
      return m_deps;
    }
    template <typename T>
    std::set<std::string> CustomSequenceGetter<T>::getUsedRemap() const {
      return m_used_remap;
    }


    // Explicit instantiations of supported types (IParticle, TrackParticle)
    template class CustomSequenceGetter<xAOD::IParticle>;
    template class CustomSequenceGetter<xAOD::TrackParticle>;
  }
}