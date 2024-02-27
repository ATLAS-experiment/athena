/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_BRANModule.h"

#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/Units.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "StoreGate/StoreGateSvc.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelKernel/GeoMaterial.h"


#include "AthenaKernel/getMessageSvc.h"
#include "CLHEP/Geometry/Transform3D.h"

/*
 * For now we create the Run3 BRAN only. The detector is a stack of plates in z
 * with grooves milled vertically to allow channels for silica rods to be placed in.
 * The stack ordering is Cu, Al, Cu, Cu, Al, Cu. This will be simplified here by
 * creating a copper body with aluminum plates placed within.
*/
GeoFullPhysVol* ZDC_BRANModule::create(){

    MsgStream LogStream(Athena::getMessageSvc(), "ZDC_ZDCModule::create");

    StoredMaterialManager *materialManager = nullptr;
    if (StatusCode::SUCCESS != m_detectorStore->retrieve(materialManager, "MATERIALS")) {
        MsgStream LogStream(Athena::getMessageSvc(), "ZDC_ZDCModule::create");
        LogStream << MSG::ERROR << "execute: Could not retrieve StoredMaterialManager object from the detector store" << endmsg;
        return nullptr;
    }

    const GeoMaterial *OpAir    = materialManager->getMaterial("ZDC::opticalAir"   );
    const GeoMaterial *OpSilica = materialManager->getMaterial("ZDC::opticalSilica");
    const GeoMaterial *copper   = materialManager->getMaterial("std::Copper"       );
    const GeoMaterial *aluminum = materialManager->getMaterial("std::Aluminium"    );

    // geometric constants. All units are in millimeters unless otherwise specified.
    // All dimensions are gathered from technical drawings available at TODO: insert link to tech drawings
    const float width           = 88.0;                                               // x dimension of the module
    const float height          = 180.0;                                              // y dimension of the module
    const float channelSide     = 11.0;                                               // side dimension of the silica rod channels
    const float pitch           = 2.0*channelSide;                                    // distance between adjacent silica rod channels
    const float faceDepth       = 10.0;                                               // z depth of the front and back faces
    const float alPlateDepth    = 14.5;                                               // z depth of the aluminum plates
    const float innerPlateDepth = 25.5;                                               // z depth of the inner plates
    const float rodDia          = 10.0;                                               // diameter of the silica rods
    const float depth           = 2.0  *(faceDepth + alPlateDepth + innerPlateDepth); // y dimension of the module

    GeoBox  *Cu_Body     = new GeoBox  (width       * Gaudi::Units::mm * 0.5, height * Gaudi::Units::mm * 0.5, depth        * Gaudi::Units::mm * 0.5 );
    GeoBox  *Al_Plate    = new GeoBox  (width       * Gaudi::Units::mm * 0.5, height * Gaudi::Units::mm * 0.5, alPlateDepth * Gaudi::Units::mm * 0.5 );
    GeoBox  *Air_Channel = new GeoBox  (channelSide * Gaudi::Units::mm * 0.5, height * Gaudi::Units::mm * 0.5, channelSide  * Gaudi::Units::mm * 0.5 );
    GeoTube *Silica_Rod  = new GeoTube (0.0         * Gaudi::Units::mm      , rodDia * Gaudi::Units::mm * 0.5, height       * Gaudi::Units::mm * 0.5 );

    GeoLogVol *Cu_Body_Logical     = new GeoLogVol("BRAN_Cu_Body_Logical"     , Cu_Body     , copper  );
    GeoLogVol *Al_Plate_Logical    = new GeoLogVol("BRAN_Al_Plate_Logical"    , Al_Plate    , aluminum);
    GeoLogVol *Air_Channel_Logical = new GeoLogVol("BRAN_Air_Channel_Logical" , Air_Channel , OpAir   );
    GeoLogVol *Silica_Rod_Logical  = new GeoLogVol("BRAN_Rod_Logical"         , Silica_Rod  , OpSilica);

    // Create the copper body
    GeoFullPhysVol *Cu_Body_Physical = new GeoFullPhysVol(Cu_Body_Logical);

    //Create the IDs and names here since we aren't doing any channel separation
    Identifier airID = m_zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::AIR);;
    Identifier aluID = m_zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::HOUSING);
    Identifier rodID = m_zdcID->channel_id(m_side,m_module,ZdcIDType::ACTIVE,ZdcIDVolChannel::READOUT);

    char airName[64], rodName[64], aluName[64];
    sprintf(airName, "ZDC::BRAN_Air %s", airID.getString().c_str());
    sprintf(aluName, "ZDC::BRAN_Alu %s", aluID.getString().c_str());
    sprintf(rodName, "ZDC::BRAN_Rod %s", rodID.getString().c_str());

    //Assemble the silica rods in the air channels
    GeoFullPhysVol *Air_Channel_Pyisical = new GeoFullPhysVol(Air_Channel_Logical);
    Air_Channel_Pyisical->add(new GeoNameTag(rodName));
    Air_Channel_Pyisical->add(new GeoIdentifierTag(rodID.get_identifier32().get_compact()));
    Air_Channel_Pyisical->add(new GeoAlignableTransform(GeoTrf::RotateX3D(90 * Gaudi::Units::deg)));
    Air_Channel_Pyisical->add(new GeoFullPhysVol(Silica_Rod_Logical));

    GeoFullPhysVol *Al_Plate_Physical = new GeoFullPhysVol(Al_Plate_Logical);

    /*This is a strange loop but the arrangement of the rods is
     *   ----------------------------------
     *   |  {-1,-1}     {-1,0}     {-1,1} |
     *   |          {-1}       {1}        |
     *   |  {1,-1}      {1,0}      {1,1}  |
     *   ----------------------------------
    */
    for(int i : {-1,1}){
        /*************************************************
         * Place the inner air channels with silica included
         **************************************************/
        Cu_Body_Physical->add(new GeoNameTag(airName));
        Cu_Body_Physical->add(new GeoIdentifierTag(airID.get_identifier32().get_compact()));
        Cu_Body_Physical->add(new GeoAlignableTransform(GeoTrf::TranslateX3D( i * channelSide * Gaudi::Units::mm)));
        Cu_Body_Physical->add(Air_Channel_Pyisical);

        /*************************************************
         * Place the aluminum plates
         **************************************************/
        Cu_Body_Physical->add(new GeoNameTag(aluName));
        Cu_Body_Physical->add(new GeoIdentifierTag(aluID.get_identifier32().get_compact()));
        Cu_Body_Physical->add(new GeoAlignableTransform(GeoTrf::TranslateZ3D( i * (innerPlateDepth + alPlateDepth * 0.5) * Gaudi::Units::mm)));
        Cu_Body_Physical->add(Al_Plate_Physical);

        for(int j : {-1,0,1}){
            /*************************************************
             * Place the outter air channels
             **************************************************/
            Cu_Body_Physical->add(new GeoNameTag(airName));
            Cu_Body_Physical->add(new GeoIdentifierTag(airID.get_identifier32().get_compact()));
            Cu_Body_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D( j * pitch * Gaudi::Units::mm, 0.0, i * (innerPlateDepth - channelSide) * Gaudi::Units::mm )));
            Cu_Body_Physical->add(Air_Channel_Pyisical);
        }
    }

    return Cu_Body_Physical;

}