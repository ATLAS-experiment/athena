/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_OVERLAPREMOVALGENUSEALG_H
#define ASSOCIATIONUTILS_OVERLAPREMOVALGENUSEALG_H

// Framework includes
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKeyArray.h"
#include "AsgDataHandles/ReadDecorHandleKeyArray.h"

#include "AnaAlgorithm/AnaAlgorithm.h"

// EDM includes
#include "xAODBase/IParticleContainer.h"

// Local includes
#include "AssociationUtils/IOverlapRemovalTool.h"

/// Algorithm to implement basic OR procedure as a prerequisite for event cleaning
///
/// @author Julia Gonski <j.gonski@cern.ch>
///
class OverlapRemovalGenUseAlg : public EL::AnaAlgorithm
{

  public:

    /// Standard algorithm constructor
    OverlapRemovalGenUseAlg(const std::string& name, ISvcLocator* svcLoc);

    /// Initialize the algorithm
    virtual StatusCode initialize();

    /// Execute the algorithm
    virtual StatusCode execute();

  private:

    /// Simple object selection
    template<class ContainerType>
    void applySelection(const ContainerType& container);

    // Reset decorations to failing
    template<class ContainerType>
    void setDefaultDecorations(const ContainerType& container);

    /// Simple object selection
    template<class ObjType>
    bool selectObject(const ObjType& obj);

    /// Print object info
    void printObjects(const xAOD::IParticleContainer& container,
                      const std::string& type);

    /// Handle to the tool
    ToolHandle<ORUtils::IOverlapRemovalTool> m_orTool{this, "OverlapRemovalTool", ""};
   
    template <class ContainerType> class DataKeyHandler {
      public:
       DataKeyHandler(EL::AnaAlgorithm* owner,
                      const std::string& propName,
                      const std::string& defaultValue,
                      const std::string& descrip):
                        m_readKey{owner,propName, defaultValue, descrip},
                        m_readDecorKeys{owner, propName + "ReadDecors", {}, "External input data dependencies for "+propName},
                        m_writeDecorKeys{owner,propName + "WriteDecors", {}, "Dependencies written for "+propName}{}
       StatusCode initialize(bool allow_empty = false){
         if (!m_readKey.initialize(!allow_empty).isSuccess()) return StatusCode::FAILURE;
         if (!m_readDecorKeys.initialize().isSuccess()) return StatusCode::FAILURE;
         if (!m_writeDecorKeys.initialize().isSuccess()) return StatusCode::FAILURE;
         return StatusCode::SUCCESS;
       }
       operator const SG::ReadHandleKey<ContainerType>& () const  {return m_readKey;}
       std::string key() const { return m_readKey.key();}
       bool empty() const { return m_readKey.empty(); }
       void declareDependency(const std::string& variable) { if (!empty() && !variable.empty()) m_readDecorKeys.emplace_back(m_readKey.key() + "." + variable );}
       void declareOutput(const std::string& variable) { if (!empty() && !variable.empty()) m_writeDecorKeys.emplace_back(m_readKey.key() + "." + variable); }
     private:
       SG::ReadHandleKey<ContainerType> m_readKey{};
       SG::ReadDecorHandleKeyArray<ContainerType> m_readDecorKeys{};
       SG::WriteDecorHandleKeyArray<ContainerType> m_writeDecorKeys{};
    };
    
    DataKeyHandler<xAOD::JetContainer> m_jetKey{this, "JetKey", "AntiKt4EMPFlowJets",
                                                    "StoreGate/TEvent key for jets"};

    DataKeyHandler<xAOD::ElectronContainer> m_electronKey{this, "ElectronKey", "Electrons",
                                                    "StoreGate/TEvent key for electrons"};
    
    DataKeyHandler<xAOD::PhotonContainer> m_photonKey{this, "PhotonKey", "Photons",
                                                    "StoreGate/TEvent key for photons"};
    
    DataKeyHandler<xAOD::MuonContainer> m_muonKey{this, "MuonKey", "Muons",
                                            "StoreGate/TEvent key for muons"};

    DataKeyHandler<xAOD::TauJetContainer> m_tauKey{this, "TauKey", "TauJets",
                                            "StoreGate/TEvent key for taus"};
    
    SG::ReadHandleKey<xAOD::VertexContainer> m_vtxKey{this, "VertexKey", "PrimaryVertices",
                                             "StoreGate/TEvent key for the vertex container"};
    
    Gaudi::Property<std::string> m_bJetLabel{this, "BJetLabel", "", "Input label for b-tagged jets"};
    Gaudi::Property<std::string> m_electronLabel{this, "ElectronLabel", "DFCommonElectronsLHLoose",
                                                 "Input label for passing electrons"};
    Gaudi::Property<std::string> m_photonLabel{this, "PhotonLabel", "DFCommonPhotonsIsEMLoose",
                                                 "Input label for passing photons"};
    Gaudi::Property<std::string> m_muonLabel{this, "MuonLabel", "DFCommonMuonPassIDCuts",
                                             "Input label for passing muons"};
    Gaudi::Property<std::string> m_tauLabel{this, "TauLabel", "DFCommonTausLoose",
                                          "Input label for passing taus" };
    
    /// Label configuration --> Internal usage no data dependency needed
    Gaudi::Property<std::string> m_selectionLabel{this, "SelectionLabel", "selected", 
                                "Input label for the OverlapRemovalTool"};
                                
    Gaudi::Property<std::string> m_overlapLabel{this, "OverlapLabel", "overlaps",
                                                "Output label for the OverlapRemovalTool"};
    
    Gaudi::Property<bool> m_defaultValue{this, "DefaultValue", true,  "Default value for objects failing OR"};
    Gaudi::Property<float> m_ptCut{this, "PtCut", 20., "Minimum pt for consideration"};
    Gaudi::Property<float> m_etaCut{this, "EtaCut", 4.5, "Maximum eta for consideration"};

};

#endif
