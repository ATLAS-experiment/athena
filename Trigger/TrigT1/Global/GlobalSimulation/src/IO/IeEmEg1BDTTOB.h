/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/IeEmTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Interface class to hold eGamma1 BDT decision bits
 */

#ifndef GLOBALSIM_IEEMEG1BDTTOB_H
#define GLOBALSIM_IEEMEG1BDTTOB_H

#include "IeEmTOB.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim::IOBitwise {
  /**
   * @brief Class to hold eGamma1 BDT decision bits
   *
   * This class stores the result of the eGamma1 BDT and defines functions
   * to retrieve this information. It has access to the eEmTOB and CommonTOB
   * class information and can be used as an eEmTOB or CommonTOB.
   */
  class IeEmEg1BDTTOB : virtual public IeEmTOB {

  public:
    /** @brief Destructor*/
    virtual ~IeEmEg1BDTTOB(){}

    /// Count: Size of output bits of the eGamma1 BDT algorithm
    static const std::size_t s_eGamma1BDT_width{10};

    /** @brief Returns the eGamma1 BDT result bits*/
    virtual std::bitset<s_eGamma1BDT_width> eGamma1BDT_bits() const = 0;
  };

} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::IeEmEg1BDTTOB , 67631718 , 1 )

#endif //GLOBALSIM_IEEMEG1BDTTOB_H
