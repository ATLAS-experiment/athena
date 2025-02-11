/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IsolationTrackDecorator.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

/*
  Decorates the track isolations and calo isolations to the track container.
  ptcone, ptvarcone, etcone, and etcluster are decorated.
  if the etcone is selected, the etcluster is also decorated automatically.

  user specifies...

  TrackIsolationTool: track isolation tool. ex.) from IsolationAlgs.IsoToolsConfig import TrackIsolationToolCfg
  CaloIsolationTool:  calo isolation tool. ex.) from IsolationAlgs.IsoToolsConfig import CaloIsolationToolCfg
  TargetContainer:    track particle container.
  SelectionString:    the selections for the tracks that are decorated. ex.) "InDetTrackParticles.pt>10*GeV".
  iso:                isolation type. ex.) isoPar = ROOT.xAOD.Iso.IsolationType; iso = [isoPar.ptcone40, isoPar.ptcone30]
  Prefix & isoSufix:  prefix and sufix for the decoration, decoraterKey = trackContainerKey + prefix + isosufix.
*/

#ifndef DERIVATIONFRAMEWORK_IsolationTrackDecorator_H
#define DERIVATIONFRAMEWORK_IsolationTrackDecorator_H

#include<string>
#include<vector>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "RecoToolInterfaces/ITrackIsolationTool.h"
#include "RecoToolInterfaces/ICaloTopoClusterIsolationTool.h"
#include "ExpressionEvaluation/ExpressionParserUser.h"
#include "xAODPrimitives/IsolationHelpers.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

namespace DerivationFramework {
  /** @class IsolationTrackDecorator
      @author daiya.akiyama@cern.ch
  */
  class IsolationTrackDecorator : public extends<ExpressionParserUser<AthAlgTool>, IAugmentationTool> {
    
  public: 
    /** Constructor with parameters */
    IsolationTrackDecorator( const std::string& t, const std::string& n, const IInterface* p);
    
    /** Destructor */
    virtual ~IsolationTrackDecorator() = default;
 
    // Athena algtool's Hooks
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    
    virtual StatusCode addBranches() const override;
    
  private:
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackContainerKey {
      this, "TargetContainer", "InDetTrackParticles", "track particle container name"};
    StringProperty m_selectionString {
      this, "SelectionString", "", "selection string"};
    StringProperty m_prefix {
      this, "Prefix", "", "prefix"};
    StringArrayProperty m_iso_suffix {
      this, "isoSuffix", {""}, "suffix for the isolations"};
    StringProperty m_selFlag {
      this, "SelectionFlag", "", "selection flag"};
    IntegerProperty m_diff_ptvarcone {
      this, "DiffPtvarcone", xAOD::Iso::ptvarcone20 - xAOD::Iso::ptcone20, "difference between Iso::ptvarcone20 and Iso::ptcone20"};
    IntegerProperty m_selFlagValue {
      this, "SelectionFlagValue", 1, "selection flag value"};
  
    /// Athena configured tools
    ToolHandle<xAOD::ITrackIsolationTool> m_trackIsolationTool {this, "TrackIsolationTool", ""};
    ToolHandle<xAOD::ICaloTopoClusterIsolationTool> m_caloIsolationTool {this, "CaloIsolationTool", ""};

    std::vector<xAOD::Iso::IsolationType> m_ptconeTypes;
    std::vector<xAOD::Iso::IsolationType> m_ptvarconeTypes;
    std::vector<xAOD::Iso::IsolationType> m_topoetconeTypes;
    IntegerArrayProperty m_iso {
      this, "iso", {}, "isolation types vector<int>"};
    xAOD::TrackCorrection m_trkCorrList; 
    xAOD::CaloCorrection m_topoconeCorrList;
    xAOD::CaloCorrection m_topoclusCorrList;

    SG::WriteDecorHandleKeyArray<xAOD::TrackParticleContainer> m_ptconeDecoratorsKey {
      this, "ptconeDecoratorsKey", {}};
    SG::WriteDecorHandleKeyArray<xAOD::TrackParticleContainer> m_ptvarconeDecoratorsKey {
      this, "ptvarconeDecoratorsKey", {}};
    SG::WriteDecorHandleKeyArray<xAOD::TrackParticleContainer> m_topoetconeDecoratorsKey {
      this, "topoetconeDecoratorsKey", {}};
    SG::WriteDecorHandleKeyArray<xAOD::TrackParticleContainer> m_topoetconeNonCoreConeDecoratorsKey {
      this, "topoetconeNonCoreConeDecoratorsKey", {}};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_dec_trkFlagKey {
      this, "dec_trkFlagKey", ""};

  }; 
}
#endif
