/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// FitQuality.cxx (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkEventPrimitives/FitQuality.h"
#include "GaudiKernel/MsgStream.h"
#include <string>
#include <ostream>
#include <sstream>

/**Overload of << operator for both, MsgStream and std::ostream for debug output*/ 
MsgStream& Trk::operator<<(MsgStream& sl, const Trk::FitQualityImpl& fq)
{
  std::ostringstream os;
  os<<fq;
  sl<<os.str();
  return sl;
}

std::ostream& Trk::operator << ( std::ostream& sl, const Trk::FitQualityImpl& fq)
{ 
  const std::streamsize old_prec = sl.precision();
  const auto old_flags = sl.flags();
  sl << std::setiosflags(std::ios::fixed)<< std::setprecision(3);
  sl <<"FitQuality: \t"
     << "("<<fq.chiSquared()<<", \t"<<fq.numberDoF()<<")\t"
     << "(chi^2, ndf)";
  sl.flags(old_flags);
  sl.precision(old_prec);
  return sl;
}

