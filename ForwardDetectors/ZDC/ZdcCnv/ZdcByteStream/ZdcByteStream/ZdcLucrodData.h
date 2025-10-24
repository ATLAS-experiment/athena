/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_LUCRODDATA_H
#define ZDC_LUCRODDATA_H

// working in ADC scale for Run 3
//#define ADC2MV 0.3663
#define ADC2MV 1 
#include <vector>
#include <cstdint>
#include <sstream>
#include <iostream>

typedef struct ZdcLucrodChannelType {

  unsigned int id;
  std::vector <uint16_t> waveform;
} ZdcLucrodChannel;



class ZdcLucrodData {

 public:
 
  ZdcLucrodData(unsigned int id) { 

    m_lucrodID  = id; 
    m_bcid      = 0;
    m_runNumber = 0;
    m_level1ID  = 0;
    m_numBCs  = 0;    
    m_status    = 0;    

    m_trigAvgA = 0;
    m_trigAvgC = 0;
    
    m_trigData.clear();
    m_chanData.clear(); 
  };
  
  ~ZdcLucrodData() {};

  void SetBCID     (unsigned int val) { m_bcid      = val; }
  void SetRunNumber(unsigned int val) { m_runNumber = val; }
  void SetLevel1ID (unsigned int val) { m_level1ID  = val; }
  void SetNumBCs (unsigned int val)   { m_numBCs  = val; }
  void SetStatus   (unsigned int val) { m_status    = val; }

  void SetTrigAvgA(uint16_t val) { m_trigAvgA = val; }
  void SetTrigAvgC(uint16_t val) { m_trigAvgC = val; }

  void AddTrigData(uint16_t val) { m_trigData.push_back(val); }

  bool AddChanData(unsigned int id, const std::vector<uint16_t>& waveform) { 
    
    ZdcLucrodChannel channel;
    
    channel.id       = id;
    channel.waveform = waveform;

    // Check for valid length
    //
    if (waveform.size() != m_numBCs*8) return false;

    m_chanData.push_back(std::move(channel));
    return true;
  }

  unsigned int GetLucrodID()  const { return m_lucrodID; }
  unsigned int GetBCID()      const { return m_bcid; }
  unsigned int GetRunNumber() const { return m_runNumber; }
  unsigned int GetLevel1ID()  const { return m_level1ID; }
  unsigned int GetNumBCs()  const { return m_numBCs; }
  unsigned int GetStatus()    const { return m_status; }

  uint16_t GetTrigAvgA() const { return m_trigAvgA; }
  uint16_t GetTrigAvgC() const { return m_trigAvgC; }

  const std::vector<uint16_t>& GetTrigData()       const { return m_trigData; }  
  const ZdcLucrodChannel&    GetChanData(int it) const { return m_chanData[it]; }

  unsigned int GetTrigDataSize() const { return m_trigData.size(); }
  unsigned int GetChanDataSize() const { return m_chanData.size(); }

  std::string str() const {
  
    std::ostringstream ost;
    
    ost << " LucrodID:  "   << std::dec << m_lucrodID  << std::endl
	<< " BCID:      "   << std::dec << m_bcid      << std::endl
	<< " RunNumber: "   << std::dec << m_runNumber << std::endl
	<< " Level1ID:  0x" << std::hex << m_level1ID  << std::endl
	<< " NumBCs:  "     << std::dec << m_numBCs  << std::endl
	<< " Status:    "   << std::dec << m_status    << std::endl;

    for (unsigned int nch=0; nch<m_chanData.size(); nch++) 
      {
	
	std::vector<uint16_t> waveform = m_chanData[nch].waveform;
	
	ost << " ch: " << m_chanData[nch].id << " waveform:";
	
	for (unsigned int sample=0; sample<waveform.size(); sample++)
	  ost << " " << waveform[sample]/ADC2MV;
	
	ost << std::endl;
      }

    ost << " trigAvgA: " << m_trigAvgA/ADC2MV << std::endl;
    ost << " trigAvgC: " << m_trigAvgC/ADC2MV << std::endl;
    ost << " trigAmpl:";

    for (unsigned int nch=0; nch<m_trigData.size(); nch++)
      ost << " " << m_trigData[nch]/ADC2MV;

    ost << std::endl << " stat: " << m_status;
    
    return ost.str();
  }
  
  std::ostream& operator<<(std::ostream& os) { os << str(); return os; }
  
 private:

  unsigned int m_lucrodID;
  unsigned int m_bcid;
  unsigned int m_runNumber;
  unsigned int m_level1ID;
  unsigned int m_numBCs;
  unsigned int m_status;

  uint16_t m_trigAvgA;
  uint16_t m_trigAvgC;

  std::vector<uint16_t>           m_trigData;
  std::vector<ZdcLucrodChannel> m_chanData;
};

#endif
