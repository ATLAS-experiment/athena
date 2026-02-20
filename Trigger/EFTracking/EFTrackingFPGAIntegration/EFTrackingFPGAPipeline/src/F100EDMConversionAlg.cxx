/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */

#include "EFTrackingFPGAPipeline/F100EDMConversionAlg.h"
#include "AthenaKernel/Chrono.h"

namespace EFTrackingFPGAIntegration
{
    StatusCode F100EDMConversionAlg::initialize()
    {
        ATH_CHECK(m_xaodClusterMaker.retrieve());

        ATH_CHECK(m_FPGAPixelOutput.initialize());
        ATH_CHECK(m_FPGAStripOutput.initialize());


        return StatusCode::SUCCESS;
    }

    StatusCode F100EDMConversionAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F100EDMConversionAlg");


        auto pixelOutput = SG::get(m_FPGAPixelOutput, ctx);
        auto stripOutput = SG::get(m_FPGAStripOutput, ctx);

        // use 64-bit pointer to access output
        const uint32_t *stripClusters = (*stripOutput).data();
        const uint32_t *pixelClusters = (*pixelOutput).data();

        unsigned int numStripClusters = stripClusters[0];
        ATH_MSG_DEBUG("numStripClusters: " << numStripClusters);

        unsigned int numPixelClusters = pixelClusters[0];
        ATH_MSG_DEBUG("numPixelClusters: " << numPixelClusters);

        std::unique_ptr<EFTrackingTransient::Metadata> metadata = std::make_unique<EFTrackingTransient::Metadata>();

        metadata->numOfStripClusters = numStripClusters;
        metadata->scRdoIndexSize = numStripClusters;
        metadata->numOfPixelClusters = numPixelClusters;
        metadata->pcRdoIndexSize = numPixelClusters;

        // make strip cluster
        ATH_CHECK(m_xaodClusterMaker->makeStripClusterContainer(stripClusters, metadata.get(), ctx));

        // Make pixel cluster
        ATH_CHECK(m_xaodClusterMaker->makePixelClusterContainer(pixelClusters, metadata.get(), ctx));


       
        return StatusCode::SUCCESS;
    }

} // namespace EFTrackingFPGAIntegration
