/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
   */


#include "FPGADataFormatTool.h"
#include "FPGADataFormatUtilities.h"

StatusCode FPGADataFormatTool::initialize() {
  ATH_MSG_INFO("Initializing IEFTrackingFPGADataFormatTool tool");

  ATH_CHECK(detStore()->retrieve(m_PIX_mgr, "ITkPixel"));
  ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_SCT_mgr, "ITkStrip"));
  ATH_CHECK(detStore()->retrieve(m_sctId, "SCT_ID"));

  return StatusCode::SUCCESS;
}

StatusCode FPGADataFormatTool::convertPixelHitsToFPGADataFormat(
    const PixelRDO_Container &pixelRDO,
    std::vector<uint64_t> &encodedData,
    const EventContext &ctx) const {

  // Fill the event header
  ATH_CHECK(fillHeader(encodedData));

  // Convert the strip RDO
  ATH_CHECK(convertPixelRDO(pixelRDO, encodedData, ctx));

  // Fill the event footer
  ATH_CHECK(fillFooter(encodedData));


  return StatusCode::SUCCESS;
}

StatusCode FPGADataFormatTool::convertStripHitsToFPGADataFormat(
    const SCT_RDO_Container &stripRDO,
    std::vector<uint64_t> &encodedData,
    const EventContext &ctx) const {

  // Fill the event header
  ATH_CHECK(fillHeader(encodedData));

  // Convert the strip RDO
  ATH_CHECK(convertStripRDO(stripRDO, encodedData, ctx));

  // Fill the event footer
  ATH_CHECK(fillFooter(encodedData));

  return StatusCode::SUCCESS;
}



StatusCode FPGADataFormatTool::convertPixelRDO(
    const PixelRDO_Container &pixelRDO,
    std::vector<uint64_t> &encodedData,
    const EventContext &/*ctx*/
    ) const {

  bool filledHeader = false;
  for (const InDetRawDataCollection<PixelRDORawData>* pixel_rdoCollection : pixelRDO) 
  {
    if (pixel_rdoCollection == nullptr) { continue; }

    // loop on all RDOs
    for (const PixelRDORawData* pixelRawData : *pixel_rdoCollection) 
    {
      Identifier rdoId = pixelRawData->identify();
      // get the det element from the det element collection
      const InDetDD::SiDetectorElement* sielement = m_PIX_mgr->getDetectorElement(rdoId); 

      // Fill the module header
      if(!filledHeader)
      {
        ATH_CHECK(fillModuleHeader(sielement, encodedData));
        filledHeader = true;
      }

          // Get the pixel word
          auto pixelWord = FPGADataFormatUtilities::fill_PIXEL_EF_RDO (
            (pixelRawData == pixel_rdoCollection->back()), // last
            m_pixelId->eta_index(rdoId), // ROW
            m_pixelId->phi_index(rdoId), // COL
            pixelRawData->getToT(), // TOT
            pixelRawData->getLVL1A(),  // Lvl!
            0 // Spare
            );

          // Push the word into the vector
          encodedData.push_back(FPGADataFormatUtilities::get_dataformat_PIXEL_EF_RDO(pixelWord));
      //}
    } // end for each RDO in the collection

    // reset the header 
    filledHeader = false;

  } // for each pixel RDO collection

  return StatusCode::SUCCESS;
}

