/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define DETAIL_DUMP_ON false
#define ADC_DUMP_ON false
#define VALIDATE_DUMP_ON false

#include "LArByteStream/LArLATOMEDecoder.h"

#include <byteswap.h>

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "LArByteStream/LATOMEMapping.h"
#include "LArByteStream/Mon.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "LArRawConditions/LArCalibParams.h"

static const InterfaceID IID_ILArLATOMEDecoder("LArLATOMEDecoder", 1, 0);

using namespace OFFLINE_FRAGMENTS_NAMESPACE;

LArLATOMEDecoder::LArLATOMEDecoder(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type, name, parent), m_sc2ccMappingTool("CaloSuperCellIDTool") {
  declareInterface<LArLATOMEDecoder>(this);
}

const InterfaceID& LArLATOMEDecoder::interfaceID() {
  return IID_ILArLATOMEDecoder;
}

StatusCode LArLATOMEDecoder::initialize() {

  ATH_CHECK(detStore()->retrieve(m_onlineId, "LArOnline_SuperCellID"));

  ATH_CHECK(m_sc2ccMappingTool.retrieve());
  ATH_CHECK(m_cablingKeySC.initialize());

  return StatusCode::SUCCESS;
}

StatusCode LArLATOMEDecoder::convert(const RawEvent* re, const LArLATOMEMapping* map, const LArOnOffIdMapping* onoffmap, const LArCalibLineMapping* clmap,
                                     LArAccumulatedDigitContainer* accdigits, LArAccumulatedCalibDigitContainer* caccdigits,
                                     LArLATOMEHeaderContainer* header_coll) const {

  if(!re) return StatusCode::FAILURE;;
  bool ret = false;
  // Check fragment validity:
  try {
    ret = re->check();
  } catch (eformat::Issue& ex) {
    ATH_MSG_WARNING("Exception while checking eformat fragment validity: " << ex.what());
    ret = false;
  }
  if (!ret) {
    ATH_MSG_ERROR("Got invalid RawEvent fragment");
    return StatusCode::FAILURE;
  }
  // Build TOC
  std::map<eformat::SubDetectorGroup, std::vector<const uint32_t*> > robIndex;
  eformat::helper::build_toc(*re, robIndex);
  for (auto mapit : robIndex)
    ATH_MSG_DEBUG("Rob Index subdetgroup is " << std::hex << mapit.first);
  std::map<eformat::SubDetectorGroup, std::vector<const uint32_t*> >::const_iterator robIt = robIndex.find(eformat::LAR);
  if (robIt != robIndex.end()) {
    const std::vector<const uint32_t*>& robs = robIt->second;
    for (const uint32_t* pRob : robs) {
      try {
        ROBFragment robFrag(pRob);
        if (m_protectSourceId) {
          uint32_t latomeSourceID = robFrag.rod_source_id();
          if (!(latomeSourceID & 0x1000)) {
            ATH_MSG_DEBUG(" discarding non latome source ID " << std::hex << latomeSourceID);
            continue;
          }
          ATH_MSG_DEBUG(" found latome source ID " << std::hex << latomeSourceID);
        }
        EventProcess ev(this, 0, 0, 0, 0, accdigits, caccdigits, header_coll);
        ev.fillCollection(&robFrag, map, onoffmap, clmap);
      } catch (eformat::Issue& ex) {
        ATH_MSG_WARNING(" exception thrown by ROBFragment, badly corrupted event. Abort decoding ");
        if (accdigits)
          accdigits->clear();
        if (caccdigits)
          caccdigits->clear();
        if (header_coll)
          header_coll->clear();
        break;
      }
    }  // end loop over robs
  }  // end if got LAr-RODs

  return StatusCode::SUCCESS;
}

StatusCode LArLATOMEDecoder::convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& robFrags, const LArLATOMEMapping* map,
                                     LArDigitContainer* adc_coll, LArDigitContainer* adc_bas_coll, LArRawSCContainer* et_coll, LArRawSCContainer* et_id_coll,
                                     LArLATOMEHeaderContainer* header_coll) const {

  // Check fragment validity:
  // Build TOC
  if (robFrags.size() > 0) {
    for (const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment* pRob : robFrags) {
      try {
        if (m_protectSourceId) {
          uint32_t latomeSourceID = pRob->rod_source_id();
          if (!(latomeSourceID & 0x1000)) {
            ATH_MSG_DEBUG(" discarding non latome source ID " << std::hex << latomeSourceID);
            continue;
          }
          ATH_MSG_DEBUG(" found latome source ID " << std::hex << latomeSourceID);
        }
        EventProcess ev(this, adc_coll, adc_bas_coll, et_coll, et_id_coll, 0, 0, header_coll);
        ev.fillCollection(pRob, map, nullptr, nullptr);
      } catch (eformat::Issue& ex) {
        ATH_MSG_WARNING(" exception thrown by ROBFragment, badly corrupted event. Abort decoding ");
        if (adc_coll)
          adc_coll->clear();
        if (adc_bas_coll)
          adc_bas_coll->clear();
        if (et_coll)
          et_coll->clear();
        if (et_id_coll)
          et_id_coll->clear();
        if (header_coll)
          header_coll->clear();
        break;
      }
    }  // end loop over robs
  }  // end if got LAr-RODs
  return StatusCode::SUCCESS;
}

