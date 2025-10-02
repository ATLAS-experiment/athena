/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ers/ers.h"
#include "MuonNSWCommonDecode/NSWPadTriggerL1a.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <exception>

Muon::nsw::NSWPadTriggerL1a::NSWPadTriggerL1a(const uint32_t* bs, const uint32_t remaining):
  NSWTriggerElink(bs, remaining)
{
  std::vector<uint32_t> words(bs, bs + remaining);

  if (checkSize(words)) {
    return;
  }

  // NSW header
  m_decoded.flags  = getFlags(words);
  m_decoded.ec     = getEc(words);
  m_decoded.secid  = getSecid(words);
  m_decoded.fragid = getFragid(words);
  m_decoded.spare  = getSpare(words);
  m_decoded.orbit  = getOrbit(words);
  m_decoded.bcid   = getBcid(words);
  m_decoded.l1id   = getL1id(words);

  // orbit
  m_decoded.orbitid = getOrbitid(words);
  m_decoded.orbit1  = getOrbit1(words);
  if (m_decoded.orbit1 != 1) {
    return;
  }

  // debug
  ERS_DEBUG(1, "flags:   " << m_decoded.flags);
  ERS_DEBUG(1, "ec:      " << m_decoded.ec);
  ERS_DEBUG(1, "secid:   " << m_decoded.secid);
  ERS_DEBUG(1, "fragid:  " << m_decoded.fragid);
  ERS_DEBUG(1, "spare:   " << m_decoded.spare);
  ERS_DEBUG(1, "orbit:   " << m_decoded.orbit);
  ERS_DEBUG(1, "bcid:    " << m_decoded.bcid);
  ERS_DEBUG(1, "l1id:    " << m_decoded.l1id);
  ERS_DEBUG(1, "orbitid: " << m_decoded.orbitid);
  ERS_DEBUG(1, "orbit1:  " << m_decoded.orbit1);

  // CRC is in the last 16b of data, if it's there, but need to consider that swROD can do 0 padding
  // NB: the rest of the bystream decoder has been design before the CRC was introduced
  //     this means that if CRC is there, we need to read it and remove it from the bitstream
  //     so that the rest of the decoder can work as usual
  if (m_decoded.spare & 0x1) {
    std::span<const std::uint32_t> data{bs, remaining};
    bool hasPadding = (data[data.size()-1] & 0xFFFF)==0; // unsafe if the CRC can be 0x0000 but it should never be the case
    std::size_t bitIndex;
    if (hasPadding) {
      // we have to read the first 16b of the last word of the data: XXXX0000
      bitIndex = (remaining-1) * Muon::nsw::Constants::N_BITS_IN_WORD32;
    } else {
      // we have to read the last 16b of the last word of the data: 0000XXXX
      bitIndex = remaining * Muon::nsw::Constants::N_BITS_IN_WORD32 - 16;
    }
    m_decoded.crc = Muon::nsw::decode_and_advance<uint64_t>(data, bitIndex, 16);
    m_computedCRC = computeCRC(bs, remaining);
    ERS_DEBUG(1, "CRC:          " << m_decoded.crc);
    ERS_DEBUG(1, "Computed CRC: " << m_computedCRC);
    ERS_DEBUG(1, "CRC ok:       " << (m_decoded.crc == m_computedCRC));

    // now we have to rebuild "words" so that it does not contain the CRC (handling the padding eventually)
    // is the CRC was present and there was padding, we simply have to remove the last word of "words"
    // if the CRC was present and there was no padding, we have to replace the last 16b of the last word with 0x0000
    // if the CRC was not present, do nothing
    if (hasPadding) {
      words.pop_back();
    } else {
      words.back() &= 0xFFFF0000; // set the last 16b to 0
    }
  }


  // null
  if (isNullPayload(words)) {
    return;
  }

  // decompression
  auto relbcid = uint32_t{0};
  auto bitIndex = Muon::nsw::Constants::FIRSTBIT_COMPRESSION;
  const auto& mapPadTriggerToTds = (isLarge(m_decoded.secid)) ? m_mapPadTriggerToTdsL : m_mapPadTriggerToTdsS;
  const auto& numberOfChannels   = (isLarge(m_decoded.secid)) ? m_numberOfChannelsL   : m_numberOfChannelsS;
  m_decoded.data.clear();
  while (true) {
    try {
      m_decoded.data.push_back(
        getOneBcOfCompressedData(words, bitIndex, relbcid, mapPadTriggerToTds, numberOfChannels)
      );
    } catch (const std::exception& ex) {
      ERS_DEBUG(1, ex.what());
      m_decoded.data.clear();
      break;
    }
    if (m_decoded.data.back().last) {
      m_decoded.status = m_decoded.data.back().status;
      break;
    }
    bitIndex = m_decoded.data.back().bitIndex;
    relbcid += 1;
  }

  // flattening
  for (const auto& databc: m_decoded.data) {

    m_bcid_rel.push_back(databc.relbcid);
    m_bcid_status.push_back(databc.status);
    m_bcid_multzero.push_back(databc.mult0);
    m_bcid_multiplicity.push_back(databc.multiplicity);
    m_bcid_n++;

    for (size_t trig = 0; trig < databc.bandids.size(); trig++) {
      m_trigger_relbcid.push_back(databc.relbcid);
      m_trigger_bandid.push_back(databc.bandids.at(trig));
      m_trigger_phiid.push_back(databc.phiids.at(trig));
      m_trigger_n++;
    }

    for (size_t pfeb = 0; pfeb < databc.hits.size(); pfeb++) {
      m_pfeb_addr.push_back(pfeb);
      m_pfeb_nchan.push_back(databc.nchans.at(pfeb));
      m_pfeb_disconnected.push_back(databc.disconnecteds.at(pfeb));
      m_pfeb_n++;
    }

    for (size_t pfeb = 0; pfeb < databc.hits.size(); pfeb++) {
      for (size_t it = 0; it < databc.hits.at(pfeb).size(); ++it) {
        m_hit_relbcid.push_back(databc.relbcid);
        m_hit_pfeb.push_back(pfeb);
        m_hit_tdschannel.push_back(databc.tdschannels.at(pfeb).at(it));
        m_hit_vmmchannel.push_back(databc.tdschannels.at(pfeb).at(it) % Muon::nsw::Constants::N_CHAN_PER_VMM);
        m_hit_vmm       .push_back(databc.tdschannels.at(pfeb).at(it) / Muon::nsw::Constants::N_CHAN_PER_VMM);
        m_hit_padchannel.push_back(databc.hits.at(pfeb).at(it));
        m_hit_n++;
      }
    }
  }
}

