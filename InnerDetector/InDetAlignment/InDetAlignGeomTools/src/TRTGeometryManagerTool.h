/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNGEOMTOOLS_TRTGEOMETRYMANAGERTOOL_H
#define INDETALIGNGEOMTOOLS_TRTGEOMETRYMANAGERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "AthContainers/DataVector.h"

#include "TrkAlignInterfaces/IGeometryManagerTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"

#include "TrkAlignEvent/AlignModuleList.h"
#include "Gaudi/Property.h"

/** 
  @file TRTGeometryManagerTool.h
  @class TRTGeometryManagerTool

  @brief Tool for setting up TRT alignment geometry.

  @author Daniel Kollar <daniel.kollar@cern.ch> 
  @date 22/01/2009
*/

namespace InDetDD
{
   class TRT_DetectorManager;
}

class TRT_ID;

namespace Trk
{
   class AlignPar;
   class AlignModule;
}

namespace InDet
{
   class TRTGeometryManagerTool : public extends<AthAlgTool, Trk::IGeometryManagerTool>
   {
      public:
         using base_class::base_class;
         virtual ~TRTGeometryManagerTool();

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

         /** builds geometry for TRT alignment */
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

         /** builds geometry for L1 TRT alignment
           * Creates 1 module - the whole TRT */
         void buildL0();

         /** builds geometry for L1 TRT Barrel alignment
           * Creates single module for barrel */
         void buildL1Barrel();

         /** builds geometry for L1 TRT Endcap alignment
           * Creates 2 modules, one for each endcap */
         void buildL1Endcaps();

         /** builds geometry for L2 TRT Barrel alignment
           * Creates 96 modules - 3 layers with 32 modules per layer */
         void buildL2Barrel();

         /** builds geometry for L2 TRT Endcapalignment
           * Creates 80 modules - 40 rings per endcap */
         void buildL2Endcaps();

         /** builds geometry for L2 TRT Endcapalignment
           * Creates 28 modules - 14 wheels per endcap */
         void buildL2EndcapsOLD();

         /** builds geometry for L3 TRT Barrel alignment
           * Creates many modules - one module per straw */
         void buildL3Barrel();

         /** builds geometry for L3 TRT Endcap alignment
           * Creates many modules - one module per straw */
         void buildL3Endcap();

         /** sets sigmas for modules */
         void setSigmas(Trk::AlignModule * mod, DataVector<Trk::AlignPar> * modPars);

         /** check wheather module is selected for module pointer */
         bool moduleSelected(Trk::AlignModule * mod);

         /** check wheather module is selected for module id */
         bool moduleSelected(unsigned long long modId);

         /** check wheather barrel module is selected by layer and phi */
         bool moduleSelectedBarrel(unsigned int iLayer, unsigned int iPhi);

         /** check wheather we are aligning the different endcaps and sectors */
         bool moduleSelectedEndcap(unsigned int iwheel, unsigned int iphi);

         /** print basic geometry info to screen */
         void dumpGeometry();

         const InDetDD::TRT_DetectorManager * m_trtDetManager{nullptr}; //!< pointer to TRT detector manager

         const TRT_ID * m_idHelper{nullptr};

         PublicToolHandle<Trk::IAlignModuleTool> m_alignModuleTool{this, "AlignModuleTool", "Trk::AlignModuleTool/AlignModuleTool"};  //!< pointer to AlignModuleTool

         Trk::AlignModuleList m_alignModuleList;
         Trk::AlignModuleList m_idHashToAlignModuleMap;

         std::vector<Trk::AlignModuleList*> m_idHashToAlignModuleMaps{std::vector<Trk::AlignModuleList *>(Trk::AlignModule::NDetectorTypes, nullptr)}; 

         DataVector< DataVector<Trk::AlignPar> > * m_alignParList{nullptr};
         DataVector< DataVector<Trk::AlignPar> > * m_fullAlignParList{nullptr};

         Gaudi::Property<bool> m_alignBarrel{this, "AlignBarrel", true};
         Gaudi::Property<bool> m_alignEndcaps{this, "AlignEndcaps", true};

         Gaudi::Property<bool> m_alignBarrelX{this, "AlignBarrelX", true};
         Gaudi::Property<bool> m_alignBarrelY{this, "AlignBarrelY", true};
         Gaudi::Property<bool> m_alignBarrelZ{this, "AlignBarrelZ", false};
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

         Gaudi::Property<std::vector<int>> m_barrelPhiSectors{this, "AlignBarrelPhiSectors", {}};   //!< list of barrel phi sectors to align (if empty, align everything)
         Gaudi::Property<std::vector<int>> m_barrelLayers{this, "AlignBarrelLayers", {}};       //!< list of barrel layers to align (if empty, align everything)

         Gaudi::Property<int> m_endcap{this, "AlignSingleEndcap", 0};                          //!< endcap_id of the endcap to align (-2 or 2 for endcap A or C, 0 for both)
         Gaudi::Property<std::vector<int>> m_endcapPhiSectors{this, "AlignEndcapPhiSectors", {}};   //!< list of endcap phi sectors to align (if empty, align everything)
         Gaudi::Property<std::vector<int>> m_endcapWheels{this, "AlignEndcapWheels", {}};       //!< list of endcap wheels to align (if empty, align everything)

         Gaudi::Property<int> m_alignLevelProp{this, "AlignmentLevel", 1}; //!< property, copied to m_alignLevel in initialize()
         Gaudi::Property<int> m_alignLevelBarrelProp{this, "AlignmentLevelBarrel", -1}; //!< property, copied to m_alignLevelBarrel in initialize()
         Gaudi::Property<int> m_alignLevelEndcapsProp{this, "AlignmentLevelEndcaps", -1}; //!< property, copied to m_alignLevelEndcaps in initialize()
         Gaudi::Property<bool> m_doModuleSelection{this, "doModuleSelection", false};
         Gaudi::Property<std::vector<unsigned long long>> m_moduleSelection{this, "ModuleSelection", {}};

         Gaudi::Property<bool> m_dumpGeometry{this, "DumpGeometry", true};
   }; // end class

} // end namespace

#endif // INDETALIGNGEOMTOOLS_TRTGEOMETRYMANAGERTOOL_H
