/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef ASG_ANALYSIS_ALGORITHMS__PDFINFO__ALG_H
#define ASG_ANALYSIS_ALGORITHMS__PDFINFO__ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include "xAODTruth/TruthEventContainer.h"
#include <xAODEventInfo/EventInfo.h>

namespace CP {
class PDFinfoAlg : public EL::AnaReentrantAlgorithm {
 public:
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute(const EventContext &ctx) const final;

 private:
  SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEventsKey{
      this, "truthEvents", "TruthEvents", "the name of the truth events container"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
      this, "eventInfo", "EventInfo", "the name of the EventInfo container to decorate"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_pdfid1Key{
      this, "PDFID1DecorKey", m_eventInfoKey, "PDFID1", "the PDFID1 decoration on EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_pdfid2Key{
      this, "PDFID2DecorKey", m_eventInfoKey, "PDFID2", "the PDFID2 decoration on EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_pdgid1Key{
      this, "PDGID1DecorKey", m_eventInfoKey, "PDGID1", "the PDGID1 decoration on EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_pdgid2Key{
      this, "PDGID2DecorKey", m_eventInfoKey, "PDGID2", "the PDGID2 decoration on EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_QKey{
      this, "QDecorKey", m_eventInfoKey, "Q", "the Q decoration on EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_X1Key{
      this, "X1DecorKey", m_eventInfoKey, "X1", "the X1 decoration on EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_X2Key{
      this, "X2DecorKey", m_eventInfoKey, "X2", "the X2 decoration on EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_XF1Key{
      this, "XF1DecorKey", m_eventInfoKey, "XF1", "the XF1 decoration on EventInfo"};
  SG::WriteDecorHandleKey<xAOD::EventInfo> m_XF2Key{
      this, "XF2DecorKey", m_eventInfoKey, "XF2", "the XF2 decoration on EventInfo"};
};

}  // namespace CP

#endif
