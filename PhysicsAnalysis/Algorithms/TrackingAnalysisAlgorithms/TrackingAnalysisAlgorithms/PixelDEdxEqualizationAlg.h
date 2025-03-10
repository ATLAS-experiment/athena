// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONALG_H
#define TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
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

  private:

    ToolHandle<CP::IPixelToTPIDDualTool> m_pixelToTPIDDualTool{this, "pixelToTPIDDualTool", "", "tool for pixel dE/dx"};

    /// @name Algorithm properties
    /// @{
    // Declare the algorithm's properties:

    /* GIVE TO TOOL INSTEAD
    /// Switching to PathResolver?
    StringProperty m_scaleFactorTreePath
    { this, "ScaleFactorTreePath", "", "Path to ROOT file containing the scale factor trees." }; // default to somewhere on CVMFS eventually?
    
    StringProperty m_scaleFactorTreeName
    { this, "ScaleFactorTreeName", "SFs_TTree", "Name of TTree containing the scale factors." }; // default to somewhere on CVMFS eventually?
    */
    

    // SysListHandle m_systematicsList {this};

    /// Input track collection to decorate
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputTrackParticles {
      this, "InputTrackParticles", "InDetTrackParticles", "Input track collection to decorate with corrected dE/dx measurements."};

    // SysReadHandle<xAOD::TrackParticleContainer> m_inputTrackParticles {
    //   this, "InputTrackParticles", "InDetTrackParticles", "Input track collection to decorate with corrected dE/dx measurements."};

    /// Decorators
    // SysWriteDecorHandle<float> m_pixeldEdxEqualDecor {
    //   this, "pixeldEdxEqualDecor", "", "the decoration for the equalized truncated mean dE/dx."};
    // }

    /// Counters
    mutable std::atomic<unsigned long> m_nEventsProcessed{};
    mutable std::atomic<unsigned long> m_nTracksProcessed{};

    /*
    /// RDataFrame that will contain the scale factors. JUST LOAD IN TOOL
    std::unique_ptr<ROOT::RDataFrame> m_df;  // Store the RDataFrame in memory
    */

  }; // class PixelDEdxEqualizationAlg

} // namespace CP

#endif // TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONALG_H

