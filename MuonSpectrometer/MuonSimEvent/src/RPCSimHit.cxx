/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "MuonSimEvent/RPCSimHit.h"


#include <sstream>

// construction/destruction

RPCSimHit::RPCSimHit( ) :
  m_RPCid(0xffff),
  m_globalTime(0.),
  m_localPosition(0.,0.,0.),         // pre-step position
  m_partLink(),
  m_localPostStepPosition(0.,0.,0.), // post-step position
  m_energyDeposit(-1.),              //Geant4 de/dx
  m_stepLength(-1.),                 //Geant4 step Length
  m_particleEncoding(0),             // PDG id
  m_kineticEnergy(-1.)               // kinetic energy of the particle
{
}
//default destructor required by Pool
RPCSimHit::~RPCSimHit( )  = default;

RPCSimHit::RPCSimHit(int id,
		     double time,
	             const Amg::Vector3D& position)
    : m_RPCid(id),
      m_globalTime(time),
      m_localPosition(position),
      m_partLink(),
      m_localPostStepPosition(0.,0.,0.), // post-step position
      m_energyDeposit(-1.),              //Geant4 de/dx
      m_stepLength(-1.),                 //Geant4 step Length
      m_particleEncoding(0),             // PDG id
      m_kineticEnergy(-1.)               // kinetic energy of the particle
{
}

RPCSimHit::RPCSimHit(int id,
		     double time,
	             const Amg::Vector3D& position,
		     const int truthBarcode)
    : m_RPCid(id),
      m_globalTime(time),
      m_localPosition(position),
      m_partLink(truthBarcode, 0, HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE), // FIXME barcode-based
      m_localPostStepPosition(0.,0.,0.), // post-step position
      m_energyDeposit(-1.),              //Geant4 de/dx
      m_stepLength(-1.),                 //Geant4 step Length
      m_particleEncoding(0),             // PDG id
      m_kineticEnergy(-1.)               // kinetic energy of the particle
{
}

RPCSimHit::RPCSimHit(int id,
		     double time,
	             const Amg::Vector3D& prePosition,
		     const int truthBarcode,
	             const Amg::Vector3D& postPosition,
		     const double energyDeposit, const double stepLength,
		     const int particleEncoding, const double kineticEnergy)
    : m_RPCid(id), m_globalTime(time)
    , m_localPosition(prePosition)
    , m_partLink(truthBarcode, 0, HepMcParticleLink::IS_EVENTNUM, HepMcParticleLink::IS_BARCODE) // FIXME barcode-based
    , m_localPostStepPosition(postPosition)
    , m_energyDeposit(static_cast<float>(energyDeposit))
    , m_stepLength(static_cast<float>(stepLength))
    , m_particleEncoding(particleEncoding)
    , m_kineticEnergy(static_cast<float>(kineticEnergy))
{
}

RPCSimHit::RPCSimHit(int id,
		     double time,
	             const Amg::Vector3D& prePosition,
		     const HepMcParticleLink& hmpl,
	             const Amg::Vector3D& postPosition,
		     const double energyDeposit, const double stepLength,
		     const int particleEncoding, const double kineticEnergy)
    : m_RPCid(id), m_globalTime(time)
    , m_localPosition(prePosition)
    , m_partLink(hmpl)
    , m_localPostStepPosition(postPosition)
    , m_energyDeposit(static_cast<float>(energyDeposit))
    , m_stepLength(static_cast<float>(stepLength))
    , m_particleEncoding(particleEncoding)
    , m_kineticEnergy(static_cast<float>(kineticEnergy))
{
}

// Implemenation of member functions
std::string RPCSimHit::print() const {

    std::stringstream ss;

    ss << "RPCSimHit:";
    ss << "  identifier: "  << m_RPCid;
    ss << "  t:  " << m_globalTime;
    ss << "  startPoint: (" << m_localPosition.x()
       << ","      << m_localPosition.y()
       << ","      << m_localPosition.z()
       << ")";
    ss << "  endPoint: ("<< m_localPostStepPosition.x()
       << ","      << m_localPostStepPosition.y()
       << ","      << m_localPostStepPosition.z()
       << ")";
    ss << " EnergyDeposit: "<<m_energyDeposit<<" StepLength: "<<m_stepLength
       <<" pdgId: "<<m_particleEncoding<<" kineticEnergy: "<<m_kineticEnergy;

    return ss.str();
}

int RPCSimHit::truthBarcode() const
 { return m_partLink.barcode(); }

int RPCSimHit::truthID() const
{ return m_partLink.id(); }
