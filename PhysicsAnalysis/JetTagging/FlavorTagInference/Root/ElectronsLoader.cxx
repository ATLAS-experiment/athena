/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/ElectronsLoader.h"
#include "xAODBase/IParticle.h"
#include "xAODEgamma/ElectronFwd.h"
#include "FlavorTagInference/ConstituentsLoader.h"

namespace FlavorTagInference {
    
    // factory for functions which return the sort variable we
    // use to order Electrons
    ElectronsLoader::ElectronSortVar ElectronsLoader::electronSortVar(
        ConstituentsSortOrder config) 
    {
      typedef xAOD::Electron Ip;
      typedef xAOD::Jet Jet;
      switch(config) {
        case ConstituentsSortOrder::PT_DESCENDING:
          return [](const Ip* p, const Jet&) {return p->pt();};
        default: {
          throw std::logic_error("Unknown sort function");
        }
      }
    } // end of iparticle sort getter

    // factory for functions that return true for electrons we want to
    // use, false for those we don't want
    std::pair<ElectronsLoader::ElectronFilter,std::set<std::string>> ElectronsLoader::electronFilter(
      ConstituentsSelection config)
    {
        typedef SG::AuxElement AE;
        // make sure we record accessors as data dependencies
        std::set<std::string> electron_deps;
        auto addAccessor = [&electron_deps](const std::string& n) {
                             AE::ConstAccessor<float> a(n);
                             electron_deps.insert(n);
                             return a;
                           };
        auto iso_pt = addAccessor("ptvarcone30_Nonprompt_All_MaxWeightTTVALooseCone_pt1000");

        switch (config){
          case ConstituentsSelection::R22_DEFAULT:
            return {
                [iso_pt](const xAOD::Jet& jet, const xAOD::Electron* el) {
                  TLorentzVector jet_4vec = jet.p4();
                  TLorentzVector el_4vec = el->p4();

                  float el_dpop = -1;
                  unsigned int index;
                  auto track = el->trackParticle();
                  if (track->indexOfParameterAtPosition(index, xAOD::LastMeasurement))
                  {
                      double refittedTrack_LMqoverp = track->charge() / std::sqrt(std::pow(track->parameterPX(index), 2) +
                                                                                  std::pow(track->parameterPY(index), 2) +
                                                                                  std::pow(track->parameterPZ(index), 2));
                      el_dpop = 1 - track->qOverP() / (refittedTrack_LMqoverp);
                  }
                  
                  float el_ptrel = el_4vec.Vect().Perp(jet_4vec.Vect());
                  // Get shower shapes
                  float el_rhad1 = el->showerShapeValue(xAOD::EgammaParameters::Rhad1);
                  float el_wstot = el->showerShapeValue(xAOD::EgammaParameters::wtots1);
                  float el_rphi  = el->showerShapeValue(xAOD::EgammaParameters::Rphi);
                  float el_reta  = el->showerShapeValue(xAOD::EgammaParameters::Reta);
                  float el_deta1 = el->trackCaloMatchValue(xAOD::EgammaParameters::deltaEta1);

                  if (std::abs(el->eta()) > 2.5) return false;
                  if (el->pt() <= 1000) return false;
                  if (el->pt() >= 500000) return false;
                  if (std::abs(track->d0()) >= 1) return false;
                  if (std::abs(el->caloCluster()->e() * track->qOverP()) > 30) return false;
                  if (std::abs(el->caloCluster()->e() / std::cosh(track->eta())) > 50000) return false;
                  if (std::abs(iso_pt(*el) / el->pt()) > 40) return false;
                  if (std::abs(el_ptrel) > 5000) return false;
                  if (std::abs(el_rhad1) > 4) return false;
                  if (std::abs(el_wstot) > 20) return false;
                  if (std::abs(el_rphi) > 2) return false;
                  if (std::abs(el_reta) > 2) return false;
                  if (std::abs(el_deta1) > 10) return false;
                  if (std::abs(el_dpop) > 5) return false;
                  return true;
              }, electron_deps
            };
        default:
          throw std::logic_error("unknown electron selection function");
        }
    }

    ElectronsLoader::ElectronsLoader(
        const ConstituentsInputConfig& cfg,
        const FTagOptions& options
    ):
        IConstituentsLoader(cfg),
        m_electronSortVar(ElectronsLoader::electronSortVar(cfg.order)),
        m_electronFilter(ElectronsLoader::electronFilter(cfg.selection).first),
        m_seqGetter(getter_utils::SeqGetter<xAOD::Electron>(
          cfg.inputs, options))
    {
        SG::AuxElement::ConstAccessor<PartLinks> acc(options.electron_link_name);
        m_associator = [acc](const xAOD::Jet& jet) -> IPV {
          IPV electrons;
          for (const ElementLink<IPC>& link : acc(jet)){
            if (!link.isValid()) {
              throw std::logic_error("invalid particle link");
            }
            const xAOD::Electron* el = dynamic_cast<const xAOD::Electron*>(*link);
            if (!el) {
              throw std::logic_error("iparticle does not cast to Electron");
            }
            electrons.push_back(el);
          }
          return electrons;
        };
        m_used_remap = m_seqGetter.getUsedRemap();
        m_deps.bTagInputs.insert(options.electron_link_name);
        m_name = cfg.name;
    }

    ElectronsLoader::Electrons ElectronsLoader::getElectronsFromJet(
        const xAOD::Jet& jet
    ) const
    {
        std::vector<std::pair<double, const xAOD::Electron*>> electrons;
        for (const xAOD::Electron *tp : m_associator(jet)) {
          if (m_electronFilter(jet, tp)){
            electrons.push_back({m_electronSortVar(tp, jet), tp});
          }
        }
        std::sort(electrons.begin(), electrons.end(), std::greater<>());
        std::vector<const xAOD::Electron*> only_electrons;
        only_electrons.reserve(electrons.size());
        for (const auto& el: electrons) {
          only_electrons.push_back(el.second);
        }
        return only_electrons;
    }

    std::tuple<std::string, Inputs, std::vector<const xAOD::IParticle*>> ElectronsLoader::getData(const xAOD::Jet& jet) const {
        Electrons sorted_electrons = getElectronsFromJet(jet);

        // We return a dummy vector of IParticles as we don't decorate flow elements
        return {m_config.output_name, m_seqGetter.getFeats(jet, sorted_electrons), std::vector<const xAOD::IParticle*>{}};
    }

    const FTagDataDependencyNames& ElectronsLoader::getDependencies() const {
        return m_deps;
    }
    const std::set<std::string>& ElectronsLoader::getUsedRemap() const {
        return m_used_remap;
    }
    const std::string& ElectronsLoader::getName() const {
        return m_name;
    }
    const ConstituentsType& ElectronsLoader::getType() const {
        return m_config.type;
    }

}
