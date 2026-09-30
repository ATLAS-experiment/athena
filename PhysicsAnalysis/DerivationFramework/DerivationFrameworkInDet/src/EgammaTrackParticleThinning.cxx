/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// EgammaTrackParticleThinning.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: James Catmore (James.Catmore@cern.ch)

#include "DerivationFrameworkInDet/EgammaTrackParticleThinning.h"
#include "StoreGate/ThinningHandle.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "AthContainers/ConstDataVector.h"

namespace {
const SG::AuxElement::Accessor<ElementLink<xAOD::TrackParticleContainer>> orig(
  "originalTrackParticle");
}

// Constructor
DerivationFramework::EgammaTrackParticleThinning::EgammaTrackParticleThinning(
  const std::string& t,
  const std::string& n,
  const IInterface* p)
  : base_class(t, n, p)
{}

// Destructor
DerivationFramework::EgammaTrackParticleThinning::~EgammaTrackParticleThinning()
= default;

// Athena initialize and finalize
StatusCode
DerivationFramework::EgammaTrackParticleThinning::initialize()
{
  // Decide which collections need to be checked for ID TrackParticles
  ATH_CHECK(m_egammaKey.initialize());

  ATH_CHECK(m_gsfSGKey.initialize(m_streamName));
  ATH_MSG_INFO("Using " << m_gsfSGKey.key()
                        << " as the source collection for GSF track particles");
  ATH_MSG_INFO((m_bestMatchOnly ? "Best match " : "ALL ")
               << "GSF track particles associated with objects in "
               << m_egammaKey.key() << '\n'
               << " will  be marked as kept true in the ThinningHandle "
               << "otherwise as kept false");

  ATH_CHECK(m_inDetSGKey.initialize(m_streamName, !m_inDetSGKey.empty()));
  if (!m_inDetSGKey.empty()) {
    ATH_MSG_INFO(
      "Using "
      << m_inDetSGKey.key()
      << " as the source collection for inner detector track particles");

    ATH_MSG_INFO("Inner detector track particles refitted to produce"
                 << m_gsfSGKey.key() << '\n'
                 << " will be retained when the corresponding "
                 << m_gsfSGKey.key() << " track particle will be retained");
    if (m_coneSize > 0) {
      ATH_MSG_INFO(
        "Inner detector track particles in a cone dr "
        << m_coneSize << " around the " << m_egammaKey.key() << '\n'
        << " obects  will  be marked as kept true in the ThinningHandle "
        << "otherwise as kept false");
    }
  }

  ATH_CHECK(m_gsfVtxSGKey.initialize(m_streamName, !m_gsfVtxSGKey.empty()));
  if (!m_gsfVtxSGKey.empty()) {
    ATH_MSG_INFO("Using " << m_gsfVtxSGKey.key()
		 << " as the source collection for GSF conversion vertices");
    ATH_MSG_INFO((m_bestVtxMatchOnly ? "Best match " : "ALL ")
		 << " GSF conversion vertices will be kept");
  }

  // Set up the text-parsing machinery for selectiong the objects directly
  // according to user cuts
  if (!m_selectionString.empty()) {
    ATH_CHECK(initializeParser(m_selectionString));
  }

  return StatusCode::SUCCESS;
}

