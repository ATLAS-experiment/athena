/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ETAUTOB_H
#define GLOBALSIM_ETAUTOB_H

#include "AlgoConstants.h"
#include "GepAlgoHypothesisPortsIn.h"
#include <bitset>
#include <ostream>
#include <memory>

namespace GlobalSim {

  class eTauTob {

  public:
    friend std::ostream& operator << (std::ostream&, const GlobalSim::eTauTob&);

    eTauTob(const GepAlgoHypothesisPortsIn& ports_in);
    
    std::bitset<32> as_bits() const;

        
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
  eTauTob::Et_bits() const {return m_Et;}

  inline const std::bitset<AlgoConstants::eFexDiscriminantBitWidth>&
  eTauTob::eTauTob::REta_bits() const {return m_REta;}
    
  inline const std::bitset<AlgoConstants::eFexDiscriminantBitWidth>&
  eTauTob::RHad_bits() const {return m_RHad;}
    
  inline const std::bitset<AlgoConstants::eFexDiscriminantBitWidth>&
  eTauTob::WsTot_bits() const {return m_WsTot;}
    
  inline const std::bitset<AlgoConstants::eFexEtaBitWidth>&
  eTauTob::Eta_bits() const {return m_Eta;}
    
  inline const std::bitset<AlgoConstants::eFexPhiBitWidth>&
  eTauTob::Phi_bits() const {return m_Phi;}
    
  inline const std::bitset<1>&
  eTauTob::Overflow_bits () const {return m_Overflow;}

  using eTauTobPtr = std::shared_ptr<eTauTob>;
}

#endif
