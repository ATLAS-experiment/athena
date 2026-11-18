/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNDBTOOLS_SITRKALIGNDBTOOL_H
#define INDETALIGNDBTOOLS_SITRKALIGNDBTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkAlignInterfaces/ITrkAlignDBTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"
#include "TrkAlignInterfaces/IGeometryManagerTool.h"
#include "InDetAlignGenTools/IInDetAlignDBTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "GeoPrimitives/GeoPrimitives.h"

/**
  @file SITrkAlignDBTool.h 
  @class SITrkAlignDBTool 

  @brief Tool for  writing Silicon alignment constants into oracle DB or an ASCII file.

  @author Roland Haertel <haertel@mppmu.mpg.de>
  @author Daniel Kollar <daniel.kollar@cern.ch>
  @date 27/08/2009
*/

class TFile;

class PixelID;
class SCT_ID;
class AtlasDetectorID;

class Identifier;

namespace Trk
{
  class AlignModule;
}

namespace InDet
{

  class SiTrkAlignDBTool : public extends<AthAlgTool, Trk::ITrkAlignDBTool> {

  public:

    /** constructor */
    using base_class::base_class;

    /** destructor */
    virtual ~SiTrkAlignDBTool();

    /** initialize method */
    virtual StatusCode initialize();

    /** finalize method */
    virtual StatusCode finalize();

    /** Writes alignment parameters */
    void writeAlignPar();

    /** sets ntuple to be used to write output */
    void setNtuple(TFile * file)
      { m_ntuple=file; }
    

  private:

    /** updates alignment constants in the DB */
    void updateDB();

    bool checkPixelLevel();
    bool checkSCTLevel();
    bool checkSiLevel();
    
    void updateSiL0asL1(Identifier idL0, const Amg::Transform3D & transform);
    
    void updateAsL3(const Trk::AlignModule * module, const Amg::Transform3D & transform, double);
    
    void printTransform(const Amg::Transform3D & tr) const;
    
    void updateAsL2(const Trk::AlignModule * module, const Amg::Transform3D & transform, const int);

    void updateAsL16(const Trk::AlignModule * module, double);
    
    virtual Identifier GetL1L2fromL3Identifier( const Identifier& ident , const int& level) const ;

    const PixelID          * m_pixHelper{nullptr}; //!< pointer to PIX detector manager
    const SCT_ID           * m_sctHelper{nullptr}; //!< pointer to SCT detector manager
    const AtlasDetectorID  * m_idHelper{nullptr};

    Gaudi::Property<bool>        m_writeSQLFile{this, "WriteSQLFile", true};                           //!< flag wether to write out .db file or not (file is not needed in parallel jobs)
    Gaudi::Property<std::string> m_SQLiteTag{this, "SQLiteTag", "test_tag"};                           //!< tag name for the ConditionsDB
    Gaudi::Property<std::string> m_outputAlignFile{this, "OutputTextFile", "OutputSiAlignment.txt"};   //!< filename for the textfile with final alignment constants
    Gaudi::Property<std::string> m_outIBLDistFile{this, "OutputIBLDistFile", "OutputIBLDist.txt", "File to write out IBL distortions - set empy to suppress writing this folder"}; //!< filename for the textfile with final IBLDist params
    Gaudi::Property<std::string> m_outGlobalFolderFile{this, "OutputGlobalFolderFile", "OutputSiGlobalFolder.txt"}; //!< filename for the textfile with final GlobalFolder constants
    Gaudi::Property<bool>        m_writeTextFile{this, "WriteTextFile", true};                         //!< flag whether to write out the final constants into text file
    Gaudi::Property<bool>        m_writeOldConstants{this, "WriteOldConstants", false};                //!< flag whether to write out the initial constants into text file
    Gaudi::Property<std::string> m_oldIBLDistFile{this, "OldIBLDistFile", "OldIBLDist.txt", "File to write out  (old-style) IBL distortions - set empy to suppress writing this folder"}; //!< filename for the textfile with initial IBLDist params
    Gaudi::Property<std::string> m_oldAlignFile{this, "OldAlignFile", "OldSiAlignment.txt"};           //!< filename for the textfile with initial alignment constants
    Gaudi::Property<std::string> m_oldGlobalFolderFile{this, "OldGlobalFolderFile", "OldSiGlobalFolder.txt"}; //!< filename for the textfile with initial GlobalFolder constants
    Gaudi::Property<bool>        m_updateConstants{this, "UpdateConstants", true};                     //!< flag whether to update the constants

    PublicToolHandle<IInDetAlignDBTool>        m_IDAlignDBTool{this, "IDAlignDBTool", "InDetAlignDBTool"};
    PublicToolHandle<Trk::IAlignModuleTool>    m_alignModuleTool{this, "AlignModuleTool", "Trk::AlignModuleTool/AlignModuleTool", "Tool for handling of align module collections"};

    PublicToolHandle<Trk::IGeometryManagerTool> m_pixelGeoManager{this, "PixelGeometryManager", "", "Tool for handling the Pixel alignment geometry"};
    PublicToolHandle<Trk::IGeometryManagerTool> m_sctGeoManager{this, "SCTGeometryManager", "", "Tool for handling the SCT alignment geometry"};
    PublicToolHandle<Trk::IGeometryManagerTool> m_siGeoManager{this, "SiGeometryManager", "", "Tool for handling the Silicon alignment geometry"};

    TFile * m_ntuple{};

    int m_pixelAlignLevel{-1};
    int m_pixelAlignLevelBarrel{-1};
    int m_pixelAlignLevelEndcaps{-1};
    Gaudi::Property<bool> m_pixelaAlignDBM{this, "AlignmentDBM", false};

    int m_sctAlignLevel{-1};
    int m_sctAlignLevelBarrel{-1};
    int m_sctAlignLevelEndcaps{-1};

    int m_siAlignLevel{-1};

    bool m_doSi{false};
    bool m_doPixel{false};
    bool m_doSCT{false};

    Gaudi::Property<bool> m_writeAsL3{this, "WriteAsL3", false};
    Gaudi::Property<bool> m_writeAsL2{this, "WriteAsL2", false};
    bool m_writeAsL16{false};

  }; // end class

} // end namespace

#endif // INDETALIGNDBTOOLS_SITRKALIGNDBTOOL_H
