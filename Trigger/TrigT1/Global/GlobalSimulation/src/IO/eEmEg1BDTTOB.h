/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/eEmEg1BDTTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Concrete class to hold eGamma1 BDT result
 */

#ifndef GLOBALSIM_EEMEGAMMA1BDTTOB_H
#define GLOBALSIM_EEMEGAMMA1BDTTOB_H

#include "IeEmEg1BDTTOB.h"
#include "eEmTOB.h"

#include <bitset>

namespace GlobalSim::IOBitwise {
  /*! @copydoc IeEmEg1BDTTOB */
  class eEmEg1BDTTOB : virtual public IeEmEg1BDTTOB, private eEmTOB {
    
  public:
    /**
     * @brief Constructor taking an eFexROITOB and eGamma1 BDT output bits to initialise bits..
     * @param[in] eFexTOB The input eFexRoI TOB defining the common/eFex bits.
     * @param[in] eFexTOB The input eGamma1 BDT bits defining the result of the algorithm.
     *
     * To be used to create, and initilise a global eEmTOB from an existing eFexTOB and an 
     * eGamma1 BDT result
     * eGamma1 BDT result bits are set here, eFexRoI threshold bits are set in the eEmTOB 
     * constructor, the CommonTOB constructor is used to initialise the common bits.
     */
    eEmEg1BDTTOB(const xAOD::eFexEMRoI& eFexTOB,
		 std::bitset<s_eGamma1BDT_width> eGamma1BDT_bits);
    //! @copydoc IeEmEg1BDTTOB::~IeEmEg1BDTTOB()
    virtual ~eEmEg1BDTTOB(){};

    //! @copydoc IeEmEg1BDTTOB::eGamma1BDT_bits()
    virtual std::bitset<s_eGamma1BDT_width> eGamma1BDT_bits() const override;

  private:
    /// Property: Bitset to hold the eGamma1BDT bits
    std::bitset<s_eGamma1BDT_width> m_eGamma1BDT_bits;
  };
}//End of namespace
#endif //GLOBALSIM_EEMEGAMMA1BDTTOB_H
