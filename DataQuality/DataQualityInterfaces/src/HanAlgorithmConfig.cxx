/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// **********************************************************************
// $Id: HanAlgorithmConfig.cxx,v 1.3 2009-05-07 14:45:54 ponyisi Exp $
// **********************************************************************

#include <TDirectory.h>
#include <TFile.h>
#include <TKey.h>
#include <TH1.h>
#include <TMap.h>

#include "DataQualityInterfaces/HanAlgorithmConfig.h"
#include "DataQualityInterfaces/HanConfigAlgLimit.h"
#include "DataQualityInterfaces/HanConfigAlgPar.h"
#include "DataQualityInterfaces/HanConfigAssessor.h"
#include "DataQualityInterfaces/HanConfigParMap.h"
#include "DataQualityInterfaces/HanRuntimeConfigSingleton.h"
#include "DataQualityInterfaces/HanUtils.h"


namespace dqi {

// *********************************************************************
// Public Methods
// *********************************************************************

HanAlgorithmConfig::
HanAlgorithmConfig( const HanConfigAssessor& hca, TFile* config )
  : m_file(config)
  , m_ref(0)
{
  CopyAlgConfig( hca );
  CopyRuntimeConfig();
}

HanAlgorithmConfig::
HanAlgorithmConfig( TObject* reference, 
                    const std::map< std::string, double >& parameters,
                    const std::map< std::string, std::string>& stringParameters,
                    const std::map< std::string, double >& greenThresholds,
                    const std::map< std::string, double >& redThresholds,
                    const HanConfigAssessor* hca)
    : m_file(0)
    , m_ref(reference)
    , m_hca(hca)
{
  m_parameters = parameters;
  m_generic_parameters = stringParameters;
  m_green_thresholds = greenThresholds;
  m_red_thresholds = redThresholds;
  CopyRuntimeConfig();
}

HanAlgorithmConfig::
~HanAlgorithmConfig()
{
  delete m_ref;
}


TObject*
HanAlgorithmConfig::
getReference() const
{
  if (m_ref) { 
    TObject* rv = m_ref->Clone();
    if (rv) {
      return rv;
    } else {
      const char* parname = m_hca ? m_hca->GetName() : "???";
      throw dqm_core::BadConfig( ERS_HERE, parname,
				 "Unable to clone reference" );
    }
  }
  const char* parname = m_hca ? m_hca->GetName() : "???";
  throw dqm_core::BadConfig( ERS_HERE, parname, 
			     "No reference histogram provided" );
}


// *********************************************************************
// Protected Methods
// *********************************************************************

void
HanAlgorithmConfig::
CopyAlgConfig( const HanConfigAssessor& hca )
{
  typedef std::map< std::string, double >::value_type ParsVal_t;
  typedef std::map< std::string, double >::value_type ThrVal_t;
  
  m_hca = &hca;

  std::string parName;
  TIter nextPar( hca.GetAllAlgPars() );
  HanConfigAlgPar* par;
  while( (par = dynamic_cast<HanConfigAlgPar*>( nextPar() )) != 0 ) {
    parName = std::string( par->GetName() );
    ParsVal_t parMapVal( parName, par->GetValue() );
    m_parameters.insert( std::move(parMapVal) );
  }
  TIter nextStrPar( hca.GetAllAlgStrPars() );
  HanConfigParMap* strPar;
  while( (strPar = dynamic_cast<HanConfigParMap*>( nextStrPar() )) != 0 ) {
    m_generic_parameters.emplace( strPar->GetName(), strPar->GetValue() );
  }
  
  std::string limName;
  TIter nextLim( hca.GetAllAlgLimits() );
  HanConfigAlgLimit* lim;
  while( (lim = dynamic_cast<HanConfigAlgLimit*>( nextLim() )) != 0 ) {
    limName = std::string( lim->GetName() );
    ThrVal_t greenMapVal( limName, lim->GetGreen() );
    m_green_thresholds.insert( std::move(greenMapVal) );
    ThrVal_t redMapVal( limName, lim->GetRed() );
    m_red_thresholds.insert( std::move(redMapVal) );
  }
  
  std::string refName( hca.GetAlgRefName() );
  if( refName != "" ) {
    TKey* key = dqi::getObjKey( m_file, refName );
    if (key == NULL) {
      std::cout << "ERROR: can't find reference " << refName << std::endl;
    } else {
      if (hca.GetIsRegex()) {
        TMap* map = dynamic_cast<TMap*>(key->ReadObj());
        if (! map) {
          std::cerr << "Problem reading TMap input for regex" << std::endl;
          m_ref = nullptr;
        } else {
          std::cout << "Get reference for  " << hca.GetHistPath() << std::endl;
          m_ref = map->GetValue(hca.GetHistPath());
        }
      } else {
        // not a regex
        m_ref = key->ReadObj();
      }
    }
  }
}

void
HanAlgorithmConfig::
CopyRuntimeConfig()
{
  auto& runtime = HanRuntimeConfigSingleton::getInstance();

  m_generic_parameters.emplace( "runtime_path", runtime.getPath() );
  if ( runtime.pathIsRunDirectory() )
    m_parameters.emplace( "run_number", runtime.getPathRunNumber() );
}


// *********************************************************************
// Private Methods
// *********************************************************************

} // namespace dqi
