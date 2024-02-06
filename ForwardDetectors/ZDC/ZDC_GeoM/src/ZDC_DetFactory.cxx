/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_DetFactory.h"
#include "ZDC_ZDCModule.h"
#include "ZDC_RPDModule.h"
#include "ZDC_BRANModule.h"

#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/Units.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/SystemOfUnits.h"


#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelUtilities/GeoExtendedMaterial.h"
#include "AthenaKernel/getMessageSvc.h"
#include "CLHEP/Geometry/Transform3D.h"

// Author Chad Lantz
// chad.stephen.lantz@cern.ch

// This is the ZDC Geomodel description :: see some details at
// https://atlasop.cern.ch/atlas-point1/twiki/pub/Main/ZDCOperationManualShifter/zdc_layout.png

// Each mod is located at centered at 141.580 m
// I have assumed that each module is 154mm which is 4 mm longer than the ones shown in the above webpage
// thus my modules are 13.4cm tungsten +1cm steel on either side

// I have assumed that the cavity is 1016mm (4*4=16mm larger than the one in the above webpage)
// Ionization chamber material is air currently
// Note: C side (side 0) EM module has pixels, A side (side 1) doesn't

ZDC_DetFactory::ZDC_DetFactory(StoreGateSvc *detStore) :
    AthMessaging("ZDC_DetFactory"),
    m_detectorManager(NULL),
    m_detectorStore(detStore)
{
    if (m_detectorStore->retrieve( m_zdcID ).isFailure() ) {
        MsgStream LogStream(Athena::getMessageSvc(), "ZDC_DetectorFactory::ZDC_DetFactory");
        LogStream << MSG::ERROR << "execute: Could not retrieve ZdcID object from the detector store" << endmsg;
    }
}

ZDC_DetFactory::~ZDC_DetFactory() {}

void ZDC_DetFactory::initializePbPb2015(){
    m_RPDs_On = false; //Flag for both RPD modules
    m_zdcOn = {{true, true, true, true}, //If the given ZDC is on
               {true, true, true, true}};
    m_zdcPos = {{-397.0, -27.0, 153.0, 303.0}, //Positions of the ZDC modules
                {-397.0, -27.0, 153.0, 303.0}};
    m_zdcPixelStart_Stop = {{{1,8}, {0,9}, {0,0}, {0,0}}, //Pixel start and stop layers for each ZDC
                            {{0,0}, {0,9}, {0,0}, {0,0}}};
}

void ZDC_DetFactory::initializePbPb2023(){
    m_RPDs_On = true; //Flag for both RPD modules
    m_BRANs_On = true; //Flag for both BRAN modules
    m_zdcOn = {{true, true, true, true}, //If the given ZDC is on
               {true, true, true, true}};
    m_zdcPos = {{-394.0,  -15.0, 170.0, 325.0}, //Positions of the ZDC modules
                {-272.75,   6.0, 191.0, 346.0}};
    m_zdcPixelStart_Stop = {{{1,8}, {0,9}, {0,0}, {0,0}}, //Pixel start and stop layers for each ZDC
                            {{0,0}, {0,9}, {0,0}, {0,0}}};
    m_rpdPos = {-204.625,-183.375}; //Positions of the RPD modules
    m_branPos = {-142.5,-121.5};
}

