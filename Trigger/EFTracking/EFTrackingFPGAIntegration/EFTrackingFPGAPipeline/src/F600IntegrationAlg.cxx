/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EFTrackingFPGAPipeline/F600IntegrationAlg.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"
#include "AthenaKernel/Chrono.h"
#include "EFTrackingFPGAPipeline/DataPreparationPipeline.h"

namespace EFTrackingFPGAIntegration
{
    StatusCode F600IntegrationAlg::initialize()
    {
        ATH_MSG_INFO("Running on the FPGA accelerator");

        // ATH_CHECK(IntegrationBase::precheck({m_xclbin}));

        ATH_CHECK(m_chronoSvc.retrieve());

        {
            Athena::Chrono chrono("Platform and device initlize", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::initialize());
        }

        {
            Athena::Chrono chrono("CL::loadProgram", m_chronoSvc.get());
            ATH_CHECK(IntegrationBase::loadProgram(m_xclbin));
        }

        ATH_CHECK(m_xaodClusterMaker.retrieve());
        ATH_CHECK(m_testVectorTool.retrieve());
        ATH_CHECK(m_FPGADataFormatTool.retrieve());
        ATH_CHECK(m_FPGATrackKey.initialize());
        ATH_CHECK(m_outputConversionTool.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode F600IntegrationAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F600IntegrationAlg");
	    std::vector<uint64_t> encodedData;
        auto FPGATrackHandle = SG::makeHandle(m_FPGATrackKey, ctx);
        ATH_CHECK(m_FPGADataFormatTool->convertFPGATracksToFPGADataFormat(&(*FPGATrackHandle), encodedData, ctx));

        std::unique_ptr<EFTrackingTransient::Metadata> metadata =
        std::make_unique<EFTrackingTransient::Metadata>();
          
	ATH_CHECK(m_outputConversionTool->decodeFPGAoutput(encodedData, metadata.get(), nullptr, nullptr, OutputConversion::FSM::GTracks));       

        return StatusCode::SUCCESS;
    }
}
