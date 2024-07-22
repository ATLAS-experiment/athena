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


// This class is merely a lightweight wrapper around multiple road finder tools.
class FPGATrackSimRoadUnionTool : public extends <AthAlgTool, IFPGATrackSimRoadFinderTool>
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimRoadUnionTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // IFPGATrackSimRoadFinderTool

        virtual StatusCode getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<FPGATrackSimRoad*> & roads) override;

        ///////////////////////////////////////////////////////////////////////
        // FPGATrackSimRoadUnionTool

        ToolHandleArray<IFPGATrackSimRoadFinderTool> const & tools() const { return m_tools; }

    private:

        ///////////////////////////////////////////////////////////////////////
        // Handles

        ToolHandleArray<IFPGATrackSimRoadFinderTool> m_tools;
};

#endif
