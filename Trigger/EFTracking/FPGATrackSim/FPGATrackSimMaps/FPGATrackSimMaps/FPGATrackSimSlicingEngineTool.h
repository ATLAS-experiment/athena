/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  Implements the core logic of the "slicing engine", deciding whether hits fall
  within region boundaries and splitting them by stage as appropriate.
*/

#ifndef FPGATrackSimSLICINGENGINETOOL_H
#define FPGATrackSimSLICINGENGINETOOL_H

#include <array>
#include <vector>
#include <map>

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "IFPGATrackSimMappingSvc.h"

class TH1I;

class FPGATrackSimSlicingEngineTool : public AthAlgTool {
public:
    FPGATrackSimSlicingEngineTool(const std::string&, const std::string&, const IInterface*);
    virtual ~FPGATrackSimSlicingEngineTool()  = default;
    virtual StatusCode initialize() override;

    void readLayerMap();

    void sliceHits(const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits,
                   std::vector<std::shared_ptr<const FPGATrackSimHit>>& firstHits,
                   std::vector<std::shared_ptr<const FPGATrackSimHit>>& secondHits);

    // Helper function to hook up branches for output test vector creation.
    void setupSlices(FPGATrackSimLogicalEventInputHeader *slicedFirstPixelHeader,
                           FPGATrackSimLogicalEventInputHeader *slicedSecondPixelHeader,
                           FPGATrackSimLogicalEventInputHeader *slicedStripHeader) {
        m_slicedFirstPixelHeader = slicedFirstPixelHeader;
        m_slicedSecondPixelHeader = slicedSecondPixelHeader;
        m_slicedStripHeader = slicedStripHeader;
    };

private:

    // Properties
    ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", ""};

    Gaudi::Property<bool> m_rootOutput {this, "RootOutput", true, "Wite output ROOT branches"};
    Gaudi::Property<std::string> m_layerMap {this, "LayerMap", "", "Binning layer map file, only read if doSecondStage = true."};
    Gaudi::Property<bool> m_doSecondStage {this, "doSecondStage", true, "Split hits between first and second stage. If set to false all hits are first stage" };

    // Internal storage for the sliced hits (implemented as a LogicalEventInputHeader,
    // so we can easily copy to the output ROOT file).
    FPGATrackSimLogicalEventInputHeader* m_slicedFirstPixelHeader = nullptr;
    FPGATrackSimLogicalEventInputHeader* m_slicedSecondPixelHeader = nullptr;
    FPGATrackSimLogicalEventInputHeader* m_slicedStripHeader = nullptr;

    // Internal storage, these are the modules in the configured layer map that belong
    // to the first stage.
    std::set<unsigned> m_layerMapModules;

};

#endif // FPGATrackSimSLICINGENGINETOOL_H
