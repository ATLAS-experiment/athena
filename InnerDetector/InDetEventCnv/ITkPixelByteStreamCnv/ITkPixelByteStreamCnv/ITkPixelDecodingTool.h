/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#ifndef ITKPIXELBYTESTREAMCNV_ITKPIXELDECODINGTOOL_H
#define ITKPIXELBYTESTREAMCNV_ITKPIXELDECODINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include <Gaudi/Property.h>
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"
#include "IRegionSelector/IRegSelTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include <ITkPixelDecodingStreamInfo.h>
#include <vector>
#include <cstdint>

struct ITkPixelDecodingTool: public AthAlgTool
{
  using AthAlgTool::AthAlgTool;
  virtual StatusCode initialize() override;

  /* Full-event or RoI seeded Pixel sources payload data aggregation.
   *
   * After calling this, bytestream_data contains all GBT fragment payload data,
   * grouped by Pixel module.
   * 
   * During the aggregation, streams_info is given one {start index, length,
   * module ID, transform type, chip ID} per GBT fragment.
   * 
   * TStreamInfo must implement:
   * push_back({
   *   size_t: index where stream N's data starts in bytestream_data
   *   uint16_t: size of stream N's data
   *   uint32_t: module ID associated with stream N
   *   uint8_t: transform type to use to transform the coordinates of this chip
   *   uint8_t: chip ID of this stream, required to transform coordinates
   *   })
   */
  template <typename TStreamInfo>
  StatusCode aggregateData(
    const EventContext& ctx,
    std::vector<uint64_t> & bytestream_data,
    TStreamInfo & streams_info
    ) const;

  /* Aggregated data decoding
   *
   * The caller provides data and streams info as produced by aggregateData(),
   * plus a callback object for the Endec decoder.
   *
   * For this function streams_info must be iterable.
   */
  template <typename TStreamInfo, typename TDecodingCallback> 
  StatusCode decodeData(
    std::vector<uint64_t> const & bytestream_data,
    TStreamInfo const & streams_info,
    TDecodingCallback & decoding_cb
    ) const;

private:
  ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc{this,
    "ROBDataProvider", "ROBDataProviderSvc"};
  SG::ReadCondHandleKey<ITkPixelCablingData> m_pixelCablingKey{this,
    "PixelCablingKey", "ITkPixelCablingData", "Pixel Cabling"};

  Gaudi::Property<size_t> m_expectedDataSize{this,
    "ExpectedDataSize", 2000000,
    "Expected amount of data [B] to decode per event, used to pre-allocate"};
  Gaudi::Property<bool> m_FailOnDecodingError{this,
    "FailOnDecodingError", false,
    "Whether the algortihm should fail on decoding error"};

  Gaudi::Property<bool> m_roiSeeded{this,
    "isRoI_Seeded", false, "Use RoI"};
  ToolHandle<IRegSelTool> m_regionSelector{this,
    "RegSelTool", "RegSelTool/RegSel_ITkPixel"};
  SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey{this,
    "RoIs", "", "RoIs to read in"};
};

#endif // ITKPIXELBYTESTREAMCNV_ITKPIXELDECODINGTOOL_H