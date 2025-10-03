/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
   */

#ifndef EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATTOOL_H
#define EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATTOOL_H

#include "EFTrackingFPGAUtility/IEFTrackingFPGADataFormatTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include <cinttypes>
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"

class FPGADataFormatTool
: public extends<AthAlgTool, IEFTrackingFPGADataFormatTool> {
  public:
    using extends::extends;

    virtual StatusCode initialize() override;

    /**
     * @brief Covert the Pixel RDOs to the test vector format as requited by FPGA EF tracking alogrithms
     */
    virtual StatusCode convertPixelHitsToFPGADataFormat(
        const PixelRDO_Container &pixelRDO,
        std::vector<uint64_t> &encodedData,
        const std::vector<IdentifierHash>& hashList,
        const EventContext &ctx) const override;
    /**
     * @brief Covert the Strip RDOs to the test vector format as requited by FPGA EF tracking alogrithms
     */
    virtual StatusCode convertStripHitsToFPGADataFormat(
        const SCT_RDO_Container &stripRDO,
        std::vector<uint64_t> &encodedData,
        const std::vector<IdentifierHash>& hashList,
        const EventContext &ctx) const override;
  
    virtual  StatusCode convertFPGATracksToFPGADataFormat(
	const FPGATrackSimTrackCollection* tracks,
        std::vector<uint64_t> &encodedData,
        const EventContext &ctx
        ) const override;
  
    virtual StatusCode convertFPGASliceToFPGADataFormat(
       const FPGATrackSimHitCollection*  hitsInSlices,
       std::vector<uint64_t> &encodedData,
       const EventContext &ctx
       ) const override; 
  
    virtual StatusCode convertFPGAHitsToFPGADataFormat(
       const FPGATrackSimHitCollection* hits,
       std::vector<uint64_t> &encodedData,
       const EventContext &ctx
       ) const override; 

  private:
    const PixelID* m_pixelId = nullptr;
    const SCT_ID* m_sctId = nullptr;

    const InDetDD::SiDetectorManager* m_PIX_mgr = nullptr;
    const InDetDD::SiDetectorManager* m_SCT_mgr = nullptr;

    // Helper function for converting pixel RDO
    StatusCode convertPixelRDO(
        const PixelRDO_Container &pixelRDO,
        std::vector<uint64_t> &encodedData,
        const std::vector<IdentifierHash>& hashList,
        const EventContext &ctx
        ) const;

    // Helper function for converting strip RDO
    StatusCode convertStripRDO(
        const SCT_RDO_Container &stripRDO,
        std::vector<uint64_t> &encodedData,
        const std::vector<IdentifierHash>& hashList,
        const EventContext &ctx
        ) const;

    StatusCode convertFPGATracks(
	const FPGATrackSimTrackCollection* tracks,
        std::vector<uint64_t> &encodedData,
        const EventContext &ctx
        ) const;

    StatusCode convertFPGASlices(
        const FPGATrackSimHitCollection*  hitsInSlices,
        std::vector<uint64_t> &encodedData,
        const EventContext &ctx /*ctx*/
	) const;

    StatusCode convertFPGAHits(
        const FPGATrackSimHitCollection* hits,
        std::vector<uint64_t> &encodedData,
        const EventContext &ctx /*ctx*/
	) const;


    // Helper function for common header and Footer info
    StatusCode fillHeader(std::vector<uint64_t> &encodedData) const;
    StatusCode fillFooter(std::vector<uint64_t> &encodedData) const;

    // For module
    StatusCode fillModuleHeader(const InDetDD::SiDetectorElement* sielement, std::vector<uint64_t> &encodedData) const;


};

#endif  // EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATTOOL_H
