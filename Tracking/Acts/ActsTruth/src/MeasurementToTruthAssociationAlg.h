/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_MEASUREMENTTOTRUTHASSOCIATIONALG_H
#define ACTSTRKFINDING_MEASUREMENTTOTRUTHASSOCIATIONALG_H 1

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Gaudi includes
#include "Gaudi/Property.h"

// Handle Keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "Identifier/Identifier.h"
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"

#include <string>
#include <memory>
#include <array>
#include <atomic>
#include <type_traits>

#include <cmath>
#include <iomanip>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>

#include "ActsInterop/StatUtils.h"

namespace Dbg {
using ActsUtils::Stat;
using ActsUtils::StatHist;
using ActsUtils::operator<<;

struct HistTemp {
   mutable Dbg::StatHist m_depositedEnergy         ATLAS_THREAD_SAFE     {20, -6., 14.};
   mutable Dbg::StatHist m_particlesPerMeasurement ATLAS_THREAD_SAFE     {20, -.5, 20.-.5};
   mutable Dbg::StatHist m_measurementsPerParticle ATLAS_THREAD_SAFE     {20, -0.5, 20.-0.5};
   HistTemp() = default;
   HistTemp(const HistTemp &a)
      : m_depositedEnergy(a.m_depositedEnergy.createEmptyClone()),
        m_particlesPerMeasurement( a.m_particlesPerMeasurement.createEmptyClone()),
        m_measurementsPerParticle( a.m_measurementsPerParticle.createEmptyClone())
   {
   }
};
struct Hist : public HistTemp {
   using HistTemp::HistTemp;
   mutable std::mutex m_mutex;
};

struct Empty {
   Empty()              =default;
   Empty(const Empty &) =default;
};

}

namespace ActsTrk
{

  // Helper class to emulate an RDO list for a type which provides a single RDO
  template<class T>
  class DummyRDOList {
  private:
    const T *m_ptr;
  public:
    DummyRDOList(const T &a) : m_ptr(&a) {}
    struct const_iterator {
        const T *m_ptr;
        const_iterator(const T *a_ptr) : m_ptr(a_ptr) {}
        Identifier operator*() const { return m_ptr->identifier(); }
        bool operator!=(const const_iterator &other) const {
            return m_ptr != other.m_ptr;
        }
        const_iterator &operator++() { ++m_ptr; return *this;}
    };
    const_iterator begin() const {
        return const_iterator(m_ptr);
    }
    const_iterator end() const {
        return const_iterator(m_ptr+1);
    }
  };

  // helper templates to detect whether a type has the method rdoList
  template <typename Object>
  using rdoListFunc_t = decltype(std::declval<Object>().rdoList());

  // template being used for types that do not have  a member function called rdoList
  template <typename Object, typename = std::void_t<> >
  struct has_rdoList : std::false_type{};

  // template being used for types that have the member function rdoList
  template <typename Object >
  struct has_rdoList<Object, std::void_t<rdoListFunc_t<Object> > > : std::true_type{};

  // helper template to get a list of RDOs associated to the given measurement
  // if the measurement does not provide an RDO list but only is composed of a single measurement
  // return something which behaves like a RDO list but contains only a single identifier.
  // the optimizer (with O2) should take care of eliminating the bogus loop.
  template <class T>
  auto getRDOList(const T &a) {
     if constexpr(has_rdoList<T>::value) {
        return a.rdoList();
     }
     else {
        return DummyRDOList(a);
     }
  }

  // specialisations need to be implemented for each T_TruthEventCollection used for the MeasurementToTruthAssociationAlg
  // @return helper class to get TruthParticles from a deposit, and test whether a truth particle is from the hard scatter event
  // The helper class has to implement getTruthParticle, isHardScatter which are functions from Deposits, where deposits are
  // elements from the collection returned by getSimDataDeposits, which is a function of the sim data collection
  template <class T_TruthEventCollection>
  auto makeDepositToTruthParticleMap(const T_TruthEventCollection *truth_particle_links);

  // specialisation need to be implemented for each T_TruthEventCollection used for the MeasurementToTruthAssociationAlg
  template <class T_TruthEventCollection>
  inline const char *getInTruthPropertyName();

  // specialisations need to be implemented  for each T_SimDataCollection/T_SimDataIterator used for the MeasurementToTruthAssociationAlg
  template <class T_SimDataCollection, class T_SimDataIterator>
  auto getSimDataDeposits(const T_SimDataCollection &sim_data_collection, T_SimDataIterator sim_data_iter_for_identifier);

  // specialisations need to be implemented for each T_SimDataCollection used for the MeasurementToTruthAssociationAlg
  template <class T_Deposit>
  float getDepositedEnergy(const T_Deposit &);

  /// Algorithm template to associate measurements of a certain type to a xAOD truth particles using a sim data collection
  /// where the sim data collection contains associates RDOs to truth particles and there energy/charge disposition.
  template <class T_MeasurementCollection, class T_SimDataCollection, class T_TruthEventCollection, bool IsDebug=false >
  class MeasurementToTruthAssociationAlg : public AthReentrantAlgorithm
  {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
     SG::ReadHandleKey<T_MeasurementCollection>  m_measurementKey
        {this, "Measurements",{}, "List of input measurement keys." };
     SG::ReadHandleKey<T_SimDataCollection>      m_simDataKey
        {this, "SimData",{}, "List of input simulated data keys." };
     SG::ReadHandleKey<T_TruthEventCollection> m_truthEventCollectionKey
        {this,getInTruthPropertyName<T_TruthEventCollection>(),"","The key for the truth event collection e.g. McEventCollectionWrap if one is required."};

     SG::WriteHandleKey<MeasurementToTruthParticleAssociation>  m_associationOutKey
        {this, "AssociationMapOut","", "Output association map from measurements to generator particles." };

     Gaudi::Property<float> m_depositedEnergyMin
        {this, "DepositedEnergyMin", 0., "Only consider gnerator particles which"
         " deposed more than this amount of energy (in MeV)" };

     enum EStat {
        kNoTruth,
        kHasSimHit,
        kInvalidTruthLink,
        kHasSimHitNoParticle,
        kBeyondSmallVectorSize,
        kNCategories
     };
     // statistics counter
     mutable std::array<std::atomic<std::size_t>,kNCategories>       m_statRDO ATLAS_THREAD_SAFE;
     // counter of deposits from truth particles of the hard scatter and pileup with or without
     // associated xAOD truth particle.
     mutable std::array<std::atomic<std::size_t>,4>                  m_depositCounts ATLAS_THREAD_SAFE {0u, 0u, 0u, 0u};
     // optional histograms of deposits per particle, particles per measurement, and measurements per particle
     typename std::conditional<IsDebug, Dbg::Hist, Dbg::Empty>::type m_stat;
  };

} // namespace

#endif
