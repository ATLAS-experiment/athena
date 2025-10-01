/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/F1X0XRTIntegrationAlg.h
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_F1X0XRTIntegrationAlg_H
#define EFTRACKING_FPGA_INTEGRATION_F1X0XRTIntegrationAlg_H

// EFTracking include
#include "EFTrackingFPGAPipeline/IntegrationBase.h"
#include "EFTrackingFPGAUtility/xAODClusterMaker.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"

// Athena include
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoSvc.h"
#include <TrigSteeringEvent/TrigRoiDescriptorCollection.h>
#include <IRegionSelector/IRegSelTool.h>

// XRT native C++ API
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_uuid.h>

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

namespace EFTrackingFPGAIntegration
{
    /**
     * @brief Benchmark algorithm for FPGA integration and output conversion.
     *
     * Uses native XRT API (no OpenCL).
     */
    class F1X0XRTIntegrationAlg : public IntegrationBase
    {
    public:
        using IntegrationBase::IntegrationBase;
        virtual StatusCode initialize() override final;
        virtual StatusCode execute(const EventContext &ctx) const override final;
        virtual StatusCode finalize() override final;

    private:
        ServiceHandle<IChronoSvc> m_chronoSvc{"ChronoStatSvc", name()}; //!< Service for timing the algorithm

        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAPixelRDO{this, "FPGAEncodedPixelKey", "FPGAEncodedPixelRDOs", "Pixel RDO converted to FPGA format"};
        SG::ReadHandleKey<std::vector<uint64_t>> m_FPGAStripRDO{this, "FPGAEncodedStripKey", "FPGAEncodedStripRDOs", "Strip RDO converted to FPGA format"};

        SG::WriteHandleKey<std::vector<uint64_t>> m_FPGAPixelOutput{this, "FPGAOutputPixelKey", "FPGAPixelOutput", "Pixel output from FPGA format"};
        SG::WriteHandleKey<std::vector<uint64_t>> m_FPGAStripOutput{this, "FPGAOutputStripKey", "FPGAStripOutput", "Strip output from FPGA format"};

        Gaudi::Property<int> m_FPGAThreads{this, "FPGAThreads", 1, "number of FPGA threads to initialize"};

        Gaudi::Property<std::string> m_xclbin{
            this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file

        Gaudi::Property<bool> m_doF110{
            this, "doF110", false, "Run F110 instead of F100"}; //!< Boolean to run F110 instead of F100

        Gaudi::Property<std::string> m_pixelEdmKernelName{
            this, "PixelEDMPrepKernelName", "", "Name of the Pixel EDM kernel"};

        Gaudi::Property<std::string> m_stripEdmKernelName{
            this, "StripEDMPrepKernelName", "", "Name of the Strip EDM kernel"};

        Gaudi::Property<std::string> m_pixelClusterKernelName{
            this, "PixelClusterKernelName", "", "Name of the pixel clustering kernel"};

        Gaudi::Property<std::string> m_stripClusterKernelName{
            this, "StripClusterKernelName", "", "Name of the strip clustering kernel"};

        Gaudi::Property<std::string> m_pixelL2GKernelName{
            this, "PixelL2GKernelName", "", "Name of the pixel L2G kernel"};

        Gaudi::Property<std::string> m_stripL2GKernelName{
            this, "StripL2GKernelName", "", "Name of the strip L2G kernel"};

        // Counters / timing (nanoseconds)
        mutable std::atomic<uint64_t> m_numEvents{0};           //!< Number of events processed
        mutable std::atomic<uint64_t> m_pixelInputTime{0};      //!< Time for pixel input buffer write
        mutable std::atomic<uint64_t> m_stripInputTime{0};      //!< Time for strip input buffer write
        mutable std::atomic<uint64_t> m_pixelClusteringTime{0}; //!< Time for pixel clustering
        mutable std::atomic<uint64_t> m_stripClusteringTime{0}; //!< Time for strip clustering
        mutable std::atomic<uint64_t> m_pixelL2GTime{0};        //!< Time for pixel L2G (F100)
        mutable std::atomic<uint64_t> m_stripL2GTime{0};        //!< Time for strip L2G
        mutable std::atomic<uint64_t> m_edmPrepTime{0};         //!< Time for EDM preparation (F100)
        mutable std::atomic<uint64_t> m_pixelEdmPrepTime{0};    //!< Time for pixel EDM preparation (F110)
        mutable std::atomic<uint64_t> m_stripEdmPrepTime{0};    //!< Time for strip EDM preparation (F110)
        mutable std::atomic<uint64_t> m_pixelOutputTime{0};     //!< Time for pixel output buffer read
        mutable std::atomic<uint64_t> m_stripOutputTime{0};     //!< Time for strip output buffer read
        mutable std::atomic<uint64_t> m_kernelTime{0};          //!< Time window covering kernel execution

        // XRT device / xclbin
        xrt::device m_xrtDevice;
        xrt::uuid   m_xrtUuid;

        // Kernels (XRT)
        // Clustering
        mutable std::vector<xrt::kernel> m_pixelClusteringKernels ATLAS_THREAD_SAFE;
        mutable std::vector<xrt::kernel> m_stripClusteringKernels ATLAS_THREAD_SAFE;

        // L2G
        mutable std::vector<xrt::kernel> m_pixelL2GKernels ATLAS_THREAD_SAFE; // F100
        mutable std::vector<xrt::kernel> m_stripL2GKernels ATLAS_THREAD_SAFE;

        // EDM prep
        mutable std::vector<xrt::kernel> m_pixelEdmPrepKernels ATLAS_THREAD_SAFE;
        mutable std::vector<xrt::kernel> m_stripEdmPrepKernels ATLAS_THREAD_SAFE;

        // Buffers (XRT BOs) — must be mutable for execute() const
        mutable std::vector<xrt::bo> m_pixelClusterInputBOList ATLAS_THREAD_SAFE;
        mutable std::vector<xrt::bo> m_stripClusterInputBOList ATLAS_THREAD_SAFE;

        mutable std::vector<xrt::bo> m_pixelClusterOutputBOList ATLAS_THREAD_SAFE;     // (F100 only)
        mutable std::vector<xrt::bo> m_stripClusterOutputBOList ATLAS_THREAD_SAFE;
        mutable std::vector<xrt::bo> m_pixelClusterEDMOutputBOList ATLAS_THREAD_SAFE;
        mutable std::vector<xrt::bo> m_stripClusterEDMOutputBOList ATLAS_THREAD_SAFE;

        mutable std::vector<xrt::bo> m_pixelL2GOutputBOList ATLAS_THREAD_SAFE;         // (F100 only)
        mutable std::vector<xrt::bo> m_stripL2GOutputBOList ATLAS_THREAD_SAFE;
        mutable std::vector<xrt::bo> m_pixelL2GEDMOutputBOList ATLAS_THREAD_SAFE;      // (F100 only)
        mutable std::vector<xrt::bo> m_stripL2GEDMOutputBOList ATLAS_THREAD_SAFE;

        mutable std::vector<xrt::bo> m_edmPixelOutputBOList ATLAS_THREAD_SAFE;
        mutable std::vector<xrt::bo> m_edmStripOutputBOList ATLAS_THREAD_SAFE;

        void getListofCUs(std::vector<std::string>& cuNames);
    };
}

#endif // EFTRACKING_FPGA_INTEGRATION_F1X0XRTIntegrationAlg_H
