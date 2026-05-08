// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file TrkVKalVrtFitter/src/TrkVKalVrtFitterTestAlg.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2019
 * @brief Algorithm for testing TrkVKalVrtFitter.
 */


#ifndef TRKVKALVRTFITTER_TRKVKALVRTFITTERTESTALG_H
#define TRKVKALVRTFITTER_TRKVKALVRTFITTERTESTALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "GaudiKernel/ToolHandle.h"


namespace Trk {


class TrkVKalVrtFitterTestAlg
  : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;


  /// Standard Gaudi initialize method.
  virtual StatusCode initialize() override;

  /// Execute the algorithm.
  virtual StatusCode execute(const EventContext& ctx) const override;


private:
  StatusCode test1(const EventContext& ctx) const;
  StatusCode test2(const EventContext& ctx) const;
  StatusCode test3(const EventContext& ctx) const;
  StatusCode test4() const;

  ToolHandle<Trk::IVertexFitter> m_fitter
  { this, "Tool", "Trk::TrkVKalVertexFitter", "Tool to test." };
};


} // namespace Trk


#endif // not TRKVKALVRTFITTER_TRKVKALVRTFITTERTESTALG_H
