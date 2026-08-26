/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */

#include "PFTrackCaloExtensionTool.h"

#include "eflowTrackCaloPoints.h"

PFTrackCaloExtensionTool::PFTrackCaloExtensionTool(const std::string& type, const std::string& name, const IInterface* parent)  :
    AthAlgTool(type, name, parent)
{
  declareInterface<eflowTrackExtrapolatorBaseAlgTool>(this);
}

StatusCode PFTrackCaloExtensionTool::initialize() {
  return StatusCode::SUCCESS;
}

std::unique_ptr<eflowTrackCaloPoints> PFTrackCaloExtensionTool::execute(const EventContext& ctx, const xAOD::TrackParticle* track) const {
  return std::make_unique<eflowTrackCaloPoints>();
}

StatusCode PFTrackCaloExtensionTool::finalize() {
  return StatusCode::SUCCESS;
}