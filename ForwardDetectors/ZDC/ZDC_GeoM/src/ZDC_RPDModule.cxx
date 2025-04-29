/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_RPDModule.h"

#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/Units.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/MsgStream.h"
#include "AthenaKernel/getMessageSvc.h"
#include "ZdcIdentifier/ZdcID.h"
#include "CLHEP/Geometry/Transform3D.h"
#include <stdexcept>

void ZDC_RPDModule::create(GeoFullPhysVol* mother, StoredMaterialManager *materialManager, const ZdcID *zdcID){

    MsgStream LogStream(Athena::getMessageSvc(), "ZDC_RPDModule::create");

    const GeoMaterial *Aluminum = materialManager->getMaterial("std::Aluminium");
    const GeoMaterial *OpAir = materialManager->getMaterial("ZDC::opticalAir");
    const GeoMaterial *OpSilicaCore = materialManager->getMaterial("ZDC::opticalSilica");
    const GeoMaterial *OpSilicaClad = materialManager->getMaterial("ZDC::opticalSilicaClad");
    const GeoMaterial *OpKapton = materialManager->getMaterial("ZDC::opticalKapton");

    // All parameters are either in mm or are unitless
    const int nRows = 4;
    const int nCols = 4;
    const float coreDia = 0.60;                                 // Fiber core diameter
    const float cladDia = 0.66;                                 // Fiber cladding diameter
    const float buffDia = 0.71;                                 // Fiber buffer diameter
    const float fiberPitchX = 1.425;                            // Spacing between fibers in X
    const float fiberPitchZ = 1.63;                             // Spacing between fibers in Z
    const float tileSize = 11.4;                                // Size of a square "tile"
    const float housingThickness = 5;                           // Thickness of aluminum housing
    const float detectorInnerWidth = 4 * tileSize;              // Width (x) of the detector cavity which contains the fibers
    const float detectorInnerDepth = 8 * fiberPitchZ + buffDia; // Depth (z) of the detector cavity which contains the fibers
    const float footWidth = 91;                                 // Width of the foot of the module
    const float footDepth = 75;                                 // Depth of the foot of the module
    const float footHeight = 10;                                // Height of the foot of the module
    const float posY = m_trf.translation().y();                 // Y position of center of the tiles

    const GeoBox* motherBox = dynamic_cast<const GeoBox*>(mother->getLogVol()->getShape());
    if (not motherBox){
      throw (std::runtime_error("ZDC_RPDModule::create: motherBox is a nullptr"));
    }
    const float halfY = motherBox->getYHalfLength();

    // Height of the extension between the main RPD body and the foot
    const float detectorHeight = 2*halfY - footHeight;          // Height of the detector minus the foot
    const float readoutFiberLength = halfY - posY - 2*tileSize; // Length of the "readout" section of the fibers, not including the active area extension
    
    // Build the main transforms here
    //The foot has to be placed at the bottom of the TAN/TAXN slot
    const GeoTrf::Transform3D footTrf = GeoTrf::TranslateY3D((-halfY - posY + footHeight * 0.5) * Gaudi::Units::mm) * m_trf;

    //The main housing is shifted up by half the foot height
    const GeoTrf::Transform3D moduleTrf = GeoTrf::TranslateY3D((0.5 * footHeight - posY) * Gaudi::Units::mm) * m_trf;

    char volName[64];

    // Aluminum housing (case)
    GeoBox *Housing_Box = new GeoBox((detectorInnerWidth + 2 * housingThickness) * Gaudi::Units::mm * 0.5,
                                      detectorHeight * Gaudi::Units::mm * 0.5,
                                     (detectorInnerDepth + 2 * housingThickness) * Gaudi::Units::mm * 0.5);

    GeoLogVol *Housing_Logical = new GeoLogVol("RPD_Housing_Logical", Housing_Box, Aluminum);
    GeoFullPhysVol *Housing_Physical = new GeoFullPhysVol(Housing_Logical);

    // Detector inner volume
    GeoBox *Module_Box = new GeoBox(detectorInnerWidth * Gaudi::Units::mm * 0.5,
                                    readoutFiberLength * Gaudi::Units::mm * 0.5,
                                    detectorInnerDepth * Gaudi::Units::mm * 0.5);

    GeoLogVol *Module_Logical = new GeoLogVol("RPD_Module_Logical", Module_Box, OpAir);
    GeoPhysVol *Module_Physical = new GeoPhysVol(Module_Logical);

    // Foot
    GeoBox *Foot_Box = new GeoBox(footWidth * Gaudi::Units::mm * 0.5,
                                  footHeight * Gaudi::Units::mm * 0.5,
                                  footDepth * Gaudi::Units::mm * 0.5);

    GeoLogVol *Foot_Logical = new GeoLogVol("RPD_Foot_Logical", Foot_Box, Aluminum);
    GeoPhysVol *Foot_Physical = new GeoPhysVol(Foot_Logical);

    // Readout fiber volumes are made here because they're all the same length
    // Core
    GeoTube *Readout_Core_Tube = new GeoTube(0.0 * Gaudi::Units::mm, coreDia * Gaudi::Units::mm * 0.5, readoutFiberLength * Gaudi::Units::mm * 0.5);
    GeoLogVol *Readout_Core_Logical = new GeoLogVol("RPD_Core_Readout_Logical", Readout_Core_Tube, OpSilicaCore);
    GeoPhysVol *Readout_Core_Physical = new GeoPhysVol(Readout_Core_Logical);

    // Cladding
    GeoTube *Readout_Clad_Tube = new GeoTube(coreDia * Gaudi::Units::mm * 0.5, cladDia * Gaudi::Units::mm * 0.5, readoutFiberLength * Gaudi::Units::mm * 0.5);
    GeoLogVol *Readout_Clad_Logical = new GeoLogVol("RPD_Clad_Readout_Logical", Readout_Clad_Tube, OpSilicaClad);
    GeoPhysVol *Readout_Clad_Physical = new GeoPhysVol(Readout_Clad_Logical);

    // Buffer
    GeoTube *Readout_Buffer_Tube = new GeoTube(cladDia * Gaudi::Units::mm * 0.5, buffDia * Gaudi::Units::mm * 0.5, readoutFiberLength * Gaudi::Units::mm * 0.5);
    GeoLogVol *Readout_Buffer_Logical = new GeoLogVol("RPD_Buff_Readout_Logical", Readout_Buffer_Tube, OpKapton);
    GeoPhysVol *Readout_Buffer_Physical = new GeoPhysVol(Readout_Buffer_Logical);

    /***************************************************
     *
     * The fibers are positioned in a way that distributes of all channels in a given column evenly in z
     * This is done by offsetting each fiber in x and z. For example, fibers for a single channel would look like this
     *
     * -0-------0--------
     * ---0-------0------
     * -----0-------0----
     * -------0--------0-
     *
     * The cycle has been repeated twice in order to achieve a "tile" size of approximately 10mm (11.4) while maintaining maximum density of fibers
     * Additionally this pattern is mirrored in z and interlaced with the first pattern. This is done to have uniformity in z for a given channel
     *
     *       Pattern1                  Pattern2                   Sum
     *    -0-------0--------       -------0--------0-      -0-----0-0------0-
     * so ---0-------0------  +    -----0-------0----  =   ---0-0-----0-0----
     *    -----0-------0----       ---0-------0------      ---0-0-----0-0----
     *    -------0--------0-       -0-------0--------      -0-----0-0------0-
     *
     * Everything above was for a single channel in row 0. Each row in a given colum starts the same pattern, just offset by fiberPitchZ
     * So that looks like this, where the number represents the row
     *
     * -0--1--2--3--0--1--2--3--
     * -3--0--1--2--3--0--1--2--
     * -2--3--0--1--2--3--0--1--
     * -1--2--3--0--1--2--3--0-
     *
     ****************************************************/

    float startX = tileSize * nCols * 0.5 - fiberPitchX / 4;
    float startZ = -fiberPitchZ * (nCols - 0.5) * 0.5;
    const unsigned int NFIBERS = 64; // Per row
    for (int row = 0; row < nRows; ++row)
    {
        float fiberLength = (1 + row) * tileSize;
        // float y = (-detectorHeight * 0.5 + housingThickness + tileSize * nRows - fiberLength * 0.5) * Gaudi::Units::mm;
        float y = (posY - 0.5 * footHeight + 0.5 * tileSize * nRows - fiberLength * 0.5) * Gaudi::Units::mm;

        // Core
        sprintf(volName, "RPD_Core_Active_Logical %d", row);
        GeoTube *Active_Core_Tube = new GeoTube(0.0 * Gaudi::Units::mm, coreDia * Gaudi::Units::mm * 0.5, fiberLength * Gaudi::Units::mm * 0.5);
        GeoLogVol *Active_Core_Logical= new GeoLogVol(volName, Active_Core_Tube, OpSilicaCore);
        GeoPhysVol *Active_Core_Physical = new GeoPhysVol(Active_Core_Logical);

        // Cladding
        sprintf(volName, "RPD_Clad_Active_Logical %d", row);
        GeoTube *Active_Clad_Tube = new GeoTube(coreDia * Gaudi::Units::mm * 0.5, cladDia * Gaudi::Units::mm * 0.5, fiberLength * Gaudi::Units::mm * 0.5);
        GeoLogVol *Active_Clad_Logical = new GeoLogVol(volName, Active_Clad_Tube, OpSilicaClad);
        GeoPhysVol *Active_Clad_Physical = new GeoPhysVol(Active_Clad_Logical);

        // Buffer
        sprintf(volName, "RPD_Buff_Active_Logical %d", row);
        GeoTube *Active_Buffer_Tube = new GeoTube(cladDia * Gaudi::Units::mm * 0.5, buffDia * Gaudi::Units::mm * 0.5, fiberLength * Gaudi::Units::mm * 0.5);
        GeoLogVol *Active_Buffer_Logical = new GeoLogVol(volName, Active_Buffer_Tube, OpKapton);
        GeoPhysVol *Active_Buffer_Physical = new GeoPhysVol(Active_Buffer_Logical);

        for (uint fiber = 0; fiber < NFIBERS; ++fiber)
        {

            // functions
            int index = fiber * 0.5;
            int pattern = fiber % 2;
            int cycle = fiber / (2 * nCols) % 2;
            int col = 2 * index / (nRows * nCols);
            float cycleOffset = cycle * nCols * fiberPitchX;
            float patternOffsetX = -pattern * fiberPitchX * 0.5;
            float patternOffsetZ = pattern * fiberPitchZ * 0.5;

            float x = startX - tileSize * col - cycleOffset + patternOffsetX - index % 4 * fiberPitchX;
            float z = startZ + fiberPitchZ * ((row + index % 4 + 2 * pattern) % 4) + patternOffsetZ;

            int channel = 4*row + col;

            std::string channelHashStr = zdcID->channel_id(m_side,m_module,ZdcIDType::ACTIVE,channel).getString();
            uint32_t channelHash = zdcID->channel_id(m_side,m_module,ZdcIDType::ACTIVE,channel).get_identifier32().get_compact();


            /*******************************************
             *     Core
             ********************************************/

            // Active region core get placed in aluminum

            // Active area
            sprintf(volName, "ZDC::RPD_Core_Active %s", channelHashStr.c_str());
            Housing_Physical->add(new GeoNameTag(volName));
            Housing_Physical->add(new GeoIdentifierTag(channelHash));
            Housing_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(x * Gaudi::Units::mm, y * Gaudi::Units::mm, z * Gaudi::Units::mm)));
            Housing_Physical->add(new GeoAlignableTransform(GeoTrf::RotateX3D(90 * Gaudi::Units::deg)));
            Housing_Physical->add(Active_Core_Physical);

            // Readout Fiber
            sprintf(volName, "ZDC::RPD_Core_Readout %s", channelHashStr.c_str());
            Module_Physical->add(new GeoNameTag(volName));
            Module_Physical->add(new GeoIdentifierTag(channelHash));
            Module_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(x * Gaudi::Units::mm, 0.0, z * Gaudi::Units::mm)));
            Module_Physical->add(new GeoAlignableTransform(GeoTrf::RotateX3D(90 * Gaudi::Units::deg)));
            Module_Physical->add(Readout_Core_Physical);

            /*******************************************
             *     Cladding
             ********************************************/

            // Active area
            sprintf(volName, "ZDC::RPD_Clad_Active %s", channelHashStr.c_str());
            Housing_Physical->add(new GeoNameTag(volName));
            Housing_Physical->add(new GeoIdentifierTag(channelHash));
            Housing_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(x * Gaudi::Units::mm, y * Gaudi::Units::mm, z * Gaudi::Units::mm)));
            Housing_Physical->add(new GeoAlignableTransform(GeoTrf::RotateX3D(90 * Gaudi::Units::deg)));
            Housing_Physical->add(Active_Clad_Physical);

            // Readout
            sprintf(volName, "ZDC::RPD_Clad_Readout %s", channelHashStr.c_str());
            Module_Physical->add(new GeoNameTag(volName));
            Module_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(x * Gaudi::Units::mm, 0.0, z * Gaudi::Units::mm)));
            Module_Physical->add(new GeoIdentifierTag(channelHash));
            Module_Physical->add(new GeoAlignableTransform(GeoTrf::RotateX3D(90 * Gaudi::Units::deg)));
            Module_Physical->add(Readout_Clad_Physical);

            /*******************************************
             *     Buffer
             ********************************************/

            // Active area
            sprintf(volName, "ZDC::RPD_Buffer_Active %s", channelHashStr.c_str());
            Housing_Physical->add(new GeoNameTag(volName));
            Housing_Physical->add(new GeoIdentifierTag(channelHash));
            Housing_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(x * Gaudi::Units::mm, y * Gaudi::Units::mm, z * Gaudi::Units::mm)));
            Housing_Physical->add(new GeoAlignableTransform(GeoTrf::RotateX3D(90 * Gaudi::Units::deg)));
            Housing_Physical->add(Active_Buffer_Physical);

            // Readout Fiber
            sprintf(volName, "ZDC::RPD_Buffer_Readout %s", channelHashStr.c_str());
            Module_Physical->add(new GeoNameTag(volName));
            Module_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(x * Gaudi::Units::mm, 0.0, z * Gaudi::Units::mm)));
            Module_Physical->add(new GeoAlignableTransform(GeoTrf::RotateX3D(90 * Gaudi::Units::deg)));
            Module_Physical->add(new GeoIdentifierTag(channelHash));
            Module_Physical->add(Readout_Buffer_Physical);

        } // end loop over fibers
    }// end loop over rows

    Identifier id;

    // Place the fiber routing volume
    id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::AIR);
    sprintf(volName, "ZDC::RPD_Air_Cavity %s", id.getString().c_str());
    Housing_Physical->add(new GeoNameTag(volName));
    Housing_Physical->add(new GeoIdentifierTag( id.get_identifier32().get_compact()));
    Housing_Physical->add(new GeoAlignableTransform(GeoTrf::TranslateY3D(  0.5 * (detectorHeight - readoutFiberLength) * Gaudi::Units::mm)));
    Housing_Physical->add(Module_Physical);
    
    // Place the foot in the mother volume
    id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::HOUSING);
    sprintf(volName, "ZDC::RPD_Foot %s",id.getString().c_str());
    mother->add(new GeoNameTag(volName));
    mother->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
    mother->add(new GeoAlignableTransform( footTrf ));
    mother->add(Foot_Physical);

    // Place the module in the mother volume
    id = zdcID->channel_id(m_side, 4, ZdcIDType::INACTIVE,ZdcIDVolChannel::HOUSING);
    sprintf(volName, "Zdc::RPD_Mod %s", id.getString().c_str());
    mother->add(new GeoNameTag(volName));
    mother->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
    mother->add(new GeoAlignableTransform( moduleTrf));
    mother->add(Housing_Physical);

}