/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNDBTOOLS_TRTTRKALIGNDBTOOL_H
#define INDETALIGNDBTOOLS_TRTTRKALIGNDBTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkAlignInterfaces/ITrkAlignDBTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"
#include "TrkAlignInterfaces/IGeometryManagerTool.h"
#include "TRT_ConditionsServices/ITRT_AlignDbSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"


/**
   @file TRTTrkAlignDBTool.h
   @class TRTTrkAlignDBTool

   @brief Tool for writing TRT alignment constants into oracle DB or an ASCII file.

   @author Daniel Kollar <daniel.kollar@cern.ch>
   @date 25/02/2009
*/

class TFile;

class TRT_ID;
class PixelID;

class Identifier;

namespace Trk
{
   class AlignModule;
}

namespace InDet
{

   class TRTTrkAlignDBTool : public extends<AthAlgTool, Trk::ITrkAlignDBTool>
   {

      public:
         using base_class::base_class;
         virtual ~TRTTrkAlignDBTool();

         StatusCode initialize();
         StatusCode finalize();

         /** Writes alignment parameters */
         void writeAlignPar();

         /** sets ntuple to be used to write output */
         void setNtuple(TFile * file)
            { m_ntuple=file; }

      private:

         /** print translation to stdout */
         void printTransform(const Amg::Transform3D & tr) const;

         /** updates alignment constants in the DB */
         void updateDB();

         /** checks whether the AlignModule contains TRT detector elements*/
         bool containsTRT(Trk::AlignModule * module) const;

         /** updates level 0 transform as level 1 constants in the DB */
         void updateL0asL1(Identifier idL0, const Amg::Transform3D & transform);

         bool checkAlignLevel();

         const TRT_ID  * m_trtHelper{nullptr};
         const PixelID * m_pixHelper{nullptr};

         Gaudi::Property<std::string> m_outputAlignFile{this, "OutputTextFile", "OutputTRTAlignment.txt", "Text file to write the final alignment constants"};
         Gaudi::Property<std::string> m_outputStrawAlignFile{this, "OutputStrawTextFile", "OutputTRTStrawAlignment.txt", "Text file to write the final straw alignment constants"};
         Gaudi::Property<std::string> m_oldAlignFile{this, "OldAlignFile", "OldTRTAlignment.txt", "Text file to write the initial/old alignment constants"};
         Gaudi::Property<std::string> m_oldStrawAlignFile{this, "OldStrawAlignFile", "OldTRTStrawAlignment.txt", "Text file to write the initial/old straw alignment constants"};
         Gaudi::Property<bool> m_writeTextFile{this, "WriteTextFile", true, "Whether or not to write the final/new alignment constants to text file"};
         Gaudi::Property<bool> m_writeOldConstants{this, "WriteOldConstants", false, "Whether or not to write the initial/old alignment constants to text file"};
         Gaudi::Property<bool> m_updateConstants{this, "UpdateConstants", true, "Whether or no to update the alignment constants"};

         ServiceHandle<ITRT_AlignDbSvc>         m_trtAlignDbSvc{this, "TrtAlignDbSvc", "TRT_AlignDbSvc", "Service for interaction with the TRT alignment DB"};
         PublicToolHandle<Trk::IAlignModuleTool>      m_alignModuleTool{this, "AlignModuleTool", "Trk::AlignModuleTool/AlignModuleTool", "Tool for handling of align module collections"};
         PublicToolHandle<Trk::IGeometryManagerTool>  m_trtGeoManager{this, "TRTGeometryManager", "", "Tool for handling the TRT alignment geometry"};

         int m_trtAlignLevel{-1};
         int m_trtAlignLevelBarrel{-1};
         int m_trtAlignLevelEndcaps{-1};

         TFile * m_ntuple{};

   }; // end class

} // end namespace

#endif // INDETALIGNDBTOOLS_TRTTRKALIGNDBTOOL_H
