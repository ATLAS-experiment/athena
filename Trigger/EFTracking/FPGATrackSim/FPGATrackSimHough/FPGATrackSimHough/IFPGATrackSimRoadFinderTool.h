// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimROADFINDERI_H
#define FPGATrackSimROADFINDERI_H

/**
 * @file IFPGATrackSimRoadFinderTool.h
 * @author Riley Xu - rixu@cern.ch
 * @date 10/23/19
 * @brief Interface declaration for road finder tools
 *
 * This class is implemented by
 *      - FPGATrackSimRoadUnionTool
 *      - FPGATrackSimPatternMatchTool
 *      - FPGATrackSimSectorMatchTool
 *      - FPGATrackSimHoughTransformTool
 *      - FPGATrackSimHough1DShiftTool
 *      - and other LRT ones
 */

#include "GaudiKernel/IAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"

#include <vector>

class FPGATrackSimHit;


/**
 * A road finder returns a vector of roads given a vector of hits.
 *
 * Note that the roads are owned by the tool, and are cleared at each successive
 * call of getRoads().
 */


class IFPGATrackSimRoadFinderTool : virtual public IAlgTool
{
    public:
        DeclareInterfaceID(IFPGATrackSimRoadFinderTool, 1, 0);
        virtual StatusCode getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<FPGATrackSimRoad*> & roads) = 0;
};


#endif