StatusCode FPGADataFormatTool::convertStripRDO(
    const SCT_RDO_Container &stripRDO,
    std::vector<uint64_t> &encodedData,
    const EventContext &/*ctx*/
) const {
    constexpr int MaxChannelinStripRow = 128;
    long unsigned int stripNumber = 0;
    bool filledHeader = false;

    uint64_t packedWord = 0;
    bool firstClusterFilled = false;

    for (const InDetRawDataCollection<SCT_RDORawData>* SCT_Collection : stripRDO) {
        if (SCT_Collection == nullptr) { continue; }

        std::map<int, bool> firedStrips;

        // Preprocess the SCT collection hits to get information for encoding strip in ITK format
        // All fired strips are stored in a map to get an overview of the full module that should be 
        // used to encode the data into the ITk format.
        for (const SCT_RDORawData* sctRawData : *SCT_Collection) {
            const Identifier rdoId = sctRawData->identify();
            const int baseLineStrip{m_sctId->strip(rdoId)};
            for (int i = 0; i < sctRawData->getGroupSize(); i++) {
                firedStrips[baseLineStrip + i] = true;
            }
        }
        // Loop over the fired hits and encode them in the ITk strips hit map
        // Finds unique hits in the list that can be encoded and don't overlap
        std::map<int, int> stripEncodingForITK;
        for (auto& [stripID, fired] : firedStrips) {
            // Skip strips that have already been used in a cluster
            if (!fired) continue;

            // Check the next 3 hits if they exist and have a hit in them
            std::bitset<3> hitMap;
            int currChipID = stripID / MaxChannelinStripRow;
            int maxStripIDForCurrChip = (currChipID + 1) * MaxChannelinStripRow;

            for (int i = 0; i < 3; i++) {
                // Do not cluster strips that are outside the range of this chip
                if ((stripID + 1 + i) >= maxStripIDForCurrChip) continue;
                if (firedStrips.find(stripID + 1 + i) != firedStrips.end()) {
                    if (firedStrips.at(stripID + 1 + i)) {
                        hitMap[2 - i] = 1;
                        firedStrips[stripID + 1 + i] = false;
                    } else {
                        hitMap[2 - i] = 0;
                    }
                }
            }

            // Encode the hit map into an integer
            stripEncodingForITK[stripID] = static_cast<int>(hitMap.to_ulong());
        }

        stripNumber = 0;
        firstClusterFilled = false;
        // Process each fired strip and encode it
        for (const SCT_RDORawData* sctRawData : *SCT_Collection) {
            const Identifier rdoId = sctRawData->identify();
            const InDetDD::SiDetectorElement* sielement = m_SCT_mgr->getDetectorElement(rdoId);
            int stripID = m_sctId->strip(rdoId);

            if (stripEncodingForITK.find(stripID) != stripEncodingForITK.end()) {
                // Fill the module header if not already filled
                if (!filledHeader) {
                    if (!fillModuleHeader(sielement, encodedData)) return StatusCode::FAILURE;
                    filledHeader = true;
                }

                // Compute chip ID and ITk strip ID
                int chipID = stripID / MaxChannelinStripRow;
                int ITkStripID = stripID % MaxChannelinStripRow;

                // Adjust for row offset based on the eta module index
                int offset = m_sctId->eta_module(rdoId) % 2;
                if (m_sctId->barrel_ec(rdoId) == 0) {
                    offset = (std::abs(m_sctId->eta_module(rdoId)) - 1) % 2;
                }
                ITkStripID += offset * MaxChannelinStripRow;
                stripNumber++;
                // Determine if this is the last cluster in the module
                bool lastBit = (stripNumber == stripEncodingForITK.size());

                // Create the encoded strip word
                auto stripWord = FPGADataFormatUtilities::fill_STRIP_EF_RDO(
                    lastBit,          // last bit indicating module boundary
                    chipID,           // chip ID
                    ITkStripID,      // cluster number
                    stripEncodingForITK.at(stripID), // cluster map
                    0                // spare bits
                );

                uint32_t encodedCluster = FPGADataFormatUtilities::get_dataformat_STRIP_EF_RDO(stripWord);

                // **Pack two clusters into a single 64-bit word**
                if (!firstClusterFilled) {
                    packedWord = (static_cast<uint64_t>(encodedCluster) << 32); // Store first cluster in upper 32 bits
                    firstClusterFilled = true;
                } else {
                    packedWord |= static_cast<uint64_t>(encodedCluster); // Store second cluster in lower 32 bits
                    encodedData.push_back(packedWord);  // Push the full packed word
                    firstClusterFilled = false;  // Reset flag
                    packedWord = 0;  // Clear for the next pair
                }

                // If this is the last cluster in the module and a single cluster is left, push it
                if (lastBit && firstClusterFilled) {
                    encodedData.push_back(packedWord);
                }
            }
        }
      // Reset the header flag for the next module
      filledHeader = false;
    } // end for each RDO in the strip collection

    return StatusCode::SUCCESS;
}

// Helper function for common header and Footer info
StatusCode FPGADataFormatTool::fillHeader(std::vector<uint64_t> &encodedData) const
{
  // Fill the event header
  auto header_w1 = FPGADataFormatUtilities::fill_EVT_HDR_w1 (FPGADataFormatUtilities::EVT_HDR_FLAG, 1, 0, 0);
  encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_HDR_w1(header_w1));

  // Fill the event header
  auto header_w2 = FPGADataFormatUtilities::fill_EVT_HDR_w2 (242000, 0);
  encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_HDR_w2(header_w2));

  // Fill the event header
  auto header_w3 = FPGADataFormatUtilities::fill_EVT_HDR_w3 (0, 0);
  encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_HDR_w3(header_w3));

  return StatusCode::SUCCESS;
}

StatusCode FPGADataFormatTool::fillFooter(std::vector<uint64_t> &encodedData) const
{
  // Fill the event header
  auto footer_w1 = FPGADataFormatUtilities::fill_EVT_FTR_w1 (FPGADataFormatUtilities::EVT_FTR_FLAG, 0, 0);
  encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w1(footer_w1));

  // Fill the event header
  auto footer_w2 = FPGADataFormatUtilities::fill_EVT_FTR_w2 (0);
  encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w2(footer_w2));

  // Fill the event header
  auto footer_w3 = FPGADataFormatUtilities::fill_EVT_FTR_w3 (encodedData.size(), 44939973);
  encodedData.push_back(FPGADataFormatUtilities::get_dataformat_EVT_FTR_w3(footer_w3));

  return StatusCode::SUCCESS;
}

StatusCode FPGADataFormatTool::fillModuleHeader(const InDetDD::SiDetectorElement* sielement, std::vector<uint64_t> &encodedData) const
{
  auto mod_w1 = FPGADataFormatUtilities::fill_M_HDR_w1 (FPGADataFormatUtilities::M_HDR_FLAG, sielement->identify().get_identifier32().get_compact(), 
                                                        sielement->identifyHash().value(), 0);
  encodedData.push_back(FPGADataFormatUtilities::get_dataformat_M_HDR_w1(mod_w1));

  return StatusCode::SUCCESS;
}
