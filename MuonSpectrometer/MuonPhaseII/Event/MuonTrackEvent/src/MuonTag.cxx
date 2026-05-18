
/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/MuonTag.h"

namespace {
    using Author = MuonR4::MuonTag::Author;
}

namespace MuonR4 {
    const std::vector<const xAOD::MuonSegment*>& MuonTag::segments() const {
        return m_segments;
    }
    Author MuonTag::author() const { return m_author; }

    const xAOD::TrackParticle* MuonTag::idTrack() const { return m_idTrack; }
    const xAOD::TrackParticle* MuonTag::cbTrack() const { return m_cbTrack; }
    const xAOD::TrackParticle* MuonTag::msTrack() const { return m_msTrack; }
    const HitSummary* MuonTag::summary() const { return m_summary.get(); }
    void MuonTag::copyParameters(xAOD::Muon& muon) const {
        for (const auto& [par, data] : m_params) {
            std::visit([par, &muon](const auto& d) {
               muon.setParameter(d, par);
            }, data);
        }
    }

    const xAOD::TrackParticle* MuonTag::primaryTrack() const {
        if (m_cbTrack != nullptr) {
            return m_cbTrack;
        } else if (m_idTrack != nullptr) {
            return m_idTrack;
        }
        return m_msTrack;
    }

    void MuonTag::setIdTrack(const xAOD::TrackParticle* idTrack) {
        assert(idTrack != nullptr);
        m_idTrack = idTrack;
    }
     
    void MuonTag::setCbTrack(const xAOD::TrackParticle* cbTrack) {
        assert(cbTrack != nullptr);
        m_cbTrack = cbTrack;
    }
    void MuonTag::setMsTrack(const xAOD::TrackParticle* msTrack) {
        assert (msTrack != nullptr);
        m_msTrack = msTrack;
    }

    void MuonTag::setSegments(const std::span<const xAOD::MuonSegment*> segs){
        m_segments.insert(m_segments.end(), segs.begin(), segs.end());
    }
      
    void MuonTag::setParameter(const ParamDef par,  ParamData_t data) {
        m_params[par] = std::move(data);
    }
    void MuonTag::setAuthor(const Author author) {
        m_author = author;
    }
    void MuonTag::setSummary(HitSummary&& summary) {
        m_summary = std::make_unique<HitSummary>(std::move(summary));
    }
}