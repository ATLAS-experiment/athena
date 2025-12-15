/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <ostream>
#include "L1TopoRDO/L1TopoROD.h"
#include "L1TopoRDO/Helpers.h"

namespace L1Topo {

  L1TopoROD::L1TopoROD(const uint32_t trailer1, const uint32_t trailer2)
    :m_rodTrailer1(trailer1), m_rodTrailer2(trailer2) {
    this->decode();
  }
  
  void L1TopoROD::decode(){
    m_shelf = L1Topo::decode(m_rodTrailer1,18,2);
    m_rod = L1Topo::decode(m_rodTrailer1,16,2);
    m_linkErrMap = L1Topo::decode(m_rodTrailer1,20,12);
    m_payloadLength = L1Topo::decode(m_rodTrailer1,0,16);
    
    m_crc = L1Topo::decode(m_rodTrailer2,12,20);
    m_linkErrs = (L1Topo::decode(m_rodTrailer2,8,4) & 0xf) != 0;
    m_ct = (L1Topo::decode(m_rodTrailer2,6,1) & 0xf) != 0;
    m_pc = (L1Topo::decode(m_rodTrailer2,5,1) & 0xf) != 0;
    m_hc = (L1Topo::decode(m_rodTrailer2,4,1) & 0xf) != 0;
    m_pe = (L1Topo::decode(m_rodTrailer2,3,1) & 0xf) != 0;
    m_lm = (L1Topo::decode(m_rodTrailer2,2,1) & 0xf) != 0;
    m_hm = (L1Topo::decode(m_rodTrailer2,1,1) & 0xf) != 0;
    m_pt = (L1Topo::decode(m_rodTrailer2,0,1) & 0xf) != 0;
  }
  
  uint32_t L1TopoROD::rodTrailer1() const{
    return m_rodTrailer1;
  }
  
  uint32_t L1TopoROD::rodTrailer2() const{
    return m_rodTrailer2;
  }
  
  uint32_t L1TopoROD::shelf() const{
    return m_shelf;
  }
  
  uint32_t L1TopoROD::rod() const{
    return m_rod;
  }
  
  uint32_t L1TopoROD::linkErrMap() const{
    return m_linkErrMap;
  }
  
  uint32_t L1TopoROD::payloadLength() const{
    return m_payloadLength;
  }
  
  uint32_t L1TopoROD::crc() const{
    return m_crc;
  }
  
  bool L1TopoROD::linkErrs() const{
    return m_linkErrs;
  }
  
  bool L1TopoROD::ct() const{
    return m_ct;
  }
  
  bool L1TopoROD::pc() const{
    return m_pc;
  }
  
  bool L1TopoROD::hc() const{
    return m_hc;
  }
  
  bool L1TopoROD::pe() const{
    return m_pe;
  }
  
  bool L1TopoROD::lm() const{
    return m_lm;
  }
  
  bool L1TopoROD::hm() const{
    return m_hm;
  }
  
  bool L1TopoROD::pt() const{
    return m_pt;
  }
  
  std::ostream& operator<<(std::ostream& os, const L1TopoROD& s) {
    os << std::hex << std::showbase << s.rodTrailer1() << std::dec <<
      " Shelf= " << s.shelf() << " ROD= " << s.rod() <<
      " Link error map= " << s.linkErrMap() << " Payload length= " << s.payloadLength() <<
      "\n" <<
      std::hex << std::showbase << s.rodTrailer2() << std::dec <<
      " CRC= " << s.crc() << " Link errors= " << s.linkErrs() <<
      " ROD errors: CT= " << s.ct() << " pc= " << s.pc() << " hc= " << s.hc() <<
      " pe= " <<s.pe() << " lm= " << s.lm() << " hm= " << s.hm() << " pt= " << s.pt() <<
      std::dec << "\n";
    return os;
  }
    
} // namespace L1Topo

