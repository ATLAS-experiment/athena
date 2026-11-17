/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNGEOMTOOLS_PIXELGEOMETRYMANAGERTOOL_H
#define INDETALIGNGEOMTOOLS_PIXELGEOMETRYMANAGERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "AthContainers/DataVector.h"

#include "TrkAlignInterfaces/IGeometryManagerTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"

#include "TrkAlignEvent/AlignModuleList.h"
#include "Gaudi/Property.h"

/**
  @file PixelGeometryManagerTool.h
  @class PixelGeometryManagerTool

  @brief Tool for setting up Pixel alignment geometry.

  @author Vicente Lacuesta <Vicente.Lacuesta.Miquel@cern.ch>
  @author Daniel Kollar <daniel.kollar@cern.ch>
  @date 28/08/2009
*/

namespace InDetDD
{
  class PixelDetectorManager;
}

class PixelID;

namespace Trk
{
  class AlignPar;
  class AlignModule;
}

class IRDBAccessSvc;
class IGeoModelSvc;

namespace InDet
{
  class PixelGeometryManagerTool : virtual public Trk::IGeometryManagerTool, public AthAlgTool
  {

  public:
    PixelGeometryManagerTool(const std::string& type, const std::string& name, const IInterface * parent);
    virtual ~PixelGeometryManagerTool();

    StatusCode initialize();
    StatusCode finalize();

    /** read the geometry
     * Method is called from the main AlignAlg to build the geometry
     * based on the requested level of alignment. It calls the other
     * build() methods based on that. */
    int ReadGeometry(int solveLevel);

    // the following two methods need to be overloaded but we don't need them (for the moment)
    void PrintModPositions() {};
    void StoreModPositions(std::string) {};

    /** builds geometry for Silicon alignment */
    void buildGeometry();

    /** adds alignment parameters for the module
     * checks for active parameters and calls setSigmas() */
    void addModuleParameters(Trk::AlignModule * module, DataVector< DataVector<Trk::AlignPar> > * allFullModPars, DataVector< DataVector<Trk::AlignPar> > * allActiveModPars);

    /** check whether the alignment level is correct */
    bool checkAlignLevel();

    /** check whether the alignment level for the barrel is correct */
    bool checkAlignLevelBarrel();

    /** check whether the alignment level for the endcaps is correct */
    bool checkAlignLevelEndcaps();

    void setAlignLevel(int level);

   /** get sipeta value from geometry DB */
    int getNoSkipEtaValueFromGeometry() const;


  private:
      
    /** eta correction value needed when IBL is present*/
    int m_etaCorrection{};
    
    /** creates L1 AlignModules for Pixel : full Pixel*/
    void buildL1();

    /** creates L11 IBL as 1 module and old-pixel as 1 module **/
    void buildL11();
    
    /** creates L12 AlignModules for Pixel Barrel : whole barrel as 1 module */
    void buildL12Barrel();

    /** creates L12 AlignModules for Pixel Barrel : 2 endcaps */
    void buildL12Endcaps();

    /** creates L15 AlignModules for Pixel Barrel : 2 half-shells */
    void buildL15Barrel();

    /** creates L16 AlignModules for Pixel Barrel :  IBL staves , remainder of barrel 1 Module*/
    void buildL16Barrel();

    /** creates L2 AlignModules for Pixel Barrel : 3 layers*/
    void buildL2Barrel();

    /** creates L2 AlignModules for Pixel Endcaps : 2 x 3 disks */
    void buildL2Endcaps();

    /** creates L22 AlignModules for Pixel Barrel : 6 half-layers */
    void buildL22Barrel();

    /** creates L26 AlignModules for Pixel Barrel : IBL staves remainder at L2 */
    void buildL26Barrel();

    /** creates L27 AlignModules for Pixel Barrel : 112 staves (ladders) */
    void buildL27Barrel();

    /** creates L3 AlignModules for Pixel Barrel : module (wafer) level */
    void buildL3Barrel();

    /** creates L3 AlignModules for Pixel Endcaps : module (wafer) level */
    void buildL3Endcaps();
    
