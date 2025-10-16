/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/IeEmNbhoodTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Interface class to hold eGamma1 BDT decision bits
 */

#ifndef GLOBALSIM_IEEMNBHOODTOB_H
#define GLOBALSIM_IEEMNBHOODTOB_H

#include "IeEmTOB.h"
#include "LArStripNeighborhood.h"

#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim::IOBitwise {
  /**
   * @brief Class to hold eGamma1 BDT decision bits
   *
   * This class stores the result of the eGamma1 BDT and defines functions
   * to retrieve this information. It has access to the eEmTOB and CommonTOB
   * class information and can be used as an eEmTOB or CommonTOB.
   */
  class IeEmNbhoodTOB : virtual public IeEmTOB {

  public:
    /** @brief Destructor*/
    virtual ~IeEmNbhoodTOB(){}

    /** @brief Returns the eGamma1 BDT result bits*/
    virtual const LArStripNeighborhood& Neighbourhood() const = 0;
  };

} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::IeEmNbhoodTOB , 81881587 , 1 )

#endif //GLOBALSIM_IEEMNBHOODTOB_H