void ZDC_DetFactory::create(GeoPhysVol *world)
{
    m_detectorManager = new ZDC_DetManager();
    MsgStream LogStream(Athena::getMessageSvc(), "ZDC_DetectorFactory::create");

    StoredMaterialManager *theMaterialManager;
    if (StatusCode::SUCCESS != m_detectorStore->retrieve(theMaterialManager, "MATERIALS")) {
        MsgStream LogStream(Athena::getMessageSvc(), "ZDC_DetectorFactory::create");
        LogStream << MSG::ERROR << "execute: Could not retrieve StoredMaterialManager object from the detector store" << endmsg;
        return;
    }

    buildMaterials(theMaterialManager);

    //Create the TAN/TAXN slot
    const GeoMaterial *Air = theMaterialManager->getMaterial("std::Air");
    GeoBox *Envelope_Box = new GeoBox(91 * Gaudi::Units::mm * 0.5, 181 * Gaudi::Units::mm * 0.5, 94.3 * Gaudi::Units::cm * 0.5);
    GeoLogVol *Envelope_Logical = new GeoLogVol("Envelope_Logical", Envelope_Box, Air);

    Identifier id;

    char volName[256];
    for(int side : {0, 1}){
        int sideSign = (side == 0) ? -1 : 1;
        GeoFullPhysVol *Envelope_Physical = new GeoFullPhysVol(Envelope_Logical);

        /*************************************************
         * Place ZDC modules
         **************************************************/
        for(int module = 0; module < 4; ++module){
            if(!m_zdcOn[side][module]) continue;
            id = m_zdcID->channel_id(sideSign,module,ZdcIDType::INACTIVE,ZdcIDVolChannel::HOUSING);
            ZDC_ZDCModule *zdcMod = new ZDC_ZDCModule(m_detectorStore, sideSign ,module, m_zdcID, m_zdcPixelStart_Stop[side][module].first, m_zdcPixelStart_Stop[side][module].second);
            sprintf(volName, "Zdc::Steel_Mod %s", id.getString().c_str());
            Envelope_Physical->add(new GeoNameTag(volName));
            Envelope_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
            Envelope_Physical->add(new GeoAlignableTransform(GeoTrf::TranslateZ3D(m_zdcPos[side][module] * Gaudi::Units::mm)));
            Envelope_Physical->add(zdcMod->create());
        }

        /*************************************************
         * Place RPD
         **************************************************/
        if(m_RPDs_On){
            id = m_zdcID->channel_id(sideSign, 4, ZdcIDType::INACTIVE,ZdcIDVolChannel::HOUSING);
            ZDC_RPDModule *rpdMod = new ZDC_RPDModule(m_detectorStore, sideSign, 4, m_zdcID);
            sprintf(volName, "Zdc::RPD_Mod %s", id.getString().c_str());
            Envelope_Physical->add(new GeoNameTag(volName));
            Envelope_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
            Envelope_Physical->add(new GeoAlignableTransform(GeoTrf::TranslateZ3D(m_rpdPos[side] * Gaudi::Units::mm)));
            Envelope_Physical->add(rpdMod->create());
        }

        /*************************************************
         * Place BRAN
         **************************************************/
        if(m_BRANs_On){
            id = m_zdcID->channel_id(sideSign, 5, ZdcIDType::INACTIVE,ZdcIDVolChannel::HOUSING);
            ZDC_BRANModule *branMod = new ZDC_BRANModule(m_detectorStore, sideSign, 5, m_zdcID);
            sprintf(volName, "Zdc::BRAN_Mod %s", id.getString().c_str());
            Envelope_Physical->add(new GeoNameTag(volName));
            Envelope_Physical->add(new GeoIdentifierTag(id.get_identifier32().get_compact()));
            Envelope_Physical->add(new GeoAlignableTransform(GeoTrf::TranslateZ3D(m_branPos[side] * Gaudi::Units::mm)));
            Envelope_Physical->add(branMod->create());
        }

        /*************************************************
         * Place TAN/TAXN slot
         **************************************************/
        sprintf(volName, "Zdc::ZDC_Air_Envelope %c", (side == 0) ? 'C' : 'A');
        world->add(new GeoNameTag(volName));
        world->add(new GeoIdentifierTag(m_zdcID->channel_id(sideSign, 0, ZdcIDType::INACTIVE,ZdcIDVolChannel::AIR).get_identifier32().get_compact()));
        world->add(new GeoAlignableTransform(GeoTrf::TranslateZ3D(sideSign*141.580 * CLHEP::m)));
        if(side == 0) world->add(new GeoAlignableTransform(GeoTrf::RotateY3D(180 * Gaudi::Units::deg)));
        world->add(Envelope_Physical);

        m_detectorManager->addTreeTop(Envelope_Physical);
    }
}

