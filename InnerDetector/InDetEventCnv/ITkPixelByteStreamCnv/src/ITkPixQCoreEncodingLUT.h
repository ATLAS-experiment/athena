#ifndef ITKPIXV2QCOREENCODINGLUT_H
#define ITKPIXV2QCOREENCODINGLUT_H
#include <cstdint>

namespace ITkPixEncoding{

    //QCore encoding function, taken from Matthias Wittgen and Carlos
    static inline constexpr uint32_t encode_(uint32_t decoded, uint32_t &encoded) {
        uint32_t b[8];
        for(int i = 0 ;i<8;i++) {
            b[i] =  ((decoded >> (2*i)) & 0x1) << 1 | ((decoded >> (2*i+1)) & 0x1);
        }
        auto one_bit = [](uint32_t value) ->uint32_t {
            return value!=0?1:0;
        };
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
        for(auto val:{
            S1, S2t, S3tl, S3tr, b[0], b[1],b[2], b[3],
            S2b, S3bl, S3br,b[4], b[5], b[6], b[7]
        }) if(val) writeTwo(val);
        return pos;
    }

    //Create LUTs - if argument is true, return length LUT, if it's false return the encoded QCore LUT.
    //It could be prettified / optimized to fill in both LUTs in one go, but happens at compile time
    //so no real gain there.
    static inline auto create_lut_encode_(bool length = false) {
        //std::array<uint32_t, 1 << 16> lut;//[1 << 16];
        uint32_t lut[1 << 16];
        lut[0] = 0;
        for (uint32_t i = 1; i < 1 << 16; i++) {
            uint32_t encoded;
            uint64_t len = encode_(i, encoded);
            lut[i] = length ? len : encoded;
        }
        return *lut;
    }



    //static const std::array<uint32_t, 1 << 16> ITkPixV2QCoreEncodingLUT_Tree   = {create_lut_encode_(false)};
    //static const std::array<uint32_t, 1 << 16> ITkPixV2QCoreEncodingLUT_Length = {create_lut_encode_(true) };
    static const uint32_t ITkPixV2QCoreEncodingLUT_Tree[1 << 16]   = {create_lut_encode_(false)};
    static const uint32_t ITkPixV2QCoreEncodingLUT_Length[1 << 16] = {create_lut_encode_(true) };

}
#endif