/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingVolumeDisplayer.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_TRACKINGVOLUMEDISPLAYER_H
#define TRKDETDESCRTOOLS_TRACKINGVOLUMEDISPLAYER_H

// Trk
#include "TrkDetDescrTestTools/RecursiveGeometryProcessor.h"
// STL
#include <string>
#include <fstream>

#include "CxxUtils/checker_macros.h" 
namespace Trk {

    class TrackingVolume;
    class Layer;
    class Surface;
    class MaterialProperties;

    /** @class TrackingVolumeDisplayer

        The TrackingVolumeDisplayer writes a root .C file 
        which can be executed and used the OpenGL root viewer

        @author Andreas.Salzburger@cern.ch   
     */

    class ATLAS_NOT_THREAD_SAFE TrackingVolumeDisplayer : //mutable
      virtual public RecursiveGeometryProcessor {

      public:

        /** Constructor */
        using RecursiveGeometryProcessor::RecursiveGeometryProcessor;

        /** AlgTool initialize method */
        StatusCode initialize();
        
        /** AlgTool finalize method */
        StatusCode finalize();

      private:

        /** Current implementation: write root visualization to file stream */
        StatusCode processNode(const TrackingVolume& tvol, size_t level=0) const;

        /** Current implementation: write root visualization to file stream */
        StatusCode processNode(const Layer& lay, size_t level=0) const;

        /** Current implementation: write root visualization to file stream */
        StatusCode processNode(const Surface&, size_t level=0) const;

        static void openFile(std::ofstream& output, const std::string& filename) ;  //!< File handling: open + write header
        static void closeFile(std::ofstream& output) ;                              //!< File handling: write footer + close

        /** calculate the color code from the Material */
        static int colorCodeFromMaterial(const Trk::MaterialProperties* prop, std::ofstream& output) ;

        mutable int                         m_volumeCounter = 0;                      //!< volume counter

        mutable std::ofstream               m_fileVolumeOutput;                   //!< file output for visualization action
        Gaudi::Property<std::string> m_fileVolumeOutputName
          {this, "TrackingVolumeOutputFile", "TrackingGeometryVolumeDisplay.C",
           "file name for visualization action"};
        Gaudi::Property<bool> m_fileVolumeOutputMode
          {this, "TrackingVolumeOutput", true, "steer writing"};

        mutable std::ofstream               m_fileLayerOutput;                    //!< file output for visualization action
        Gaudi::Property<std::string> m_fileLayerOutputName
          {this, "LayerOutputFile", "TrackingGeometryLayerDisplay.C",
           "file name for visualization action"};
        Gaudi::Property<bool> m_fileLayerOutputMode
          {this, "LayerOutput", true, "steer writing"};

        mutable std::ofstream               m_fileSurfaceOutput;                  //!< file output for visualization action
        Gaudi::Property<std::string> m_fileSurfaceOutputName
          {this, "SurfaceOutputFile", "TrackingGeometrySurfaceDisplay.C",
           "file name for visualization action"};
        Gaudi::Property<bool> m_fileSurfaceOutputMode
          {this, "SurfaceOutput", true, "steer writing"};
        Gaudi::Property<bool> m_fileSurfaceOutputSplit
          {this, "SurfaceOutputSplit", false, "use one file for each layer"};

        static int                          s_displaySurfaces;                    //!< static surface counter

    };

}

#endif

