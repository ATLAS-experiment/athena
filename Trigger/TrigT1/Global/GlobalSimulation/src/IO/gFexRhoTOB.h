/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/gFexRhoTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2026
 * @brief Concrete class to hold gFex TOB bits 
 */

#ifndef GLOBALSIM_GFEXRHOTOB_H
#define GLOBALSIM_GFEXRHOTOB_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthContainers/DataVector.h"

#include "xAODTrigger/gFexJetRoI.h"

#include <bitset>

namespace GlobalSim::IOBitwise{

  class gFexRhoTOB {

    /// Count: Size of hadronic thresholds satisfied bitset
    static constexpr std::size_t s_rho_width{15};

  public:

    /**
     * @brief Constructor taking an eFexROITOB to initialise bits.
     * @param[in] eFexTOB The input eFexRoI TOB defining the common/eFex bits.
     *
     * To be used to create, and initilise a global eEmTOB from an existing eFexTOB
     * eFexRoI threshold bits are set here, the CommonTOB constructor is used
     * to initialise the common bits.
     */
    gFexRhoTOB(const xAOD::gFexJetRoI& gFexRhoTOB);
    
    /** 
     * @brief Constructor taking an existing gFexRhoTOB to initialise common bits
     * @param[in] gFexRhoTOB The input Global gFexRhoTOB defining the common bits.
     *
     * To be used to create and initilaise a gFexRhoTOB from an existing gFexRhoTOB
     */
    gFexRhoTOB(const gFexRhoTOB& gFexRhoTOB);

    /**
     * @brief Constructor taking raw bitsets to initialise common bits
     *
     * To be used to create and initialise a gFexRhoTOB from individual bitsets.
     */
    gFexRhoTOB(const std::bitset<s_rho_width>&);
    
    //! @copydoc IgFexRhoTOB::~IgFexRhoTOB()
    virtual ~gFexRhoTOB(){};

    //! @copydoc IeEmTOB::Rho_bits()
    virtual std::bitset<s_rho_width> rho_bits() const;
    
    virtual std::string to_string() const;
  private:
    
    /// Property: Rho bitset within the gFexRhoTOB word
    std::bitset<s_rho_width> m_rho_bits;
  };

} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::gFexRhoTOB , 224783411 , 1 )

namespace GlobalSim {
  namespace IOBitwise {
    using gFexRhoTOBContainer = DataVector<GlobalSim::IOBitwise::gFexRhoTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::gFexRhoTOBContainer , 1266261565 , 1 )

#endif //GLOBALSIM_GFEXRHOTOB_H
