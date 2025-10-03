/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// base class for Calorimeter Cells
#include "CaloEvent/CaloCell.h"
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "AthenaKernel/BaseInfo.h"
#include <cmath>

CaloCell::~CaloCell()
{
// destructor
}


CaloCell::CaloCell(const CaloDetDescrElement* caloDDE,
			  float   energy,
			  float   time,
			  double   quality,
			  CaloGain::CaloGain gain)
    :
    m_energy(energy),
    m_time(time),
    m_quality((int)(quality)),
    m_ID(caloDDE ? caloDDE->identify() : Identifier()),
    m_gain(gain),
    m_caloDDE(caloDDE)
{}

// cppcheck-suppress uninitMemberVar  ; m_quality
CaloCell::CaloCell(const CaloDetDescrElement* caloDDE,
			  float      energy,
			  float      time,
			  uint16_t   quality,
        uint16_t   provenance,
			  CaloGain::CaloGain gain)
    :
    m_energy(energy),
    m_time(time),
    m_ID(caloDDE ? caloDDE->identify() : Identifier()),
    m_gain(gain),
    m_caloDDE(caloDDE)
{
  m_qualProv[0]=quality;
  // cppcheck-suppress objectIndex
  m_qualProv[1]=provenance;
}


CaloCell::CaloCell(const CaloDetDescrElement* caloDDE,
                          const Identifier & ID,
			  float   energy,
			  float   time,
			  double   quality,
			  CaloGain::CaloGain gain)
    :
    m_energy(energy),
    m_time(time),
    m_quality((int)(quality)),
    m_ID(ID),
    m_gain(gain),
    m_caloDDE(caloDDE)
{}

// cppcheck-suppress uninitMemberVar  ; m_quality
CaloCell::CaloCell(const CaloDetDescrElement* caloDDE,
                          const Identifier & ID,
			  float   energy,
			  float   time,
			  uint16_t   quality,
		          uint16_t   provenance,
			  CaloGain::CaloGain gain)
    :
    m_energy(energy),
    m_time(time),
    m_ID(ID),
    m_gain(gain),
    m_caloDDE(caloDDE)
{
  m_qualProv[0]=quality;
  // cppcheck-suppress objectIndex
  m_qualProv[1]=provenance;
}


void CaloCell::set (float energy,
                    float time,
                    uint16_t   quality,
                    uint16_t   provenance,
                    CaloGain::CaloGain gain)
{
  m_energy = energy;
  m_time = time;
  m_gain = gain;
  m_qualProv[0]=quality;
  // cppcheck-suppress objectIndex
  m_qualProv[1]=provenance;
}


void CaloCell::set (float energy,
                    float time,
                    double quality,
                    CaloGain::CaloGain gain)
{
  m_energy = energy;
  m_time = time;
  m_quality = (int)(quality);
  m_gain = gain;
}

void CaloCell::set4Mom (const I4Momentum * const  )
{
  std::cout << " FATAL ERROR : CaloCell::set4Mom called. Cannot change 4mom " << std::endl ;
  std::abort();
}

void CaloCell::set4Mom (const I4Momentum & )
{
  std::cout << " FATAL ERROR : CaloCell::set4Mom called. Cannot change 4mom " << std::endl ;
  std::abort();
}

void CaloCell::set4Mom (const CLHEP::HepLorentzVector & )
{
  std::cout << " FATAL ERROR : CaloCell::set4Mom called. Cannot change 4mom " << std::endl ;
  std::abort();
}


std::unique_ptr<CaloCell> CaloCell::clone() const
{
  return std::make_unique<CaloCell>(this->caloDDE(),
				    this->energy(),
				    this->time(),
				    this->quality(),
				    this->provenance(),
				    this->gain() );
}

bool CaloCell::badcell() const
{
  // actual correct implementation is in LArCell and TileCell
 return false;
}


SG_ADD_BASE (CaloCell, INavigable4Momentum);
