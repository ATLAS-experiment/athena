/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeantFollowerMSHelper.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

// StoreGate
#include "TrkG4UserActions/GeantFollowerMSHelper.h"

#include "GaudiKernel/ITHistSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "TTree.h"
// Trk
#include <cmath>

#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkExUtils/ExtrapolationCache.h"
#include "TrkMaterialOnTrack/EnergyLoss.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkMaterialOnTrack/ScatteringAngles.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/PlaneSurface.h"

Trk::GeantFollowerMSHelper::GeantFollowerMSHelper(const std::string& t,
                                                  const std::string& n,
                                                  const IInterface* p)
    : base_class(t, n, p),
      m_extrapolator(""),
      m_extrapolateDirectly(false),
      m_extrapolateIncrementally(false),
      m_speedup(false),
      m_useCovMatrix(true),
      m_useIDExit(false),
      m_parameterCache(nullptr),
      m_parameterCacheCov(nullptr),
      m_parameterCacheEntry(nullptr),
      m_parameterCacheEntryCov(nullptr),
      m_tX0Cache(0.),
      m_crossedEntry(false),
      m_exitLayer(false),
      m_destinationSurface(),
      m_validationTreeName("G4Follower"),
      m_validationTreeDescription("Output of the G4Follower_"),
      m_validationTreeFolder("/val/G4Follower"),
      m_validationTree(nullptr) {
  declareProperty("Extrapolator", m_extrapolator);
  declareProperty("ExtrapolateDirectly", m_extrapolateDirectly);
  declareProperty("ExtrapolateIncrementally", m_extrapolateIncrementally);

  // SpeedUp False takes more CPU because it will stop at each G4 Step in the
  declareProperty("SpeedUp", m_speedup);
  declareProperty("UseCovMatrix", m_useCovMatrix);
  declareProperty("UseIDExit",m_useIDExit);  // used for validating the ID and Calo tracking

      
}

// destructor
Trk::GeantFollowerMSHelper::~GeantFollowerMSHelper() = default;

// Athena standard methods
// initialize
StatusCode Trk::GeantFollowerMSHelper::initialize() {
  m_treeData = std::make_unique<TreeData>();





  if (m_extrapolator.retrieve().isFailure()) {
    ATH_MSG_ERROR("Could not retrieve Extrapolator " << m_extrapolator
                                                     << " . Abort.");
    return StatusCode::FAILURE;
  }

  if (m_elossupdator.retrieve().isFailure()) {
    ATH_MSG_ERROR("Could not retrieve ElossUpdator " << m_elossupdator
                                                     << " . Abort.");
    return StatusCode::FAILURE;
  }


  if (m_speedup) {
    ATH_MSG_INFO(" SpeedUp GeantFollowerMS ");
  } else {
    ATH_MSG_INFO(" NO SpeedUp GeantFollowerMS ");
  }
  ATH_MSG_INFO("initialize()");

  // create the new Tree
  m_validationTree = new TTree(m_validationTreeName.c_str(),
                               m_validationTreeDescription.c_str());

  m_validationTree->Branch("InitX", &m_treeData->m_t_x, "initX/F");
  m_validationTree->Branch("InitY", &m_treeData->m_t_y, "initY/F");
  m_validationTree->Branch("InitZ", &m_treeData->m_t_z, "initZ/F");
  m_validationTree->Branch("InitTheta", &m_treeData->m_t_theta, "initTheta/F");
  m_validationTree->Branch("InitEta", &m_treeData->m_t_eta, "initEta/F");
  m_validationTree->Branch("InitPhi", &m_treeData->m_t_phi, "initPhi/F");
  m_validationTree->Branch("InitP", &m_treeData->m_t_p, "initP/F");
  m_validationTree->Branch("InitPdg", &m_treeData->m_t_pdg, "initPdg/I");
  m_validationTree->Branch("InitCharge", &m_treeData->m_t_charge, "initQ/F");

  m_validationTree->Branch("MEntryX", &m_treeData->m_m_x, "mentryX/F");
  m_validationTree->Branch("MEntryY", &m_treeData->m_m_y, "mentryY/F");
  m_validationTree->Branch("MEntryZ", &m_treeData->m_m_z, "mentryZ/F");
  m_validationTree->Branch("MEntryTheta", &m_treeData->m_m_theta,
                           "mentryTheta/F");
  m_validationTree->Branch("MEntryEta", &m_treeData->m_m_eta, "mentryEta/F");
  m_validationTree->Branch("MEntryPhi", &m_treeData->m_m_phi, "mentryPhi/F");
  m_validationTree->Branch("MEntryP", &m_treeData->m_m_p, "mentryP/F");

  m_validationTree->Branch("BackX", &m_treeData->m_b_x, "backX/F");
  m_validationTree->Branch("BackY", &m_treeData->m_b_y, "backY/F");
  m_validationTree->Branch("BackZ", &m_treeData->m_b_z, "backZ/F");
  m_validationTree->Branch("BackTheta", &m_treeData->m_b_theta, "backTheta/F");
  m_validationTree->Branch("BackEta", &m_treeData->m_b_eta, "backEta/F");
  m_validationTree->Branch("BackPhi", &m_treeData->m_b_phi, "backPhi/F");
  m_validationTree->Branch("BackP", &m_treeData->m_b_p, "backP/F");
  m_validationTree->Branch("BackX0", &m_treeData->m_b_X0, "backX0/F");
  m_validationTree->Branch("BackEloss", &m_treeData->m_b_Eloss, "backEloss/F");

  m_validationTree->Branch("G4Steps", &m_treeData->m_g4_steps, "g4steps/I");
  m_validationTree->Branch("TrkStepScats", &m_treeData->m_trk_scats,
                           "trkscats/I");

  m_validationTree->Branch("G4StepP", m_treeData->m_g4_p, "g4stepP[g4steps]/F");
  m_validationTree->Branch("G4StepEta", m_treeData->m_g4_eta,
                           "g4stepEta[g4steps]/F");
  m_validationTree->Branch("G4StepTheta", m_treeData->m_g4_theta,
                           "g4stepTheta[g4steps]/F");
  m_validationTree->Branch("G4StepPhi", m_treeData->m_g4_phi,
                           "g4stepPhi[g4steps]/F");
  m_validationTree->Branch("G4StepX", m_treeData->m_g4_x, "g4stepX[g4steps]/F");
  m_validationTree->Branch("G4StepY", m_treeData->m_g4_y, "g4stepY[g4steps]/F");
  m_validationTree->Branch("G4StepZ", m_treeData->m_g4_z, "g4stepZ[g4steps]/F");
  m_validationTree->Branch("G4AccumTX0", m_treeData->m_g4_tX0,
                           "g4stepAccTX0[g4steps]/F");
  m_validationTree->Branch("G4StepT", m_treeData->m_g4_t,
                           "g4stepTX[g4steps]/F");
  m_validationTree->Branch("G4StepX0", m_treeData->m_g4_X0,
                           "g4stepX0[g4steps]/F");

  m_validationTree->Branch("TrkStepStatus", m_treeData->m_trk_status,
                           "trkstepStatus[g4steps]/I");
  m_validationTree->Branch("TrkStepP", m_treeData->m_trk_p,
                           "trkstepP[g4steps]/F");
  m_validationTree->Branch("TrkStepEta", m_treeData->m_trk_eta,
                           "trkstepEta[g4steps]/F");
  m_validationTree->Branch("TrkStepTheta", m_treeData->m_trk_theta,
                           "trkstepTheta[g4steps]/F");
  m_validationTree->Branch("TrkStepPhi", m_treeData->m_trk_phi,
                           "trkstepPhi[g4steps]/F");
  m_validationTree->Branch("TrkStepX", m_treeData->m_trk_x,
                           "trkstepX[g4steps]/F");
  m_validationTree->Branch("TrkStepY", m_treeData->m_trk_y,
                           "trkstepY[g4steps]/F");
  m_validationTree->Branch("TrkStepZ", m_treeData->m_trk_z,
                           "trkstepZ[g4steps]/F");
  m_validationTree->Branch("TrkStepLocX", m_treeData->m_trk_lx,
                           "trkstepLX[g4steps]/F");
  m_validationTree->Branch("TrkStepLocY", m_treeData->m_trk_ly,
                           "trkstepLY[g4steps]/F");
  m_validationTree->Branch("TrkStepEloss", m_treeData->m_trk_eloss,
                           "trkstepEloss[g4steps]/F");
  m_validationTree->Branch("TrkStepEloss1", m_treeData->m_trk_eloss1,
                           "trkstepEloss1[g4steps]/F");
  m_validationTree->Branch("TrkStepEloss0", m_treeData->m_trk_eloss0,
                           "trkstepEloss0[g4steps]/F");
  m_validationTree->Branch("TrkStepEloss5", m_treeData->m_trk_eloss5,
                           "trkstepEloss5[g4steps]/F");
  m_validationTree->Branch("TrkStepEloss10", m_treeData->m_trk_eloss10,
                           "trkstepEloss10[g4steps]/F");
  m_validationTree->Branch("TrkStepScaleEloss", m_treeData->m_trk_scaleeloss,
                           "trkstepScaleEloss[g4steps]/F");
  m_validationTree->Branch("TrkStepScaleX0", m_treeData->m_trk_scalex0,
                           "trkstepScaleX0[g4steps]/F");
  m_validationTree->Branch("TrkStepX0", m_treeData->m_trk_x0,
                           "trkstepX0[g4steps]/F");
  m_validationTree->Branch("TrkStepErd0", m_treeData->m_trk_erd0,
                           "trkstepErd0[g4steps]/F");
  m_validationTree->Branch("TrkStepErz0", m_treeData->m_trk_erz0,
                           "trkstepErz0[g4steps]/F");
  m_validationTree->Branch("TrkStepErphi", m_treeData->m_trk_erphi,
                           "trkstepErphi[g4steps]/F");
  m_validationTree->Branch("TrkStepErtheta", m_treeData->m_trk_ertheta,
                           "trkstepErtheta[g4steps]/F");
  m_validationTree->Branch("TrkStepErqoverp", m_treeData->m_trk_erqoverp,
                           "trkstepErqoverp[g4steps]/F");

  m_validationTree->Branch("TrkStepScatStatus", m_treeData->m_trk_sstatus,
                           "trkscatStatus[trkscats]/I");
  m_validationTree->Branch("TrkStepScatX", m_treeData->m_trk_sx,
                           "trkscatX[trkscats]/F");
  m_validationTree->Branch("TrkStepScatY", m_treeData->m_trk_sy,
                           "trkscatY[trkscats]/F");
  m_validationTree->Branch("TrkStepScatZ", m_treeData->m_trk_sz,
                           "trkscatZ[trkscats]/F");
  m_validationTree->Branch("TrkStepScatX0", m_treeData->m_trk_sx0,
                           "trkscatX0[trkscats]/F");
  m_validationTree->Branch("TrkStepScatEloss", m_treeData->m_trk_seloss,
                           "trkscatEloss[trkscats]/F");
  m_validationTree->Branch("TrkStepScatMeanIoni", m_treeData->m_trk_smeanIoni,
                           "trkscatMeanIoni[trkscats]/F");
  m_validationTree->Branch("TrkStepScatSigIoni", m_treeData->m_trk_ssigIoni,
                           "trkscatSigIoni[trkscats]/F");
  m_validationTree->Branch("TrkStepScatMeanRad", m_treeData->m_trk_smeanRad,
                           "trkscatMeanRad[trkscats]/F");
  m_validationTree->Branch("TrkStepScatSigRad", m_treeData->m_trk_ssigRad,
                           "trkscatSigRad[trkscats]/F");
  m_validationTree->Branch("TrkStepScatSigTheta", m_treeData->m_trk_ssigTheta,
                           "trkscatSigTheta[trkscats]/F");
  m_validationTree->Branch("TrkStepScatSigPhi", m_treeData->m_trk_ssigPhi,
                           "trkscatSigPhi[trkscats]/F");

  m_crossedEntry = false;
  m_exitLayer = false;
  // now register the Tree
  SmartIF<ITHistSvc> tHistSvc{Gaudi::svcLocator()->service("THistSvc")};
  if ( !tHistSvc ) {
    ATH_MSG_ERROR(
        "Could not find Hist Service -> Switching ValidationMode Off !");
    delete m_validationTree;
    m_validationTree = nullptr;
  }
  if ((tHistSvc->regTree(m_validationTreeFolder, m_validationTree))
          .isFailure()) {
    ATH_MSG_ERROR(
        "Could not register the validation Tree -> Switching ValidationMode "
        "Off !");
    delete m_validationTree;
    m_validationTree = nullptr;
  }

  ATH_MSG_INFO("initialize() successful");
  return StatusCode::SUCCESS;
}

