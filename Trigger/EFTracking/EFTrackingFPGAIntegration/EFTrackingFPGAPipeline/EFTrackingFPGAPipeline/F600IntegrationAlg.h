/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


 #ifndef EFTRACKING_FPGA_INTEGRATION_F600INTEGRATIONALG_H
 #define EFTRACKING_FPGA_INTEGRATION_F600INTEGRATIONALG_H
 
 // EFTracking include
 #include "EFTrackingFPGAPipeline/IntegrationBase.h"
 #include "EFTrackingFPGAUtility/xAODClusterMaker.h"
 #include "EFTrackingFPGAUtility/TestVectorTool.h"
 #include "EFTrackingFPGAUtility/FPGADataFormatTool.h"
 #include "EFTrackingFPGAUtility/OutputConversionTool.h"
 #include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"
 #include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"

 // Athena include
 #include "GaudiKernel/ServiceHandle.h"
 #include "GaudiKernel/IChronoSvc.h"
 
 namespace EFTrackingFPGAIntegration
 {
     class F600IntegrationAlg : public IntegrationBase
     {
     public:
         using IntegrationBase::IntegrationBase;
         virtual StatusCode initialize() override final;
         virtual StatusCode execute(const EventContext &ctx) const override final;
         virtual StatusCode finalize() override final;
        
                
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
 
         Gaudi::Property<std::string> m_xclbin{
             this, "xclbin", "", "xclbin path and name"}; //!< Path and name of the xclbin file
 
        SG::ReadHandleKey<FPGATrackSimTrackCollection> m_FPGATrackKey{this, "FPGATrackSimTrack1stKey","FPGATracks_1st_reg34","FPGATrackSim Tracks 1st stage key"};

        // Tool for output conversion
        ToolHandle<OutputConversionTool> m_outputConversionTool{this, "OutputConversionTool", "OutputConversionTool", "tool for output conversion"};

       mutable std::atomic<cl_ulong> m_kernelTime{0};       //!< Time for kernel execution
       mutable std::atomic<cl_ulong> m_sum_kernelTime{0};  //!< Sum for the average time of the kernel execution
       mutable std::atomic<ulonglong> m_num_Events{0}; //!< Number of events for the average time of the kernel execution

     };
 }
 
 #endif // EFTRACKING_FPGA_INTEGRATION_F600INTEGRATIONALG_H
 
