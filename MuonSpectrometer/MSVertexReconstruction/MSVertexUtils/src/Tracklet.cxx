/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVertexUtils/Tracklet.h"


Tracklet::Tracklet(const TrackletSegment& ML1seg,
                   const TrackletSegment& ML2seg, 
                   const Amg::Vector3D& p,
                   const AmgSymMatrix(5) & ErrorMatrix, 
                   double charge) :
    m_ml1seg(ML1seg),
    m_ml2seg(ML2seg),
    m_momentum(p),
    m_pos(ML1seg.globalPosition()),
    m_mdts(ML1seg.mdtHitsOnTrack()),
    m_ErrorMatrix(ErrorMatrix),
    m_charge(charge) {
    for (const Muon::MdtPrepData* mdt_ml2 : m_ml2seg.mdtHitsOnTrack()) m_mdts.push_back(mdt_ml2);
}

Tracklet::Tracklet(const TrackletSegment& ML1seg, 
                   const Amg::Vector3D& p, 
                   const AmgSymMatrix(5) & ErrorMatrix, 
                   double charge) : 
    m_ml1seg(ML1seg),
    m_ml2seg(ML1seg),
    m_momentum(p),
    m_pos(ML1seg.globalPosition()),
    m_mdts(ML1seg.mdtHitsOnTrack()),
    m_ErrorMatrix(ErrorMatrix),
    m_charge(charge) {}

Tracklet::~Tracklet() = default;


// set functions
void Tracklet::setTrackParticle(xAOD::TrackParticle* track){ m_track = track; }
void Tracklet::momentum(const Amg::Vector3D& p) { m_momentum = p; }
void Tracklet::charge(double charge) { m_charge = charge; }

// get functions
const xAOD::TrackParticle* Tracklet::getTrackParticle() const { return m_track; }
const std::vector<const Muon::MdtPrepData*>& Tracklet::mdtHitsOnTrack() const { return m_mdts; }
Identifier Tracklet::muonIdentifier() const { return m_ml1seg.getIdentifier(); }
const TrackletSegment& Tracklet::getML1seg() const { return m_ml1seg; }
const TrackletSegment& Tracklet::getML2seg() const { return m_ml2seg; }
const Amg::Vector3D& Tracklet::globalPosition() const { return m_pos; }
const Amg::Vector3D& Tracklet::momentum() const { return m_momentum; }
double Tracklet::alpha() const { return std::atan2(m_momentum.perp(), m_momentum.z()); }
double Tracklet::deltaAlpha() const { return (m_ml1seg.alpha() - m_ml2seg.alpha()); }
double Tracklet::charge() const { return m_charge; }
const AmgSymMatrix(5) & Tracklet::errorMatrix() const { return m_ErrorMatrix; }
int Tracklet::mdtChamber() const { return m_ml1seg.mdtChamber(); }
int Tracklet::mdtChEta() const { return m_ml1seg.mdtChEta(); }
int Tracklet::mdtChPhi() const { return m_ml1seg.mdtChPhi(); }