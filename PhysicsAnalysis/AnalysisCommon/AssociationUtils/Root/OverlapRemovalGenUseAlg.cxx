/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM includes

// Local includes
#include "AssociationUtils/OverlapRemovalDefs.h"
#include "AssociationUtils/OverlapRemovalGenUseAlg.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgTools/CurrentContext.h"

namespace
{
  /// Unit conversion constants
  const float GeV = 1000.;
  const float invGeV = 1./GeV;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
OverlapRemovalGenUseAlg::OverlapRemovalGenUseAlg(const std::string& name,
    ISvcLocator* svcLoc)
  : EL::AnaAlgorithm(name, svcLoc) { }


//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalGenUseAlg::initialize()
{
  ATH_MSG_INFO("Initialize");

  // Try to retrieve the tool
  ATH_CHECK( m_orTool.retrieve() );

  if (!m_bJetLabel.empty()){
    m_jetKey.declareDependency(m_bJetLabel);
  }
  if (!m_electronLabel.empty()){
    m_electronKey.declareDependency(m_electronLabel);
  }
  if (!m_photonLabel.empty()){
    m_photonKey.declareDependency(m_photonLabel);
  }
  if (!m_muonLabel.empty()){
    m_muonKey.declareDependency(m_muonLabel);
  }
  if (!m_tauLabel.empty()){
    m_tauKey.declareDependency(m_tauLabel);
  }
  
  m_jetKey.declareOutput(m_overlapLabel);
  m_electronKey.declareOutput(m_overlapLabel);
  m_photonKey.declareOutput(m_overlapLabel);
  m_muonKey.declareOutput(m_overlapLabel);
  m_tauKey.declareOutput(m_overlapLabel);
  ATH_CHECK(m_jetKey.initialize());
  ATH_CHECK(m_electronKey.initialize());
  ATH_CHECK(m_muonKey.initialize());
  ATH_CHECK(m_vtxKey.initialize());
  ATH_CHECK(m_photonKey.initialize(m_photonKey.empty()));
  ATH_CHECK(m_tauKey.initialize(m_tauKey.empty()));
  
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Execute
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalGenUseAlg::execute()
{
    const EventContext& ctx = Gaudi::Hive::currentContext();
    // Electrons
    SG::ReadHandle<xAOD::ElectronContainer> electrons{m_electronKey, ctx};
    if (!electrons.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve electron container "<<m_electronKey.key());
      return StatusCode::FAILURE;
    }
    applySelection(*electrons);
    // Muons
    SG::ReadHandle<xAOD::MuonContainer> muons{m_muonKey, ctx};
    if (!muons.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve muon container "<<m_muonKey.key());
      return StatusCode::FAILURE;
    }
    applySelection(*muons);
    // Jets
    SG::ReadHandle<xAOD::JetContainer> jets{m_jetKey, ctx};
    if (!jets.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve jet container "<<m_jetKey.key());
      return StatusCode::FAILURE;
    }
    applySelection(*jets);
    // Taus
    const xAOD::TauJetContainer* taus{nullptr};
    if(!m_tauKey.empty()) {
        SG::ReadHandle<xAOD::TauJetContainer> readHandle{m_tauKey, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve TauJetContainer "<<m_tauKey.key());
            return StatusCode::FAILURE;
        }
        taus = readHandle.cptr();
        applySelection(*taus);
    }
    const xAOD::PhotonContainer* photons{nullptr};
    if(!m_photonKey.empty()) {
        SG::ReadHandle<xAOD::PhotonContainer> readHandle{m_photonKey, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve PhotonContainer "<<m_photonKey.key());
            return StatusCode::FAILURE;
        }
        photons = readHandle.cptr();
        applySelection(*photons);
    }

    // Primary Vertices
    SG::ReadHandle<xAOD::VertexContainer> vertices{m_vtxKey, ctx};
    if (!vertices.isValid()) {
       ATH_MSG_FATAL("Failed to retrieve primary vertex container "<<m_vtxKey.key());
       return StatusCode::FAILURE;
    }
    bool checkVtx = false;
    for(const xAOD::Vertex* vtx : *vertices) {
        if(vtx->vertexType() == xAOD::VxType::PriVtx)
            checkVtx = true;
    }
    

    if(checkVtx) {
        // Apply the overlap removal
        ATH_CHECK( m_orTool->removeOverlaps(electrons.cptr(), muons.cptr(), jets.cptr(), taus, photons) );}
    else{
        // Reset all decorations to failing
        ATH_MSG_DEBUG("No primary vertices found, cannot do overlap removal! Will return all fails.");
        setDefaultDecorations(*jets);
        setDefaultDecorations(*electrons);
        setDefaultDecorations(*muons);
        if(taus)      setDefaultDecorations(*taus);
        if(photons)   setDefaultDecorations(*photons);
    }

    // Dump the objects
    ATH_MSG_VERBOSE("Dumping results");
#ifdef XAOD_STANDALONE
    auto msglvl = msg().level();
#else
    auto msglvl = msgLevel();
#endif
    if(msglvl >= MSG::VERBOSE){
        printObjects(*electrons, "ele");
        printObjects(*muons, "muo");
        printObjects(*jets, "jet");
        if(taus) printObjects(*taus, "tau");
        if(photons) printObjects(*photons, "pho");
    }

    return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
// Reset output decoration
//---------------------------------------------------------------------------
  template<class ContainerType>
void OverlapRemovalGenUseAlg::setDefaultDecorations(const ContainerType& container) {
  const ort::inputDecorator_t defaultDec(m_overlapLabel);
  for(auto obj : container){
    defaultDec(*obj) = m_defaultValue; //default to all objects being overlaps if we can't get primary vertices. Ensures the event cleaning decision fails.
  }
}


//-----------------------------------------------------------------------------
// Apply selection to a container
//-----------------------------------------------------------------------------
  template<class ContainerType>
void OverlapRemovalGenUseAlg::applySelection(const ContainerType& container)
{
  const ort::inputDecorator_t selDec(m_selectionLabel);
  for(auto obj : container){
    selDec(*obj) = selectObject(*obj);
        ATH_MSG_VERBOSE("  Obj " << obj->index() << " of type " << obj->type()
                        << " selected? " << int(selDec(*obj)));
  }
}
//-----------------------------------------------------------------------------
template<class ObjType>
bool OverlapRemovalGenUseAlg::selectObject(const ObjType& obj)
{
  if(obj.pt() < m_ptCut*GeV || std::abs(obj.eta()) > m_etaCut) return false;
  return true;
}
//-----------------------------------------------------------------------------
template<>
bool OverlapRemovalGenUseAlg::selectObject<xAOD::Jet>(const xAOD::Jet& obj)
{
  // Selection
  if(obj.pt() < m_ptCut*GeV || std::abs(obj.eta()) > m_etaCut) return false;
  return true;
}

//-----------------------------------------------------------------------------
template<>
bool OverlapRemovalGenUseAlg::selectObject<xAOD::Electron>(const xAOD::Electron& obj)
{
  if(m_electronLabel.empty()) return true;    //disable selection for objects with empty labels
  const SG::AuxElement::ConstAccessor<char> acc_ElectronPass(m_electronLabel);
  if(obj.pt() < m_ptCut*GeV || std::abs(obj.eta()) > m_etaCut) return false;
  if(!acc_ElectronPass(obj)) return false;
  return true;
}

//-----------------------------------------------------------------------------
template<>
bool OverlapRemovalGenUseAlg::selectObject<xAOD::Photon>(const xAOD::Photon& obj)
{
  if(m_photonLabel.empty()) return true;    //disable selection for objects with empty labels
  const SG::AuxElement::ConstAccessor<char> acc_PhotonPass(m_photonLabel);
  if(obj.pt() < m_ptCut*GeV || std::abs(obj.eta()) > m_etaCut) return false;
  if(!acc_PhotonPass(obj)) return false;
  return true;
}

//-----------------------------------------------------------------------------
template<>
bool OverlapRemovalGenUseAlg::selectObject<xAOD::Muon>(const xAOD::Muon& obj)
{
  if(m_muonLabel.empty()) return true;    //disable selection for objects with empty labels
  const SG::AuxElement::ConstAccessor<char> acc_MuonPass(m_muonLabel);
  if(obj.pt() < m_ptCut*GeV || std::abs(obj.eta()) > m_etaCut) return false;
  if(!acc_MuonPass(obj)) return false;
  return true;
}

//-----------------------------------------------------------------------------
template<>
bool OverlapRemovalGenUseAlg::selectObject<xAOD::TauJet>(const xAOD::TauJet& obj)
{
  if(m_tauLabel.empty()) return true;    //disable selection for objects with empty labels
  const SG::AuxElement::ConstAccessor<char> acc_TauPass(m_tauLabel);
  if(obj.pt() < m_ptCut*GeV || std::abs(obj.eta()) > m_etaCut) return false;
  if(!acc_TauPass(obj)) return false;
  return true;
}


//-----------------------------------------------------------------------------
// Print object
//-----------------------------------------------------------------------------
void OverlapRemovalGenUseAlg::printObjects(const xAOD::IParticleContainer& container,
                                         const std::string& type)
{
  const ort::inputAccessor_t selectAcc(m_selectionLabel);
  const ort::outputAccessor_t overlapAcc(m_overlapLabel);
  for(auto obj : container){
    if(selectAcc(*obj)){
      bool overlaps = overlapAcc(*obj);
      ATH_MSG_VERBOSE("  " << type << " pt " << obj->pt()*invGeV
                    << " eta " << obj->eta() << " phi " << obj->phi()
                    << " overlaps " << overlaps);
    }
  }
}
