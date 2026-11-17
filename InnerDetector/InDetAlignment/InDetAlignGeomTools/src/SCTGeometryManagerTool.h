/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNGEOMTOOLS_SCTGEOMETRYMANAGERTOOL_H
#define INDETALIGNGEOMTOOLS_SCTGEOMETRYMANAGERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "AthContainers/DataVector.h"

#include "TrkAlignInterfaces/IGeometryManagerTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"

#include "TrkAlignEvent/AlignModuleList.h"
#include "Gaudi/Property.h"

/**
  @file SCTGeometryManagerTool.h
  @class SCTGeometryManagerTool

  @brief Tool for setting up SCT alignment geometry.

  @author Vicente Lacuesta <Vicente.Lacuesta.Miquel@cern.ch>
  @author Daniel Kollar <daniel.kollar@cern.ch>
  @date 28/08/2009
*/

namespace InDetDD
{
  class SCT_DetectorManager;
}

class SCT_ID;

namespace Trk
{
  class AlignPar;
  class AlignModule;
}

namespace InDet
{
  class SCTGeometryManagerTool : public extends<AthAlgTool, Trk::IGeometryManagerTool>
  {
  public:
    using base_class::base_class;
    virtual ~SCTGeometryManagerTool();

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

    /** builds geometry for SCT alignment */
    void buildGeometry();

    /** adds alignment parameters for the module
     * checks for active parameters and calls setSigmas() */
    void addModuleParameters(Trk::AlignModule * module, DataVector< DataVector<Trk::AlignPar> > * allFullModPars, DataVector< DataVector<Trk::AlignPar> > * allActiveModPars);

    /** check whether the alignment level is correct */
    bool checkAlignLevel();

    /** check whether the Barrel alignment level is correct */
    bool checkAlignLevelBarrel();

    /** check whether the Endcap alignment level is correct */
    bool checkAlignLevelEndcaps();

    void setAlignLevel(int level);


  private:

    /** creates L0 AlignModules for SCT */
    void buildL0();

    /** creates L1 AlignModules for SCT barrel */
    void buildL1Barrel();

    /** creates L1 AlignModules for SCT endcaps */
    void buildL1Endcaps();

    /** creates L2 AlignModules for SCT barrel */
    void buildL2Barrel();

    /** creates L27 AlignModules for SCT barrel */
    void buildL27Barrel();

    /** creates L2 AlignModules for SCT endcaps */
    void buildL2Endcaps();

    /** creates L25 AlignModules for SCT endcaps */
    void buildL25Endcaps();

    /** creates L2 AlignModules for the full Pixel in the ordering equal to the old code */
    void buildL2Old();

    /** creates L3 AlignModules for SCT barrel */
    void buildL3Barrel();

    /** creates L3 AlignModules for SCT endcaps */
    void buildL3Endcaps();

    /** sets sigmas for modules */
    void setSigmas(Trk::AlignModule * mod, DataVector<Trk::AlignPar> * modPars);

    /** check wheather module is selected for module pointer */
    bool moduleSelected(Trk::AlignModule * mod);

    /** check wheather module is selected for module id */
    bool moduleSelected(unsigned long long modId);

    /** print basic geometry info to screen */
    void dumpGeometry();

    const InDetDD::SCT_DetectorManager * m_detManager{nullptr}; //!< pointer to SCT detector manager

    const SCT_ID * m_idHelper{nullptr}; //!< pointer to SCT detector manager

    PublicToolHandle<Trk::IAlignModuleTool> m_alignModuleTool{this, "AlignModuleTool", "Trk::AlignModuleTool/AlignModuleTool"};  //!< pointer to AlignModuleTool

    Trk::AlignModuleList m_alignModuleList;
    Trk::AlignModuleList m_idHashToAlignModuleMap;
      
    std::vector<Trk::AlignModuleList *> m_idHashToAlignModuleMaps{std::vector<Trk::AlignModuleList *>(Trk::AlignModule::NDetectorTypes, nullptr)}; 

    DataVector< DataVector<Trk::AlignPar> > * m_alignParList{nullptr};
    DataVector< DataVector<Trk::AlignPar> > * m_fullAlignParList{nullptr};

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

    Gaudi::Property<double> m_softcutEndcapX{this, "SetSoftCutEndcapX", 1.};
    Gaudi::Property<double> m_softcutEndcapY{this, "SetSoftCutEndcapY", 1.};
    Gaudi::Property<double> m_softcutEndcapZ{this, "SetSoftCutEndcapZ", 1.};
    Gaudi::Property<double> m_softcutEndcapRotX{this, "SetSoftCutEndcapRotX", 1.};
    Gaudi::Property<double> m_softcutEndcapRotY{this, "SetSoftCutEndcapRotY", 1.};
    Gaudi::Property<double> m_softcutEndcapRotZ{this, "SetSoftCutEndcapRotZ", 1.};

    Gaudi::Property<bool> m_useOldL2{this, "UseOldL2", false};
    Gaudi::Property<bool> m_dumpGeometry{this, "DumpGeometry", true};

    Gaudi::Property<std::string> m_stripDetManagerName{this,"StripDetectorManager","SCT","Name of the Strip Detector Manager to attempt to retrieve"};


  }; // end class

} // end namespace

#endif // INDETALIGNGEOMTOOLS_SCTGEOMETRYMANAGERTOOL_H
