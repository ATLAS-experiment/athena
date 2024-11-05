/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_IMUONHITTIMINGTOOL_H
#define MUON_IMUONHITTIMINGTOOL_H

/**
    @class IMuonHitTimingTool

    Interface class for tools calculating the offset wrt the current bunch of a vector of MuonClusterOnTrack.
    These tools are used to identify out of time segments during the reconstruction.

    @author Niels.Van.Eldik@cern.ch
*/

#include <limits>
#include <vector>

#include "GaudiKernel/IAlgTool.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include <unordered_set>

namespace Muon {

    class MuonClusterOnTrack;

    /** Tool to calculate the timing wrt to the current BC
        Depending to the implementation the tool can handle different types of technologies BUT hits
        for different technolgies should NEVER be passes into a single call to the tool.
        If there are multiple technologies, uses will have to call the tool multiple times and combine the information
     */
    class IMuonHitTimingTool : virtual public IAlgTool {
    public:
        
        DeclareInterfaceID(Muon::IMuonHitTimingTool, 1, 0);
        /** simple struct holding the result of the tool */
        struct TimingResult {
            /** default constructor, constructs an invalid TimingResult */
            TimingResult() = default;
            /** full constructor */
            TimingResult(bool valid_, double time_, double error_) : valid(valid_), time(time_), error(error_) {}

            /** flag indicating whether the time was correctly calculated */
            bool valid{false};

            /** the actual time offset (ns) wrt the current bunch assuming the particle travelled in a straight line at the speed of light
             */
            float time{std::numeric_limits<float>::lowest()};

            /** uncertainty on the time (ns) */
            float error{std::numeric_limits<float>::lowest()};
        };

        /**Virtual destructor*/
        virtual ~IMuonHitTimingTool() = default;


        /** Calculate the time offset of a given set of hits wrt to the current bunch
            The hits SHOULD be of the same technology
         */
        virtual TimingResult calculateTimingResult(const std::vector<const MuonClusterOnTrack*>& hits) const = 0;

    };

}  // namespace Muon

#endif
