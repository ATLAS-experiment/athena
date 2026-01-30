/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
///////////////////////////////////////////////////////////////////
#ifndef HIEVENTUTILS_HIEVENTFILTERALGRUN3_H
#define HIEVENTUTILS_HIEVENTFILTERALGRUN3_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AsgDataHandles/ReadHandleKey.h"
#include "AthenaBaseComps/AthFilterAlgorithm.h"
#include "HIEventUtils/IHIEventSelectionToolRun3.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

namespace HI {

class HIEventFilterAlgRun3 : public ::AthFilterAlgorithm {

 public:
  HIEventFilterAlgRun3(const std::string& name, ISvcLocator* pSvcLocator);

  virtual ~HIEventFilterAlgRun3() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

 private:
  using mask_t = unsigned int;

  Gaudi::Property<bool> m_doFilter{this, "doFilter", true,
                                   "When false no filtering is actually done"};
  Gaudi::Property<mask_t> m_selectionMask{
      this, "SelectionMask",
      static_cast<mask_t>(HI::SelectionMask::NoEventError),
      "Mask required to pass the event, by default only NoEventError is "
      "required"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
      this, "EventInfo", "EventInfo", "EventInfo key"};
  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tracksKey{
      this, "Tracks", "InDetTrackParticle", "Tracks key"};
  SG::ReadHandleKey<xAOD::VertexContainer> m_verticesKey{
      this, "Vertices", "Vertices", "Vertices key"};
  SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_hiEventShapeKey{
      this, "HIEventShape", "HIEventShape", "Vertices key"};
  SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcKey{
      this, "ZDC", "ZDCModules", "Vertices key"};

  SG::WriteDecorHandleKey<xAOD::EventInfo> m_decisionBitsKey{
      this, "HIEventSelection", m_eventInfoKey, "HIEventSelection",
      "Name of HI EventSelection info decoration in EventInfo"};

  ToolHandle<HI::IHIEventSelectionToolRun3> m_tool{this, "SelectionTool",
                                                   "HIEventSelectionToolRun3"};

  auto isRequested(HI::SelectionMask m) const {
    return (m_selectionMask & static_cast<mask_t>(m)) != 0;
  }

  void store(HI::SelectionMask m, mask_t& mask) {
    mask |= static_cast<mask_t>(m);
  }
};
}  // namespace HI
#endif  //> !HIEVENTUTILS_HIEVENTFILTERALGRUN3_H