uint32_t Muon::nsw::NSWPadTriggerL1a::checkSize(const std::vector<uint32_t>& words) const
{
  if (words.size() < Muon::nsw::Constants::SIZE_HEADER_BYTES) {
    return 1;
  }
  return 0;
}

bool Muon::nsw::NSWPadTriggerL1a::isNullPayload(const std::vector<uint32_t>& words) const
{
  constexpr std::array<uint32_t, 3> nulls{0x0, 0xf000, 0xf0000000};
  constexpr size_t padding = 1;
  const auto first = words.begin()
    + Muon::nsw::Constants::SIZE_HEADER_BYTES / Muon::nsw::Constants::N_BYTES_IN_WORD32
    + padding;
  const auto last  = words.end() - padding;
  return std::all_of(first, last, [&nulls](const uint32_t word){
    return std::find(nulls.begin(), nulls.end(), word) != nulls.end();
  });
}

uint32_t Muon::nsw::NSWPadTriggerL1a::lastValidByteIndex(const std::vector<uint32_t>& words) const
{
  return words.size() * Muon::nsw::Constants::N_BYTES_IN_WORD32
    - Muon::nsw::Constants::SIZE_STATUS / Muon::nsw::Constants::N_BITS_IN_BYTE
    - Muon::nsw::Constants::SIZE_TRAILER / Muon::nsw::Constants::N_BITS_IN_BYTE;
}

