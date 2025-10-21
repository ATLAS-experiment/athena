/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/ICommonTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Interface class to hold common (eta/eta/phi) TOB bits
 */

#ifndef GLOBALSIM_ICOMMONTOB_H
#define GLOBALSIM_ICOMMONTOB_H

#include <bitset>
#include <ostream>
#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim::IOBitwise{
  /**
   * @brief Class to hold common (eta/eta/phi) TOB bits
   *
   * This base class defines the bitsets to hold the common et, eta, phi
   * bits in GlobalTOBs, and their retrieval functions.
   */

  template<std::size_t e>
  constexpr std::size_t pow2(){
    std::size_t result{1};
    for (auto i = e; i != 0; --i) {
	result *= 2;
    }
    return result;
  }

  class ICommonTOB {

  public:

    /** @brief Destructor*/
    virtual ~ICommonTOB(){}

    ///Size of the eT bitset
    static constexpr std::size_t s_et_width{13};
    ///Size of the eta bitset
    static constexpr std::size_t s_eta_width{10};
    ///Size of the phi bitset
    static constexpr std::size_t s_phi_width{9};

    static constexpr std::size_t s_eFex_granularity{100}; // MeV

    static constexpr std::size_t max_et{pow2<s_et_width>()-1};


    /** @brief Returns the eT bits of this TOB*/
    virtual std::bitset<s_et_width> et_bits() const = 0;
    /** @brief Returns the eta bits of this TOB*/
    virtual std::bitset<s_eta_width> eta_bits() const = 0;
    /** @brief Returns the phi bits of this TOB*/
    virtual std::bitset<s_phi_width> phi_bits() const = 0;
    /** @brief print out contents to string*/
    virtual std::string to_string() const = 0;
  };

  using ICommonTOBContainer = std::vector<std::shared_ptr<ICommonTOB>>;
  
  /** @brief Output stream operator*/

} //End of namespace 

std::ostream& operator << (std::ostream&,
			   const GlobalSim::IOBitwise::ICommonTOB&);

CLASS_DEF( GlobalSim::IOBitwise::ICommonTOB , 220265942 , 1 )
CLASS_DEF( GlobalSim::IOBitwise::ICommonTOBContainer , 1229615490 , 1 )

#endif //GLOBALSIM_ICOMMONTOB_H
