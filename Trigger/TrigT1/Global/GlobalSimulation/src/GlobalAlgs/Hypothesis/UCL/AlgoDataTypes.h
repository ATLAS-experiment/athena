/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ALGODATATYPES_H
#define GLOBALSIM_ALGODATATYPES_H

#include "AlgoConstants.h"

#include <bitset>
#include <ostream>
#include <vector>
#include <memory>

namespace GlobalSim {

  struct eEmTob {
    // vhdl type: record
    
    std::bitset<AlgoConstants::eFexEtBitWidth> Et;
    std::bitset<AlgoConstants::eFexDiscriminantBitWidth> REta;
    std::bitset<AlgoConstants::eFexDiscriminantBitWidth> RHad;
    std::bitset<AlgoConstants::eFexDiscriminantBitWidth> WsTot;
    std::bitset<AlgoConstants::eFexEtaBitWidth> Eta;
    std::bitset<AlgoConstants::eFexPhiBitWidth> Phi;
    std::bitset<1> Overflow;
  };

  using eEmTobPtr = std::shared_ptr<eEmTob>;

 
  
  template<typename BITSET>
  int bitSetToInt(BITSET bitset) {


    if (!bitset[bitset.size()-1]) { return bitset.to_ulong();}

    bitset.flip();
    return -(bitset.to_ulong() + 1);
  }
 
  struct GenericTob {
     // vhdl type: record

    GenericTob(const eEmTobPtr& in_tob) {
      m_Et = bitSetToInt(in_tob->Et);
      auto etaBin = (in_tob->Eta).to_ulong();
      m_Eta = etaBin < 200 ? -100 + etaBin : 100;
      
      m_Phi = static_cast<unsigned int>(bitSetToInt(in_tob->Phi));
    
      m_Charge = 0;
      m_Overflow = 0;
    }			      
  
    // at the time of writing the int widhs in the VHDL code were all < 16.
    
    unsigned int m_Et; // originally 13 bits
    int m_Eta; // originally 9 bits
    int m_Phi; // originally 7 bits
    int m_Charge; // originally 2 bits
    std::bitset<1> m_Overflow; // originally 1 bit
    
  };

}

std::ostream& operator << (std::ostream&, const GlobalSim::eEmTob&);
std::ostream& operator << (std::ostream&, const GlobalSim::GenericTob&);

  
template<typename T>
std::ostream& operator << (std::ostream& os,
			   const std::vector<T>& v){

  for(const T& t : v) {
    os << t << ' ';
  }
  return os;
}

  
template<typename T>
std::ostream& operator << (std::ostream& os,
			   const std::vector<std::shared_ptr<T>>& v){

  for(const auto& t : v) {
    os << *t << ' ';
  }
  return os;
}

template<typename T>
std::ostream& operator << (std::ostream& os,
			   const std::vector<std::vector<T>>& vv){

  std::size_t iv{0};
  for(const std::vector<T>& v : vv) {
    os << "\ncontainer " << iv++ << " [" << v.size() << "]:\n\n";
    os << v;
  }
  return os;
}

#endif
