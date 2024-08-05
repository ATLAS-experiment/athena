/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkNeutralParameters/NeutralParameters.h"

namespace Trk {

 //explicit instantiation
 template class ParametersBase<NeutralParametersDim,Neutral>;
 template class ParametersT<NeutralParametersDim,Neutral,PlaneSurface>;
 template class ParametersT<NeutralParametersDim,Neutral,CylinderSurface>;
 template class ParametersT<NeutralParametersDim,Neutral,DiscSurface>;
 template class ParametersT<NeutralParametersDim,Neutral,ConeSurface>;
 template class ParametersT<NeutralParametersDim,Neutral,PerigeeSurface>;
 template class ParametersT<NeutralParametersDim,Neutral,StraightLineSurface>;
 template class CurvilinearParametersT<NeutralParametersDim,Neutral,PlaneSurface>;

}

/**Overload of << operator for both, MsgStream and std::ostream for debug output*/
MsgStream& operator << ( MsgStream& sl, const Trk::NeutralParameters& pars)
{ return pars.dump(sl); }

/**Overload of << operator for both, MsgStream and std::ostream for debug output*/
std::ostream& operator << ( std::ostream& sl, const Trk::NeutralParameters& pars)
{ return pars.dump(sl); }
