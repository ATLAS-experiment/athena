/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITracccTritonTool_H
#define ITracccTritonTool_H

#include "GaudiKernel/IAlgTool.h"
#include "TrkSpacePoint/SpacePoint.h"
#include "TrkTrack/Track.h"
#include <cstdint>
#include <vector>

// model output
struct TracccTrackParameters {
    float chi2;
    float ndf;
    float l0;
    float l1;
    float phi;
    float theta;
    float qop;
    float time;
};
struct LocalMeasurementInfoInTracks {
    std::vector<float> local_x;
    std::vector<float> local_y;
    std::vector<float> phi;
    std::vector<float> theta;
    std::vector<float> qop;
    std::vector<float> time;
    std::vector<float> covariances; // flattened 6x6 covariance matrix per measurement
    std::vector<int64_t> athena_id;
};


/**
 * @class ITracccTritonTool
 * @brief Interface for tools that produce track candidates using traccc-based tracking
 * via inference with the TritonTool.
 */
class ITracccTritonTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(ITracccTritonTool, 1, 0);

    /**
     * @brief Get track candidates from serialized traccc cells.
     * @param cellBytes serialized traccc silicon_cell_collection (see
     *        TrackMaker::serializeCells for the byte layout), sent to the
     *        server as a single UINT8 tensor
     * @param TracccTrackParameters a vector of fitted track parameters
     * @param TracccMeasurementInfoInTracks a vector of measurements per fitted track
     * @return StatusCode indicating success or failure
     */

    virtual StatusCode getTracks(
        std::vector<uint8_t>& cellBytes,
        std::vector<TracccTrackParameters>& TracccTrackParameters,
        std::vector<LocalMeasurementInfoInTracks>& TracccMeasurementInfoInTracks
    ) const = 0;

};

#endif  // ITracccTritonTool_H
