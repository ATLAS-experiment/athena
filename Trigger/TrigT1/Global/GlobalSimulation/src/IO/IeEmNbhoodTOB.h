/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/IeEmNbhoodTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Interface class to hold eEmTOBs alongside LArStripNeighbourhoods
 */

#ifndef GLOBALSIM_IEEMNBHOODTOB_H
#define GLOBALSIM_IEEMNBHOODTOB_H

#include "IeEmTOB.h"
#include "LArStripNeighborhood.h"

#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim::IOBitwise {
  /**
   * @brief Class to hold an eFexRoI and LAr strip neighbourhood
   *
   * This class stores the eFEXROI information contained in an eEmTOB
   * and associates it with a LArStripNeighborhood: a window of 3*17 
   * cells around the centre of an eFexRoI
   */
  class IeEmNbhoodTOB : virtual public IeEmTOB {

  public:
    /** @brief Destructor*/
    virtual ~IeEmNbhoodTOB(){}

    /** @brief Returns the LarStripNeighbourhood: 3*17 cells centred on the eFexROI*/
    virtual const LArStripNeighborhood& Neighbourhood() const = 0;

    /** @brief print out contents to string*/
    virtual std::string to_string() const = 0;
  };

} //End of namespace

std::ostream& operator << (std::ostream&,
                           const GlobalSim::IOBitwise::IeEmNbhoodTOB&);

CLASS_DEF( GlobalSim::IOBitwise::IeEmNbhoodTOB , 81881587 , 1 )

#endif //GLOBALSIM_IEEMNBHOODTOB_H
