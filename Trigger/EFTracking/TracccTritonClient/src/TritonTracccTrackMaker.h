/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TritonTracccTrackMaker_H
#define TritonTracccTrackMaker_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "GaudiKernel/ToolHandle.h"

#include "AthDeviceInterfaces/IMemoryResourceTool.h"
#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "ActsGPUEvent/TracccSiliconClusterCollection.h"
#include "ActsGPUEvent/TracccTrackContainer.h"

// Tool handles
#include "ITracccTritonTool.h"

/**
 * @class TritonTracccTrackMaker
 *
 * @brief TritonTracccTrackMaker sends the traccc cells of an event to a
 * Triton server running the traccc chain, and records the traccc
 * measurements, clusters and tracks the server returns in StoreGate as host
 * buffers. From there on the event is converted exactly as in the local
 * device chain, by TracccMeasurementConverterAlg and TracccTrackConverterAlg.
 *
 * @author miles.cb@cern.ch
 */
class TritonTracccTrackMaker : public AthReentrantAlgorithm {

public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    SG::ReadHandleKey<traccc::edm::silicon_cell_collection::const_view>
        m_tracccCellsKey{
            this, "TracccCells", "TracccCells",
            "Input traccc cell collection (from RDOtoTracccCellConverterAlg)"};

    SG::WriteHandleKey<traccc::edm::measurement_collection::buffer>
        m_measurementsKey{
            this, "OutputTracccMeasurements", "TracccTritonMeasurements",
            "Output traccc measurement collection returned by the server"};

    SG::WriteHandleKey<traccc::edm::silicon_cluster_collection::buffer>
        m_clustersKey{
            this, "OutputTracccClusters", "TracccTritonClusters",
            "Output traccc cluster collection returned by the server"};

    SG::WriteHandleKey<traccc_track_container::buffer> m_tracksKey{
        this, "OutputTracccTracks", "TracccTritonTracks",
        "Output traccc track container returned by the server"};

    ToolHandle<AthDevice::IMemoryResourceTool> m_hostMR{
        this, "HostMR", "", "Host memory resource tool for the output buffers"};

    ToolHandle<ITracccTritonTool> m_tracccTrackingTool{
        this, "TracccTritonTool", "TracccTritonTool"};

}; // TritonTracccTrackMaker

#endif
