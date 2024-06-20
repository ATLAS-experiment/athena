// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimROADFILTERI_H
#define FPGATrackSimROADFILTERI_H

/**
 * @file IFPGATrackSimRoadFilterTool.h
 * @author Elliot Lipeles  lipeles@cern.ch
 * @date 03/25/21
 * @brief Interface declaration for road filter tools
 *
 * This class is implemented by
 *      - FPGATrackSimEtaPatternFilterTool
 *      - add other LRT ones
 */

#include "GaudiKernel/IAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"

#include <vector>


/**
 * A road filter returns a vector of roads given a vector of roads.
 *
 * Note that the postfilter_roads are owned by the tool, and are cleared at each successive
 * call of filterRoads().
 */

class IFPGATrackSimRoadFilterTool : virtual public IAlgTool
{
    public:
        DeclareInterfaceID(IFPGATrackSimRoadFilterTool, 1, 0);
        virtual StatusCode filterRoads(const std::vector<FPGATrackSimRoad*> & prefilter_roads, std::vector<FPGATrackSimRoad*> & postfilter_roads) = 0;
};


#endif
