/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TracccTritonTool_H
#define TracccTritonTool_H

// System include(s).
#include <iostream>
#include <list>
#include <memory>

#include "AthOnnxInterfaces/IAthInferenceTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "ITracccTritonTool.h"

/**
 * @class TracccTritonTool
 * @brief TracccTritonTool is a tool that produces track
 * candidates with graph neural networks-based pipeline using space points as
 * inputs.
 * @author xiangyang.ju@cern.ch
 */

class TracccTritonTool : public extends<AthAlgTool, ITracccTritonTool> {
public:
    TracccTritonTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual StatusCode initialize() override;

    ///////////////////////////////////////////////////////////////////
    // Main methods for remote track finding asked by the ITracccTritonTool
    ///////////////////////////////////////////////////////////////////

    /**
     * @brief Get track candidates from serialized traccc cells.
     * @param cellBytes serialized traccc silicon_cell_collection, forwarded to
     * the server as a single UINT8 tensor.
     * @param TracccTrackParameters a list of fitted track parameters
     * @param TracccMeasurementsInfoInTracks measurements per fitted track
     *
     * @return
     */
    virtual StatusCode getTracks(
        std::vector<uint8_t>& cellBytes,
        std::vector<TracccTrackParameters>& TracccTrackParameters,
        std::vector<LocalMeasurementInfoInTracks>& TracccMeasurementsInfoInTracks) const override;


private:
    ToolHandle<AthInfer::IAthInferenceTool> m_TracccTritonTool{
        this, "TritonTool", "AthInfer::TritonTool"};

};

#endif  // TracccTritonTool_H