StatusCode Trk::GeantFollowerMSHelper::finalize() {
  return StatusCode::SUCCESS;
}

void Trk::GeantFollowerMSHelper::beginEvent() {
  m_treeData->m_t_x = 0.;
  m_treeData->m_t_y = 0.;
  m_treeData->m_t_z = 0.;
  m_treeData->m_t_theta = 0.;
  m_treeData->m_t_eta = 0.;
  m_treeData->m_t_phi = 0.;
  m_treeData->m_t_p = 0.;
  m_treeData->m_t_charge = 0.;
  m_treeData->m_t_pdg = 0;

  m_treeData->m_m_x = 0.;
  m_treeData->m_m_y = 0.;
  m_treeData->m_m_z = 0.;
  m_treeData->m_m_theta = 0.;
  m_treeData->m_m_eta = 0.;
  m_treeData->m_m_phi = 0.;
  m_treeData->m_m_p = 0.;

  m_treeData->m_b_x = 0.;
  m_treeData->m_b_y = 0.;
  m_treeData->m_b_z = 0.;
  m_treeData->m_b_theta = 0.;
  m_treeData->m_b_eta = 0.;
  m_treeData->m_b_phi = 0.;
  m_treeData->m_b_p = 0.;
  m_treeData->m_b_X0 = 0.;
  m_treeData->m_b_Eloss = 0.;

  m_treeData->m_g4_steps = -1;
  m_treeData->m_g4_stepsEntry = -1;
  m_treeData->m_trk_scats = 0;
  m_tX0Cache = 0.;

  m_crossedEntry = false;
  m_exitLayer = false;
}

