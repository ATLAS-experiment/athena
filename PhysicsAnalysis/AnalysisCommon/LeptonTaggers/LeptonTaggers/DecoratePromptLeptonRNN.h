// This is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DECORATELEPTONTAGGERRNN_H
#define DECORATELEPTONTAGGERRNN_H

/**********************************************************************************
 * @Package: PhysicsAnpProd
 * @Class  : DecoratePromptLeptonRNN
 * @Author : Rustem Ospanov
 * @Author : Fudong He
 *
 * @Brief  : Algorithm to compute RNN score for electrons and muons
 *
 **********************************************************************************/

// Local
#include "IRNNTool.h"

// ROOT
#include "TStopwatch.h"

// Gaudi
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ToolHandle.h"

// Athena
#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include <unordered_map>

namespace Prompt
{
  // Forward declaration
  class VarHolder;

  // Main body

  class DecoratePromptLeptonRNN : public AthAlgorithm
  {
    /*
      Select the ID tracks near to the lepton, and prepare the RNN inputs variables of those ID tracks to the VarHolder objects.
      Then pass the VarHolder objects to RNNTool class to get the RNN prediction.
      Decorate the lepton with the RNN predictions.

    */
  public:

    DecoratePromptLeptonRNN(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

    using decoratorFloatH_t = SG::WriteDecorHandle<xAOD::IParticleContainer, float>;

    const xAOD::TrackParticle* findMuonTrack(const xAOD::Muon *muon);

    const xAOD::Jet* findClosestTrackJet(const xAOD::TrackParticle *particle, const xAOD::JetContainer &trackJets);

    bool compDummy(const xAOD::IParticle &particle,
                   std::vector<decoratorFloatH_t>& decors) const;

    bool prepTrackObject(Prompt::VarHolder         &p,
                         const xAOD::TrackParticle &track,
                         const xAOD::TrackParticle &lepton,
                         const xAOD::Jet           &trackJet,
                         const xAOD::Vertex        &priVtx,
       const xAOD::EventInfo     event);

    bool compScore(const xAOD::IParticle                &particle,
                   const std::vector<Prompt::VarHolder> &tracks,
                   std::vector<decoratorFloatH_t>& decors);

    bool passTrack(Prompt::VarHolder &p);

    StatusCode makeHist(TH1 *&h, const std::string &key, int nbin, double xmin, double xmax);

    // Properties:
    Gaudi::Property<std::string> m_outputStream {this, "outputStream", "", "Path of the ROOT output directory of the histograms for RNN debug"};
    Gaudi::Property<std::string> m_decorationPrefixRNN {this, "decorationPrefixRNN", "", "Prefix of the name for the decorator of RNN to the lepton"};

    Gaudi::Property<double> m_minTrackpT {this, "minTrackpT", 500.0, "pT cut config for the input tracks to RNN"};
    Gaudi::Property<double> m_maxTrackEta {this, "maxTrackEta", 2.5, "abs(eta) cut config for the input tracks to RNN"};
    Gaudi::Property<double> m_maxTrackZ0Sin {this, "maxTrackZ0Sin", 1.0, "Z0sin cut config for the input tracks to RNN"};

    Gaudi::Property<double> m_minTrackLeptonDR {this, "minTrackLeptonDR", 1.0e-6, "Delta R between lepton and track cut config for the input tracks to RNN"};
    Gaudi::Property<double> m_maxTrackLeptonDR {this, "maxTrackLeptonDR", 0.4, "Delta R between lepton and track cut config for the input tracks to RNN"};
    Gaudi::Property<double> m_maxLepTrackJetDR {this, "maxLepTrackJetDR", 0.4, "Maximum distance between lepton and track jet for track jet matching"};

    Gaudi::Property<double> m_maxTrackSharedSiHits {this, "maxTrackSharedSiHits", 1.5, "track shared si hits cut config for the input tracks to RNN"};
    Gaudi::Property<unsigned> m_minTrackSiHits {this, "minTrackSiHits", 6.5, "track silicon detector hits cut config for the input tracks to RNN"};
    Gaudi::Property<unsigned> m_maxTrackSiHoles {this, "maxTrackSiHoles", 2.5, "track holes cut config for the input tracks to RNN"};
    Gaudi::Property<unsigned> m_maxTrackPixHoles {this, "maxTrackPixHoles", 1.5, "track pixel holes cut config for the input tracks to RNN"};

    Gaudi::Property<bool> m_debug {this, "debug", false, "debug statement"};
    Gaudi::Property<bool> m_printTime {this, "printTime", false, "print running time, for debug"};

    // Tools and services:
    ToolHandle<IRNNTool> m_toolRNN {
      this, "toolRNN", "defaultToolRNN", "Dedicated tool for RNN prediction"
    };

    ServiceHandle<ITHistSvc>                          m_histSvc;

    // Read/write handles:
    SG::ReadHandleKey<xAOD::IParticleContainer> m_inputContainerLeptonKey {
      this, "inputContainerLepton", "",
      "Container's name of the lepton that you want to decorate"
    };
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inputContainerTrackKey {
      this, "inputContainerTrack", "", "Track container name"
    };
    SG::ReadHandleKey<xAOD::JetContainer> m_inputContainerTrackJetKey {
      this, "inputContainerTrackJet", "", "Track Jet container name"
    };
    SG::ReadHandleKey<xAOD::VertexContainer> m_inputContainerPrimaryVerticesKey {
      this, "inputContainerPrimaryVertices", "", "Primary vertex container name"
    };

    SG::ReadHandleKey<xAOD::EventInfo> m_eventHandleKey {
      this, "EventHandleKey", "EventInfo"
    };

    // Variables:
    TStopwatch                                        m_timerEvent;
    int                                               m_countEvent;

    std::map<std::string, TH1*>                       m_hists;

    std::unique_ptr<SG::AuxElement::ConstAccessor<unsigned char> > m_accessQuality;

    SG::WriteDecorHandleKeyArray<xAOD::IParticleContainer> m_decorHandleKeys
      { this, "DecorHandleKeys", {} };
    std::unordered_map<std::string, size_t> m_decorNameMap;
  };
}

#endif