LArLATOMEDecoder::EventProcess::EventProcess(const LArLATOMEDecoder* decoderInput, LArDigitContainer* adc_coll, LArDigitContainer* adc_bas_coll,
                                             LArRawSCContainer* et_coll, LArRawSCContainer* et_id_coll, LArAccumulatedDigitContainer* accdigits,
                                             LArAccumulatedCalibDigitContainer* caccdigits, LArLATOMEHeaderContainer* header_coll)
    : AthMessaging(Gaudi::svcLocator()->service<IMessageSvc>("MessageSvc"), "LArLATOMEDecoder::EventProcess"),
      m_decoder(decoderInput),
      m_adc_coll(adc_coll),
      m_adc_bas_coll(adc_bas_coll),
      m_et_coll(et_coll),
      m_et_id_coll(et_id_coll),
      m_accdigits(accdigits),
      m_caccdigits(caccdigits),
      m_header_coll(header_coll) {
  m_latomeID = 0;
  m_l1ID = 0;
  m_ROBFragSize = 0;
  m_nPackets = 0;
  m_iPacket = 0;
  m_nWordsPerPacket = 0;
  m_monHeaderSize = 0;
  m_region = 0;
  m_nStreams = 0;
  m_streamNumber = 0;
  m_at0typeRec = (Word)MonDataType::Invalid;
  m_at1typeRec = (Word)MonDataType::Invalid;
  m_at0type = (Word)MonDataType::Invalid;
  m_at1type = (Word)MonDataType::Invalid;
  m_at0nBC = 0;
  m_at1nBC = 0;
  m_at0BC = 0;
  m_at1BC = 0;
  m_activeSC = 0;
  m_nsc1 = 0;
  m_nsc2 = 0;
  m_nsc3 = 0;
  m_nsc4 = 0;
  m_nsc5 = 0;
  m_nsc6 = 0;
  m_headerDecoded = false;

  m_nBC_rawADC = 0;
  m_nBC_ADC = 0;
  m_nBC_E = 0;
  m_nBC_EID = 0;

  m_nBC_Averaged = 0;

  m_BC_rawADC = 0;
  m_BC_ADC = 0;
  m_BC_E = 0;
  m_BC_EID = 0;

  m_hasRawAdc = false;
  m_hasAdc = false;
  m_hasE = false;
  m_hasEID = false;

  m_isAveraged = (m_caccdigits != 0);
  m_isAutoCorr = (m_accdigits != 0);

  if (!m_isAveraged && !m_isAutoCorr) {
    m_rawValuesInEvent.resize(N_LATOME_CHANNELS);
  } else {
    m_averagedRawValuesInEvent.resize(N_LATOME_CHANNELS);
    m_latomeCalibPatternsInEvent.resize(2);
  }
}

bool LArLATOMEDecoder::EventProcess::compareOrSet(Word& param, Word value, bool compare) {
  if (!compare) {
    param = value;
    return true;
  }
  return param == value;
}

unsigned int LArLATOMEDecoder::EventProcess::decodeTrailer(const uint32_t* p, unsigned int offset) {
  /// for now the trailerhas only 2 fixed words, just check them
  if (bswap_32(p[offset]) != 0xc0ffee00 || bswap_32(p[offset + 1] != 0xaaaaaaaa)) {
    ATH_MSG_WARNING("Problem in trailer at packet " << m_iPacket << " words " << std::hex << bswap_32(p[offset]) << ", " << bswap_32(p[offset + 1])
                                                    << std::dec);
  }
  /// fixed shift for now but in case we get a dynamic trailer (hopefully not)
  return offset + m_monTrailerSize;
}

