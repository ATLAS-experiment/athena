/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_VERTEXPARAMETERSHELPER_H
#define INDETTRACKPERFMON_VERTEXPARAMETERSHELPER_H

/**
 * @file VertexParametersHelper.h
 * @brief Utility methods to access 
 *        reco/truth vertices parmeters in
 *        a consitent way across this package
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 27 November 2024
 **/

/// xAOD includes
#include "xAODTracking/Vertex.h"
#include "xAODTruth/TruthVertex.h"

/// STD includes
#include <cmath>


namespace IDTPM {

  /// Accessor utility function for getting the value of vertex position x
  template< class V >
  inline float posX( const V& v ) { return v.x(); }

  /// Accessor utility function for getting the value of vertex position y
  template< class V >
  inline float posY( const V& v ) { return v.y(); }

  /// Accessor utility function for getting the value of vertex position z
  template< class V >
  inline float posZ( const V& v ) { return v.z(); }

  /// Accessor utility function for getting the value of chi^2
  inline float getChiSquared( const xAOD::Vertex& v ) { return v.chiSquared(); }
  inline float getChiSquared( const xAOD::TruthVertex& ) { return -9999; }
  //inline float chiSquared( const V& v ) defined in TrackParametersHelper

  /// Accessor utility function for getting the value of #dof
  inline float getNdof( const xAOD::Vertex& v ) { return v.numberDoF(); }
  inline float getNdof( const xAOD::TruthVertex& ) { return -9999; }
  //inline float ndof( const V& v ) defined in TrackParametersHelper

  /// Accessor utility function for getting vertex hasValidTime
  inline uint8_t getHasValidTime( const xAOD::Vertex& v ) {
    static thread_local SG::ConstAccessor< uint8_t > accHasValidTime( "hasValidTime" );
    return accHasValidTime.isAvailable(v) ? accHasValidTime(v) : 0;
  }
  inline uint8_t getHasValidTime( const xAOD::TruthVertex& ) { return 1; }
  //inline uint8_t hasValidTime( const V& v ) defined in TrackParametersHelper

  /// Accessor utility function for getting the vertex time
  inline float getTime( const xAOD::Vertex& v ) {
    static thread_local SG::ConstAccessor< float > accTime( "time" );
    return accTime.isAvailable(v) ? accTime(v) : -9999.;
  }
  inline float getTime( const xAOD::TruthVertex& v ) { return v.t(); }
  //inline float time( const V& v ) defined in TrackParametersHelper

  /// Accessor utility function for getting the vertex type
  inline int getVertexType( const xAOD::Vertex& v ) { return int( v.vertexType() ); }
  inline int getVertexType( const xAOD::TruthVertex& ) { return -1; }
  template< class V >
  inline int vertexType( const V& v ) { return getVertexType(v); }

  /// enum for vertex position
  enum VposDefs : int { VposX=0, VposY=1, VposZ=2, NVpos=3 };

  /// Accessor utility function for getting the vertex position covariance
  inline float getCov( const xAOD::Vertex& v, VposDefs par1, VposDefs par2 ) {
    return v.covariancePosition()( par1, par2 );
  }
  inline float getCov( const xAOD::TruthVertex&, VposDefs, VposDefs ) { return 0.; }
  template< class V >
  inline float cov( const V& v, VposDefs par1, VposDefs par2 ) { return getCov( v, par1, par2 ); }

  /// Accessor utility function for getting the vertex position error
  inline float getError( const xAOD::Vertex& v, VposDefs par ) {
    return ( cov(v, par, par) < 0 ) ? 0. : std::sqrt( cov(v, par, par) ); }
  inline float getError( const xAOD::TruthVertex&, VposDefs ) { return 0.; }
  template< class V >
  inline float error( const V& v, VposDefs par ) { return getError( v, par ); }

  /// Accessor utility function for getting the vertex time resolution
  inline float getTimeErr( const xAOD::Vertex& v ) {
    static thread_local SG::ConstAccessor< float > accTimeResol( "timeResolution" );
    return accTimeResol.isAvailable(v) ? accTimeResol(v) : -9999.;
  }
  inline float getTimeErr( const xAOD::TruthVertex& ) { return 0.; }
  template< class V >
  inline float timeErr( const V& v ) { return getTimeErr(v); }

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_VERTEXPARAMETERSHELPER_H
