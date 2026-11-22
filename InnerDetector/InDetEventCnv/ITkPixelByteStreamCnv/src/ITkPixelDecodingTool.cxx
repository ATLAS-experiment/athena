/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#include "ITkPixelByteStreamCnv/ITkPixelDecodingTool.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "ITkPixelDecodingAlg.h"
#include "ITkPixelPhaseIIRDOCallback.h"
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/StatusCode.h>
#include <itksw/pix/endec/DecCore.hpp>
#include <itksw/pix/endec/intf/Options.hpp>
#include <cstdint>

StatusCode ITkPixelDecodingTool::initialize()
{
  ATH_CHECK(m_robDataProviderSvc.retrieve());
  ATH_CHECK(m_pixelCablingKey.initialize());

  ATH_CHECK(m_roiCollectionKey.initialize(m_roiSeeded));
  if (m_roiSeeded) {
    ATH_CHECK(m_regionSelector.retrieve());
  } else {
    m_regionSelector.disable();
  }

  return StatusCode::SUCCESS;
}

template <typename TStreamInfo>
StatusCode ITkPixelDecodingTool::aggregateData(
  const EventContext& ctx,
  std::vector<uint64_t> & bytestream_data,
  TStreamInfo & streams_info
  ) const
{
  // Cabling data
  SG::ReadCondHandle<ITkPixelCablingData> cabling_handle(m_pixelCablingKey, ctx);
  ATH_CHECK(cabling_handle.isValid());
  const ITkPixelCablingData* cabling = *cabling_handle;
  if (!cabling) {
    ATH_MSG_ERROR("no cabling data");
    return StatusCode::FAILURE;
  }

  // Get list of ROBs to decode
  std::vector<const eformat::ROBFragment<const uint32_t*>*> robs;
  if (!m_roiSeeded) {
    m_robDataProviderSvc->getROBData(ctx, cabling->sourceIDs(), robs);
  } else {
    std::vector<uint32_t> listOfROBs;
    SG::ReadHandle<TrigRoiDescriptorCollection> roiCollection(m_roiCollectionKey, ctx);
    ATH_CHECK(roiCollection.isValid());
    TrigRoiDescriptor superRoI; // Add all RoIs to a super-RoI
    superRoI.reserve(roiCollection->size());
    superRoI.setComposite(true);
    superRoI.manageConstituents(false);
    for (const TrigRoiDescriptor* roi : *roiCollection) {
      superRoI.push_back(roi);
    }
    m_regionSelector->lookup(ctx)->ROBIDList(superRoI, listOfROBs);
    m_robDataProviderSvc->getROBData(ctx, listOfROBs, robs);
  }
  ATH_MSG_DEBUG("retrieved " << robs.size() << " ROB fragments");

  // Pre-allocate expected amount of data to avoid effect of per-ROB resizing
  bytestream_data.reserve(m_expectedDataSize / 8);

  //The ROB/ROD payload contains a chain of [FE header 0] [FE data 0] ... [FE
  //header N] [FE data N]. We need to sort these header-data pairs in each ROB
  //payload according to the module ID to fill each module exactly once. from
  //the GBT fragment building algorithm documentation
  //(https://gitlab.cern.ch/atlas-tdaq-software/swrod#gbt-fragment-building-algorithm)
  //"size: 16-bit value that contains a total size (in 4-byte words) of the data
  //packet including the size of the header itself." i. e. the first 16 bits of
  //the first word allow us to identify where the next FE starts, etc. We can
  //jump once through the payload to separate the FE data and store the
  //moduleID, {start idx, size} map, sort according to the key and then read in
  //that order from the underlying array without copying.

  for (const auto& rob : robs) {
    const uint32_t * const payload = rob->rod_data();
    const uint32_t payload_length = rob->rod_ndata();

    // Data from different Pixel modules might be interleaved but the decoders
    // have to decode one module after the other. So we sort the GBT fragments
    // by module ID. Module ID is the 24 least-significant bits of a detector
    // resource ID.
    auto comp = [](const uint32_t& a, const uint32_t& b){
      uint32_t aLSB = a & 0x00FFFFFF;
      uint32_t bLSB = b & 0x00FFFFFF;
      if (aLSB != bLSB) return aLSB < bLSB;
      return a < b;
    };
    // detector resource ID -> {GBT fragment start offset, GBT fragment length}
    std::map<uint32_t, std::pair<size_t, uint16_t>, decltype(comp)> gbt_fragments(comp);

    // total GBT data payload size in this ROB
    size_t data_size_64bits = 0;
    size_t idx = 0;
    while (idx < payload_length){
      const uint16_t gbt_fragment_size = (payload[idx] & 0xFFFF0000) >> 16;
      const uint32_t detector_resource_id = payload[idx + 1];
      // This is where we want to map detectorResourceID on trueDetectorResourceID
      gbt_fragments.insert({detector_resource_id, {idx, gbt_fragment_size}});
      idx += gbt_fragment_size;

      // The frames are always 64 bits split into 32-bit words, so always
      // divisible by 2. The first 32-bit words are the GBT header.
      data_size_64bits += (gbt_fragment_size - 2) / 2;
    }

    // This is where we write (the current GBT fragment) in the aggregation buffer
    size_t payload64_idx = bytestream_data.size();
    // We need to store the start index of each stream. The first one starts here.
    size_t payload64_start = bytestream_data.size();
    // Make room for this ROB's data
    bytestream_data.resize(payload64_idx + data_size_64bits);

    // The GBT fragments are sorted: looping over the map gives access to data
    // for all modules sequentially.
    for (const auto& [detector_resource_id, start_length] : gbt_fragments) {
      const auto [start, length] = start_length;

      // Copy data to output buffer swapping 32-bit words.
      // The 64-bit GBT header is skipped at the beginning.
      for (uint32_t word = start + 2; word < start + length; word += 2){
        bytestream_data[payload64_idx] =
          (static_cast<uint64_t>(payload[word])<< 32)
          | static_cast<uint64_t>(payload[word + 1]);
        ++payload64_idx;
      }

      const uint16_t stream_size_64bits = (length - 2) / 2;
      const uint32_t module_id = ITkPixelCabling::dridToModuleID(detector_resource_id);
      streams_info.push_back({payload64_start, stream_size_64bits,
        module_id, static_cast<uint8_t>(cabling->transformType(module_id)),
        ITkPixelCabling::dridToChipID(detector_resource_id)});

        // This will be the start index of the next stream
        payload64_start += stream_size_64bits;
      }
    }

    return StatusCode::SUCCESS;
  }

  template <typename TStreamInfo, typename TDecodingCallback>
  StatusCode ITkPixelDecodingTool::decodeData(
    std::vector<uint64_t> const & bytestream_data,
    TStreamInfo const & streams_info,
    TDecodingCallback & decoding_cb
  ) const
{
  constexpr itksw::pix::endec::intf::Options DECODING_OPTIONS{
    .en_chip_id = true, .en_eos = false};
  constexpr itksw::pix::endec::DataFormat DECODING_FORMAT{
    .options = DECODING_OPTIONS};

  itksw::pix::endec::DecCore<TDecodingCallback> decoder(DECODING_FORMAT, decoding_cb);

  uint32_t current_module_id = 0xFFFFFFFF;
  for (ITkPixelDecodingStreamInfo const & stream_info: streams_info) {

    // When handling data for a new module, we need to inform the decoding callback
    const uint32_t module_id = stream_info.m_moduleId;
    if (module_id != current_module_id) {
      decoding_cb.setOfflineID(stream_info.m_moduleId);
      decoding_cb.setTransformType(ITkPixelCabling::TransformType(
        stream_info.m_transformType));
    }
    decoding_cb.setChipID(stream_info.m_chipId);

    try {
      decoder.initialize();
      decoder.decode({bytestream_data.data() + stream_info.m_dataOffset,
        stream_info.m_dataLength});
      decoder.finalize();
    } catch (std::runtime_error & exc) {
      if (m_FailOnDecodingError) {
        ATH_MSG_ERROR(exc.what());
        return StatusCode::FAILURE;
      } else {
        ATH_MSG_WARNING(exc.what());
      }
    }
  }

  return StatusCode::SUCCESS;
}

/*
 * Instantiate the required templates
 */

// Legacy RDO and Phase-II RDO decoders use the same type for stream info
template StatusCode ITkPixelDecodingTool::aggregateData<> (
  const EventContext& ctx,
  std::vector<uint64_t> & bytestream_data,
  std::vector<ITkPixelDecodingStreamInfo> & streams_info
  ) const;

// Legacy RDO decoder
template StatusCode ITkPixelDecodingTool::decodeData<>(
  std::vector<uint64_t> const & bytestream_data,
  std::vector<ITkPixelDecodingStreamInfo> const & streams_info,
  PixelCallbacks::RDOCallback & decoding_cb
  ) const;

// Phase-II RDO decoder
template StatusCode ITkPixelDecodingTool::decodeData<>(
  std::vector<uint64_t> const & bytestream_data,
  std::vector<ITkPixelDecodingStreamInfo> const & streams_info,
  PixelCallbacksPhaseIIRDO::PhaseIIRDOCallback & decoding_cb
  ) const;