StatusCode
DerivationFramework::EgammaTrackParticleThinning::finalize()
{
  ATH_MSG_INFO("Selected " << m_nSelEgammas <<" out of " << m_nEgammas
                           << " objects from " << m_egammaKey.key());
  ATH_MSG_INFO("Kept " << m_nGSFPass << " out of " << m_ntotGSF
                       << " objects from " << m_gsfSGKey.key());
  if (!m_gsfVtxSGKey.empty()) {
    ATH_MSG_INFO("Kept " << m_nGSFVtxPass << " out of " << m_ntotGSFVtx
		 << " vertices from " << m_gsfVtxSGKey.key());
  }
  if (!m_inDetSGKey.empty()) {
    ATH_MSG_INFO("Kept " << m_npass << "out of " << m_ntot << " objects from "
                         << m_inDetSGKey.key());
  }

  ATH_CHECK(finalizeParser());
  return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode
DerivationFramework::EgammaTrackParticleThinning::doThinning(const EventContext& ctx) const
{
  SG::ThinningHandle<xAOD::TrackParticleContainer> importedGSFTrackParticles(
    m_gsfSGKey, ctx);

  // Allow for not input Indet Track Particle collection
  std::unique_ptr<SG::ThinningHandle<xAOD::TrackParticleContainer>>
    importedTrackParticles = nullptr;
  if (!m_inDetSGKey.empty()) {
    importedTrackParticles =
      std::make_unique<SG::ThinningHandle<xAOD::TrackParticleContainer>>(
        m_inDetSGKey, ctx);
  }

  // Check the event contains tracks
  const xAOD::TrackParticleContainer* tps = (importedTrackParticles != nullptr)
                                              ? importedTrackParticles->cptr()
                                              : nullptr;
  const xAOD::TrackParticleContainer* gsfs = importedGSFTrackParticles.cptr();
  unsigned int nTracks = tps ? tps->size() : 0;
  unsigned int nGSF = gsfs->size();

  ATH_MSG_DEBUG("nTracks : " << nTracks << " , nGSF : " << nGSF);
  if (nTracks == 0 && nGSF == 0) {
    ATH_MSG_DEBUG("Nothing to thin");
    return StatusCode::SUCCESS;
  }

  // Set up a mask with the same entries as the full TrackParticle collection(s)
  std::vector<bool> mask, gsfMask;
  mask.assign(nTracks, false); // default: don't keep any tracks
  gsfMask.assign(nGSF, false);
  m_ntot += nTracks;
  m_ntotGSF += nGSF;

  // Retrieve e-gamma container
  SG::ReadHandle<xAOD::EgammaContainer> importedEgamma(m_egammaKey, ctx);
  if (!importedEgamma.isValid()) {
    ATH_MSG_ERROR("No e-gamma collection with name " << m_egammaKey.key()
                                                     << " found in StoreGate!");
    return StatusCode::FAILURE;
  }

  size_t nEgammas(importedEgamma->size());
  ATH_MSG_DEBUG("nEgammas : " << nEgammas);
  m_nEgammas += nEgammas;
  bool doSelect = !m_selectionString.empty();
  if (nEgammas != 0) {
    ConstDataVector<xAOD::EgammaContainer> tofill(SG::VIEW_ELEMENTS);
    // Execute the text parsers if requested
    if (doSelect) {
      std::vector<int> entries = m_parser->evaluateAsVector();
      unsigned int nEntries = entries.size();
      // check the sizes are compatible
      if (nEgammas != nEntries) {
        ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string "
                      "used e-gamma objects??");
        return StatusCode::FAILURE;
      } else {
        // identify which e-gammas to keep for the thinning check
        for (unsigned int i = 0; i < nEgammas; ++i)
          if (entries[i] == 1)
            tofill.push_back(importedEgamma->at(i));
      }
    } // end of selection
    const xAOD::EgammaContainer* egToCheck = doSelect
      ? tofill.asDataVector() : importedEgamma.cptr();
    ATH_MSG_DEBUG("Setting the masks");
    m_nSelEgammas += egToCheck->size();
    // Are we dealing with electrons or photons?
    if (dynamic_cast<const xAOD::ElectronContainer*>(importedEgamma.cptr()) != nullptr)
      setElectronMasks(mask, gsfMask, egToCheck, tps, gsfs);
    else if (dynamic_cast<const xAOD::PhotonContainer*>(importedEgamma.cptr()) != nullptr)
      setPhotonMasks(ctx, mask, gsfMask, egToCheck, tps, gsfs);
    else
      ATH_MSG_WARNING("Input container is neither for Electrons, "
		      "nor for Photons ??");
  }//end of if nEgammas != 0
  else if (!m_gsfVtxSGKey.empty()) {
    clearGSFVtx(ctx);
  }

  // Count up the mask contents
  unsigned int n_pass = 0;
  for (unsigned int i = 0; i < nTracks; ++i) {
    if (mask[i]) {
      ++n_pass;
    }
  }
  m_npass += n_pass;
  unsigned int n_gsf_pass = 0;
  for (unsigned int i = 0; i < nGSF; ++i) {
    if (gsfMask[i]) {
      ++n_gsf_pass;
    }
  }
  m_nGSFPass += n_gsf_pass;

  // Execute the thinning service based on the mask. Finish.
  importedGSFTrackParticles.keep(gsfMask);
  if (tps) {
    importedTrackParticles->keep(mask);
  }

  return StatusCode::SUCCESS;
}

void DerivationFramework::EgammaTrackParticleThinning::clearGSFVtx(
    const EventContext& ctx) const
{
  SG::ThinningHandle<xAOD::VertexContainer> importedGSFConversionVtx(
      m_gsfVtxSGKey, ctx);
  const xAOD::VertexContainer* gsfVtxs = importedGSFConversionVtx.cptr();
  unsigned int nGSFVtx = gsfVtxs->size();
  if (nGSFVtx == 0) {
    ATH_MSG_DEBUG("No conversion vertex to thin");
    return;
  }
  std::vector<bool> gsfVtxMask(nGSFVtx,false);
  m_ntotGSFVtx += nGSFVtx;
  ATH_MSG_DEBUG("nGSFVtx : " << nGSFVtx);
  importedGSFConversionVtx.keep(gsfVtxMask);
}

