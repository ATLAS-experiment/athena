/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"
// Local include(s):
#include "xAODMuonRDO/versions/NRPCRDO_v1.h"

namespace {
    static const std::string preFixStr{"MuonRdo_"};   
}
#define IMPLEMENT_SETTER_GETTER( DTYPE, GETTER, SETTER)                          \
      DTYPE NRPCRDO_v1::GETTER() const {                                  \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         return acc(*this);                                                      \
      }                                                                          \
                                                                                 \
      void NRPCRDO_v1::SETTER(DTYPE value) {                        \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         acc(*this) = value;                                                     \
      }


namespace xAOD{



IMPLEMENT_SETTER_GETTER( uint32_t, bcid, setBcid)
IMPLEMENT_SETTER_GETTER( float, time, setTime)
IMPLEMENT_SETTER_GETTER( uint16_t, subdetector, setSubdetector)
IMPLEMENT_SETTER_GETTER( uint16_t, boardsector, setBoardsector)
IMPLEMENT_SETTER_GETTER( uint16_t, board, setBoard)
IMPLEMENT_SETTER_GETTER( uint16_t, channel, setChannel)
IMPLEMENT_SETTER_GETTER( float, timeoverthr, setTimeoverthr)
}
#undef IMPLEMENT_SETTER_GETTER
