/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITracccTritonTool_H
#define ITracccTritonTool_H

#include "GaudiKernel/IAlgTool.h"
#include <cstdint>
#include <vector>

/// The (serialized) traccc EDM collections returned by the server
struct TracccTritonResult {
    std::vector<uint8_t> measurements;
    std::vector<uint8_t> clusters;
    std::vector<uint8_t> tracks;
    std::vector<uint8_t> trackStates;
};


/**
 * @class ITracccTritonTool
 * @brief Interface for tools that run traccc-based tracking via inference
 * with the TritonTool.
 */
class ITracccTritonTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(ITracccTritonTool, 1, 0);

    /**
     * @brief Reconstruct the tracks of one event.
     * @param cellBytes serialized traccc silicon_cell_collection, sent to the
     *        server as a single UINT8 tensor
     * @param result the serialized collections the server returns
     * @return StatusCode indicating success or failure
     */
    virtual StatusCode getTracks(
        std::vector<uint8_t>& cellBytes,
        TracccTritonResult& result
    ) const = 0;

};

#endif  // ITracccTritonTool_H
