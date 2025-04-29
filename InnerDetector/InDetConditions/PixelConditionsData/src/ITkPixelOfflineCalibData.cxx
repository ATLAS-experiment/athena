/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ITkPixelOfflineCalibData.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "PixelConditionsData/ITkPixelOfflineCalibData.h"
#include "Identifier/Identifier.h"

#include <sstream>

namespace ITk
{

  std::vector<float> PixelOfflineCalibData::getConstants() const {
    const std::vector< std::array<float, ITk::PixelClusterErrorData::kNParam> > &constMap = m_clusterErrorData->getConstMap();
    std::vector<float> constants;
    constants.reserve( (ITk::PixelClusterErrorData::kNParam+1) * constMap.size() );
    //
    for(unsigned int id_hash=0; const std::array<float, ITk::PixelClusterErrorData::kNParam>& values : constMap){
      long long pixelId = m_clusterErrorData->getIdentifier(id_hash++).get_compact();
      constants.push_back(pixelId); // @TODO not necessariy  lossless to convert an IdentifierHash into a float
      constants.insert(constants.end(), values.begin(),values.end());
    }
    return constants;
  }

  void PixelOfflineCalibData::dump() {
    m_clusterErrorData->print("ITkPixelClusterDump.txt");
  }


  void PixelOfflineCalibData::setConstants(const std::vector<float> &constants) {

    int entry_size = 9;
    int map_size = constants.size()/entry_size;

    for(int i=0;i<map_size;i++){

      long long pixelId_long = constants[i*entry_size];
      std::ostringstream ss;
      ss << "0x" << std::hex << pixelId_long;
      std::string pixelId_str(ss.str());
      Identifier pixelId;
      pixelId.set(pixelId_str);

      double period_phi = constants[i*entry_size + 1];
      double period_sinheta = constants[i*entry_size + 2];
      double delta_x_slope = constants[i*entry_size + 3];
      double delta_x_offset = constants[i*entry_size + 4];
      double delta_err_x = constants[i*entry_size + 5];
      double delta_y_slope = constants[i*entry_size + 6];
      double delta_y_offset = constants[i*entry_size + 7];
      double delta_err_y = constants[i*entry_size + 8];

      m_clusterErrorData->setDeltaError(m_clusterErrorData->getIdentifierHash(pixelId), period_phi, period_sinheta,
					delta_x_slope, delta_x_offset, delta_err_x,
					delta_y_slope, delta_y_offset, delta_err_y);

    }
  }

} // end of namespace ITk
