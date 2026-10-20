/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ELASTICDECAYUTIL_H
#define ELASTICDECAYUTIL_H 1

#include "xAODTruth/TruthParticle.h"

// for debugging
#include "CxxUtils/checker_macros.h"
#include <type_traits>
#include <initializer_list>
#include <mutex>
#include "ActsInterop/StatUtils.h"

// Empty struct emulating enough of the interface to be used as a dummy replacement for a Gaudi property
struct EmptyProperty {
   template <typename... T_Args>
   EmptyProperty(T_Args... ) {}
   // to match empty or some non-empty initializer list
   template <typename T_Arg1, typename T_Arg2, typename... T_Args>
   EmptyProperty(T_Arg1, T_Arg2, std::initializer_list<int> &&, T_Args... ) {}
   template <typename T_Arg1, typename T_Arg2, typename... T_Args>
   EmptyProperty(T_Arg1, T_Arg2, std::initializer_list<double> &&, T_Args... ) {}
   const EmptyProperty &value() { return *this; }
};

template <bool IsDebug=false>
struct ElasticDecayUtil {
   /// @brief follow decay chain upwards assuming quasi elastic processes.
   /// @param truth_particle the truth particle to follow which must not be nullptr.
   /// @param max_energy_loss if the energy loss is larger than this stop following the particle.
   /// @return a pointer to the top most particle of the quasi elastic decay chain
   /// Follow the decay chain upwards if the process is quasi elastic i.e.
   /// one parent of same PDG id, at most one sibling.
   /// If IsDebug is true some histograms are filled and some statistics is gathered.
   const xAOD::TruthParticle *getMother(const xAOD::TruthParticle &truth_particle, float max_energy_loss) const;

   // only used if IsDebug=true
   template <class T_OutStream>
   void dumpStatistics(T_OutStream &out) const;

   // @param binning number of bins, lower edge, upper edge in MeV
   void setEnergyLossBinning(const typename std::conditional<IsDebug, std::vector<float>, EmptyProperty>::type &binning);

   struct Empty {
      template <typename... T_Args>
      Empty(T_Args... ) {}
   };
   mutable typename std::conditional_t<IsDebug,std::mutex, Empty> m_mutex ATLAS_THREAD_SAFE;

   // PDG IDs of siblings.
   mutable typename std::conditional_t<IsDebug,std::set<int>, Empty> m_outgoingPdgIds ATLAS_THREAD_SAFE;

   /// PDG IDs of parents.
   mutable typename std::conditional_t<IsDebug,std::set<int>, Empty> m_outgoingMotherPdgIds ATLAS_THREAD_SAFE;
   enum ParticleTypes {
      kNoSibling,
      kElectron,
      kPhoton,
      kOther,
      kNParticleTypes
   };
   /// energy less per vertex.
   mutable typename std::conditional_t<IsDebug,std::array<ActsUtils::StatHist,kNParticleTypes>, Empty> m_energyLossStat ATLAS_THREAD_SAFE;

   // number of parents in a decay chain.
   mutable typename std::conditional_t<IsDebug,ActsUtils::StatHist,Empty> m_nParentsStat ATLAS_THREAD_SAFE {20,-.5,20.-.5};
};

#include "ElasticDecayUtil.icc"
#endif