void ZDC_DetFactory::buildMaterials(StoredMaterialManager *materialManager){

    const GeoElement *Oxygen   = materialManager->getElement("Oxygen");
    const GeoElement *Silicon  = materialManager->getElement("Silicon");
    const GeoElement *Hydrogen = materialManager->getElement("Hydrogen");
    const GeoElement *Nitrogen = materialManager->getElement("Nitrogen");
    const GeoElement *Carbon   = materialManager->getElement("Carbon");
    const GeoElement *Argon    = materialManager->getElement("Argon");
    const GeoElement *Tung     = materialManager->getElement("Wolfram");
    const GeoElement *Iron     = materialManager->getElement("Iron");
    const GeoElement *Nickel   = materialManager->getElement("Nickel");

    const int nEntries = 50; // Number of data points in each array
    double eV = Gaudi::Units::eV;
    double cm = Gaudi::Units::cm;

    //Evenly distributed photon energy bins for the coming arrays of material properties. Range is selected for a PMT which is sensitive from 300-650nm wavelength
    double photonEnergy[nEntries] = {0};
    double minEnergy = 1.90769 * eV; double maxEnergy = 4.08882 * eV;
    double step = (maxEnergy-minEnergy)/nEntries;
    for(int i=0; i<nEntries; ++i){
       photonEnergy[i] = minEnergy + i*step;
    }

    // Optical Air density and composition obtained from
    // From https://physics.nist.gov/cgi-bin/Star/compos.pl?matno=104
    GeoExtendedMaterial *OpAir = new GeoExtendedMaterial("ZDC::opticalAir", 1.2e-3 * GeoModelKernelUnits::g / Gaudi::Units::cm3, stateGas, Gaudi::Units::STP_Temperature);
    OpAir->add(Nitrogen, 0.755);
    OpAir->add(Oxygen  , 0.232);
    OpAir->add(Argon   , 0.013);

    // The air in the modules must have refractive index defined for total internal reflection to work
    // From NIST https://emtoolbox.nist.gov/wavelength/documentation.asp
    double RefractiveIndexAir[nEntries];
    for(int i=0; i<nEntries; ++i)
        RefractiveIndexAir[i] = 1.000271800;

    GeoMaterialPropertiesTable *airMPT = new GeoMaterialPropertiesTable();
    airMPT->AddProperty("RINDEX", photonEnergy, RefractiveIndexAir, nEntries);
    OpAir->SetMaterialPropertiesTable(airMPT);
    OpAir->lock();
    materialManager->addMaterial("ZDC", OpAir);

    // RPD fiber core and ZDC rod material
    // Composition from https://physics.nist.gov/cgi-bin/Star/compos.pl?matno=245
    GeoExtendedMaterial *OpSilicaCore = new GeoExtendedMaterial("ZDC::opticalSilica", 2.320 * GeoModelKernelUnits::g / Gaudi::Units::cm3, stateSolid, Gaudi::Units::STP_Temperature);
    OpSilicaCore->add(Silicon, 0.467);
    OpSilicaCore->add(Oxygen, 0.533);

    // Refractive index of fused silica obtained from 
    // https://www.heraeus.com/media/media/hca/doc_hca/products_and_solutions_8/optics/Data_and_Properties_Optics_fused_silica_EN.pdf
    double silica_RIND[] = {1.45656, 1.45694, 1.45737, 1.45781, 1.45825, 1.4587 , 1.45914, 1.45959, 1.46003, 1.46048,
                            1.46095, 1.46145, 1.46194, 1.46242, 1.46289, 1.4634 , 1.46394, 1.46448, 1.46502, 1.46556,
                            1.46608, 1.46666, 1.46725, 1.46784, 1.46843, 1.46902, 1.46961, 1.47024, 1.4709 , 1.47155,
                            1.4722 , 1.47288, 1.47356, 1.47425, 1.47494, 1.47566, 1.4764 , 1.47715, 1.4779 , 1.47864,
                            1.47935, 1.48014, 1.48093, 1.48172, 1.48254, 1.48339, 1.48424, 1.4851 , 1.48598, 1.48689};

    // Absorption length index of fused silica derrived from 
    // https://www.heraeus.com/media/media/hca/doc_hca/products_and_solutions_8/optics/Data_and_Properties_Optics_fused_silica_EN.pdf
    double silica_ABSL[nEntries];
    for(int i=0; i<nEntries-2; ++i)
        silica_ABSL[i] = 302.163 * cm;
    silica_ABSL[nEntries - 1] = silica_ABSL[nEntries - 2] = 204.542 * cm;

    GeoMaterialPropertiesTable *silicaCoreMPT = new GeoMaterialPropertiesTable();
    silicaCoreMPT->AddProperty("RINDEX"   , photonEnergy, silica_RIND, nEntries); // index of refraction
    silicaCoreMPT->AddProperty("ABSLENGTH", photonEnergy, silica_ABSL, nEntries); // absorption length
    OpSilicaCore->SetMaterialPropertiesTable(silicaCoreMPT);
    OpSilicaCore->lock();
    materialManager->addMaterial("ZDC", OpSilicaCore);

    // RPD fiber cladding
    // Composition from https://physics.nist.gov/cgi-bin/Star/compos.pl?matno=245
    GeoExtendedMaterial *OpSilicaClad = new GeoExtendedMaterial("ZDC::opticalSilicaClad", 2.320 * GeoModelKernelUnits::g / Gaudi::Units::cm3, stateSolid, Gaudi::Units::STP_Temperature);
    OpSilicaClad->add(Silicon, 0.467);
    OpSilicaClad->add(Oxygen, 0.533);

    // Numerical aperture is given by data sheet as 0.22 and NA = sqrt( n1^2 - n2^2 ), so n2 = sqrt( n1^2 - NA^2 ) where n1 is silica_RIND
    // https://www.content.molex.com/dxdam/literature/987650-8936.pdf
    double silica_clad_RIND[] = {1.43985, 1.44023, 1.44067, 1.44112, 1.44156, 1.44201, 1.44246, 1.44291, 1.44336, 1.44381,
                                 1.44429, 1.4448 , 1.44529, 1.44577, 1.44625, 1.44677, 1.44731, 1.44786, 1.44841, 1.44895,
                                 1.44948, 1.45007, 1.45067, 1.45126, 1.45186, 1.45245, 1.45305, 1.45369, 1.45435, 1.45501,
                                 1.45567, 1.45635, 1.45705, 1.45774, 1.45844, 1.45916, 1.45992, 1.46067, 1.46143, 1.46219,
                                 1.4629 , 1.4637 , 1.46449, 1.46529, 1.46612, 1.46698, 1.46785, 1.46871, 1.4696 , 1.47052};

    // Silica cladding
    GeoMaterialPropertiesTable *silicaCladMPT = new GeoMaterialPropertiesTable();
    silicaCladMPT->AddProperty("RINDEX"   , photonEnergy, silica_clad_RIND, nEntries); // index of refraction
    silicaCladMPT->AddProperty("ABSLENGTH", photonEnergy, silica_ABSL     , nEntries); // absorption length
    OpSilicaClad->SetMaterialPropertiesTable(silicaCladMPT);
    OpSilicaClad->lock();
    materialManager->addMaterial("ZDC", OpSilicaClad);

    // Kapton fiber optic buffer material
    // Composition from https://physics.nist.gov/cgi-bin/Star/compos.pl?matno=179
    GeoExtendedMaterial *OpKapton = new GeoExtendedMaterial("ZDC::opticalKapton", 1.42 * GeoModelKernelUnits::g / Gaudi::Units::cm3, stateSolid, Gaudi::Units::STP_Temperature);
    OpKapton->add(Hydrogen, 0.026362);
    OpKapton->add(Carbon  , 0.691133);
    OpKapton->add(Nitrogen, 0.073270);
    OpKapton->add(Oxygen  , 0.209235);

    // Refractive index obtained from
    // https://engineering.case.edu/centers/sdle/sites/engineering.case.edu.centers.sdle/files/optical_properties_of_materials.pdf
    double kapton_RIND[] = {1.7095 , 1.7111 , 1.7143 , 1.7191, 1.7207 , 1.7255 , 1.7271 , 1.73157, 1.7351 , 1.7383 ,
                            1.7416 , 1.7464 , 1.74978, 1.7545, 1.7593 , 1.766  , 1.7692 , 1.7758 , 1.78179, 1.79009,
                            1.794  , 1.80245, 1.8074 , 1.8157, 1.82184, 1.82659, 1.8344 , 1.84222, 1.8514 , 1.8584 ,
                            1.86392, 1.8723 , 1.88251, 1.8959, 1.90567, 1.92604, 1.93911, 1.95036, 1.96867, 1.97804,
                            1.9905 , 1.99755, 2.00821, 2.0146, 2.03435, 2.05705, 2.08078, 2.10021, 2.12912, 2.14333};

    // Reflectivity obtained from
    // https://amostech.com/TechnicalPapers/2018/Poster/Bengtson.pdf
    double kapton_REFL[] = {0.502195  , 0.473894  , 0.446164  , 0.413816  , 0.375095 , 0.336845  , 0.293879  , 0.239299  , 0.200573  , 0.141596  ,
                            0.0949924 , 0.0590249 , 0.0353952 , 0.0206475 , 0.01305  , 0.00915075, 0.00722501, 0.00551299, 0.00552271, 0.00553177,
                            0.00554062, 0.00554942, 0.00555642, 0.00556579, 0.0083157, 0.011944  , 0.0172255 , 0.0225071 , 0.0277887 , 0.0330702 ,
                            0.0383518 , 0.0436334 , 0.0489149 , 0.0541965 , 0.0594781, 0.0647597 , 0.0700412 , 0.0753228 , 0.0806044 , 0.0858859 ,
                            0.0911675 , 0.0964491 , 0.101731  , 0.107012  , 0.112294 , 0.117575  , 0.122857  , 0.128139  , 0.13342   , 0.138702  };

    // Absorption length obtained from
    // https://pubs.rsc.org/fa/content/articlehtml/2018/ra/c7ra12101f
    double kapton_ABSL[] = {0.00867389  * cm, 0.00842316  * cm, 0.00818715  * cm, 0.00798542  * cm, 0.00774517  * cm, 0.00751684  * cm, 0.00729959  * cm, 0.00709258  * cm, 0.00685686  * cm, 0.0066337   * cm,
                            0.00642212  * cm, 0.00616231  * cm, 0.00587855  * cm, 0.00561968  * cm, 0.00541849  * cm, 0.0052337   * cm, 0.00504545  * cm, 0.00487671  * cm, 0.00474623  * cm, 0.00461459  * cm,
                            0.00449314  * cm, 0.00437628  * cm, 0.0042637   * cm, 0.00413695  * cm, 0.00401798  * cm, 0.00382827  * cm, 0.003625    * cm, 0.00335813  * cm, 0.00303474  * cm, 0.00264672  * cm,
                            0.00226016  * cm, 0.00185863  * cm, 0.00146109  * cm, 0.00116967  * cm, 0.000901973 * cm, 0.000721492 * cm, 0.000559526 * cm, 0.000463349 * cm, 0.00034795  * cm, 0.000317447 * cm,
                            0.000317447 * cm, 0.000317447 * cm, 0.000317447 * cm, 0.000317447 * cm, 0.000317447 * cm, 0.000317447 * cm, 0.000317447 * cm, 0.000317447 * cm, 0.000317447 * cm, 0.000317447 * cm};

    // Kapton
    GeoMaterialPropertiesTable *kaptonMPT = new GeoMaterialPropertiesTable();
    kaptonMPT->AddProperty("RINDEX"      , photonEnergy, kapton_RIND, nEntries);
    kaptonMPT->AddProperty("ABSLENGTH"   , photonEnergy, kapton_ABSL, nEntries);
    kaptonMPT->AddProperty("REFLECTIVITY", photonEnergy, kapton_REFL, nEntries);
    OpKapton->SetMaterialPropertiesTable(kaptonMPT);
    OpKapton->lock();
    materialManager->addMaterial("ZDC", OpKapton);

    // Absorber composition:  savannah.cern.ch/task/download.php?file_id=22925
    GeoMaterial *Tungsten = new GeoMaterial("ZDC::Tungsten", 18.155 * GeoModelKernelUnits::g / Gaudi::Units::cm3);
    Tungsten->add(Tung  , 0.948);
    Tungsten->add(Nickel, 0.037);
    Tungsten->add(Iron  , 0.015);
    Tungsten->lock();
    materialManager->addMaterial("ZDC", Tungsten);

    // ZDC housing material
    GeoMaterial *Steel = new GeoMaterial("ZDC::Steel", 7.9 * GeoModelKernelUnits::g / Gaudi::Units::cm3);
    Steel->add(Iron  , 0.98);
    Steel->add(Carbon, 0.02);
    Steel->lock();
    materialManager->addMaterial("ZDC", Steel);

}

const ZDC_DetManager *ZDC_DetFactory::getDetectorManager() const { return m_detectorManager; }
