/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTHALGS_TRACKRECORDFILTER_H
#define MCTRUTHALGS_TRACKRECORDFILTER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"


// the TrackRecordCollection
#include "TrackRecord/TrackRecordCollection.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include <string>

class TrackRecordFilter : public AthReentrantAlgorithm {


public:
  TrackRecordFilter (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TrackRecordFilter(){};

  StatusCode initialize() override;
  StatusCode finalize() override;
  StatusCode execute(const EventContext& ctx) const override;

private:
  SG::ReadHandleKey<TrackRecordCollection> m_inputKey{this, "inputName", "MuonEntryLayer"};
  SG::WriteHandleKey<TrackRecordCollection> m_outputKey{this, "outputName", "MuonEntryLayerFilter"};
  DoubleProperty m_cutOff{this, "threshold", 100.*CLHEP::MeV};
};

#endif
