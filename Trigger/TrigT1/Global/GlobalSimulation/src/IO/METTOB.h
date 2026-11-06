/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/METTOB.h
 * @brief Concrete class to hold MET TOB bits
 */

#ifndef GLOBALSIM_METTOB_H
#define GLOBALSIM_METTOB_H

#include "CommonTOB.h"

#include "AthenaKernel/CLASS_DEF.h"
#include "AthContainers/DataVector.h"

#include <bitset>

namespace GlobalSim::IOBitwise {
  /**
   * @brief Class to hold MET TOB bits
   *
   * The 64-bit MET TOB Format of Table 4.5
   *
   *   CommonTOB::et_bits   ->  ET_miss    [12:0]  13b, the full field
   *   CommonTOB::phi_bits  ->  phi_miss   [18:13]  6b of the 9b CommonTOB field
   *   CommonTOB::eta_bits  ->  UNUSED, set to zero
   *   ex/ey/sum_et and the five overflow flags are declared below
   *
   *
   * Things to note:
   * et_miss is NOT exactly sqrt(ex^2 + ey^2). It comes from the normalized-LUT square root
   * in MET_LUT_SQRT.v, which deviates from the exact integer root by up to 3 counts (0.75 GeV).
   *
   * phi_miss is NOT exactly atan2(ey, ex) rounded. It comes from the divider-free comparator
   * ladder in MET_PHI_COMPARATOR.v, accurate to about half a bin.
   *
   * ex and ey are now two's complement
   */
  class METTOB : public CommonTOB {

  public:

    /// Count: Size of the missing-energy x component bitset (two's complement)
    static constexpr std::size_t s_ex_width{13};
    /// Count: Size of the missing-energy y component bitset (two's complement)
    static constexpr std::size_t s_ey_width{13};
    /// Count: Size of the scalar energy sum bitset
    static constexpr std::size_t s_sum_et_width{13};
    /// Count: Size of the Ex overflow flag
    static constexpr std::size_t s_ex_overflow_width{1};
    /// Count: Size of the Ey overflow flag
    static constexpr std::size_t s_ey_overflow_width{1};
    /// Count: Size of the energy sum overflow flag
    static constexpr std::size_t s_sum_et_overflow_width{1};
    /// Count: Size of the missing-energy magnitude overflow flag
    static constexpr std::size_t s_met_overflow_width{1};
    /// Count: Size of the upstream overflow flag
    static constexpr std::size_t s_upstream_overflow_width{1};

    /**
     * @brief Constructor taking a METTOB to initialise bits.
     * @param[in] METTOB The input METTOB defining the common/MET bits.
     */
    METTOB(const METTOB&) = default;

    /**
     * @brief Constructor taking a CommonTOB and the MET bitsets to initialise bits.
     * @param[in] CommonTOB The input CommonTOB carrying ET_miss and phi_miss.
     * @param[in] bitsets The input bitsets defining the MET-specific bits.
     */
    METTOB(const GlobalSim::IOBitwise::CommonTOB&,
	   const std::bitset<s_ex_width>&,
	   const std::bitset<s_ey_width>&,
	   const std::bitset<s_sum_et_width>&,
	   const std::bitset<s_ex_overflow_width>&,
	   const std::bitset<s_ey_overflow_width>&,
	   const std::bitset<s_sum_et_overflow_width>&,
	   const std::bitset<s_met_overflow_width>&,
	   const std::bitset<s_upstream_overflow_width>&
	   );

    /** @brief Destructor*/
    virtual ~METTOB(){};

    /** @brief Returns the missing-energy x component bits (two's complement)*/
    virtual const std::bitset<s_ex_width>& ex_bits() const;
    /** @brief Returns the missing-energy y component bits (two's complement)*/
    virtual const std::bitset<s_ey_width>& ey_bits() const;
    /** @brief Returns the scalar energy sum bits*/
    virtual const std::bitset<s_sum_et_width>& sum_et_bits() const;
    /** @brief Returns the Ex overflow flag bit*/
    virtual const std::bitset<s_ex_overflow_width>& ex_overflow_bit() const;
    /** @brief Returns the Ey overflow flag bit*/
    virtual const std::bitset<s_ey_overflow_width>& ey_overflow_bit() const;
    /** @brief Returns the energy sum overflow flag bit*/
    virtual const std::bitset<s_sum_et_overflow_width>& sum_et_overflow_bit() const;
    /** @brief Returns the missing-energy magnitude overflow flag bit*/
    virtual const std::bitset<s_met_overflow_width>& met_overflow_bit() const;
    /** @brief Returns the upstream overflow flag bit*/
    virtual const std::bitset<s_upstream_overflow_width>& upstream_overflow_bit() const;

    /** @brief print out contents to string*/
    virtual std::string to_string() const;

  private:

    /// Property: missing-energy x component bitset within the METTOB word
    std::bitset<s_ex_width> m_ex_bits;
    /// Property: missing-energy y component bitset within the METTOB word
    std::bitset<s_ey_width> m_ey_bits;
    /// Property: scalar energy sum bitset within the METTOB word
    std::bitset<s_sum_et_width> m_sum_et_bits;
    /// Property: Ex overflow flag bitset within the METTOB word
    std::bitset<s_ex_overflow_width> m_ex_overflow_bit;
    /// Property: Ey overflow flag bitset within the METTOB word
    std::bitset<s_ey_overflow_width> m_ey_overflow_bit;
    /// Property: energy sum overflow flag bitset within the METTOB word
    std::bitset<s_sum_et_overflow_width> m_sum_et_overflow_bit;
    /// Property: missing-energy magnitude overflow flag bitset within the METTOB word
    std::bitset<s_met_overflow_width> m_met_overflow_bit;
    /// Property: upstream overflow flag bitset within the METTOB word
    std::bitset<s_upstream_overflow_width> m_upstream_overflow_bit;

  };
} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::METTOB , 137204915 , 1 )

DATAVECTOR_BASE(GlobalSim::IOBitwise::METTOB, GlobalSim::IOBitwise::CommonTOB);

namespace GlobalSim {
  namespace IOBitwise {
    using METTOBContainer = DataVector<GlobalSim::IOBitwise::METTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::METTOBContainer , 1099238456 , 1 )

#endif //GLOBALSIM_METTOB_H
