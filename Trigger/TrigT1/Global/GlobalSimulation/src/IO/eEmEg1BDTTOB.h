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

#include "eEmTOB.h"
#include "AthenaKernel/CLASS_DEF.h"

#include <bitset>

namespace GlobalSim::IOBitwise {
  /*! @copydoc IeEmEg1BDTTOB */
  class eEmEg1BDTTOB : public eEmTOB {
    
  public:
    static const std::size_t s_eGamma1BDT_width{10};

    /**
     * @brief Constructor taking an eFexROITOB and eGamma1 BDT output bits to initialise bits..
     * @param[in] eFexTOB The input eFexRoI TOB defining the common/eFex bits.
     * @param[in] eGamma1BDT_bits The input eGamma1 BDT bits defining the result of the algorithm.
     *
     * To be used to create, and initilise a global eEmTOB from an existing eFexTOB and an 
     * eGamma1 BDT result
     * eGamma1 BDT result bits are set here, eFexRoI threshold bits are set in the eEmTOB 
     * constructor, the CommonTOB constructor is used to initialise the common bits.
     */
    eEmEg1BDTTOB(const xAOD::eFexEMRoI& eFexTOB,
		 std::bitset<s_eGamma1BDT_width> eGamma1BDT_bits);
    /**
     * @brief Constructor taking an eEmTOB and eGamma1 BDT output bits to initialise bits..
     * @param[in] eEmTOB The input eEmTOB defining the common/eFex bits.
     * @param[in] eGamma1BDT_bits The input eGamma1 BDT bits defining the result of the algorithm.
     *
     * To be used to create, and initilise a global eEmTOB from an existing eEmTOB and an 
     * eGamma1 BDT result
     * eGamma1 BDT result bits are set here, eFexRoI threshold bits are set in the eEmTOB 
     * constructor, the CommonTOB constructor is used to initialise the common bits.
     */
    eEmEg1BDTTOB(const eEmTOB&,
		 std::bitset<eEmEg1BDTTOB::s_eGamma1BDT_width> eGamma1BDT_bits);

    //! @copydoc IeEmEg1BDTTOB::~IeEmEg1BDTTOB()
    virtual ~eEmEg1BDTTOB(){};

    //! @copydoc IeEmEg1BDTTOB::eGamma1BDT_bits()
    virtual std::bitset<eEmEg1BDTTOB::s_eGamma1BDT_width> eGamma1BDT_bits() const;

    //! @copydoc IeEmEg1BDTTOB::to_string() 
    virtual std::string to_string() const;
  private:
    /// Property: Bitset to hold the eGamma1BDT bits
    std::bitset<eEmEg1BDTTOB::s_eGamma1BDT_width> m_eGamma1BDT_bits;
  };
}//End of namespace

CLASS_DEF( GlobalSim::IOBitwise::eEmEg1BDTTOB , 6527352 , 1 )

#endif //GLOBALSIM_EEMEGAMMA1BDTTOB_H
