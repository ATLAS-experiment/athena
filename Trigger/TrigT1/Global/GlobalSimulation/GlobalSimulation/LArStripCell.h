/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_LARSTRIPCELL_H
#define GLOBALSIM_LARSTRIPCELL_H

#include "GlobalSimulation/BitSpec.h"

namespace GlobalSim {

    namespace v1 {
        class LArStripCell : public BitSpec<LArStripCell,32> {

            // this is the "6-40-4" MLE ... only 6 of the 13 bits of et field will encode energy
            // the 2 MSB are the "range index" and the remaining bits are the "code" within the range
            // the LSBs in each range are 40, 40*4, 40*4*4, 40*4*4*4 MeV
        public:
            static const int sBITS = 6;
            static const int sLSB = 40; // in MeV! Assumes cannot have fractional MeV!
            static const int sGAIN = 4;
        protected:
            static const size_t sMAXCODE = (1<<(sBITS-2)) - 1; // max code in a given range

            static std::pair<float,float> et_mld(std::bitset<13> et) {
                auto range = (et.to_ulong() >> (sBITS-2));
                auto code = (et.to_ulong()) & sMAXCODE;
                int out = 0;
                auto scale = sLSB;
                while(range>0) {
                    out += scale*sMAXCODE;
                    scale *= sGAIN;
                    range--;
                }
                return {out + scale*code,out+scale*(code+1)};
            }

            static Bitset<13> et_mle(float et) {
                // this code is based off "LArCellPreparationAlg::encodeEnergy"

                if(et < 0) return 0;
                int et_int = std::round(et); // uses only integer MeV values
                unsigned int out_range = 0;
                auto scale = sLSB;
                float offset = 0;
                while(et_int >= (offset + sMAXCODE*scale)) {
                    out_range++;
                    if(out_range==4) {
                        // saturated cell, return max code
                        return (1<<sBITS)-1;
                    }
                    offset += sMAXCODE*scale;
                    scale *= sGAIN;
                }
                // return out_range in top 2 bits, and code in remaining bits
                return (out_range << (sBITS-2)) + (et_int - offset)/scale;
            }

        public:
            static inline const BitField<0, 12, float> et{{.name="et", .auxvar="et", .description="Transverse energy", .encoder=et_mle, .decoder=et_mld,.centre=0}};
            static inline const SignedBitField<13, 22, float> eta{{.name="eta", .auxvar="eta", .description="Eta coordinate", .scale=0.0125}};
            static inline const UnclampedBitField<23, 31, float> phi{{.name="phi", .auxvar="phi", .description="Phi coordinate", .scale=M_PI / 256}};

            DECLARE_FIELDS(et, eta, phi);
        };

    }

    typedef v1::LArStripCell LArStripCell;

}

#endif