    /** creates L1 AlignModules for Pixel DBM : 2 endcaps */
    void buildL1DBM();

    /** creates L2 AlignModules for Pixel DBM : 8 telescopes */
    void buildL2DBM();
    
    /** creates L2 AlignModules for Pixel DBM : 24 modules */
    void buildL3DBM();
    
    /** sets sigmas for modules */
    void setSigmas(Trk::AlignModule * mod, DataVector<Trk::AlignPar> * modPars);

    /** check wheather module is selected for module pointer */
    bool moduleSelected(Trk::AlignModule * mod);

    /** check wheather module is selected for module id */
    bool moduleSelected(unsigned long long modId);

    /** print basic geometry info to screen */
    void dumpGeometry();

    /** Checks whether the given phi module in a given barrel layer belongs to top or bottom half */
    bool isTopHalf(unsigned int phi, unsigned int layer) const;

    void printTransform(const Amg::Transform3D & tr) const;

    const InDetDD::PixelDetectorManager * m_detManager{nullptr}; //!< pointer to PIX detector manager

    const PixelID * m_idHelper{nullptr}; //!< pointer to PIX detector manager

    PublicToolHandle<Trk::IAlignModuleTool> m_alignModuleTool{this, "AlignModuleTool", "Trk::AlignModuleTool/AlignModuleTool"};  //!< pointer to AlignModuleTool

    Trk::AlignModuleList m_alignModuleList;
    Trk::AlignModuleList m_idHashToAlignModuleMap;

    std::vector<Trk::AlignModuleList *> m_idHashToAlignModuleMaps{std::vector<Trk::AlignModuleList *>(Trk::AlignModule::NDetectorTypes, nullptr)}; 

    DataVector< DataVector<Trk::AlignPar> > * m_alignParList{nullptr};
    DataVector< DataVector<Trk::AlignPar> > * m_fullAlignParList{nullptr};

    ServiceHandle< IRDBAccessSvc > m_rdbAccessSvc;
    ServiceHandle< IGeoModelSvc > m_geoModelSvc;

    Gaudi::Property<int> m_alignLevelProp{this, "AlignmentLevel", 1}; //!< property, copied to m_alignLevel in initialize()
    Gaudi::Property<int> m_alignLevelBarrelProp{this, "AlignmentLevelBarrel", -1}; //!< property, copied to m_alignLevelBarrel in initialize()
    Gaudi::Property<int> m_alignLevelEndcapsProp{this, "AlignmentLevelEndcaps", -1}; //!< property, copied to m_alignLevelEndcaps in initialize()
    Gaudi::Property<bool> m_doModuleSelection{this, "doModuleSelection", false};
    Gaudi::Property<std::vector<unsigned long long>> m_moduleSelection{this, "ModuleSelection", {}};

    Gaudi::Property<bool> m_alignBarrel{this, "AlignBarrel", true};
    Gaudi::Property<bool> m_alignEndcaps{this, "AlignEndcaps", true};

    Gaudi::Property<bool> m_alignBarrelX{this, "AlignBarrelX", true};
    Gaudi::Property<bool> m_alignBarrelY{this, "AlignBarrelY", true};
    Gaudi::Property<bool> m_alignBarrelZ{this, "AlignBarrelZ", true};
    Gaudi::Property<bool> m_alignBarrelRotX{this, "AlignBarrelRotX", true};
    Gaudi::Property<bool> m_alignBarrelRotY{this, "AlignBarrelRotY", true};
    Gaudi::Property<bool> m_alignBarrelRotZ{this, "AlignBarrelRotZ", true};
    Gaudi::Property<bool> m_alignBarrelBowX{this, "AlignBarrelBowX", false};


    Gaudi::Property<bool> m_alignEndcapX{this, "AlignEndcapX", true};
    Gaudi::Property<bool> m_alignEndcapY{this, "AlignEndcapY", true};
    Gaudi::Property<bool> m_alignEndcapZ{this, "AlignEndcapZ", true};
    Gaudi::Property<bool> m_alignEndcapRotX{this, "AlignEndcapRotX", true};
    Gaudi::Property<bool> m_alignEndcapRotY{this, "AlignEndcapRotY", true};
    Gaudi::Property<bool> m_alignEndcapRotZ{this, "AlignEndcapRotZ", true};

