#ifndef ACTSTRK_CLUSTERTOTRUTHASSOCIATION_H
#define ACTSTRK_CLUSTERTOTRUTHASSOCIATION_H 1

#include "InDetSimData/InDetSimDataCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"

#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include <unordered_map>

#include "MeasurementToTruthAssociationAlg.h"

// Adapter to use MeasurementToTruthAssociation algorithm to
// xAOD::PixelClusterContainer and xAOD::StripClusterContainer to
// xAOD::TruthParticles using the InDetSimDataCollection and
// the xAODTruthParticleLink which pairs HepMcParticleLinks with the
// corresponding xAOD::TruthParticle.

#include <iostream>

class HepMcLinkToTruthParticleMap
{
private:
   const xAODTruthParticleLinkVector *m_truthParticleLinks;
public:
   HepMcLinkToTruthParticleMap(const xAODTruthParticleLinkVector &truth_particle_links)
      : m_truthParticleLinks(&truth_particle_links)
   {}
   operator bool() const {
      return true;
   }
   inline const xAOD::TruthParticle *getTruthParticle(const InDetSimData::Deposit &deposit) {
      if (deposit.first.isValid()) {
         ElementLink<xAOD::TruthParticleContainer> truth_particle_link = m_truthParticleLinks->find(deposit.first);
         if (truth_particle_link) {
            return *truth_particle_link;
         }
      }
      return nullptr;
   }
   inline bool isHardScatter(const InDetSimData::Deposit &deposit) {
      return deposit.first.eventIndex()==0;
   }
};

// specialisation for the MeasurementToTruthAssociationAlg
template <>
inline auto ActsTrk::makeDepositToTruthParticleMap(const xAODTruthParticleLinkVector *truth_particle_links) {
   if (!truth_particle_links) {
      throw std::runtime_error("Invalid xAODTruthParticleLinkVector.");
   }

   return HepMcLinkToTruthParticleMap(*truth_particle_links);
}

// property name for the input xAODTruthParticleLinkVector
template <>
inline const char *ActsTrk::getInTruthPropertyName<xAODTruthParticleLinkVector>() {
   return "InputTruthParticleLinks";
}

// specialisation for the MeasurementToTruthAssociationAlg
template <>
inline auto ActsTrk::getSimDataDeposits([[maybe_unused]] const InDetSimDataCollection &sim_data_collection,
                                        InDetSimDataCollection::const_iterator sim_data_iter_for_identifier) {
   return sim_data_iter_for_identifier->second.getdeposits();
}

template <>
inline float ActsTrk::getDepositedEnergy(const std::pair<HepMcParticleLink, float> &deposit) {
   return deposit.second;
}

constexpr bool MeasurementToTruthAssociationDebugHistograms = false;


namespace ActsTrk {

   // name the specialistion to get a nicer name in python
   class PixelClusterToTruthAssociationAlg
      : public MeasurementToTruthAssociationAlg<xAOD::PixelClusterContainer,
                                                InDetSimDataCollection,
                                                xAODTruthParticleLinkVector,
                                                MeasurementToTruthAssociationDebugHistograms>
   {
   public:
      using MeasurementToTruthAssociationAlg<xAOD::PixelClusterContainer,
                                             InDetSimDataCollection,
                                             xAODTruthParticleLinkVector,
                                             MeasurementToTruthAssociationDebugHistograms>::MeasurementToTruthAssociationAlg;
   };

   // name the specialistion to get a nicer name in python
   class StripClusterToTruthAssociationAlg
      : public MeasurementToTruthAssociationAlg<xAOD::StripClusterContainer,
                                                InDetSimDataCollection,
                                                xAODTruthParticleLinkVector,
                                                MeasurementToTruthAssociationDebugHistograms>
   {
   public:
      using MeasurementToTruthAssociationAlg<xAOD::StripClusterContainer,
                                             InDetSimDataCollection,
                                             xAODTruthParticleLinkVector,
                                             MeasurementToTruthAssociationDebugHistograms>::MeasurementToTruthAssociationAlg;
   };
      // name the specialistion to get a nicer name in python
   class HgtdClusterToTruthAssociationAlg
      : public MeasurementToTruthAssociationAlg<xAOD::HGTDClusterContainer,
                                                InDetSimDataCollection,
                                                xAODTruthParticleLinkVector,
                                                MeasurementToTruthAssociationDebugHistograms>
   {
   public:
      using MeasurementToTruthAssociationAlg<xAOD::HGTDClusterContainer,
                                             InDetSimDataCollection,
                                             xAODTruthParticleLinkVector,
                                             MeasurementToTruthAssociationDebugHistograms>::MeasurementToTruthAssociationAlg;
   };

}
#endif
