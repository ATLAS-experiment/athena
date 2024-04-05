/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_ZDCModule.h"

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

ZDC_ZDCModule::ZDC_ZDCModule()
    : ZDC_ModuleBase(),
      m_pixelStart(0),
      m_pixelStop(0)
{
}

ZDC_ZDCModule::ZDC_ZDCModule(StoreGateSvc *detStore, int side, int module, const ZdcID *zdcID, int pixelStart, int pixelStop)
    : ZDC_ModuleBase(detStore, side, module, zdcID),
      m_pixelStart(pixelStart),
      m_pixelStop(pixelStop)
{
}

ZDC_ZDCModule::ZDC_ZDCModule(ZDC_ZDCModule *right, int side, int module) 
    : ZDC_ModuleBase((ZDC_ModuleBase*)right, side, module)
{
    m_pixelStart = right->m_pixelStart;
    m_pixelStop = right->m_pixelStop;
}


void ZDC_ZDCModule::create(GeoFullPhysVol* mother, GeoAlignableTransform* trf){

    MsgStream LogStream(Athena::getMessageSvc(), "ZDC_ZDCModule::create");

    StoredMaterialManager *materialManager = nullptr;
    if (StatusCode::SUCCESS != m_detectorStore->retrieve(materialManager, "MATERIALS")) {
        MsgStream LogStream(Athena::getMessageSvc(), "ZDC_ZDCModule::create");
        LogStream << MSG::ERROR << "execute: Could not retrieve StoredMaterialManager object from the detector store" << endmsg;
        return;
    }

    const GeoMaterial *OpAir = materialManager->getMaterial("ZDC::opticalAir");
    const GeoMaterial *OpSilica = materialManager->getMaterial("ZDC::opticalSilica");
    const GeoMaterial *Tungsten = materialManager->getMaterial("ZDC::Tungsten");
    const GeoMaterial *Steel = materialManager->getMaterial("ZDC::Steel");

    // geometric constants. All units are in millimeters unless otherwise specified.
    // All dimensions are gathered from technical drawings available at TODO: insert link to tech drawings

    // Dimensions determined by the pixel modules
    // Both modules with pixels equipped and without have features determined by the pixel geometry, namely the gap number and spacing between
    // groups of vertical radiator rods
    const float pixelPitch = 10.3;                                    // pixel rod pitch in both x and y
    const float pixelGap = 1.2;                                       // distance between the main fiber channels for the pixel fibers to be routed
    const float pixelRodDia = 1.0;                                    // diameter of the quartz rods used for the pixel
    const float pixelHoleDia = 1.35;                                  // diameter of the holes drilled in the tungsten plates which the pixel rods pass through
    const int   nPixelHolesX = 8;                                     // number of positions for pixel rods in x (May or may not contain a quartz rod)
    const int   nPixelHolesY = 16;                                    // number of positions for pixel rods in y (May or may not contain a quartz rod)
    const float firstPixelX = -pixelPitch * (nPixelHolesX - 1) * 0.5; // Position of the outter most pixel placement in x
    const float firstPixelY = -pixelPitch * (nPixelHolesY - 1) * 0.5; // Position of the lowest pixel placement in y

    // These dimensions exist independent of the pixels
    const float radiator_gap_depth = 2.0;                                                             // Depth of the radiator gap
    const int nRadGaps = 12;                                                                          // number of tungsten plates
    const float strip_diameter = 1.5;                                                                 // Diameter of the vertical quartz rods
    const float absorber_width = 89.57;                                                               // Depth (z) of the tungsten modulecase
    const float absorber_depth = 10.0;                                                                // Depth (z) of the tungsten absorber
    const float absorber_height = 180.0;                                                              // Height (y) of the tungsten absorber
    const float wallThicknessFront = 8.0;                                                             // Depth (z) of the steel case wall closest to the IP
    const float wallThicknessBack = 10.0;                                                             // Depth (z) of the steel case wall furthest from the IP
    const float wallThicknessSide = 1.2;                                                              // Thickness of the side walls (x) of the module
    const float zPitch = radiator_gap_depth + absorber_depth;                                         // tungsten plate/quartz rod pitch in z
    const float strip_gap_center = pixelPitch - pixelGap;                                             // Width of the center channels for quartz rods
    const float strip_gap_edge = (absorber_width - pixelPitch * (nPixelHolesX - 1) - pixelGap) * 0.5; // Width of the edge channels for quartz rods
    const float module_depth = (nRadGaps - 1) * zPitch + radiator_gap_depth;                          // Depth (z) of the air cavity within the steel housing
    const float housing_width = absorber_width + 2 * wallThicknessSide;                               // Width (x) of the steel module case
    const float housing_height = absorber_height + 5.0;                                               // Height (y) of the steel module case
    const float housing_depth = module_depth + wallThicknessFront + wallThicknessBack;                // Depth (z) of the steel module case
    const float zStartStrip = (-module_depth + radiator_gap_depth) * 0.5;                             // Location (z) of the first radiator
    const float zStartW = zStartStrip + zPitch * 0.5;                                                 // Location (z) of the first tungsten plate

    GeoTube *Strip_Tube     = new GeoTube(0.0            * Gaudi::Units::mm      , strip_diameter  * Gaudi::Units::mm * 0.5, absorber_height    * Gaudi::Units::mm * 0.5);
    GeoBox  *Steel_Box      = new GeoBox (housing_width  * Gaudi::Units::mm * 0.5, housing_height  * Gaudi::Units::mm * 0.5, housing_depth      * Gaudi::Units::mm * 0.5);
    GeoBox  *Module_Box     = new GeoBox (absorber_width * Gaudi::Units::mm * 0.5, absorber_height * Gaudi::Units::mm * 0.5, module_depth       * Gaudi::Units::mm * 0.5);
    GeoBox  *W_Plate        = new GeoBox (absorber_width * Gaudi::Units::mm * 0.5, absorber_height * Gaudi::Units::mm * 0.5, absorber_depth     * Gaudi::Units::mm * 0.5);
    GeoTube *Pixel_Tube_W   = new GeoTube(0.0            * Gaudi::Units::mm      , pixelRodDia     * Gaudi::Units::mm * 0.5, absorber_depth     * Gaudi::Units::mm * 0.5);
    GeoTube *Pixel_Tube_Rad = new GeoTube(0.0            * Gaudi::Units::mm      , pixelRodDia     * Gaudi::Units::mm * 0.5, radiator_gap_depth * Gaudi::Units::mm * 0.5);
    GeoTube *Pixel_Hole     = new GeoTube(0.0            * Gaudi::Units::mm      , pixelHoleDia    * Gaudi::Units::mm * 0.5, absorber_depth     * Gaudi::Units::mm * 0.5);

    GeoLogVol *Strip_Logical      = new GeoLogVol("Strip_Logical"     , Strip_Tube    , OpSilica);
    GeoLogVol *Steel_Logical      = new GeoLogVol("Steel_Logical"     , Steel_Box     , Steel   );
    GeoLogVol *Module_Logical     = new GeoLogVol("Module_Logical"    , Module_Box    , OpAir   );
    GeoLogVol *W_Plate_Logical    = new GeoLogVol("W_Plate_Logical"   , W_Plate       , Tungsten);
    GeoLogVol *Pixel_W_Logical    = new GeoLogVol("Pixel_W_Logical"   , Pixel_Tube_W  , OpSilica);
    GeoLogVol *Pixel_Rad_Logical  = new GeoLogVol("Pixel_Rad_Logical" , Pixel_Tube_Rad, OpSilica);
    GeoLogVol *Pixel_Hole_Logical = new GeoLogVol("Pixel_Hole_Logical", Pixel_Hole    , OpAir   );

    char volName[256];
    int nPixelMods = 0;
    if(m_pixelStop - m_pixelStart > 0) nPixelMods = 1;
    if(m_pixelStop - m_pixelStart > 8) nPixelMods = 2;

    // TODO: Try using ZdcId hashes for the GeoIdentifierTags

    // Create the steel box (housing) and place air inside (module)
    GeoFullPhysVol *Module_Physical = new GeoFullPhysVol(Module_Logical);
    GeoFullPhysVol *Housing_Physical = new GeoFullPhysVol(Steel_Logical);

    //A surrogate ID to give the volumes a reference
    Identifier id;

    // Because there is a radiator gap at the front and back of the modules, there is one more gap than absorbers
    for (int radGap = 0; radGap < nRadGaps; ++radGap){
        /*************************************************
         * Place tungsten plates
         **************************************************/
        GeoFullPhysVol *W_Plate = 0;
        if (radGap != nRadGaps - 1){ // Don't place a tungsten plate after the last radiator gap
            id = m_zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::TUNGSTEN);
            sprintf(volName, "ZDC::W_Mod %s", id.getString().c_str());
            Module_Physical->add(new GeoNameTag(volName));
            Module_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
            Module_Physical->add(new GeoAlignableTransform(GeoTrf::TranslateZ3D((zStartW + radGap * zPitch) * Gaudi::Units::mm)));
            W_Plate = new GeoFullPhysVol(W_Plate_Logical);
            Module_Physical->add(W_Plate);
        }

        // Make a few variables that will be used to place the rods in the next loop
        float radGapz = zStartStrip + zPitch * radGap;

        // All pixel fibers go in the radiator gap and tungsten plates, except the last layer which goes in the steel housing
        // If this isn't a pixel module, these variables just won't get used
        GeoFullPhysVol *pixelsPlacedIn = (radGap != nRadGaps - 1) ? W_Plate : Housing_Physical;
        float absPixelZ = (radGap != nRadGaps - 1) ? 0.0 : (wallThicknessBack - wallThicknessFront) + (module_depth + wallThicknessBack) * 0.5;

        for (int rodChannel = 0; rodChannel < nPixelHolesX + 1; ++rodChannel){
            /*************************************************
             * Place vertical quartz rods
             **************************************************/

            int nRods = 6;
            float rodPitch = strip_gap_center / nRods;
            float startX = pixelPitch * (-(nPixelHolesX - 1) * 0.5 + rodChannel - 1) + (pixelGap + rodPitch) * 0.5; // Location of the first rod in this set

            // The first and last channels are smaller and contain one fewer rods
            if (rodChannel == 0 || rodChannel == nPixelHolesX){
                // Making this a bit flexible in case I find the dimensions are different from the drawings
                nRods = 5;
                rodPitch = strip_gap_edge / nRods;
                // The first edge has unique placement, but the last one can use the normal method
                if (rodChannel == 0){
                    startX = (-absorber_width + rodPitch) * 0.5;
                }
            }

            for (int rod = 0; rod < nRods; ++rod){
                id = m_zdcID->channel_id(m_side,m_module,ZdcIDType::ACTIVE, radGap*(nPixelHolesX+1) + rodChannel);
                sprintf(volName, "ZDC::Strip %s", id.getString().c_str());
                Module_Physical->add(new GeoNameTag(volName));
                Module_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                Module_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D((startX + rod * rodPitch) * Gaudi::Units::mm, 0.0, radGapz * Gaudi::Units::mm)));
                Module_Physical->add(new GeoAlignableTransform(GeoTrf::RotateX3D(90 * Gaudi::Units::deg)));
                Module_Physical->add(new GeoFullPhysVol(Strip_Logical));
            } // end vertical rod placement

            /*************************************************
             * Place pixel rods (if needed)
             **************************************************/
            int firstPixelLayer = 0;
            int lastPixelLayer = 0;
            float pixelX = firstPixelX + pixelPitch * rodChannel;
            if (nPixelMods == 0 || rodChannel == nPixelHolesX){ // Don't place any pixel rods or holes
                continue;
            }
            else if (nPixelMods == 1){ // Place pixel rods in layer 1-8, holes in the rest
                firstPixelLayer = 1;
                lastPixelLayer = 8;
            }
            else if (nPixelMods == 2){ // Place pixel rods in layer 0-9, holes in the rest
                firstPixelLayer = 0;
                lastPixelLayer = 9;
            }

            for (int pixelLayer = 0; pixelLayer <= nPixelHolesY; ++pixelLayer){
                float pixelY = firstPixelY + pixelPitch * pixelLayer;

                if (pixelLayer >= firstPixelLayer && pixelLayer <= lastPixelLayer){
                    // Place pixel rods in the radiator gaps
                    id = m_zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::PIXEL);
                    sprintf(volName, "ZDC::Pixel_Rad %s", id.getString().c_str());
                    Module_Physical->add(new GeoNameTag(volName));
                    Module_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                    Module_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(pixelX * Gaudi::Units::mm, pixelY * Gaudi::Units::mm, radGapz * Gaudi::Units::mm)));
                    Module_Physical->add(new GeoFullPhysVol(Pixel_Rad_Logical));

                    // Place a pixel rod in the absorber/housing
                    sprintf(volName, "ZDC::Pixel_Abs %s", id.getString().c_str());
                    pixelsPlacedIn->add(new GeoNameTag(volName));
                    pixelsPlacedIn->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                    pixelsPlacedIn->add(new GeoAlignableTransform(GeoTrf::Translate3D(pixelX * Gaudi::Units::mm, pixelY * Gaudi::Units::mm, absPixelZ * Gaudi::Units::mm)));
                    pixelsPlacedIn->add(new GeoFullPhysVol(Pixel_W_Logical));
                }
                else{ // Place a hole in the absorber/housing
                    id = m_zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::AIR);
                    sprintf(volName, "ZDC::Pixel_Hole %s", id.getString().c_str());
                    pixelsPlacedIn->add(new GeoNameTag(volName));
                    pixelsPlacedIn->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                    pixelsPlacedIn->add(new GeoAlignableTransform(GeoTrf::Translate3D(pixelX * Gaudi::Units::mm, pixelY * Gaudi::Units::mm, absPixelZ * Gaudi::Units::mm)));
                    pixelsPlacedIn->add(new GeoFullPhysVol(Pixel_Hole_Logical));
                }
            } // end loop over pixel y placement
        }     // end loop over pixel x placement
    }         // end loop over radiator gaps

    /*************************************************
     * Place module in steel case
     **************************************************/
    id = m_zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::AIR);
    sprintf(volName, "ZDC::Air_Mod %s", id.getString().c_str());
    Housing_Physical->add(new GeoNameTag(volName));
    Housing_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
    Housing_Physical->add(new GeoAlignableTransform(GeoTrf::TranslateZ3D((wallThicknessBack - wallThicknessFront) * Gaudi::Units::mm)));
    Housing_Physical->add(Module_Physical);

    // Place the steel case in the mother volume
    id = m_zdcID->channel_id(m_side, m_module, ZdcIDType::INACTIVE,ZdcIDVolChannel::HOUSING);
    sprintf(volName, "Zdc::ZDC_Mod %s", id.getString().c_str());
    mother->add(new GeoNameTag(volName));
    mother->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
    mother->add(trf);
    mother->add(Housing_Physical);

}