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

#include "ICommonTOB.h" // const statics
#include "xAODTrigger/eFexEMRoI.h"
#include "AthenaKernel/CLASS_DEF.h"

#include <bitset>

namespace GlobalSim::IOBitwise{
  /*! @copydoc ICommonTOB */
  class CommonTOB {

  public:
    /** 
     * @brief Constructor taking an eFexROITOB to initialise common bits
     * @param[in] eFexTOB The input eFexRoI TOB defining the common/eFex bits.
     *
     * To be used to create and initialise a CommonTOB from an existing eFexTOB
     */
    CommonTOB(const xAOD::eFexEMRoI& eFexTOB);
    /** 
     * @brief Constructor taking an existing CommonTOB to initialise common bits
     * @param[in] CommonTOB The input Global CommonTOB defining the common bits.
     *
     * To be used to create and initilaise a CommonTOB from an existing CommonTOB
     */
    CommonTOB(const GlobalSim::IOBitwise::CommonTOB& CommonTOB);

    /**
     * @brief Constructor taking raw bitsets to initialise common bits
     * @param[in] et_bits the transverse energy bits
     * @param[in] eta_bits the eta bits
     * @param[in] phi_bits the phi bits
     *
     * To be used to create and initialise a CommonTOB from individual 4-vector bitsets.
     */
    CommonTOB(const std::bitset<ICommonTOB::s_et_width>& et_bits,
	      const std::bitset<ICommonTOB::s_eta_width>& eta_bits,
	      const std::bitset<ICommonTOB::s_phi_width>& phi_bits);
    
    //! @copydoc ICommonTOB::~ICommonTOB()
    virtual ~CommonTOB(){};

    //! @copydoc ICommonTOB::et_bits() 
    virtual std::bitset<ICommonTOB::s_et_width> et_bits() const;
    //! @copydoc ICommonTOB::eta_bits()
    virtual std::bitset<ICommonTOB::s_eta_width> eta_bits() const;
    //! @copydoc ICommonTOB::phi_bits()
    virtual std::bitset<ICommonTOB::s_phi_width> phi_bits() const; 

    virtual std::string to_string() const;
  private:
    /// Property: eT bitset within the common TOB word
    std::bitset<ICommonTOB::s_et_width> m_et_bits;
    /// Property: eta bitset within the common TOB word
    std::bitset<ICommonTOB::s_eta_width> m_eta_bits;
    /// Property: phi bitset within the common TOB word
    std::bitset<ICommonTOB::s_phi_width> m_phi_bits;
  };

} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::CommonTOB , 186129504 , 1 )

#endif //GLOBALSIM_COMMONTOB_H
