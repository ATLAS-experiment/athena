/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMTOB_H
#define GLOBALSIM_EEMTOB_H

#include "AlgoConstants.h"
#include "GepAlgoHypothesisPortsIn.h"
#include <bitset>
#include <ostream>
#include <memory>

namespace GlobalSim {

  class eEmTob {

  public:
    friend std::ostream& operator << (std::ostream&, const GlobalSim::eEmTob&);

    eEmTob(const GepAlgoHypothesisPortsIn& ports_in);
    
    std::bitset<32> asBits() const;

        
    const std::bitset<AlgoConstants::eFexEtBitWidth>&
    Et_bits() const;

    const std::bitset<AlgoConstants::eFexDiscriminantBitWidth>&
    REta_bits() const;
    
    const std::bitset<AlgoConstants::eFexDiscriminantBitWidth>&
    RHad_bits() const;
    
    const std::bitset<AlgoConstants::eFexDiscriminantBitWidth>&
    WsTot_bits() const;
    
    const std::bitset<AlgoConstants::eFexEtaBitWidth>&
    Eta_bits() const;
    
    const std::bitset<AlgoConstants::eFexPhiBitWidth>&
    Phi_bits() const;
    
    const std::bitset<1>&
    Overflow_bits () const;
 
  private:
    // vhdl type: record
    
    std::bitset<AlgoConstants::eFexEtBitWidth> m_Et;
    std::bitset<AlgoConstants::eFexDiscriminantBitWidth> m_REta;
    std::bitset<AlgoConstants::eFexDiscriminantBitWidth> m_RHad;
    std::bitset<AlgoConstants::eFexDiscriminantBitWidth> m_WsTot;
    std::bitset<AlgoConstants::eFexEtaBitWidth> m_Eta;
    std::bitset<AlgoConstants::eFexPhiBitWidth> m_Phi;
    std::bitset<1> m_Overflow;
  };

          
  inline const std::bitset<AlgoConstants::eFexEtBitWidth>&
  eEmTob::Et_bits() const {return m_Et;}

  inline const std::bitset<AlgoConstants::eFexDiscriminantBitWidth>&
  eEmTob::eEmTob::REta_bits() const {return m_REta;}
    
  inline const std::bitset<AlgoConstants::eFexDiscriminantBitWidth>&
  eEmTob::RHad_bits() const {return m_RHad;}
    
  inline const std::bitset<AlgoConstants::eFexDiscriminantBitWidth>&
  eEmTob::WsTot_bits() const {return m_WsTot;}
    
  inline const std::bitset<AlgoConstants::eFexEtaBitWidth>&
  eEmTob::Eta_bits() const {return m_Eta;}
    
  inline const std::bitset<AlgoConstants::eFexPhiBitWidth>&
  eEmTob::Phi_bits() const {return m_Phi;}
    
  inline const std::bitset<1>&
  eEmTob::Overflow_bits () const {return m_Overflow;}

  using eEmTobPtr = std::shared_ptr<eEmTob>;
}

#endif
