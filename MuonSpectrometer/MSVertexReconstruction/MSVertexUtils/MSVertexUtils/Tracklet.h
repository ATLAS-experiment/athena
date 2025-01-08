/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include <vector>
#include <cmath>

#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MSVertexUtils/TrackletSegment.h"
#include "xAODTracking/TrackParticle.h"


// HV Track Class
class Tracklet {
    private:
        TrackletSegment m_ml1seg{}, m_ml2seg{};
        Amg::Vector3D m_momentum{Amg::Vector3D::Zero()};
        Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
        std::vector<const Muon::MdtPrepData*> m_mdts{};
        AmgSymMatrix(5) m_ErrorMatrix{};
        double m_charge{0.};
        xAOD::TrackParticle* m_track{nullptr};

    public:
        Tracklet() = default;
        Tracklet(const TrackletSegment& ML1seg, 
                const TrackletSegment& ML2seg, 
                const Amg::Vector3D& p, 
                const AmgSymMatrix(5) & ErrorMatrix,
                double charge);
        Tracklet(const TrackletSegment& ML1seg, 
                const Amg::Vector3D& p, 
                const AmgSymMatrix(5) & ErrorMatrix, 
                double charge);

        ~Tracklet();

        // set functions
        void setTrackParticle(xAOD::TrackParticle* track);
        void momentum(const Amg::Vector3D& p);
        void charge(double charge);

        // get functions
        const xAOD::TrackParticle* getTrackParticle() const;
        const std::vector<const Muon::MdtPrepData*>& mdtHitsOnTrack() const;
        Identifier muonIdentifier() const;
        const TrackletSegment& getML1seg() const;
        const TrackletSegment& getML2seg() const;
        const Amg::Vector3D& globalPosition() const;
        const Amg::Vector3D& momentum() const;
        double alpha() const;
        double deltaAlpha() const;
        const AmgSymMatrix(5) & errorMatrix() const;
        double charge() const;
        int mdtChamber() const;
        int mdtChEta() const;
        int mdtChPhi() const;
};
