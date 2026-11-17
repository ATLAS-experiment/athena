/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNGEOMTOOLS_INDETGEOMETRYMANAGERTOOL_H
#define INDETALIGNGEOMTOOLS_INDETGEOMETRYMANAGERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "AthContainers/DataVector.h"

#include "TrkAlignInterfaces/IGeometryManagerTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"

#include "TrkAlignEvent/AlignModuleList.h"
#include "Gaudi/Property.h"

/**
  @file InDetGeometryManagerTool.h
  @class InDetGeometryManagerTool

  @brief Tool for setting up Inner Detector alignment geometry.

  @author Daniel Kollar <daniel.kollar@cern.ch>
  @date 04/09/2009
*/

namespace InDetDD
{
  class SCT_DetectorManager;
  class PixelDetectorManager;
  class SiGeometryManagerTool;
}

class SCT_ID;
class PixelID;
class SiliconID;
class TRT_ID;

namespace Trk
{
  class AlignPar;
  class AlignModule;
}

namespace InDet
{
  class InDetGeometryManagerTool : public extends<AthAlgTool, Trk::IGeometryManagerTool>
  {
  public:
    using base_class::base_class;
    virtual ~InDetGeometryManagerTool();

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

    /** check whether the module is Silicon only */
    bool isSiOnly(const Trk::AlignModule * mod) const;

    const InDetDD::PixelDetectorManager * m_pixelDetManager{nullptr}; //!< pointer to PIX detector manager
    const InDetDD::SCT_DetectorManager  * m_sctDetManager{nullptr};   //!< pointer to SCT detector manager
    const InDetDD::TRT_DetectorManager  * m_trtDetManager{nullptr};   //!< pointer to TRT detector manager

    const PixelID   * m_pixHelper{nullptr}; //!< pointer to PIX detector manager
    const SCT_ID    * m_sctHelper{nullptr}; //!< pointer to SCT detector manager
    const SiliconID * m_siHelper{nullptr};  //!< pointer to Silicon detector manager
    const TRT_ID    * m_trtHelper{nullptr}; //!< pointer to TRT detector manager

    PublicToolHandle<Trk::IGeometryManagerTool> m_siGeoManager{this, "SiGeometryManager", ""};    //!< pointer to Silicon geometry manager
    PublicToolHandle<Trk::IGeometryManagerTool> m_trtGeoManager{this, "TRTGeometryManager", ""};   //!< pointer to TRT geometry manager

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
    Gaudi::Property<bool> m_alignSi{this, "AlignSilicon", true};
    Gaudi::Property<bool> m_alignTRT{this, "AlignTRT", true};

    Gaudi::Property<int> m_alignLevelProp{this, "AlignmentLevel", -1}; //!< property, copied to m_alignLevel in initialize()
    Gaudi::Property<bool> m_doModuleSelection{this, "doModuleSelection", false};
    Gaudi::Property<std::vector<unsigned long long>> m_moduleSelection{this, "ModuleSelection", {}};

    Gaudi::Property<bool> m_dumpGeometry{this, "DumpGeometry", true};

  }; // end class

} // end namespace

#endif // INDETALIGNGEOMTOOLS_INDETGEOMETRYMANAGERTOOL_H
