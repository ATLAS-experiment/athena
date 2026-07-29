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
     * @brief Get track candidates from a list of space points.
     * @param spacepoints a list of spacepoints as inputs to the GNN-based track
     * finder.
     * @param tracks a list of fitted tracks
     *
     * @return
     */
    virtual StatusCode getTracks(
        std::vector<TracccCell>& cells,
        std::vector<TracccTrackParameters>& TracccTrackParameters,
        std::vector<LocalMeasurementInfoInTracks>& TracccMeasurementsInfoInTracks) const override;


private:
    ToolHandle<AthInfer::IAthInferenceTool> m_TracccTritonTool{
        this, "TritonTool", "AthInfer::TritonTool"};

    Gaudi::Property<bool> m_saveEventsToCSV{
        this, "SaveEventsToCSV", false,
        "Whether to save input/output of each event to CSV files for debugging"};

    Gaudi::Property<int> m_maxEventsToSave{
        this, "MaxEventsToSave", 10,
        "Maximum number of events to save to CSV files if SaveEventsToCSV is true"};

    mutable std::atomic<int> m_eventCounter{0};

    // TODO: add variable to specify which pipeline to run
};

#endif  // TracccTritonTool_H
