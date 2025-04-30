/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/ITkPixelClusterErrorData.h"

#include "GaudiKernel/ISvcLocator.h"

#include "StoreGate/StoreGateSvc.h"

#include <fstream>
#include <stdexcept>


namespace ITk
{

void PixelClusterErrorData::initialize()
{
  SmartIF<StoreGateSvc> detStore{Gaudi::svcLocator()->service("DetectorStore")};
  if(!detStore){
    throw std::runtime_error("Could not retrieve DetectorStore");
  }
  StatusCode sc = detStore->retrieve(m_pixelID, "PixelID");
  if(sc.isFailure() or (m_pixelID == nullptr)){
    throw std::runtime_error("Could not retrieve PixelID");
  }
  m_constmap.resize(m_pixelID->wafer_hash_max(),std::array<float,kNParam>{});
}


std::pair<double,double> PixelClusterErrorData::getDelta(IdentifierHash idHash,
                                                         int sizePhi, double angle,
                                                         int sizeZ, double eta) const
{

  const std::array<float,kNParam> &value = m_constmap.at(idHash);
  double period_phi = value[kPeriod_phi];
  double period_sinheta = value[kPeriod_sinheta];
  double delta_x_slope = value[kDelta_x_slope];
  double delta_x_offset = value[kDelta_x_offset];
  double delta_y_slope = value[kDelta_y_slope];
  double delta_y_offset = value[kDelta_y_offset];

  double delta_x = delta_x_slope * std::fabs(angle - period_phi*(sizePhi-2)) + delta_x_offset;
  double delta_y = delta_y_slope * std::fabs(std::sinh(std::fabs(eta)) - period_sinheta*(sizeZ-2)) + delta_y_offset;
  return std::make_pair(delta_x,delta_y);

}




// SET METHODS

void PixelClusterErrorData::setDeltaError(IdentifierHash idHash,
                                          float period_phi, float period_sinheta,
                                          float delta_x_slope, float delta_x_offset, float error_x,
                                          float delta_y_slope, float delta_y_offset, float error_y)
{
   setDeltaError(idHash, std::array<float, kNParam>{period_phi, period_sinheta,
                                                    delta_x_slope, delta_x_offset, error_x,
                                                    delta_y_slope, delta_y_offset, error_y});
}


// save all constants to file
void PixelClusterErrorData::print(const std::string& file) const
{
  std::ofstream outfile(file.c_str());
  for(unsigned int id_hash=0; const std::array<float, kNParam> &values : m_constmap){
     outfile << id_hash++;
     for (double a_val : values ) {
        outfile << " " << a_val;
     }
  }
  outfile.close();
}



// Load ITk constants from file
unsigned int PixelClusterErrorData::load(const std::string& file){

  std::ifstream infile( file.c_str() );
  unsigned int n_entries=0;
  if(infile.is_open()){

    //
    // Data in the file is stored in the following columns:
    // waferID_hash : period_phi : period_sinheta : delta_x_slope : delta_x_offset : delta_error_x : delta_y_slope : delta_y_offset : delta_error_y
    //

    int waferID_hash_int;
    double period_phi;
    double period_sinheta;
    double delta_x_slope;
    double delta_x_offset;
    double delta_error_x;
    double delta_y_slope;
    double delta_y_offset;
    double delta_error_y;

    while(!infile.eof()){

      infile >> waferID_hash_int >> period_phi >> period_sinheta >> delta_x_slope >> delta_x_offset >> delta_error_x >> delta_y_slope >> delta_y_offset >> delta_error_y;

      IdentifierHash waferID_hash(waferID_hash_int);
      setDeltaError(waferID_hash,
		    period_phi, period_sinheta,
		    delta_x_slope, delta_x_offset, delta_error_x,
		    delta_y_slope, delta_y_offset, delta_error_y);
      ++n_entries;

    }

    infile.close();

  } else {
    throw std::runtime_error("ITkAnalogueClusteringConstantsFile \"" + file + "\" can not be read. Unable to proceed.");
  }
  return n_entries;
}

} // namespace ITk
