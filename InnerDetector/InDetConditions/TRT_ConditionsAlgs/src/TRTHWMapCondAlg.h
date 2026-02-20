/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRTHWMAPCONDALG_H
#define TRTHWMAPCONDALG_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "Gaudi/Property.h"
#include "TRT_ConditionsData/HWMap.h"

class TRTHWMapCondAlg : public AthCondAlgorithm
{
 public:
  TRTHWMapCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TRTHWMapCondAlg() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

  StatusCode build_BarrelHVLinePadMaps(const EventContext& ctx, EventIDRange& range, TRTCond::HWMap* writeCdo) const;
  StatusCode build_EndcapHVLinePadMaps(const EventContext& ctx, EventIDRange& range, TRTCond::HWMap* writeCdo) const;
  int hashThisBarrelPad( int sector, int module, int padNum ) const;
  int hashThisEndcapCell( int sector, int wheel, int layer, int cellNum ) const;

 private:
  SG::ReadCondHandleKey<CondAttrListCollection> m_BarrelReadKey{this,"BarrelHWReadKey","TRT/DCS/HV/BARREL","Barrel HV in-key"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_EndAReadKey{this,"EndcapAHWReadKey","TRT/DCS/HV/ENDCAPA","EndcapA HV in-key"};
  SG::ReadCondHandleKey<CondAttrListCollection> m_EndCReadKey{this,"EndcapCHWReadKey","TRT/DCS/HV/ENDCAPC","EndcapC HV in-key"};
  SG::WriteCondHandleKey<TRTCond::HWMap> m_WriteKey{this,"HWMapWriteKey","HWMap","HWMap out-key"};

};
#endif
