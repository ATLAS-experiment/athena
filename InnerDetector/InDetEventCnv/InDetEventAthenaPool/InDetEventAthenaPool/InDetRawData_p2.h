/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETRAWDATA_P2_H
#define INDETRAWDATA_P2_H

#include "Identifier/Identifier.h"

class PhaseIIPixelRawDataContainerCnv;
class PhaseIIStripRawDataContainerCnv;

class InDetRawData_p2 {
 public:
  InDetRawData_p2():m_rdoId(0),m_word(0)  {};
// List of Cnv classes that convert this into Rdo objects
  friend class TRT_LoLumRawDataCnv_p2;
  friend class SCT1_RawDataCnv_p2;
  friend class Pixel1RawDataCnv_p2;
  friend class PhaseIIPixelRawDataContainerCnv;
  friend class PhaseIIStripRawDataContainerCnv;
 private:
  Identifier::value_type m_rdoId; //Offline ID for readout channel
  unsigned int m_word; // raw data word 
};

#endif
