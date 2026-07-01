/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  Loader for CaloCalFwdTopoTowers accessed via GhostTower ElementLinks.
  Single-hop navigation: jet -> GhostTower -> CaloCluster.
*/

#ifndef TOWER_LOADER_H
#define TOWER_LOADER_H

#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/CustomGetterUtils.h"

#include "xAODCaloEvent/CaloCluster.h"

#include <string>
#include <vector>
#include <set>
#include <functional>

namespace FlavorTagInference {

    class TowerLoader : public IConstituentsLoader {
      public:
        TowerLoader(const ConstituentsInputConfig& cfg,
                    const FTagOptions& options);
        Inputs getData(const xAOD::IParticle& jet) const override;
        const FTagDataDependencyNames& getDependencies() const override;
        const std::set<std::string>& getUsedRemap() const override;
        const std::string& getName() const override;
        const ConstituentsType& getType() const override;
      private:
        using IPC = xAOD::IParticleContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using Towers = std::vector<const xAOD::CaloCluster*>;

        using SortVar = std::function<double(const xAOD::CaloCluster*,
                                             const xAOD::IParticle&)>;
        SortVar m_sortVar;
        getter_utils::SeqGetter<xAOD::CaloCluster> m_seqGetter;

        Towers getTowersFromJet(const xAOD::IParticle& jet) const;
    };
}

#endif