unsigned int LArLATOMEDecoder::EventProcess::decodeHeader(const uint32_t* p, unsigned int offset) {

  //// lets do it quick and dirty now. Should be handled more properly when we have the LatomeHeaderClass
  int monheadererror = 0;
  int monheadererrorbit = 0;
  if (!compareOrSet(m_latomeID, bswap_32(p[0 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  Word l1IDtmp = m_l1ID;
  if (!compareOrSet(l1IDtmp, bswap_32(p[1 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (l1IDtmp != m_l1ID) {
    ATH_MSG_DEBUG("Mon header L1ID " << l1IDtmp << " different from rod header L1ID " << m_l1ID);
  }
  ATH_MSG_DEBUG(" latomeID: " << m_latomeID << " l1ID: " << m_l1ID);

  Word monHeaderMarker = bswap_32(p[2 + offset]);
  Word monCheckPoint = bswap_32(p[4 + offset]);
  if (s_monHeaderMarker != monHeaderMarker) {
    monheadererror |= (1 << monheadererrorbit++);
    ATH_MSG_WARNING("Problem in monHeaderMarker: " << monHeaderMarker);
  }
  if (s_monCheckPoint != monCheckPoint) {
    monheadererror |= (1 << monheadererrorbit++);
    ATH_MSG_WARNING("Problem in monCheckPoint: " << monCheckPoint);
  }

  if (!compareOrSet(m_nPackets, bswap_32(p[3 + offset]) >> 24, m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);

  if (m_nPackets == 0xFF) {
    m_nPackets = 1;
    m_iPacket = 0;
    m_nWordsPerPacket = (bswap_32(p[3 + offset]) & 0xffffff) / 4.;
  } else {
    if (m_headerDecoded)
      ++m_iPacket;
    else
      m_iPacket = 0;
    if (!compareOrSet(m_iPacket, (bswap_32(p[3 + offset]) >> 16) & 0xf, m_headerDecoded))
      monheadererror |= (1 << monheadererrorbit++);
    m_nWordsPerPacket = (bswap_32(p[3 + offset]) & 0xffff) / 4.;
  }

  if (!compareOrSet(m_monHeaderSize, bswap_32(p[5 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);

  //// for now we require more that 18
  //// we will need to understand how to support different header format with firmware version, but we should minimize changes (only add words).
  if (m_monHeaderSize < 18) {
    ATH_MSG_WARNING(" Mon header size should not be less that 18 : =" << m_monHeaderSize << " are you reading old data file?");
    return 0;
  }

  ATH_MSG_DEBUG(" nPackets: " << m_nPackets << " iPacket: " << m_iPacket << " nWordsPerPacket: " << m_nWordsPerPacket << " monHeaderSize: " << m_monHeaderSize);

  /// now these are taken from the ROD header but the word are still here (maybe we will use them for something else)
  if (!compareOrSet(m_at0typeRec, bswap_32(p[9 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_at1typeRec, bswap_32(p[12 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  monheadererrorbit += 2;

  if (!compareOrSet(m_at0nBC, bswap_32(p[10 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_at1nBC, bswap_32(p[13 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_at0BC, bswap_32(p[11 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_at1BC, bswap_32(p[14 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_activeSC, bswap_32(p[15 + offset]), m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_nsc1, bswap_32(p[16]) >> 24, m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_nsc2, (bswap_32(p[16]) >> 16) & 0xff, m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_nsc3, (bswap_32(p[16]) >> 8) & 0xff, m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_nsc4, (bswap_32(p[16])) & 0xff, m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_nsc5, bswap_32(p[17]) >> 24, m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);
  if (!compareOrSet(m_nsc6, (bswap_32(p[17]) >> 16) & 0xff, m_headerDecoded))
    monheadererror |= (1 << monheadererrorbit++);

  ATH_MSG_DEBUG(" at0type " << m_at0typeRec << " at1type " << m_at1typeRec << " at0nBC " << m_at0nBC << " at1nBC " << m_at1nBC << " at0BC " << m_at0BC
                            << " at1BC " << m_at1BC << " nsc1 " << m_nsc1 << " nsc2 " << m_nsc2 << " nsc3 " << m_nsc3 << " nsc4 " << m_nsc4 << " nsc5 "
                            << m_nsc5 << " nsc6 " << m_nsc6);

  if (monheadererror) {
    ATH_MSG_WARNING(" consistency error in mon checker at packet " << m_iPacket << " errorbits " << std::hex << monheadererror << std::dec);
  }

  m_headerDecoded = true;
  return m_monHeaderSize + m_nWordsPerPacket + offset;
}

int LArLATOMEDecoder::EventProcess::signEnergy(unsigned int energy) {

  if (energy & (1 << 17))
    return energy - pow(2, 18);
  else
    return energy;
}

unsigned int LArLATOMEDecoder::EventProcess::bytesPerChannel(MonDataType at0, MonDataType at1) {
  unsigned int b = 0;
  if (at0 == MonDataType::Averaged || at0 == MonDataType::AutoCorr)
    return 8;

  //// 2 bytes per stream and 1 bytes for sat/val if energy
  if (at0 != MonDataType::Invalid)
    b += 2;
  if (at1 != MonDataType::Invalid)
    b += 2;
  if (at0 == MonDataType::Energy || at1 == MonDataType::Energy || at0 == MonDataType::SelectedEnergy || at1 == MonDataType::SelectedEnergy)
    ++b;
  return b;
}

void LArLATOMEDecoder::EventProcess::increaseWordShift(unsigned int& wordshift) {
  ++wordshift;
  if (m_packetEnd[m_iPacket] == wordshift) {
    ++m_iPacket;
    wordshift += (m_monHeaderSize + m_monTrailerSize);
    ATH_MSG_DEBUG("switch packet " << m_iPacket << "  " << wordshift);
  }
}

void LArLATOMEDecoder::EventProcess::increaseByteShift(unsigned int& wordshift, unsigned int& byteshift) {
  ++byteshift;
  if (byteshift == 4) {
    increaseWordShift(wordshift);
    byteshift = 0;
  }
}

void LArLATOMEDecoder::EventProcess::decodeByte(unsigned int& byte, unsigned int wordshift, unsigned int byteshift, const uint32_t* p) {
  byte = ((bswap_32(p[wordshift])) >> (8 * (4 - 1 - byteshift))) & 0xff;
}

void LArLATOMEDecoder::EventProcess::decodeWord(unsigned int& word, unsigned int& wordshift, unsigned int& byteshift, const uint32_t* p) {
  unsigned int msb = 0;
  unsigned int lsb = 0;
  decodeByte(msb, wordshift, byteshift, p);
  increaseByteShift(wordshift, byteshift);
  decodeByte(lsb, wordshift, byteshift, p);
  increaseByteShift(wordshift, byteshift);
  word = lsb | (msb << 8);
}

void LArLATOMEDecoder::EventProcess::decodeChannel(unsigned int& wordshift, unsigned int& byteshift, const uint32_t* p, MonDataType at0, MonDataType at1,
                                                   unsigned int& at0Data, unsigned int& at1Data, unsigned int& saturation, bool& at0val, bool& at1val) {

  // the structure of data is always consisting of 2,3,4 or 5 bytes depending on the recipe.
  // If there is an energy it means there is the quality/saturation byte thus odd number of bytes. Otherwise ADC or Energy spans <mostly> over 2 bytes.
  // avoid vectors and loops and bitsets to make it faster (can change later if needed but there is only few configurations so no need to make it infinitely
  // flexible. use unsigned int everywhere for now since we have 18 bits for energy which does not fit in short, cast later.
  unsigned int nbytesPerChannel = bytesPerChannel(at0, at1);
  bool satByte = nbytesPerChannel % 2;  // do we have satbyte

  at0val = false;
  at1val = false;
  at0Data = 0;
  at1Data = 0;
  unsigned int satData = 0;
  saturation = 0;

  unsigned int word1 = 0;
  unsigned int word2 = 0;
  decodeWord(word1, wordshift, byteshift, p);
  if (nbytesPerChannel > 3) {
    decodeWord(word2, wordshift, byteshift, p);
  }
  at0Data = word1 & 0x7fff;
  at0val = word1 & 0x8000;
  if (nbytesPerChannel > 3) {
    at1Data = word2 & 0x7fff;
    at1val = word2 & 0x8000;
  }
  if (satByte) {
    decodeByte(satData, wordshift, byteshift, p);
    increaseByteShift(wordshift, byteshift);
  }

  /// now that we have the basic words we can rearrange the bits in case it is energy, ADC do not need further arrangement
  /// this is assuming energy lsb is at bit[0-2] and energy-id at [5-7]. Make sure this is the case for all recipe in the firmware.
  /// assumptions are in https://its.cern.ch/jira/browse/LDPBFW-2976

  if (at0 == MonDataType::Energy && at1 == MonDataType::SelectedEnergy) {
    at0Data = (at0Data << 3) | (satData & 0x7);
    at1Data = (at1Data << 3) | ((satData & 0x70) >> 4);
    saturation = ((satData & 0x88) == 0x88);
  } else if (at1 == MonDataType::Energy && at0 == MonDataType::SelectedEnergy) {
    at0Data = (at0Data << 3) | ((satData & 0x70) >> 4);
    at1Data = (at1Data << 3) | (satData & 0x7);
    saturation = ((satData & 0x88) == 0x88);
  } else {
    if (at0 == MonDataType::Energy || at0 == MonDataType::SelectedEnergy) {
      at0Data = (at0Data << 3) | (satData & 0x7);
      saturation = (satData & 0x20);
    }
    if (at1 == MonDataType::Energy || at1 == MonDataType::SelectedEnergy) {
      at1Data = (at1Data << 3) | (satData & 0x7);
      saturation = (satData & 0x20);
    }
  }
}

void LArLATOMEDecoder::EventProcess::fillCollection(const ROBFragment* robFrag, const LArLATOMEMapping* map, const LArOnOffIdMapping* onoffmap,
                                                    const LArCalibLineMapping* clmap) {
  // Mon* mon = new Mon;

  /// some of this info should be used in the LatomeHeader class and for cross checks also (same as for the mon header)
  // const unsigned int rod_Size_words = robFrag->rod_ndata();
  // const unsigned int rob_Size_words = robFrag->payload_size_word();
  const unsigned int sourceID = robFrag->rob_source_id();
  // const unsigned int rod_fragment_size_word = robFrag->rod_fragment_size_word();
  // const unsigned int rod_header_size_word = robFrag->rod_header_size_word();
  // const unsigned int rod_trailer_size_word = robFrag->rod_trailer_size_word();
  // const unsigned int rod_bc_id = robFrag->rod_bc_id();
  // const unsigned int rod_nstatus = robFrag->rod_nstatus();
  // const unsigned int rod_status_position = robFrag->rod_status_position();
  // const uint32_t* rod_start = robFrag->rod_start();
  m_l1ID = robFrag->rod_lvl1_id();
  m_ROBFragSize = robFrag->rod_ndata();
  const uint32_t* p = robFrag->rod_data();
  const unsigned int n = robFrag->rod_ndata();
  if (0 == n) {
    /// this is an empty fragment, skip it
    ATH_MSG_DEBUG("Empty fragment, skip ");
    return;
  }
  m_latomeBCID = robFrag->rod_bc_id();
  const uint32_t* rod_status = robFrag->rod_status();
  const unsigned int rod_nstatus = robFrag->rod_nstatus();
  if (rod_nstatus != 27) {
    ATH_MSG_WARNING("Inconsistent number of rod header status elements: nstatus= " << rod_nstatus);
    return;
  }
  if (rod_nstatus > 8) {
    uint32_t status8 = rod_status[8];
    m_at0type = status8 & 0x3;
    m_at1type = (status8 >> 2) & 0x3;
  }
  m_nthLATOME = robFrag->rod_source_id();

  LatomeCalibPatterns pat1, pat2, pat3;
  pat1.DAC = rod_status[9];
  pat1.delay = rod_status[10];
  pat1.patterns.resize(4);
  for (unsigned int i = 0; i < 4; ++i)
    pat1.patterns[i] = rod_status[i + 11];

  pat2.DAC = rod_status[15];
  pat2.delay = rod_status[16];
  pat2.patterns.resize(4);
  for (unsigned int i = 0; i < 4; ++i)
    pat2.patterns[i] = rod_status[i + 17];

  pat3.DAC = rod_status[21];
  pat3.delay = rod_status[22];
  pat3.patterns.resize(4);
  for (unsigned int i = 0; i < 4; ++i)
    pat3.patterns[i] = rod_status[i + 23];

  m_latomeCalibPatternsInEvent = {pat1, pat2, pat3};

  const HWIdentifier hwidEmpty;

  ////lets first decode the mon header in the first packet and get the info.

  unsigned int offset = decodeHeader(p, 0);
  if (offset > m_ROBFragSize) {
    ATH_MSG_WARNING("Data corruption, offset found at pos 0 (" << offset << ") is larger than the ROB fragment size (" << m_ROBFragSize << "). Ignoring data.");
    return;
  }

  if (m_isAveraged) {
    m_at0type = m_at0typeRec >> 24;
    m_at1type = (int)MonDataType::Invalid;
    if (m_at0type != (int)MonDataType::Averaged) {
      ATH_MSG_WARNING(" inconsistant data type with requested averaged decoding at0 type " << m_at0type << " " << m_at0typeRec << " " << std::hex << m_l1ID
                                                                                           << " " << sourceID << std::dec);
      return;
    }
  }
  if (m_isAutoCorr) {
    m_at0type = m_at0typeRec >> 24;
    m_at1type = (int)MonDataType::Invalid;
    if (m_at0type != (int)MonDataType::AutoCorr) {
      ATH_MSG_WARNING(" inconsistant data type with requested averaged decoding at0 type " << m_at0type << " " << m_at0typeRec << " " << std::hex << m_l1ID
                                                                                           << " " << sourceID << std::dec);
      return;
    }
  }

  m_packetEnd.push_back(offset);
  offset = decodeTrailer(p, offset);
  /// not we have the packet size from the first packet, check all packet headers before decoding
  /// we can decide later if we drop decoding if we have inconsistency
  for (unsigned int ip = 1; ip < m_nPackets; ++ip) {
    if (offset > m_ROBFragSize) {
      ATH_MSG_WARNING("Data corruption, offset found at pos 0 (" << offset << ") is larger than the ROB fragment size (" << m_ROBFragSize << "). Ignoring data.");
      return;
    }
    offset = decodeHeader(p, offset);
    if (offset > m_ROBFragSize) {
      ATH_MSG_WARNING("Data corruption, offset found at pos 0 (" << offset << ") is larger than the ROB fragment size (" << m_ROBFragSize << "). Ignoring data.");
      return;
    }
    m_packetEnd.push_back(offset);
    offset = decodeTrailer(p, offset);
  }

  ATH_MSG_DEBUG(" end of header check computed offset=" << std::dec << offset << " nwords in payload=" << n);

  /// OK all headers checked and we have all info we need to decode each packet, so lets start
  m_iPacket = 0;
  if (m_nPackets==0) {
    ATH_MSG_WARNING("Data corruption, nPackets=0");
    return;
  }

  if (m_nPackets>m_packetEnd.size()) {
    ATH_MSG_WARNING("Data corruption, nPackets " << m_nPackets << " exceeds size " << m_packetEnd.size());
    return;
  }
     
  if (m_packetEnd[m_nPackets - 1] + m_monTrailerSize != n) {
    ATH_MSG_WARNING("problem in packet size loop " << m_packetEnd[m_nPackets - 1] << " != " << n);
  }

  std::vector<unsigned int> timeslot_nsc = {m_nsc1, m_nsc2, m_nsc3, m_nsc4, m_nsc5, m_nsc6};
  std::vector<unsigned int> bc_size;
  /// for now just getting the ADCs from AT0
  /// Change later to add energy as well and use the BC shift to determine at which BC there is energy

  short nBC = 0;
  short nBC1 = 0;
  short startBC1 = 0;
  MonDataType type0 = MonDataType::Invalid;
  MonDataType type1 = MonDataType::Invalid;

  if (m_isAveraged || m_isAutoCorr) {
    nBC = m_at0nBC;
    nBC1 = m_at1nBC;
    m_nBC_Averaged = nBC;
    startBC1 = 0;
    type0 = static_cast<MonDataType>(m_at0type);
    type1 = static_cast<MonDataType>(m_at1type);
    for (auto& val : m_averagedRawValuesInEvent) {
      val.sum.resize(m_nBC_Averaged);
      val.sumSq.resize(m_nBC_Averaged);
      val.nTrigValid.resize(m_nBC_Averaged);
      val.latomeChannel = 99999;
    }
  } else {
    if (m_at1nBC == 0) {  /// recipe does not have at1, set at1 mux to invalid
      m_at1type = (Word)MonDataType::Invalid;
    }

    if (m_at0type != (Word)MonDataType::Invalid && m_at1type != (Word)MonDataType::Invalid) {
      if (m_at0nBC >= m_at1nBC) {
        nBC = m_at0nBC;
        nBC1 = m_at1nBC;
        startBC1 = m_at1BC - m_at0BC;
        type0 = static_cast<MonDataType>(m_at0type);
        type1 = static_cast<MonDataType>(m_at1type);
      } else {
        nBC1 = m_at0nBC;
        nBC = m_at1nBC;
        startBC1 = m_at0BC - m_at1BC;
        type1 = static_cast<MonDataType>(m_at0type);
        type0 = static_cast<MonDataType>(m_at1type);
      }
    } else if (m_at0type != (Word)MonDataType::Invalid) {
      nBC = m_at0nBC;
      type0 = static_cast<MonDataType>(m_at0type);
    } else if (m_at1type != (Word)MonDataType::Invalid) {
      nBC = m_at1nBC;
      type0 = static_cast<MonDataType>(m_at1type);
    } else {
      ATH_MSG_ERROR("No valid data type in Mon Header");
      return;
    }

    switch (type0) {
      case MonDataType::RawADC:
        m_hasRawAdc = true;
        m_nBC_rawADC = nBC;
        break;
      case MonDataType::ADC:
        m_hasAdc = true;
        m_nBC_ADC = nBC;
        break;
      case MonDataType::Energy:
        m_hasE = true;
        m_nBC_E = nBC;
        break;
      case MonDataType::SelectedEnergy:
        m_hasEID = true;
        m_nBC_EID = nBC;
        break;
      case MonDataType::Invalid:
        break;
      default:
        ATH_MSG_ERROR("wrong mux0 value " << (Word)type0);
        return;
    }

    switch (type1) {
      case MonDataType::RawADC:
        m_hasRawAdc = true;
        m_nBC_rawADC = nBC1;
        m_BC_rawADC = startBC1;
        break;
      case MonDataType::ADC:
        m_hasAdc = true;
        m_nBC_ADC = nBC1;
        m_BC_ADC = startBC1;
        break;
      case MonDataType::Energy:
        m_hasE = true;
        m_nBC_E = nBC1;
        m_BC_E = startBC1;
        break;
      case MonDataType::SelectedEnergy:
        m_hasEID = true;
        m_nBC_EID = nBC1;
        m_BC_EID = startBC1;
        break;
      case MonDataType::Invalid:
        break;
      default:
        ATH_MSG_ERROR("wrong mux1 value " << (Word)type1);
        return;
    }

    for (auto& val : m_rawValuesInEvent) {
      if (m_hasRawAdc)
        val.adc.resize(m_nBC_rawADC);
      if (m_hasAdc)
        val.adc_bas.resize(m_nBC_ADC);
      if (m_hasE)
        val.et.resize(m_nBC_E);
      if (m_hasEID)
        val.et_id.resize(m_nBC_EID);
      if (m_hasEID || m_hasE) {
        val.saturation.resize(std::max(m_nBC_EID, m_nBC_E));
      }

      val.latomeChannel = 99999;
    }
  }

  m_BCIDsInEvent.resize(nBC);

  unsigned int s = m_monHeaderSize;  /// lets start from here
  unsigned int bcid = s_nBunches;

  for (short iBC = 0; iBC < nBC; ++iBC) {
    MonDataType at0 = type0;
    MonDataType at1 = type1;
    if (type1 != MonDataType::Invalid) {
      if (iBC < startBC1 || iBC >= startBC1 + nBC1)
        at1 = MonDataType::Invalid;
    }
    unsigned int nbytesPerChannel = bytesPerChannel(at0, at1);

    int nsc = 0;
    unsigned int oldipacket = 0;
    for (unsigned int itimeslot = 0; itimeslot < 6; ++itimeslot) {
      unsigned int l_bcid = (bswap_32(p[s]))>>16;
      if(itimeslot!=0){
	if(l_bcid!=bcid){
	  ATH_MSG_WARNING( "ERROR: inconsistent BCID between time slots" );
	}
      }
      else{
	if(bcid!=s_nBunches){ /// start of packet, bcid still unvalid
	  //// should increase by one but take care of rotation at s_nBunches;
	  unsigned int bcid_c = bcid+1;
	  if(bcid_c==s_nBunches){
            bcid=0;
	    bcid_c = 0;
	  }
	  if(bcid_c != l_bcid){
	   ATH_MSG_WARNING( "ERROR: BCID not increasing properly between samples, sourceId: " << m_nthLATOME << " L1ID is: " << m_l1ID << ", BCID is from payload: " << l_bcid << ", expected BCID is: " << bcid_c << ", LATOME channel is: " << nsc );
	  }
	}
	m_BCIDsInEvent[iBC] = l_bcid;
      }
      bcid=l_bcid;

      unsigned int mux = ((bswap_32(p[s])) >> 8) & 0xff;
      increaseWordShift(s);
      //// skip trailing 0
      increaseWordShift(s);
      if (s >= n)
        break;

      unsigned int timeslotsize = timeslot_nsc[itimeslot];
      unsigned int nbytes = timeslotsize * nbytesPerChannel;
      unsigned int n64word = nbytes / 8;  /// the data is packed with 64 bits words. Will be padded with 0s at the end of the timeslot time slot.
      if (nbytes % 8)
        ++n64word;
      ATH_MSG_DEBUG(" at BC " << iBC << " timeslot " << itimeslot << "  " << bcid << "  " << mux << " n64word " << n64word << " at0 " << (int)at0 << " at1 "
                              << (int)at1 << " l_bcid " << bcid);

      unsigned int wordshift = s;
      unsigned int byteshift = 0;

      //// for the padding consistency check, need to know if we skipped a packet.
      oldipacket = m_iPacket;
      for (unsigned int ichan = 0; ichan < timeslotsize; ++ichan) {
        unsigned int at0Data = 0, at1Data = 0, satData = 0;
        bool at0val = false, at1val = false;

        // protection against corrupted bytestream data
        if (nsc > N_LATOME_CHANNELS - 1) {
          break;
          // FIXME: should fill some default data to all channels, because clearly this block is corrupted
        }
        if (!m_isAveraged && !m_isAutoCorr) {
          decodeChannel(wordshift, byteshift, p, at0, at1, at0Data, at1Data, satData, at0val, at1val);
          ATH_MSG_DEBUG(" wordshift " << wordshift << " byteshift " << byteshift << " at0data " << at0Data << " at1Data " << at1Data << " satData " << satData
                                      << " at0val " << at0val << " at1val " << at1val << " nsc " << nsc);

          /// lets fill now, using the older code structure but this should be change to support having energy.
          const auto SCID = map ? map->getChannelID(m_nthLATOME, nsc) : hwidEmpty;
          if (SCID == hwidEmpty) {
            ATH_MSG_DEBUG("No mapping for ch: " << std::dec << nsc);
          }
          int RAWValue0 = -999;
          if (!at0val && SCID != hwidEmpty && at0 != MonDataType::Invalid) {
            ATH_MSG_DEBUG("at0 bad quality bit for SC:" << nsc << " BC " << iBC << " latome " << robFrag->rod_source_id());
          } else {
            //// no need to reinterpret the bits. for ADC there will be an implicit case to short
            RAWValue0 = at0Data;
          }
          int defaultADCValue = -1;
          int defaultEValue = -99999;
          auto& rawValuesInEvent = m_rawValuesInEvent[nsc];
          switch (at0) {
            case MonDataType::RawADC:
              if ((unsigned)iBC < rawValuesInEvent.adc.size()) {
                rawValuesInEvent.adc[iBC] = (at0val) ? RAWValue0 : defaultADCValue;
              }
              break;
            case MonDataType::ADC:
              if ((unsigned)iBC < rawValuesInEvent.adc_bas.size()) {
                rawValuesInEvent.adc_bas[iBC] = (at0val) ? RAWValue0 : defaultADCValue;
              }
              break;
            case MonDataType::Energy:
              if ((unsigned)iBC < rawValuesInEvent.et.size()) {
                rawValuesInEvent.et[iBC] = (at0val) ? signEnergy(RAWValue0) : defaultEValue;
                rawValuesInEvent.saturation[iBC] = satData;
              }
              break;
            case MonDataType::SelectedEnergy:
              if ((unsigned)iBC < rawValuesInEvent.et_id.size()) {
                rawValuesInEvent.et_id[iBC] = (at0val) ? signEnergy(RAWValue0) : defaultEValue;
                rawValuesInEvent.saturation[iBC] = satData;
              }
              break;
            case MonDataType::Invalid:
              break;
            default:
              ATH_MSG_ERROR("wrong at0 value " << (Word)at0);
              return;
          }

          int RAWValue1 = -999;
          if (!at1val && SCID != hwidEmpty && at1 != MonDataType::Invalid) {
            ATH_MSG_DEBUG("at1 bad quality bit for SC:" << nsc << " BC " << iBC << " latome " << robFrag->rod_source_id());
          } else {
            //// no need to reinterpret the bits. for ADC there will be an implicit case to short
            RAWValue1 = at1Data;
          }

          const size_t BCidx = iBC - startBC1;
          switch (at1) {
            case MonDataType::RawADC:
              if (BCidx < rawValuesInEvent.adc.size()) {
                rawValuesInEvent.adc[BCidx] = (at1val) ? RAWValue1 : defaultADCValue;
              }
              break;
            case MonDataType::ADC:
              if (BCidx < rawValuesInEvent.adc_bas.size()) {
                rawValuesInEvent.adc_bas[BCidx] = (at1val) ? RAWValue1 : defaultADCValue;
              }
              break;
            case MonDataType::Energy:
              if (BCidx < rawValuesInEvent.et.size()) {
                rawValuesInEvent.et[BCidx] = (at1val) ? signEnergy(RAWValue1) : defaultEValue;
                rawValuesInEvent.saturation[BCidx] = satData;
              }
              break;
            case MonDataType::SelectedEnergy:
              if (BCidx < rawValuesInEvent.et_id.size()) {
                m_rawValuesInEvent[nsc].et_id[BCidx] = (at1val) ? signEnergy(RAWValue1) : defaultEValue;
                m_rawValuesInEvent[nsc].saturation[BCidx] = satData;
              }
              break;
            case MonDataType::Invalid:
              break;
            default:
              ATH_MSG_ERROR("wrong at1 value " << (Word)at1);
              return;
          }

          m_rawValuesInEvent[nsc].latomeChannel = nsc;

        } else {
          if (wordshift % 2) {
            ATH_MSG_ERROR("inconsistant wordshift in decoding everaged data");
            return;
          }
          unsigned int averageword = bswap_32(p[wordshift]);
          wordshift += 1;
          unsigned int sumSq = bswap_32(p[wordshift]);
          wordshift += 1;
          unsigned long long sumsqMSB = averageword >> 28;
          sumsqMSB = sumsqMSB << 32;

          m_averagedRawValuesInEvent[nsc].sum[iBC] = averageword & 0xFFFFF;
          m_averagedRawValuesInEvent[nsc].sumSq[iBC] = sumSq | sumsqMSB;
          m_averagedRawValuesInEvent[nsc].nTrigValid[iBC] = (averageword >> 20) & 0xFF;
          m_averagedRawValuesInEvent[nsc].latomeChannel = nsc;
        }

        ++nsc;

      }  /// Loop over SC

      //// lets make sure the 0 padding is correct
      if(byteshift!=0){
	  increaseWordShift(wordshift);
	  byteshift=0;
      }
      ATH_MSG_DEBUG("wordshift before: " << wordshift << ", s: " << s);
      if ((wordshift - s) % 2)
        increaseWordShift(wordshift);
      ATH_MSG_DEBUG("wordshift after : " << wordshift << ", s: " << s);
      if ((wordshift - s - ((m_iPacket - oldipacket) * (m_monHeaderSize + m_monTrailerSize))) != n64word * 2) {
        ATH_MSG_WARNING(" ERROR: time slice end is not padded properly " << (wordshift - s - m_iPacket * (m_monHeaderSize + m_monTrailerSize))
                                                                         << "!=" << n64word * 2 << " m_ipacket " << m_iPacket);
      }
      s = wordshift;
      oldipacket = m_iPacket;
    }  /// Loop over time slots

  }  /// Loop over BC

  if (!m_isAveraged && !m_isAutoCorr) {
    fillRaw(map);
  } else {
    if (onoffmap && clmap) {
      fillCalib(map, onoffmap, clmap);
    } else {
      ATH_MSG_ERROR("Do not have mapping !!!");
    }
  }
  fillHeader();
}

void LArLATOMEDecoder::EventProcess::fillCalib(const LArLATOMEMapping* map, const LArOnOffIdMapping* cablingLeg, const LArCalibLineMapping* clcabling) {

  CaloGain::CaloGain gain = CaloGain::LARHIGHGAIN;
  uint32_t DAC_value = 0;
  uint16_t delay_value = 0;
  uint16_t isPulsed_value = false;
  LArCalibParams* calibParams1 = 0;
  LArCalibParams* calibParams2 = 0;

  int pattype = m_nthLATOME >> 16;
  const LArOnOffIdMapping* cabling = 0;
  if (m_caccdigits) {  /// accumulated calib digits requested so decode dac and delay from headers
    m_caccdigits->setDelayScale(25. / 240.);

    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_decoder->m_cablingKeySC};
    cabling = {*cablingHdl};
    if (!cabling) {
      ATH_MSG_ERROR("Do not have mapping object " << m_decoder->m_cablingKeySC.key());
      return;
    }

    calibParams1 = new LArCalibParams();
    calibParams2 = new LArCalibParams();

    if (pattype > 0x48) {
      StatusCode sc1 = calibParams1->initialize();
      StatusCode sc2 = calibParams2->initialize();
      if (sc1 != StatusCode::SUCCESS || sc2 != StatusCode::SUCCESS) {
        ATH_MSG_WARNING("could not initialize LArCalibParams, acc calib will not be filled ");
        delete calibParams1;
        delete calibParams2;
        return;
      }
      if (pattype == 0x49 || pattype == 0x4a) {

        calibParams1->set(1, m_latomeCalibPatternsInEvent[0].patterns, {m_latomeCalibPatternsInEvent[0].DAC}, {m_latomeCalibPatternsInEvent[0].delay});

        calibParams2->set(1, m_latomeCalibPatternsInEvent[2].patterns, {m_latomeCalibPatternsInEvent[2].DAC}, {m_latomeCalibPatternsInEvent[2].delay});
      } else {

        calibParams1->set(1, m_latomeCalibPatternsInEvent[1].patterns, {m_latomeCalibPatternsInEvent[1].DAC}, {m_latomeCalibPatternsInEvent[1].delay});

        calibParams2->set(1, m_latomeCalibPatternsInEvent[2].patterns, {m_latomeCalibPatternsInEvent[2].DAC}, {m_latomeCalibPatternsInEvent[2].delay});
      }
    } else {
      StatusCode sc1 = calibParams1->initialize();
      if (sc1 != StatusCode::SUCCESS) {
        ATH_MSG_WARNING("could not initialize LArCalibParams, acc calib will not be filled ");
        delete calibParams1;
        delete calibParams2;
        return;
      }
      calibParams1->set(1, m_latomeCalibPatternsInEvent[0].patterns, {m_latomeCalibPatternsInEvent[0].DAC}, {m_latomeCalibPatternsInEvent[0].delay});
    }
  }

  const HWIdentifier hwidEmpty;
  for (SuperCell ch = 0; ch < N_LATOME_CHANNELS; ++ch) {
    LArCalibParams* calibParams = 0;
    auto SCID = map ? map->getChannelID(m_nthLATOME, ch) : hwidEmpty;
    if (SCID == hwidEmpty) {
      ATH_MSG_DEBUG("No mapping for ch: " << std::dec << ch);
      continue;
    }
    if (m_decoder->m_ignoreBarrelChannels && m_decoder->m_onlineId->barrel_ec(SCID) == 0)
      continue;
    if (m_decoder->m_ignoreEndcapChannels && m_decoder->m_onlineId->barrel_ec(SCID) == 1)
      continue;

    std::vector<uint64_t> sum;
    std::vector<uint64_t> sum2;
    unsigned int ntmin = 9999999;
    /// scale to the min nt since we have only one nt per channel and not by sample as with raw data
    for (auto nt : m_averagedRawValuesInEvent[ch].nTrigValid) {
      if (nt < ntmin) {
        ntmin = nt;
      }
    }
    sum.resize(m_averagedRawValuesInEvent[ch].sum.size());
    sum2.resize(m_averagedRawValuesInEvent[ch].sum.size());

    for (unsigned int is = 0; is < m_averagedRawValuesInEvent[ch].sum.size(); ++is) {
      double fsum = (double)m_averagedRawValuesInEvent[ch].sum[is] / m_averagedRawValuesInEvent[ch].nTrigValid[is] * ntmin;
      double fsum2 = (double)m_averagedRawValuesInEvent[ch].sumSq[is] / m_averagedRawValuesInEvent[ch].nTrigValid[is] * ntmin;
      sum[is] = round(fsum);
      sum2[is] = round(fsum2);
    }
    if (m_accdigits) {

      LArAccumulatedDigit* accdigi = new LArAccumulatedDigit(SCID, gain, sum, sum2, ntmin);
      m_accdigits->push_back(accdigi);
    }
    if (m_caccdigits) {
      if (pattype == 0x49 || pattype == 0x4a) {
        if (m_decoder->m_onlineId->barrel_ec(SCID) == 0) {
          calibParams = calibParams1;
        } else {
          calibParams = calibParams2;
        }
      } else if (pattype == 0x4b || pattype == 0x4c) {
        if (m_decoder->m_onlineId->isHECchannel(SCID)) {
          calibParams = calibParams1;
        } else {
          calibParams = calibParams2;
        }
      } else {
        calibParams = calibParams1;
      }
      unsigned int eventNb = 0;  ////dummy event number
      unsigned int numCL = 0;
      unsigned int numPulsedLeg = 0;
      std::vector<Identifier> ccellIds(0);
      Identifier myofflineID = cabling->cnvToIdentifier(SCID);
      ccellIds = m_decoder->m_sc2ccMappingTool->superCellToOfflineID(myofflineID);
      for (Identifier id : ccellIds) {  // loop cells in sc
        HWIdentifier cellLegHWID = cablingLeg->createSignalChannelID(id);
        const std::vector<HWIdentifier>& calibLineLeg = clcabling->calibSlotLine(cellLegHWID);
        numCL += calibLineLeg.size();
        for (HWIdentifier calibLineHWID : calibLineLeg) {  // loop legacy calib lines
          if (calibParams->isPulsed(eventNb, calibLineHWID)) {
            numPulsedLeg += 1;
          }
        }
      }

      isPulsed_value = true;
      if (numPulsedLeg != numCL) {
        if (m_decoder->m_keepPulsed)
          continue;
        isPulsed_value = false;
      }
      DAC_value = calibParams->DAC(eventNb, SCID) * numPulsedLeg;
      // Here we want to change the DAC value to reflect the incorrect factors applied in HEC, where some of the cells are much smaller yet have DAC2MeV factors
      // which came from averaging over many cells
      int ft = m_decoder->m_onlineId->feedthrough(SCID);
      int slot = m_decoder->m_onlineId->slot(SCID);
      int channel = m_decoder->m_onlineId->channel(SCID);
      if (m_decoder->m_onlineId->barrel_ec(SCID) == 1 && (ft == 3 || ft == 10 || ft == 16 || ft == 22)) {
        // if it's HEC
        if (slot == 1) {
          if (channel >= 16 && channel <= 31) {  // eta 1.65 bin
            //DAC_value = DAC_value / 1.363; // measured value
            DAC_value = DAC_value / 1.2;   // computed from geometry
            m_decoder->msg(MSG::DEBUG) << "Multiplying DAC for channel " << SCID << "by 1/1.2" << endmsg;
          } else if (channel >= 32 && channel <= 47) {  // eta 1.75 bin
            //DAC_value = DAC_value / 1.206; // measured value
            DAC_value = DAC_value * 7. / 8.; // computed from geometry
            m_decoder->msg(MSG::DEBUG) << "Multiplying DAC for channel " << SCID << "by 7./8." << endmsg;
          }
        }
      }

      delay_value = calibParams->Delay(eventNb, SCID);
      LArAccumulatedCalibDigit* accdigi = new LArAccumulatedCalibDigit(SCID, gain, sum, sum2, ntmin, DAC_value, delay_value, isPulsed_value);

      m_caccdigits->push_back(accdigi);
    }

  }  /// for loop on SCs

  delete calibParams1;
  delete calibParams2;
}

// Pass ADC values from an event
void LArLATOMEDecoder::EventProcess::fillRaw(const LArLATOMEMapping* map) {
  // const CaloGain::CaloGain dummyGain = CaloGain::LARHIGHGAIN;
  const HWIdentifier hwidEmpty;
  for (SuperCell ch = 0; ch < N_LATOME_CHANNELS; ++ch) {
    const auto SCID = map ? map->getChannelID(m_nthLATOME, ch) : hwidEmpty;
    if (SCID == hwidEmpty) {
      ATH_MSG_DEBUG("No mapping for ch: " << std::dec << ch);
      continue;
    }
    if (m_decoder->m_ignoreBarrelChannels && m_decoder->m_onlineId->barrel_ec(SCID) == 0)
      continue;
    if (m_decoder->m_ignoreEndcapChannels && m_decoder->m_onlineId->barrel_ec(SCID) == 1)
      continue;

    std::vector<short> adcValues_inChannel_inEvent;

    if (m_hasRawAdc) {  /// don't copy vectors for nothing
      adcValues_inChannel_inEvent = m_rawValuesInEvent[ch].adc;
    }

    if (m_hasRawAdc && m_adc_coll) {
      /// need to shift the BCID as well
      /// do it at the same time

      std::vector<unsigned short> bcid_in_event;
      for (short b = m_BC_rawADC; b < m_BC_rawADC + m_nBC_rawADC; ++b) {
        bcid_in_event.push_back(m_BCIDsInEvent[b]);
      }

      LArSCDigit* scDigit = new LArSCDigit(SCID, m_rawValuesInEvent[ch].latomeChannel, m_nthLATOME, adcValues_inChannel_inEvent, bcid_in_event);
      m_adc_coll->push_back(scDigit);
    }

    if (m_hasAdc && m_adc_bas_coll) {
      std::vector<unsigned short> bcid_in_event;
      if (m_nBC_ADC == (short)m_BCIDsInEvent.size()) {
        bcid_in_event = m_BCIDsInEvent;
      } else {
        for (short b = m_BC_ADC; b < m_BC_ADC + m_nBC_ADC; ++b) {
          bcid_in_event.push_back(m_BCIDsInEvent[b]);
        }
      }
      LArSCDigit* scDigit = new LArSCDigit(SCID, m_rawValuesInEvent[ch].latomeChannel, m_nthLATOME, m_rawValuesInEvent[ch].adc_bas, bcid_in_event);
      m_adc_bas_coll->push_back(scDigit);
    }

    if (m_hasE && m_et_coll) {
      std::vector<unsigned short> bcid_in_event;
      if (m_nBC_E == (unsigned short)m_BCIDsInEvent.size()) {
        bcid_in_event = m_BCIDsInEvent;
      } else {
        for (short b = m_BC_E; b < m_BC_E + m_nBC_E; ++b) {
          bcid_in_event.push_back(m_BCIDsInEvent[b]);
        }
      }
      LArRawSC* scDigit =
          new LArRawSC(SCID, m_rawValuesInEvent[ch].latomeChannel, m_nthLATOME, m_rawValuesInEvent[ch].et, bcid_in_event, m_rawValuesInEvent[ch].saturation);
      m_et_coll->push_back(scDigit);
    }

    if (m_hasEID && m_et_id_coll) {
      std::vector<unsigned short> bcid_in_event;
      if (m_nBC_EID == (short)m_BCIDsInEvent.size()) {
        bcid_in_event = m_BCIDsInEvent;
      } else {
        for (short b = m_BC_EID; b < m_BC_EID + m_nBC_EID; ++b) {
          bcid_in_event.push_back(m_BCIDsInEvent[b]);
        }
      }
      LArRawSC* scDigit =
          new LArRawSC(SCID, m_rawValuesInEvent[ch].latomeChannel, m_nthLATOME, m_rawValuesInEvent[ch].et_id, bcid_in_event, m_rawValuesInEvent[ch].saturation);
      m_et_id_coll->push_back(scDigit);
    }
  }
}

void LArLATOMEDecoder::EventProcess::fillHeader() {

  if (m_header_coll) {
    LArLATOMEHeader* latome = new LArLATOMEHeader(m_nthLATOME, m_latomeID, m_activeSC, m_latomeBCID, m_l1ID, m_ROBFragSize);
    m_header_coll->push_back(latome);
  }
}
