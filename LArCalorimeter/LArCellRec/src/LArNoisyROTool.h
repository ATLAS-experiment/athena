///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// LArNoisyROTool.h
// Header file for class LArNoisyROTool
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef LARCELLREC_LARNOISYROTOOL_H
#define LARCELLREC_LARNOISYROTOOL_H 1


// FrameWork includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloInterface/ILArHVMapTool.h"
#include "CaloInterface/ILArNoisyROTool.h"

#include "Identifier/HWIdentifier.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRecConditions/LArHVIdMapping.h"
#include "LArRecEvent/LArNoisyROSummary.h"
#include "StoreGate/ReadCondHandleKey.h"

// STL includes
#include <string>
#include <set>
#include <array>
#include <unordered_map>
#include <memory>

class LArOnlineID;
class CaloCell_ID;
class LArOnOffIdMapping;
class CaloCellContainer;
class LArElectrodeID;
class LArHVNMap;

class LArNoisyROTool : public extends<AthAlgTool, ILArNoisyROTool> {
 public:
  // delegate constructor
  using base_class::base_class;

  /// Destructor:
  virtual ~LArNoisyROTool() = default;

   // Athena algtool's Hooks
  virtual StatusCode initialize();

  virtual std::unique_ptr<LArNoisyROSummary> process(const EventContext&, const CaloCellContainer*, const std::set<unsigned int>*,
                                                     const std::vector<HWIdentifier>*, const LArHVNMap*, const CaloDetDescrManager*,
                                                     const LArHVIdMapping*) const;

 private:  // classes
  // this class accumulates the number of bad channel in a FEB, per preamp in a FEB
  class FEBEvtStat{
  public:
    FEBEvtStat() = default;

    void addBadChannel(unsigned int channel){
      m_chanCounter++;
      unsigned int preamp = channel / 4;
      m_PAcounters[preamp]++;
    }

    void resetCounters(){
      m_chanCounter = 0;
      for (size_t i = 0; i < 32; i++)
        m_PAcounters[i] = 0;
    }

    unsigned int badChannels() const { return m_chanCounter; }
    const unsigned int* PAcounters() const { return &m_PAcounters[0]; }


   private:
    unsigned int m_chanCounter{};
    unsigned int m_PAcounters[32]{};
  };

  size_t partitionNumber(const HWIdentifier) const;

  typedef std::unordered_map<unsigned int, FEBEvtStat> FEBEvtStatMap;

  std::unordered_map<unsigned int, unsigned int> m_mapPSFEB;

  typedef std::unordered_map<HWIdentifier, unsigned int> HVlinesStatMap;

 private:
  ToolHandle<ILArHVMapTool> m_hvMapTool{"LArHVMapTool"};

  const CaloCell_ID* m_calo_id = nullptr;
  const LArOnlineID* m_onlineID = nullptr;
  const LArElectrodeID* m_elecID = nullptr;
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "CablingKey", "LArOnOffIdMap", "key to read OnOff mapping"};

  Gaudi::Property<unsigned int> m_CellQualityCut{this, "CellQualityCut", 4000, "Qfactor value above which a channel is considered bad"};

  Gaudi::Property<bool> m_ignore_masked_cells{this, "IgnoreMaskedCells", false, "ignore masked cells"};

  Gaudi::Property<bool> m_ignore_front_innerwheel_cells{this, "IgnoreFrontInnerWheelCells", true, "ignore front inner wheel cells ?"};

  Gaudi::Property<unsigned int> m_BadChanPerFEB{this, "BadChanPerFEB", 30, "number of bad channels to declare a FEB noisy"};

  Gaudi::Property<unsigned int> m_MinBadFEB{this, "BadFEBCut", 3, "min number of bad FEB to put LAr warning in event info"};

  Gaudi::Property<unsigned int> m_SaturatedCellQualityCut{this, "SaturatedCellQualityCut", 65535,
                                                          " Qfactor value above which (>=) a channel is considered with a saturated Qfactor"};

  Gaudi::Property<float> m_SaturatedCellEnergyTightCut{this, "SaturatedCellEnergyTightCut", 1000.,
                                                       "Count saturated Qfactor cells above this energy cut (absolute value)"};

  Gaudi::Property<unsigned int> m_SaturatedCellTightCut{this, "SaturatedCellTightCut", 20, "min number of saturated Qfactor cells to declare an event bad"};

  Gaudi::Property<bool> m_doHVline{this, "DoHVflag", true, "do HVline flagging"};

  Gaudi::Property<float> m_BadChanFracPerHVline{this, "BadChanFracPerHVline", 0.25, "fraction of bad cells in one HV line"};

  Gaudi::Property<unsigned int> m_MinBadHV{this, "BadHVCut", 3, " min number of bad HV lines"};

  Gaudi::Property<unsigned int> m_MNBLooseCut{this, "MNBLooseCut", 5, "Loose cut on number of cells above CellQualityCut"};
  Gaudi::Property<unsigned int> m_MNBTightCut{this, "MNBTightCut", 17, "Thight cut on number of cells above CellQualityCut"};
  Gaudi::Property<std::vector<unsigned int> > m_MNBTight_PsVetoCut{this, "MNBTight_PsVetoCut", {13, 3}};

  std::array<uint8_t, 4> m_partitionMask{
      {LArNoisyROSummary::EMECAMask, LArNoisyROSummary::EMBAMask, LArNoisyROSummary::EMBCMask, LArNoisyROSummary::EMECCMask}};
  // beware: The order matters!
};

inline size_t LArNoisyROTool::partitionNumber(const HWIdentifier hwid) const {
  int pn=m_onlineID->pos_neg(hwid);

  if (m_onlineID->isEMECchannel(hwid)) {
    if (pn)
      return 0;  // positive EMECA side
    else
      return 3;  // negative EMECC side
  }
  if (m_onlineID->isEMBchannel(hwid)) {
    if (pn)
      return 1;  // positive EMBA side
    else
      return 2;  // negative EMBC side
  }
  return 4;//Anything else
}

#endif //> !LARCELLREC_LARNOISYROTOOL_H
