/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVertexUtils/TrackletSegment.h"


TrackletSegment::TrackletSegment(const Muon::IMuonIdHelperSvc* idHelperSvc,
                                 const std::vector<const Muon::MdtPrepData*>& mdts, 
                                 const Amg::Vector3D& gpos, 
                                 double alpha, double dalpha, 
                                 double rErr, double zErr, 
                                 int pattern) : 
    m_idHelperSvc(idHelperSvc),
    m_mdts(mdts),
    m_gpos(gpos),
    m_alpha(alpha),
    m_dalpha(dalpha),
    m_rErr(rErr),
    m_zErr(zErr),
    m_pattern(pattern) {}

TrackletSegment::~TrackletSegment() = default;

// set functions
void TrackletSegment::clearMdt() { m_mdts.clear(); }
void TrackletSegment::isCombined(bool iscomb) { m_isCombined = iscomb; }

// get functions
const std::vector<const Muon::MdtPrepData*>& TrackletSegment::mdtHitsOnTrack() const { return m_mdts; }
const Identifier TrackletSegment::getIdentifier() const {return m_mdts.at(0)->identify();}
const Amg::Vector3D& TrackletSegment::globalPosition() const { return m_gpos; }
double TrackletSegment::alpha() const { return m_alpha; }
double TrackletSegment::alphaError() const { return m_dalpha; }
double TrackletSegment::zError() const { return m_zErr; }
double TrackletSegment::rError() const { return m_rErr; }
int TrackletSegment::getHitPattern() const { return m_pattern; }
bool TrackletSegment::isCombined() const { return m_isCombined; }

// get properties of the MDT chamber
int TrackletSegment::mdtChamber() const { return m_idHelperSvc->mdtIdHelper().stationName(getIdentifier()); }
int TrackletSegment::mdtChEta() const { return m_idHelperSvc->mdtIdHelper().stationEta(getIdentifier()); }
int TrackletSegment::mdtChPhi() const { return m_idHelperSvc->mdtIdHelper().stationPhi(getIdentifier()); }
double TrackletSegment::getChMidPoint() const { 
    double mlmidpt = 0;
    if (m_idHelperSvc->mdtIdHelper().isBarrel(getIdentifier()))
        mlmidpt = std::hypot(m_mdts.at(0)->detectorElement()->center().x(), m_mdts.at(0)->detectorElement()->center().y());
    else if (m_idHelperSvc->mdtIdHelper().isEndcap(getIdentifier()))
        mlmidpt = m_mdts.at(0)->detectorElement()->center().z();
    
    return mlmidpt;
}
