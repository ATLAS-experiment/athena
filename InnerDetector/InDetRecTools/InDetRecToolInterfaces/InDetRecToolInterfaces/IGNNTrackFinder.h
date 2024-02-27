/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IGNNTrackFinder_H
#define IGNNTrackFinder_H

#include <list>

#include "GaudiKernel/AlgTool.h"
#include "TrkSpacePoint/SpacePoint.h"

class MsgStream;

namespace InDet {

  /** 
   * @interface IGNNTrackFinder
   * @brief Find track candidates from a list of spacepoints
   * @author xiangyang.ju@cern.ch
   */
  class IGNNTrackFinder : virtual public IAlgTool
  {
    public:
    ///////////////////////////////////////////////////////////////////
    /// @name InterfaceID
    ///////////////////////////////////////////////////////////////////
    //@{
    DeclareInterfaceID(IGNNTrackFinder, 1, 0);
    //@}

    ///////////////////////////////////////////////////////////////////
    /// Main methods for track-finding
    ///////////////////////////////////////////////////////////////////
    /**
     * @brief Get track candidates from a list of space points.
     * @param spacepoints a list of spacepoints as inputs to the GNN-based track finder.
     * @param tracks a list of track candidates in terms of spacepoint indices.
     * @return 
    */
    virtual StatusCode getTracks(
      const std::vector<const Trk::SpacePoint*>& spacepoints,
      std::vector<std::vector<uint32_t> >& tracks) const=0;

    ///////////////////////////////////////////////////////////////////
    // Print internal tool parameters and status
    ///////////////////////////////////////////////////////////////////
  
    virtual MsgStream&    dump(MsgStream&    out) const=0;
    virtual std::ostream& dump(std::ostream& out) const=0;
  };


  ///////////////////////////////////////////////////////////////////
  // Overload of << operator for MsgStream and  std::ostream
  ///////////////////////////////////////////////////////////////////

  MsgStream&    operator << (MsgStream&   ,const IGNNTrackFinder&);
  std::ostream& operator << (std::ostream&,const IGNNTrackFinder&);

  ///////////////////////////////////////////////////////////////////
  // Overload of << operator MsgStream
  ///////////////////////////////////////////////////////////////////
    
  inline MsgStream& operator << (MsgStream& sl,const IGNNTrackFinder& se) { 
      return se.dump(sl); 
  }
  ///////////////////////////////////////////////////////////////////
  // Overload of << operator std::ostream
  ///////////////////////////////////////////////////////////////////

  inline std::ostream& operator << (std::ostream& sl,const IGNNTrackFinder& se) { 
      return se.dump(sl); 
  }
}

#endif