/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/eEmTOB.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date September 2025
 * @brief Concrete class to hold eFexROI TOB bits
 */

#ifndef GLOBALSIM_EEMTOB_H
#define GLOBALSIM_EEMTOB_H

#include "IeEmTOB.h"
#include "CommonTOB.h"
#include "AthenaKernel/CLASS_DEF.h"

#include <bitset>

namespace GlobalSim::IOBitwise {
  /*! @copydoc IeEmTOB */
  class eEmTOB : virtual public IeEmTOB, private CommonTOB {
    
  public:
    /**
     * @brief Constructor taking an eFexROITOB to inisialise bits.
     * @param[in] eFexTOB The input eFexRoI TOB defining the common/eFex bits.
     * 
     * To be used to create, and initilise a global eEmTOB from an existing eFexTOB
     * eFexRoI threshold bits are set here, the CommonTOB constructor is used
     * to initialise the common bits.
     */
    eEmTOB(const xAOD::eFexEMRoI& eFexTOB);
    /**
     * @brief Constructor taking an existing eEmTOB to inisialise bits.
     * @param[in] eEmTOB The input eEmTOB.
     * 
     * To be used to create, and initilise a global eEmTOB from an existing eEmTOB
     * eFexRoI threshold bits are set here, the CommonTOB constructor is used
     * to initialise the common bits.
     */

    eEmTOB(const GlobalSim::IOBitwise::IeEmTOB& eEmTOB);

    eEmTOB(const GlobalSim::IOBitwise::ICommonTOB&,
	   const std::bitset<s_RHad_width>&,
	   const std::bitset<s_REta_width>&,
	   const std::bitset<s_WsTot_width>&,
	   const std::bitset<s_Seed_width>&,
	   const std::bitset<s_UpNotDown_width>&,
	   const std::bitset<s_SeedIsMax_width>&
	   );
    
    //! @copydoc IeEmTOB::~IeEmTOB()     
    virtual ~eEmTOB(){};

    //! @copydoc IeEmTOB::RHad_bits()
    virtual const std::bitset<s_RHad_width>& RHad_bits() const override;
    //! @copydoc IeEmTOB::WsTot_bits()
    virtual const std::bitset<s_WsTot_width>& WsTot_bits() const override;
    //! @copydoc IeEmTOB::REta_bits()
    virtual const std::bitset<s_REta_width>& REta_bits() const override;
    //! @copydoc IeEmTOB::Seed_bits()
    virtual const std::bitset<s_Seed_width>& Seed_bits() const override;
    //! @copydoc IeEmTOB::UpNotDown_bit()
    virtual const std::bitset<s_UpNotDown_width>& UpNotDown_bit() const override;
    //! @copydoc IeEmTOB::SeedIsMax_bit()
    virtual const std::bitset<s_SeedIsMax_width>& SeedIsMax_bit() const override;

    virtual std::string to_string() const override;

  private:

    /// Property: RHad threshold bitset within the eEmTOB word 
    std::bitset<s_RHad_width> m_RHad_bits;
    /// Property: Wstot threshold bitset within the eEmTOB word 
    std::bitset<s_WsTot_width> m_WsTot_bits;
    /// Property: REta threshold bitset within the eEmTOB word 
    std::bitset<s_REta_width> m_REta_bits;
    /// Property: Seed eta position bitset within the eEmTOB word 
    std::bitset<s_Seed_width> m_Seed_bits;
    /// Property: Up not down bitset within the eEmTOB word
    std::bitset<s_UpNotDown_width> m_UpNotDown_bit;
    /// Property: Seed is a maximum bitset within the eEmTOB word 
    std::bitset<s_SeedIsMax_width> m_SeedIsMax_bit;
  };
} //End of namespace

CLASS_DEF( GlobalSim::IOBitwise::eEmTOB , 13709477 , 1 )

#endif //GLOBALSIM_EEMTOB_H
