/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXV2QCOREENCODINGLUT_H
#define ITKPIXV2QCOREENCODINGLUT_H
#include <iostream>
#include <cstdint>
#include <array>
#include <cstddef>
#include <algorithm>
#include <ranges>

namespace ITkPixEncoding{
    constexpr std::size_t LookUpTableSize(1<<16);
    
    
    template<std::size_t Length, typename Generator>
    constexpr auto lut(Generator&& f){
      using content_type = decltype(f(std::size_t{0}));
      std::array<content_type, Length> arr;
      using namespace std::ranges;
      auto rangy = views::iota(std::size_t{0}, Length) // Generate a sequence
                 | views::transform(std::forward<Generator>(f)); // Transform using our generator
      copy(rangy, arr.begin());
      return arr;
  }

    constexpr uint32_t 
    one_bit(uint32_t v){
      return v!=0;
    }

    // nb. If we use std::array here instead of a C array,
    // then compilation fails with gcc13 in the dbg build
    // because the compile-time execution of encode() then takes
    // too many steps.
    constexpr void prepByte(uint32_t word, uint32_t b[8]){
      for(int i = 0 ;i<8;i++) {
        b[i] =  ((word >> (2*i)) & 0x1) << 1 | ((word >> (2*i+1)) & 0x1);
      }
    }

    std::array<uint32_t, 8> prepByte(uint32_t word)
    {
      std::array<uint32_t, 8> b;
      prepByte(word, b.data());
      return b;
    }
    

    //QCore encoding function, taken from Matthias Wittgen and Carlos
    uint32_t 
    constexpr encode(uint32_t decoded, uint32_t &encoded) {
        uint32_t b[8];
        prepByte(decoded, b);
        //
        uint32_t S1 = (one_bit(b[0] | b[1] | b[2] | b[3]) << 1) | one_bit(b[4] | b[5] | b[6] | b[7]);
        uint32_t S2t = (one_bit(b[0] | b[1]) << 1) | one_bit(b[2] | b[3]);
        uint32_t S2b = (one_bit(b[4] | b[5]) << 1) | one_bit(b[6] | b[7]);
        uint32_t S3tl = (one_bit(b[0]) << 1) | one_bit(b[1]);
        uint32_t S3tr = (one_bit(b[2]) << 1) | one_bit(b[3]);
        uint32_t S3bl = (one_bit(b[4]) << 1) | one_bit(b[5]);
        uint32_t S3br = (one_bit(b[6]) << 1) | one_bit(b[7]);

        uint32_t pos = 0;
        encoded = 0;

        auto writeTwo = [&](uint32_t src) {
            if (src == 0b01) {
                encoded |= (0b0) << (28 - pos);
                pos++;
            } else {
                encoded |= (src & 0x3) << (28 - pos);
                pos += 2;
            }
        };
        for(auto & val:{
            S1, S2t, S3tl, S3tr, b[0], b[1],b[2], b[3],
            S2b, S3bl, S3br,b[4], b[5], b[6], b[7]
        }) if(val) writeTwo(val);
        return pos;
    }

    //Create LUTs - if argument is true, return length LUT, if it's false return the encoded QCore LUT.
    //It could be prettified / optimized to fill in both LUTs in one go, but happens once per job and takes O(ms)
    //so no real gain there.
    inline auto create_lut_encode(bool length = false) {
        std::array<uint32_t, LookUpTableSize> lut;
        lut[0] = 0;
        for (uint32_t i = 1; i < 1 << 16; i++) {
            uint32_t encoded;
            uint64_t len = encode(i, encoded);
            lut[i] = length ? len : encoded;
        }
        return lut;
    }

    //It could be prettified / optimized to fill in both LUTs in one go, but happens once per job and takes O(ms)
    //so no real gain there.
    template<std::size_t Length>
    constexpr auto LutLen = lut<Length>([](uint32_t i){
        uint32_t encoded;
        return encode(i, encoded);
    });
    
    template<std::size_t Length>
    constexpr auto LutBTree = lut<Length>([](uint32_t i){
        uint32_t encoded;
        uint64_t len = encode(i, encoded);
        return encoded >> (30 - len);
    });


    constexpr std::array<uint32_t, LookUpTableSize > ITkPixV2QCoreEncodingLUT_Tree = LutBTree<LookUpTableSize>;
    constexpr std::array<uint32_t, LookUpTableSize > ITkPixV2QCoreEncodingLUT_Length = LutLen<LookUpTableSize>;

}
#endif
