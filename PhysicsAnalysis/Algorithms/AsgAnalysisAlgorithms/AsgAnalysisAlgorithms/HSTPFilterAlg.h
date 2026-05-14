/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
    Implemented by Zackary Alegria following instructions from the JETM
    twiki recommendation on Dijet sample normalization
    https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/JetEtMissMCSamples#Dijet_normalization_procedure_HS
    Updated by Baptiste Ravina <baptiste.ravina@cern.ch>
*/

#ifndef HSTPFILTERALG_H
#define HSTPFILTERALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <EventBookkeeperTools/FilterReporterParams.h>

// Framework includes
#include <xAODJet/JetContainer.h>

namespace CP {
  
class HSTPFilterAlg final : public EL::AnaReentrantAlgorithm {
  
 public:
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute(const EventContext &ctx) const final;
  virtual StatusCode finalize() override;

  private:
  SG::ReadHandleKey<xAOD::JetContainer> m_truthHSCollection{
      this, "truthHSCollection", "", "the name of the truth HS jet collection"};

  SG::ReadHandleKey<xAOD::JetContainer> m_truthPUCollection{
      this, "truthPUCollection", "", "the name of the truth PU jet collection"};

  FilterReporterParams m_filterParams{
    this, "HSTPFilterSelection", "HS softer than PU event filter"};
};
}  // namespace CP

#endif
