#ifndef MUONALIGNERRORTOOL_ALIGNMENTERRORTESTALG_H
#define MUONALIGNERRORTOOL_ALIGNMENTERRORTESTALG_H

#include <GaudiKernel/ToolHandle.h>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "TrkToolInterfaces/ITrkAlignmentDeviationTool.h"
#include "TrkTrack/TrackCollection.h"
#include "MuonCalibITools/IIdToFixedIdTool.h"
#include "xAODEventInfo/EventInfo.h"

namespace MuonAlign {

class AlignmentErrorTestAlg : public AthAlgorithm {
 public:
  AlignmentErrorTestAlg(const std::string& name, ISvcLocator* pSvcLocator);
  ~AlignmentErrorTestAlg() override = default;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) override;

 private:
  ToolHandle<Trk::ITrkAlignmentDeviationTool> m_alignmentErrorTool{
      this, "alignmentErrorTool", "MuonAlign::AlignmentErrorTool"};
  ToolHandle<MuonCalib::IIdToFixedIdTool> m_idTool{this, "idTool", "MuonCalib::IdToFixedIdTool"};
  SG::ReadHandleKey<::TrackCollection> m_trackCollection{
      this, "trackCollection", "MuonSpectrometerTracks"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
      this, "EvtInfo", "EventInfo", "EventInfo name"};
};

}  // namespace MuonAlign

#endif
