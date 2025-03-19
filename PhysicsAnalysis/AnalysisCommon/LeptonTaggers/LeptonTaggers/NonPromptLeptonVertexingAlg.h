// This is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NONPROMPTLEPTONVERTEXINGALG_H
#define NONPROMPTLEPTONVERTEXINGALG_H

/**********************************************************************************
 * @Package: LeptonTaggers
 * @Class  : NonPromptLeptonVertexingAlg
 * @Author : Fudong He
 * @Author : Rustem Ospanov
 *
 * @Brief  :
 *
 *  Decorate leptons with secondary vertex algorithm output
 *
 **********************************************************************************/

// Local
#include "VertexMergingTool.h"
#include "VertexFittingTool.h"

// Athena
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODMuon/Muon.h"

// ROOT
#include "TStopwatch.h"

// C/C++
#include <set>

namespace Prompt
{
  class NonPromptLeptonVertexingAlg: public AthAlgorithm
  {

  public:

    NonPromptLeptonVertexingAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:

    std::vector<const xAOD::TrackParticle*> findNearbyTracks(
      const xAOD::TrackParticle &tracklep,
      const xAOD::TrackParticleContainer &inDetTracks,
      const xAOD::Vertex &priVtx
    ) const;

    bool passElecCand(const xAOD::Electron &elec) const;
    bool passMuonCand(const xAOD::Muon     &muon) const;

    std::vector<std::unique_ptr<xAOD::Vertex>> prepLepWithTwoTrkSVVec(
      const FittingInput &input,
      const xAOD::TrackParticle *tracklep,
      const std::vector<const xAOD::TrackParticle* > &tracks
    );

    std::vector<std::unique_ptr<xAOD::Vertex>> prepLepWithMergedSVVec(
      const FittingInput &input,
      const xAOD::TrackParticle* tracklep,
      std::vector<std::unique_ptr<xAOD::Vertex>> &twoTrkVertices
    );

    void makeVertexCluster(
      std::vector<std::unique_ptr<xAOD::Vertex>> &clusterVtxs,
      std::vector<std::unique_ptr<xAOD::Vertex>> &inputVtxs
    );

    void saveSecondaryVertices(
      std::vector<std::unique_ptr<xAOD::Vertex>> &vtxs,
      std::vector<int> &indexVector,
      std::vector<ElementLink<xAOD::VertexContainer> > &svLinks,
      xAOD::VertexContainer &SVContainer,
      std::set< xAOD::Vertex* >& svSet
    );

  private:

    typedef SG::AuxElement::Decorator<std::vector<int> > decoratorVecInt_t;
    typedef SG::AuxElement::Decorator<std::vector<ElementLink<xAOD::VertexContainer> > > decoratorVecElemVtx_t;

  private:

    //
    // Tools and services:
    //
    ToolHandle<Prompt::IVertexMergingTool> m_vertexMerger {
      this, "VertexMergingTool",
      "Prompt::VertexMergingTool/PromptVertexMergingTool"
    };
    ToolHandle<Prompt::VertexFittingTool> m_vertexFitterTool {
      this, "VertexFittingTool", "Prompt::VertexFittingTool/VertexFittingTool"
    };

    //
    // Properties:
    //
    Gaudi::Property<bool> m_printTime {this, "PrintTime", false};
    Gaudi::Property<bool> m_selectTracks {this, "SelectTracks", true};

    Gaudi::Property<double> m_mergeMinVtxDist {this, "MergeMinVtxDist", 1.0};
    Gaudi::Property<double> m_mergeChi2OverDoF {this, "MergeChi2OverDoF", 5.0};

    Gaudi::Property<std::string> m_linkNameRefittedPriVtxWithoutLepton {this, "NoLeptonPriVtxLinkName"};

    Gaudi::Property<std::string> m_refittedVertexTypeName{
      this, "ReFitPriVtxTypeName", "refittedVertexType"
    };

    Gaudi::Property<float> m_minTrackpT {this, "minTrackpT", 500.0};
    Gaudi::Property<float> m_maxTrackEta {this, "maxTrackEta", 2.5};
    Gaudi::Property<float> m_maxTrackZ0Sin {this, "maxTrackZ0Sin", 1.0};

    Gaudi::Property<float> m_minTrackLeptonDR {this, "minTrackLeptonDR", 1.0e-6};
    Gaudi::Property<float> m_maxTrackLeptonDR {this, "maxTrackLeptonDR", 0.4};

    Gaudi::Property<unsigned> m_minTrackSiHits {this, "minTrackSiHits", 7};
    Gaudi::Property<float> m_maxTrackSharedSiHits {this, "maxTrackSharedSiHits", 1.0};
    Gaudi::Property<unsigned> m_maxTrackSiHoles {this, "maxTrackSiHoles", 2};
    Gaudi::Property<unsigned> m_maxTrackPixHoles {this, "maxTrackPixHoles", 1};

    // Read/write handles
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inDetTracksKey{
      this, "InDetTrackParticlesKey", "InDetTrackParticles"
    };
    SG::ReadHandleKey<xAOD::IParticleContainer> m_leptonContainerKey {
      this, "LeptonContainerName", "default"
    };
    SG::ReadHandleKey<xAOD::VertexContainer> m_primaryVertexContainerName {
      this, "PriVertexContainerName", "PrimaryVertices"
    };
    SG::ReadHandleKey<xAOD::VertexContainer> m_refittedPriVtxContainerName {
      this, "ReFitPriVtxContainerName", "default"
    };
    SG::WriteHandleKey<xAOD::VertexContainer> m_svContainerName {
      this, "SVContainerName", "default"
    };


    //
    // Variables
    //
    TStopwatch                                             m_timerAll;
    TStopwatch                                             m_timerExec;

    unsigned                                               m_countEvents;

    //
    // Decorators
    //

    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoratorNameSecVtxLinks
      { this, "SecVtxLinksName", m_leptonContainerKey, "default", };
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoratorNameDeepMergedSecVtxLinks
      { this, "DeepMergedSecVtxLinksName", m_leptonContainerKey, "default", };
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoratorNameIndexVector
      { this, "IndexVectorName", m_leptonContainerKey, "", };
    SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoratorNameIndexVectorDeepMerge
      { this, "IndexVectorNameDeepMerge", m_leptonContainerKey, "", };
  };
}

#endif // NONPROMPTLEPTONVERTEXINGALG_H
