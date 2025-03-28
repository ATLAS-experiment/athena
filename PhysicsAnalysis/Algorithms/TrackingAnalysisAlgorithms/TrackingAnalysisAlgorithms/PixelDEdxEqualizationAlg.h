// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONALG_H
#define TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
//#include <AnaAlgorithm/AnaAlgorithm.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/TrackParticleAuxContainer.h>

#include <AsgTools/PropertyWrapper.h>
#include <AthContainers/ConstDataVector.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/ReadHandleKeyArray.h>
#include <AsgDataHandles/ReadDecorHandleKeyArray.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/ReadHandle.h>
#include "PathResolver/PathResolver.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandle.h"

#include "TrkAnalysisInterfaces/IPixelToTPIDDualTool.h"

#include <string>
#include <vdt/vdtMath.h> // for RDataFrame

#include <TString.h>
#include <ROOT/RDataFrame.hxx>

namespace CP {

  /// \brief an algorithm to calculate equalized pixel dE/dx. 

  /// Algorithm to decorate tracks with corrected pixel dE/dx measurements.
  ///
  /// Scale factors are applied to either track-level truncatd mean dE/dx...
  /// or to individual pixel cluster dE/dx measurements.
  /// These scale factors equalize the dE/dx measurments throughout time (lumi).
  ///
  /// @author Ian Dyckes
  /// @author Simone Pagan Griso
  ///
  class PixelDEdxEqualizationAlg final : public EL::AnaReentrantAlgorithm {

  public:
    /// Algorithm constructor
    PixelDEdxEqualizationAlg( const std::string& name, ISvcLocator* svcLoc );

    /// Function initialising the algorithm
    StatusCode initialize() override;

    /// Function executing the algorithm
    StatusCode execute(const EventContext& ctx) const override;

  /*
  class PixelDEdxEqualizationAlg final : public EL::AnaAlgorithm {
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
  */


  private:

    ToolHandle<CP::IPixelToTPIDDualTool> m_pixelToTPIDDualTool{this, "PixelToTPIDDualTool", "", "tool for pixel dE/dx"};
    //ToolHandle<CP::IPixelToTPIDDualTool> m_pixelToTPIDDualTool{this, "PixelToTPIDDualTool", "CP::PixelToTPIDDualTool/PixelToTPIDDualTool", "tool for pixel dE/dx"}; // ?

    /// @name Algorithm properties
    /// @{
    // Declare the algorithm's properties:

    /// Input track collection to decorate
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackContainerName {
    this, "TrackContainerName", "InDetTrackParticles", "Input track collection to decorate with corrected dE/dx measurements."};

    /// Decorators
    /// Equalized dE/dx.  Provide the variable name without the container.  Container will be set dynamically in initialize.  Form will be <container>.<m_dEdxEqKey>.
    Gaudi::Property<std::string>  m_dEdxEqVarName
    { this, "dEdxEqVarName", "dEdxEq", "Variabel name for the equalized pixel dE/dx attribute" };
    /// Declaire WriteDectorHandleKey but set dynamically in initialize once track container is known...
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_dEdxEqKey; 
    // SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_dEdxEqKey{this, "dEdxEqName", "dEdxEq", "SG key for the equalized pixel dE/dx attribute"}; //set container  dynamically in initialize.

    /// Counters.  Maybe drop?
    mutable std::atomic<unsigned long> m_nEventsProcessed{};
    mutable std::atomic<unsigned long> m_nTracksProcessed{};

  }; // class PixelDEdxEqualizationAlg

} // namespace CP

#endif // TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONALG_H

