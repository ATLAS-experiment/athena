/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/IeEmTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Interface class to hold eFexROI TOB bits
 */

#ifndef GLOBALSIM_IEEMTOB_H
#define GLOBALSIM_IEEMTOB_H

#include "ICommonTOB.h"
#include "AthenaKernel/CLASS_DEF.h"

#include <ostream>

namespace GlobalSim::IOBitwise {
  /**
   * @brief Class to hold eFexROI TOB bits
   *
   * This class stores the threshold, seed and boolean bits
   * which describe the output of an eFexROI, and defines functions
   * to retrieve this information. It has access to the CommonTOB
   * class information and can be used as a CommonTOB.
   */
  class IeEmTOB : virtual public ICommonTOB {

  public:

    /** @brief Destructor*/
    virtual ~IeEmTOB(){}


    /// Count: Size of hadronic thresholds satisfied bitset
    static constexpr std::size_t s_RHad_width{2};
    /// Count: Size of WsTot algorithm thresholds satisfied bitset
    static constexpr std::size_t s_WsTot_width{2};
    /// Count: Size of R0 thresholds satisfied bitset 
    static constexpr std::size_t s_REta_width{2};
    /// Count: Size of Seed eta position in the TOB bitset
    static constexpr std::size_t s_Seed_width{2};
    /// Count: Size of UpnotDown bit
    static constexpr std::size_t s_UpNotDown_width{1};
    /// Count: Size of Seed supercell is a local maxima bit
    static constexpr std::size_t s_SeedIsMax_width{1};

    /** @brief Returns the eFexRoI Rhad threshold bits*/
    virtual const std::bitset<s_RHad_width>& RHad_bits() const = 0;
    /** @brief Returns the eFexRoI Wstot threshold bits*/
    virtual const std::bitset<s_WsTot_width>& WsTot_bits() const = 0;
    /** @brief Returns the eFexRoI REta threshold bits*/
    virtual const std::bitset<s_REta_width>& REta_bits() const = 0;
    /** @brief Returns the eFexRoI seed eta position bits*/
    virtual const std::bitset<s_Seed_width>& Seed_bits() const = 0;
    /** @brief Returns the eFexRoI up not down bit
     *
     * True, if the seed includes the supercell above in phi.
     **/
    virtual const std::bitset<s_UpNotDown_width>& UpNotDown_bit() const = 0;
    /** @brief Returns the eFexRoI seed is a local maxima bit
     *
     * True if the seed supercell is a local maxima
     **/
    virtual const std::bitset<s_SeedIsMax_width>& SeedIsMax_bit() const = 0;
    /** @brief print out contents to string*/
    virtual std::string to_string() const = 0;
  };
} //End of namespace

std::ostream& operator << (std::ostream&,
			   const GlobalSim::IOBitwise::IeEmTOB&);

CLASS_DEF( GlobalSim::IOBitwise::IeEmTOB , 246749139 , 1 )

#endif //GLOBALSIM_IEEMTOB_H
