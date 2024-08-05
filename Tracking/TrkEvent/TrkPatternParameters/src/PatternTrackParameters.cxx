/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
// PatternTrackParameters.cxx , (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////


#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/ConeSurface.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkPatternParameters/PatternTrackParameters.h"
#include <iostream>
#include <iomanip>
#include <sstream>

///////////////////////////////////////////////////////////////////
// Conversion Trk::PatternTrackParameters to  Trk::TrackParameters
///////////////////////////////////////////////////////////////////

std::unique_ptr<Trk::TrackParameters>
Trk::PatternTrackParameters::convert(bool covariance) const
{
  std::optional<AmgSymMatrix(5)> e = std::nullopt;
  if (covariance && m_covariance != std::nullopt) {
    e = AmgSymMatrix(5)(*m_covariance);
  }
  const AmgVector(5)& p = m_parameters;
  return m_surface ? m_surface->createUniqueTrackParameters(
                        p[0], p[1], p[2], p[3], p[4], std::move(e))
                   : nullptr;
}

///////////////////////////////////////////////////////////////////
// Conversion Trk::TrackParameters to Trk::PatternTrackParameters
///////////////////////////////////////////////////////////////////

bool Trk::PatternTrackParameters::production(const Trk::ParametersBase<5,Trk::Charged>* T) {

  if (!T) {
    return false;
  }
  if (!T->hasSurface()) {
    return false;
  }

  m_surface.reset(T->associatedSurface().isFree() ? T->associatedSurface().clone() : &T->associatedSurface());

  m_parameters = T->parameters();

  const AmgSymMatrix(5)* C = T->covariance();

  if(C) {
    if (m_covariance == std::nullopt) {
      m_covariance.emplace();
    }

    for (std::size_t i = 0; i < 5; i++) {
      for (std::size_t j = 0; j <= i; j++) {
        m_covariance->fillSymmetric(i, j, (*C)(i, j));
      }
    }
  }
  else {
    m_covariance.reset();
  }

  return true;
}

///////////////////////////////////////////////////////////////////
// Global position of simple track parameters
///////////////////////////////////////////////////////////////////

Amg::Vector3D Trk::PatternTrackParameters::position() const
{
  return calculatePosition();
}

///////////////////////////////////////////////////////////////////
// Overload of << operator std::ostream
///////////////////////////////////////////////////////////////////

std::ostream& Trk::operator <<
  (std::ostream& sl,const Trk::PatternTrackParameters& se)
{
  return se.dump(sl);
}

MsgStream& Trk::operator    <<
(MsgStream& sl, const Trk::PatternTrackParameters& se)
{
  return se.dump(sl);
}

///////////////////////////////////////////////////////////////////
// Put track parameters information in a string representation
///////////////////////////////////////////////////////////////////

std::string
Trk::PatternTrackParameters::to_string() const {
  std::stringstream ss;
  const Trk::Surface*  s = m_surface.get();
  const AmgVector(5)&  P = m_parameters;
  const std::string name{s?s->name():""};
  const std::string N("\n");
  ss << "Track parameters for " << name << " surface " << N;
  ss.unsetf(std::ios::fixed);
  ss.setf  (std::ios::showpos);
  ss.setf  (std::ios::scientific);
  if (m_covariance != std::nullopt) {
    const AmgSymMatrix(5) & V = *m_covariance;
    ss << std::setprecision(4) <<
      P[ 0]<<" |"<<V(0, 0) << N;
    ss << std::setprecision(4) <<
      P[ 1]<<" |"<<V(0, 1)<<" "<<V(1, 1) << N;
    ss << std::setprecision(4) <<
      P[ 2]<<" |"<<V(0, 2)<<" "<<V(1, 2)<<" "<<V(2, 2) << N;
    ss << std::setprecision(4) <<
      P[ 3]<<" |"<<V(0, 3)<<" "<<V(1, 3)<<" "<<V(2, 3)<<" "<<V(3, 3) << N;
    ss << std::setprecision(4) <<
      P[ 4]<<" |"<<V(0, 4)<<" "<<V(1, 4)<<" "<<V(2, 4)<<" "<<V(3, 4)<<" "<<V(4, 4) << N;
  }
  else {
    ss << std::setprecision(4) << P[ 0] << " |" << N;
    ss << std::setprecision(4) << P[ 1] << " |" << N;
    ss << std::setprecision(4) << P[ 2] << " |" << N;
    ss << std::setprecision(4) << P[ 3] << " |" << N;
    ss << std::setprecision(4) << P[ 4] << " |" << N;
  }
  return ss.str();
}


