/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeometryTTreeDumper.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_GEOMETRYTTREEDUMPER_H
#define TRKDETDESCRTOOLS_GEOMETRYTTREEDUMPER_H

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
    class MaterialProperties;

    /** @class GeometryTTreeDumper

        The GeometryTTreeDumper writes geometrical
        information of the tracking geomtetry 
        into a TTree for chekcs and comparisons

        @author Andreas.Salzburger@cern.ch   
     */

    class ATLAS_NOT_THREAD_SAFE GeometryTTreeDumper : //mutable
      virtual public RecursiveGeometryProcessor {
  
    public:
        /** Constructor */
        using RecursiveGeometryProcessor::RecursiveGeometryProcessor;

      private:

        /** Current implementation: write root visualization to file stream */
        StatusCode processNode(const TrackingVolume& tvol, size_t level=0) const;

        /** Current implementation: write root visualization to file stream */
        StatusCode processNode(const Layer& lay, size_t level=0) const;

        /** Current implementation: write root visualization to file stream */
        StatusCode processNode(const Surface&, size_t level=0) const;

        mutable TTree*      m_currentTree = nullptr; //!< the tree for the currently processed tracking Volume
        std::string         m_treeFolder = "/val/";  //!< where the tree is written to

    };
}

#endif

