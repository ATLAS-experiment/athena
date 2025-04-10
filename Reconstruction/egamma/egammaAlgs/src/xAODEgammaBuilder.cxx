/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "xAODEgammaBuilder.h"

#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "CaloDetDescr/CaloDetDescrManager.h"

#include "egammaUtils/EMFourMomBuilder.h"
#include "egammaUtils/egAmbLinkHelper.h"

#include "xAODEgamma/EgammaContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/PhotonAuxContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

xAODEgammaBuilder::xAODEgammaBuilder(const std::string& name,
                                     ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode
xAODEgammaBuilder::initialize()
{
  m_deltaEta1Pear = std::make_unique<electronPearShapeAlignmentCorrection>();
  // the data handle keys
  ATH_CHECK(m_electronClusterRecContainerKey.initialize(m_doElectrons));
  ATH_CHECK(m_photonClusterRecContainerKey.initialize(m_doPhotons));
  ATH_CHECK(m_electronOutputKey.initialize(m_doElectrons));
  ATH_CHECK(m_photonOutputKey.initialize(m_doPhotons));
  ATH_CHECK(m_caloDetDescrMgrKey.initialize());

  // retrieve tools
  ATH_CHECK(m_clusterTool.retrieve());
  ATH_CHECK(m_ShowerTool.retrieve());

  if (m_doElectrons) {
    ATH_CHECK(m_electronTools.retrieve());
  }
  if (m_doPhotons) {
    ATH_CHECK(m_photonTools.retrieve());
  }
  m_doOQ = !m_egammaOQTool.empty();
  if (m_doOQ) {
    ATH_CHECK(m_egammaOQTool.retrieve());
  } else {
    m_egammaOQTool.disable();
  }

  // do we actually do ambiguity
  m_doAmbiguity = !m_ambiguityTool.empty();
  if (m_doElectrons && m_doPhotons && m_doAmbiguity) {
    ATH_CHECK(m_ambiguityTool.retrieve());
  } else {
    m_ambiguityTool.disable();
  }
  m_doDummyElectrons = !(m_dummyElectronOutputKey.empty());
  ATH_CHECK(m_dummyElectronOutputKey.initialize(m_doDummyElectrons));
  return StatusCode::SUCCESS;
}

StatusCode
xAODEgammaBuilder::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode
xAODEgammaBuilder::execute(const EventContext& ctx) const
{

  const EgammaRecContainer* inputElRecs = nullptr;
  const EgammaRecContainer* inputPhRecs = nullptr;
  xAOD::ElectronContainer* electrons = nullptr;
  xAOD::PhotonContainer* photons = nullptr;
  /*
   * From here on if a Read/Write handle
   * is retrieved the above will be !=
   * nullptr for electron or photons or both
   */
  std::optional<SG::WriteHandle<xAOD::ElectronContainer>> electronContainer;
  std::optional<SG::WriteHandle<xAOD::PhotonContainer>> photonContainer;

  if (m_doElectrons) {
    SG::ReadHandle<EgammaRecContainer> electronSuperRecs(
      m_electronClusterRecContainerKey, ctx);
    inputElRecs = electronSuperRecs.ptr();

    electronContainer.emplace(m_electronOutputKey, ctx);
    ATH_CHECK(electronContainer->record(
      std::make_unique<xAOD::ElectronContainer>(),
      std::make_unique<xAOD::ElectronAuxContainer>()));

    electrons = electronContainer->ptr();
    electrons->reserve(inputElRecs->size());
  }
  if (m_doPhotons) {
    SG::ReadHandle<EgammaRecContainer> photonSuperRecs(
      m_photonClusterRecContainerKey, ctx);
    inputPhRecs = photonSuperRecs.ptr();

    photonContainer.emplace(m_photonOutputKey, ctx);
    ATH_CHECK(
      photonContainer->record(std::make_unique<xAOD::PhotonContainer>(),
                              std::make_unique<xAOD::PhotonAuxContainer>()));

    photons = photonContainer->ptr();
    photons->reserve(inputPhRecs->size());
  }

  /*
   * Now fill the electrons and photons
   */
  if (m_doElectrons) {
    for (const egammaRec* electronRec : *inputElRecs) {
      // in case for some reasons we reach here with no trackparticles
      if (electronRec->getNumberOfTrackParticles() == 0) {
        continue;
      }
      unsigned int author = xAOD::EgammaParameters::AuthorElectron;
      xAOD::AmbiguityTool::AmbiguityType type = xAOD::AmbiguityTool::electron;
      if (m_doPhotons) {
        // get the hottest cell
        const xAOD::CaloCluster* const elClus = electronRec->caloCluster();
        const double elEta0 = elClus->eta0();
        const double elPhi0 = elClus->phi0();
        for (const egammaRec* photonRec : *inputPhRecs) {
          const xAOD::CaloCluster* const phClus = photonRec->caloCluster();
          // See if they have the same hottest cell
          if (elEta0 == phClus->eta0() && elPhi0 == phClus->phi0()) {
            if (m_doAmbiguity) { // should be the default
              author =
                m_ambiguityTool->ambiguityResolve(elClus,
                                                  photonRec->vertex(),
                                                  electronRec->trackParticle(),
                                                  type);
            } else { // in case the ambiguity tool is not set ambiguity is not
                     // resolved
              author = xAOD::EgammaParameters::AuthorAmbiguous;
            }
            break;
          }
        }
      }
      // Create Electron xAOD objects
      if (author == xAOD::EgammaParameters::AuthorElectron ||
          author == xAOD::EgammaParameters::AuthorAmbiguous) {
        if (!getElectron(electronRec, electrons, author, type)) {
          return StatusCode::FAILURE;
        }
      }
    }
  }

  if (m_doPhotons) {
    for (const egammaRec* photonRec : *inputPhRecs) {
      unsigned int author = xAOD::EgammaParameters::AuthorPhoton;
      xAOD::AmbiguityTool::AmbiguityType type = xAOD::AmbiguityTool::photon;
      if (m_doElectrons) {
        // get the hottest cell
        const xAOD::CaloCluster* const phClus = photonRec->caloCluster();
        const double phEta0 = phClus->eta0();
        const double phPhi0 = phClus->phi0();
        for (const egammaRec* electronRec : *inputElRecs) {
          const xAOD::CaloCluster* const elClus = electronRec->caloCluster();
          // See if they have the same hottest cell
          if (phEta0 == elClus->eta0() && phPhi0 == elClus->phi0()) {
            if (m_doAmbiguity) { // should be the default
              author =
                m_ambiguityTool->ambiguityResolve(elClus,
                                                  photonRec->vertex(),
                                                  electronRec->trackParticle(),
                                                  type);
            } else { // in case the ambiguity tool is not set ambiguity is not
                     // resolved
              author = xAOD::EgammaParameters::AuthorAmbiguous;
            }
            break;
          }
        }
      }
      // Create Photon xAOD objects
      if (author == xAOD::EgammaParameters::AuthorPhoton ||
          author == xAOD::EgammaParameters::AuthorAmbiguous) {
        if (!getPhoton(photonRec, photons, author, type)) {
          return StatusCode::FAILURE;
        }
      }
    }
  }

  SG::ReadCondHandle<CaloDetDescrManager> caloDetDescrMgrHandle{
    m_caloDetDescrMgrKey, ctx
  };
  ATH_CHECK(caloDetDescrMgrHandle.isValid());
  const CaloDetDescrManager* calodetdescrmgr = *caloDetDescrMgrHandle;

  // Shower Shapes
  if (electrons) {
    for (xAOD::Electron* electron : *electrons) {
      ATH_CHECK(m_ShowerTool->execute(ctx, *calodetdescrmgr, electron));
    }
  }
  if (photons) {
    for (xAOD::Photon* photon : *photons) {
      ATH_CHECK(m_ShowerTool->execute(ctx, *calodetdescrmgr, photon));
    }
  }

  // Object Quality
  if (m_doOQ) {
    if (electrons) {
      for (xAOD::Electron* electron : *electrons) {
        ATH_CHECK(m_egammaOQTool->execute(ctx, *electron));
      }
    }
    if (photons) {
      for (xAOD::Photon* photon : *photons) {
        ATH_CHECK(m_egammaOQTool->execute(ctx, *photon));
      }
    }
  }

  // Energy calibration
  ATH_CHECK(m_clusterTool->contExecute(ctx, electrons, photons));

  //Followed by 4-Mom Building
  EMFourMomBuilder::calculate(electrons);
  EMFourMomBuilder::calculate(photons);
  //Additional tools fpr electrons/photons
  //e.g identification
  if (m_doElectrons) {
    for (const auto& tool : m_electronTools) {
      ATH_CHECK(CallTool(ctx, tool, electrons));
    }
  }
  if (m_doPhotons) {
    for (const auto& tool : m_photonTools) {
      ATH_CHECK(CallTool(ctx, tool, photons));
    }
  }
  // Do the ambiguity Links
  if (m_doElectrons && m_doPhotons) {
    egAmbLinkHelper::doAmbiguityLinks(ctx, electrons, photons);
    egAmbLinkHelper::doAmbiguityLinks(ctx, photons, electrons);
  }

  if (m_doDummyElectrons) {
    SG::WriteHandle<xAOD::ElectronContainer> dummyElectronContainer(
      m_dummyElectronOutputKey, ctx);
    ATH_CHECK(dummyElectronContainer.record(
      std::make_unique<xAOD::ElectronContainer>(),
      std::make_unique<xAOD::ElectronAuxContainer>()));

    dummyElectronContainer->push_back(std::unique_ptr<xAOD::Electron>());
  }

  return StatusCode::SUCCESS;
}

template <typename T>
StatusCode
xAODEgammaBuilder::CallTool(
  const EventContext& ctx,
  const ToolHandle<IegammaBaseTool>& tool,
  DataVector<T> *container) const
{
  if (container) {
    for (T *egamma : *container) {
      ATH_CHECK(tool->execute(ctx, egamma));
    }
  }

  return StatusCode::SUCCESS;
}

bool
xAODEgammaBuilder::getElectron(const egammaRec* egRec,
                               xAOD::ElectronContainer* electronContainer,
                               const unsigned int author,
                               const uint8_t type) const
{
  if (!egRec || !electronContainer) {
    return false;
  }

  xAOD::Electron* electron = electronContainer->push_back(std::make_unique<xAOD::Electron>());
  electron->setAuthor(author);

  static const SG::AuxElement::Accessor<uint8_t> acc("ambiguityType");
  acc(*electron) = type;

  electron->setCaloClusterLinks(egRec->caloClusterElementLinks());
  electron->setTrackParticleLinks(egRec->trackParticleElementLinks());

  const xAOD::TrackParticle* trackParticle = electron->trackParticle();
  if (trackParticle) {
    electron->setCharge(trackParticle->charge());
  }
  // Set DeltaEta, DeltaPhi , DeltaPhiRescaled
  electron->setTrackCaloMatchValues(
    egRec->deltaEta(),
    egRec->deltaPhi(),
    egRec->deltaPhiRescaled(),
    egRec->deltaPhiLast()
  );

  static const SG::AuxElement::Accessor<float> pear("deltaEta1PearDistortion");
  pear(*electron) = m_isTruth ?  0.0 : m_deltaEta1Pear->getDeltaEtaDistortion(
    electron->caloCluster()->etaBE(2),
    electron->caloCluster()->phiBE(2)
  );

  return true;
}

bool
xAODEgammaBuilder::getPhoton(const egammaRec* egRec,
                             xAOD::PhotonContainer* photonContainer,
                             const unsigned int author,
                             const uint8_t type) const
{
  if (!egRec || !photonContainer) {
    return false;
  }

  xAOD::Photon* photon = photonContainer->push_back(std::make_unique<xAOD::Photon>());
  photon->setAuthor(author);
  static const SG::AuxElement::Accessor<uint8_t> acc("ambiguityType");
  acc(*photon) = type;

  photon->setCaloClusterLinks(egRec->caloClusterElementLinks());
  photon->setVertexLinks(egRec->vertexElementLinks());

  // Transfer deltaEta/Phi info
  float deltaEta = egRec->deltaEtaVtx();
  float deltaPhi = egRec->deltaPhiVtx();
  if (!photon->setVertexCaloMatchValue(
        deltaEta, xAOD::EgammaParameters::convMatchDeltaEta1)) {
    return false;
  }

  if (!photon->setVertexCaloMatchValue(
        deltaPhi, xAOD::EgammaParameters::convMatchDeltaPhi1)) {
    return false;
  }
  return true;
}
