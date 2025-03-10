/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONNSWCOMMONDECODE_NSWDECODEHELPER_H
#define MUONNSWCOMMONDECODE_NSWDECODEHELPER_H

#include <stdint.h>
#include <stdexcept>

#include <sstream>
#include <span>

namespace Muon
{
  namespace nsw
  {
    enum channel_type
    {
      OFFLINE_CHANNEL_TYPE_PAD = 0,
      OFFLINE_CHANNEL_TYPE_STRIP = 1,
      OFFLINE_CHANNEL_TYPE_WIRE = 2
    };

    enum vmm_channels
    {
      VMM_per_MMFE8 = 8,
      VMM_per_sFEB  = 3,
      VMM_per_pFEB  = 3,
      VMM_channels  = 64
    };

    namespace helper
    {
      uint32_t get_bits (uint32_t word, uint32_t mask, uint8_t position);
      //uint32_t set_bits (uint32_t word, uint32_t mask, uint8_t position);
    }

    // Helper function to replace placeholders in a string                       
    template<typename T>
      std::string format(const std::string& str, const T& arg) {
      std::stringstream ss;
      std::size_t pos = str.find("{}");
      if (pos == std::string::npos) {
	ss << str;
      } else {
	ss << str.substr(0, pos) << arg << str.substr(pos + 2);
      }
      return ss.str();
    }
    
    // Variadic template
    template<typename T, typename... Args>
      std::string format(const std::string& str, const T& arg, const Args&... args) {
      std::stringstream ss;
      std::size_t pos = str.find("{}");
      if (pos == std::string::npos) {
	ss << str;
      } else {
	ss << str.substr(0, pos) << arg << format(str.substr(pos + 2), args...);
      }
      return ss.str();
    }

    /**
     * @brief Decode bits from data of words
     *
     * Start and end included
     *
     * @tparam Target Type of the value that is decoded
     * @tparam Source Type of the words in the data
     * @param words Data
     * @param start Start of value in bits
     * @param end Stop of value in bits
     * @return Target Decoded value
     */
    template <typename Target, typename Source>
      Target bit_slice(const std::span<const Source> words, const std::size_t start, const std::size_t end) {
      // start and end are the positions in the entire stream
      // start and end included;
      const auto wordSize = sizeof(Source) * 8;
      auto s = Target{};
      const auto n = end / wordSize;
      const auto m = start / wordSize;

      if (end < start) {
	throw std::runtime_error(format("End must be larger than start ({} vs {})", start, end));
      }
      if (n >= std::size(words)) {
	throw std::runtime_error(format("End index ({}) out of range (number of words: {}, maximum allowed index: {})", n, std::size(words), std::size(words) - 1));
      }
      if (sizeof(Target) < sizeof(Source) * (n - m + 1)) {
	throw std::runtime_error(format("Target type (size {}) too small to fit result of bit_slice given start {} and end {} and source size {}", sizeof(Target), start, end, sizeof(Source)));
      }

      for (auto i = m; i <= n; ++i) {
	s = (s << wordSize) + words[i];
      }
      s >>= (n + 1) * wordSize - (end + 1);
      // len = end - start + 1
      const Target mask = ((Target{1}) << (end - start + 1)) - 1;
      s &= mask;
      return s;
    }

    /**
     * @brief Decode bits from data of words and advance the read pointer
     *
     * @tparam Target Type of the value that is decoded
     * @tparam Source Type of the words in the data
     * @param words Data
     * @param start Read pointer (start of value in bits), increased by function
     * @param size Number of bits to be decoded
     * @return Target Decoded value
     */
    template <typename Target, typename Source>
      constexpr Target decode_and_advance(const std::span<const Source> words, std::size_t& start, const std::size_t size) {
      const auto res = bit_slice<Target, Source>(words, start, start + size - 1);
      start += size;
      return res;
    }

    /**
     * @brief Decode bits from data of words at read pointer + offset and NOT advance the read pointer
     *
     * @tparam Target Type of the value that is decoded
     * @tparam Source Type of the words in the data
     * @param words Data
     * @param start Read pointer (start of value in bits)
     * @param offset decoding begins at read pointer + offset
     * @param size Number of bits to be decoded
     * @return Target Decoded value
     */
    template <typename Target, typename Source>
      constexpr Target decode_at_loc(const std::span<const Source> words, std::size_t& start, const int offset, const std::size_t size) {
      const auto res = bit_slice<Target, Source>(words, start + offset, start + size + offset - 1);
      return res;
    }

    /// @brief Returns the most left hand bit which is set in a number
    /// @tparam T Any built-in data type
    /// @param number value
    /// @return  Set bit. -1 if no bit is set
    template <class T>
      constexpr int8_t max_bit(const T &number) {
      constexpr int8_t num_bits = sizeof(number) * 8 - 1;
      for (int8_t bit = num_bits; bit >= 0; --bit) {
	if (number & (1u << bit))
	  return bit;
      }
      return -1;
    }
    /// @brief Returns the most right hand bit which is set in a number
    /// @tparam T  Any built-in data type
    /// @param number value
    /// @return Position of the bit. -1 if no bit is set
    template <class T>
      constexpr int8_t min_bit(const T &number) {
      constexpr int8_t num_bits = sizeof(number) * 8 - 1;  
      for (size_t bit = 0; bit <= num_bits; ++bit) {
	if (number & (1u << bit))
	  return bit;
      }
      return -1;
    }
    template <class Out>
      constexpr Out fill_bitmask(const uint8_t first_bit,  const uint8_t num_bits) {
      Out mask {0};
      for (uint8_t bit = first_bit; bit <= first_bit + num_bits ; ++bit){
	mask |= (1<<bit);
      }
      return mask;
    }


    uint16_t get_16bxor_crc(const uint32_t * dataPointer, uint32_t dataSize);

  }

}

inline uint32_t Muon::nsw::helper::get_bits (uint32_t word, uint32_t mask, uint8_t position)
{
  return (word >> position) & mask;
}


inline uint16_t Muon::nsw::get_16bxor_crc(const uint32_t * dataPointer, uint32_t dataSize) {

  uint16_t crc = 0;
  // checking if last 16b are 0 padding (possibly added by swROD)
  bool hasPadding = (dataPointer[dataSize-1] & 0xFFFF)==0; // unsafe if the CRC can be 0x0000 but it should never be the case
  // one could simply cast the (std::uint32_t*) to a (std::uint16_t*) 
  // but then we might need to handle the word swap (system dependent)
  for (uint32_t i = 0; i<dataSize-1; ++i) {
    crc = crc ^ ((dataPointer[i] >> 16) & 0xFFFF);
    crc = crc ^ ((dataPointer[i] >>  0) & 0xFFFF);
  }
  // last word can be: 1) 16b CRC + 16b 0padding 2) 16b of valida data + 16b CRC
  // need to handle case 2) 
  if (!hasPadding) {crc = crc ^ ((dataPointer[dataSize-1] >> 16) & 0xFFFF);}
  
  return crc;    
}

//inline uint32_t Muon::nsw::helper::set_bits (uint32_t word, uint32_t setbits, uint32_t mask)
//{
//  return word; //TODO 
//}

#endif // not MUONNSWCOMMONDECODE_NSWDECODEHELPER_H
