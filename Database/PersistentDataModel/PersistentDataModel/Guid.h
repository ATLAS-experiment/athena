/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PERSISTENTDATAMODEL_GUID_H
#define PERSISTENTDATAMODEL_GUID_H

/** @file Guid.h
 *  @brief This file contains the class definition for the Guid class (migrated from POOL).
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include <iosfwd> // for std::ostream
#include <string>
#include <array>
#include <stdexcept>
#include <type_traits>
#include <span>
#include <algorithm>

/** @class Guid
 *  @brief This class provides a encapsulation of a GUID/UUID/CLSID/IID data structure (128 bit number).
 **/
class Guid {
public:
   /// Standard constructor
   constexpr Guid() : m_data1(0U), m_data2(0U), m_data3(0U), m_data4() {}
   /// Standard constructor (With possible initialization)
   explicit Guid(bool assign) : Guid() { if (assign) create(*this); }
   /// Constructor for Guid from string_view
   constexpr Guid(std::string_view s) { fromString(s); }
   //Constructor for const char* -- prevents trying to call bool version
   constexpr Guid(const char *s) { fromString(s); }
   /// Copy constructor
   Guid(const Guid& c) = default;
   /// Magic spaceship operator
   auto operator<=>(const Guid&) const = default;
   bool operator==(const Guid&) const = default;
   bool operator==(std::string_view str) const;

   /// Automatic conversion to string representation
   constexpr void toString(std::span<char, 36> buf, bool uppercase = true) const noexcept;
   constexpr std::string toString(bool uppercase = true) const{
      std::string buf(36, ' ');
      toString(std::span<char, 36>(buf.data(), 36), uppercase);
      return buf;
   }
   static bool isGuid(std::string_view) noexcept;
   /// Automatic conversion from string representation 
   constexpr Guid& fromString(std::string_view s);
   /// NULL-Guid: static class method
   static const Guid& null() noexcept;

   enum GuidGenMethod { GuidGenDefault, GuidGenRandom, GuidGenByTime };
   static const GuidGenMethod m_guidGenMethod;
   /// Checks for POOL_GUID_TIME or POOL_GUID_RANDOM env variables
   static GuidGenMethod initGuidGenMethod();

   /// Create a new Guid
   /// default method is currently Random, can be changed by param, API or environment
   static void create(Guid& guid, GuidGenMethod method = GuidGenDefault);

   /// Allow accessors to member data
   unsigned int data1() const { return m_data1; }
   unsigned short data2() const { return m_data2; }
   unsigned short data3() const { return m_data3; }
   unsigned char data4(unsigned int i) const { if (i < 8) return m_data4[i]; return 0; }

   /// Allow modifiers for member data
   void setData1(unsigned int data) { m_data1 = data; }
   void setData2(unsigned short data) { m_data2 = data; }
   void setData3(unsigned short data) { m_data3 = data; }
   void setData4(unsigned char data, unsigned int i) { if (i < 8) m_data4[i] = data; }

   /// Equality operator
   friend bool operator==( std::string_view str, const Guid& rhs) { return (rhs.operator==(str)); }
   /// Non-equality operator
   friend bool operator!=( std::string_view str, const Guid& rhs) { return !(rhs.operator==(str)); }
   /// Extraction operators
   friend std::ostream& operator<<(std::ostream& os, const Guid& rhs);

private:
   constexpr void setToNull() noexcept;
   unsigned int m_data1{};
   unsigned short m_data2{};
   unsigned short m_data3{};
   std::array<unsigned char,8> m_data4{};
};

constexpr void Guid::setToNull() noexcept {
   m_data1 = 0U;
   m_data2 = 0U;
   m_data3 = 0U;
   m_data4.fill('\0');
}

