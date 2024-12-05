// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimTrackFitterTool.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"

#include "GaudiKernel/MsgStream.h"

#include <sstream>
#include <iostream>
#include <fstream>

/////////////////////////////////////////////////////////////////////////////
FPGATrackSimTrackFitterTool::FPGATrackSimTrackFitterTool(const std::string& type,
							const std::string& name,
							const IInterface* parent)
  : FPGATrackSimTrackingToolBase(type, name, parent)
{
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimTrackFitterTool::initialize()
{
    ATH_MSG_DEBUG("FPGATrackSimTrackFitterTool::initialize()");

    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    ATH_CHECK(m_FPGATrackSimBank.retrieve());
    if (m_useSpacePoints) ATH_CHECK(m_spRoadFilterTool.retrieve(EnableTool{m_spRoadFilterTool}));

    const FPGATrackSimFitConstantBank* nominalbank;
    std::vector<const FPGATrackSimFitConstantBank*> bankvec;
    if(!m_do2ndStage){
      nominalbank = m_FPGATrackSimBank->FitConstantBank_1st();
      if (!m_guessHits) {
        for (unsigned int iplane = 0; iplane < m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers(); iplane++) {
	  const FPGATrackSimFitConstantBank* bank = m_FPGATrackSimBank->FitConstantBank_1st(iplane);
	  bankvec.push_back(bank);
        }
      }
    }
    else {
      nominalbank = m_FPGATrackSimBank->FitConstantBank_2nd();
      if (!m_guessHits) {
        for (unsigned int iplane = 0; iplane < m_FPGATrackSimMapping->PlaneMap_2nd()->getNLogiLayers(); iplane++) {
          const FPGATrackSimFitConstantBank* bank = m_FPGATrackSimBank->FitConstantBank_2nd(iplane);
          bankvec.push_back(bank);
        }
      }
    }

    MsgStream m(msgSvc(), "TrackFitter");
    m.setLevel(msg().level());
    m_tfpobj = std::unique_ptr<TrackFitter>(new TrackFitter(nominalbank, bankvec, m_guessHits));

    if(!m_do2ndStage) {
      m_tfpobj->setPlaneMap(m_FPGATrackSimMapping->PlaneMap_1st(0));
      m_tfpobj->setRegionMap(m_FPGATrackSimMapping->RegionMap_1st());
    } else {
      m_tfpobj->setPlaneMap(m_FPGATrackSimMapping->PlaneMap_2nd());
      m_tfpobj->setRegionMap(m_FPGATrackSimMapping->RegionMap_2nd());
    }

    // set parameter object to TrackFitter
    m_tfpobj->setChi2DofRecoveryMin(m_chi2dof_recovery_min);
    m_tfpobj->setChi2DofRecoveryMax(m_chi2dof_recovery_max);

    m_tfpobj->setDoMajority(m_doMajority);

    m_tfpobj->setRequireFirst(0);
    m_tfpobj->setDoSecondStage(m_do2ndStage);

    m_tfpobj->setNorecoveryNhits(m_noRecoveryNHits);

    if (!m_guessHits && m_doMissingHitsChecks) ATH_MSG_WARNING("We can't do missing hits check if we don't guess hits");
    m_tfpobj->setDoMissingHitsCheck(m_doMissingHitsChecks);
    m_tfpobj->setIdealCoordFitType(static_cast<TrackCorrType>(m_idealCoordFitType.value()));
    m_tfpobj->setDoDeltaGPhis(m_doDeltaGPhis);

    return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimTrackFitterTool::getTracks(std::vector<std::shared_ptr<const FPGATrackSimRoad>>& roads, std::vector<FPGATrackSimTrack>& tracks) {
    // elaborate the next event
    ATH_CHECK(setRoadSectors(roads));
    int status = m_tfpobj->fitTracks(roads,tracks);
    if (status != FITTRACKS_OK) return StatusCode::FAILURE;

    if (msgLvl(MSG::DEBUG))
    {
        ATH_MSG_DEBUG("getTracks() returning " << tracks.size() << " tracks:");
        for (auto & t : tracks)
            ATH_MSG_DEBUG("\tchi2 = " << t.getChi2() << " and phi = " << t.getPhi()
                    << " and eta = " << t.getEta() << " and d0 = " << t.getD0()
                    << " and z0 = " << t.getZ0() << "and pt = " << t.getPt());
    }

    return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimTrackFitterTool::getMissingHitsCheckTracks(std::vector<FPGATrackSimTrack>& tracks_guessed)
{
    // only call this after we first fit the tracks!
    tracks_guessed = m_tfpobj->getMissingHitsCheckTracks();
    return StatusCode::SUCCESS;
}