///////////////////////////////////////////////////////////////////
// Print track parameters information
///////////////////////////////////////////////////////////////////

std::ostream& Trk::PatternTrackParameters::dump( std::ostream& out ) const
{
  out<<to_string();
  return out;
}

///////////////////////////////////////////////////////////////////
// Print track parameters information
///////////////////////////////////////////////////////////////////

MsgStream& Trk::PatternTrackParameters::dump(MsgStream& out) const
{
  out<<to_string();
  return out;
}

///////////////////////////////////////////////////////////////////
// Protected methods
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// Global position for track parameters on plane
///////////////////////////////////////////////////////////////////

Amg::Vector3D Trk::PatternTrackParameters::localToGlobal
(const Trk::PlaneSurface* su) const
{
  const Amg::Transform3D&  T = su->transform();
  double Ax[3] = {T(0,0),T(1,0),T(2,0)};
  double Ay[3] = {T(0,1),T(1,1),T(2,1)};

  Amg::Vector3D gp (m_parameters[0]*Ax[0]+m_parameters[1]*Ay[0]+T(0,3),
		    m_parameters[0]*Ax[1]+m_parameters[1]*Ay[1]+T(1,3),
		    m_parameters[0]*Ax[2]+m_parameters[1]*Ay[2]+T(2,3));
  return gp;
}

///////////////////////////////////////////////////////////////////
// Global position for track parameters on straight line
///////////////////////////////////////////////////////////////////

Amg::Vector3D Trk::PatternTrackParameters::localToGlobal
(const Trk::StraightLineSurface* su) const
{
  const Amg::Transform3D&  T = su->transform();
  double A[3] = {T(0,2),T(1,2),T(2,2)};

  double Sf;
  double Cf; sincos(m_parameters[2],&Sf,&Cf);
  double Se;
  double Ce; sincos(m_parameters[3],&Se,&Ce);

  double P3 = Cf*Se;
  double P4 = Sf*Se;
  double P5 = Ce;
  double Bx = A[1]*P5-A[2]*P4;
  double By = A[2]*P3-A[0]*P5;
  double Bz = A[0]*P4-A[1]*P3;
  double Bn = 1./std::sqrt(Bx*Bx+By*By+Bz*Bz); Bx*=Bn; By*=Bn; Bz*=Bn;

  Amg::Vector3D gp
    (m_parameters[1]*A[0]+Bx*m_parameters[0]+T(0,3),
     m_parameters[1]*A[1]+By*m_parameters[0]+T(1,3),
     m_parameters[1]*A[2]+Bz*m_parameters[0]+T(2,3));
  return gp;
}

///////////////////////////////////////////////////////////////////
// Global position for track parameters on disck
///////////////////////////////////////////////////////////////////

Amg::Vector3D Trk::PatternTrackParameters::localToGlobal
(const Trk::DiscSurface* su) const
{
  const Amg::Transform3D&  T = su->transform();
  double Ax[3] = {T(0,0),T(1,0),T(2,0)};
  double Ay[3] = {T(0,1),T(1,1),T(2,1)};

  double Sf;
  double Cf; sincos(m_parameters[1],&Sf,&Cf);

  double d0 = Cf*Ax[0]+Sf*Ay[0];
  double d1 = Cf*Ax[1]+Sf*Ay[1];
  double d2 = Cf*Ax[2]+Sf*Ay[2];

  Amg::Vector3D gp
    (m_parameters[0]*d0+T(0,3),
     m_parameters[0]*d1+T(1,3),
     m_parameters[0]*d2+T(2,3));
  return gp;
}

///////////////////////////////////////////////////////////////////
// Global position for track parameters on cylinder
///////////////////////////////////////////////////////////////////

