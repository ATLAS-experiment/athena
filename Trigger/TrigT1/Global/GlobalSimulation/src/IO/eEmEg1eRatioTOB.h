/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/eEmEg1eRatioTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Concrete class to hold eGamma1 eRatio result bits
 */

#ifndef GLOBALSIM_EEMEGAMMA1ERATIOTOB_H
#define GLOBALSIM_EEMEGAMMA1ERATIOTOB_H

#include "IeEmEg1eRatioTOB.h"
#include "eEmTOB.h"
#include "AthenaKernel/CLASS_DEF.h"

#include <bitset>

namespace GlobalSim::IOBitwise {
  /*! @copydoc IeEmEg1eRatioTOB */
  class eEmEg1eRatioTOB : virtual public IeEmEg1eRatioTOB, private eEmTOB {
    
  public:
    /**
     * @brief Constructor taking an eFexROITOB and eGamma1 eRatio output bits to initialise bits.
     * @param[in] eFexTOB The input eFexRoI TOB defining the common/eFex bits.
     * @param[in] eFexTOB The input eGamma1 BDT bits defining the result of the algorithm.
     *
     * To be used to create, and initilise a global eEmTOB from an existing eFexTOB and an
     * eGamma1 BDT result
     * eGamma1 BDT result bits are set here, eFexRoI threshold bits are set in the eEmTOB
     * constructor, the CommonTOB constructor is used to initialise the common bits.
     */
    eEmEg1eRatioTOB(const xAOD::eFexEMRoI& eFexTOB,
		 std::bitset<s_eGamma1eRatio_width> eGamma1eRatio_bits);
    //! @copydoc IeEmEg1eRatioTOB::~IeEmEg1eRatioTOB()
    virtual ~eEmEg1eRatioTOB(){};

    //! @copydoc IeEmEg1eRatioTOB::eGamma1eRatio_bits()
    virtual std::bitset<s_eGamma1eRatio_width> eGamma1eRatio_bits() const override;

  private:
    // Property: Bitset to hold the eGamma1eRatio bits
    std::bitset<s_eGamma1eRatio_width> m_eGamma1eRatio_bits;
  };
} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::eEmEg1eRatioTOB , 134972597 , 1 )

#endif //GLOBALSIM_EEMEGAMMA1ERATIOTOB_H
