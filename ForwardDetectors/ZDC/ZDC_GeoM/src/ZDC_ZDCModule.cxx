/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

#include "ZdcIdentifier/ZdcID.h"
#include "AthenaKernel/getMessageSvc.h"
#include "CLHEP/Geometry/Transform3D.h"

ZDC_ZDCModule::ZDC_ZDCModule()
    : ZDC_ModuleBase(),
      m_modType(1)
{
}

ZDC_ZDCModule::ZDC_ZDCModule(const std::string& name, int side, int module, int modType)
    : ZDC_ModuleBase(name, side, module),
      m_modType(modType)
{
}

ZDC_ZDCModule::ZDC_ZDCModule(ZDC_ZDCModule *right, int side, int module) 
    : ZDC_ModuleBase((ZDC_ModuleBase*)right, side, module)
{
    m_modType = right->m_modType;
}


void ZDC_ZDCModule::create(GeoFullPhysVol* mother, StoredMaterialManager *materialManager, const ZdcID *zdcID){

    MsgStream LogStream(Athena::getMessageSvc(), "ZDC_ZDCModule::create");

    const GeoMaterial *OpAir    = materialManager->getMaterial("ZDC::opticalAir"   );
    const GeoMaterial *OpSilica = materialManager->getMaterial("ZDC::opticalSilica");
    const GeoMaterial *Silica   = materialManager->getMaterial("std::Quartz"       );
    const GeoMaterial *Tungsten = materialManager->getMaterial("ZDC::Tungsten"     );
    const GeoMaterial *Steel    = materialManager->getMaterial("ZDC::Steel"        );

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
    

    // These dimensions exist independent of the pixels (Though some still involve pixel dimensions for inter compatibility)
    const float radiator_gap_depth = 2.0;                                                             // Depth of the radiator gap
    const int   nRadGaps = 12;                                                                        // number of radiator gaps
    const float strip_diameter = 1.5;                                                                 // Diameter of the vertical quartz rods
    const float absorber_width = 89.57;                                                               // Width (x) of the tungsten absorber
    const float absorber_depth = 10.0;                                                                // Depth (z) of the tungsten absorber
    const float absorber_height = 180.0;                                                              // Height (y) of the tungsten absorber
    const float wallThicknessFront = 10.0;                                                            // Depth (z) of the steel case wall closest to the IP
    const float wallThicknessSide = 1.21;                                                             // Thickness of the side walls (x) of the module
    const float floorThickness = 1.21;                                                                // Thickness of the bottom of the module
    const float zPitch = radiator_gap_depth + absorber_depth;                                         // tungsten plate/quartz rod pitch in z
    const float strip_gap_center = pixelPitch - pixelGap;                                             // Width of the center channels for quartz rods
    const float strip_gap_edge = (absorber_width - pixelPitch * (nPixelHolesX - 1) - pixelGap) * 0.5; // Width of the edge channels for quartz rods
    const int   nRods_center_gap = 6;                                                                 // Number of rods in the center gaps
    const int   nRods_edge_gap = 5;                                                                   // Number of rods in the edge gaps
    const float housing_width = absorber_width + 2 * wallThicknessSide;                               // Width (x) of the steel module case
    const float housing_height = absorber_height + floorThickness;                                    // Height (y) of the steel module case


    // These dimensions are for the default module type 1, but are modified by the presence of pixels
    float zBackCavity = 0.0;        // extra air cavity for pixel rods routing to the readout 
    float wallThicknessBack = 10.0; // Depth (z) of the steel case wall furthest from the IP
    float rearCoverThickness = 0.0; // Thickness of additional rear cover for pixel modules
    int pixelStart = 0, pixelStop = 0; // Start and stop rows for pixel rods

    //Set the dimensions for module type 2 or 3
    switch (m_modType){
        case 1: //type 1 (no pixel)
            break;
        case 2: //type 2 (pixel)
            pixelStop = 9;
            wallThicknessBack = 8.0;
            rearCoverThickness = 1.21;
            zBackCavity = 28;
            break;
        case 3: //type 3 (Bigger back cavity)
            pixelStart = 1;
            pixelStop = 8;
            wallThicknessBack = 8.0;
            rearCoverThickness = 1.21;
            zBackCavity = 102;
            break;
        default:
            LogStream << MSG::ERROR << "Invalid module type: " << m_modType << endmsg;
            return;
    }


    // Finally calculate the total module z depth
    const float housing_depth = (nRadGaps - 1) * zPitch + radiator_gap_depth + wallThicknessFront + wallThicknessBack + zBackCavity + rearCoverThickness;

    /*************************************************
     * Create the volumes that are used independent of
     * module type
     **************************************************/
    GeoTube *Strip_Tube = new GeoTube (0.0            * Gaudi::Units::mm      , strip_diameter  * Gaudi::Units::mm * 0.5, absorber_height    * Gaudi::Units::mm * 0.5);
    GeoBox  *Steel_Box  = new GeoBox  (housing_width  * Gaudi::Units::mm * 0.5, housing_height  * Gaudi::Units::mm * 0.5, housing_depth      * Gaudi::Units::mm * 0.5);
    GeoBox  *RadGap_Box = new GeoBox  (absorber_width * Gaudi::Units::mm * 0.5, absorber_height * Gaudi::Units::mm * 0.5, radiator_gap_depth * Gaudi::Units::mm * 0.5);
    GeoBox  *Abs_Box    = new GeoBox  (absorber_width * Gaudi::Units::mm * 0.5, absorber_height * Gaudi::Units::mm * 0.5, absorber_depth     * Gaudi::Units::mm * 0.5);

    GeoLogVol *Strip_Logical     = new GeoLogVol("Strip_Logical"     , Strip_Tube , OpSilica );
    GeoLogVol *Steel_Logical     = new GeoLogVol("Steel_Logical"     , Steel_Box  , Steel    );
    GeoLogVol *RadGap_Logical    = new GeoLogVol("RadGap_Logical"    , RadGap_Box , OpAir    );
    GeoLogVol *Abs_Plate_Logical = new GeoLogVol("Abs_Plate_Logical" , Abs_Box    , Tungsten );

    GeoFullPhysVol *Housing_Physical = new GeoFullPhysVol(Steel_Logical    );
    GeoFullPhysVol *Abs_Plate        = new GeoFullPhysVol(Abs_Plate_Logical);
    GeoFullPhysVol *RadiatorGap      = new GeoFullPhysVol(RadGap_Logical   );

    // Surrogate ID and string to give the volumes dynamic references
    Identifier id;
    char volName[256];

    /*************************************************
     * Pre-assemble tungsten plates, radiator gaps, and
     * steel housing with pixel holes and rods if applicable
     **************************************************/
    if(m_modType > 1){

        // Z position of the inner wall that the pixel rods pass through
        const float inner_wall_z = -0.5 * housing_depth + wallThicknessFront + (nRadGaps - 1) * zPitch + radiator_gap_depth + 0.5 * wallThicknessBack ;

        GeoTube *Pixel_Rod_Abs      = new GeoTube ( 0.0           * Gaudi::Units::mm     , pixelRodDia     * Gaudi::Units::mm * 0.5 , absorber_depth     * Gaudi::Units::mm * 0.5 );
        GeoTube *Pixel_Rod_Rad      = new GeoTube ( 0.0           * Gaudi::Units::mm     , pixelRodDia     * Gaudi::Units::mm * 0.5 , radiator_gap_depth * Gaudi::Units::mm * 0.5 );
        GeoTube *Pixel_Rod_Housing  = new GeoTube ( 0.0           * Gaudi::Units::mm     , pixelRodDia     * Gaudi::Units::mm * 0.5 , wallThicknessBack  * Gaudi::Units::mm * 0.5 );
        GeoTube *Pixel_Hole_Abs     = new GeoTube ( 0.0           * Gaudi::Units::mm     , pixelHoleDia    * Gaudi::Units::mm * 0.5 , absorber_depth     * Gaudi::Units::mm * 0.5 );
        GeoTube *Pixel_Hole_Housing = new GeoTube ( 0.0           * Gaudi::Units::mm     , pixelHoleDia    * Gaudi::Units::mm * 0.5 , wallThicknessBack  * Gaudi::Units::mm * 0.5 );
        GeoBox  *Pixel_Routing_Box  = new GeoBox  (absorber_width * Gaudi::Units::mm* 0.5, absorber_height * Gaudi::Units::mm * 0.5 , zBackCavity        * Gaudi::Units::mm * 0.5 );

        GeoLogVol *Pixel_Abs_Logical        = new GeoLogVol("Pixel_Abs_Logical"        , Pixel_Rod_Abs      , Silica );
        GeoLogVol *Pixel_Rad_Logical        = new GeoLogVol("Pixel_Rad_Logical"        , Pixel_Rod_Rad      , Silica );
        GeoLogVol *Pixel_House_Logical      = new GeoLogVol("Pixel_House_Logical"      , Pixel_Rod_Housing  , Silica );
        GeoLogVol *Pixel_Hole_Abs_Logical   = new GeoLogVol("Pixel_Hole_Logical"       , Pixel_Hole_Abs     , OpAir  );
        GeoLogVol *Pixel_Hole_House_Logical = new GeoLogVol("Pixel_Hole_House_Logical" , Pixel_Hole_Housing , OpAir  );
        GeoLogVol *Pixel_Routing_Logical    = new GeoLogVol("Pixel_Routing_Logical"    , Pixel_Routing_Box  , OpAir  );


        /*************************************************
         * Place the cavity the pixel rods are routed in
         **************************************************/
        id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::AIR);
        sprintf(volName, "ZDC::Pixel_Routing %s", id.getString().c_str());
        Housing_Physical->add(new GeoNameTag(volName));
        Housing_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
        Housing_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(0.0, 0.5 * floorThickness * Gaudi::Units::mm, (housing_depth * 0.5 - rearCoverThickness - zBackCavity * 0.5) * Gaudi::Units::mm)));
        Housing_Physical->add(new GeoFullPhysVol(Pixel_Routing_Logical));

        /*************************************************
         * Make unpopulated and populated holes for the
         * tungsten plates
         **************************************************/
        GeoFullPhysVol *Pixel_Hole_Abs_Empty  = new GeoFullPhysVol(Pixel_Hole_Abs_Logical);
        
        GeoFullPhysVol *Pixel_Hole_Abs_Filled = new GeoFullPhysVol(Pixel_Hole_Abs_Logical);
        id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::PIXEL);
        sprintf(volName, "ZDC::Pixel %s", id.getString().c_str());
        Pixel_Hole_Abs_Filled->add(new GeoNameTag(volName));
        Pixel_Hole_Abs_Filled->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
        Pixel_Hole_Abs_Filled->add(new GeoFullPhysVol(Pixel_Abs_Logical));

        /*************************************************
         * Make unpopulated and populated holes for the
         * inner wall
         **************************************************/
        GeoFullPhysVol *Pixel_Hole_House_Empty  = new GeoFullPhysVol(Pixel_Hole_House_Logical);
        
        GeoFullPhysVol *Pixel_Hole_House_Filled = new GeoFullPhysVol(Pixel_Hole_House_Logical);
        id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::PIXEL);
        sprintf(volName, "ZDC::Pixel %s", id.getString().c_str());
        Pixel_Hole_House_Filled->add(new GeoNameTag(volName));
        Pixel_Hole_House_Filled->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
        Pixel_Hole_House_Filled->add(new GeoFullPhysVol(Pixel_House_Logical));

        /*************************************************
         * Make a pixel rod for the radiator gaps
         **************************************************/
        GeoFullPhysVol *Pixel_Rad = new GeoFullPhysVol(Pixel_Rad_Logical);
        
        // Populate the absorber, air gaps, and inner housing with the pixel rods and holes
        for (int rodChannel = 0; rodChannel < nPixelHolesX; ++rodChannel){
            float pixelX = firstPixelX + pixelPitch * rodChannel;

            for (int pixelLayer = 0; pixelLayer <= nPixelHolesY; ++pixelLayer){
                float pixelY = firstPixelY + pixelPitch * pixelLayer;

                if (pixelLayer >= pixelStart && pixelLayer <= pixelStop){//Populated holes
                    // Place pixel rods in the radiator gaps
                    id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::PIXEL);
                    sprintf(volName, "ZDC::Pixel_Rad %s", id.getString().c_str());
                    RadiatorGap->add(new GeoNameTag(volName));
                    RadiatorGap->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                    RadiatorGap->add(new GeoAlignableTransform(GeoTrf::Translate3D(pixelX * Gaudi::Units::mm, pixelY * Gaudi::Units::mm, 0.0 * Gaudi::Units::mm)));
                    RadiatorGap->add(Pixel_Rad);

                    // Place a pixel rod in the tungsten plate
                    sprintf(volName, "ZDC::Pixel_Abs %s", id.getString().c_str());
                    Abs_Plate->add(new GeoNameTag(volName));
                    Abs_Plate->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                    Abs_Plate->add(new GeoAlignableTransform(GeoTrf::Translate3D(pixelX * Gaudi::Units::mm, pixelY * Gaudi::Units::mm, 0.0 * Gaudi::Units::mm)));
                    Abs_Plate->add(Pixel_Hole_Abs_Filled);

                    // Place a pixel rod in the inner housing
                    id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::AIR);
                    sprintf(volName, "ZDC::Pixel_Housing %s", id.getString().c_str());
                    Housing_Physical->add(new GeoNameTag(volName));
                    Housing_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                    Housing_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(pixelX * Gaudi::Units::mm, 0.5 * floorThickness + pixelY * Gaudi::Units::mm, inner_wall_z * Gaudi::Units::mm)));
                    Housing_Physical->add(Pixel_Hole_House_Filled);
                
                }else{// Unpopulated holes
                    id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::AIR);
                    sprintf(volName, "ZDC::Pixel_Hole %s", id.getString().c_str());

                    // Place an unpopulated hole in the tungsten plate
                    Abs_Plate->add(new GeoNameTag(volName));
                    Abs_Plate->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                    Abs_Plate->add(new GeoAlignableTransform(GeoTrf::Translate3D(pixelX * Gaudi::Units::mm, pixelY * Gaudi::Units::mm, 0.0 * Gaudi::Units::mm)));
                    Abs_Plate->add(Pixel_Hole_Abs_Empty);

                    // Place an unpopulated hole in the housing
                    Housing_Physical->add(new GeoNameTag(volName));
                    Housing_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                    Housing_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(pixelX * Gaudi::Units::mm, 0.5 * floorThickness + pixelY * Gaudi::Units::mm, inner_wall_z * Gaudi::Units::mm)));
                    Housing_Physical->add(Pixel_Hole_House_Empty);
                }
            } // end loop over pixel y placement
        }// end loop over pixel x placement
    }// end if pixel module



    /*************************************************
     * Place the pre-assembled tungsten plates and
     * radiator gaps in the steel housing. Also,
     * add the vertical quartz rods to the radiator gaps
     **************************************************/
    for (int radGap = 0; radGap < nRadGaps; ++radGap){
        float zRadGap = -housing_depth * 0.5 + wallThicknessFront + radiator_gap_depth * 0.5 + zPitch * radGap;
        float zAbsorber = zRadGap + radiator_gap_depth * 0.5 + absorber_depth * 0.5;

        /*************************************************
         * Place tungsten plates
         **************************************************/
        if (radGap != nRadGaps - 1){ // Don't place a tungsten plate after the last radiator gap
            id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::TUNGSTEN);
            sprintf(volName, "ZDC::W_Mod %s", id.getString().c_str());
            Housing_Physical->add(new GeoNameTag(volName));
            Housing_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
            Housing_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(0.0,  0.5 * floorThickness * Gaudi::Units::mm, zAbsorber * Gaudi::Units::mm)));
            Housing_Physical->add(Abs_Plate);
        }

        /*************************************************
         * Place vertical quartz rods
         **************************************************/
        
        // Clone the air gap so we can place rods in the clone
        // Unless it's the last volume, then we can just use the original
        // so GeoModel can take ownership of it
        GeoFullPhysVol *thisRadGap = (radGap == nRadGaps - 1) ? RadiatorGap : RadiatorGap->clone(false);

        for (int rodChannel = 0; rodChannel < nPixelHolesX + 1; ++rodChannel){
            //Default values are for central gaps
            int nRods = nRods_center_gap;
            float rodPitch = strip_gap_center / nRods;
            float startX = pixelPitch * (-(nPixelHolesX - 1) * 0.5 + rodChannel - 1) + (pixelGap + rodPitch) * 0.5; // Location of the first rod in this set

            // The first and last channels are smaller and contain one fewer rods
            if (rodChannel == 0 || rodChannel == nPixelHolesX){
                // Making this a bit flexible in case I find the dimensions are different from the drawings
                nRods = nRods_edge_gap;
                rodPitch = strip_gap_edge / nRods;
                // The first edge has unique placement, but the last one can use the normal method
                if (rodChannel == 0){
                    startX = (-absorber_width + rodPitch) * 0.5;
                }
            }

            id = zdcID->channel_id(m_side,m_module,ZdcIDType::ACTIVE, radGap*(nPixelHolesX+1) + rodChannel);
            sprintf(volName, "ZDC::Strip %s", id.getString().c_str());
            for (int rod = 0; rod < nRods; ++rod){
                thisRadGap->add(new GeoNameTag(volName));
                thisRadGap->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
                thisRadGap->add(new GeoAlignableTransform(GeoTrf::TranslateX3D((startX + rod * rodPitch) * Gaudi::Units::mm)));
                thisRadGap->add(new GeoAlignableTransform(GeoTrf::RotateX3D(90 * Gaudi::Units::deg)));
                thisRadGap->add(new GeoFullPhysVol(Strip_Logical));
            } // end vertical rod placement
        } // end loop over rod channels

        /*************************************************
         * Place the radiator gap
         **************************************************/
        id = zdcID->channel_id(m_side,m_module,ZdcIDType::INACTIVE,ZdcIDVolChannel::AIR);
        sprintf(volName, "ZDC::Rad_Gap %s", id.getString().c_str());
        Housing_Physical->add(new GeoNameTag(volName));
        Housing_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
        Housing_Physical->add(new GeoAlignableTransform(GeoTrf::Translate3D(0.0, 0.5 * floorThickness * Gaudi::Units::mm, zRadGap * Gaudi::Units::mm)));
        Housing_Physical->add(thisRadGap);
    }// end loop over radiator gaps

    // Place the steel case in the mother volume
    id = zdcID->channel_id(m_side, m_module, ZdcIDType::INACTIVE,ZdcIDVolChannel::HOUSING);
    sprintf(volName, "Zdc::ZDC_Mod %s", id.getString().c_str());
    mother->add(new GeoNameTag(volName));
    mother->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
    mother->add(new GeoAlignableTransform( m_trf ));
    mother->add(Housing_Physical);

}