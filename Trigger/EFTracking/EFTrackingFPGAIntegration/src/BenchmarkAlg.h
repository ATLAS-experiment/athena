/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/BenchmarkAlg.h
 * @author zhaoyuan.cui@cern.ch
 * @date Feb. 25, 2025
 * @brief Class for the benchmark algorithm specific to the FPGA integration and output conversion
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_BENCHMARKALG_H
#define EFTRACKING_FPGA_INTEGRATION_BENCHMARKALG_H

// EFTracking include
#include "IntegrationBase.h"
#include "xAODContainerMaker.h"
#include "TestVectorTool.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoSvc.h"

namespace EFTrackingFPGAIntegration
{
    /**
     * @brief This is the class for the benchmark algorithm specific to the FPGA integration and output conversion.
     *
     * This algorithm is used to benchmark and optimize the FPGA output memory migration
     * and output conversion. It expects the use of FPGA pass-through kernel.
     */
    class BenchmarkAlg : public IntegrationBase
    {
    public:
        using IntegrationBase::IntegrationBase;
        StatusCode initialize() override final;
        StatusCode execute(const EventContext &ctx) const override final;
        StatusCode setupBuffers();
        StatusCode setupKernelArgs();

    private:
        // We need kernel and buffer objects

        cl::Kernel m_kernel;       //!< FPGA pass-through kernel
        cl::Buffer m_inPixelBuff;  //!< Input pixel tv buffer
        cl::Buffer m_inStripBuff;  //!< Input strip tv buffer
        cl::Buffer m_outPixelBuff; //!< Output pixel edm buffer
        cl::Buffer m_outStripBuff; //!< Output strip edm buffer

        ServiceHandle<IChronoSvc> m_chronoSvc{
            "ChronoStatSvc", name()}; //!< Service for timing the algorithm

        ToolHandle<xAODContainerMaker> m_xaodContainerMaker{
            this,
            "xAODContainerMaker",
            "xAODContainerMaker",
            "Tool for creating xAOD containers"}; //!< Tool for creating xAOD containers

        ToolHandle<TestVectorTool> m_testVectorTool{
            this, "TestVectorTool", "TestVectorTool", "Tool for preparing test vectors"}; //!< Tool for preparing test vectors

        SG::ReadHandleKey<xAOD::PixelClusterContainer> m_inputPixelClusterKey{
            this, "InputPixelClusterKey", "ITkPixelClusters", "Key to access input pixel clusters"}; //!< Key to access input pixel clusters

        SG::ReadHandleKey<xAOD::StripClusterContainer> m_inputStripClusterKey{
            this, "InputStripClusterKey", "ITkStripClusters", "Key to access input strip clusters"}; //!< Key to access input strip clusters

        Gaudi::Property<std::string> m_xclbin{
            this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file

        Gaudi::Property<std::string> m_kernelName{
            this, "kernelName", "", "Name of the FPGA kernel"}; //!< Name of the FPGA kernel

        Gaudi::Property<std::string> m_pixelClusterTVPath{
            this, "pixelClusterTVPath", "", "Path to the pixel clustering test vector"}; //!< Path to the pixel clustering test vector

        Gaudi::Property<std::string> m_stripClusterTVPath{
            this, "stripClusterTVPath", "", "Path to the strip clustering test vector"}; //!< Path to the strip clustering test vector
    };
}

#endif // EFTRACKING_FPGA_INTEGRATION_BENCHMARKALG_H
