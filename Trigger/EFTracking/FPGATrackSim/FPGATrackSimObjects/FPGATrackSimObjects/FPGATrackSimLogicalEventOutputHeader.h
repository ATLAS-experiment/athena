// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimLOGICALEVENTOUTPUTHEADER_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimLOGICALEVENTOUTPUTHEADER_H

#include "FPGATrackSimObjects/FPGATrackSimDataFlowInfo.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include <TObject.h>

class FPGATrackSimLogicalEventOutputHeader : public TObject {
public:

    FPGATrackSimLogicalEventOutputHeader() {};
    virtual ~FPGATrackSimLogicalEventOutputHeader();

    void reset(); //reset per event variables

    // First Stage FPGATrackSim Roads
    void getFPGATrackSimRoads_1st(std::vector<FPGATrackSimRoad*>& roads_1st) { roads_1st.reserve(m_FPGATrackSimRoads_1st.size()); for ( auto& r : m_FPGATrackSimRoads_1st) roads_1st.push_back(&r); }
    size_t nFPGATrackSimRoads_1st() const { return m_FPGATrackSimRoads_1st.size(); }
    void reserveFPGATrackSimRoads_1st(size_t size) { m_FPGATrackSimRoads_1st.reserve(size); }
    void addFPGATrackSimRoads_1st(std::vector<FPGATrackSimRoad*> const& roads_1st) { for ( auto& r : roads_1st) m_FPGATrackSimRoads_1st.push_back(*r); }

    // Second Stage FPGATrackSim Roads
    void getFPGATrackSimRoads_2nd(std::vector<FPGATrackSimRoad*>& roads_2nd) { roads_2nd.reserve(m_FPGATrackSimRoads_2nd.size()); for ( auto& r : m_FPGATrackSimRoads_2nd) roads_2nd.push_back(&r); }
    size_t nFPGATrackSimRoads_2nd() const { return m_FPGATrackSimRoads_2nd.size(); }
    void reserveFPGATrackSimRoads_2nd(size_t size) { m_FPGATrackSimRoads_2nd.reserve(size); }
    void addFPGATrackSimRoads_2nd(std::vector<FPGATrackSimRoad*> const& roads_2nd) { for ( auto& r : roads_2nd) m_FPGATrackSimRoads_2nd.push_back(*r); }

    // First Stage FPGATrackSim Tracks
    std::vector<FPGATrackSimTrack> const& getFPGATrackSimTracks_1st() const { return m_FPGATrackSimTracks_1st; }
    size_t nFPGATrackSimTracks_1st() const { return m_FPGATrackSimTracks_1st.size(); }
    void reserveFPGATrackSimTracks_1st(size_t size) { m_FPGATrackSimTracks_1st.reserve(size); }
    void addFPGATrackSimTracks_1st(std::vector<FPGATrackSimTrack> const& tracks_1st) { m_FPGATrackSimTracks_1st = tracks_1st; }

    // Second Stage FPGATrackSim Tracks
    std::vector<FPGATrackSimTrack> const& getFPGATrackSimTracks_2nd() const { return m_FPGATrackSimTracks_2nd; }
    size_t nFPGATrackSimTracks_2nd() const { return m_FPGATrackSimTracks_2nd.size(); }
    void reserveFPGATrackSimTracks_2nd(size_t size) { m_FPGATrackSimTracks_2nd.reserve(size); }
    void addFPGATrackSimTracks_2nd(std::vector<FPGATrackSimTrack> const& tracks_2nd) { m_FPGATrackSimTracks_2nd = tracks_2nd; }

    // Data Flow Information
    FPGATrackSimDataFlowInfo const& getDataFlowInfo() const { return m_dataflowInfo; }
    void setDataFlowInfo(FPGATrackSimDataFlowInfo const& info) { m_dataflowInfo = info; }

private:

    std::vector<FPGATrackSimRoad>        m_FPGATrackSimRoads_1st;
    std::vector<FPGATrackSimRoad>        m_FPGATrackSimRoads_2nd;
    std::vector<FPGATrackSimTrack>       m_FPGATrackSimTracks_1st;
    std::vector<FPGATrackSimTrack>       m_FPGATrackSimTracks_2nd;

    FPGATrackSimDataFlowInfo             m_dataflowInfo;

    ClassDef(FPGATrackSimLogicalEventOutputHeader, 4);
};

std::ostream& operator<<(std::ostream& s, FPGATrackSimLogicalEventOutputHeader const& h);

#endif // FPGATrackSimEVENTOUTPUTHEADER_H
