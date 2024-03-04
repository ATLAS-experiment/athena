/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SeedFitterTool_H
#define SeedFitterTool_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "InDetRecToolInterfaces/ISeedFitter.h"

class MsgStream;

namespace InDet {

  /** 
   * @class SeedFitterTool
   * @brief Estimate the initial track parameters for a list of spacepoints/measurements (i.e. Track candidates),
   * which latter can be used as input for the track fitter. The method is based on a simple linear conformal mapping.
   * @author xiangyang.ju@cern.ch
   */
  class SeedFitterTool : public extends<AthAlgTool, ISeedFitter>
  {
    public:
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
    SeedFitterTool(const std::string&,const std::string&,const IInterface*);

    ///////////////////////////////////////////////////////////////////
    // Methods to convert spacepoints to Trk::Track
    ///////////////////////////////////////////////////////////////////
    virtual std::unique_ptr<const Trk::TrackParameters> fit(
        const std::vector<const Trk::SpacePoint*>& spacepoints
    ) const override;

    ///////////////////////////////////////////////////////////////////
    // Print internal tool parameters and status
    ///////////////////////////////////////////////////////////////////
    virtual MsgStream&    dump(MsgStream&    out) const override;
    virtual std::ostream& dump(std::ostream& out) const override;
    MsgStream& dumpevent( MsgStream& out ) const;

    protected:

    SeedFitterTool() = delete;
    SeedFitterTool(const SeedFitterTool&) = delete;
    SeedFitterTool& operator=(const SeedFitterTool&) = delete;


  };
}

#endif
