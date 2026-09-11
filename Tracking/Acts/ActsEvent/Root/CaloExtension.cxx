/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsEvent/CaloExtension.h"
#include "ActsEvent/Decoration.h"
#include "ActsEvent/ParticleHypothesisEncoding.h"
#include "ActsInterop/UnitConverters.h"

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
        if (!m_idTrack) {
            return std::nullopt;
        }
        if (auto result = ActsTrk::lastTrackParameters(*m_idTrack); result != std::nullopt){
            return result;
        }
        /// Fall back solution
        unsigned int lastMeasIdx = 0;
        if (!m_idTrack->indexOfParameterAtPosition(lastMeasIdx, xAOD::LastMeasurement)) {
            return std::nullopt;
        }

        const Acts::Vector3 lastPos{m_idTrack->parameterX(lastMeasIdx),
                                    m_idTrack->parameterY(lastMeasIdx),
                                    m_idTrack->parameterZ(lastMeasIdx)};

        const Acts::Vector3 lastMom{m_idTrack->parameterPX(lastMeasIdx),
                                    m_idTrack->parameterPY(lastMeasIdx),
                                    m_idTrack->parameterPZ(lastMeasIdx)};
        Acts::BoundMatrix cov{Acts::BoundMatrix::Identity()};

        return Acts::BoundTrackParameters::createCurvilinear(
                    convertPosToActs(lastPos, m_idTrack->hasValidTime() ? m_idTrack->time() : 0.), 
                    lastMom.unit(), m_idTrack->charge() / energyToActs(lastMom.mag()),
                    cov, ParticleHypothesis::convert(m_idTrack->particleHypothesis()));
    }
    std::optional<Acts::BoundTrackParameters> CaloExtension::lastParameters() const {
        if (!m_parameters.empty()) {
            return m_parameters.back();
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