void Trk::GeantFollowerMSHelper::trackParticle(const G4ThreeVector& pos,
                                               const G4ThreeVector& mom,
                                               int pdg, double charge, float t,
                                               float X0) {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  // as the MS starts at 6736 in R.07 the cut is just before

  double zMuonEntry = 6735.;
  double rMuonEntry = 4254;
  double zMuonExit = 21800.;
  double rMuonExit = 12500.;
  double zIDExit = 2720.;
  double rIDExit = 1080.;

  double zEntry = zMuonEntry;
  double rEntry = rMuonEntry;
  double zExit = zMuonExit;
  double rExit = rMuonExit;
 
  if(m_useIDExit) {
     zEntry = zIDExit;
     rEntry = rIDExit;
     zExit = zMuonEntry;
     rExit = rMuonEntry;
  } 
 
  double scale = 1.;

  Amg::Vector3D npos(scale * pos.x(), scale * pos.y(), scale * pos.z());
  Amg::Vector3D nmom(mom.x(), mom.y(), mom.z());

  if (m_treeData->m_g4_steps == -1) {
    ATH_MSG_INFO("Initial step ... preparing event cache.");
    m_treeData->m_t_x = npos.x();
    m_treeData->m_t_y = npos.y();
    m_treeData->m_t_z = npos.z();
    m_treeData->m_t_theta = nmom.theta();
    m_treeData->m_t_eta = nmom.eta();
    m_treeData->m_t_phi = nmom.phi();
    m_treeData->m_t_p = nmom.mag();
    m_treeData->m_t_charge = charge;
    m_treeData->m_t_pdg = pdg;
    m_treeData->m_g4_steps = 0;
    m_tX0Cache = 0.;

    // construct the intial parameters
    m_parameterCache = new Trk::CurvilinearParameters(npos, nmom, charge);
    AmgSymMatrix(5) covMatrix;
    covMatrix.setZero();
    // covMatrix(0, 0) = 1e-34;
    // covMatrix(1, 1) = 1e-34;
    // covMatrix(2, 2) = 1e-34;
    // covMatrix(3, 4) = 1e-34;
    // covMatrix(4, 4) = 1e-34;
    ATH_MSG_DEBUG(" covMatrix " << covMatrix);
    m_parameterCacheCov = new Trk::CurvilinearParameters(npos, nmom, charge,
                                                         std::move(covMatrix));
    ATH_MSG_DEBUG(" Made m_parameterCacheCov with covMatrix "
                  << *m_parameterCacheCov->covariance());
    return;
  }

  float tX0 = X0 > 10e-5 ? t / X0 : 0.;
  m_tX0Cache += tX0;
  ATH_MSG_DEBUG(" position R " << npos.perp() << " z " << npos.z() << " X0 "
                               << X0 << " t " << t << " m_tX0Cache "
                               << m_tX0Cache);

  bool useEntry = true;

  // Muon Entry or ID exit
  if (useEntry && !m_crossedEntry &&
      (std::fabs(npos.z()) > zEntry || npos.perp() > rEntry)) {
    m_treeData->m_m_x = npos.x();
    m_treeData->m_m_y = npos.y();
    m_treeData->m_m_z = npos.z();
    m_treeData->m_m_theta = nmom.theta();
    m_treeData->m_m_eta = nmom.eta();
    m_treeData->m_m_phi = nmom.phi();
    m_treeData->m_m_p = nmom.mag();
    // overwrite everything before ME layer
    m_treeData->m_g4_stepsEntry = 0;
    // construct the intial parameters
    m_parameterCacheEntry = new Trk::CurvilinearParameters(npos, nmom, charge);
    //m_parameterCache = new Trk::CurvilinearParameters(npos, nmom, charge);
    AmgSymMatrix(5) covMatrix;
    covMatrix.setZero();
    m_parameterCacheEntryCov = new Trk::CurvilinearParameters(
        npos, nmom, charge, std::move(covMatrix));
    ATH_MSG_DEBUG("m_crossedEntry x "
                  << m_parameterCacheEntry->position().x() << " y "
                  << m_parameterCacheEntry->position().y() << " z "
                  << m_parameterCacheEntry->position().z());
    m_crossedEntry = true;
    Trk::CurvilinearParameters g4Parameters =
        Trk::CurvilinearParameters(npos, nmom, m_treeData->m_t_charge);
    // Muon Entry
    m_destinationSurface = g4Parameters.associatedSurface();
  }

  // jumping over inital step
  m_treeData->m_g4_steps =
      (m_treeData->m_g4_steps == -1) ? 0 : m_treeData->m_g4_steps;

  if (!m_parameterCache) {
    ATH_MSG_WARNING("No Parameters available. Bailing out.");
    return;
  }

  if (m_treeData->m_g4_steps >= MAXPROBES) {
    ATH_MSG_WARNING("Maximum number of " << MAXPROBES
                                         << " reached, step is ignored.");
    return;
  }

  // DO NOT store before Entry is crossed (gain CPU)
  if (!m_crossedEntry) return;
  if (m_exitLayer) return;

  // PK 2023
  // store G4 steps if m_crossedEntry
  m_treeData->m_g4_p[m_treeData->m_g4_steps] = nmom.mag();
  m_treeData->m_g4_eta[m_treeData->m_g4_steps] = nmom.eta();
  m_treeData->m_g4_theta[m_treeData->m_g4_steps] = nmom.theta();
  m_treeData->m_g4_phi[m_treeData->m_g4_steps] = nmom.phi();
  m_treeData->m_g4_x[m_treeData->m_g4_steps] = npos.x();
  m_treeData->m_g4_y[m_treeData->m_g4_steps] = npos.y();
  m_treeData->m_g4_z[m_treeData->m_g4_steps] = npos.z();
  m_treeData->m_g4_tX0[m_treeData->m_g4_steps] = m_tX0Cache;
  m_treeData->m_g4_t[m_treeData->m_g4_steps] = t;
  m_treeData->m_g4_X0[m_treeData->m_g4_steps] = X0;

  m_treeData->m_trk_p[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_eta[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_theta[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_phi[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_x[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_y[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_z[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_lx[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_ly[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_eloss[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_eloss0[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_eloss1[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_eloss5[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_eloss10[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_scaleeloss[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_scalex0[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_x0[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_status[m_treeData->m_g4_steps] = 0;
  m_treeData->m_trk_erd0[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_erz0[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_erphi[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_ertheta[m_treeData->m_g4_steps] = 0.;
  m_treeData->m_trk_erqoverp[m_treeData->m_g4_steps] = 0.;
  ++m_treeData->m_g4_steps;

  ATH_MSG_DEBUG("initialise m_treeData->m_g4_steps" << m_treeData->m_g4_steps);

  bool crossedExitLayer = false;
  // ID envelope
  if (std::fabs(npos.z()) > zExit || npos.perp() > rExit)
    crossedExitLayer = true;

  ATH_MSG_DEBUG("npos Z: " << npos.z() << "npos prep: " << npos.perp()
                           << "crossedExitLayer: " << crossedExitLayer);

  if (m_speedup) {
    ATH_MSG_DEBUG("Starting speed up:"
                  << "m_crossedEntry: " << m_crossedEntry
                  << "m_treeData->m_g4_steps: " << m_treeData->m_g4_steps
                  << "m_treeData->m_g4_stepsEntry: " << m_treeData->m_g4_stepsEntry);
    if (m_crossedEntry && m_treeData->m_g4_steps >= 2 && !crossedExitLayer)
      return;
  }

  Trk::EnergyLoss eloss = EnergyLoss(0., 0., 0., 0., 0., 0);
  Trk::ExtrapolationCache extrapolationCache = ExtrapolationCache(0., &eloss);

  // Cache ONLY used for extrapolateM and extrapolate with covariance Matrix

  // parameters of the G4 step point
  Trk::CurvilinearParameters g4Parameters =
      Trk::CurvilinearParameters(npos, nmom, m_treeData->m_t_charge);
  // destination surface
  const Trk::PlaneSurface& destinationSurface =
      g4Parameters.associatedSurface();
  // extrapolate to the destination surface
  std::unique_ptr<Trk::TrackParameters> trkParameters =
      m_extrapolateDirectly && m_crossedEntry
          ? m_extrapolator->extrapolateDirectly(
                ctx, *m_parameterCache, destinationSurface, Trk::alongMomentum,
                false, Trk::muon)
          : m_extrapolator->extrapolate(ctx, *m_parameterCache,
                                        destinationSurface, Trk::alongMomentum,
                                        false, Trk::muon);
  if (!trkParameters) {
    ATH_MSG_DEBUG(
        " G4 extrapolate failed without covariance to destination surface ");
  }
  if (m_treeData->m_g4_stepsEntry == 0 && m_useCovMatrix) {
    ATH_MSG_DEBUG(" Extrapolate m_parameterCacheCov with covMatrix ");
    extrapolationCache.reset();
    trkParameters = m_extrapolateDirectly && m_crossedEntry
                        ? m_extrapolator->extrapolateDirectly(
                              ctx, *m_parameterCacheCov, destinationSurface,
                              Trk::alongMomentum, false, Trk::muon)
                        : m_extrapolator->extrapolate(
                              ctx, *m_parameterCacheCov, destinationSurface,
                              Trk::alongMomentum, false, Trk::muon,
                              Trk::addNoise, &extrapolationCache);
    if (!trkParameters) {
      ATH_MSG_DEBUG(" G4 extrapolate failed with covariance to Muon Entry or ID Exit");
      ATH_MSG_DEBUG(" Redo G4 extrapolateM without covariance matrix to Muon Entry or ID Exit ");
      extrapolationCache.reset();
      trkParameters = m_extrapolateDirectly && m_crossedEntry
                         ? m_extrapolator->extrapolateDirectly(
                               ctx, *m_parameterCache, destinationSurface,
                               Trk::alongMomentum, false, Trk::muon)
                         : m_extrapolator->extrapolate(
                               ctx, *m_parameterCache, destinationSurface,
                               Trk::alongMomentum, false, Trk::muon,
                               Trk::addNoise, &extrapolationCache);

    } else {
      ATH_MSG_DEBUG(
          " G4 extrapolate succesfull with covariance to Muon Entry or ID exit "
          << " X0 " << extrapolationCache.x0tot() << " Eloss deltaE "
          << extrapolationCache.eloss()->deltaE() << " Eloss sigma "
          << extrapolationCache.eloss()->sigmaDeltaE() << " meanIoni "
          << extrapolationCache.eloss()->meanIoni() << " sigmaIoni "
          << extrapolationCache.eloss()->sigmaIoni() << " meanRad "
          << extrapolationCache.eloss()->meanRad() << " sigmaRad "
          << extrapolationCache.eloss()->sigmaRad() << " depth "
          << extrapolationCache.eloss()->length());
    }
  }

  // sroe: coverity 31530
  m_treeData->m_trk_status[m_treeData->m_g4_steps] = trkParameters ? 1 : 0;
  ATH_MSG_DEBUG("m_treeData->m_g4_steps: "
                << m_treeData->m_g4_steps << " exit Layer: " << crossedExitLayer
                << " track parameters: " << trkParameters.get());
  ATH_MSG_DEBUG("m_parameterCache: " << m_parameterCache);
  if (!trkParameters) {
    return;
  }

  ATH_MSG_DEBUG(" exit Layer: " << crossedExitLayer
                                << "track parameters: " << trkParameters.get());
  if (crossedExitLayer) {
    ATH_MSG_DEBUG(" exit layer found ");
    // PK 2023
    m_exitLayer = true;
    m_treeData->m_trk_status[m_treeData->m_g4_steps] = 1000;
    // Get extrapolatio with errors
    if (m_useCovMatrix) {
      extrapolationCache.reset();
      ATH_MSG_DEBUG(" Extrapolate m_parameterCacheEntryCov with covMatrix "
                    << " x " << m_parameterCacheEntryCov->position().x() << " y "
                    << m_parameterCacheEntryCov->position().y() << " z "
                    << m_parameterCacheEntryCov->position().z());
      ATH_MSG_DEBUG(" m_parameterCacheEntryCov "
                    << "m_extrapolateDirectly: " << m_extrapolateDirectly
                    << "m_crossedEntry: " << m_crossedEntry);

      trkParameters = m_extrapolateDirectly && m_crossedEntry
                          ? m_extrapolator->extrapolateDirectly(
                                ctx, *m_parameterCacheEntryCov, destinationSurface,
                                Trk::alongMomentum, false, Trk::muon)
                          : m_extrapolator->extrapolate(
                                ctx, *m_parameterCacheEntryCov, destinationSurface,
                                Trk::alongMomentum, false, Trk::muon,
                                Trk::addNoise, &extrapolationCache);
      if (trkParameters) {
        ATH_MSG_DEBUG("extrapolation with m_parameterCacheEntryCov succeeded ");
      } else {
        ATH_MSG_DEBUG(" extrapolation failed with m_parameterCacheEntryCov ");
        if (!m_parameterCacheEntryCov) {
          ATH_MSG_DEBUG(" failed due to m_parameterCacheEntryCov is zero");
        }
        extrapolationCache.reset();
        trkParameters = m_extrapolateDirectly
                            ? m_extrapolator->extrapolateDirectly(
                                  ctx, *m_parameterCacheEntry, destinationSurface,
                                  Trk::alongMomentum, false, Trk::muon)
                            : m_extrapolator->extrapolate(
                                  ctx, *m_parameterCacheEntry, destinationSurface,
                                  Trk::alongMomentum, false, Trk::muon,
                                  Trk::addNoise, &extrapolationCache);
      }
      if (trkParameters)
        ATH_MSG_DEBUG("extrapolation with m_parameterCacheEntry succeeded");
    } else {
      // no covariance matrix
      extrapolationCache.reset();
      trkParameters = m_extrapolateDirectly
                          ? m_extrapolator->extrapolateDirectly(
                                ctx, *m_parameterCacheEntry, destinationSurface,
                                Trk::alongMomentum, false, Trk::muon)
                          : m_extrapolator->extrapolate(
                                ctx, *m_parameterCacheEntry, destinationSurface,
                                Trk::alongMomentum, false, Trk::muon,
                                Trk::addNoise, &extrapolationCache);
    }

    // Backwards from Exit to ME
    if (trkParameters) {
      ATH_MSG_DEBUG(" forward extrapolation succeeded ");
      bool doBackWard = false;
      if (doBackWard) {
        std::unique_ptr<Trk::TrackParameters> trkParameters_BACK =
            m_extrapolateDirectly
                ? m_extrapolator->extrapolateDirectly(
                      ctx, *trkParameters, m_destinationSurface,
                      Trk::oppositeMomentum, false, Trk::muon)
                : m_extrapolator->extrapolate(
                      ctx, *trkParameters, m_destinationSurface,
                      Trk::oppositeMomentum, false, Trk::muon);
        if (trkParameters_BACK) {
          ATH_MSG_DEBUG(" back extrapolation succeeded ");
          m_exitLayer = true;
          m_treeData->m_b_p = trkParameters_BACK->momentum().mag();
          m_treeData->m_b_eta = trkParameters_BACK->momentum().eta();
          m_treeData->m_b_theta = trkParameters_BACK->momentum().theta();
          m_treeData->m_b_phi = trkParameters_BACK->momentum().phi();
          m_treeData->m_b_x = trkParameters_BACK->position().x();
          m_treeData->m_b_y = trkParameters_BACK->position().y();
          m_treeData->m_b_z = trkParameters_BACK->position().z();
          if (std::fabs(m_treeData->m_m_p - m_treeData->m_b_p) > 10.)
            ATH_MSG_DEBUG(
                " Back extrapolation to Muon Entry or ID Exit finds different "
                "momentum  difference MeV "
                << m_treeData->m_m_p - m_treeData->m_b_p);
          // delete  trkParameters_BACK;
          extrapolationCache.reset();
          const std::vector<const Trk::TrackStateOnSurface*>* matvec_BACK =
              m_extrapolator->extrapolateM(
                  ctx, *trkParameters, m_destinationSurface,
                  Trk::oppositeMomentum, false, Trk::muon, &extrapolationCache);
          double Eloss = 0.;
          double x0 = 0.;

          int mmat = 0;
          if (matvec_BACK && !matvec_BACK->empty()) {
            std::vector<const Trk::TrackStateOnSurface*>::const_iterator it =
                matvec_BACK->begin();
            std::vector<const Trk::TrackStateOnSurface*>::const_iterator
                it_end = matvec_BACK->end();
            for (; it != it_end; ++it) {
              const Trk::MaterialEffectsBase* matEf =
                  (*it)->materialEffectsOnTrack();
              if (matEf) {
                mmat++;
                if (m_treeData->m_trk_status[m_treeData->m_g4_steps] == 1000)
                  ATH_MSG_DEBUG(" mmat " << mmat << " matEf->thicknessInX0() "
                                         << matEf->thicknessInX0());
                x0 += matEf->thicknessInX0();
                const Trk::MaterialEffectsOnTrack* matEfs =
                    dynamic_cast<const Trk::MaterialEffectsOnTrack*>(matEf);
                if (not matEfs) continue;
                //
                double eloss0 = 0.;
                double meanIoni = 0.;
                double sigmaIoni = 0.;
                double meanRad = 0.;
                double sigmaRad = 0.;
                double sigmaTheta = 0.;
                double sigmaPhi = 0.;

                const Trk::EnergyLoss* eLoss = (matEfs)->energyLoss();
                if (eLoss) {
                  Eloss += eLoss->deltaE();
                  eloss0 = eLoss->deltaE();
                  meanIoni = eLoss->meanIoni();
                  sigmaIoni = eLoss->sigmaIoni();
                  meanRad = eLoss->meanRad();
                  sigmaRad = eLoss->sigmaRad();
                  if (m_treeData->m_trk_status[m_treeData->m_g4_steps] == 1000)
                    ATH_MSG_DEBUG(" mmat " << mmat << " eLoss->deltaE() "
                                           << eLoss->deltaE()
                                           << "  eLoss->length() "
                                           << eLoss->length());
                }

                const Trk::ScatteringAngles* scatAng =
                    (matEfs)->scatteringAngles();
                if (scatAng) {
                  sigmaTheta = scatAng->sigmaDeltaTheta();
                  sigmaPhi = scatAng->sigmaDeltaPhi();
                }
                if (m_treeData->m_trk_scats < 500) {
                  // backwards
                  if (m_treeData->m_trk_status[m_treeData->m_g4_steps] == 1000)
                    m_treeData->m_trk_sstatus[m_treeData->m_trk_scats] = -1000;
                  if ((*it)->trackParameters()) {
                    m_treeData->m_trk_sx[m_treeData->m_trk_scats] =
                        (*it)->trackParameters()->position().x();
                    m_treeData->m_trk_sy[m_treeData->m_trk_scats] =
                        (*it)->trackParameters()->position().y();
                    m_treeData->m_trk_sz[m_treeData->m_trk_scats] =
                        (*it)->trackParameters()->position().z();
                  }
                  m_treeData->m_trk_sx0[m_treeData->m_trk_scats] =
                      matEf->thicknessInX0();
                  m_treeData->m_trk_seloss[m_treeData->m_trk_scats] = eloss0;
                  m_treeData->m_trk_smeanIoni[m_treeData->m_trk_scats] =
                      meanIoni;
                  m_treeData->m_trk_ssigIoni[m_treeData->m_trk_scats] =
                      sigmaIoni;
                  m_treeData->m_trk_smeanRad[m_treeData->m_trk_scats] = meanRad;
                  m_treeData->m_trk_ssigRad[m_treeData->m_trk_scats] = sigmaRad;
                  m_treeData->m_trk_ssigTheta[m_treeData->m_trk_scats] =
                      sigmaTheta;
                  m_treeData->m_trk_ssigPhi[m_treeData->m_trk_scats] = sigmaPhi;
                  m_treeData->m_trk_scats++;
                }
              }
            }
          }
          m_treeData->m_b_X0 = x0;
          m_treeData->m_b_Eloss = Eloss;
          delete matvec_BACK;
        }
      }
    }
  }

  extrapolationCache.reset();
  const std::vector<const Trk::TrackStateOnSurface*>* matvec =
      m_extrapolator->extrapolateM(ctx, *m_parameterCache, destinationSurface,
                                   Trk::alongMomentum, false, Trk::muon,
                                   &extrapolationCache);

  if (matvec)
    ATH_MSG_DEBUG("MatVec 1: " << matvec->size());
  else
    ATH_MSG_DEBUG("MatVec 1: NULL");

  if (m_useCovMatrix) {
    ATH_MSG_DEBUG(
        "m_treeData->m_g4_stepsEntry (debug): " << m_treeData->m_g4_stepsEntry);
    if (m_treeData->m_g4_stepsEntry <= 0) {
      extrapolationCache.reset();
      matvec = m_extrapolator->extrapolateM(
          ctx, *m_parameterCacheCov, destinationSurface, Trk::alongMomentum,
          false, Trk::muon, &extrapolationCache);
      if (!matvec || matvec->empty()) {
        ATH_MSG_DEBUG(
            " G4 extrapolateM failed with covariance matrix to Muon Entry or ID Exit ");
        ATH_MSG_DEBUG(
            " Redo G4 extrapolateM without covariance matrix to Muon Entry or ID Exit ");
        extrapolationCache.reset();
        matvec = m_extrapolator->extrapolateM(
            ctx, *m_parameterCache, destinationSurface, Trk::alongMomentum,
            false, Trk::muon, &extrapolationCache);
      } else {
        ATH_MSG_DEBUG(
            " G4 extrapolateM succesfull with covariance matrix to Muon Entry or ID Exit ");
      }
      ATH_MSG_DEBUG("From Entry Cache X0 "
                    << extrapolationCache.x0tot() << " Eloss deltaE "
                    << extrapolationCache.eloss()->deltaE() << " Eloss sigma "
                    << extrapolationCache.eloss()->sigmaDeltaE() << " meanIoni "
                    << extrapolationCache.eloss()->meanIoni() << " sigmaIoni "
                    << extrapolationCache.eloss()->sigmaIoni() << " meanRad "
                    << extrapolationCache.eloss()->meanRad() << " sigmaRad "
                    << extrapolationCache.eloss()->sigmaRad());
    }
    if (m_treeData->m_g4_stepsEntry == 1) {
      extrapolationCache.reset();
      matvec = m_extrapolator->extrapolateM(
          ctx, *m_parameterCacheEntryCov, destinationSurface, Trk::alongMomentum,
          false, Trk::muon, &extrapolationCache);
      if (!matvec || matvec->empty()) {
        ATH_MSG_DEBUG(
            " G4 extrapolateM failed with covariance matrix to Muon or Calo Exit ");
        ATH_MSG_DEBUG(
            " Redo G4 extrapolateM without covariance matrix to Muon or Calo Exit ");
        extrapolationCache.reset();
        matvec = m_extrapolator->extrapolateM(
            ctx, *m_parameterCacheEntry, destinationSurface, Trk::alongMomentum,
            false, Trk::muon, &extrapolationCache);
      } else {
        ATH_MSG_DEBUG(
            " G4 extrapolateM succesfull with covariance matrix to Muon or Calo Exit ");
      }
      ATH_MSG_DEBUG("From Muon or Calo Exit Cache X0 "
                    << extrapolationCache.x0tot() << " Eloss deltaE "
                    << extrapolationCache.eloss()->deltaE() << " Eloss sigma "
                    << extrapolationCache.eloss()->sigmaDeltaE() << " meanIoni "
                    << extrapolationCache.eloss()->meanIoni() << " sigmaIoni "
                    << extrapolationCache.eloss()->sigmaIoni() << " meanRad "
                    << extrapolationCache.eloss()->meanRad() << " sigmaRad "
                    << extrapolationCache.eloss()->sigmaRad());
    }
  } else {
    if (m_treeData->m_g4_stepsEntry == 1) {
      extrapolationCache.reset();
      matvec = m_extrapolator->extrapolateM(
          ctx, *m_parameterCacheEntry, destinationSurface, Trk::alongMomentum,
          false, Trk::muon, &extrapolationCache);
      ATH_MSG_DEBUG(" G4 extrapolateM without covariance matrix to Muon Entry or ID Exit "
                    << " X0 " << extrapolationCache.x0tot() << " Eloss deltaE "
                    << extrapolationCache.eloss()->deltaE() << " Eloss sigma "
                    << extrapolationCache.eloss()->sigmaDeltaE() << " meanIoni "
                    << extrapolationCache.eloss()->meanIoni() << " sigmaIoni "
                    << extrapolationCache.eloss()->sigmaIoni() << " meanRad "
                    << extrapolationCache.eloss()->meanRad() << " sigmaRad "
                    << extrapolationCache.eloss()->sigmaRad());
    }
  }

  double Elosst = 0.;
  const std::vector<const Trk::TrackStateOnSurface*> matvecNewRepAggrUp =
      modifyTSOSvector(*matvec, 1.0, 1.0, true, true, true, 0., 0., 10000., 0.,
                       Elosst);

  double X0Scale = 1.0;
  double ElossScale = 1.0;

  double Eloss0 = 0.;
  double Eloss1 = 0.;
  double Eloss5 = 0.;
  double Eloss10 = 0.;
  bool system1 = false; // ID or Calorimeter
  bool system2 = false; // Calorimeter or Muon system 

  if (!matvec->empty()) {
    if (m_crossedEntry && !m_exitLayer) system1 = true;
    if (m_crossedEntry && m_exitLayer) system2 = true;
  }

  ATH_MSG_DEBUG(" ID or Calorimeter system  " << system1 << " Calorimeter or Muon system " << system2);
  if (system2) {
    //
    // Calorimeter or Muon system
    //
    m_elossupdator->getX0ElossScales(0, m_treeData->m_m_eta,
                                     m_treeData->m_m_phi, X0Scale, ElossScale);
    ATH_MSG_DEBUG(" system2 scales X0 " << X0Scale << " ElossScale "
                                           << ElossScale);

    const std::vector<const Trk::TrackStateOnSurface*> matvecNew1 =
        modifyTSOSvector(*matvec, X0Scale, 1., true, true, true, 0., 0.,
                         m_treeData->m_m_p, 0., Eloss1);
    const std::vector<const Trk::TrackStateOnSurface*> matvecNew0 =
        modifyTSOSvector(*matvec, X0Scale, ElossScale, true, true, true, 0., 0.,
                         m_treeData->m_m_p, 0., Eloss0);
    ATH_MSG_DEBUG(" muon system modify with 5 percent ");
    const std::vector<const Trk::TrackStateOnSurface*> matvecNew5 =
        modifyTSOSvector(*matvec, X0Scale, ElossScale, true, true, true, 0., 0.,
                         m_treeData->m_m_p, 0.05 * m_treeData->m_m_p, Eloss5);
    ATH_MSG_DEBUG(" muon system modify with 10 percent ");
    const std::vector<const Trk::TrackStateOnSurface*> matvecNew10 =
        modifyTSOSvector(*matvec, X0Scale, ElossScale, true, true, true, 0., 0.,
                         m_treeData->m_m_p, 0.10 * m_treeData->m_m_p, Eloss10);
  }
  if (system1) {
    //
    // ID or Calorimeter system
    //
    double phiCaloExit = atan2(m_treeData->m_m_y, m_treeData->m_m_x);
    m_elossupdator->getX0ElossScales(1, m_treeData->m_t_eta, phiCaloExit,
                                     X0Scale, ElossScale);
    ATH_MSG_DEBUG(" calorimeter scales X0 " << X0Scale << " ElossScale "
                                            << ElossScale);
    const std::vector<const Trk::TrackStateOnSurface*> matvecNew1 =
        modifyTSOSvector(*matvec, X0Scale, 1., true, true, true, 0., 0.,
                         m_treeData->m_m_p, 0., Eloss1);
    const std::vector<const Trk::TrackStateOnSurface*> matvecNew0 =
        modifyTSOSvector(*matvec, X0Scale, ElossScale, true, true, true, 0., 0.,
                         m_treeData->m_t_p, 0., Eloss0);
    if (std::fabs(Eloss1) > 0)
      ATH_MSG_DEBUG(" **** Cross Check calorimeter with Eloss Scale1 "
                    << Eloss1 << " Eloss0 " << Eloss0 << " ratio "
                    << Eloss0 / Eloss1);

    ATH_MSG_DEBUG(" calorimeter modify with 5 percent ");
    const std::vector<const Trk::TrackStateOnSurface*> matvecNew5 =
        modifyTSOSvector(*matvec, X0Scale, ElossScale, true, true, true, 0., 0.,
                         m_treeData->m_t_p, 0.05 * m_treeData->m_m_p, Eloss5);
    ATH_MSG_DEBUG(" calorimeter modify with 10 percent ");
    const std::vector<const Trk::TrackStateOnSurface*> matvecNew10 =
        modifyTSOSvector(*matvec, X0Scale, ElossScale, true, true, true, 0., 0.,
                         m_treeData->m_t_p, 0.10 * m_treeData->m_m_p, Eloss10);
  }

  ATH_MSG_DEBUG(" status " << m_treeData->m_trk_status[m_treeData->m_g4_steps]
                           << "Eloss1 " << Eloss1 << " Eloss0 " << Eloss0
                           << " Eloss5 " << Eloss5 << " Eloss10 " << Eloss10);

  double Eloss = 0.;
  double x0 = 0.;

  int mmat = 0;
  // PK 2023 only add scatterers for the ID or Calorimeter
  if (!(matvec->empty()) && m_treeData->m_g4_stepsEntry <= 1) {
    std::vector<const Trk::TrackStateOnSurface*>::const_iterator it =
        matvec->begin();
    std::vector<const Trk::TrackStateOnSurface*>::const_iterator it_end =
        matvec->end();
    for (; it != it_end; ++it) {
      const Trk::MaterialEffectsBase* matEf = (*it)->materialEffectsOnTrack();
      if (matEf) {
        mmat++;
        if (m_treeData->m_trk_status[m_treeData->m_g4_steps] == 1000)
          ATH_MSG_DEBUG(" mmat " << mmat << " matEf->thicknessInX0() "
                                 << matEf->thicknessInX0());
        x0 += matEf->thicknessInX0();
        const Trk::MaterialEffectsOnTrack* matEfs =
            dynamic_cast<const Trk::MaterialEffectsOnTrack*>(matEf);
        double eloss0 = 0.;
        double meanIoni = 0.;
        double sigmaIoni = 0.;
        double meanRad = 0.;
        double sigmaRad = 0.;
        double sigmaTheta = 0.;
        double sigmaPhi = 0.;
        if (matEfs) {
          const Trk::EnergyLoss* eLoss = (matEfs)->energyLoss();
          if (eLoss) {
            Eloss += eLoss->deltaE();
            eloss0 = eLoss->deltaE();
            meanIoni = eLoss->meanIoni();
            sigmaIoni = eLoss->sigmaIoni();
            meanRad = eLoss->meanRad();
            sigmaRad = eLoss->sigmaRad();
            ATH_MSG_DEBUG("m_treeData->m_g4_stepsEntry "
                          << m_treeData->m_g4_stepsEntry << " mmat " << mmat
                          << " X0 " << matEf->thicknessInX0()
                          << " eLoss->deltaE() " << eLoss->deltaE()
                          << " meanIoni " << meanIoni << " Total Eloss "
                          << Eloss << "  eLoss->length() " << eLoss->length());
          }
        }
        // sroe: coverity 31532
        const Trk::ScatteringAngles* scatAng =
            (matEfs) ? ((matEfs)->scatteringAngles()) : (nullptr);

        if (scatAng) {
          sigmaTheta = scatAng->sigmaDeltaTheta();
          sigmaPhi = scatAng->sigmaDeltaPhi();
          ATH_MSG_DEBUG("m_treeData->m_g4_stepsEntry "
                        << m_treeData->m_g4_stepsEntry << " mmat " << mmat
                        << " sigmaTheta " << sigmaTheta << " sigmaPhi "
                        << sigmaPhi);
        }

        if (m_treeData->m_trk_scats < 500) {
          if (m_treeData->m_g4_stepsEntry == 0 ||
              m_treeData->m_trk_status[m_treeData->m_g4_steps] == 1000) {
            // forwards
            if (m_treeData->m_g4_stepsEntry == 0)
              m_treeData->m_trk_sstatus[m_treeData->m_trk_scats] = 10;
            if (m_treeData->m_trk_status[m_treeData->m_g4_steps] == 1000)
              m_treeData->m_trk_sstatus[m_treeData->m_trk_scats] = 1000;
            if ((*it)->trackParameters()) {
              m_treeData->m_trk_sx[m_treeData->m_trk_scats] =
                  (*it)->trackParameters()->position().x();
              m_treeData->m_trk_sy[m_treeData->m_trk_scats] =
                  (*it)->trackParameters()->position().y();
              m_treeData->m_trk_sz[m_treeData->m_trk_scats] =
                  (*it)->trackParameters()->position().z();
            }
            m_treeData->m_trk_sx0[m_treeData->m_trk_scats] =
                matEf->thicknessInX0();
            m_treeData->m_trk_seloss[m_treeData->m_trk_scats] = eloss0;
            m_treeData->m_trk_smeanIoni[m_treeData->m_trk_scats] = meanIoni;
            m_treeData->m_trk_ssigIoni[m_treeData->m_trk_scats] = sigmaIoni;
            m_treeData->m_trk_smeanRad[m_treeData->m_trk_scats] = meanRad;
            m_treeData->m_trk_ssigRad[m_treeData->m_trk_scats] = sigmaRad;
            m_treeData->m_trk_ssigTheta[m_treeData->m_trk_scats] = sigmaTheta;
            m_treeData->m_trk_ssigPhi[m_treeData->m_trk_scats] = sigmaPhi;
            m_treeData->m_trk_scats++;
          }
        }
      }
    }
    delete matvec;
  }

  ATH_MSG_DEBUG("  m_treeData->m_g4_steps "
                << m_treeData->m_g4_steps << " Radius " << npos.perp() << " z "
                << npos.z() << " size matvec "
                << " total X0 " << x0 << " total Eloss " << Eloss);

  // go back and refill information
  // PK 2023
  if (m_treeData->m_g4_steps > 0) --m_treeData->m_g4_steps;
  // fill the geant information and the trk information
  m_treeData->m_g4_p[m_treeData->m_g4_steps] = nmom.mag();
  m_treeData->m_g4_eta[m_treeData->m_g4_steps] = nmom.eta();
  m_treeData->m_g4_theta[m_treeData->m_g4_steps] = nmom.theta();
  m_treeData->m_g4_phi[m_treeData->m_g4_steps] = nmom.phi();
  m_treeData->m_g4_x[m_treeData->m_g4_steps] = npos.x();
  m_treeData->m_g4_y[m_treeData->m_g4_steps] = npos.y();
  m_treeData->m_g4_z[m_treeData->m_g4_steps] = npos.z();
  m_treeData->m_g4_tX0[m_treeData->m_g4_steps] = m_tX0Cache;
  m_treeData->m_g4_t[m_treeData->m_g4_steps] = t;
  m_treeData->m_g4_X0[m_treeData->m_g4_steps] = X0;

  m_treeData->m_trk_p[m_treeData->m_g4_steps] =
      trkParameters ? trkParameters->momentum().mag() : 0.;
  m_treeData->m_trk_eta[m_treeData->m_g4_steps] =
      trkParameters ? trkParameters->momentum().eta() : 0.;
  m_treeData->m_trk_theta[m_treeData->m_g4_steps] =
      trkParameters ? trkParameters->momentum().theta() : 0.;
  m_treeData->m_trk_phi[m_treeData->m_g4_steps] =
      trkParameters ? trkParameters->momentum().phi() : 0.;
  m_treeData->m_trk_x[m_treeData->m_g4_steps] =
      trkParameters ? trkParameters->position().x() : 0.;
  m_treeData->m_trk_y[m_treeData->m_g4_steps] =
      trkParameters ? trkParameters->position().y() : 0.;
  m_treeData->m_trk_z[m_treeData->m_g4_steps] =
      trkParameters ? trkParameters->position().z() : 0.;
  m_treeData->m_trk_lx[m_treeData->m_g4_steps] =
      trkParameters ? trkParameters->parameters()[Trk::locX] : 0.;
  m_treeData->m_trk_ly[m_treeData->m_g4_steps] =
      trkParameters ? trkParameters->parameters()[Trk::locY] : 0.;
  m_treeData->m_trk_eloss[m_treeData->m_g4_steps] = Eloss;
  m_treeData->m_trk_eloss0[m_treeData->m_g4_steps] = Eloss0;
  m_treeData->m_trk_eloss1[m_treeData->m_g4_steps] = Eloss1;
  m_treeData->m_trk_eloss5[m_treeData->m_g4_steps] = Eloss5;
  m_treeData->m_trk_eloss10[m_treeData->m_g4_steps] = Eloss10;
  m_treeData->m_trk_scaleeloss[m_treeData->m_g4_steps] = ElossScale;
  m_treeData->m_trk_scalex0[m_treeData->m_g4_steps] = X0Scale;
  m_treeData->m_trk_x0[m_treeData->m_g4_steps] = x0;
  if (m_treeData->m_g4_stepsEntry == 0)
    m_treeData->m_trk_status[m_treeData->m_g4_steps] = 10;
  else
    m_treeData->m_trk_status[m_treeData->m_g4_steps] = 1000;

  double errord0 = 0.;
  double errorz0 = 0.;
  double errorphi = 0.;
  double errortheta = 0.;
  double errorqoverp = 0.;
  if (trkParameters && trkParameters->covariance()) {
    errord0 = (*trkParameters->covariance())(Trk::d0, Trk::d0);
    errorz0 = (*trkParameters->covariance())(Trk::z0, Trk::z0);
    errorphi = (*trkParameters->covariance())(Trk::phi, Trk::phi);
    errortheta = (*trkParameters->covariance())(Trk::theta, Trk::theta);
    errorqoverp = (*trkParameters->covariance())(Trk::qOverP, Trk::qOverP);
    ATH_MSG_DEBUG(" Covariance found for m_treeData->m_trk_status "
                  << m_treeData->m_trk_status[m_treeData->m_g4_steps]);
  }

  m_treeData->m_trk_erd0[m_treeData->m_g4_steps] = sqrt(errord0);
  m_treeData->m_trk_erz0[m_treeData->m_g4_steps] = sqrt(errorz0);
  m_treeData->m_trk_erphi[m_treeData->m_g4_steps] = sqrt(errorphi);
  m_treeData->m_trk_ertheta[m_treeData->m_g4_steps] = sqrt(errortheta);
  m_treeData->m_trk_erqoverp[m_treeData->m_g4_steps] = sqrt(errorqoverp);

  // reset X0 at Muon Entry or ID Exit
  if (m_treeData->m_g4_stepsEntry == 0) m_tX0Cache = 0.;
  // update the parameters if needed/configured
  if (m_extrapolateIncrementally && trkParameters) {
    delete m_parameterCache;
    // Unsure what to do here?
    // m_parameterCache = trkParameters;
  }

  ++m_treeData->m_g4_steps;
  if (m_treeData->m_g4_stepsEntry != -1) ++m_treeData->m_g4_stepsEntry;
}

std::vector<const Trk::TrackStateOnSurface*>
Trk::GeantFollowerMSHelper::modifyTSOSvector(
    const std::vector<const Trk::TrackStateOnSurface*>& matvec, double scaleX0,
    double scaleEloss, bool reposition, bool aggregate, bool updateEloss,
    double caloEnergy, double caloEnergyError, double pCaloEntry,
    double momentumError, double& Eloss_tot) const {
  //
  // inputs: TSOSs for material (matvec) and scale factors for X0 (scaleX0) and
  // Eloss (scaleEloss)
  //
  // returns: new vector of TSOSs including scaling of X0 and Eloss;
  //
  // options:
  // bool reposition    correct repositioning of the scattering centers in space
  // bool aggregate     put scattering centra together in two planes
  // bool update Eloss  correct energy loss 1) including the measured
  // calorimeter Eloss 2) include smearing of the muon momentum
  //
  // the routine should NOT be called for the ID
  // for best use in the Calorimeter:      bool reposition = true, bool
  // aggregate = true and updateEloss = true (measured caloEnergy and
  // caloEnergyError should be passed)
  //                                       note that the updateEloss is only
  //                                       active with aggregate = true
  // for best use in the Muon Specrometer: bool reposition = true, bool
  // aggregate = true and updateEloss = false
  //
  // if one runs with reposition = false the scattering centra are kept at the
  // END of the thick/dense material: that is not right for thick material for
  // thin it is OK
  //
  std::vector<const Trk::TrackStateOnSurface*> newTSOSvector;
  int maxsize = 2 * matvec.size();
  if (aggregate) maxsize = 2;
  newTSOSvector.reserve(maxsize);
  //
  // initialize total sum variables
  //
  //
  Eloss_tot = 0.;

  double X0_tot = 0.;

  double sigmaDeltaPhi2_tot = 0.;
  double sigmaDeltaTheta2_tot = 0.;
  double deltaE_tot = 0.;
  double sigmaDeltaE_tot = 0.;
  double sigmaPlusDeltaE_tot = 0.;
  double sigmaMinusDeltaE_tot = 0.;
  double deltaE_ioni_tot = 0.;
  double sigmaDeltaE_ioni_tot = 0.;
  double deltaE_rad_tot = 0.;
  double sigmaDeltaE_rad_tot = 0.;

  const Trk::TrackStateOnSurface* mprevious = nullptr;
  const Trk::TrackStateOnSurface* mfirst = nullptr;
  const Trk::TrackStateOnSurface* mlast = nullptr;
  Amg::Vector3D posFirst(0., 0., 0.);

  double deltaEFirst = 0.;

  double deltaPhi = 0.;
  double deltaTheta = 0.;

  int n_tot = 0;

  double w_tot = 0.;
  double wdist2 = 0.;
  Amg::Vector3D wdir(0., 0., 0.);
  Amg::Vector3D wpos(0., 0., 0.);

  std::bitset<Trk::MaterialEffectsBase::NumberOfMaterialEffectsTypes>
      meotPattern(0);
  meotPattern.set(Trk::MaterialEffectsBase::EnergyLossEffects);
  meotPattern.set(Trk::MaterialEffectsBase::ScatteringEffects);
  // meotPattern.set(Trk::MaterialEffectsBase::FittedMaterialEffects);

  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>
      typePattern(0);
  typePattern.set(Trk::TrackStateOnSurface::InertMaterial);
  typePattern.set(Trk::TrackStateOnSurface::Scatterer);

  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>
      typePatternDeposit(0);
  typePatternDeposit.set(Trk::TrackStateOnSurface::InertMaterial);
  typePatternDeposit.set(Trk::TrackStateOnSurface::CaloDeposit);
  typePatternDeposit.set(Trk::TrackStateOnSurface::Scatterer);

  for (const auto* m : matvec) {
    if (!m->trackParameters()) {
      ATH_MSG_WARNING("No trackparameters on TrackStateOnSurface ");
      continue;
    }
    if (m->materialEffectsOnTrack()) {
      double X0 = m->materialEffectsOnTrack()->thicknessInX0();
      const Trk::MaterialEffectsOnTrack* meot =
          dynamic_cast<const Trk::MaterialEffectsOnTrack*>(
              m->materialEffectsOnTrack());
      const Trk::EnergyLoss* energyLoss = nullptr;
      const Trk::ScatteringAngles* scat = nullptr;
      if (meot) {
        energyLoss = meot->energyLoss();
        if (energyLoss) {
          //           double deltaE = energyLoss->deltaE();
        } else {
          ATH_MSG_WARNING("No energyLoss on TrackStateOnSurface ");
          continue;
        }
        scat = meot->scatteringAngles();
        if (scat) {
          //           double dtheta = scat->sigmaDeltaTheta();
        } else {
          ATH_MSG_WARNING("No scatteringAngles on TrackStateOnSurface ");
          continue;
        }
      } else {
        ATH_MSG_WARNING("No materialEffectsOnTrack on TrackStateOnSurface ");
        continue;
      }

      double depth = energyLoss->length();
      ATH_MSG_DEBUG(" ");
      ATH_MSG_DEBUG(" original TSOS type "
                    << m->dumpType() << " TSOS surface "
                    << m->trackParameters()->associatedSurface()
                    << " position x " << m->trackParameters()->position().x()
                    << " y " << m->trackParameters()->position().y() << " z "
                    << m->trackParameters()->position().z() << " direction x "
                    << m->trackParameters()->momentum().unit().x() << " y "
                    << m->trackParameters()->momentum().unit().y() << " z "
                    << m->trackParameters()->momentum().unit().z() << " p "
                    << m->trackParameters()->momentum().mag() << " X0 " << X0
                    << " deltaE " << energyLoss->deltaE()
                    << " sigma deltaTheta " << scat->sigmaDeltaTheta()
                    << " depth " << depth);

      X0_tot += scaleX0 * X0;

      sigmaDeltaTheta2_tot +=
          scaleX0 * scat->sigmaDeltaTheta() * scat->sigmaDeltaTheta();
      sigmaDeltaPhi2_tot +=
          scaleX0 * scat->sigmaDeltaPhi() * scat->sigmaDeltaPhi();

      // Eloss sigma values add up linearly for Landau and exponential
      // distributions

      deltaE_tot += scaleEloss * energyLoss->deltaE();
      sigmaDeltaE_tot += scaleEloss * energyLoss->sigmaDeltaE();
      sigmaPlusDeltaE_tot += scaleEloss * energyLoss->sigmaPlusDeltaE();
      sigmaMinusDeltaE_tot += scaleEloss * energyLoss->sigmaMinusDeltaE();
      deltaE_ioni_tot += scaleEloss * energyLoss->meanIoni();
      sigmaDeltaE_ioni_tot += scaleEloss * energyLoss->sigmaIoni();
      deltaE_rad_tot += scaleEloss * energyLoss->meanRad();
      sigmaDeltaE_rad_tot += scaleEloss * energyLoss->sigmaRad();

      n_tot++;

      Amg::Vector3D dir = m->trackParameters()->momentum().unit();
      Amg::Vector3D pos = m->trackParameters()->position();
      if (mprevious) {
        dir += mprevious->trackParameters()->momentum().unit();
      }

      dir = dir / dir.mag();
      ATH_MSG_DEBUG(" position at end " << pos.x() << " y " << pos.y() << " z "
                                        << pos.z() << " perp " << pos.perp());
      ATH_MSG_DEBUG(" direction x " << dir.x() << " y " << dir.y() << " z "
                                    << dir.z());
      Amg::Vector3D pos0 = pos - (depth / 2. + depth / sqrt(12.)) * dir;
      Amg::Vector3D posNew = pos - (depth / 2. - depth / sqrt(12.)) * dir;

      ATH_MSG_DEBUG(" position scattering centre0 x "
                    << pos0.x() << " y " << pos0.y() << " z " << pos0.z()
                    << " perp " << pos0.perp());
      ATH_MSG_DEBUG(" position scattering centre1 x "
                    << posNew.x() << " y " << posNew.y() << " z " << posNew.z()
                    << " perp " << posNew.perp() << " distance "
                    << (pos0 - posNew).mag() << " depth " << depth);
      if (!mfirst) {
        mfirst = m;
        posFirst = pos0;
        deltaEFirst = energyLoss->deltaE();
      }
      mlast = m;

      double w = scat->sigmaDeltaTheta() * scat->sigmaDeltaTheta();
      w_tot += w;
      wpos += w * pos0 / 2.;
      wpos += w * posNew / 2.;
      wdir += w * dir;

      wdist2 += w * (pos0 - posFirst).mag2() / 2.;
      wdist2 += w * (posNew - posFirst).mag2() / 2.;

      if (!aggregate && !reposition) {
        auto scatNew = ScatteringAngles(deltaPhi, deltaTheta,
                                        std::sqrt(sigmaDeltaPhi2_tot),
                                        std::sqrt(sigmaDeltaTheta2_tot));
        auto energyLossNew = std::make_unique<Trk::EnergyLoss>(
            deltaE_tot, sigmaDeltaE_tot, sigmaPlusDeltaE_tot,
            sigmaMinusDeltaE_tot, deltaE_ioni_tot, sigmaDeltaE_ioni_tot,
            deltaE_rad_tot, sigmaDeltaE_rad_tot, depth);
        Eloss_tot += energyLossNew->deltaE();
        const Trk::Surface& surf = *(meot->associatedSurface().clone());
        auto meotLast = std::make_unique<Trk::MaterialEffectsOnTrack>(
            X0_tot, scatNew, std::move(energyLossNew), surf, meotPattern);
        auto pars = m->trackParameters()->uniqueClone();

        // make new TSOS
        const Trk::TrackStateOnSurface* newTSOS = new Trk::TrackStateOnSurface(
            nullptr, std::move(pars), std::move(meotLast), typePattern);
        newTSOSvector.push_back(newTSOS);

        X0_tot = 0.;
        sigmaDeltaTheta2_tot = 0.;
        sigmaDeltaPhi2_tot = 0.;
        deltaE_tot = 0.;
        sigmaDeltaE_tot = 0;
        sigmaPlusDeltaE_tot = 0.;
        sigmaMinusDeltaE_tot = 0.;
        deltaE_ioni_tot = 0.;
        sigmaDeltaE_ioni_tot = 0.;
        deltaE_rad_tot = 0.;
        sigmaDeltaE_rad_tot = 0.;

      } else if (!aggregate && reposition) {
        if (std::abs(depth) < 10.) {
          auto scatNew = ScatteringAngles(deltaPhi, deltaTheta,
                                          std::sqrt(sigmaDeltaPhi2_tot),
                                          std::sqrt(sigmaDeltaTheta2_tot));
          auto energyLossNew = std::make_unique<Trk::EnergyLoss>(
              deltaE_tot, sigmaDeltaE_tot, sigmaPlusDeltaE_tot,
              sigmaMinusDeltaE_tot, deltaE_ioni_tot, sigmaDeltaE_ioni_tot,
              deltaE_rad_tot, sigmaDeltaE_rad_tot, depth);
          const Trk::Surface& surf = *(meot->associatedSurface().clone());
          Eloss_tot += energyLossNew->deltaE();
          auto meotLast = std::make_unique<Trk::MaterialEffectsOnTrack>(
              X0_tot, scatNew, std::move(energyLossNew), surf, meotPattern);
          std::unique_ptr<Trk::TrackParameters> pars =
              m->trackParameters()->uniqueClone();
          //        make new TSOS
          const Trk::TrackStateOnSurface* newTSOS =
              new Trk::TrackStateOnSurface(nullptr, std::move(pars),
                                           std::move(meotLast), typePattern);
          newTSOSvector.push_back(newTSOS);
          X0_tot = 0.;
          sigmaDeltaTheta2_tot = 0.;
          sigmaDeltaPhi2_tot = 0.;
          deltaE_tot = 0.;
          sigmaDeltaE_tot = 0;
          sigmaPlusDeltaE_tot = 0.;
          sigmaMinusDeltaE_tot = 0.;
          deltaE_ioni_tot = 0.;
          sigmaDeltaE_ioni_tot = 0.;
          deltaE_rad_tot = 0.;
          sigmaDeltaE_rad_tot = 0.;

        } else {
          //
          //        Thick scatterer: make two TSOSs
          //
          //        prepare for first MaterialEffectsOnTrack with X0 = X0/2
          //        Eloss = 0 and scattering2 = total2 / 2. depth = 0
          auto energyLoss0 = std::make_unique<Trk::EnergyLoss>(0., 0., 0., 0.);
          auto scatFirst = ScatteringAngles(deltaPhi, deltaTheta,
                                            sqrt(sigmaDeltaPhi2_tot / 2.),
                                            sqrt(sigmaDeltaTheta2_tot / 2.));

          //        prepare for second MaterialEffectsOnTrack with X0 =  X0/2
          //        Eloss = Eloss total and scattering2 = total2 / 2. depth = 0
          auto scatNew = ScatteringAngles(deltaPhi, deltaTheta,
                                          sqrt(sigmaDeltaPhi2_tot / 2.),
                                          sqrt(sigmaDeltaTheta2_tot / 2.));
          auto energyLossNew = std::make_unique<Trk::EnergyLoss>(
              deltaE_tot, sigmaDeltaE_tot, sigmaPlusDeltaE_tot,
              sigmaMinusDeltaE_tot, deltaE_ioni_tot, sigmaDeltaE_ioni_tot,
              deltaE_rad_tot, sigmaDeltaE_rad_tot, 0.);
          double norm = dir.perp();
          //        Rotation matrix representation
          Amg::Vector3D colx(-dir.y() / norm, dir.x() / norm, 0);
          Amg::Vector3D coly(-dir.x() * dir.z() / norm,
                             -dir.y() * dir.z() / norm, norm);
          Amg::Vector3D colz(dir.x(), dir.y(), dir.z());

          Amg::Transform3D surfaceTransformFirst(colx, coly, colz, pos0);
          Amg::Transform3D surfaceTransformLast(colx, coly, colz, posNew);
          Trk::PlaneSurface* surfFirst =
              new Trk::PlaneSurface(surfaceTransformFirst);
          Trk::PlaneSurface* surfLast =
              new Trk::PlaneSurface(surfaceTransformLast);
          Eloss_tot += energyLossNew->deltaE();
          //        make MaterialEffectsOnTracks
          auto meotFirst = std::make_unique<Trk::MaterialEffectsOnTrack>(
              X0_tot / 2., scatFirst, std::move(energyLoss0), *surfFirst,
              meotPattern);
          auto meotLast = std::make_unique<Trk::MaterialEffectsOnTrack>(
              X0_tot / 2., scatNew, std::move(energyLossNew), *surfLast,
              meotPattern);

          //        calculate TrackParameters at first surface
          double qOverP0 = m->trackParameters()->charge() /
                           (m->trackParameters()->momentum().mag() -
                            std::fabs(energyLoss->deltaE()));
          if (mprevious)
            qOverP0 = mprevious->trackParameters()->charge() /
                      mprevious->trackParameters()->momentum().mag();
          std::unique_ptr<Trk::TrackParameters> parsFirst =
              surfFirst->createUniqueParameters<5, Trk::Charged>(
                  0., 0., dir.phi(), dir.theta(), qOverP0);
          //        calculate TrackParameters at second surface
          double qOverPNew = m->trackParameters()->charge() /
                             m->trackParameters()->momentum().mag();
          std::unique_ptr<Trk::TrackParameters> parsLast =
              surfLast->createUniqueParameters<5, Trk::Charged>(
                  0., 0., dir.phi(), dir.theta(), qOverPNew);
          // make TSOS
          //
          const Trk::TrackStateOnSurface* newTSOSFirst =
              new Trk::TrackStateOnSurface(nullptr, std::move(parsFirst),
                                           std::move(meotFirst), typePattern);
          const Trk::TrackStateOnSurface* newTSOS =
              new Trk::TrackStateOnSurface(nullptr, std::move(parsLast),
                                           std::move(meotLast), typePattern);

          newTSOSvector.push_back(newTSOSFirst);
          newTSOSvector.push_back(newTSOS);

          X0_tot = 0.;
          sigmaDeltaTheta2_tot = 0.;
          sigmaDeltaPhi2_tot = 0.;
          deltaE_tot = 0.;
          sigmaDeltaE_tot = 0;
          sigmaPlusDeltaE_tot = 0.;
          sigmaMinusDeltaE_tot = 0.;
          deltaE_ioni_tot = 0.;
          sigmaDeltaE_ioni_tot = 0.;
          deltaE_rad_tot = 0.;
          sigmaDeltaE_rad_tot = 0.;
        }
      }

      mprevious = m;
    }
  }
  if (aggregate && reposition) {
    if (n_tot > 0) {
      //
      //        Make three scattering planes in Calorimeter else make two
      //
      Amg::Vector3D pos = wpos / w_tot;
      bool threePlanes = false;
      if (X0_tot > 50 && std::fabs(pos.z()) < 6700 && pos.perp() < 4200)
        threePlanes = true;
      //
      auto energyLoss0 = std::make_unique<Trk::EnergyLoss>(0., 0., 0., 0.);
      auto scatFirst =
          ScatteringAngles(deltaPhi, deltaTheta, sqrt(sigmaDeltaPhi2_tot / 2.),
                           sqrt(sigmaDeltaTheta2_tot / 2.));

      auto scatNew =
          ScatteringAngles(deltaPhi, deltaTheta, sqrt(sigmaDeltaPhi2_tot / 2.),
                           sqrt(sigmaDeltaTheta2_tot / 2.));
      auto energyLoss2 = Trk::EnergyLoss(
          deltaE_tot, sigmaDeltaE_tot, sigmaPlusDeltaE_tot,
          sigmaMinusDeltaE_tot, deltaE_ioni_tot, sigmaDeltaE_ioni_tot,
          deltaE_rad_tot, sigmaDeltaE_rad_tot, 0.);

      int elossFlag = 0;  // return Flag for updateEnergyLoss Calorimeter
                          // energy (0 = not used)
      auto energyLossNew =
          (updateEloss
               ? m_elossupdator->updateEnergyLoss(energyLoss2, caloEnergy,
                                                  caloEnergyError, pCaloEntry,
                                                  momentumError, elossFlag)
               : Trk::EnergyLoss(deltaE_tot, sigmaDeltaE_tot,
                                 sigmaPlusDeltaE_tot, sigmaMinusDeltaE_tot,
                                 deltaE_ioni_tot, sigmaDeltaE_ioni_tot,
                                 deltaE_rad_tot, sigmaDeltaE_rad_tot, 0.));

      //        direction of plane
      Amg::Vector3D dir = wdir / w_tot;
      dir = dir / dir.mag();
      double norm = dir.perp();
      //       Rotation matrix representation
      Amg::Vector3D colx(-dir.y() / norm, dir.x() / norm, 0);
      Amg::Vector3D coly(-dir.x() * dir.z() / norm, -dir.y() * dir.z() / norm,
                         norm);
      Amg::Vector3D colz(dir.x(), dir.y(), dir.z());
      //        Centre position of the two planes
      double halflength2 =
          wdist2 / w_tot - (pos - posFirst).mag() * (pos - posFirst).mag();
      double halflength = 0.;
      if (halflength2 > 0) halflength = sqrt(halflength2);
      Amg::Vector3D pos0 = pos - halflength * dir;
      Amg::Vector3D posNew = pos + halflength * dir;
      if (updateEloss) ATH_MSG_DEBUG("WITH updateEloss");

      ATH_MSG_DEBUG(" WITH aggregation and WITH reposition center planes x "
                    << pos.x() << " y " << pos.y() << " z " << pos.z()
                    << " halflength " << halflength << " w_tot " << w_tot
                    << " X0_tot " << X0_tot);

      Amg::Transform3D surfaceTransformFirst(colx, coly, colz, pos0);
      Amg::Transform3D surfaceTransformLast(colx, coly, colz, posNew);
      Trk::PlaneSurface* surfFirst =
          new Trk::PlaneSurface(surfaceTransformFirst);
      Trk::PlaneSurface* surfLast = new Trk::PlaneSurface(surfaceTransformLast);
      //        calculate TrackParameters at first surface
      double qOverP0 = mfirst->trackParameters()->charge() /
                       (mfirst->trackParameters()->momentum().mag() +
                        std::fabs(deltaEFirst));
      //        calculate TrackParameters at last surface
      double qOverPNew = mlast->trackParameters()->charge() /
                         mlast->trackParameters()->momentum().mag();
      std::unique_ptr<Trk::TrackParameters> parsFirst =
          surfFirst->createUniqueParameters<5, Trk::Charged>(
              0., 0., dir.phi(), dir.theta(), qOverP0);
      std::unique_ptr<Trk::TrackParameters> parsLast =
          surfLast->createUniqueParameters<5, Trk::Charged>(
              0., 0., dir.phi(), dir.theta(), qOverPNew);

      Eloss_tot += energyLossNew.deltaE();
      if (!threePlanes) {
        //
        // make two scattering planes and TSOS
        //
        //          prepare for first MaterialEffectsOnTrack with X0 = X0/2
        //          Eloss = 0 and scattering2 = total2 / 2. depth = 0
        auto meotFirst = std::make_unique<Trk::MaterialEffectsOnTrack>(
            X0_tot / 2., scatFirst, std::move(energyLoss0), *surfFirst,
            meotPattern);
        //          prepare for second MaterialEffectsOnTrack with X0 = X0/2
        //          Eloss = Eloss total and scattering2 = total2 / 2. depth
        //          = 0
        auto meotLast = std::make_unique<Trk::MaterialEffectsOnTrack>(
            X0_tot / 2., scatNew,
            std::make_unique<Trk::EnergyLoss>(std::move(energyLossNew)),
            *surfLast, meotPattern);

        const Trk::TrackStateOnSurface* newTSOSFirst =
            new Trk::TrackStateOnSurface(nullptr, std::move(parsFirst),
                                         std::move(meotFirst), typePattern);
        auto whichType = (elossFlag != 0) ? typePatternDeposit : typePattern;
        const Trk::TrackStateOnSurface* newTSOS = new Trk::TrackStateOnSurface(
                                  nullptr, std::move(parsLast),
                                  std::move(meotLast), whichType);
       
        newTSOSvector.push_back(newTSOSFirst);
        newTSOSvector.push_back(newTSOS);
      } else {
        //
        // make three scattering planes and TSOS in Calorimeter
        //
        auto scatZero = ScatteringAngles(0., 0., 0., 0.);
        Amg::Transform3D surfaceTransform(colx, coly, colz, pos);
        Trk::PlaneSurface* surf = new Trk::PlaneSurface(surfaceTransform);
        std::unique_ptr<Trk::TrackParameters> pars =
            surf->createUniqueParameters<5, Trk::Charged>(
                0., 0., dir.phi(), dir.theta(), qOverPNew);
        //        prepare for first MaterialEffectsOnTrack with X0 = X0/2
        //        Eloss = 0 and scattering2 = total2 / 2. depth = 0
        auto meotFirst = std::make_unique<Trk::MaterialEffectsOnTrack>(
            X0_tot / 2., scatFirst,
            std::make_unique<Trk::EnergyLoss>(0., 0., 0., 0.), *surfFirst,
            meotPattern);
        //        prepare for middle MaterialEffectsOnTrack with X0 =  0
        //        Eloss = ElossNew and scattering2 = 0. depth = 0
        auto meot = std::make_unique<Trk::MaterialEffectsOnTrack>(
            0., scatZero,
            std::make_unique<Trk::EnergyLoss>(std::move(energyLossNew)), *surf,
            meotPattern);
        //        prepare for last MaterialEffectsOnTrack with X0 =  X0/2
        //        Eloss = 0 total and scattering2 = total2 / 2. depth = 0
        auto meotLast = std::make_unique<Trk::MaterialEffectsOnTrack>(
            X0_tot / 2., scatNew,
            std::make_unique<Trk::EnergyLoss>(0., 0., 0., 0.), *surfLast,
            meotPattern);
        const Trk::TrackStateOnSurface* newTSOSFirst =
            new Trk::TrackStateOnSurface(nullptr, std::move(parsFirst),
                                         std::move(meotFirst), typePattern);
        const Trk::TrackStateOnSurface* newTSOS = new Trk::TrackStateOnSurface(
            nullptr, std::move(pars), std::move(meot), typePatternDeposit);
        const Trk::TrackStateOnSurface* newTSOSLast =
            new Trk::TrackStateOnSurface(nullptr, std::move(parsLast),
                                         std::move(meotLast), typePattern);
        newTSOSvector.push_back(newTSOSFirst);
        newTSOSvector.push_back(newTSOS);
        newTSOSvector.push_back(newTSOSLast);
      }
    }
  }

  return newTSOSvector;
}
void Trk::GeantFollowerMSHelper::endEvent() {
  // fill the validation tree
  m_validationTree->Fill();
  delete m_parameterCache;
  delete m_parameterCacheCov;

  if (m_crossedEntry) {
    if (m_parameterCacheEntry) delete m_parameterCacheEntry;
    if (m_parameterCacheEntryCov) delete m_parameterCacheEntryCov;
  }
}
