/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsEvent/CaloExtension.h"
#include "ActsEvent/Decoration.h"

#include "GeoModelKernel/throwExcept.h"

namespace ActsTrk{
    const CaloExtension* getCaloExtension(const xAOD::TrackParticle& track) {
        static const SG::ConstAccessor<ElementLink<CaloExtensionContainer>> acc{"caloExtensionLink"};
        if (!acc.isAvailable(track)) {
            return nullptr;
        }
        const auto& link = acc(track);
        if (link.isValid()) {
            return *link;
        }
        return nullptr;
    }


    CaloExtension::CaloExtension(const xAOD::TrackParticle* track):
        m_idTrack{track} {}

    const xAOD::TrackParticle* CaloExtension::track() const {
        return m_idTrack;
    }
    std::optional<Acts::BoundTrackParameters> CaloExtension::lastTrackParameters() const {
        return m_idTrack ? ActsTrk::lastTrackParameters(*m_idTrack) : std::nullopt;
    }
    std::optional<Acts::BoundTrackParameters> CaloExtension::lastParameters() const {
        if (! m_parameters.empty()) {
            return  m_parameters.back();
        }
        return lastTrackParameters();
    }

    void CaloExtension::appendParameters(Acts::BoundTrackParameters&& pars) {
        m_parameters.emplace_back(std::move(pars));
    }
    void CaloExtension::associateCluster(const xAOD::CaloCluster* clust) {
        if (!clust) {
            THROW_EXCEPTION("The calorimeter cluster not be a nullptr");
        }
        if (!Acts::rangeContainsValue(m_clusters, clust)) {
            m_clusters.push_back(clust);
        }
    }
    const CaloExtension::ParamVec_t& CaloExtension::parameters() const {
        return m_parameters;
    }
    const CaloExtension::ClusterVec_t& CaloExtension::associatedClusters() const {
        return m_clusters;
    }
    std::size_t CaloExtension::size() const {
        return m_parameters.size();
    }
    bool CaloExtension::empty() const {
        return m_parameters.empty();
    }
}
