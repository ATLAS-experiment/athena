/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "ALFA_SimEv/ALFA_ODHit.h"


  // Called by ALFA_SensitiveDetector::ProcessHits

ALFA_ODHit::ALFA_ODHit(int hit, 
                        int /*track*/, 
			int particle,
			float kinEnergy, 
			float eneDeposit, 
			float preX, float preY, float preZ, 
			float postX, float postY, float postZ, 
			float time,
			int sign, int side, int plate, int fiber, int station
			):
  m_hitID(hit),
  m_particleEncoding(particle),
  m_kineticEnergy(kinEnergy), 
  m_energyDeposit(eneDeposit), 
  m_preStepX(preX), m_preStepY(preY), m_preStepZ(preZ), 
  m_postStepX(postX), m_postStepY(postY), m_postStepZ(postZ), 
  m_globalTime(time),
  m_sign_fiber(sign), m_OD_side(side),m_n_plate(plate), m_n_fiber(fiber), m_n_station(station) 
  {} 


// barcode
int ALFA_ODHit::GetTrackID() const 
{
  return 22;  
}
