/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BeamSpotRetriever.h"

#include "JiveXML/IFormatTool.h"

namespace JiveXML {

  /**
   * Standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  BeamSpotRetriever::BeamSpotRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent) {
    
    //Declare the interface
    declareInterface<IDataRetriever>(this);
  }
  
  StatusCode BeamSpotRetriever::initialize() {
      ATH_CHECK(m_beamSpotKey.initialize());
      return StatusCode::SUCCESS;
  }
  
  /**
   * Implementation of DataRetriever interface
   * - for each pixel raw data object obtain the identifier
   * - find the corresponding detector element and get its coordinates
   * - fill in all information in the data map
   * @param FormatTool the tool that will create formated output from the DataMap
   **/
  StatusCode BeamSpotRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    //be verbose
    ATH_MSG_DEBUG( "Retrieving " << dataTypeName() ); 

    DataVect x; x.reserve( 1 );  
    DataVect y; y.reserve( 1 );  
    DataVect z; z.reserve( 1 );  
    DataVect sigmaX; sigmaX.reserve( 1 );  
    DataVect sigmaXY; sigmaXY.reserve( 1 );  
    DataVect sigmaY; sigmaY.reserve( 1 );  
    DataVect sigmaZ; sigmaZ.reserve( 1 );  
    DataVect tiltX; tiltX.reserve( 1 );  
    DataVect tiltY; tiltY.reserve( 1 );  
    DataVect label; label.reserve( 1 );  
    DataVect status; status.reserve( 1 );  

/// from: https://twiki.cern.ch/twiki/bin/view/Atlas/InDetBeamSpotFAQ#How_can_I_retrieve_the_beam_spot
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey };
// add the following into the initialize routine.
  if ( !beamSpotHandle.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve beamspot " );
    return StatusCode::RECOVERABLE;
  } else {

// references:
//    https://twiki.cern.ch/twiki/bin/view/Atlas/InDetBeamSpotFAQ#How_can_I_retrieve_the_beam_spot
//    InnerDetector/InDetCalibAlgs/InDetBeamSpotFinder/trunk/#path_InnerDetector_InDetCalibAlgs_InDetBeamSpotFinder_trunk_
//
// https://twiki.cern.ch/twiki/bin/view/Atlas/CoolBeamSpotParameters#Beam_Spot_Parametrization
//
// status  	 Status word - see bit definition below
// posX 	x position of beam spot center (mm)
// posY 	y position of beam spot center (mm)
// posZ 	z position of beam spot center (mm)
// sigmaX 	Width of beam spot in x (mm)
// sigmaY 	Width of beam spot in x (mm)
// sigmaXY 	rhoXY*sigmaX*sigmaY (mm^2) (describes rotation of non-circular beam spot in transverse plane)
// sigmaZ 	Width of beam spot in z (mm)
// tiltX 	Tilt angle in x-z plane (rad)
// tiltY 	Tilt angle in y-z plane (rad)



    label.push_back( "Beamspot_position_at_PV_z_position" ); 
	
    Trk::RecVertex beamposition(beamSpotHandle->beamVtx());
    Amg::Vector3D posBS(beamposition.position());

    x.push_back( posBS.x() );
    y.push_back( posBS.y() );
    z.push_back( posBS.z() );

    sigmaX.push_back( beamSpotHandle->beamSigma(0) );
    sigmaY.push_back( beamSpotHandle->beamSigma(1) );
    sigmaZ.push_back( beamSpotHandle->beamSigma(2) );
    sigmaXY.push_back( beamSpotHandle->beamSigmaXY() );

    tiltX.push_back( beamSpotHandle->beamTilt(0) );
    tiltY.push_back( beamSpotHandle->beamTilt(1) );

    status.push_back( beamSpotHandle->beamStatus() );

    ATH_MSG_DEBUG( "BeamSpot Position: "
                   << beamSpotHandle->beamPos() );
    ATH_MSG_DEBUG( "BeamSpot Sigma "
                   << beamSpotHandle->beamSigma(0) << ", "
                   << beamSpotHandle->beamSigma(1) << ", "
                   << beamSpotHandle->beamSigma(2) );
    ATH_MSG_DEBUG( "BeamSpot Tilt: "
                   << beamSpotHandle->beamTilt(0) << ", "
                   << beamSpotHandle->beamTilt(1) );
    ATH_MSG_DEBUG( "Beamspot position at PV z-position" );

    //be verbose about the amount of data we retrieved
    ATH_MSG_DEBUG( "Retrieved " << label.size() << "Beamspot" );

    //Create a data map
    DataMap dataMap;
    //Add the data vectors to our map
    dataMap["x"]=std::move(x);
    dataMap["y"]=std::move(y);
    dataMap["z"]=std::move(z);
    dataMap["sigmaX"]=std::move(sigmaX);
    dataMap["sigmaY"]=std::move(sigmaY);
    dataMap["sigmaXY"]=std::move(sigmaXY);
    dataMap["sigmaZ"]=std::move(sigmaZ);
    dataMap["tiltX"]=std::move(tiltX);
    dataMap["tiltY"]=std::move(tiltY);
    dataMap["label"]=std::move(label);
    dataMap["status"]=std::move(status);

    //Forward data to formatting tool
    if ( FormatTool->AddToEvent(dataTypeName(), "BeamSpot", &dataMap).isFailure())
      return StatusCode::RECOVERABLE;

    //Clean up and exit
    ATH_MSG_DEBUG( dataTypeName() << " retrieved" );
    
    return StatusCode::SUCCESS;
    }
  }
}

