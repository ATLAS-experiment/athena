/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimSPACEPOINTSTOOL_H
#define FPGATrackSimSPACEPOINTSTOOL_H

#include <array>
#include <vector>
#include <map>

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimMaps/FPGATrackSimSpacePointsToolI.h"

class TH1I;

class FPGATrackSimSpacePointsTool : public extends<AthAlgTool, FPGATrackSimSpacePointsToolI> {
 public:
    FPGATrackSimSpacePointsTool(const std::string &, const std::string &, const IInterface *);
    virtual ~FPGATrackSimSpacePointsTool() {}

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    virtual StatusCode DoSpacePoints(FPGATrackSimLogicalEventInputHeader &, std::vector<FPGATrackSimCluster> &) override;

 private:
    StatusCode fillMaps(std::vector<FPGATrackSimHit>& hits);
    StatusCode makeSpacePoints(FPGATrackSimTowerInputHeader &tower, std::vector<FPGATrackSimCluster> &spacepoints);
    void calcPosition(FPGATrackSimHit &hit_in, FPGATrackSimHit &hit_out, float &x, float &y, float &z);
    bool searchForMatch(FPGATrackSimHit& hit_in,std::vector<FPGATrackSimHit>& hits_outer,FPGATrackSimTowerInputHeader &tower, std::vector<FPGATrackSimCluster> &spacepoints);
    void addSpacePoints(FPGATrackSimHit hit_in, FPGATrackSimHit hit_out ,FPGATrackSimTowerInputHeader &tower, std::vector<FPGATrackSimCluster> &spacepoints);

    //----------------------
    // Working Memory
    std::map<std::vector<int>,std::pair<std::vector<FPGATrackSimHit>,std::vector<FPGATrackSimHit>>> m_map;
    std::vector<FPGATrackSimHit> m_pixel;

    Gaudi::Property<float> m_phiwindow { this, "PhiWindow", 0.008, "Distance in phi to consider two hits for making a space-point" };
    Gaudi::Property<bool> m_duplicate { this, "Duplication", false, "Duplicate spacepoints to layer on the other side of the stave" };
    Gaudi::Property<bool> m_filter { this, "Filtering", false, "Filter out incomplete spacepoints" };
    Gaudi::Property<bool> m_filterClose { this, "FilteringClosePoints", false, "Filter out single hits cloes to spacepoints" };
    Gaudi::Property<bool> m_sameModulesOnly { this, "SameModulesOnly", false, "Only use hits on the same physical (eta) module" };
    Gaudi::Property<bool> m_boundsCheck { this, "BoundsCheck", false, "Perform a module bounds check in the endcaps" };

    // self monitoring
    unsigned m_inputhits = 0;
    unsigned m_spacepts = 0;
    unsigned m_filteredhits = 0;
    unsigned m_adjacent_eta_sp = 0;
    unsigned m_adjacent_phi_sp = 0;
    unsigned m_diagonal_sp = 0;
    TH1I*    m_spacepts_per_hit;

};

#endif // FPGATrackSimSPACEPOINTSTOOL_H
