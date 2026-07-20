/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*! \file DummyAlgorithm.cxx
 *  Always returns Result::Undefined.
 *  Assign this in the .config to any histogram whose detector channel
 *  is switched off so the DQ web display shows no colour (not red/green/yellow).
 */

#include <dqm_algorithms/DummyAlgorithm.h>
#include <dqm_core/AlgorithmManager.h>
#include <dqm_core/Result.h>
#include <TObject.h>
#include <ers/ers.h>

namespace {
    dqm_algorithms::DummyAlgorithm Dummy( "Dummy_Algorithm" );
}

dqm_algorithms::DummyAlgorithm::DummyAlgorithm( const std::string& name )
    : m_name( name )
{
    dqm_core::AlgorithmManager::instance().registerAlgorithm( name, this );
}

dqm_algorithms::DummyAlgorithm*
dqm_algorithms::DummyAlgorithm::clone()
{
    return new DummyAlgorithm( m_name );
}

dqm_core::Result*
dqm_algorithms::DummyAlgorithm::execute(
    const std::string&               name,
    const TObject&                   /*object*/,
    const dqm_core::AlgorithmConfig& /*config*/ )
{
    ERS_DEBUG(1, "DummyAlgorithm: returning Undefined for " << name
                 << " (channel intentionally disabled in detector)");
    return new dqm_core::Result( dqm_core::Result::Undefined );
}

void
dqm_algorithms::DummyAlgorithm::printDescription( std::ostream& out )
{
    out << "Dummy_Algorithm: Always returns Result::Undefined.\n"
        << "Use for histograms whose detector channel is intentionally\n"
        << "disabled (e.g. switched-off sTGC Qi/Li layers in Pad/Strip/Wire).\n"
        << "No parameters or thresholds required.\n" << std::endl;
}