    Gaudi::Property<double> m_sigmaBarrelX{this, "SetSigmaBarrelX", 1.};
    Gaudi::Property<double> m_sigmaBarrelY{this, "SetSigmaBarrelY", 1.};
    Gaudi::Property<double> m_sigmaBarrelZ{this, "SetSigmaBarrelZ", 1.};
    Gaudi::Property<double> m_sigmaBarrelRotX{this, "SetSigmaBarrelRotX", 0.001};
    Gaudi::Property<double> m_sigmaBarrelRotY{this, "SetSigmaBarrelRotY", 0.001};
    Gaudi::Property<double> m_sigmaBarrelRotZ{this, "SetSigmaBarrelRotZ", 0.001};
    Gaudi::Property<double> m_sigmaBarrelBowX{this, "SetSigmaBarrelBowX", 1};

    Gaudi::Property<double> m_sigmaEndcapX{this, "SetSigmaEndcapX", 1.};
    Gaudi::Property<double> m_sigmaEndcapY{this, "SetSigmaEndcapY", 1.};
    Gaudi::Property<double> m_sigmaEndcapZ{this, "SetSigmaEndcapZ", 1.};
    Gaudi::Property<double> m_sigmaEndcapRotX{this, "SetSigmaEndcapRotX", 0.001};
    Gaudi::Property<double> m_sigmaEndcapRotY{this, "SetSigmaEndcapRotY", 0.001};
    Gaudi::Property<double> m_sigmaEndcapRotZ{this, "SetSigmaEndcapRotZ", 0.001};

    Gaudi::Property<double> m_softcutBarrelX{this, "SetSoftCutBarrelX", 1.};
    Gaudi::Property<double> m_softcutBarrelY{this, "SetSoftCutBarrelY", 1.};
    Gaudi::Property<double> m_softcutBarrelZ{this, "SetSoftCutBarrelZ", 1.};
    Gaudi::Property<double> m_softcutBarrelRotX{this, "SetSoftCutBarrelRotX", 1.};
    Gaudi::Property<double> m_softcutBarrelRotY{this, "SetSoftCutBarrelRotY", 1.};
    Gaudi::Property<double> m_softcutBarrelRotZ{this, "SetSoftCutBarrelRotZ", 1.};
    Gaudi::Property<double> m_softcutBarrelBowX{this, "SetSoftCutBarrelBowX", 1.};

    Gaudi::Property<double> m_softcutEndcapX{this, "SetSoftCutEndcapX", 1.};
    Gaudi::Property<double> m_softcutEndcapY{this, "SetSoftCutEndcapY", 1.};
    Gaudi::Property<double> m_softcutEndcapZ{this, "SetSoftCutEndcapZ", 1.};
    Gaudi::Property<double> m_softcutEndcapRotX{this, "SetSoftCutEndcapRotX", 1.};
    Gaudi::Property<double> m_softcutEndcapRotY{this, "SetSoftCutEndcapRotY", 1.};
    Gaudi::Property<double> m_softcutEndcapRotZ{this, "SetSoftCutEndcapRotZ", 1.};

    Gaudi::Property<bool> m_dumpGeometry{this, "DumpGeometry", true};
    Gaudi::Property<bool> m_alignDBM{this, "AlignmentDBM", false};
    
    Gaudi::Property<std::string> m_pixelDetManagerName{this,"PixelDetectorManager","Pixel","Name of the Pixel Detector Manager to attempt to retrieve"};
    Gaudi::Property<bool> m_doEtaCorrection{this,"EtaCorrection",true,"If we should check the DB to see if eta 0 should be skipped"};

  }; // end class

} // end namespace

#endif // INDETALIGNGEOMTOOLS_PIXELGEOMETRYMANAGERTOOL_H

