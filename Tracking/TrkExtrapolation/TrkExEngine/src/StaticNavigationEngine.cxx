/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// StaticNavigationEngine.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// STL
#include <sstream>
// Trk include
#include "TrkExEngine/StaticNavigationEngine.h"

// constructor
Trk::StaticNavigationEngine::StaticNavigationEngine(const std::string& t, const std::string& n, const IInterface* p)
: AthAlgTool(t,n,p)
{
    declareInterface<Trk::INavigationEngine>(this);
}

// destructor
Trk::StaticNavigationEngine::~StaticNavigationEngine()
= default;

// the interface method initialize
StatusCode Trk::StaticNavigationEngine::initialize()
{
    m_sopPrefix = m_sopPrefix_prop;
    m_sopPostfix = m_sopPostfix_prop;
    
    ATH_CHECK( m_trackingGeometryReadKey.initialize(!m_trackingGeometryReadKey.key().empty()) );
    ATH_CHECK( m_propagationEngine.retrieve());
    ATH_CHECK( m_materialEffectsEngine.retrieve());

    EX_MSG_DEBUG("", "initialize", "", "successful" );
    return StatusCode::SUCCESS;
}    

// the interface method finalize
StatusCode Trk::StaticNavigationEngine::finalize()
{    
    EX_MSG_DEBUG("", "finalize", "", "successful" );    
    return StatusCode::SUCCESS;
}

/** charged situation */
Trk::ExtrapolationCode Trk::StaticNavigationEngine::resolveBoundary(Trk::ExCellCharged& ecCharged, PropDirection dir) const
{ return resolveBoundaryT<Trk::TrackParameters>(ecCharged,dir); }

/** charged situation */
Trk::ExtrapolationCode Trk::StaticNavigationEngine::resolveBoundary(Trk::ExCellNeutral& ecNeutral, PropDirection dir) const
{ return resolveBoundaryT<Trk::NeutralParameters>(ecNeutral,dir); }

/** charged  */
Trk::ExtrapolationCode Trk::StaticNavigationEngine::resolvePosition(Trk::ExCellCharged& ecCharged, PropDirection dir, bool noLoop) const
{ return resolvePositionT<Trk::TrackParameters>(ecCharged,dir, noLoop); }

/** neutral */
Trk::ExtrapolationCode Trk::StaticNavigationEngine::resolvePosition(Trk::ExCellNeutral& ecNeutral, PropDirection dir, bool noLoop) const
{ return resolvePositionT<Trk::NeutralParameters>(ecNeutral,dir, noLoop); }

void Trk::StaticNavigationEngine::throwFailedToGetTrackingGeomtry() const {
   std::stringstream msg;
   msg << "Failed to get conditions data " << m_trackingGeometryReadKey.key() << ".";
   throw std::runtime_error(msg.str());
}
