/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

 #include "egammaAmbiguityRelinker.h"

 #include "StoreGate/ReadHandle.h"
 #include "StoreGate/WriteHandle.h"

 #include "xAODEgamma/EgammaContainer.h"
 #include "xAODEgamma/Electron.h"
 #include "xAODEgamma/ElectronAuxContainer.h"
 #include "xAODEgamma/ElectronContainer.h" 
 #include "xAODEgamma/Photon.h"
 #include "xAODEgamma/PhotonAuxContainer.h"
 #include "xAODEgamma/PhotonContainer.h"

  #include "egammaUtils/egAmbLinkHelper.h"


 egammaAmbiguityRelinker::egammaAmbiguityRelinker(const std::string& name,
                                      ISvcLocator* pSvcLocator)
   : AthReentrantAlgorithm(name, pSvcLocator)
 {}
 
 StatusCode
 egammaAmbiguityRelinker::initialize()
 {
   // the data handle keys
   ATH_CHECK(m_electronOutputKey.initialize());
   ATH_CHECK(m_electronInputKey.initialize());
   ATH_CHECK(m_photonOutputKey.initialize());
   ATH_CHECK(m_photonInputKey.initialize());
   return StatusCode::SUCCESS;
 }
 
 StatusCode
 egammaAmbiguityRelinker::execute(const EventContext& ctx) const {
   SG::ReadHandle<xAOD::ElectronContainer> el_inputContainer(m_electronInputKey,ctx);
   SG::WriteHandle<xAOD::ElectronContainer> el_outputContainer(m_electronOutputKey,ctx);
   SG::ReadHandle<xAOD::PhotonContainer> ph_inputContainer(m_photonInputKey,ctx);
   SG::WriteHandle<xAOD::PhotonContainer> ph_outputContainer(m_photonOutputKey,ctx);

   ATH_CHECK(el_outputContainer.record(std::make_unique<xAOD::ElectronContainer>(),
                     std::make_unique<xAOD::ElectronAuxContainer>()));

   ATH_CHECK(ph_outputContainer.record(std::make_unique<xAOD::PhotonContainer>(),
                     std::make_unique<xAOD::PhotonAuxContainer>()));

 
   xAOD::ElectronContainer* electrons = el_outputContainer.ptr();
   xAOD::PhotonContainer* photons = ph_outputContainer.ptr();
   electrons->reserve(el_inputContainer->size());
   photons->reserve(ph_inputContainer->size());

   for (const xAOD::Electron* old_el : *el_inputContainer) {
     xAOD::Electron*  electron = electrons->push_back(std::make_unique<xAOD::Electron>());
     *electron=*old_el;
   }

   for (const xAOD::Photon* old_ph : *ph_inputContainer) {
    xAOD::Photon*  photon = photons->push_back(std::make_unique<xAOD::Photon>());
    *photon=*old_ph;
  }
   
   // Recompute ambiguity links 
   egAmbLinkHelper::doAmbiguityLinks(ctx, electrons, photons);
   egAmbLinkHelper::doAmbiguityLinks(ctx, photons, electrons);
   
   return StatusCode::SUCCESS;
 }
 