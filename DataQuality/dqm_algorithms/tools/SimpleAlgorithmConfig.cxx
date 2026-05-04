/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*! \file SimpleAlgorithmConfig  Defines the class SimpleAlgorithmConfig a concrete simple implementation of dqm_core::AlgorithmConfig
 *  \author andrea.dotti@cern.ch
 */
#include <dqm_algorithms/tools/SimpleAlgorithmConfig.h>

#include "CxxUtils/checker_macros.h"

dqm_algorithms::tools::SimpleAlgorithmConfig::SimpleAlgorithmConfig() :
  dqm_core::AlgorithmConfig() , m_ref(0)
{
  //Empty
}

dqm_algorithms::tools::SimpleAlgorithmConfig::SimpleAlgorithmConfig( TObject * ref ) :
  dqm_core::AlgorithmConfig() , m_ref(ref)
{
  //Empty
}

#ifndef __MAKECINT__

dqm_algorithms::tools::SimpleAlgorithmConfig::SimpleAlgorithmConfig(const AlgorithmConfig& conf):
  dqm_core::AlgorithmConfig(), m_ref(conf.getReference())
{
  //Copy configuration in this
  m_parameters         = conf.getParameters();
  m_generic_parameters = conf.getGenericParameters();
  m_green_thresholds   = conf.getGreenThresholds();
  m_red_thresholds     = conf.getRedThresholds();
}
#endif

TObject* 
dqm_algorithms::tools::SimpleAlgorithmConfig::getReference() const
{
  TObject* ref ATLAS_THREAD_SAFE = const_cast<TObject*>(m_ref);  // dictated by interface
  return ref;
}

void
dqm_algorithms::tools::SimpleAlgorithmConfig::setReference(TObject* o)
{
  m_ref = o;
}

void
dqm_algorithms::tools::SimpleAlgorithmConfig::addParameter(const std::string & key, double value)
{
  m_parameters.insert( std::make_pair(key,value) );
}

void
dqm_algorithms::tools::SimpleAlgorithmConfig::addGenericParameter(const std::string & key, const std::string & value)
{
  m_generic_parameters.emplace( key, value );
}

void
dqm_algorithms::tools::SimpleAlgorithmConfig::addGreenThreshold(const std::string & key, double value)
{
  m_green_thresholds.insert( std::make_pair(key,value) );
}

void
dqm_algorithms::tools::SimpleAlgorithmConfig::addRedThreshold(const std::string & key, double value)
{
  m_red_thresholds.insert( std::make_pair(key,value) );
}

