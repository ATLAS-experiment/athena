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

  int pixelCounter = 0;
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
            pixelCounter,  // id
            0 // Spare
            );
            pixelCounter++;

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
  int stripNumber = 0;

  bool filledHeader = false;

  for (const InDetRawDataCollection<SCT_RDORawData>* SCT_Collection : stripRDO) {
    if (SCT_Collection == nullptr) { continue; }

    std::map<int, bool> firedStrips;
    // Preprocess the SCT collection hits to get information for encoding strip in ITK format
    // All strips fired read into a map to an overview of full module that should be used to encode
    // the data into the ITk formatl
    for (const SCT_RDORawData* sctRawData : *SCT_Collection) 
    {
      const Identifier rdoId = sctRawData->identify();
      const int baseLineStrip{m_sctId->strip(rdoId)};

      for(int i = 0; i < sctRawData->getGroupSize(); i++) {
        firedStrips[baseLineStrip+ i] = true;
      }
    }

    // Loop over the fired hits and encode them in the ITk strips hit map
    // It find unique hits in the list that can be encoded and don't overlap
    std::map<int, int> stripEncodingForITK;
    for(auto& [stripID, fired]: firedStrips)
    {
      // Don't use the strip that has been set false. 
      // This will be the case where neighbouring strip will "used up in the cluster"
      // And then we don't want to re use them 
      if(!fired) continue;

      // Check the next 3 hits if they are there and have a hit in them
      std::bitset<3> hitMap;

      // Get the current chip id of the strip
      int currChipID = stripID / MaxChannelinStripRow;
      // Compute the maximum stripID this chip can have
      int maxStripIDForCurrChip = (currChipID + 1) * MaxChannelinStripRow;

      for(int i = 0; i < 3; i++)
      {    
        // We don't want to "cluster" strips that are outside the range of this chip
        if((stripID + 1 + i) >= maxStripIDForCurrChip) continue;

        if(firedStrips.find(stripID + 1 + i) != firedStrips.end())
        {
          if(firedStrips.at(stripID + 1 + i))
          {
            hitMap[2 - i] = 1;
            firedStrips[stripID + 1 + i] = false;
          }
          else
          {
            hitMap[2 - i] = 0;
          }
        }
      }
      // Encode the hit map into a int
      stripEncodingForITK[stripID] = (int)(hitMap.to_ulong());
    }

    // count number of strip we need to encode for figuring if we are at the last strip
    int stripToEncode = 0;
    int stripAlreadyEncoded = 0;
    for(const auto& [stripID, fired]: firedStrips)
    {
      if(fired) stripToEncode += 1;
    }

    // Actual creation of the FPGAHit objects
    for (const SCT_RDORawData* sctRawData : *SCT_Collection) 
    {
      const Identifier rdoId = sctRawData->identify();
      // get the det element from the det element collection
      const InDetDD::SiDetectorElement* sielement = m_SCT_mgr->getDetectorElement(rdoId);

      // If the strip has been identified by the previous for loop as a valid hit that can be encoded into ITk Strip format
      int stripID   = m_sctId->strip(rdoId);
      if(stripEncodingForITK.find(stripID) != stripEncodingForITK.end())
      { 
        // Fill the module header
        if(!filledHeader)
        {
          if(!fillModuleHeader(sielement, encodedData)) return StatusCode::FAILURE;
          filledHeader = true;
        }

        // Each ITK ABC chip reads 128 channels in one row, so we just need to divide the current strip with 128 to get the chip index
        // for the Strip ID, it is the remainder left after dividing by 128
        int chipID = stripID / MaxChannelinStripRow;
        int ITkStripID = stripID % MaxChannelinStripRow;

        // for each ABC chip readout, each reads 256 channels actually. 0-127 corresponds to lower row and then 128-255 corresponds to the 
        // upper. This can be simulated in the code by using the eta module index. Even index are not offest, while odd index, the 
        // strip id is offest by 128
        // One point to not is that for barrel, the eta module index start at 1, and not zero. Hence a shift of 1 is needed
        int offset = m_sctId->eta_module(rdoId) % 2;
        if(m_sctId->barrel_ec(rdoId) == 0) offset = (std::abs(m_sctId->eta_module(rdoId)) - 1) % 2;

        ITkStripID += offset * MaxChannelinStripRow;

        stripAlreadyEncoded++;
        auto stripWord = FPGADataFormatUtilities::fill_STRIP_EF_RDO (
          (stripAlreadyEncoded == stripToEncode), //last
          chipID, //chip ID 
          ITkStripID, //strip ID 
          stripEncodingForITK.at(stripID), // cluster map 
          stripNumber, // id 
          0 // spare
          );
        stripNumber++;
        // Push the word into the vector
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_STRIP_EF_RDO(stripWord));
      }

    } // end for each RDO in the strip collection

    // reset the header 
    filledHeader = false;

  } // end for each strip RDO collection
  // dump all RDO's and SDO's for a given event, for debugging purposes

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
  auto mod_w1 = FPGADataFormatUtilities::fill_M_HDR_w1 (FPGADataFormatUtilities::M_HDR_FLAG, sielement->identify().get_identifier32().get_compact(), 0);
  encodedData.push_back(FPGADataFormatUtilities::get_dataformat_M_HDR_w1(mod_w1));

  return StatusCode::SUCCESS;
}