uint32_t Muon::nsw::NSWPadTriggerL1a::roundUpIfOdd(const uint32_t nbytes) const
{
  if (nbytes % 2 == 0) {
    return nbytes;
  }
  return nbytes + 1;
}

Muon::nsw::NSWPadTriggerL1a::OneBCOfData
Muon::nsw::NSWPadTriggerL1a::getOneBcOfCompressedData(const std::vector<uint32_t>& words,
                                                       const uint32_t bitIndex,
                                                       const uint32_t relbcid,
                                                       const Muon::nsw::Constants::ArrayOfPfebChannels& mapPadTriggerToTds,
                                                       const Muon::nsw::Constants::ArrayOfPfebs& numberOfChannels) const
{

  // decode the data
  Muon::nsw::NSWPadTriggerL1a::OneBCOfData bcData;
  bcData.relbcid       = relbcid;
  bcData.multiplicity  = getMultiplicity(words, bitIndex);
  bcData.mult0         = getMult0(words, bitIndex);
  bcData.phiid0        = getPhiid0(words, bitIndex);
  bcData.phiids        = getPhiids(words, bitIndex);
  bcData.bandids       = getBandids(words, bitIndex);
  bcData.l0size        = getL0size(words, bitIndex);
  bcData.l1size        = getL1size(words, bitIndex);
  bcData.l2            = getL2(words, bitIndex);
  bcData.l1            = getL1(words, bitIndex, bcData.l1size);
  bcData.l0            = getL0(words, bitIndex, bcData.l1size, bcData.l0size);
  bcData.status        = 0;
  bcData.bytemap       = getBytemap(bcData.l2, bcData.l1, bcData.l0);
  bcData.bitmaps       = getBitmaps(bcData.bytemap, numberOfChannels);
  bcData.disconnecteds = getDisconnecteds(bcData.bitmaps);
  bcData.hits          = getHits(bcData.bitmaps);
  bcData.tdschannels   = getTdsChannels(bcData.hits, mapPadTriggerToTds);
  bcData.nchans.clear();
  for (const auto& bitmap: bcData.bitmaps) {
    bcData.nchans.push_back(bitmap.size());
  }

  // clear disconnected pfebs
  for (size_t pfeb = 0; pfeb < bcData.disconnecteds.size(); pfeb++) {
    if (bcData.disconnecteds.at(pfeb)) {
      bcData.hits.at(pfeb).clear();
      bcData.tdschannels.at(pfeb).clear();
    }
  }

  // move the bitIndex and byteIndex forward
  bcData.bitIndex = bitIndex;
  bcData.bitIndex += Muon::nsw::Constants::FIRSTBIT_L1;
  bcData.byteIndex = bcData.bitIndex / Muon::nsw::Constants::N_BITS_IN_BYTE;
  bcData.byteIndex += roundUpIfOdd(bcData.l1size);
  bcData.byteIndex += roundUpIfOdd(bcData.l0size);
  bcData.bitIndex = bcData.byteIndex * Muon::nsw::Constants::N_BITS_IN_BYTE;
  bcData.last = bcData.byteIndex == lastValidByteIndex(words) or
                bcData.byteIndex == lastValidByteIndex(words) - Muon::nsw::Constants::PAD_TO_WORD32;
  if (bcData.byteIndex > lastValidByteIndex(words)) {
    throw std::runtime_error("byteIndex > lastValidByteIndex");
  }
  if (bcData.last) {
    bcData.status = getStatus(words, bitIndex);
  }

  // debug
  ERS_DEBUG(1, "------------");
  ERS_DEBUG(1, "relbcid:      " << bcData.relbcid);
  ERS_DEBUG(1, "multiplicity: " << bcData.multiplicity);
  ERS_DEBUG(1, "mult0:        " << bcData.mult0);
  ERS_DEBUG(1, "phiid0:       " << bcData.phiid0);
  for (const auto& phiid: bcData.phiids) {
    std::ignore = phiid;
    ERS_DEBUG(1, "phiid:        " << phiid);
  }
  for (const auto& bandid: bcData.bandids) {
    std::ignore = bandid;
    ERS_DEBUG(1, "bandid:       " << bandid);
  }
  ERS_DEBUG(1, "l0size:       " << bcData.l0size);
  ERS_DEBUG(1, "l1size:       " << bcData.l1size);
  ERS_DEBUG(1, "l2:           " << bcData.l2);
  ERS_DEBUG(1, "l1.size():    " << bcData.l1.size());
  for (const auto& l1: bcData.l1) {
    std::ignore = l1;
  }
  ERS_DEBUG(1, "l0.size(): " << bcData.l0.size());
  for (const auto& l0: bcData.l0) {
    std::ignore = l0;
  }
  ERS_DEBUG(1, "status: " << bcData.status);
  ERS_DEBUG(1, "bytemap.size(): " << bcData.bytemap.size());
  for (const auto& byte: bcData.bytemap) {
    std::ignore = byte;
  }
  for (const auto& bitmap: bcData.bitmaps) {
    std::stringstream str;
    for (auto it = bitmap.crbegin(); it != bitmap.crend(); ++it) {
      str << int{*it};
    }
    ERS_DEBUG(1, "bitmap: " << str.str());
  }
  {
    std::stringstream str;
    for (auto it = bcData.disconnecteds.crbegin(); it != bcData.disconnecteds.crend(); ++it) {
      str << int{*it};
    }
    ERS_DEBUG(1, "disconnected: " << str.str());
  }
  for (const auto& hits: bcData.hits) {
    std::stringstream str;
    for (const auto& hit: hits) {
      str << hit;
    }
    ERS_DEBUG(1, "hits: " << str.str());
  }

  return bcData;
}

