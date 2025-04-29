/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimHITFILTERINGTOOL_H
#define FPGATrackSimHITFILTERINGTOOL_H

/*
 * httHitFiltering
 * ---------------
 *
 * Routines to filter hits/clusters, based on FPGATrackSim
 *
 */

#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"

#include "FPGATrackSimMaps/IFPGATrackSimHitFilteringTool.h"

#include "TRandom3.h"
#include <unordered_map>


class IFPGATrackSimMappingSvc;

class FPGATrackSimHitFilteringTool : public extends<AthAlgTool, IFPGATrackSimHitFilteringTool> {
 public:
    FPGATrackSimHitFilteringTool(const std::string &, const std::string &, const IInterface *);

    virtual ~FPGATrackSimHitFilteringTool() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode DoRandomRemoval(FPGATrackSimLogicalEventInputHeader &, bool) override;
    virtual StatusCode GetPairedStripPhysLayers(const FPGATrackSimPlaneMap*, std::vector<int> &) override;
    virtual StatusCode DoHitFiltering(FPGATrackSimLogicalEventInputHeader &,
                                      std::vector<int>, std::vector<int>,
                                      std::vector<FPGATrackSimCluster> &) override;

 private:
    void FilterHits(std::vector<FPGATrackSimHit> &, std::vector<int> &, std::vector<int> &, std::vector<FPGATrackSimHit> &);

    bool check_hit_stub(const FPGATrackSimHit&, const FPGATrackSimHit&, float, float);
    void fill_cut_values(const FPGATrackSimHit&, float &, float &);

    // configuration
    Gaudi::Property<bool> m_digitalClustering {this, "DigitalClustering", true, "flag to enable digital clustering instead of ToT weighted position calculation" };
    Gaudi::Property<bool> m_doRandomRemoval {this, "doRandomRemoval", false, "remove hits/clusters at random"};
    Gaudi::Property<float> m_rndPixelHitRmFrac {this, "pixelHitRmFrac", 0.0, "fraction of pixel hits to randomly remove"};
    Gaudi::Property<float> m_rndStripHitRmFrac {this, "stripHitRmFrac", 0.0, "fraction of strip hits to randomly remove"};
    Gaudi::Property<float> m_rndPixelClustRmFrac {this, "pixelClusRmFrac", 0.0, "fraction of pixel clusters to randomly remove"};
    Gaudi::Property<float> m_rndStripClustRmFrac {this, "stripClusRmFrac", 0.0, "fraction of strip clusters to randomly remove"};
    Gaudi::Property<bool> m_doStubs {this, "doStubs", false, "flag to enable dphi stub hit filtering"};
    Gaudi::Property<std::string> m_stubCutsFile {this, "stubsCutsFile", "", "stubs cuts file"};
    Gaudi::Property<float> m_barrelStubDphiCut {this, "barrelStubDphiCut", 0.0, "barrel stub dPhi cut. Overridden by stubCutsFile"};
    Gaudi::Property<float> m_endcapStubDphiCut {this, "endcapStubDphiCut", 0.0, "endcap stub dPhi cut. Overridden by stubCutsFile"};
    Gaudi::Property<bool> m_useNstrips {this, "useNstrips", false, "use nStrips instead of dPhi for stub filtering"};


    TRandom3 m_random;
    std::unordered_map<std::string, std::unordered_map<int, std::unordered_map<int, std::pair<float,float>>>> m_stubCutMap;

};

#endif // FPGATrackSimHITFILTERINGTOOL_H
