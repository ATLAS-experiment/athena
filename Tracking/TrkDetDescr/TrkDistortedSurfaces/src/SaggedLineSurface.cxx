/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SaggedLineSurface.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkDistortedSurfaces/SaggedLineSurface.h"
#include "TrkSurfaces/CylinderBounds.h"

// From Dan Levin: MDT
// linear density of wire: lambda=wireLinearDensity=19.3 [gm/cm^3] * PI*
//(25 *10^-4 )^2 [cm^2] = 378.954 microgram/cm
// From Dan Levin: MDT
// wireTen=350 for most chambers,  285 gm for some NIKHEF chambers (BOL),

Trk::SaggedLineSurface::SaggedLineSurface() :
  Trk::StraightLineSurface(),
  m_saggingDescriptor(nullptr),
  m_saggedLineDirection{}
{}

Trk::SaggedLineSurface::SaggedLineSurface(
  const Amg::Transform3D& htrans,
  double radius,
  double halez,
  Trk::LineSaggingDescriptor* lsd)
  : Trk::StraightLineSurface(htrans, radius, halez)
  , m_saggingDescriptor(lsd)
  , m_saggedLineDirection{}
{}

Trk::SaggedLineSurface::SaggedLineSurface(const Amg::Transform3D& htrans)
  : Trk::StraightLineSurface(htrans)
  , m_saggingDescriptor(nullptr)
  , m_saggedLineDirection{}
{}

Trk::SaggedLineSurface::SaggedLineSurface(
  const Trk::TrkDetElementBase& detelement,
  const Identifier& id)
  : Trk::StraightLineSurface(detelement, id)
  , m_saggingDescriptor()
  , m_saggedLineDirection{}
{}

Trk::SaggedLineSurface::SaggedLineSurface(const Trk::TrkDetElementBase& detelement, const Identifier& id,
                                          double wireLength, double wireTension, double linearDensity) :
  Trk::StraightLineSurface(detelement,id),
  m_saggingDescriptor(new Trk::LineSaggingDescriptor(wireLength, wireTension, linearDensity)), m_saggedLineDirection{}
{

}

Trk::SaggedLineSurface::SaggedLineSurface(const Trk::SaggedLineSurface& sls) :
  Trk::StraightLineSurface(sls),
  m_saggingDescriptor(new Trk::LineSaggingDescriptor(sls.distortionDescriptor())),
  m_saggedLineDirection{}
{}

Trk::SaggedLineSurface::~SaggedLineSurface()
{
  delete m_saggingDescriptor;
}

Trk::SaggedLineSurface& Trk::SaggedLineSurface::operator=(const Trk::SaggedLineSurface& sls)
{
  if (this != &sls ) {
    Trk::StraightLineSurface::operator=(sls);
    delete m_saggingDescriptor;
    m_saggedLineDirection=sls.m_saggedLineDirection;
    m_saggingDescriptor=new Trk::LineSaggingDescriptor(sls.distortionDescriptor());
  }
  return *this;
}

