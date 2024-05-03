// This is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PROMPT_DECORATEPROMPTLEPTONRNN_H
#define PROMPT_DECORATEPROMPTLEPTONRNN_H

/**********************************************************************************
 * @Package: LeptonTaggers
 * @Class  : DecoratePromptLeptonImproved
 * @Author : Fudong He
 * @Author : Rustem Ospanov
 *
 * @Brief  :
 *
 *  Decorate leptons with prompt BDT output
 *
 **********************************************************************************/

// Local
#include "VarHolder.h"

// Tools
#include "PathResolver/PathResolver.h"

// Athena
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

// xAOD
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/Vertex.h"

// ROOT
#include "TMVA/Reader.h"
#include "TStopwatch.h"
#include "TH1.h"

namespace Prompt
{
  class DecoratePromptLeptonImproved: public AthAlgorithm
  {
  /*
    This class is mainly used to calculate and  decorate the PromptLeptonImproved BDT to the lepton, workflow like:
    1. Calculate the input variables of the BDT:
      1.1 Find the track jet nearby and calculate the variables relate to the track jet.
      1.2 Calculate the secondary vertex variables with the vertices that associate to the lepton.
      1.3 Get the dedicated RNN score of the lepton
    2. Predict the PromptLeptonImproved for electrons or muons by TMVA
    3. Decorate the PromptLeptonImproved BDT and its input variables to the leptons

  */

  public:

