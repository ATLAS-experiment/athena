/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODMuonRDO/versions/NSWMMTPRDO_v1.h"
namespace {
   static const std::string preFixStr {"NSWMMTP_"};
}
#define IMPLEMENT_VECTOR(DATA_TYPE, VAR_NAME) \
   const std::vector<DATA_TYPE>& NSWMMTPRDO_v1::VAR_NAME() const { \
       static const SG::AuxElement::Accessor<std::vector<DATA_TYPE>> dec{preFixStr+#VAR_NAME};\
       return dec (*this); \
   } \
   std::vector<DATA_TYPE>& NSWMMTPRDO_v1::VAR_NAME()  { \
       static const SG::AuxElement::Accessor<std::vector<DATA_TYPE>> dec{preFixStr+#VAR_NAME};\
       return dec(*this); \
   }

#define IMPLEMENT_SCALAR(DATA_TYPE, VAR_NAME) \
   DATA_TYPE NSWMMTPRDO_v1::VAR_NAME() const { \
      static const SG::AuxElement::Accessor<DATA_TYPE> dec{preFixStr+#VAR_NAME};\
      return dec (*this); \
   } \
   void NSWMMTPRDO_v1::set_##VAR_NAME(const DATA_TYPE val) { \
      static const SG::AuxElement::Accessor<DATA_TYPE> dec{preFixStr+#VAR_NAME}; \
      dec (*this) = val; \
   }
namespace xAOD{

  //ROD info
  IMPLEMENT_SCALAR(uint32_t, sourceID)
  IMPLEMENT_SCALAR(uint16_t, moduleID)
  IMPLEMENT_SCALAR(uint32_t, ROD_L1ID)
  IMPLEMENT_SCALAR(uint16_t, ROD_BCID)

  //TP head
  IMPLEMENT_SCALAR(uint8_t , EC)
  IMPLEMENT_SCALAR(uint8_t , sectID)
  IMPLEMENT_SCALAR(uint32_t, L1ID)
  IMPLEMENT_SCALAR(uint16_t, BCID)

  //TP L1A head
  IMPLEMENT_SCALAR(uint16_t, l1a_request_BCID)
  IMPLEMENT_SCALAR(uint16_t, l1a_release_BCID)
  IMPLEMENT_SCALAR(uint16_t, l1a_window_open)
  IMPLEMENT_SCALAR(uint16_t, l1a_window_center)
  IMPLEMENT_SCALAR(uint16_t, l1a_window_close)
  IMPLEMENT_SCALAR(uint16_t, l1a_window_open_offset)
  IMPLEMENT_SCALAR(uint16_t, l1a_window_center_offset)
  IMPLEMENT_SCALAR(uint16_t, l1a_window_close_offset)

  //L1A data quality
  IMPLEMENT_SCALAR(uint16_t, l1a_timeout)
  IMPLEMENT_SCALAR(uint16_t, l1a_engines)

  //ART data
  IMPLEMENT_VECTOR(uint16_t, art_BCID)
  IMPLEMENT_VECTOR(uint8_t,  art_layer)
  IMPLEMENT_VECTOR(uint16_t, art_channel)

  //trigger data
  IMPLEMENT_VECTOR(uint16_t, trig_globalX)
  IMPLEMENT_VECTOR(uint16_t, trig_globalU)
  IMPLEMENT_VECTOR(uint16_t, trig_BCID)
  IMPLEMENT_VECTOR(uint8_t,  trig_dTheta)
  IMPLEMENT_VECTOR(uint8_t,  trig_ROI_rID)
  IMPLEMENT_VECTOR(uint8_t,  trig_ROI_phiID)

    
}

#undef IMPLEMENT_VECTOR
#undef IMPLEMENT_SCALAR
