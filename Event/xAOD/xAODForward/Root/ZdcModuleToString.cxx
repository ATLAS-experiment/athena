/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODForward/ZdcModuleToString.h"
#include "AthContainers/ConstAccessor.h"
#include <sstream>

std::string ZdcModuleToString(const xAOD::ZdcModule& zm) 
{
  std::stringstream o;
  o << " ID=" << zm.zdcId();
  o << " S/M/T/C=" << zm.zdcSide();
  o << "/" << zm.zdcModule(); 
  o << "/" << zm.zdcType();
  o << "/" << zm.zdcChannel();
  o << "\n";
  for (auto s : {"g0data","g1data","g0d0data","g0d1data","g1d0data","g1d1data"} )
    {
      SG::ConstAccessor<std::vector<uint16_t> >acc(s);
      if (acc.isAvailable(zm))
	{
      	  o << s << ": ";
	  const std::vector<uint16_t>& v = zm.getWaveform(s);
	  for (uint16_t d : v)
	    {
	      o << " " << std::to_string(d);
	    }
	  o << "\n";
	}

    }
  static const SG::ConstAccessor<uint16_t> LucrodTriggerAmpAcc("LucrodTriggerAmp");
  if (LucrodTriggerAmpAcc.isAvailable(zm))
    {
      o << "Trigger amp:" << LucrodTriggerAmpAcc(zm) << "\n";
    }
  static const SG::ConstAccessor<uint16_t> LucrodTriggerSideAmpAcc("LucrodTriggerSideAmp");
  if (LucrodTriggerSideAmpAcc.isAvailable(zm))
    {
      o << "Trigger side amp:" << LucrodTriggerSideAmpAcc(zm) << "\n";
    }
  return o.str();
}

std::string ZdcModuleToString(const xAOD::ZdcModuleContainer& zc) 
{
  std::stringstream o;
  for (auto z: zc) { o << ZdcModuleToString(*z); }
  return o.str();
}
