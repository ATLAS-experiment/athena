/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
   */


#include "EFTrackingFPGAUtility/FPGADataFormatTool.h"
#include "EFTrackingFPGAUtility/FPGADataFormatUtilities.h"

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
    const std::vector<IdentifierHash>& hashList,
    const EventContext &ctx) const {

  // Fill the event header
  ATH_CHECK(fillHeader(encodedData));

  // Convert the strip RDO
  ATH_CHECK(convertPixelRDO(pixelRDO, encodedData, hashList, ctx));

  // Fill the event footer
  ATH_CHECK(fillFooter(encodedData));


  return StatusCode::SUCCESS;
}

StatusCode FPGADataFormatTool::convertStripHitsToFPGADataFormat(
    const SCT_RDO_Container &stripRDO,
    std::vector<uint64_t> &encodedData,
    const std::vector<IdentifierHash>& hashList,
    const EventContext &ctx) const {

  // Fill the event header
  ATH_CHECK(fillHeader(encodedData));

  // Convert the strip RDO
  ATH_CHECK(convertStripRDO(stripRDO, encodedData, hashList, ctx));

  // Fill the event footer
  ATH_CHECK(fillFooter(encodedData));

  return StatusCode::SUCCESS;
}



StatusCode FPGADataFormatTool::convertFPGATracksToFPGADataFormat(
  const FPGATrackSimTrackCollection* tracks,
  std::vector<uint64_t> &encodedData,
  const EventContext &ctx) const {

// Fill the event header
ATH_CHECK(fillHeader(encodedData));

// Convert the strip RDO
ATH_CHECK(convertFPGATracks(tracks, encodedData, ctx));

// Fill the event footer
ATH_CHECK(fillFooter(encodedData));


return StatusCode::SUCCESS;
}

StatusCode FPGADataFormatTool::convertFPGATracks(
  const FPGATrackSimTrackCollection* tracks,
  std::vector<uint64_t> &encodedData,
  const EventContext &/*ctx*/
  ) const {

  for (const FPGATrackSimTrack& track : *tracks) 
  {
      int bitmask = 0;
      for(const auto& hit: track.getFPGATrackSimHits())
      {
        bitmask |= 2 << hit.getLayer();
      }

      ATH_MSG_DEBUG("Encoded GTrack: ");
      ATH_MSG_DEBUG("\tetaregion: " << track.getHoughY());
      ATH_MSG_DEBUG("\tphiregion: " << track.getHoughX());
      ATH_MSG_DEBUG("\tlayerbitmask: " << bitmask);
      ATH_MSG_DEBUG("\td0: " << track.getD0());
      ATH_MSG_DEBUG("\tz0: " << track.getZ0());
      ATH_MSG_DEBUG("\tqoverpt: " << track.getQOverPt());
      ATH_MSG_DEBUG("\tphi: " << track.getPhi());
      ATH_MSG_DEBUG("\teta: " << track.getEta());

      auto gtrackWord_w1 = FPGADataFormatUtilities::fill_GTRACK_HDR_w1(
        0xee,
        0,
        track.getHoughY(),
        track.getHoughX(),
        0,
        0,
        0,
        bitmask);
      encodedData.push_back(FPGADataFormatUtilities::get_dataformat_GTRACK_HDR_w1(gtrackWord_w1));      

      auto gtrackWord_w2 = FPGADataFormatUtilities::fill_GTRACK_HDR_w2(
         0, 
         track.getD0(),
         track.getZ0(), 
         0);
      encodedData.push_back(FPGADataFormatUtilities::get_dataformat_GTRACK_HDR_w2(gtrackWord_w2));  
      
      auto gtrackWord_w3 = FPGADataFormatUtilities::fill_GTRACK_HDR_w3(
        track.getQOverPt(), 
        track.getPhi(),
        track.getEta(), 
        0);
     encodedData.push_back(FPGADataFormatUtilities::get_dataformat_GTRACK_HDR_w3(gtrackWord_w3));  
      
      auto hits = track.getFPGATrackSimHits();
      for(unsigned int i = 0 ; i < hits.size(); i++)
      {
        const auto& hit = hits[i];
        int cluster1D=0;
        int cluster2D=0;
        if(hit.getOriginalHit().getCluster1ID() >=0)
        {
          cluster1D=hit.getOriginalHit().getCluster1ID();
        }
        
        if(hit.getOriginalHit().getCluster2ID() >=0)
        {
          cluster2D=hit.getOriginalHit().getCluster2ID();
        }

        ATH_MSG_DEBUG("Encoded Hits: ");
        ATH_MSG_DEBUG("\tlast: " << (i+1 == hits.size()));
        ATH_MSG_DEBUG("\tphiregion: " << hit.getLayer());
        ATH_MSG_DEBUG("\tlayerbitmask: " << hit.getR());
        ATH_MSG_DEBUG("\td0: " << hit.getGPhi());
        ATH_MSG_DEBUG("\tz0: " << hit.getZ());
        ATH_MSG_DEBUG("\tcluster1D: " << cluster1D);
        ATH_MSG_DEBUG("\tcluster2D: " << cluster2D);

    
        auto ghit_w1 = FPGADataFormatUtilities::fill_GHITZ_w1 ((i+1 == hits.size()), hit.getLayer(), hit.getR(), hit.getGPhi(), hit.getZ(), 0, 0);
        auto ghit_w2 = FPGADataFormatUtilities::fill_GHITZ_w2 (cluster1D, cluster2D, hit.getEtaModule(), 0);
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_GHITZ_w1(ghit_w1));  
        encodedData.push_back(FPGADataFormatUtilities::get_dataformat_GHITZ_w2(ghit_w2));  
      

      }

      
    }


  return StatusCode::SUCCESS;
  }



