/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/CaloClusterLoader.h"
#include "xAODBase/IParticle.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODCaloEvent/CaloCluster.h"
#include <unordered_set>
#include <vector>

namespace FlavorTagInference {

    // factory for functions which return the sort variable we
    // use to order calo clusters
    CaloClusterLoader::CaloClusterSortVar CaloClusterLoader::caloClusterSortVar(
        ConstituentsSortOrder config)
    {
      typedef xAOD::CaloCluster CC;
      typedef xAOD::IParticle Jet;
      switch(config) {
        case ConstituentsSortOrder::PT_DESCENDING:
          return [](const CC* p, const Jet&) {return p->pt();};
        default: {
          throw std::logic_error("Unknown sort function");
        }
      }
    } // end of CaloCluster sort getter

    CaloClusterLoader::CaloClusterLoader(
        const ConstituentsInputConfig& cfg,
        const FTagOptions& options
    ):
        IConstituentsLoader(cfg),
        m_caloClusterSortVar(CaloClusterLoader::caloClusterSortVar(cfg.order)),
        m_seqGetter(getter_utils::SeqGetter<xAOD::CaloCluster>(
          cfg.inputs, options))
    {
        m_used_remap = m_seqGetter.getUsedRemap();
        m_name = cfg.name;
        m_skipInvalidLinks = options.skip_invalid_links;
    }

    CaloClusterLoader::CaloClusters CaloClusterLoader::getCaloClustersFromJet(
        const xAOD::IParticle& jet
    ) const
    {
        // Three-hop navigation for UFO jets:
        //   jet -> constituentLinks -> FlowElement (UFO)
        //       -> otherObjects() -> FlowElement (intermediate CSSK PFO)
        //       -> otherObjects() -> CaloCluster
        // Two-hop navigation for PFlow jets (small-R):
        //   jet -> constituentLinks -> FlowElement (PFO)
        //       -> otherObjects() -> CaloCluster
        // Dedup via unordered_set because UFO->cluster is many-to-many.
        static const SG::ConstAccessor<PartLinks> acc("constituentLinks");

        std::vector<std::pair<double, const xAOD::CaloCluster*>> clusters;
        std::unordered_set<const xAOD::CaloCluster*> seen;

        for (const ElementLink<IPC>& link : acc(jet)) {
            if (!link.isValid()) {
                if (m_skipInvalidLinks) { continue; }
                throw std::logic_error("invalid constituentLink in CaloClusterLoader");
            }
            const auto* flow = dynamic_cast<const xAOD::FlowElement*>(*link);
            if (!flow) {
                continue;
            }
            // Second hop: FlowElement -> otherObjects()
            for (const auto* other : flow->otherObjects()) {
                if (!other) {
                    continue;
                }
                // PFlow path: `other` is already a CaloCluster.
                if (const auto* cluster = dynamic_cast<const xAOD::CaloCluster*>(other)) {
                    if (seen.insert(cluster).second) {
                        clusters.push_back({m_caloClusterSortVar(cluster, jet), cluster});
                    }
                    continue;
                }
                // UFO path: `other` is an intermediate FlowElement
                // (e.g. CSSKGParticleFlowObject). Recurse one more hop.
                if (const auto* intermediate = dynamic_cast<const xAOD::FlowElement*>(other)) {
                    for (const auto* grandOther : intermediate->otherObjects()) {
                        if (!grandOther) { continue; }
                        const auto* cluster = dynamic_cast<const xAOD::CaloCluster*>(grandOther);
                        if (cluster && seen.insert(cluster).second) {
                            clusters.push_back({m_caloClusterSortVar(cluster, jet), cluster});
                        }
                    }
                }
            }
        }

        // Sort by pt descending
        std::sort(clusters.begin(), clusters.end(), std::greater<>());

        CaloClusters sorted_clusters;
        sorted_clusters.reserve(clusters.size());
        for (const auto& cl : clusters) {
            sorted_clusters.push_back(cl.second);
        }
        return sorted_clusters;
    }

    Inputs CaloClusterLoader::getData(const xAOD::IParticle& jet) const {
        CaloClusters sorted_clusters = getCaloClustersFromJet(jet);
        return m_seqGetter.getFeats(jet, sorted_clusters);
    }

    const FTagDataDependencyNames& CaloClusterLoader::getDependencies() const {
        return m_deps;
    }
    const std::set<std::string>& CaloClusterLoader::getUsedRemap() const {
        return m_used_remap;
    }
    const std::string& CaloClusterLoader::getName() const {
        return m_name;
    }
    const ConstituentsType& CaloClusterLoader::getType() const {
        return m_config.type;
    }

}
