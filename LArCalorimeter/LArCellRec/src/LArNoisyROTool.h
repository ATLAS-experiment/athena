///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// LArNoisyROTool.h 
// Header file for class LArNoisyROTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef LARCELLREC_LARNOISYROTOOL_H
#define LARCELLREC_LARNOISYROTOOL_H 1



// FrameWork includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloInterface/ILArNoisyROTool.h"
#include "CaloInterface/ILArHVMapTool.h"

#include "Identifier/HWIdentifier.h"
#include "LArIdentifier/LArOnlineID.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArHVIdMapping.h"

// STL includes
#include <string>
#include <set>
#include <array>
#include <unordered_map>
#include <memory>

class LArOnlineID;
class CaloCell_ID;
class LArOnOffIdMapping;
class LArNoisyROSummary;
class CaloCellContainer;
class LArElectrodeID;
class LArHVNMap;

class LArNoisyROTool: 
  virtual public ILArNoisyROTool,
  public AthAlgTool
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  LArNoisyROTool() = delete;
  /// Constructor with parameters: 
  LArNoisyROTool( const std::string& type,
		  const std::string& name, 
		  const IInterface* parent );

  /// Destructor: 
  virtual ~LArNoisyROTool() = default; 

  static const InterfaceID& interfaceID();

  // Athena algtool's Hooks
  virtual StatusCode  initialize();

  virtual 
  std::unique_ptr<LArNoisyROSummary> 
  process(const EventContext&, const CaloCellContainer*, const std::set<unsigned int>*, 
    const std::vector<HWIdentifier>*, const LArHVNMap*, const CaloDetDescrManager*, 
    const LArHVIdMapping*) const;

 private: 

  // Containers

  //enum LARFLAGREASON { BADFEBS=0, MEDIUMSATURATEDQ=1, TIGHTSATURATEDQ=2, BADFEBS_W=3} ;

 private:  // classes

  // this class accumulates the number of bad channel in a FEB, per preamp in a FEB
  class FEBEvtStat{
  public:
    FEBEvtStat() = default;

    void addBadChannel(unsigned int channel){
      m_chanCounter++;
      unsigned int preamp = channel/4;
      m_PAcounters[preamp]++;
    }

    void resetCounters(){
      m_chanCounter = 0;
      for ( size_t i = 0; i < 32; i++ ) m_PAcounters[i] = 0;
    }

    unsigned int badChannels() const { return m_chanCounter; }
    const unsigned int* PAcounters() const { return &m_PAcounters[0]; }
  private:
    unsigned int m_chanCounter{};
    unsigned int m_PAcounters[32]{};
  };

  size_t partitionNumber(const HWIdentifier) const;
  typedef std::unordered_map<unsigned int, FEBEvtStat> FEBEvtStatMap;
  typedef std::unordered_map<unsigned int, FEBEvtStat>::iterator FEBEvtStatMapIt;
  typedef std::unordered_map<unsigned int, FEBEvtStat>::const_iterator FEBEvtStatMapCstIt;
  std::unordered_map<unsigned int,unsigned int> m_mapPSFEB;
  typedef std::unordered_map<HWIdentifier, unsigned int> HVlinesStatMap;

 private: 
  ToolHandle<ILArHVMapTool> m_hvMapTool;
  const CaloCell_ID* m_calo_id{};
  const LArOnlineID* m_onlineID{};
  const LArElectrodeID* m_elecID{};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey {this, "CablingKey", "LArOnOffIdMap", "key to read OnOff mapping"};

  //** Qfactor value above which a channel is considered bad */
  unsigned int m_CellQualityCut{};

  //** ignore masked cells ? */
  bool m_ignore_masked_cells{};

  //** ignore front inner wheel cells ? */
  bool m_ignore_front_innerwheel_cells{};

  //** number of bad channels to declare a preamp noisy */
  unsigned int m_BadChanPerPA = 0U;

  //** number of bad channels to declare a FEB noisy */
  unsigned int m_BadChanPerFEB{};

  //** min number of bad FEB to put LAr warning in event info */
  unsigned int m_MinBadFEB{};


  //** Qfactor value above which (>=) a channel is considered with a saturated Qfactor*/
  unsigned int m_SaturatedCellQualityCut{};

  //** Count saturated Qfactor cells above this energy cut (absolute value)*/
  float m_SaturatedCellEnergyTightCut{};

  //** min number of saturated Qfactor cells to declare an event bad */
  unsigned int m_SaturatedCellTightCut{};

  //** Count events with too many saturated Qfactor cells */
  unsigned int m_SaturatedCellTightCutEvents = 0U;

  //** do HVline flagging
  bool m_doHVline{};

  //** fraction of bad cells in one HV line
  float m_BadChanFracPerHVline{};

  //** min. number of bad HV lines
  unsigned int m_MinBadHV{};
  unsigned int m_MNBLooseCut{};
  unsigned int m_MNBTightCut{};
  std::vector<unsigned int> m_MNBTight_PsVetoCut;
  std::array<uint8_t,4> m_partitionMask{};

}; 




inline size_t LArNoisyROTool::partitionNumber(const HWIdentifier hwid) const {
  int pn=m_onlineID->pos_neg(hwid);
  if (m_onlineID->isEMECchannel(hwid)) {
    if (pn) 
      return 0; //positive EMECA side
    else
      return 3; //negative EMECC side
  }
  if (m_onlineID->isEMBchannel(hwid)) {
    if (pn) 
      return 1; //positive EMBA side
    else
      return 2; //negative EMBC side
  }
  return 4;//Anything else
}

#endif //> !LARCELLREC_LARNOISYROTOOL_H
