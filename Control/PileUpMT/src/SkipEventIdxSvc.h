/* -*- C++ -*- */
/*
 * Copyright (C) 2023 CERN for the benefit of the ATLAS collaboration.
 */
#ifndef PILEUPMT_SKIPEVENTIDXSVC_H
#define PILEUPMT_SKIPEVENTIDXSVC_H

#include <format>

#include "AthenaBaseComps/AthService.h"
#include "ISkipEventIdxSvc.h"

class SkipEventIdxSvc
    : public extends<AthService, IIncidentListener, ISkipEventIdxSvc> {
 public:
  SkipEventIdxSvc(const std::string& name, ISvcLocator* svc);
  ~SkipEventIdxSvc() override = default;

  StatusCode initialize() override;
  StatusCode start() override;
  StatusCode registerCallback(
      std::function<StatusCode(EvtIter, EvtIter)>&& callback) override;
  void handle(const Incident& inc) override;

 private:
  int m_initial_skip_events{};
  std::vector<EvtId> m_events{};
  std::vector<std::function<StatusCode(EvtIter, EvtIter)>> m_callbacks{};
  bool m_started = false;
};

#endif  // PILEUPMT_SKIPEVENTIDXSVC_H
