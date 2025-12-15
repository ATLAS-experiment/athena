/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/eEmNbhoodTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Concrete class to hold eEmTOB information alongside a LArStripNeighborhood
 */

#ifndef GLOBALSIM_EEMNBHOODTOB_H
#define GLOBALSIM_EEMNBHOODTOB_H

#include "eEmTOB.h"
#include "AthenaKernel/CLASS_DEF.h"

#include "LArStripNeighborhood.h"

#include <bitset>

namespace GlobalSim::IOBitwise {
  /*! @copydoc IeEmNbhoodTOB 
  *
  * Additionally holds an LArStripNeighborhood alongside the eEmTOB information.
  */
  class eEmNbhoodTOB : public eEmTOB {
    
  public:
    /**
     * @brief Constructor taking an eFexROITOB and a LArStripNeighborhood
     * @param[in] eFexTOB The input eFexRoI TOB defining the common/eFex bits.
     * @param[in] nbhood The associated input neighbourhood
     *
     * To be used to create, and initilise a global eEmTOB from an existing eFexTOB
     * alongside an associated LArStripNeighbourhood
     * eFexRoI threshold bits are set here, the CommonTOB constructor is used
     * to initialise the common bits.
     */
    eEmNbhoodTOB(const xAOD::eFexEMRoI& eFexTOB, const GlobalSim::LArStripNeighborhood& nbhood);
    /**
     * @brief Constructor taking an eEmTOB and a LArStripNeighborhood
     * @param[in] eFexTOB The input eEmTOB
     * @param[in] nbhood The associated input neighbourhood
     *
     * To be used to create, and initilise a global eEmTOB from an existing eEmTOB
     * alongside an associated LArStripNeighbourhood.
     * eFexRoI threshold bits are set here, the CommonTOB constructor is used
     * to initialise the common bits.
     */
    eEmNbhoodTOB(const GlobalSim::IOBitwise::eEmTOB& tob,
		 const GlobalSim::LArStripNeighborhood& nbhood);

    //! @copydoc IeEmNbhoodTOB::~IeEmNbhoodTOB()   
    virtual ~eEmNbhoodTOB(){};

    //! @copydoc IeEmNbhoodTOB::Neighbourhood()
    virtual const LArStripNeighborhood& Neighbourhood() const;

    //! @copydoc IeEmNbhoodTOB::to_string()  
    virtual std::string to_string() const;
    
  private:
     /// Property: LArStripNeighborhood associated with this eEmTOB 
    LArStripNeighborhood m_neighbourhood;
  };
} //End of namespace 

CLASS_DEF( GlobalSim::IOBitwise::eEmNbhoodTOB , 229822253 , 1 )

#endif //GLOBALSIM_EEMNBHOODTOB_H
