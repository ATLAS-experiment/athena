// this file is -*- C++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETREC_JETCLUSTERER_H
#define JETREC_JETCLUSTERER_H
///***********************************************
///
/// \class JetClusterer
///
/// Creates a new JetContainer by running fastjet on an input PseudoJetVector
///
/// This tool implements the IJetProvider interface. The JetContainer it returns is build by
/// running a fastjet::ClusterSequence according to the tool's Properties (alg type, radius, ...)
/// The ClusterSequence is run on an input PseudoJetVector which must be present in the event store (this PseudoJetVector key is also a Property of the tool).
///


#include "xAODEventInfo/EventInfo.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"
#include "AthContainers/AuxElement.h"

#include "JetInterface/IJetProvider.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

#include "JetRec/PseudoJetContainer.h"
#include "JetRec/JetFromPseudojet.h"
#include "JetRec/PseudoJetTranslator.h"
#include "JetEDM/PseudoJetVector.h"
#include "JetEDM/ClusterSequence.h"

#include "fastjet/PseudoJet.hh"
#include "fastjet/AreaDefinition.hh"
#include "fastjet/JetDefinition.hh"

#include "xAODJet/JetAuxContainer.h"
#include "xAODTracking/VertexContainer.h"


class JetClusterer
: public asg::AsgTool,
  virtual public JetProvider<xAOD::JetAuxContainer> {
  ASG_TOOL_CLASS(JetClusterer, IJetProvider)

public:

  // Can't use "using ctor" because of incompatiblity with pyroot in AnalysisBase
  JetClusterer(const std::string &name)
  : AsgTool(name),
    m_jetRankAccessor("jetRank")
  {}

  StatusCode initialize() override;

  /// Return the final jets with their aux store.
  /// Can return a pair of null pointers if an error occurs.
  std::pair<std::unique_ptr<xAOD::JetContainer>, std::unique_ptr<SG::IAuxStore> > getJets() const override;


protected:

  /// Build the area definition when running with area calculation. The seedsok flag is set to false when error occurs when retrieving event/run number to initiate the seeds.
  fastjet::AreaDefinition buildAreaDefinition(bool &seedsok) const ;

  /// Used to create the cluster sequence
  std::unique_ptr<fastjet::ClusterSequence> buildClusterSequence(const PseudoJetVector* pseudoJetvector) const ;
    
  /// translate to xAOD::Jet
  void processPseudoJet(const fastjet::PseudoJet& pj, const PseudoJetContainer& pjCont, xAOD::JetContainer* jets, const xAOD::Vertex* vertex) const;

  /// Handle to EventInfo. This is used to get the evt&run number to set the Ghost area random seeds.
  SG::ReadHandleKey<xAOD::EventInfo> m_eventinfokey{"EventInfo"};

  /// Handle Input PseudoJetContainer
  SG::ReadHandleKey<PseudoJetContainer> m_inputPseudoJets {this, "InputPseudoJets", "inputpseudojet", "input constituents"};

  /// used to build the key under which the final PJ will be stored in evtStore() 
  SG::WriteHandleKey<PseudoJetVector> m_finalPseudoJets {this, "FinalPseudoJets_DONOTSET", "", "output pseudojets -- autoconfigured name"};
  SG::WriteHandleKey<jet::ClusterSequence> m_clusterSequence {this, "ClusterSequence_DONOTSET", "", "output pseudojets -- autoconfigured name"};
  Gaudi::Property<std::string> m_jetRank {this, "jetRankName", "jetRank", "name for accessor for jet rank"};
  
  // Job options.
  Gaudi::Property<std::string>  m_jetalg {this, "JetAlgorithm", "AntiKt", "alg type : AntiKt, Kt, CA..."};
  Gaudi::Property<float> m_jetrad        {this, "JetRadius", 0.4 , "jet size parameter"}; 
  Gaudi::Property<float> m_ptmin         {this, "PtMin", 0.0, "pT min in MeV"};
  Gaudi::Property<float> m_ghostarea     {this, "GhostArea", 0.0, "Area for ghosts. 0==>no ghosts."};
  Gaudi::Property<int> m_ranopt          {this, "RandomOption", 1, "Rand option: 0=fj default, 1=run/event"};

  Gaudi::Property<int> m_inputType       {this, "JetInputType", 0, "input type as in xAOD::JetInput (see xAODJet/JetContainerInfo.h)"};

  
  // internal data set from properties during initialize()
  fastjet::JetAlgorithm m_fjalg;
  bool m_useArea{};
  
  Gaudi::Property<float> m_minrad        {this, "VariableRMinRadius", -1.0, "Variable-R min radius" };
  Gaudi::Property<float> m_massscale     {this, "VariableRMassScale", -1.0, "Variable-R mass scale" };
  bool m_isVariableR{};  
  bool isVariableR() const { return m_isVariableR;}

  SG::AuxElement::Accessor<int> m_jetRankAccessor;

};


namespace JetClustererHelper {

  /// Fill seeds vector from run & event number. This functio is separated from the class so it's easier to re-use and test
  void seedsFromEventInfo(const xAOD::EventInfo* ei, std::vector<int> &seeds);
  
}

#endif
