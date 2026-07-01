/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_WriteTrackingGeometry_H
#define ACTSGEOMETRY_WriteTrackingGeometry_H

// ATHENA
#include "AthenaBaseComps/AthAlgorithm.h"

// PACKAGE
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

// STL
#include <fstream>
#include <memory>
#include <vector>


namespace ActsTrk{


/** @brief Algorithm that uses the Acts JSON plugin to dump the tracking geometry
 *         into a JSON format which can be read back by Acts Standalone. The algorithm
 *         is appended to the chain of event algorithms and the dump happens once per job */
class WriteTrackingGeometry : public AthAlgorithm {
public:
  using AthAlgorithm::AthAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;

  virtual unsigned int cardinality() const override final { return 1; }

private:
    PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};

    Gaudi::Property<std::string> m_outFile{this, "outFile", "ActsTrackingGeometry.json"};

    bool m_dumped{false};


};
}
#endif
