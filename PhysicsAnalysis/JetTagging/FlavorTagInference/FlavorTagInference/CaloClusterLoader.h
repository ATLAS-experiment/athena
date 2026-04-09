/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load CaloCluster
  objects from the jet via a two-hop navigation:
    jet -> constituentLinks -> FlowElement -> otherObjects() -> CaloCluster
  and extract their features for the NN evaluation.
*/

#ifndef CALO_CLUSTER_LOADER_H
#define CALO_CLUSTER_LOADER_H

// local includes
#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/CustomGetterUtils.h"

// EDM includes
#include "xAODJet/JetFwd.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODCaloEvent/CaloCluster.h"

// STL includes
#include <string>
#include <vector>
#include <functional>

namespace FlavorTagInference {

    // Subclass for CaloCluster loader inherited from abstract IConstituentsLoader class
    class CaloClusterLoader : public IConstituentsLoader {
      public:
        CaloClusterLoader(const ConstituentsInputConfig& cfg, const FTagOptions& options);
        Inputs getData(const xAOD::IParticle& jet) const override;
        const FTagDataDependencyNames& getDependencies() const override;
        const std::set<std::string>& getUsedRemap() const override;
        const std::string& getName() const override;
        const ConstituentsType& getType() const override;
      protected:
        // typedefs
        typedef xAOD::IParticle Jet;
        typedef std::pair<std::string, double> NamedVar;
        typedef std::pair<std::string, std::vector<double> > NamedSeq;
        // CaloCluster typedefs
        typedef std::vector<const xAOD::CaloCluster*> CaloClusters;
        typedef std::function<double(const xAOD::CaloCluster*,
                                    const Jet&)> CaloClusterSortVar;

        // usings for IParticle getter
        using AE = SG::AuxElement;
        using IPC = xAOD::IParticleContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;

        CaloClusterSortVar caloClusterSortVar(ConstituentsSortOrder);

        CaloClusters getCaloClustersFromJet(const xAOD::IParticle& jet) const;

        CaloClusterSortVar m_caloClusterSortVar;
        getter_utils::SeqGetter<xAOD::CaloCluster> m_seqGetter;
        bool m_skipInvalidLinks{false};
    };
}

#endif