void
DerivationFramework::EgammaTrackParticleThinning::setPhotonMasks(
  const EventContext& ctx,
  std::vector<bool>& mask,
  std::vector<bool>& gsfMask,
  const xAOD::EgammaContainer* egammas,
  const xAOD::TrackParticleContainer* tps,
  const xAOD::TrackParticleContainer* gsfs) const
{
  if (m_gsfVtxSGKey.empty()) {
    ATH_MSG_ERROR("Thinning track particles/vertices associated to photons"
		  "but no conversion vertex key provided");
    return;
  }

  SG::ThinningHandle<xAOD::VertexContainer> importedGSFConversionVtx(
     m_gsfVtxSGKey, ctx);
  const xAOD::VertexContainer* gsfVtxs = importedGSFConversionVtx.cptr();
  unsigned int nGSFVtx = gsfVtxs->size(), n_gsfVtx_pass = 0;
  std::vector<bool> gsfVtxMask(nGSFVtx,false);
  m_ntotGSFVtx += nGSFVtx;
  ATH_MSG_DEBUG("nGSFVtx : " << nGSFVtx);

  DerivationFramework::TracksInCone trIC;
  for (const auto* egamma : *egammas) {
    const xAOD::Photon* photon = egamma->type() == xAOD::Type::Photon
                                   ? static_cast<const xAOD::Photon*>(egamma)
                                   : nullptr;
    if (!photon) {
      ATH_MSG_ERROR("Did not get a photon object in "
                    "EgammaTrackParticleThinning::setPhotonMasks");
      return;
    }
    if (tps && m_coneSize > 0.0) {
      trIC.select(photon, m_coneSize, tps, mask);
    } // check InDet tracks in a cone around the e-gammas

    std::vector<ElementLink<xAOD::VertexContainer>> vertexLinks =
      photon->vertexLinks();
    unsigned int nLinks = vertexLinks.size();
    if (nLinks == 0) {
      continue;
    }
    if (!m_bestVtxMatchOnly) {
      for (unsigned int i = 0; i < nLinks; ++i) {
	if (!(vertexLinks[i])) {
	  continue;
	}
	if (!(vertexLinks[i]).isValid()) {
	  continue;
	}
	gsfVtxMask[vertexLinks[i].index()] = true;
      }
    }
    if (m_bestMatchOnly) {
      nLinks = 1;
    }
    for (unsigned int i = 0; i < nLinks; ++i) {
      if (!(vertexLinks[i]).isValid()) {
        continue;
      }
      gsfVtxMask[vertexLinks[i].index()] = true;
      const xAOD::Vertex* vx = *(vertexLinks[i]);
      if (!vx) {
        continue;
      }
      auto trackParticleLinks = vx->trackParticleLinks();
      for (const auto& link : trackParticleLinks) {
        if (!link.isValid()) {
          continue;
        }
        gsfMask[link.index()] = true;
        if (tps) {
          const ElementLink<xAOD::TrackParticleContainer>& origTrackLink =
            orig(*((*gsfs)[link.index()]));
          if (origTrackLink.isValid()) {
            int inDetIndex = origTrackLink.index();
            if (inDetIndex>=0 ) [[likely]] mask[inDetIndex] = true;
          }
        }
      }
    }
  }
  importedGSFConversionVtx.keep(gsfVtxMask);
  for (bool b : gsfVtxMask) {
    if (b)
      ++n_gsfVtx_pass;
  }
  m_nGSFVtxPass += n_gsfVtx_pass;
}

void
DerivationFramework::EgammaTrackParticleThinning::setElectronMasks(
  std::vector<bool>& mask,
  std::vector<bool>& gsfMask,
  const xAOD::EgammaContainer* egammas,
  const xAOD::TrackParticleContainer* tps,
  const xAOD::TrackParticleContainer* gsfs) const
{
  DerivationFramework::TracksInCone trIC;
  for (const auto *egamma : *egammas) {
    const xAOD::Electron* electron =
      egamma->type() == xAOD::Type::Electron
        ? static_cast<const xAOD::Electron*>(egamma)
        : nullptr;

    if (!electron) {
      ATH_MSG_ERROR("Did not get an electron object in "
                    "EgammaTrackParticleThinning::setElectronMasks");
      return;
    }
    if (tps && m_coneSize > 0.0)
      trIC.select(electron,
                  m_coneSize,
                  tps,
                  mask); // check InDet tracks in a cone around the e-gammas

    unsigned int nGSFLinks = m_bestMatchOnly ? 1 : electron->nTrackParticles();
    for (unsigned int i = 0; i < nGSFLinks; ++i) {
      if (!(electron->trackParticleLink(i).isValid())) {
        continue;
      }
      int gsfIndex = electron->trackParticleLink(i).index();
      if (gsfIndex < 0)[[unlikely]] continue;
      gsfMask[gsfIndex] = true;
      if (tps) {
        const ElementLink<xAOD::TrackParticleContainer>& origTrackLink =
          orig(*((*gsfs)[gsfIndex]));
        if (origTrackLink.isValid()) {
          int inDetIndex = origTrackLink.index();
          if (inDetIndex>=0 ) [[likely]] mask[inDetIndex] = true;
        }
      }
    }
  }
}