Amg::Vector3D Trk::PatternTrackParameters::localToGlobal
(const Trk::CylinderSurface* su) const
{
  const Amg::Transform3D&  T = su->transform();
  double Ax[3] = {T(0,0),T(1,0),T(2,0)};
  double Ay[3] = {T(0,1),T(1,1),T(2,1)};
  double Az[3] = {T(0,2),T(1,2),T(2,2)};

  double  R = su->bounds().r();
  double fr = m_parameters[0]/R;

  double Sf;
  double Cf; sincos(fr,&Sf,&Cf);

  Amg::Vector3D gp
    (R*(Cf*Ax[0]+Sf*Ay[0])+m_parameters[1]*Az[0]+T(0,3),
     R*(Cf*Ax[1]+Sf*Ay[1])+m_parameters[1]*Az[1]+T(1,3),
     R*(Cf*Ax[2]+Sf*Ay[2])+m_parameters[1]*Az[2]+T(2,3));
  return gp;
}

///////////////////////////////////////////////////////////////////
// Global position for track parameters on perigee
///////////////////////////////////////////////////////////////////

Amg::Vector3D Trk::PatternTrackParameters::localToGlobal
(const Trk::PerigeeSurface* su) const
{
  const Amg::Transform3D&  T = su->transform();
  double A[3] = {T(0,2),T(1,2),T(2,2)};

  double Sf;
  double Cf; sincos(m_parameters[2],&Sf,&Cf);
  double Se;
  double Ce; sincos(m_parameters[3],&Se,&Ce);

  double P3 = Cf*Se;
  double P4 = Sf*Se;
  double P5 = Ce;
  double Bx = A[1]*P5-A[2]*P4;
  double By = A[2]*P3-A[0]*P5;
  double Bz = A[0]*P4-A[1]*P3;
  double Bn = 1./std::sqrt(Bx*Bx+By*By+Bz*Bz); Bx*=Bn; By*=Bn; Bz*=Bn;

  Amg::Vector3D gp
    (m_parameters[1]*A[0]+Bx*m_parameters[0]+T(0,3),
     m_parameters[1]*A[1]+By*m_parameters[0]+T(1,3),
     m_parameters[1]*A[2]+Bz*m_parameters[0]+T(2,3));
  return gp;
}

///////////////////////////////////////////////////////////////////
// Global position for track parameters on cone
///////////////////////////////////////////////////////////////////

Amg::Vector3D Trk::PatternTrackParameters::localToGlobal
(const Trk::ConeSurface* su) const
{
  const Amg::Transform3D&  T = su->transform();
  double Ax[3] = {T(0,0),T(1,0),T(2,0)};
  double Ay[3] = {T(0,1),T(1,1),T(2,1)};
  double Az[3] = {T(0,2),T(1,2),T(2,2)};

  double r  = m_parameters[1]*su->bounds().tanAlpha();
  double Sf;
  double Cf; sincos((m_parameters[0]/r),&Sf,&Cf);
  double xl = r*Cf;
  double yl = r*Sf;


  Amg::Vector3D gp
    (Ax[0]*xl+Ay[0]*yl+Az[0]*m_parameters[1]+T(0,3),
     Ax[1]*xl+Ay[1]*yl+Az[1]*m_parameters[1]+T(1,3),
     Ax[2]*xl+Ay[2]*yl+Az[2]*m_parameters[1]+T(2,3));
  return gp;
}

///////////////////////////////////////////////////////////////////
// Initiate track parameters
///////////////////////////////////////////////////////////////////