//This is an unrolled method provided by AI, it should work to ~20ns
constexpr Guid& Guid::fromString(std::string_view sv) {
   // Trim any whitespace
   if(std::is_constant_evaluated() && std::min(sv.find_first_not_of(' '), sv.size()) > 0){
        throw std::runtime_error("Remove spaces from GUID");
   }
   sv.remove_prefix(std::min(sv.find_first_not_of(' '), sv.size()));
   auto last_non_space = sv.find_last_not_of(' ');
   if (last_non_space == std::string_view::npos) {
      sv = {}; // String is empty or all spaces
   } else {
      if(std::is_constant_evaluated() && (sv.size() - last_non_space - 1)!=0){
         throw std::runtime_error("Remove spaces from GUID");
      }
      sv.remove_suffix(sv.size() - last_non_space - 1);
   }

   // Validate format
   if (sv.size() != 36 ||
       sv[8] != '-' || sv[13] != '-' || sv[18] != '-' || sv[23] != '-') {
      setToNull();
      if(std::is_constant_evaluated()){
         throw std::runtime_error("failed to compile time parse GUID");
      }
      return *this;
   }
   bool success = true;
   // Custom constexpr hex parser
   auto parse_hex = [&success](std::string_view part) -> unsigned long long {
      unsigned long long val = 0;
      for (char c : part) {
         val <<= 4; // Multiply by 16
         if (c >= '0' && c <= '9') {
            val += static_cast<unsigned long long>(c - '0');
         } else if (c >= 'a' && c <= 'f') {
            val += static_cast<unsigned long long>(10 + c - 'a');
         } else if (c >= 'A' && c <= 'F') {
            val += static_cast<unsigned long long>(10 + c - 'A');
         } else {
            success = false;
            return 0;
         }
      }
      return val;
   };

   // Parse m_data1 (positions 0-7)
   m_data1 = static_cast<unsigned int>(parse_hex(sv.substr(0, 8)));

   // Parse m_data2 (positions 9-12)
   m_data2 = static_cast<unsigned short>(parse_hex(sv.substr(9, 4)));

   // Parse m_data3 (positions 14-17)
   m_data3 = static_cast<unsigned short>(parse_hex(sv.substr(14, 4)));


   // Parse m_data4 bytes
   int pos = 19;
   for(int i =0; i< 2; i++){
       auto val = parse_hex(sv.substr(pos, 2));
       m_data4[i] = static_cast<unsigned char>(val);
       pos+=2;
   }
   //Skip the dash at pos 23
   pos = 24;
   for(int i =2; i< 8; i++){
       auto val = parse_hex(sv.substr(pos, 2));
       m_data4[i] = static_cast<unsigned char>(val);
       pos+=2;
   }

   if (!success) {
      setToNull();
      if(std::is_constant_evaluated()){
         throw std::runtime_error("failed to compile time parse GUID");
      }
   }
   return *this;
}

//This is an unrolled method provided by ai, is should work to ~20ns
constexpr void Guid::toString(std::span<char, 36> buf, bool uppercase) const noexcept {

    constexpr char lowhex[] = "0123456789abcdef";
    constexpr char uphex[]  = "0123456789ABCDEF";
    const char *hex = uppercase ? uphex : lowhex;

    // Unrolled for m_data1 (8 hex digits)
    unsigned int v1 = m_data1;
    buf[0] = hex[(v1 >> 28) & 0xF];
    buf[1] = hex[(v1 >> 24) & 0xF];
    buf[2] = hex[(v1 >> 20) & 0xF];
    buf[3] = hex[(v1 >> 16) & 0xF];
    buf[4] = hex[(v1 >> 12) & 0xF];
    buf[5] = hex[(v1 >> 8) & 0xF];
    buf[6] = hex[(v1 >> 4) & 0xF];
    buf[7] = hex[v1 & 0xF];
    buf[8] = '-';

    // Unrolled for m_data2 (4 hex digits)
    unsigned short v2 = m_data2;
    buf[9] = hex[(v2 >> 12) & 0xF];
    buf[10] = hex[(v2 >> 8) & 0xF];
    buf[11] = hex[(v2 >> 4) & 0xF];
    buf[12] = hex[v2 & 0xF];
    buf[13] = '-';

    // Unrolled for m_data3 (4 hex digits)
    unsigned short v3 = m_data3;
    buf[14] = hex[(v3 >> 12) & 0xF];
    buf[15] = hex[(v3 >> 8) & 0xF];
    buf[16] = hex[(v3 >> 4) & 0xF];
    buf[17] = hex[v3 & 0xF];
    buf[18] = '-';

    // Unrolled for m_data4[0] and [1] (2+2 hex digits)
    int pos = 19;
    for(int i =0; i<2;i++){
      unsigned char c = m_data4[i];
      buf[pos] = hex[c >> 4];
      buf[pos+1] = hex[c & 0xF];
      pos +=2;
    }
    buf[23] = '-';
    pos = 24;
    // Unrolled for remaining m_data4[2..7] (6 pairs of hex digits)
    for(int i =2; i<8;i++){
      unsigned char c = m_data4[i];
      buf[pos] = hex[c >> 4];
      buf[pos+1] = hex[c & 0xF];
      pos +=2;
    }

}


namespace std {
 template<>
 struct hash<Guid> {
     size_t operator()(const Guid& g) const noexcept {
         // Treat the Guid as a 16-byte array for hashing.
         // Checking the memory layout is contiguous and test for padding at compile time.
         static_assert(sizeof(Guid) == (sizeof(unsigned int) + sizeof(unsigned short)
			    + sizeof(unsigned short) + sizeof(std::array<unsigned char,8>)));
         const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&g);
         size_t hash_value = 0;
         // Simple hash combiner: FNV-1a inspired for 128-bit data
         constexpr size_t fnv_prime = sizeof(size_t) == 8 ? 0x100000001b3ULL : 0x01000193U;
         constexpr size_t fnv_offset = sizeof(size_t) == 8 ? 0xcbf29ce484222325ULL : 0x811c9dc5U;
         hash_value = fnv_offset;
         for (size_t i = 0; i < sizeof(Guid); ++i) {
             hash_value ^= static_cast<size_t>(bytes[i]);
             hash_value *= fnv_prime;
         }
         return hash_value;
     }
    };
}


#endif
