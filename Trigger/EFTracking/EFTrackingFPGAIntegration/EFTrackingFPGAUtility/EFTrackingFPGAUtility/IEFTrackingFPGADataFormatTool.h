/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef EFTRACKINGFPGAINTEGRATION_IEFTRACKINGFPGADATAFORMATTOOL_H
#define EFTRACKINGFPGAINTEGRATION_IEFTRACKINGFPGADATAFORMATTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"

/**
 * @class IEFTrackingFPGADataFormatTool
 * @brief Abstrct interface class for EFTrackingFPGADataFromatTool
 */
class IEFTrackingFPGADataFormatTool : virtual public IAlgTool {
 public:
  // Interface ID declaration for Gaudi
  DeclareInterfaceID(IEFTrackingFPGADataFormatTool, 1, 0);

  virtual StatusCode convertPixelHitsToFPGADataFormat(
      const PixelRDO_Container &pixelRDO,
      std::vector<uint64_t> &encodedData,
      const std::vector<IdentifierHash>& hashList,
      const EventContext &ctx) const = 0;

  virtual StatusCode convertStripHitsToFPGADataFormat(
      const SCT_RDO_Container &stripRDO,
      std::vector<uint64_t> &encodedData,
      const std::vector<IdentifierHash>& hashList,
      const EventContext &ctx) const = 0;

  virtual  StatusCode convertFPGATracksToFPGADataFormat(
        const FPGATrackSimTrackCollection* tracks,
        std::vector<uint64_t> &encodedData,
        const EventContext &ctx
        ) const = 0;
};

#endif  // EFTRACKINGFPGAINTEGRATION_IEFTRACKINGFPGADATAFORMATTOOL_H
