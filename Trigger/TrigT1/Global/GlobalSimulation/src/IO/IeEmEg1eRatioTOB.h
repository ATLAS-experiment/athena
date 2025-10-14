/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/IeEmTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Interface class to hold eGamma1 eRatio decision bits
 */

#ifndef GLOBALSIM_IEEMEG1ERATIOTOB_H
#define GLOBALSIM_IEEMEG1ERATIOTOB_H

#include "IeEmTOB.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim::IOBitwise {
  /**
   * @brief Class to hold eGamma1 eRatio result bits
   *
   * This class stores the result of the eGamma1 eRatio and defines functions
   * to retrieve this information. It has access to the eEmTOB and CommonTOB
   * class information and can be used as an eEmTOB or CommonTOB.
   */
  class IeEmEg1eRatioTOB : virtual public IeEmTOB {

  public:
    /** @brief Destructor*/
    virtual ~IeEmEg1eRatioTOB(){}

    /// Count: Size of output bits of the eGamma1 eRatio algorithm
    static const std::size_t s_eGamma1eRatio_width{11};

    /** @brief Returns the eGamma1 eRatio result bits*/
    virtual std::bitset<s_eGamma1eRatio_width> eGamma1eRatio_bits() const = 0;
  };
} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::IeEmEg1eRatioTOB , 207229871 , 1 )

#endif //GLOBALSIM_IEEMEG1ERATIOTOB_H
