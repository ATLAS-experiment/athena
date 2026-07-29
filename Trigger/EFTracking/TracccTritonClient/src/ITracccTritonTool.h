/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITracccTritonTool_H
#define ITracccTritonTool_H

#include "GaudiKernel/IAlgTool.h"
#include "TrkSpacePoint/SpacePoint.h"
#include "TrkTrack/Track.h"
#include <vector>

class MsgStream;

// model input
struct TracccCell {

    int64_t geometry_id = 0;
    int64_t measurement_id = 0;
    int64_t channel0 = 0;
    int64_t channel1 = 0;
    float timestamp = 0.f;
    float value = 0.f;

};

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
    std::vector<float> covariances; // flattened 5x5 covariance matrix per measurement
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
     * @brief Get track candidates from hits.
     * @param cells a vector of hits per cell
     * @param TracccTrackParameters a vector of fitted track parameters
     * @param TracccMeasurementInfoInTracks a vector of measurements per fitted track
     * @return StatusCode indicating success or failure
     */

    virtual StatusCode getTracks(
        std::vector<TracccCell>& cells,
        std::vector<TracccTrackParameters>& TracccTrackParameters,
        std::vector<LocalMeasurementInfoInTracks>& TracccMeasurementInfoInTracks
    ) const = 0;

    // TODO: Add pipeline for combined GNN+traccc (should be nearly identical to getTracks)
};

#endif  // ITracccTritonTool_H
