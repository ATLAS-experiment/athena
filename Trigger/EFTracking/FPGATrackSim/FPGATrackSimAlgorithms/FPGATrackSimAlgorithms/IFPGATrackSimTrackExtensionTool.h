// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimTRACKEXTENSIONI_H
#define FPGATrackSimTRACKEXTENSIONI_H

/**
 * @file IFPGATrackSimTrackExtensionTool.h
 * @author Ben Rosser - brosser@uchicago.edu
 * @date 2024/10/08
 * @brief Interface declaration for track extension tools (tracks + hits -> roads)
 *
 * This class is implemented by
 *      - FPGATrackSimWindowExtensionTool
 *      - and possibly others (NN "pathfinder") in the future.
 */

#include "GaudiKernel/IAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"

#include <vector>

/**
 * A track extension tool takes tracks and hits, and returns a NEW set of roads.
 *
 * Note that the roads are owned by the tool, and are cleared at each successive
 * call of getRoads().
 *
 */


class IFPGATrackSimTrackExtensionTool : virtual public IAlgTool
{
    public:
        DeclareInterfaceID(IFPGATrackSimTrackExtensionTool, 1, 0);
        
        virtual StatusCode extendTracks(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits,
                                        const std::vector<std::shared_ptr<const FPGATrackSimTrack>> & tracks,
                                        std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads) = 0;
};


#endif
