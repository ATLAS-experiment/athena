// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef ADDTRACKSUMMARYALG_H
#define ADDTRACKSUMMARYALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkTrack/TrackCollection.h"
#include "AthContainers/ConstDataVector.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk {

/**
 * @brief Algorithm to add TrackSummary to Track collections read from file
 * 
 * This algorithm is needed for track overlay where pileup tracks are read from file
 * and don't have TrackSummary objects (which are not persisted with Track EDM).
 * It loops through the input track collection and calls computeAndReplaceTrackSummary
 * on each track to create the summary.
 */
class AddTrackSummaryAlg : public AthReentrantAlgorithm {
public:
  AddTrackSummaryAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~AddTrackSummaryAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  /// Input track collection (read-only)
  SG::ReadHandleKey<TrackCollection> m_inputTrackCollection{
    this, "InputTrackCollection", "", "Input track collection to add summaries to"};
  
  /// Output track collection with summaries added
  SG::WriteHandleKey<ConstDataVector<TrackCollection>> m_outputTrackCollection{
    this, "OutputTrackCollection", "", "Output track collection with summaries added"};

  /// Track summary tool
  ToolHandle<Trk::ITrackSummaryTool> m_trackSummaryTool{
    this, "TrackSummaryTool", "", "Track summary tool"};

  //Make AddTrackSummaryAlg only run for pileup tracks that could possibly pass the extension/scoring preselection
   Gaudi::Property<float> m_minPt{this, "MinPt", 0.0, "Only add summary if pT > MinPt [MeV]"};
   Gaudi::Property<float> m_maxAbsEta{this, "MaxAbsEta", 99.0, "Only add summary if |eta| < MaxAbsEta"};
};

} // namespace Trk

#endif // ADDTRACKSUMMARYALG_H

