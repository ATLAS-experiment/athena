/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/Jet1TOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2026
 * @brief Concrete class to hold Jet1 WTA TOB bits
 */

#ifndef GLOBALSIM_JET1TOB_H
#define GLOBALSIM_JET1TOB_H

#include "CommonTOB.h"

#include "AthenaKernel/CLASS_DEF.h"
#include "AthContainers/DataVector.h"

#include <bitset>

namespace GlobalSim::IOBitwise {
  /**
   * @brief Class to hold Jet1 WTA TOB bits
   *
   * This class stores the ring energy and tob count bits 
   * which describe the output of the Jet1 WTA algorithm, and defines functions
   * to retrieve this information. It has access to the CommonTOB
   * class information and can be used as a CommonTOB.
   */
  class Jet1TOB : public  CommonTOB {
    
  public:

    
    /// Count: Size of ring zero energy bitset
    static constexpr std::size_t s_ring_zero_ptt_width{12};
    /// Count: Size of ring one energy bitset
    static constexpr std::size_t s_ring_one_ptt_width{15};
    /// Count: Size of ring two energy bitset
    static constexpr std::size_t s_ring_two_ptt_width{15};
    /// Count: Size of ring three energy bitset
    static constexpr std::size_t s_ring_three_ptt_width{15};
    /// Count: Size of ring four energy bitset
    static constexpr std::size_t s_ring_four_ptt_width{12};
    /// Count: Size of total number of tobs bitset
    static constexpr std::size_t s_total_tobs_width{6};
    /// Count: Size of ring one tobs bitset
    static constexpr std::size_t s_ring_one_tobs_width{4};
    /// Count: Size of ring two tobs bitset
    static constexpr std::size_t s_ring_two_tobs_width{5};
    /// Count: Size of ring three tobs bitset
    static constexpr std::size_t s_ring_three_tobs_width{5};
    /// Count: Size of ring four tobs bitset
    static constexpr std::size_t s_ring_four_tobs_width{3};
    /// Count: Size of truncated flag
    static constexpr std::size_t s_truncate_width{1};
    /// Count: Size of next tob same et flag
    static constexpr std::size_t s_next_tob_same_et_width{1};
    /// Count: Size of error state flag
    static constexpr std::size_t s_error_width{1};
    /// Count: Size of overflow flag
    static constexpr std::size_t s_et_overflow_width{1};
    
    /**
     * @brief Constructor taking an Jet1TOB to initialise bits.
     * @param[in] Jet1TOB The input Jet1TOB defining the common/Jet1 bits.
     * 
     * To be used to create, and initilise a global Jet1TOB from an existing Jet1TOB
     * Jet1RoI threshold bits are set here, the CommonTOB constructor is used
     * to initialise the common bits.
     */
    Jet1TOB(const Jet1TOB&);

    /**
     * @brief Constructor taking an CommonTOB and Jet1 WTA bitsets to initialise bits.
     * @param[in] CommonTOB The input CommonTOB defining the common bits.
     * @param[in] bitsets The input bitsets defining the Jet1 WTA jet bits.
     * 
     * To be used to create, and initilise a global Jet1TOB from an existing CommonTOB
     * and a set of Jet1 WTA jet bits.
     * Jet1RoI threshold bits are set here, the CommonTOB constructor is used
     * to initialise the common bits.
     */
    Jet1TOB(const GlobalSim::IOBitwise::CommonTOB&,
	    const std::bitset<s_ring_zero_ptt_width>&,
	    const std::bitset<s_ring_one_ptt_width>&,
	    const std::bitset<s_ring_two_ptt_width>&,
	    const std::bitset<s_ring_three_ptt_width>&,
	    const std::bitset<s_ring_four_ptt_width>&,
	    const std::bitset<s_total_tobs_width>&,
	    const std::bitset<s_ring_one_tobs_width>&,
	    const std::bitset<s_ring_two_tobs_width>&,
	    const std::bitset<s_ring_three_tobs_width>&,
	    const std::bitset<s_ring_four_tobs_width>&,
	    const std::bitset<s_truncate_width>&,
	    const std::bitset<s_next_tob_same_et_width>&,
	    const std::bitset<s_error_width>&,
	    const std::bitset<s_et_overflow_width>&
	    );
    
    /** @brief Destructor*/
    virtual ~Jet1TOB(){};

