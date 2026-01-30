/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */

#include "EFTrackingFPGAPipeline/F100DataEncodingAlg.h"
#include "AthenaKernel/Chrono.h"

namespace EFTrackingFPGAIntegration
{
    StatusCode F100DataEncodingAlg::initialize()
    {
        ATH_MSG_INFO("Running on the FPGA accelerator");

        ATH_CHECK(m_pixelRDOKey.initialize());
        ATH_CHECK(m_stripRDOKey.initialize());
        ATH_CHECK(m_FPGAPixelRDO.initialize());
        ATH_CHECK(m_FPGAStripRDO.initialize());

        ATH_CHECK(m_FPGAPixelRDOSize.initialize());
        ATH_CHECK(m_FPGAStripRDOSize.initialize());

        ATH_CHECK(m_FPGADataFormatTool.retrieve());

        ATH_CHECK(m_roiCollectionKey.initialize(m_roiSeeded));
        ATH_CHECK(m_regionPixelSelector.retrieve(EnableTool{m_roiSeeded}));
        ATH_CHECK(m_regionStripSelector.retrieve(EnableTool{m_roiSeeded}));
        ATH_MSG_DEBUG("Running in ROI mode: "<<m_roiSeeded);


        return StatusCode::SUCCESS;
    }

    StatusCode F100DataEncodingAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F100DataEncodingAlg");

        // Get the RDOs from the SG
        auto pixelRDOHandle = SG::makeHandle(m_pixelRDOKey, ctx);
        auto stripRDOHandle = SG::makeHandle(m_stripRDOKey, ctx);
        ATH_CHECK(pixelRDOHandle.isValid());
        ATH_CHECK(stripRDOHandle.isValid());



        // Encode RDO into byte stream
        SG::WriteHandle<std::vector<uint64_t>> FPGAPixelRDO(m_FPGAPixelRDO, ctx);
        ATH_CHECK(FPGAPixelRDO.record(std::make_unique<std::vector<uint64_t> >()));

        SG::WriteHandle<std::vector<uint64_t>> FPGAStripRDO(m_FPGAStripRDO, ctx);
        ATH_CHECK(FPGAStripRDO.record(std::make_unique<std::vector<uint64_t> >()));

        std::vector<IdentifierHash> listOfPixelIds;
        std::vector<IdentifierHash> listOfStripIds;

        if (m_roiSeeded) {//enter RoI-seeded mode
            SG::ReadHandle<TrigRoiDescriptorCollection> roiCollection(m_roiCollectionKey, ctx);
            ATH_CHECK(roiCollection.isValid());
        
        
            for (const auto* roi : *roiCollection) {
                m_regionPixelSelector->lookup(ctx)->HashIDList( *roi, listOfPixelIds );
                m_regionStripSelector->lookup(ctx)->HashIDList( *roi, listOfStripIds );
            }
        }


        // Encode RDOs into byte stream
        ATH_CHECK(m_FPGADataFormatTool->convertPixelHitsToFPGADataFormat(*pixelRDOHandle, *FPGAPixelRDO, listOfPixelIds, ctx));
        ATH_CHECK(m_FPGADataFormatTool->convertStripHitsToFPGADataFormat(*stripRDOHandle, *FPGAStripRDO, listOfStripIds, ctx));

        // Store the size
        SG::WriteHandle<int> FPGAPixelRDOSize(m_FPGAPixelRDOSize, ctx);
        ATH_CHECK(FPGAPixelRDOSize.record(std::make_unique<int>(FPGAPixelRDO->size())));

        SG::WriteHandle<int> FPGAStripRDOSize(m_FPGAStripRDOSize, ctx);
        ATH_CHECK(FPGAStripRDOSize.record(std::make_unique<int>(FPGAStripRDO->size())));

        int pixelPadLength = 8;
        auto pixRemainder = FPGAPixelRDO->size() % pixelPadLength;
        if (pixRemainder != 0) {
            size_t to_add = pixelPadLength - pixRemainder;
            FPGAPixelRDO->insert(FPGAPixelRDO->end(), to_add, 0); // append zeros
        }

        int stripPadLength = 8;
        size_t stripRemainder = FPGAStripRDO->size() % stripPadLength;
        if (stripRemainder != 0) {
            size_t to_add = stripPadLength - stripRemainder;
            FPGAStripRDO->insert(FPGAStripRDO->end(), to_add, 0); // append zeros
        }

        if (msgLvl(MSG::DEBUG)){
          for (unsigned int i = 0; i < FPGAPixelRDO->size(); i++)
          {
            ATH_MSG_DEBUG("Pixel RDO[" << i << "]: " << std::hex << FPGAPixelRDO->at(i) << std::dec);
          }
          for (unsigned int i = 0; i < FPGAStripRDO->size(); i++)
          {
            ATH_MSG_DEBUG("Strip RDO[" << i << "]: " << std::hex << FPGAStripRDO->at(i) << std::dec);
          }
        }
        ATH_MSG_DEBUG("Done F100DataEncodingAlg");


       
        return StatusCode::SUCCESS;
    }

} // namespace EFTrackingFPGAIntegration