StatusCode FPGADataFormatTool::convertPixelRDO(
    const PixelRDO_Container &pixelRDO,
    std::vector<uint64_t> &encodedData,
    const std::vector<IdentifierHash>& hashList,
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
      // if hash list has elements, check if the current Si in the list otherwise, continue
      if(hashList.size() > 0)
      {
        if(std::find(hashList.begin(), hashList.end(), sielement->identifyHash()) == hashList.end()) continue;
      }

      // Fill the module header
      if(!filledHeader)
      {
        ATH_CHECK(fillModuleHeader(sielement, encodedData));
        filledHeader = true;
      }

          // Get the pixel word
          auto pixelWord = FPGADataFormatUtilities::fill_PIXEL_EF_RDO (
            (pixelRawData == pixel_rdoCollection->back()), // last
            m_pixelId->phi_index(rdoId), // ROW
            m_pixelId->eta_index(rdoId), // COL
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
    const std::vector<IdentifierHash>& hashList,
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
        std::map<int, const SCT_RDORawData*> firedStripsToRDO;

        // Preprocess the SCT collection hits to get information for encoding strip in ITK format
        // All fired strips are stored in a map to get an overview of the full module that should be 
        // used to encode the data into the ITk format.
        for (const SCT_RDORawData* sctRawData : *SCT_Collection) {
            const Identifier rdoId = sctRawData->identify();
            const int baseLineStrip{m_sctId->strip(rdoId)};
            for (int i = 0; i < sctRawData->getGroupSize(); i++) {
                firedStrips[baseLineStrip + i] = true;
                firedStripsToRDO[baseLineStrip + i] = sctRawData;
            }
        }
        // Loop over the fired hits and encode them in the ITk strips hit map
        // Finds unique hits in the list that can be encoded and don't overlap
        std::map<int, int> stripEncodingForITK;
        std::map<int, const SCT_RDORawData* > stripEncodingForITKToRDO;
        for (const auto& [stripID, fired] : firedStrips) {
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
            stripEncodingForITKToRDO[stripID] = firedStripsToRDO[stripID];
        }

        stripNumber = 0;
        firstClusterFilled = false;

        // Process each fired strip and encode it
        for (const auto& [stripID, encoding] : stripEncodingForITK) {
            const SCT_RDORawData* sctRawData = stripEncodingForITKToRDO[stripID];
            const Identifier rdoId = sctRawData->identify();
            const InDetDD::SiDetectorElement* sielement = m_SCT_mgr->getDetectorElement(rdoId);

            // if hash list has elements, check if the current Si in the list otherwise, continue
            if(hashList.size() > 0)
            {
              if(std::find(hashList.begin(), hashList.end(), sielement->identifyHash()) == hashList.end()) continue;
            }

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
