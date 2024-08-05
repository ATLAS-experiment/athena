/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackParameters.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkParameters/TrackParameters.h"

namespace Trk {

 //explicit intantiation
 template class ParametersCommon<TrackParametersDim,Charged>;
 template class ParametersBase<TrackParametersDim,Charged>;

 template class ParametersT<TrackParametersDim,Charged,PlaneSurface>;
 template class ParametersT<TrackParametersDim,Charged,CylinderSurface>;
 template class ParametersT<TrackParametersDim,Charged,DiscSurface>;
 template class ParametersT<TrackParametersDim,Charged,ConeSurface>;
 template class ParametersT<TrackParametersDim,Charged,PerigeeSurface>;
 template class ParametersT<TrackParametersDim,Charged,StraightLineSurface>;
 template class CurvilinearParametersT<TrackParametersDim,Charged,PlaneSurface>;

}

/**Overload of << operator for both, MsgStream and std::ostream for debug output*/
MsgStream& operator << ( MsgStream& sl, const Trk::TrackParameters& pars)
{ return pars.dump(sl); }

/**Overload of << operator for both, MsgStream and std::ostream for debug output*/
std::ostream& operator << ( std::ostream& sl, const Trk::TrackParameters& pars)
{ return pars.dump(sl); }

