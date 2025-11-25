/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1NSWSIMEXTRAS_H
#define TRIGT1NSWSIMEXTRAS_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <unordered_map>
#include <iomanip>
#include <cmath>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "CxxUtils/StringUtils.h"

/**
 * @publicsection TrigT1NSWSimExtras
 * @brief List of common functions, shared between sTGC and MM codes
 *
 * A list of common functions, mainly used by sTGC Pad Trigger code.
 * They will be extended to MicroMegas code and sTGC Strip Trigger, when available
 **/

namespace NSWL1 {
  // Space
  const std::string SPACE{" "};

  // Underscore
  const std::string UNDERSCORE{"_"};

  // Comma
  const std::string COMMA{","};

  // Semicolon
  const std::string SEMICOLON{";"};

  // VHDL comment
  const std::string VHDLCOMMENT{"--"};

  // Wheels
  const std::vector<std::string> WHEELS{"A", "C"};

  // Number of layers/sectors per wheel
  constexpr uint32_t NLAYERS{8};
  constexpr uint32_t NSECTORS{16};

  // Sector name
  inline std::string sectorName(const std::string& wheel, const uint32_t sec) {
    std::stringstream ss;
    ss << wheel;
    ss << std::setfill('0') << std::setw(2) << (sec + 1);
    return ss.str();
  }

  // Switch to int phiID
  inline int getSignedPhiID(const uint32_t phiid) {
    // 1 bit of sign (0 = positive) followed by 5 bits of phiid
    constexpr size_t nbitsPhi{5};
    constexpr size_t mask{(1 << nbitsPhi) - 1};
    return std::pow(-1, phiid >> nbitsPhi) * (phiid & mask);
  }

  // Pad specific conventions
  namespace PAD {
    // Large - Small
    const std::string LARGE{"large"};
    const std::string SMALL{"small"};

    // Allowed layers
    constexpr std::array<uint32_t, NLAYERS> LAYERS{0, 1, 2, 3, 4, 5, 6, 7};

