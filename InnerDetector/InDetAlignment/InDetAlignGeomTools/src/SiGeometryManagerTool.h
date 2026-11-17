/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNGEOMTOOLS_SIGEOMETRYMANAGERTOOL_H
#define INDETALIGNGEOMTOOLS_SIGEOMETRYMANAGERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "AthContainers/DataVector.h"

#include "TrkAlignInterfaces/IGeometryManagerTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"

#include "TrkAlignEvent/AlignModuleList.h"

#include <TGeoManager.h>
#include "Gaudi/Property.h"


/**
  @file SiGeometryManagerTool.h
  @class SiGeometryManagerTool

  @brief Tool for setting up Silicon alignment geometry.

  @author Vicente Lacuesta <Vicente.Lacuesta.Miquel@cern.ch>
  @author Daniel Kollar <daniel.kollar@cern.ch>
  @date 28/08/2009
*/

namespace InDetDD
{
  class SCT_DetectorManager;
  class PixelDetectorManager;
}

class SCT_ID;
class PixelID;
class SiliconID;

namespace Trk
{
  class AlignPar;
  class AlignModule;
}

namespace InDet
{
  class SiGeometryManagerTool : public extends<AthAlgTool, Trk::IGeometryManagerTool>
  {
  public:
    using base_class::base_class;
    virtual ~SiGeometryManagerTool();

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

    /** sets the output stream for the logfile */
    void setLogStream(std::ostream * os);

  private:

    /** creates L0 AlignModules for Silicon */
    void buildL0();

    /** sets sigmas for modules */
    void setSigmas(Trk::AlignModule * mod, DataVector<Trk::AlignPar> * modPars);

    /** check wheather module is selected for module pointer */
    bool moduleSelected(Trk::AlignModule * mod);

    /** check wheather module is selected for module id */
    bool moduleSelected(unsigned long long modId);

    /** print basic geometry info to screen */
    void dumpGeometry();

    /** check whether the module is of a single detector type */
    bool isOneDetOnly(const Trk::AlignModule * mod, Trk::AlignModule::DetectorType dettype) const;

    const InDetDD::PixelDetectorManager * m_pixelDetManager{nullptr}; //!< pointer to PIX detector manager
    const InDetDD::SCT_DetectorManager  * m_sctDetManager{nullptr};   //!< pointer to SCT detector manager

    const PixelID * m_pixHelper{nullptr};  //!< pointer to PIX detector manager
    const SCT_ID * m_sctHelper{nullptr};   //!< pointer to SCT detector manager
    const SiliconID * m_idHelper{nullptr}; //!< pointer to Silicon detector manager

    PublicToolHandle<Trk::IGeometryManagerTool> m_pixelGeoManager{this, "PixelGeometryManager", ""}; //!< pointer to Pixel geometry manager
    PublicToolHandle<Trk::IGeometryManagerTool> m_sctGeoManager{this, "SCTGeometryManager", ""};   //!< pointer to SCT geometry manager

    PublicToolHandle<Trk::IAlignModuleTool> m_alignModuleTool{this, "AlignModuleTool", "Trk::AlignModuleTool/AlignModuleTool"};     //!< pointer to AlignModuleTool
      
    Trk::AlignModuleList m_alignModuleList;
    Trk::AlignModuleList m_idHashToAlignModuleMap;
      
    std::vector<Trk::AlignModuleList *> m_idHashToAlignModuleMaps{std::vector<Trk::AlignModuleList *>(Trk::AlignModule::NDetectorTypes, nullptr)}; 

    DataVector< DataVector<Trk::AlignPar> > * m_alignParList{nullptr};
    DataVector< DataVector<Trk::AlignPar> > * m_fullAlignParList{nullptr};

    // alignment transforms
    Gaudi::Property<bool> m_alignX{this, "AlignX", true};
    Gaudi::Property<bool> m_alignY{this, "AlignY", true};
    Gaudi::Property<bool> m_alignZ{this, "AlignZ", true};
    Gaudi::Property<bool> m_alignRotX{this, "AlignRotX", true};
    Gaudi::Property<bool> m_alignRotY{this, "AlignRotY", true};
    Gaudi::Property<bool> m_alignRotZ{this, "AlignRotZ", true};

    Gaudi::Property<double> m_sigmaX{this, "SetSigmaX", 1.};
    Gaudi::Property<double> m_sigmaY{this, "SetSigmaY", 1.};
    Gaudi::Property<double> m_sigmaZ{this, "SetSigmaZ", 1.};
    Gaudi::Property<double> m_sigmaRotX{this, "SetSigmaRotX", 0.001};
    Gaudi::Property<double> m_sigmaRotY{this, "SetSigmaRotY", 0.001};
    Gaudi::Property<double> m_sigmaRotZ{this, "SetSigmaRotZ", 0.001};

    // Silicon joboptions
    Gaudi::Property<bool> m_alignPixel{this, "AlignPixel", true};
    Gaudi::Property<bool> m_alignSCT{this, "AlignSCT", true};

    Gaudi::Property<int> m_alignLevelProp{this, "AlignmentLevel", -1}; //!< property, copied to m_alignLevel in initialize()
    Gaudi::Property<bool> m_doModuleSelection{this, "doModuleSelection", false};
    Gaudi::Property<std::vector<unsigned long long>> m_moduleSelection{this, "ModuleSelection", {}};

    Gaudi::Property<bool> m_dumpGeometry{this, "DumpGeometry", true};
    Gaudi::Property<bool> m_actualGeom{this, "ActualGeometry", false};

    Gaudi::Property<std::string> m_pixelDetManagerName{this,"PixelDetectorManager","Pixel","Name of the Pixel Detector Manager to attempt to retrieve"};
    Gaudi::Property<std::string> m_stripDetManagerName{this,"StripDetectorManager","SCT","Name of the Strip Detector Manager to attempt to retrieve"};

  }; // end class

} // end namespace

#endif // INDETALIGNGEOMTOOLS_SIGEOMETRYMANAGERTOOL_H
