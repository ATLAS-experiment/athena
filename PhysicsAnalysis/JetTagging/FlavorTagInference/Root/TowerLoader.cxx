/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/TowerLoader.h"
#include "xAODBase/IParticle.h"
#include <vector>
#include <stdexcept>

namespace FlavorTagInference {

    TowerLoader::TowerLoader(
        const ConstituentsInputConfig& cfg,
        const FTagOptions& options
    ):
        IConstituentsLoader(cfg),
        m_sortVar([](const xAOD::CaloCluster* p, const xAOD::IParticle&) {
            return p->pt();
        }),
        m_seqGetter(getter_utils::SeqGetter<xAOD::CaloCluster>(
          cfg.inputs, options))
    {
        m_used_remap = m_seqGetter.getUsedRemap();
        m_name = cfg.name;
    }

    TowerLoader::Towers TowerLoader::getTowersFromJet(
        const xAOD::IParticle& jet
    ) const
    {
        // Single-hop: jet -> GhostTower ElementLinks -> CaloCluster
        static const SG::ConstAccessor<PartLinks> acc("GhostTower");

        Towers towers;

        if (!acc.isAvailable(jet)) {
            return towers;
        }

        for (const ElementLink<IPC>& link : acc(jet)) {
            if (!link.isValid()) {
                throw std::logic_error("invalid GhostTower link in TowerLoader");
            }
            const auto* cluster = dynamic_cast<const xAOD::CaloCluster*>(*link);
            if (cluster) {
                towers.push_back(cluster);
            }
        }

        // Sort by pT descending
        std::sort(towers.begin(), towers.end(),
            [this, &jet](const xAOD::CaloCluster* a, const xAOD::CaloCluster* b) {
                return m_sortVar(a, jet) > m_sortVar(b, jet);
            });

        return towers;
    }

    Inputs TowerLoader::getData(const xAOD::IParticle& jet) const {
        Towers sorted_towers = getTowersFromJet(jet);
        return m_seqGetter.getFeats(jet, sorted_towers);
    }

    const FTagDataDependencyNames& TowerLoader::getDependencies() const {
        return m_deps;
    }
    const std::set<std::string>& TowerLoader::getUsedRemap() const {
        return m_used_remap;
    }
    const std::string& TowerLoader::getName() const {
        return m_name;
    }
    const ConstituentsType& TowerLoader::getType() const {
        return m_config.type;
    }

}
