/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/MuonsLoader.h"
#include "xAODBase/IParticle.h"
#include "FlavorTagInference/ConstituentsLoader.h"

namespace FlavorTagInference {

    // factory for functions which return the sort variable we
    // use to order Muons
    MuonsLoader::MuonSortVar MuonsLoader::muonSortVar(
        ConstituentsSortOrder config)
    {
      typedef xAOD::Muon Ip;
      typedef xAOD::IParticle Jet;
      switch(config) {
        case ConstituentsSortOrder::PT_DESCENDING:
          return [](const Ip* p, const Jet&) {return p->pt();};
        default: {
          throw std::logic_error("Unknown sort function");
        }
      }
    } // end of iparticle sort getter

    // factory for functions that return true for muons we want to
    // use, false for those we don't want
    std::pair<MuonsLoader::MuonFilter,std::set<std::string>> MuonsLoader::muonFilter(
      ConstituentsSelection config)
    {
        // make sure we record accessors as data dependencies
        std::set<std::string> muon_deps;

        switch (config){
          case ConstituentsSelection::R22_DEFAULT:
            return {
                [](const xAOD::IParticle&, const xAOD::Muon* mu) {

                  if (std::abs(mu->eta()) > 2.5) return false;
                  if (mu->pt() <= 2000) return false;
                  if (mu->pt() >= 3000000) return false;
                  if (mu->muonType() != xAOD::Muon::MuonType::Combined) return false;

                  // Check that ID and MS tracks are good
                  auto InnerDetectorTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle);
                  auto MuonSpectrometerTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle);
                  if (!InnerDetectorTrack || !MuonSpectrometerTrack) return false;
                  if (MuonSpectrometerTrack->qOverP() == 0) return false;

                  float momBalSignif = 0;
                  mu->parameter(momBalSignif, xAOD::Muon::ParamDef::momentumBalanceSignificance);
                  if (momBalSignif == 0) return false;

                  return true;
              }, muon_deps
            };
        case ConstituentsSelection::R22_BJR:
            return {
                [](const xAOD::IParticle& jet, const xAOD::Muon* mu) {
                  if (mu->quality() > xAOD::Muon::Quality::Loose)
                    return false;
                  // Check minimum muon pT
                  if (mu->pt() < 1000 || mu->pt() > 500000) // 1 < pT < 500 GeV
                    return false;

                  if (std::abs(mu->eta()) > 2.5) // |eta| < 2.5
                    return false;

                  // Check if it's inside jet
                  if (jet.p4().DeltaR(mu->p4()) > 0.4)
                    return false;
                  
                  // Check relative pT
                  if (mu->p4().Vect().Perp(jet.p4().Vect()) > 5000) // pTrel < 5 GeV
                    return false;

                  const auto* track = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
                  if (!track)
                    return false;
                  if (std::abs(track->d0()) >= 1)
                    return false;
                  return true;
              }, muon_deps
            };
        default:
          throw std::logic_error("unknown muon selection function");
        }
    }

    MuonsLoader::MuonsLoader(
        const ConstituentsInputConfig& cfg,
        const FTagOptions& options
    ):
        IConstituentsLoader(cfg),
        m_muonSortVar(MuonsLoader::muonSortVar(cfg.order)),
        m_muonFilter(nullptr),
        m_seqGetter(getter_utils::SeqGetter<xAOD::Muon>(
          cfg.inputs, options))
    {

      // set the filter
      auto [filter, deps] = muonFilter(cfg.selection);
      m_muonFilter = filter;
      m_deps.muonInputs = deps;

      SG::AuxElement::ConstAccessor<PartLinks> acc(options.muon_link_name);
      m_associator = [acc](const xAOD::IParticle& jet) -> IPV {
        IPV muons;
        for (const ElementLink<IPC>& link : acc(jet)){
          if (!link.isValid()) {
            throw std::logic_error("invalid particle link");
          }
          const xAOD::Muon* el = dynamic_cast<const xAOD::Muon*>(*link);
          if (!el) {
            throw std::logic_error("iparticle does not cast to Muon");
          }
          muons.push_back(el);
        }
        return muons;
      };
      m_used_remap = m_seqGetter.getUsedRemap();
      m_deps.bTagInputs.insert(options.muon_link_name);
      m_name = cfg.name;
    }

    MuonsLoader::Muons MuonsLoader::getMuonsFromJet(
        const xAOD::IParticle& jet
    ) const
    {
        std::vector<std::pair<double, const xAOD::Muon*>> muons;
        for (const xAOD::Muon *tp : m_associator(jet)) {
          if (m_muonFilter(jet, tp)){
            muons.push_back({m_muonSortVar(tp, jet), tp});
          }
        }
        std::sort(muons.begin(), muons.end(), std::greater<>());
        std::vector<const xAOD::Muon*> only_muons;
        only_muons.reserve(muons.size());
        for (const auto& mu: muons) {
          only_muons.push_back(mu.second);
        }
        return only_muons;
    }

    Inputs MuonsLoader::getData(const xAOD::IParticle& jet) const {
        Muons sorted_muons = getMuonsFromJet(jet);
        return m_seqGetter.getFeats(jet, sorted_muons);
    }

    const FTagDataDependencyNames& MuonsLoader::getDependencies() const {
        return m_deps;
    }
    const std::set<std::string>& MuonsLoader::getUsedRemap() const {
        return m_used_remap;
    }
    const std::string& MuonsLoader::getName() const {
        return m_name;
    }
    const ConstituentsType& MuonsLoader::getType() const {
        return m_config.type;
    }

}