    // Number of sectors
    constexpr std::array<uint32_t, NSECTORS> SECTORS{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    // Unpacking pad patterns from vhd file (start)
    const std::string PATTERN_TAG{"pad_trigger_pattern_array :="};

    // Unpacking pad patterns from vhd file (our signal that all the useful blocks are done)
    const std::string PATTERN_END{"bandid_tds_numbers"};

    // Expected length of 1 line of pattern info: pfeb0, chan0, ..., pfeb7, chan7, phiid
    constexpr size_t PATTERNLEN{17};

    // Source ID from wheel + sector
    inline uint32_t wheelSectorToSourceID(const char wheel, const uint32_t sector) {
      uint32_t prefix = 0x6d0020;
      if(wheel == 'C') prefix += (1<<16);
      return prefix+sector;
    }

    // Calculate pFEB
    inline uint32_t getpFEBAthena(const int gas_gap, const int multiplet, const int stationEta) {
      const auto layer = (multiplet-1)*4 + (gas_gap-1);
      uint32_t pfeb = (std::abs(stationEta)-1)*8 + layer;
      return pfeb;
    }

    inline std::pair<int,int> getNrowsNcols(const int pfeb, const bool isLarge) {
      if (isLarge) {
        if(pfeb == 0) return std::make_pair(17, 6);
        if(pfeb == 1) return std::make_pair(17, 6);
        if(pfeb == 2) return std::make_pair(16, 7);
        if(pfeb == 3) return std::make_pair(16, 7);
        if(pfeb == 4) return std::make_pair(16, 6);
        if(pfeb == 5) return std::make_pair(16, 6);
        if(pfeb == 6) return std::make_pair(16, 6);
        if(pfeb == 7) return std::make_pair(16, 6);
        if(pfeb == 8) return std::make_pair(14, 4);
        if(pfeb == 9) return std::make_pair(14, 4);
        if(pfeb == 10) return std::make_pair(15, 5);
        if(pfeb == 11) return std::make_pair(15, 5);
        if(pfeb == 12) return std::make_pair(14, 4);
        if(pfeb == 13) return std::make_pair(14, 4);
        if(pfeb == 14) return std::make_pair(14, 4);
        if(pfeb == 15) return std::make_pair(14, 4);
        if(pfeb == 16) return std::make_pair(15, 4);
        if(pfeb == 17) return std::make_pair(15, 4);
        if(pfeb == 18) return std::make_pair(14, 5);
        if(pfeb == 19) return std::make_pair(14, 5);
        if(pfeb == 20) return std::make_pair(13, 4);
        if(pfeb == 21) return std::make_pair(13, 4);
        if(pfeb == 22) return std::make_pair(14, 4);
        if(pfeb == 23) return std::make_pair(14, 4);
      }
      else {
        if(pfeb == 0) return std::make_pair(17, 4);
        if(pfeb == 1) return std::make_pair(17, 4);
        if(pfeb == 2) return std::make_pair(18, 4);
        if(pfeb == 3) return std::make_pair(18, 4);
        if(pfeb == 4) return std::make_pair(17, 4);
        if(pfeb == 5) return std::make_pair(17, 4);
        if(pfeb == 6) return std::make_pair(17, 3);
        if(pfeb == 7) return std::make_pair(17, 3);
        if(pfeb == 8) return std::make_pair(16, 3);
        if(pfeb == 9) return std::make_pair(16, 3);
        if(pfeb == 10) return std::make_pair(15, 3);
        if(pfeb == 11) return std::make_pair(15, 3);
        if(pfeb == 12) return std::make_pair(15, 2);
        if(pfeb == 13) return std::make_pair(15, 2);
        if(pfeb == 14) return std::make_pair(15, 3);
        if(pfeb == 15) return std::make_pair(15, 3);
        if(pfeb == 16) return std::make_pair(13, 3);
        if(pfeb == 17) return std::make_pair(13, 3);
        if(pfeb == 18) return std::make_pair(14, 3);
        if(pfeb == 19) return std::make_pair(14, 3);
        if(pfeb == 20) return std::make_pair(12, 2);
        if(pfeb == 21) return std::make_pair(12, 2);
        if(pfeb == 22) return std::make_pair(13, 3);
        if(pfeb == 23) return std::make_pair(13, 3);
      }
      return std::make_pair(0,0);
    }

    inline std::pair<int,int> getRowColAthena(const int athenaCh) {
      const int arow = (athenaCh-1)%18;
      const int acol = int((athenaCh-1)/18);
      return std::make_pair(arow, acol);
    }

    inline uint32_t getPadchAthena(const int athCh, const int pfeb, const int sector, int gasGap) {
      bool isLarge = (sector % 2 == 0);
      const auto [Nrows, Ncols] = getNrowsNcols(pfeb, isLarge);
      const auto [arow, acol] = getRowColAthena(athCh);
      //row and column counting all begin at 0
      const int ABrow = Nrows -1 -arow;
      const int ABcol = ((gasGap-1)%2 ==0) ? (Ncols-1)-acol : acol;
      return ABrow*Ncols + ABcol;
    }

    // Dummy bandid
    constexpr uint32_t DUMMY_BANDID{0xff};

    /// Dummy pads
    constexpr std::array<uint32_t,8> DUMMYPADS{104,105,110,111,112,113,114,115};

    // Pad trigger ID
    constexpr uint32_t PAD_TRIGGER_ROB{0x20};

    // Pad trigger N(BC) in readout
    constexpr uint32_t PAD_TRIGGER_READOUT_NBC{7};

    // Wheels
    constexpr uint32_t MUON_STGC_ENDCAP_A_SIDE{0x6D};
    constexpr uint32_t MUON_STGC_ENDCAP_C_SIDE{0x6E};

    // Number of pfeb per radius
    constexpr uint32_t NPFEB_PER_RADIUS{NLAYERS};

    // Unpacking a line of pattern info
    constexpr size_t I_PFEB0    = 0;
    constexpr size_t I_PADCHAN0 = 1;
    constexpr size_t I_PFEB1    = 2;
    constexpr size_t I_PADCHAN1 = 3;
    constexpr size_t I_PFEB2    = 4;
    constexpr size_t I_PADCHAN2 = 5;
    constexpr size_t I_PFEB3    = 6;
    constexpr size_t I_PADCHAN3 = 7;
    constexpr size_t I_PFEB4    = 8;
    constexpr size_t I_PADCHAN4 = 9;
    constexpr size_t I_PFEB5    = 10;
    constexpr size_t I_PADCHAN5 = 11;
    constexpr size_t I_PFEB6    = 12;
    constexpr size_t I_PADCHAN6 = 13;
    constexpr size_t I_PFEB7    = 14;
    constexpr size_t I_PADCHAN7 = 15;
    constexpr size_t I_PHIID    = 16;

    /*
     * The PriorityEncoder deals with the cases where we have more than one trigger segment in the same BC.
     * The stripTDS can only deal with one request per BC, so if two triggers have bandIDs that require strips from the same stripTDS, we can only send one of the triggers.
     * The trigger with highest BandID is given priority.
     * This function defines for each BandID a range of forbidden bandIDs. It returns the lower bound of the forbidden range.
     *
     * Ex: {94,84} means if we have a trigger with bandID 94, the forbidden range is 84-94.
     * Any additional trigger with BandID in that range will be ignored.
     *
     * So it would be something like map<triggered bandID, lower bound of the forbidden BandID range>
     */
    inline uint32_t priorityEncoderL(uint32_t bandid) {
      static const std::unordered_map<uint32_t,uint32_t> bandids = {
        {94,84}, {93,84}, {92,84}, {91,84}, {90,82},
        {89,80}, {88,80}, {87,77}, {86,76}, {85,76}, {84,74}, {83,74}, {82,72}, {81,72}, {80,70},
        {79,70}, {78,70}, {77,67}, {76,66}, {75,66}, {74,66}, {73,62}, {72,62}, {71,62}, {70,62},
        {69,62}, {68,62}, {67,62}, {66,62}, {65,54}, {64,54}, {63,54}, {62,54}, {61,51}, {60,51},
        {59,51}, {58,51}, {57,51}, {56,46}, {55,45}, {54,44}, {53,44}, {52,43}, {51,41}, {50,41},
        {49,41}, {48,41}, {47,41}, {46,36}, {45,35}, {44,35}, {43,33}, {42,32}, {41,32}, {40,32},
        {39,32}, {38,32}, {37,32}, {36,32}, {35,24}, {34,24}, {33,24}, {32,22}, {31,22}, {30,22},
        {29,22}, {28,22}, {27,22}, {26,22}, {25,22}, {24,14}, {23,14}, {22,12}, {21,12}, {20,12},
        {19,12}, {18,12}, {17,12}, {16,12}, {15,12}, {14, 6}, {13, 6}, {12, 6}, {11, 6}, {10, 6},
        { 9, 6}, { 8, 6}, { 7, 6}, { 6, 6}};
      auto it = bandids.find(bandid);
      if(it == bandids.end()) return bandid;
      else return it->second;
    }
    inline uint32_t priorityEncoderS(uint32_t bandid) {
      static const std::unordered_map<uint32_t,uint32_t> bandids = {
        {94,85}, {93,85}, {92,85}, {91,80}, {90,80},
        {89,80}, {88,80}, {87,80}, {86,76}, {85,74}, {84,74}, {83,74}, {82,74}, {81,69}, {80,69},
        {79,69}, {78,69}, {77,69}, {76,64}, {75,64}, {74,64}, {73,64}, {72,64}, {71,64}, {70,64},
        {69,64}, {68,57}, {67,57}, {66,57}, {65,57}, {64,53}, {63,53}, {62,53}, {61,53}, {60,53},
        {59,53}, {58,47}, {57,46}, {56,46}, {55,44}, {54,44}, {53,43}, {52,43}, {51,43}, {50,43},
        {49,43}, {48,37}, {47,37}, {46,37}, {45,37}, {44,33}, {43,33}, {42,33}, {41,33}, {40,33},
        {39,33}, {38,33}, {37,33}, {36,25}, {35,25}, {34,25}, {33,22}, {32,22}, {31,22}, {30,22},
        {29,22}, {28,22}, {27,22}, {26,22}, {25,14}, {24,14}, {23,12}, {22,12}, {21,12}, {20,12},
        {19,12}, {18,12}, {17,12}, {16,12}, {15,12}, {14, 6}, {13, 6}, {12, 5}, {11, 5}, {10, 5},
        { 9, 5}, { 8, 5}, { 7, 5}, { 6, 2}, { 5, 2}, { 4, 2}, { 3, 2}, { 2, 2}};
      auto it = bandids.find(bandid);
      if(it == bandids.end()) return bandid;
      else return it->second;
    }

    inline bool isDummyPad(const uint32_t padchan) {
      return std::count(DUMMYPADS.begin(), DUMMYPADS.end(), padchan);
    }

    // Helper functions: contains
    inline bool contains(const std::string& str, const std::string& substr) {
      return str.find(substr) != std::string::npos;
    }
    inline bool contains(const std::vector<std::string>& cont, const std::string& val) {
      return std::count(cont.begin(), cont.end(), val);
    }
    inline bool contains(const std::vector<uint32_t>& cont, const uint32_t val) {
      return std::count(cont.begin(), cont.end(), val);
    }
    inline bool contains(const std::set<uint32_t>& cont, const uint32_t val) {
      return cont.contains(val);
    }

    // Helper functions: replacement
    inline std::string replace(std::string subject, const std::string& search, const std::string& replace) {
      if (subject.empty() or search.empty()) return subject;

      size_t pos = 0;
      while((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
      }
      return subject;
    }

    /* Helper function: parsing line
     * - take a line like this and return the BandID as an integer, i.e.: "constant bandid_91_small_patterns : pad_trigger_pattern_array := ("
     * - Retrieve first object: bandid_91_small_patterns
     * - Split the object in its components: bandid, 91, small, patterns
     * - Now save the number: 91
     */
    inline uint32_t parseLineForBandid(const std::string& line) {
      const auto split = CxxUtils::tokenize(line, SPACE);
      const auto word = split.at(1);
      const auto word_split = CxxUtils::tokenize(line, UNDERSCORE);
      return static_cast<uint32_t>(std::stoul(word_split.at(1)));
    }
  }

  // MM specific conventions
  namespace MM {
    // Wheels
    constexpr uint32_t MUON_MM_ENDCAP_A_SIDE{0x6B};
    constexpr uint32_t MUON_MM_ENDCAP_C_SIDE{0x6C};
  }

  // Find side by source ID
  inline bool isA(const uint32_t sourceid) {
    return ( ((sourceid >> 16) & 0xff) == PAD::MUON_STGC_ENDCAP_A_SIDE || ((sourceid >> 16) & 0xff) == MM::MUON_MM_ENDCAP_A_SIDE );
  }
}
#endif
