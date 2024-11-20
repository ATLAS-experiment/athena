/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/DataPreparationPipeline.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Feb. 18, 2024
 * @brief Class for the data preparation pipeline
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H
#define EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H

// EFTracking include
#include "EFTrackingDataFormats.h"
#include "IntegrationBase.h"
#include "xAODContainerMaker.h"
#include "PassThroughTool.h"

// Athena include
#include "StoreGate/ReadHandleKey.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

/**
 * @brief This is the class for the data preparation pipeline.
 *
 * The output of this pipeline are the xAOD::StripCluster(Container) and
 * xAOD::PixelCluster(Container). These containers are to be used in other
 * CPU-based algorithms.
 */
class DataPreparationPipeline : public IntegrationBase
{
public:
    using IntegrationBase::IntegrationBase;
    StatusCode initialize() override final;
    StatusCode execute(const EventContext &ctx) const override final;

private:
    Gaudi::Property<std::string> m_xclbin{
        this, "xclbin", "",
        "xclbin path and name"}; //!< Path and name of the xclbin file
    Gaudi::Property<std::string> m_kernelName{this, "KernelName", "",
                                              "Kernel name"}; //!< Kernel name
    Gaudi::Property<bool> m_usePassThrough{
        this, "RunPassThrough", false,
        "Use the passthrough tool instead of data prep pipeline"}; //!< Use the pass through tool instead of data prep pipeline. It can be either sw or hw, setting in the tool

    // Conver kernel outputs into xAOD containers
    ToolHandle<xAODContainerMaker> m_xAODContainerMaker{
        this, "xAODMaker", "xAODContainerMaker",
        "tool to make cluster"}; //!< Tool handle for xAODContainerMaker
    ToolHandle<PassThroughTool> m_passThroughTool{
        this, "PassThroughTool", "PassThroughTool",
        "The pass through tool"}; //!< Tool handle for PassThroughTool
};

#endif // EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H
