/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/CommonTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Concrete class to hold common (eta/eta/phi) TOB bits 
 */

#ifndef GLOBALSIM_COMMONTOB_H
#define GLOBALSIM_COMMONTOB_H

#include "ICommonTOB.h"
#include "xAODTrigger/eFexEMRoI.h"

#include <bitset>

namespace GlobalSim::IOBitwise{
  /*! @copydoc ICommonTOB */
  class CommonTOB : virtual public ICommonTOB {

  public:
    /** 
     * @brief Constructor taking an eFexROITOB to initialise common bits
     * @param[in] eFexTOB The input eFexRoI TOB defining the common/eFex bits.
     *
     * To be used to create, and initilise a CommonTOB from an existing eFexTOB
     */
    CommonTOB(const xAOD::eFexEMRoI& eFexTOB);
    /** 
     * @brief Constructor taking an existing CommonTOB to initialise common bits
     * @param[in] CommonTOB The input Global CommonTOB defining the common bits.
     *
     * To be used to create and initilaise a CommonTOB from an existing CommonTOB
     */
    CommonTOB(const GlobalSim::IOBitwise::ICommonTOB& CommonTOB);

    //! @copydoc ICommonTOB::~ICommonTOB()
    virtual ~CommonTOB(){};

    //! @copydoc ICommonTOB::et_bits() 
    virtual std::bitset<s_et_width> et_bits() const override;
    //! @copydoc ICommonTOB::eta_bits()
    virtual std::bitset<s_eta_width> eta_bits() const override;
    //! @copydoc ICommonTOB::phi_bits()
    virtual std::bitset<s_phi_width> phi_bits() const override; 

  private:
    /// Property: eT bitset within the common TOB word
    std::bitset<ICommonTOB::s_et_width> m_et_bits;
    /// Property: eta bitset within the common TOB word
    std::bitset<ICommonTOB::s_eta_width> m_eta_bits;
    /// Property: phi bitset within the common TOB word
    std::bitset<ICommonTOB::s_phi_width> m_phi_bits;
  };
} //End of namespace
#endif //GLOBALSIM_COMMONTOB_H
