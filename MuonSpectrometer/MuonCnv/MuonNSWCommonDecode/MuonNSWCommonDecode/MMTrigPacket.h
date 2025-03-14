/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_MMTRIG_H_
#define _MUON_NSW_MMTRIG_H_

#include <stdint.h>
#include <vector>
#include <exception>

namespace Muon
{
  namespace nsw
  {
    class MMTrigPacket
    {
    public:
      
      MMTrigPacket (std::vector<uint32_t>& payload);
      virtual ~MMTrigPacket () = default;
      uint32_t trig_globalX () const {return m_trig_globalX;};
      uint32_t trig_globalU () const {return m_trig_globalU;};
      uint32_t trig_BCID () const {return m_trig_BCID;};
      uint32_t trig_reserved () const {return m_trig_reserved;};
      uint32_t trig_dTheta () const {return m_trig_dTheta;};
      uint32_t trig_phiBin () const {return m_trig_phiBin;};
      uint32_t trig_rBin () const {return m_trig_rBin;};
      
    private:
      uint32_t m_trig_globalX;
      uint32_t m_trig_globalU;
      uint32_t m_trig_BCID;
      uint32_t m_trig_reserved;
      uint32_t m_trig_dTheta;
      uint32_t m_trig_phiBin;
      uint32_t m_trig_rBin;
      
    };
  }
}

#endif // _MUON_NSW_MMTRIG_H_
