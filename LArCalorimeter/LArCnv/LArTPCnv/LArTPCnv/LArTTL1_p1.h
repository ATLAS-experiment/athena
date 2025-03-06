/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARTPCNV_LARTTL1_P1_H
#define LARTPCNV_LARTTL1_P1_H

#include <vector>

#include "Identifier/Identifier.h"

// forward declarations
class LArTTL1Cnv_p1;

class LArTTL1_p1 {

  // Make the AthenaPoolCnv class our friend
  friend class LArTTL1Cnv_p1;

public:

  /** Default constructor: 
   */
  LArTTL1_p1() : m_offlineId(), m_samples() {}

private:

  Identifier m_offlineId;
  std::vector<float> m_samples;

};

#endif //> LARTPCNV_LARTTL1_P1_H