    DecoratePromptLeptonImproved(const std::string& name, ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:

    bool initializeTMVAReader();

    StatusCode initializeDecorators();
    void initializeConstAccessors();

    void decorateElec(
      const xAOD::Electron &electron,
      const xAOD::JetContainer &trackJets,
      const xAOD::CaloClusterContainer &clusters,
      const xAOD::Vertex *primaryVertex
    );

    void decorateMuon(
      const xAOD::Muon         &muon,
      const xAOD::JetContainer &trackJets,
      const xAOD::Vertex *primaryVertex
    );

    void getMutualVariables(
      const xAOD::IParticle     &particle,
      const xAOD::Jet           &track_jet,
      const xAOD::TrackParticle *track,
      Prompt::VarHolder         &vars
    );

    void getMuonAnpVariables(
      const xAOD::Muon &muon,
      Prompt::VarHolder &vars,
      const xAOD::Vertex *primaryVertex
    );

    void getElectronAnpVariables(
      const xAOD::Electron             &elec,
      const xAOD::CaloClusterContainer &clusters,
      Prompt::VarHolder                &vars,
      const xAOD::Vertex               *primaryVertex
    );

    float accessIsolation(SG::AuxElement::ConstAccessor<float> &isoAccessor,
        const xAOD::IParticle &particle);

    void addVarsToTMVA(Prompt::VarHolder &vars);

    void fillVarDefault(Prompt::VarHolder &vars) const;

    void decorateAuxLepton(
      const xAOD::IParticle &particle,
      Prompt::VarHolder &vars
    );

    template<class T> std::pair<double, const xAOD::Jet*> findTrackJet(const T &part, const xAOD::JetContainer &jets);

    double getVertexLongitudinalNormDist(const xAOD::IParticle &lepton,
                                         const xAOD::Vertex    *secondaryVertex,
                                         const xAOD::Vertex    *primaryVertex);

    double getVertexCosThetaWithLepDir(const xAOD::IParticle &lepton,
                                       const xAOD::Vertex    *secondaryVertex,
                                       const xAOD::Vertex    *primaryVertex);

    typedef std::map<Prompt::Def::Var, SG::AuxElement::Decorator<short> > shortDecoratorMap;
    typedef std::map<Prompt::Def::Var, SG::AuxElement::Decorator<float> > floatDecoratorMap;

    typedef SG::AuxElement::ConstAccessor<float>                                             AccessFloat;
    typedef SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::VertexContainer> > > AccessVertex;

    typedef std::map<Prompt::Def::Var, AccessFloat> floatAccessorMap;

    // Properties:
    Gaudi::Property<std::string> m_leptonsName {
      this, "LeptonContainerName", "",
      "Container's name of the lepton that you want to decorate. Also need to set ElectronContainerKey or MuonContainerKey accordingly"
    };

    Gaudi::Property<std::string> m_configFileVersion {this, "ConfigFileVersion", "", "BDT weight file version"};
    Gaudi::Property<std::string> m_configPathOverride {this, "ConfigPathOverride", "", "Path of the local BDT weight file you want to study/test"};
    Gaudi::Property<std::string> m_inputVarDecoratePrefix {this, "InputVarDecoratePrefix", "", "Prefix of the variables that will be decorated into the lepton"};
    Gaudi::Property<std::string> m_BDTName {this, "BDTName", "", "BDT name"};
    Gaudi::Property<std::string> m_methodTitleMVA {this, "MethodTitleMVA", "BDT", "Help to config the path of the BDT xml file"};

    Gaudi::Property<std::vector<std::string>> m_accessorRNNVars {this, "accessorRNNVars", {}, "Name of the RNN accessor of the lepton"};
    Gaudi::Property<std::vector<std::string>> m_stringIntVars {this, "stringIntVars", {}, "Vector of the BDT int variables' names and they will be decorated into lepton if not in the veto list"};
    Gaudi::Property<std::vector<std::string>> m_stringFloatVars {this, "stringFloatVars", {}, "Vector of the BDT float variables' names and they will be decorated into lepton if not in the veto list"};
    Gaudi::Property<std::vector<std::string>> m_extraDecoratorFloatVars {this, "extraDecoratorFloatVars", {}, "Extra float variables' names you want to compute and decorate into the lepton"};
    Gaudi::Property<std::vector<std::string>> m_extraDecoratorShortVars {this, "extraDecoratorShortVars", {}, "Extra short variables' names you want to compute and decorate into the lepton"};
    Gaudi::Property<std::vector<std::string>> m_vetoDecoratorFloatVars {this, "vetoDecoratorFloatVars", {}, "Vector of the float variables' names you do not want to save"};
    Gaudi::Property<std::vector<std::string>> m_vetoDecoratorShortVars {this, "vetoDecoratorShortVars", {}, "Vector of the short variables' names you do not want to save"};
    Gaudi::Property<std::vector<double>> m_leptonPtBinsVector {this, "leptonPtBinsVector", {}, "pT bin edges that are used for MVABin calculation"};

    Gaudi::Property<bool> m_printTime {this, "PrintTime", false, "Whether to print current time"};

    Gaudi::Property<std::string> m_vertexLinkName {this, "VertexLinkName", "", "ElementLink name of the secondary vertices"};
    Gaudi::Property<double> m_vertexMinChiSquaredProb {this, "VertexMinChiSquaredProb", 0.03, "Vertex chi2 cut"};
    Gaudi::Property<double> m_vertexMinThetaBarrElec {this, "VertexMinThetaBarrElec", 0.002, "Vertex theta between lepton and the direction of sv-pv cut for barrel electrons"};
    Gaudi::Property<double> m_vertexMinThetaEcapElec {this, "VertexMinThetaEcapElec", 0.001, "Vertex theta between lepton and the direction of sv-pv cut for central electrons"};
    Gaudi::Property<double> m_vertexBarrEcapAbsEtaAt {this, "VertexBarrEcapAbsEtaAt", 1.37, "Relate to the vertex cut above, define the barrel and central electrons by abs(eta)"};
    Gaudi::Property<double> m_elecMinCalErelConeSize {this, "ElecMinCalErelConeSize", 0.15, "Cut of the cluster for calculating the core energy of the lepton"};
    Gaudi::Property<double> m_maxLepTrackJetDR {this, "maxLepTrackJetDR", 0.4, "Maximum distance between lepton and track jet for track jet matching"};

    // Read/write handles
    SG::ReadHandleKey<xAOD::JetContainer> m_trackJetsKey {
      this, "TrackJetContainerName", "", "Track Jet container name"
    };
    SG::ReadHandleKey<xAOD::VertexContainer> m_primaryVertexKey {
      this, "PrimaryVertexContainerName", "", "Primary vertex container name"
    };
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clusterContainerKey {
      this, "ClusterContainerName", "",
      "Container name of the Clusters which are used to calculate the input variables for the PromptLeptonImproved"
    };

    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronsKey {
      this, "ElectronContainerKey", "Electrons",
      "Container's name of the electrons that you want to decorate"
    };
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonsKey {
      this, "MuonContainerKey", "Muons",
      "Container's name of the muons that you want to decorate"
    };


    // Variables:
    std::vector<Prompt::Def::Var>                        m_intVars;
    std::vector<Prompt::Def::Var>                        m_floatVars;
    std::vector<Prompt::Def::Var>                        m_allVars;

    std::unique_ptr<Prompt::VarHolder> m_vars;

    Prompt::Def::Var                                     m_BDTVarKey;

    shortDecoratorMap                                    m_shortMap;
    floatDecoratorMap                                    m_floatMap;

    std::unique_ptr<TMVA::Reader>                        m_TMVAReader;
    std::vector<Float_t>                                 m_varTMVA;

    std::unique_ptr<AccessFloat>                         m_accessCalIsolation30;
    std::unique_ptr<AccessFloat>                         m_accessTrackIsolation30;
    std::unique_ptr<AccessFloat>                         m_accessTrackIsolation30TTVA;
    std::unique_ptr<AccessFloat>                         m_accessMuonCalE;
    std::unique_ptr<AccessFloat>                         m_accessMuonParamEnergyLoss;
    std::unique_ptr<AccessVertex>                        m_accessDeepSecondaryVertex;

    floatAccessorMap                                     m_accessRNNMap;

    std::unique_ptr<TH1D>                                m_leptonPtBinHist;

    TStopwatch                                           m_timerAll;
    TStopwatch                                           m_timerExec;
    TStopwatch                                           m_timerMuon;
    TStopwatch                                           m_timerElec;
  };
}

#endif // PROMPT_DECORATEPROMPTLEPTON_H
