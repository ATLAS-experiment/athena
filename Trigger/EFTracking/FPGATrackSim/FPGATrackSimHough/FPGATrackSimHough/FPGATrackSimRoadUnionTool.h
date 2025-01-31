// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimROADUNIONTOOL_H
#define FPGATrackSimROADUNIONTOOL_H

/**
 * @file FPGATrackSimRoadUnionTool.h
 * @author Riley Xu - riley.xu@cern.ch
 * @date November 20th, 2020
 * @brief Wrapper class to combine multiple road-finding tools
 *
 * Declarations in this file:
 *      class FPGATrackSimRoadUnionTool : public AthAlgTool, virtual public IFPGATrackSimRoadFinderTool
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimHough/IFPGATrackSimRoadFinderTool.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"


// This class is merely a lightweight wrapper around multiple road finder tools.
// It has evolved into representing the "slicing engine" in firmware and thus no longer
// inherits from IFPGATrackSimRoadFinderTool.
class FPGATrackSimRoadUnionTool : public AthAlgTool
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimRoadUnionTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // IFPGATrackSimRoadFinderTool

        StatusCode getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits,
                        std::vector<std::shared_ptr<const FPGATrackSimRoad>> &roads,
                        std::vector<FPGATrackSimTruthTrack> const &truthtracks)
        {
            m_truthtracks = truthtracks;
            return getRoads(hits, roads);
        }

        StatusCode getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads);

        std::vector<FPGATrackSimTruthTrack> const *getTruthTracks() { return &m_truthtracks; }

        ///////////////////////////////////////////////////////////////////////
        // FPGATrackSimRoadUnionTool

        ToolHandleArray<IFPGATrackSimRoadFinderTool> const & tools() const { return m_tools; }
        void setupSlices(FPGATrackSimLogicalEventInputHeader *slicedHitHeader) {
          m_slicedHitHeader = slicedHitHeader;
        }

        ///////////////////////////////////////////////////////////////////////
        // Properties
        Gaudi::Property<bool> m_noHitFilter{this, "noHitFilter", {false}, "Disable filtering of hits"};
    

    private:

        ///////////////////////////////////////////////////////////////////////
        // Handles
        
        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};

        ToolHandleArray<IFPGATrackSimRoadFinderTool> m_tools;

        // Internal storage for the sliced hits (implemented as a LogicalEventInputHeader,
        // so we can easily copy to the output ROOT file).
        FPGATrackSimLogicalEventInputHeader*  m_slicedHitHeader = nullptr;
        std::vector<FPGATrackSimTruthTrack> m_truthtracks;
};

#endif
