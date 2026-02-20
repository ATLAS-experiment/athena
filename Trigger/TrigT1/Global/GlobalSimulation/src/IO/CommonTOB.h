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

#include "xAODTrigger/eFexEMRoI.h"
#include "AthenaKernel/CLASS_DEF.h"

#include <bitset>

namespace GlobalSim::IOBitwise{
  /*! @copydoc ICommonTOB */
  class CommonTOB {

  public:

    
    ///Size of the eT bitset
    static constexpr std::size_t s_et_width{13};
    ///Size of the eta bitset
    static constexpr std::size_t s_eta_width{10};
    ///Size of the phi bitset
    static constexpr std::size_t s_phi_width{9};

    static constexpr std::size_t s_eFex_granularity{100}; // MeV

    static constexpr ulong max_et{(1UL << s_et_width)-1};
    // Errors if 0 or small
    static_assert(max_et != 0 && max_et <(1ULL << 63),
		  "Overflow or UB detected!");

    
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
    CommonTOB(const std::bitset<s_et_width>& et_bits,
	      const std::bitset<s_eta_width>& eta_bits,
	      const std::bitset<s_phi_width>& phi_bits);
    
    //! @copydoc ICommonTOB::~ICommonTOB()
    virtual ~CommonTOB(){};

    //! @copydoc ICommonTOB::et_bits() 
    virtual std::bitset<s_et_width> et_bits() const;
    //! @copydoc ICommonTOB::eta_bits()
    virtual std::bitset<s_eta_width> eta_bits() const;
    //! @copydoc ICommonTOB::phi_bits()
    virtual std::bitset<s_phi_width> phi_bits() const; 

    virtual std::string to_string() const;
  private:
    /// Property: eT bitset within the common TOB word
    std::bitset<s_et_width> m_et_bits;
    /// Property: eta bitset within the common TOB word
    std::bitset<s_eta_width> m_eta_bits;
    /// Property: phi bitset within the common TOB word
    std::bitset<s_phi_width> m_phi_bits;
  };

} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::CommonTOB , 186129504 , 1 )

#endif //GLOBALSIM_COMMONTOB_H