bool Trk::PatternTrackParameters::initiate
(PatternTrackParameters& Tp, const Amg::Vector2D& P,const Amg::MatrixX& E)
{

  int n = E.rows(); if(n<=0 || n>2) { return false;
}

  if (Tp.m_covariance != std::nullopt) {
    if (m_covariance == std::nullopt) {
      m_covariance = AmgSymMatrix(5)(*Tp.m_covariance);
    } else {
      *m_covariance = *Tp.m_covariance;
    }
  } else {
    if (m_covariance == std::nullopt) {
      m_covariance.emplace();
    }
  }

  m_parameters[0] = P  (0);

  m_covariance->fillSymmetric(0, 0, E(0,0));

  if(n==2) {
    m_parameters[ 1] = P(1);
    m_covariance->fillSymmetric(0, 1, E(1,0));
    m_covariance->fillSymmetric(1, 1, E(1,1));
  }
  else    {
    m_parameters[ 1] = Tp.m_parameters[ 1];
  }
  m_parameters[ 2] = Tp.m_parameters[ 2];
  m_parameters[ 3] = Tp.m_parameters[ 3];
  m_parameters[ 4] = Tp.m_parameters[ 4];

  if (Tp.m_surface != nullptr) {
    m_surface.reset(Tp.m_surface->isFree() ? Tp.m_surface->clone() : Tp.m_surface.get());
  } else {
    m_surface.reset(nullptr);
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////
// Change direction of the parameters
///////////////////////////////////////////////////////////////////

void Trk::PatternTrackParameters::changeDirection()
{
  constexpr double pi = M_PI;
  constexpr double pi2 = 2.*M_PI; //NB CLHEP also defines pi and pi2 constants.

  m_parameters[ 2] =  m_parameters[2]-pi;
  m_parameters[ 3] =  pi-m_parameters[3];
  m_parameters[ 4] = -m_parameters[4]   ;

  if (m_parameters[2] < -pi) {
    m_parameters[2] += pi2;
  }

  if ((m_surface->type() != Trk::SurfaceType::Line) &&
      (m_surface->type() != Trk::SurfaceType::Perigee)) {

    if (m_covariance == std::nullopt) {
      return;
    }

    m_covariance->fillSymmetric(0, 3, -(*m_covariance)(0, 3));
    m_covariance->fillSymmetric(1, 3, -(*m_covariance)(1, 3));
    m_covariance->fillSymmetric(2, 3, -(*m_covariance)(2, 3));
    m_covariance->fillSymmetric(0, 4, -(*m_covariance)(0, 4));
    m_covariance->fillSymmetric(1, 4, -(*m_covariance)(1, 4));
    m_covariance->fillSymmetric(2, 4, -(*m_covariance)(2, 4));

    return;
  }

  m_parameters[ 0] = -m_parameters[ 0];


  if(m_covariance == std::nullopt) {
    return;
  }

  m_covariance->fillSymmetric(0, 1, -(*m_covariance)(0, 1));
  m_covariance->fillSymmetric(0, 2, -(*m_covariance)(0, 2));
  m_covariance->fillSymmetric(1, 3, -(*m_covariance)(1, 3));
  m_covariance->fillSymmetric(2, 3, -(*m_covariance)(2, 3));
  m_covariance->fillSymmetric(1, 4, -(*m_covariance)(1, 4));
  m_covariance->fillSymmetric(2, 4, -(*m_covariance)(2, 4));
}

Amg::Vector3D Trk::PatternTrackParameters::calculatePosition(void) const {
  if (!m_surface) {
    return {0, 0, 0};
  }
  switch ( m_surface->type()){
  case Trk::SurfaceType::Plane:
    return localToGlobal(static_cast<const Trk::PlaneSurface*>(m_surface.get()));
    break;
  case Trk::SurfaceType::Line:
    return localToGlobal(static_cast<const Trk::StraightLineSurface*>(m_surface.get()));
    break;
  case Trk::SurfaceType::Disc:
    return localToGlobal(static_cast<const Trk::DiscSurface*>(m_surface.get()));
    break;
  case Trk::SurfaceType::Cylinder:
    return localToGlobal(static_cast<const Trk::CylinderSurface*>(m_surface.get()));
    break;
  case Trk::SurfaceType::Perigee:
    return localToGlobal(static_cast<const Trk::PerigeeSurface*>(m_surface.get()));
    break;
  case Trk::SurfaceType::Cone:
    return localToGlobal(static_cast<const Trk::ConeSurface*>(m_surface.get()));
    break;
  default:
    return {0, 0, 0};
  }
}

Amg::Vector3D Trk::PatternTrackParameters::calculateMomentum(void) const {
  double p = absoluteMomentum();
  double Sf = std::sin(m_parameters[2]), Cf = std::cos(m_parameters[2]);
  double Se = std::sin(m_parameters[3]), Ce = std::cos(m_parameters[3]);
  return {p * Se * Cf, p * Se * Sf, p * Ce};
}

