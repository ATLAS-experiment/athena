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
#include "xAODClusterMaker.h"
#include "TestVectorTool.h"
#include "FPGADataFormatTool.h"

// Athena include
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
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
        virtual StatusCode initialize() override final;
        virtual StatusCode execute(const EventContext &ctx) const override final;
        virtual StatusCode finalize() override final;
        StatusCode runPassThrough(std::vector<uint64_t> &pixelChainOutput, std::vector<uint64_t> &stripChainOutput, const EventContext &ctx) const;
        StatusCode runDataPrep(std::vector<uint64_t> &pixelChainOutput, std::vector<uint64_t> &stripChainOutput, const EventContext &ctx) const;

    private:
        ServiceHandle<IChronoSvc> m_chronoSvc{
            "ChronoStatSvc", name()}; //!< Service for timing the algorithm

        ToolHandle<xAODClusterMaker> m_xaodClusterMaker{
            this,
            "xAODClusterMaker",
            "xAODClusterMaker",
            "Tool for creating xAOD cluster containers"}; //!< Tool for creating xAOD containers

        ToolHandle<TestVectorTool> m_testVectorTool{
            this, "TestVectorTool", "TestVectorTool", "Tool for preparing test vectors"}; //!< Tool for preparing test vectors

        ToolHandle<FPGADataFormatTool> m_FPGADataFormatTool{
            this, "FPGADataFormatTool", "FPGADataFormatTool", "Tool for formatting FPGA data"}; //!< Tool for formatting FPGA data

        SG::ReadHandleKey<xAOD::PixelClusterContainer> m_inputPixelClusterKey{
            this, "InputPixelClusterKey", "ITkPixelClusters", "Key to access input pixel clusters"}; //!< Key to access input pixel clusters

        SG::ReadHandleKey<xAOD::StripClusterContainer> m_inputStripClusterKey{
            this, "InputStripClusterKey", "ITkStripClusters", "Key to access input strip clusters"}; //!< Key to access input strip clusters

        SG::ReadHandleKey<PixelRDO_Container> m_pixelRDOKey{this, "PixelRDO", "ITkPixelRDOs"};

        SG::ReadHandleKey<SCT_RDO_Container> m_stripRDOKey{this, "StripRDO", "ITkStripRDOs"};

        Gaudi::Property<std::string> m_xclbin{
            this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file

        Gaudi::Property<std::string> m_edmKernelName{
            this, "EDMPrepKernelName", "", "Name of the FPGA kernel"}; //!< Name of the FPGA kernel

        Gaudi::Property<std::string> m_pixelClusterKernelName{
            this, "PixelClusterKernelName", "", "Name of the pixel clustering kernel"}; //!< Name of the pixel clustering kernel

        Gaudi::Property<std::string> m_stripClusterKernelName{
            this, "StripClusterKernelName", "", "Name of the strip clustering kernel"}; //!< Name of the strip clustering kernel

        Gaudi::Property<std::string> m_pixelL2GKernelName{
            this, "PixelL2GKernelName", "", "Name of the pixel L2G kernel"}; //!< Name of the pixel L2G kernel

        Gaudi::Property<std::string> m_stripL2GKernelName{
            this, "StripL2GKernelName", "", "Name of the strip L2G kernel"}; //!< Name of the strip L2G kernelS

        Gaudi::Property<bool> m_runPassThrough{
            this, "runPassThrough", true, "Run the pass-through kernel"}; //!< Run the pass-through kernel

        mutable std::atomic<ulonglong> m_numEvents{0};          //!< Number of events processed
        mutable std::atomic<cl_ulong> m_pixelInputTime{0};      //!< Time for pixel input buffer write
        mutable std::atomic<cl_ulong> m_stripInputTime{0};      //!< Time for strip input buffer write
        mutable std::atomic<cl_ulong> m_pixelClusteringTime{0}; //!< Time for pixel clustering
        mutable std::atomic<cl_ulong> m_stripClusteringTime{0}; //!< Time for strip clustering
        mutable std::atomic<cl_ulong> m_pixelL2GTime{0};        //!< Time for pixel L2G
        mutable std::atomic<cl_ulong> m_stripL2GTime{0};        //!< Time for strip L2G
        mutable std::atomic<cl_ulong> m_edmPrepTime{0};         //!< Time for EDM preparation
        mutable std::atomic<cl_ulong> m_pixelOutputTime{0};     //!< Time for pixel output buffer read
        mutable std::atomic<cl_ulong> m_stripOutputTime{0};     //!< Time for strip output buffer read
        mutable std::atomic<cl_ulong> m_kernelTime{0};          //!< Time for kernel execution
    };
}

#endif // EFTRACKING_FPGA_INTEGRATION_BENCHMARKALG_H
