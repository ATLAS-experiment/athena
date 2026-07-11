/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeometryJsonDumper.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_GEOMETRYJSONDUMPER_H
#define TRKDETDESCRTOOLS_GEOMETRYJSONDUMPER_H

// Trk
#include "TrkDetDescrTestTools/RecursiveGeometryProcessor.h"
// STL
#include <algorithm>
#include <string>
#include <fstream>
#include "CxxUtils/checker_macros.h"
class TTree;

namespace Trk {

    class TrackingVolume;
    class Layer;
    class Surface;

    /** @class GeometryJsonDumper

        The GeometryJsonDumper writes geometrical
        information of the tracking geomtetry 
        into an Ascii file for comparison

        @author Andreas.Salzburger@cern.ch   
     */

    class ATLAS_NOT_THREAD_SAFE GeometryJsonDumper : //mutable
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

        mutable std::ofstream       m_outputFile;
        Gaudi::Property<std::string> m_outputFileName
          {this, "OutputFileName", "TrackingGeometry.json"};
        mutable bool                m_firstLayerWritten = false;

    };
}

#endif

