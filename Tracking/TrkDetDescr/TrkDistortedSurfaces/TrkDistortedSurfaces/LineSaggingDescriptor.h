/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DistortionDescriptor.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDISTORTEDSURFACES_LINESAGGINGDESCRIPTOR_H
#define TRKDISTORTEDSURFACES_LINESAGGINGDESCRIPTOR_H

//STL
#include <iosfwd>
//#include <iomanip>

//GaudiKernel
#include "GaudiKernel/MsgStream.h"
// Geo & Maths
#include "GeoPrimitives/GeoPrimitives.h"

namespace Trk {

  /**
   @class LineSaggingDescriptor

   Fill in documentation

   @author Andreas.Salzburger@cern.ch
   */

  class LineSaggingDescriptor final {

    public:
     /**Default Constructor*/
     LineSaggingDescriptor()
         : m_wireLength{}, m_wireTension{}, m_linearDensity{} {
       // nop
     }

     /** Constructor  with arguments */
     LineSaggingDescriptor(double wireLength,
                           double wireTension,
                           double linearDensity);

     /** Output Method for MsgStream, to be overloaded by child classes */
     MsgStream& dump(MsgStream& sl) const;

     /** Output Method for std::ostream, to be overloaded by child classes */
     std::ostream& dump(std::ostream& sl) const;

    protected:
     //!< the wire end positions
     double m_wireLength;
     double m_wireTension;
     double m_linearDensity;

     /** direction of gravity */
     static const double s_elecStatFactor;
     alignas(16) static const Amg::Vector3D s_gravityDirection;
     alignas(16) static const Amg::Vector3D s_referenceDirection;
  };

} // end of namespace

#endif
