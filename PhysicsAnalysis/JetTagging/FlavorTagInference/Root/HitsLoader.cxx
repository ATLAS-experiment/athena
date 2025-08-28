/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/HitsLoader.h"
#include "xAODPFlow/FlowElement.h"
#include "FlavorTagInference/StringUtils.h"

namespace FlavorTagInference {
    

    HitsLoader::HitsLoader(
        const ConstituentsInputConfig& cfg,
        const FTagOptions& options
    ):
        IConstituentsLoader(cfg),
        m_seqGetter(getter_utils::SeqGetter<xAOD::TrackMeasurementValidation>(
          cfg.inputs, options))
    {
        SG::AuxElement::ConstAccessor<HitLinks> acc("hitsAssociatedWithJet");
        m_associator = [acc](const xAOD::Jet& jet) -> TMVV {
          TMVV hits;
          for (const ElementLink<TMC>& link : acc(jet)){
            if (!link.isValid()) {
              throw std::logic_error("invalid hits link");
            }
            hits.push_back(*link);
          }
          return hits;
        };

        m_used_remap = m_seqGetter.getUsedRemap();
        m_name = cfg.name;
    }

    std::vector<const xAOD::TrackMeasurementValidation*> HitsLoader::getHitsFromJet(
        const xAOD::Jet& jet
    ) const
    {
        std::vector<const xAOD::TrackMeasurementValidation*> hits;
        for (const xAOD::TrackMeasurementValidation *tp : m_associator(jet)) {
          hits.push_back(tp);
        }
        return hits;
    }

    std::tuple<std::string, Inputs, std::vector<const xAOD::IParticle*>> HitsLoader::getData(
      const xAOD::Jet& jet) const {
        Hits sorted_hits = getHitsFromJet(jet);
        std::vector<const xAOD::IParticle*> dummy;
        return std::make_tuple(m_config.output_name, m_seqGetter.getFeats(jet, sorted_hits), dummy);
    }

    const FTagDataDependencyNames& HitsLoader::getDependencies() const {
        return m_deps;
    }
    const std::set<std::string>& HitsLoader::getUsedRemap() const {
        return m_used_remap;
    }
    const std::string& HitsLoader::getName() const {
        return m_name;
    }
    const ConstituentsType& HitsLoader::getType() const {
        return m_config.type;
    }

}
