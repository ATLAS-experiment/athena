/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "InDetGNNHardScatterSelection/CustomGetterUtils.h"

#include <optional>

#include "xAODMuon/Muon.h"
#include "xAODJet/Jet.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"



namespace {

  using InDetGNNHardScatterSelection::getter_utils::SequenceFromTracks;
  using InDetGNNHardScatterSelection::getter_utils::SequenceFromIParticles;
  // ______________________________________________________________________
  // Custom getters for jet-wise quantities
  //
  // this function is not at all optimized, but then it doesn't have
  // to be since it should only be called in the initialization stage.
  //
  std::function<double(const xAOD::Vertex&)> customGetter(
    const std::string& name)
  {
    if (name == "z") {
      return [](const xAOD::Vertex& v) -> float {return v.z();};
    }
    throw std::logic_error("no match for custom getter " + name);
  }

  // _______________________________________________________________________
  // Custom getters for constituent variables (CJGetter -> Constituent and Vertex Getter)
  template <typename T>
  class CJGetter
  {
    using F = std::function<double(const T&, const xAOD::Vertex&)>;
    private:
      F m_getter;
    public:
        CJGetter(F getter):
        m_getter(getter)
        {}
      std::vector<double> operator()(
        const xAOD::Vertex& vertex,
        const std::vector<const T*>& particles) const {
        std::vector<double> sequence;
        sequence.reserve(particles.size());
        for (const auto* particle: particles) {
          sequence.push_back(m_getter(*particle, vertex));
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
      std::pair<std::string, std::vector<double>> operator()(const xAOD::Vertex&, const std::vector<const U*>& consts) const {
        std::vector<double> seq;
        for (const U* el: consts) {
          seq.push_back(m_getter(*el));
        }
        return {m_name, seq};
      }
  };


  // Getters from general xAOD::IParticle and derived classes
  template <typename T>
  std::optional<
  std::function<std::vector<double>(
    const xAOD::Vertex&,
    const std::vector<const T*>&)>
  >
  getterFromIParticles(const std::string& name)
  {
    using Vertex = xAOD::Vertex;

    if (name == "photon_deltaZ") {
      return CJGetter<T>([](const T& p, const Vertex& vertex) {
        if constexpr (std::is_same_v<T, xAOD::Photon>) {

          static const SG::AuxElement::ConstAccessor<float> acc("caloPointingZ");
          float calo = acc(p);

          return calo - vertex.z();
        }
        return (float)0.;

      });
    }

    if (name == "photon_deltaZ_wBeamSpot") {
      return CJGetter<T>([](const T& p, const Vertex& vertex) {
        if constexpr (std::is_same_v<T, xAOD::Photon>) {
        
          static const SG::AuxElement::ConstAccessor<float> acc_f("zCommon");
          static const SG::AuxElement::ConstAccessor<float> acc_s("zCommonError");

          float first = acc_f(p);
          float second = acc_s(p);

          return abs(first - vertex.z())/second;

        }        
        return (float)0.;
      });
    }

    if (name == "pt") {
      return CJGetter<T>([](const T& p, const Vertex&) {
        return p.pt();
      });
    }
    if (name == "eta") {
      return CJGetter<T>([](const T& p, const Vertex&) {
        return p.eta();
      });
    }
    if (name == "phi") {
      return CJGetter<T>([](const T& p, const Vertex&) {
        return p.phi();
      });
    }
    if (name == "vertexWeight") {
      return CJGetter<T>([](const T& p, const xAOD::Vertex& vertex) {
        for (size_t i = 0; i < vertex.nTrackParticles(); i++) {
          const xAOD::TrackParticle* trackParticle = dynamic_cast<const xAOD::TrackParticle*>(&p);
          if (vertex.trackParticle(i) == trackParticle) return vertex.trackWeight(i);
        }
        return (float)0.;
      });
    }

    if (name == "deltaZ0") {
      return CJGetter<T>([](const T& p, const Vertex& vertex) {
        if constexpr (std::is_same_v<T, xAOD::Electron>) {
          const auto *gsf_trk = p.trackParticle(0);
          if (gsf_trk) {
            const auto *id_trk = xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF(gsf_trk);
            if (id_trk) {
              return id_trk->z0() + id_trk->vz() - vertex.z();
            }
          }
        } else if constexpr (std::is_same_v<T, xAOD::Muon>) {
          auto tp = p.trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
          if (tp) {
            return tp->z0() + tp->vz() - vertex.z();
          }
        }
        return (float)0.;
      });
    }
    if (name == "ntracks_ga") {
      if constexpr (std::is_same_v<T, xAOD::Jet>) {
        return CJGetter<T>([](const T& p, const Vertex&) {
          std::vector<const xAOD::TrackParticle*> ghostTracks = p.template getAssociatedObjects<xAOD::TrackParticle>(xAOD::JetAttribute::GhostTrack);
          return ghostTracks.size();
        });
      }
    }
    return std::nullopt;
  }


}
  namespace InDetGNNHardScatterSelection {
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
    std::function<std::pair<std::string, double>(const xAOD::Vertex&)>
    customGetterAndName(const std::string& name) {
      auto getter = customGetter(name);
      return [name, getter](const xAOD::Vertex& j) {
               return std::make_pair(name, getter(j));
             };
    }

    // Case for constituent variables
    // Returns getter function with dependencies
    template <typename T>
    std::pair<
    std::function<std::vector<double>(
      const xAOD::Vertex&,
      const std::vector<const T*>&)>,
    std::set<std::string>>
    customSequenceGetterWithDeps(const std::string& name) {

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
    CustomSequenceGetter<T>::customNamedSeqGetterWithDeps(const std::string& name) {
      auto [getter, deps] = customSequenceGetterWithDeps<T>(name);
      return {
        [n=name, g=getter](const xAOD::Vertex& j,
                       const std::vector<const T*>& t) {
          return std::make_pair(n, g(j, t));
        },
        deps
      };
    }

    template <typename T>
    std::pair<typename CustomSequenceGetter<T>::NamedSequenceFromConstituents, std::set<std::string>> 
    CustomSequenceGetter<T>::seqFromConsituents(
        const InputVariableConfig& cfg){
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
            cfg.name);
        }
        default: {
          throw std::logic_error("Unknown EDM type for constituent.");
        }
      }
    }

    template <typename T>
    CustomSequenceGetter<T>::CustomSequenceGetter(
      const std::vector<InputVariableConfig> & inputs)
    {
        for (const InputVariableConfig& input_cfg: inputs) {
          auto [seqGetter, seq_deps] = seqFromConsituents(input_cfg);

          m_sequencesFromConstituents.push_back(seqGetter);
        }
    }

    template <typename T>
    std::pair<std::vector<float>, std::vector<int64_t>> CustomSequenceGetter<T>::getFeats(
      const xAOD::Vertex& vertex, const Constituents& constituents) const
    {
      std::vector<float> cnsts_feats;
      int num_vars = m_sequencesFromConstituents.size();
      int num_cnsts = 0;

      int cnst_var_idx = 0;
      for (const auto& seq_builder: m_sequencesFromConstituents){
        auto double_vec = seq_builder(vertex, constituents).second;

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

    // Explicit instantiations of supported types (IParticle, TrackParticle)
    template class CustomSequenceGetter<xAOD::IParticle>;
    template class CustomSequenceGetter<xAOD::TrackParticle>;
    template class CustomSequenceGetter<xAOD::Muon>;
    template class CustomSequenceGetter<xAOD::Electron>;
    template class CustomSequenceGetter<xAOD::Photon>;
    template class CustomSequenceGetter<xAOD::Jet>;
  }
}
