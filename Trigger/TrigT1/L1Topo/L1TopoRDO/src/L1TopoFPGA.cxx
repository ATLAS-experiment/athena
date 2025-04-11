/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include "L1TopoRDO/L1TopoFPGA.h"
#include "L1TopoRDO/Helpers.h"

namespace L1Topo {

  L1TopoFPGA::L1TopoFPGA(const uint32_t trailer1, const uint32_t trailer2)
    :m_fpgaTrailer1(trailer1), m_fpgaTrailer2(trailer2) {
    this->decode();
  }
  
  void L1TopoFPGA::decode(){
    // See https://twiki.cern.ch/twiki/bin/view/Atlas/L1CaloBytestreamDecoders#jFEX_System
    m_fpgaBlockSize  = L1Topo::decode(m_fpgaTrailer1,0,0xffff);
    m_topoNumber   = L1Topo::decode(m_fpgaTrailer1,20,0x7);
    m_fpgaNumber   = L1Topo::decode(m_fpgaTrailer1,18,0x3);
    m_numSlices    = L1Topo::decode(m_fpgaTrailer1,24,0xf);
    m_sliceNumber  = L1Topo::decode(m_fpgaTrailer1,28,0xf);
    m_crc          = L1Topo::decode(m_fpgaTrailer2,12,0xfffff);
    m_ct = (L1Topo::decode(m_fpgaTrailer2,5,1) & 0x1) != 0;
    m_sm = (L1Topo::decode(m_fpgaTrailer2,4,1) & 0x1) != 0;
    m_pe = (L1Topo::decode(m_fpgaTrailer2,3,1) & 0x1) != 0;
    m_lm = (L1Topo::decode(m_fpgaTrailer2,2,1) & 0x1) != 0;
    m_hm = (L1Topo::decode(m_fpgaTrailer2,1,1) & 0x1) != 0;
    m_pt = (L1Topo::decode(m_fpgaTrailer2,0,1) & 0x1) != 0;
  }
  
  uint32_t L1TopoFPGA::fpgaTrailer1() const{
    return m_fpgaTrailer1;
  }
  
  uint32_t L1TopoFPGA::fpgaTrailer2() const{
    return m_fpgaTrailer2;
  }
  
  size_t L1TopoFPGA::fpgaBlockSize() const{
    return m_fpgaBlockSize;
  }
  
  uint32_t L1TopoFPGA::topoNumber() const{
    return m_topoNumber;
  }
  
  uint32_t L1TopoFPGA::fpgaNumber() const{
    return m_fpgaNumber;
  }
  
  uint32_t L1TopoFPGA::numSlices() const{
    return m_numSlices;
  }
  
  uint32_t L1TopoFPGA::sliceNumber() const{
    return m_sliceNumber;
  }
  
  uint32_t L1TopoFPGA::crc() const{
    return m_crc;
  }
  
  
  bool L1TopoFPGA::ct() const{
    return m_ct;
  }
  
  bool L1TopoFPGA::sm() const{
    return m_sm;
  }
  
  bool L1TopoFPGA::pe() const{
    return m_pe;
  }
  
  bool L1TopoFPGA::lm() const{
    return m_lm;
  }
  
  bool L1TopoFPGA::hm() const{
    return m_hm;
  }
  
  bool L1TopoFPGA::pt() const{
    return m_pt;
  }
  
  std::ostream& operator<<(std::ostream& os, const L1TopoFPGA& s) {
    os << std::hex << std::showbase << s.fpgaTrailer1() << std::dec <<
      " Block Size= " << s.fpgaBlockSize() << " topoNumber= " << s.topoNumber() <<
      " fpgaNumber= " << s.fpgaNumber() << " numSlices= " << s.numSlices() <<
      " sliceNumber= " << s.sliceNumber() <<
      "\n" <<
      std::hex << std::showbase << s.fpgaTrailer2() << std::dec <<
      " CRC= " << s.crc() <<
      " Errors: CT= " << s.ct() << " sm= " << s.sm() <<
      " pe= " <<s.pe() << " lm= " << s.lm() << " hm= " << s.hm() << " pt= " << s.pt() <<
      std::dec << "\n";
    return os;
  }
    
} // namespace L1Topo

