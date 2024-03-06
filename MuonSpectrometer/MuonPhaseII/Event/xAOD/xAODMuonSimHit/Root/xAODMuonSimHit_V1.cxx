/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include <xAODCore/AuxStoreAccessorMacros.h>

#include <xAODMuonSimHit/versions/MuonSimHit_v1.h>

namespace {
   static const std::string preFixStr{"MuSim_"};
   static const SG::AuxElement::Accessor<Identifier::value_type> acc_Identifier{preFixStr+"identifier"};
   static const xAOD::PosAccessor<3> acc_localPos{"localPositionDim3"};
   static const xAOD::PosAccessor<3> acc_localDir{"localDirectionDim3"};

   static const SG::AuxElement::Accessor<unsigned short> acc_mcEventIndex{preFixStr+"mcEventIndex"};
   static const SG::AuxElement::Accessor<unsigned int> acc_mcBarcode{preFixStr+"mcBarcode"};
}

#define IMPLEMENT_SETTER_GETTER( DTYPE, GETTER, SETTER)                          \
      DTYPE MuonSimHit_v1::GETTER() const {                                  \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         return acc(*this);                                                      \
      }                                                                          \
                                                                                 \
      void MuonSimHit_v1::SETTER(DTYPE value) {                        \
         static const SG::AuxElement::Accessor<DTYPE> acc{preFixStr + #GETTER};  \
         acc(*this) = value;                                                     \
      }
          
namespace xAOD {

MuonSimHit_v1& MuonSimHit_v1::operator=(const MuonSimHit_v1& other) {
   if (this != &other) {
      static_cast<SG::AuxElement&>(*this) = other;
#ifndef __CLING__
      m_hepMCLink.release();
#endif
   }
   return (*this);
}
Identifier MuonSimHit_v1::identify() const { return Identifier{acc_Identifier(*this)}; }    
void MuonSimHit_v1::setIdentifier(const Identifier& id) { acc_Identifier(*this) = id.get_compact(); }

IMPLEMENT_SETTER_GETTER(float, globalTime, setGlobalTime)
IMPLEMENT_SETTER_GETTER(int, pdgId, setPdgId)
IMPLEMENT_SETTER_GETTER(float, energyDeposit, setEnergyDeposit)
IMPLEMENT_SETTER_GETTER(float, kineticEnergy, setKineticEnergy)
IMPLEMENT_SETTER_GETTER(float, stepLength, setStepLength)

void MuonSimHit_v1::setLocalPosition(MeasVector<3> vec) {
   VectorMap<3> lPos{acc_localPos(*this).data()};
   lPos = vec;   
}
ConstVectorMap<3> MuonSimHit_v1::localPosition() const { return ConstVectorMap<3>{acc_localPos(*this).data()};}

void MuonSimHit_v1::setLocalDirection(MeasVector<3> vec) {
   VectorMap<3> lPos{acc_localDir(*this).data()};
   lPos = vec;   
}
ConstVectorMap<3> MuonSimHit_v1::localDirection() const { return ConstVectorMap<3>{acc_localDir(*this).data()};}

const HepMcParticleLink& MuonSimHit_v1::genParticleLink() const {
   if (!m_hepMCLink) {
      const unsigned short eventIndex = acc_mcEventIndex(*this);
      const HepMcParticleLink::PositionFlag flag =  eventIndex > 0 ? HepMcParticleLink::IS_EVENTNUM :
                                                                     HepMcParticleLink::IS_POSITION;
      std::unique_ptr<HepMcParticleLink> link = std::make_unique<HepMcParticleLink>();

      HepMcParticleLink::ExtendedBarCode barcode {acc_mcBarcode(*this),
                                                  eventIndex,
                                                  flag};
      link->setExtendedBarCode(std::move(barcode));
      return *m_hepMCLink.set(std::move(link));
   }
   return (*m_hepMCLink);
}
void MuonSimHit_v1::setGenParticleLink(const HepMcParticleLink& link) {
   m_hepMCLink.release();
   acc_mcEventIndex(*this) = link.eventIndex();
   acc_mcBarcode(*this) = link.barcode();
}

}
#undef IMPLEMENT_SETTER_GETTER