uint8_t Muon::nsw::NSWPadTriggerL1a::getByte(const std::vector<uint32_t>& words,
                                              size_t byteIndex) const
{
  const auto word = words.at(byteIndex / Muon::nsw::Constants::N_BYTES_IN_WORD32);
  const auto idx = byteIndex % Muon::nsw::Constants::N_BYTES_IN_WORD32;
  switch (idx) {
  case 0:
    return (word & 0x00ff0000) >> (2 * Muon::nsw::Constants::N_BITS_IN_BYTE);
  case 1:
    return (word & 0xff000000) >> (3 * Muon::nsw::Constants::N_BITS_IN_BYTE);
  case 2:
    return (word & 0x000000ff) >> (0 * Muon::nsw::Constants::N_BITS_IN_BYTE);
  case 3:
    return (word & 0x0000ff00) >> (1 * Muon::nsw::Constants::N_BITS_IN_BYTE);
  default:
    throw std::runtime_error("getByte failure");
  }
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getBitsAsWord32(const std::vector<uint32_t>& words,
                                                       size_t firstBit, size_t lastBit) const
{
  const auto bits = getBits(words, firstBit, lastBit);
  auto word = uint32_t{0};
  for (size_t it = 0; it < bits.size(); it++) {
    word += (bits.at(it) << it);
  }
  return word;
}

std::vector<uint8_t> Muon::nsw::NSWPadTriggerL1a::getBits(const std::vector<uint32_t>& words,
                                                           size_t firstBit, size_t lastBit) const
{
  if (firstBit > lastBit) {
    throw std::runtime_error("Order of getBits args is wrong");
  }
  const auto firstByte    = firstBit / Muon::nsw::Constants::N_BITS_IN_BYTE;
  const auto firstBytePos = firstBit % Muon::nsw::Constants::N_BITS_IN_BYTE;
  const auto lastByte     = lastBit  / Muon::nsw::Constants::N_BITS_IN_BYTE;
  const auto lastBytePos  = lastBit  % Muon::nsw::Constants::N_BITS_IN_BYTE;
  std::vector<uint8_t> bits{};
  bits.reserve(lastBit - firstBit + 1);
  for (auto byteIndex = firstByte; byteIndex < lastByte + 1; byteIndex++) {
    const auto byte = getByte(words, byteIndex);
    for (size_t bytePos = 0; bytePos < Muon::nsw::Constants::N_BITS_IN_BYTE; bytePos++) {
      if (byteIndex == firstByte and bytePos < firstBytePos) {
        continue;
      }
      if (byteIndex == lastByte and bytePos >= lastBytePos) {
        break;
      }
      bits.push_back((byte >> bytePos) & 0b1);
    }
  }
  return bits;
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getFragid(const std::vector<uint32_t>& words) const
{
  return getBitsAsWord32(words,
                         Muon::nsw::Constants::FIRSTBIT_FRAGID,
                         Muon::nsw::Constants::FIRSTBIT_FRAGID + Muon::nsw::Constants::SIZE_FRAGID);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getSecid(const std::vector<uint32_t>& words) const
{
  return getBitsAsWord32(words,
                         Muon::nsw::Constants::FIRSTBIT_SECID,
                         Muon::nsw::Constants::FIRSTBIT_SECID + Muon::nsw::Constants::SIZE_SECID);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getFlags(const std::vector<uint32_t>& words) const
{
  return getBitsAsWord32(words,
                         Muon::nsw::Constants::FIRSTBIT_FLAGS,
                         Muon::nsw::Constants::FIRSTBIT_FLAGS + Muon::nsw::Constants::SIZE_FLAGS);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getEc(const std::vector<uint32_t>& words) const
{
  return getBitsAsWord32(words,
                         Muon::nsw::Constants::FIRSTBIT_EC,
                         Muon::nsw::Constants::FIRSTBIT_EC + Muon::nsw::Constants::SIZE_EC);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getSpare(const std::vector<uint32_t>& words) const
{
  return getBitsAsWord32(words,
                         Muon::nsw::Constants::FIRSTBIT_SPARE,
                         Muon::nsw::Constants::FIRSTBIT_SPARE + Muon::nsw::Constants::SIZE_SPARE);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getOrbit(const std::vector<uint32_t>& words) const
{
  return getBitsAsWord32(words,
                         Muon::nsw::Constants::FIRSTBIT_ORBIT,
                         Muon::nsw::Constants::FIRSTBIT_ORBIT + Muon::nsw::Constants::SIZE_ORBIT);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getBcid(const std::vector<uint32_t>& words) const
{
  return getBitsAsWord32(words,
                         Muon::nsw::Constants::FIRSTBIT_BCID,
                         Muon::nsw::Constants::FIRSTBIT_BCID + Muon::nsw::Constants::SIZE_BCID);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getL1id(const std::vector<uint32_t>& words) const
{
  const auto l1id_31_16 = getBitsAsWord32(words,
                                          Muon::nsw::Constants::FIRSTBIT_L1ID_31_16,
                                          Muon::nsw::Constants::FIRSTBIT_L1ID_31_16 + Muon::nsw::Constants::SIZE_L1ID_31_16);
  const auto l1id_15_00 = getBitsAsWord32(words,
                                          Muon::nsw::Constants::FIRSTBIT_L1ID_15_00,
                                          Muon::nsw::Constants::FIRSTBIT_L1ID_15_00 + Muon::nsw::Constants::SIZE_L1ID_15_00);
  return (l1id_31_16 << Muon::nsw::Constants::SIZE_L1ID_31_16) + l1id_15_00;
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getOrbitid(const std::vector<uint32_t>& words) const
{
  if (not m_hasOrbit) {
    return 0;
  }
  return getBitsAsWord32(words,
                         Muon::nsw::Constants::FIRSTBIT_ORBITID,
                         Muon::nsw::Constants::FIRSTBIT_ORBITID + Muon::nsw::Constants::SIZE_ORBITID);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getOrbit1(const std::vector<uint32_t>& words) const
{
  if (not m_hasOrbit) {
    return 0;
  }
  return getBitsAsWord32(words,
                         Muon::nsw::Constants::FIRSTBIT_ORBIT1,
                         Muon::nsw::Constants::FIRSTBIT_ORBIT1 + Muon::nsw::Constants::SIZE_ORBIT1);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getMultiplicity(const std::vector<uint32_t>& words,
                                                       const uint32_t bitIndex) const
{
  return getBitsAsWord32(words,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_MULT,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_MULT + Muon::nsw::Constants::SIZE_MULT);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getMult0(const std::vector<uint32_t>& words,
                                                const uint32_t bitIndex) const
{
  return getBitsAsWord32(words,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_MULT0,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_MULT0 + Muon::nsw::Constants::SIZE_MULT0);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getPhiid0(const std::vector<uint32_t>& words,
                                                 const uint32_t bitIndex) const
{
  return getBitsAsWord32(words,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_PHIID0,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_PHIID0 + Muon::nsw::Constants::SIZE_PHIID0);
}

std::vector<uint32_t> Muon::nsw::NSWPadTriggerL1a::getPhiids(const std::vector<uint32_t>& words,
                                                              const uint32_t bitIndex) const
{
  const auto word_23_16 = getBitsAsWord32(words,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_PHIID_23_16,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_PHIID_23_16 + Muon::nsw::Constants::SIZE_PHIID_23_16);
  const auto word_15_00 = getBitsAsWord32(words,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_PHIID_15_00,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_PHIID_15_00 + Muon::nsw::Constants::SIZE_PHIID_15_00);
  uint32_t word_23_00 = (word_23_16 << Muon::nsw::Constants::N_BITS_IN_WORD16) + word_15_00;
  std::vector<uint32_t> phiids{};
  phiids.reserve(Muon::nsw::Constants::N_SEGMENTS_IN_BC);
  for (size_t it = 0; it < Muon::nsw::Constants::N_SEGMENTS_IN_BC; it++) {
    phiids.push_back( (word_23_00 >> (it * Muon::nsw::Constants::N_BITS_IN_PHIID)) & Muon::nsw::Constants::BITMASK_PHIID );
  }
  return phiids;
}

std::vector<uint32_t> Muon::nsw::NSWPadTriggerL1a::getBandids(const std::vector<uint32_t>& words,
                                                               const uint32_t bitIndex) const
{
  const auto word_31_16 = getBitsAsWord32(words,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_BANDID_31_16,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_BANDID_31_16 + Muon::nsw::Constants::SIZE_BANDID_31_16);
  const auto word_15_00 = getBitsAsWord32(words,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_BANDID_15_00,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_BANDID_15_00 + Muon::nsw::Constants::SIZE_BANDID_15_00);
  uint32_t word_31_00 = (word_31_16 << Muon::nsw::Constants::N_BITS_IN_WORD16) + word_15_00;
  std::vector<uint32_t> bandids{};
  bandids.reserve(Muon::nsw::Constants::N_SEGMENTS_IN_BC);
  for (size_t it = 0; it < Muon::nsw::Constants::N_SEGMENTS_IN_BC; it++) {
    bandids.push_back( (word_31_00 >> (it * Muon::nsw::Constants::N_BITS_IN_BANDID)) & Muon::nsw::Constants::BITMASK_BANDID );
  }
  return bandids;
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getL0size(const std::vector<uint32_t>& words,
                                                 const uint32_t bitIndex) const
{
  return getBitsAsWord32(words,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_L0SIZE,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_L0SIZE + Muon::nsw::Constants::SIZE_L0SIZE);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getL1size(const std::vector<uint32_t>& words,
                                                 const uint32_t bitIndex) const
{
  return getBitsAsWord32(words,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_L1SIZE,
                         bitIndex + Muon::nsw::Constants::FIRSTBIT_L1SIZE + Muon::nsw::Constants::SIZE_L1SIZE);
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getL2(const std::vector<uint32_t>& words,
                                             const uint32_t bitIndex) const
{
  const auto word_31_16 = getBitsAsWord32(words,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_L2_31_16,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_L2_31_16 + Muon::nsw::Constants::SIZE_L2_31_16);
  const auto word_15_00 = getBitsAsWord32(words,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_L2_15_00,
                                          bitIndex + Muon::nsw::Constants::FIRSTBIT_L2_15_00 + Muon::nsw::Constants::SIZE_L2_15_00);
  return (word_31_16 << Muon::nsw::Constants::N_BITS_IN_WORD16) + word_15_00;
}

std::vector<uint32_t> Muon::nsw::NSWPadTriggerL1a::getL1(const std::vector<uint32_t>& words,
                                                          const uint32_t bitIndex,
                                               const uint32_t l1size
                                               ) const
{
  if (bitIndex % Muon::nsw::Constants::N_BITS_IN_BYTE != 0) {
    throw std::runtime_error("L1 bit boundary wrong");
  }
  const auto byteIndexFirst = (bitIndex + Muon::nsw::Constants::FIRSTBIT_L1) / Muon::nsw::Constants::N_BITS_IN_BYTE;
  const auto byteIndexLast  = byteIndexFirst + roundUpIfOdd(l1size);
  auto l1 = std::vector<uint32_t>();
  for (size_t byteIndex = byteIndexFirst; byteIndex < byteIndexLast; byteIndex++) {
    const auto byteIndexSwap = (byteIndex % 2 == 0) ? (byteIndex + 1) : (byteIndex - 1);
    l1.push_back(getByte(words, byteIndexSwap));
  }
  return l1;
}

std::vector<uint32_t> Muon::nsw::NSWPadTriggerL1a::getL0(const std::vector<uint32_t>& words,
                                                          const uint32_t bitIndex,
                                                          const uint32_t l1size,
                                                          const uint32_t l0size
                                                          ) const
{
  if (bitIndex % Muon::nsw::Constants::N_BITS_IN_BYTE != 0) {
    throw std::runtime_error("L1 bit boundary wrong");
  }
  const auto byteIndexStart = (bitIndex + Muon::nsw::Constants::FIRSTBIT_L1) / Muon::nsw::Constants::N_BITS_IN_BYTE;
  const auto byteIndexFirst = byteIndexStart + roundUpIfOdd(l1size);
  const auto byteIndexLast  = byteIndexFirst + roundUpIfOdd(l0size);
  auto l0 = std::vector<uint32_t>();
  for (size_t byteIndex = byteIndexFirst; byteIndex < byteIndexLast; byteIndex++) {
    const auto byteIndexSwap = (byteIndex % 2 == 0) ? (byteIndex + 1) : (byteIndex - 1);
    l0.push_back(getByte(words, byteIndexSwap));
  }
  return l0;
}

uint32_t Muon::nsw::NSWPadTriggerL1a::getStatus(const std::vector<uint32_t>& words, const uint32_t bitIndex) const
{
  return getBitsAsWord32(words, bitIndex, bitIndex + Muon::nsw::Constants::SIZE_STATUS);
}

std::vector<uint8_t> Muon::nsw::NSWPadTriggerL1a::getBytemap(const uint32_t L2,
                                                              const std::vector<uint32_t>& L1,
                                                              const std::vector<uint32_t>& L0) const
{
  // byte 0 is LSB
  auto theMap = std::vector<uint8_t>();
  size_t indexL1{0};
  size_t indexL0{0};
  for (size_t bitL2 = 0; bitL2 < Muon::nsw::Constants::SIZE_L2; bitL2++) {
    if ((L2 >> bitL2 & 0b1) == 1) {
      const auto cellL1 = L1.at(L1.size() - 1 - indexL1);
      indexL1 += 1;
      for (size_t bitL1 = 0; bitL1 < Muon::nsw::Constants::CELL_SIZE; bitL1++) {
        if ((cellL1 >> bitL1 & 0b1) == 1) {
          theMap.push_back( L0.at(L0.size() - 1 - indexL0) );
          indexL0 += 1;
        } else {
          theMap.push_back(0x0);
        }
      }
    } else {
      for (size_t tmp = 0; tmp < Muon::nsw::Constants::CELL_SIZE; tmp++) {
        theMap.push_back(0x0);
      }
    }
  }
  return theMap;
}

std::vector<uint8_t> Muon::nsw::NSWPadTriggerL1a::getBitmap(const std::vector<uint8_t>& bytemap) const
{
  // byte 0 is LSB
  auto bitmap = std::vector<uint8_t>();
  for (const auto& byte: bytemap) {
    for (size_t bit = 0; bit < Muon::nsw::Constants::N_BITS_IN_BYTE; bit++) {
      bitmap.push_back(byte >> bit & 0b1);
    }
  }
  return bitmap;
}

std::vector< std::vector<uint8_t> >
Muon::nsw::NSWPadTriggerL1a::getBitmaps(const std::vector<uint8_t>& bytemap,
                                        const Muon::nsw::Constants::ArrayOfPfebs& numberOfChannels) const
{
  const auto bitmap = getBitmap(bytemap);
  auto bitmaps = std::vector< std::vector<uint8_t> >();
  size_t index = 0;
  for (size_t pfeb = 0; pfeb < numberOfChannels.size(); pfeb++) {
    const auto nchan = numberOfChannels.at(pfeb);
    bitmaps.push_back( std::vector<uint8_t>(bitmap.begin() + index,
                                            bitmap.begin() + index + nchan) );
    index += nchan;
  }
  return bitmaps;
}

std::vector<bool> Muon::nsw::NSWPadTriggerL1a::getDisconnecteds(const std::vector< std::vector<uint8_t> >& bitmaps) const
{
  auto vec = std::vector<bool>();
  for (const auto& bitmap: bitmaps) {
    vec.push_back(getDisconnected(bitmap));
  }
  return vec;
}

bool Muon::nsw::NSWPadTriggerL1a::getDisconnected(const std::vector<uint8_t>& bitmap) const
{
  auto element = uint8_t{0xff};
  for (size_t it = 0; it < bitmap.size(); it++) {
    const auto next_element = bitmap.at(it);
    if (next_element == element) {
      return false;
    }
    element = next_element;
  }
  if (element == 0xff) {
    throw std::runtime_error("Empty bitmap");
  }
  return true;
}

std::vector< std::vector<uint8_t> >
Muon::nsw::NSWPadTriggerL1a::getHits(const std::vector< std::vector<uint8_t> >& bitmaps) const
{
  auto hits = std::vector< std::vector<uint8_t> >();
  for (const auto& bitmap: bitmaps) {
    auto thesehits = std::vector<uint8_t>();
    for (size_t it = 0; it < bitmap.size(); it++) {
      if (bitmap.at(it) == 1) {
        thesehits.push_back(it);
      }
    }
    hits.push_back(thesehits);
  }
  return hits;
}

std::vector< std::vector<uint8_t> >
Muon::nsw::NSWPadTriggerL1a::getTdsChannels(const std::vector< std::vector<uint8_t> >& hits,
                                             const Muon::nsw::Constants::ArrayOfPfebChannels& mapPadTriggerToTds) const
{
  auto tdschannels = std::vector< std::vector<uint8_t> >();
  for (size_t pfeb = 0; pfeb < hits.size(); pfeb++) {
    auto tdschans = std::vector<uint8_t>();
    for (const auto& hit: hits.at(pfeb)) {
      tdschans.push_back(mapPadTriggerToTds.at(pfeb).at(hit));
    }
    tdschannels.push_back(tdschans);
  }
  return tdschannels;
}

uint16_t Muon::nsw::NSWPadTriggerL1a::computeCRC(const uint32_t* bs, const uint32_t remaining)
{
  return Muon::nsw::get_16bxor_crc(bs+2, remaining-2);
}