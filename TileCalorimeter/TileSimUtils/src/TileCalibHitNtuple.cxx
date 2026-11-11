/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TileSimUtils/TileCalibHitNtuple.h"

// Atlas includes
#include "GaudiKernel/MsgStream.h"
#include "Gaudi/Property.h"
#include "AthenaKernel/errorcheck.h"

// ROOT includes
#include "TTree.h"

static const InterfaceID IID_ITileCalibHitNtuple("TileCalibHitNtuple", 1, 0);

const InterfaceID&
TileCalibHitNtuple::interfaceID() {
  return IID_ITileCalibHitNtuple;
}

/**
 * Constructor
 */
TileCalibHitNtuple::TileCalibHitNtuple(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type, name, parent)
{
  declareInterface<TileCalibHitNtuple>(this);
}

/**
 * Destructor
 */
TileCalibHitNtuple::~TileCalibHitNtuple() {
}

/**
 * Initializer
 */
StatusCode TileCalibHitNtuple::initialize() {

  ATH_MSG_INFO("in initialize()");

  CHECK(m_thistSvc.retrieve());

  m_ntuplePtr = new TTree(m_ntupleID.value().c_str(), "TileCalibHit-Ntuple");
  m_ntuplePtr->SetMaxTreeSize(m_treeSize);

  m_ntuplePtr->Branch("detector", &m_det, "detector/S");
  m_ntuplePtr->Branch("module", &m_mod, "module/S");
  m_ntuplePtr->Branch("tower", &m_tow, "tower/S");
  m_ntuplePtr->Branch("sample", &m_samp, "sample/S");
  m_ntuplePtr->Branch("region", &m_reg, "region/S");
  m_ntuplePtr->Branch("IDcalc", &m_calc, "IDcalc/S");
  m_ntuplePtr->Branch("event", &m_evt, "event/S");
  m_ntuplePtr->Branch("xPos", &m_xPos, "xPos/F");
  m_ntuplePtr->Branch("yPos", &m_yPos, "yPos/F");
  m_ntuplePtr->Branch("zPos", &m_zPos, "zPos/F");
  m_ntuplePtr->Branch("energy", &m_ener, "energy/F");
  if (m_level != 0) {
    m_ntuplePtr->Branch("volume", &m_volume, "volume/C");
    m_ntuplePtr->Branch("pid", &m_pid, "pid/S");
    m_ntuplePtr->Branch("ekine", &m_ekine, "ekine/F");
    m_ntuplePtr->Branch("xLocal", &m_xLocal, "xLocal/F");
    m_ntuplePtr->Branch("yLocal", &m_yLocal, "yLocal/F");
    m_ntuplePtr->Branch("zLocal", &m_zLocal, "zLocal/F");
    m_ntuplePtr->Branch("phiLocal", &m_phiLocal, "phiLocal/F");
  }

  StatusCode sc = m_thistSvc->regTree("/" + m_streamName.value() + "/" + m_ntupleID.value(), m_ntuplePtr);

  if (sc.isFailure()) {
    ATH_MSG_ERROR("Problem registering TileCalibHit Tree");
  }

  return sc;
}

/**
 * Finalizer
 */
StatusCode TileCalibHitNtuple::finalize() {

  ATH_MSG_INFO("Finalizing");

  return StatusCode::SUCCESS;
}

void TileCalibHitNtuple::storeHit(int det, int mod, int tow, int samp, int reg, int calc, int evt, float x, float y,
                                  float z, float e, const char *volume, int pid, float ekine, float xLocal,
                                  float yLocal, float zLocal, float phiLocal) {
  m_det = det;
  m_mod = mod;
  m_tow = tow;
  m_samp = samp;
  m_reg = reg;
  m_calc = calc;
  m_evt = evt;
  m_xPos = x;
  m_yPos = y;
  m_zPos = z;
  m_ener = e;
  if (m_level != 0) {
    if (volume) strncpy(m_volume, volume, 99);
    else m_volume[0] = 0;
    m_pid = pid;
    m_ekine = ekine;
    m_xLocal = xLocal;
    m_yLocal = yLocal;
    m_zLocal = zLocal;
    m_phiLocal = phiLocal;
  }

  m_ntuplePtr->Fill();
}
