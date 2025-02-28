/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// LineSaggingDescriptor.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkDistortedSurfaces/LineSaggingDescriptor.h"
#include "TrkEventPrimitives/ParamDefs.h"
#include <iomanip>
#include <ostream>

// static gravity direction
alignas(16)
const Amg::Vector3D Trk::LineSaggingDescriptor::s_gravityDirection = -1 * Amg::Vector3D::UnitY();

// a reference direction
alignas(16)
const Amg::Vector3D Trk::LineSaggingDescriptor::s_referenceDirection(1./sqrt(3.),1./sqrt(3.),1./sqrt(3.));

// and a electro static scale factor
const double Trk::LineSaggingDescriptor::s_elecStatFactor(1.);

Trk::LineSaggingDescriptor::LineSaggingDescriptor(double wireLength,
                                                  double wireTension,
                                                  double linearDensity) :
   m_wireLength(wireLength),
   m_wireTension(wireTension),
   m_linearDensity(linearDensity)
{

}

MsgStream& Trk::LineSaggingDescriptor::dump( MsgStream& sl ) const
{
    sl << std::setiosflags(std::ios::fixed);
    sl << std::setprecision(7);
    sl << "Trk::LineSaggingDescriptor";
    sl <<  '\t' << "- wire length    :" << m_wireLength    << std::endl;
    sl <<  '\t' << "- wire tension   :" << m_wireTension   << std::endl;
    sl <<  '\t' << "- linear density :" << m_linearDensity << std::endl;
    sl << std::setprecision(-1);
    return sl;
}

std::ostream& Trk::LineSaggingDescriptor::dump( std::ostream& sl ) const
{
    sl << std::setiosflags(std::ios::fixed);
    sl << std::setprecision(7);
    sl << "Trk::LineSaggingDescriptor:";
    sl <<  '\t' << "- wire length    :" << m_wireLength    << std::endl;
    sl <<  '\t' << "- wire tension   :" << m_wireTension   << std::endl;
    sl <<  '\t' << "- linear density :" << m_linearDensity << std::endl;
    sl << std::setprecision(-1);
    return sl;
}
