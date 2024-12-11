/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IGNNTrackReaderTool_H
#define IGNNTrackReaderTool_H

#include <list>
#include <vector>

#include "GaudiKernel/AlgTool.h"

class MsgStream;

namespace InDet {

  /** 
   * @interface IGNNTrackReaderTool
   * @brief Read GNN Track candidates from a CSV file
   * @author xiangyang.ju@cern.ch
   */
  class IGNNTrackReaderTool : virtual public IAlgTool
  {
    public:
    ///////////////////////////////////////////////////////////////////
    /// @name InterfaceID
    ///////////////////////////////////////////////////////////////////
    //@{
    DeclareInterfaceID(IGNNTrackReaderTool, 1, 0);
    //@}

    ///////////////////////////////////////////////////////////////////
    /// Main methods for reading track candidates
    ///////////////////////////////////////////////////////////////////
    /**
     * @brief Get track candidates from a CSV file named by runNumber and eventNumber.
     * @param runNumber run number of the event.
     * @param eventNumber event number of the event.
     * @param tracks a list of track candidates in terms of spacepoint indices as read from the CSV file.
    */
    virtual void getTracks(uint32_t runNumber, uint32_t eventNumber,
      std::vector<std::vector<uint32_t> >& tracks) const =0;

    virtual void getTracks(uint32_t runNumber, uint32_t eventNumber,
      std::vector<std::vector<uint32_t> >& tracks, std::vector<std::vector<uint32_t> >& seeds) const =0;


    ///////////////////////////////////////////////////////////////////
    // Print internal tool parameters and status
    ///////////////////////////////////////////////////////////////////
  
    virtual MsgStream&    dump(MsgStream&    out) const=0;
    virtual std::ostream& dump(std::ostream& out) const=0;
  };


  ///////////////////////////////////////////////////////////////////
  // Overload of << operator for MsgStream and  std::ostream
  ///////////////////////////////////////////////////////////////////

  MsgStream&    operator << (MsgStream&   ,const IGNNTrackReaderTool&);
  std::ostream& operator << (std::ostream&,const IGNNTrackReaderTool&);

  ///////////////////////////////////////////////////////////////////
  // Overload of << operator MsgStream
  ///////////////////////////////////////////////////////////////////
    
  inline MsgStream& operator << (MsgStream& sl,const IGNNTrackReaderTool& se) { 
      return se.dump(sl); 
  }
  ///////////////////////////////////////////////////////////////////
  // Overload of << operator std::ostream
  ///////////////////////////////////////////////////////////////////

  inline std::ostream& operator << (std::ostream& sl,const IGNNTrackReaderTool& se) { 
      return se.dump(sl); 
  }
}

#endif