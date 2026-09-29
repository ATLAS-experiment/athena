#include "GlobalSimulation/BitSpec.h"

namespace GlobalSim {

    // an example of specification of size 64-bit
    class TestSpec : public BitSpec<TestSpec, 64> {

        // Example of a custom encoding/decoding for one of the bitfields
        static constexpr float s_UNIT = 0.25f;
        static std::bitset<13> f1_encoder(float value) { return static_cast<unsigned long long>(value / s_UNIT); }
        static std::pair<float,float> f1_decoder(std::bitset<13> bits) {
            float v = (bits.to_ullong() * s_UNIT);
            return {v-0.5*s_UNIT,v+0.5*s_UNIT};
        }

    public:
        // syntax is:
        // BitField<Lo,Hi, AuxType [, ValueType=AuxType]> name{"name","auxvar", "description" [, encoder, decoder]}
        // auxvar can optionally include bitpacking-specifiers, in form of either auxvar[i] or auxvar[a:b]
        // this will map the bitfield into subbits of the auxvar. AuxType will need to be an integral type,
        // e.g. you cannot use bitpacking-specifiers on AuxType=float fields
        static inline const BitField<0, 12, float> f1{"f1","auxvar1", "Description 1", f1_encoder, f1_decoder};
        static inline const BitField<13, 22, int>  f2{"f2","auxvar2", "Description 2"};
        static inline const BitField<32, 32, uint8_t, bool> f3{"f3","auxvar3[0]", "one-bit flag, packed into auxvar3"};
        static inline const BitField<33, 34, uint8_t, int> f4{"f4","auxvar3[1:2]", "two-bit field, packed into auxvar3"};

        // next line was an example that will fail compilation because auxvar type isn't big enough for the bitfield
        //static inline const BitField<33, 55, uint8_t> f5{"f5","auxvar4", "two-bit field, packed into auxvar3"};

        DECLARE_FIELDS(f1,f2,f3,f4);

    };
}