    /** @brief Returns the ring zero energy bits*/
    virtual const std::bitset<s_ring_zero_ptt_width>& ring_zero_ptt_bits() const;
    /** @brief Returns the ring one energy bits*/
    virtual const std::bitset<s_ring_one_ptt_width>& ring_one_ptt_bits() const;
    /** @brief Returns the ring two energy bits*/
    virtual const std::bitset<s_ring_two_ptt_width>& ring_two_ptt_bits() const;
    /** @brief Returns the ring three energy bits*/
    virtual const std::bitset<s_ring_three_ptt_width>& ring_three_ptt_bits() const;
    /** @brief Returns the ring four energy bits*/
    virtual const std::bitset<s_ring_four_ptt_width>& ring_four_ptt_bits() const;
    /** @brief Returns the total tobs bits*/
    virtual const std::bitset<s_total_tobs_width>& total_tobs_bits() const;
    /** @brief Returns the ring one tobs bits*/
    virtual const std::bitset<s_ring_one_tobs_width>& ring_one_tobs_bits() const;
    /** @brief Returns the ring two tobs bits*/
    virtual const std::bitset<s_ring_two_tobs_width>&  ring_two_tobs_bits() const;
    /** @brief Returns the ring three tobs bits*/
    virtual const std::bitset<s_ring_three_tobs_width>&  ring_three_tobs_bits() const;
    /** @brief Returns the ring four tobs bits*/
    virtual const std::bitset<s_ring_four_tobs_width>&  ring_four_tobs_bits() const;
    /** @brief Returns the truncated flag bit*/
    virtual const std::bitset<s_truncate_width>&  truncate_bit() const;
    /** @brief Returns the next tob is same ET flag  bit*/
    virtual const std::bitset<s_next_tob_same_et_width>&  next_tob_same_et_bit() const;
    /** @brief Returns the error flag bit*/
    virtual const std::bitset<s_error_width>&  error_bit() const;
    /** @brief Returns the et overflow flag bit*/
    virtual const std::bitset<s_et_overflow_width>& et_overflow_bit() const;

    /** @brief print out contents to string*/
    virtual std::string to_string() const;

  private:

    /// Property: ring zero ptt bitset within the Jet1TOB word 
    std::bitset<s_ring_zero_ptt_width> m_ring_zero_ptt_bits;
    /// Property: ring one ptt bitset within the Jet1TOB word 
    std::bitset<s_ring_one_ptt_width> m_ring_one_ptt_bits;
    /// Property: ring two ptt bitset within the Jet1TOB wor d
    std::bitset<s_ring_two_ptt_width> m_ring_two_ptt_bits;
    /// Property: ring three ptt bitset within the Jet1TOB word 
    std::bitset<s_ring_three_ptt_width> m_ring_three_ptt_bits;
    /// Property: ring four ptt bitset within the Jet1TOB word 
    std::bitset<s_ring_four_ptt_width> m_ring_four_ptt_bits;
    /// Property: total tobs bitset within the Jet1TOB word 
    std::bitset<s_total_tobs_width> m_total_tobs_bits;
    /// Property: ring one tobs bitset within the Jet1TOB word 
    std::bitset<s_ring_one_tobs_width> m_ring_one_tobs_bits;
    /// Property: ring two tobs bitset within the Jet1TOB word 
    std::bitset<s_ring_two_tobs_width> m_ring_two_tobs_bits;
    /// Property: ring three tobs bitset within the Jet1TOB word 
    std::bitset<s_ring_three_tobs_width> m_ring_three_tobs_bits;
    /// Property: ring four tobs bitset within the Jet1TOB word 
    std::bitset<s_ring_four_tobs_width> m_ring_four_tobs_bits;
    /// Property: truncated flag bitset within the Jet1TOB word 
    std::bitset<s_truncate_width> m_truncate_bit;
    /// Property: next tob is same et flag bitset within the Jet1TOB word 
    std::bitset<s_next_tob_same_et_width> m_next_tob_same_et_bit;
    /// Property: error falg bitset within the Jet1TOB word 
    std::bitset<s_error_width> m_error_bit;
    /// Property: et overflow flag  bitset within the Jet1TOB word 
    std::bitset<s_et_overflow_width> m_et_overflow_bit;

  };
} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::Jet1TOB , 69350821 , 1 )

DATAVECTOR_BASE(GlobalSim::IOBitwise::Jet1TOB, GlobalSim::IOBitwise::CommonTOB); 

namespace GlobalSim {
  namespace IOBitwise {
    using Jet1TOBContainer = DataVector<GlobalSim::IOBitwise::Jet1TOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::Jet1TOBContainer , 1285016723 , 1 )

#endif //GLOBALSIM_JET1TOB_H
