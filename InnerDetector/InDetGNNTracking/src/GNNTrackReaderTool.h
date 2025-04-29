/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GNNTrackReaderTool_H
#define GNNTrackReaderTool_H

// System include(s).
#include <list>
#include <iostream>
#include <memory>

#include "AthenaBaseComps/AthAlgTool.h"
#include "IGNNTrackReaderTool.h"

class MsgStream;

namespace InDet{
  /**
   * @class InDet::GNNTrackReaderTool
   * @brief InDet::GNNTrackReaderTool is a tool that reads track candidates 
   * of each event from a CSV file named as "track_runNumber_eventNumber.csv" 
   * or with a customized pre-fix.
   * The directory, inputTracksDir, contains CSV files for all events. 
   * 1) If the inputTracksDir is not specified, the tool will read the CSV files
   * from the current directory.
   * 2) If the corresponding CSV file does not exist for a given event (runNumber, eventNumber),
   * the tool will print an error message and return an empty list.
   * @author xiangyang.ju@cern.ch
   */

  class GNNTrackReaderTool: public extends<AthAlgTool, IGNNTrackReaderTool>
  {
    public:
    GNNTrackReaderTool(const std::string& type, const std::string& name, const IInterface* parent);

    ///////////////////////////////////////////////////////////////////
    // Main methods for local track finding asked by the ISiMLTrackFinder
    ///////////////////////////////////////////////////////////////////

    /**
     * @brief Get track candidates from a CSV file named by runNumber and eventNumber.
     * @param runNumber run number of the event.
     * @param eventNumber event number of the event.
     * @param tracks a list of track candidates in terms of spacepoint indices as read from the CSV file.
    */
    virtual void getTracks(uint32_t runNumber, uint32_t eventNumber,
      std::vector<std::vector<uint32_t> >& tracks) const override final;

    virtual void getTracks(
      uint32_t runNumber, uint32_t eventNumber,
      std::vector<std::vector<uint32_t>>& tracks,
      std::vector<std::vector<uint32_t>>& seeds) const override final;
    ///////////////////////////////////////////////////////////////////
    // Print internal tool parameters and status
    ///////////////////////////////////////////////////////////////////
    virtual MsgStream&    dump(MsgStream&    out) const override;
    virtual std::ostream& dump(std::ostream& out) const override;

    protected:

    GNNTrackReaderTool() = delete;
    GNNTrackReaderTool(const GNNTrackReaderTool&) =delete;
    GNNTrackReaderTool &operator=(const GNNTrackReaderTool&) = delete;

    StringProperty m_inputTracksDir{this, "inputTracksDir", "."};
    StringProperty m_csvPrefix{this, "csvPrefix", "track"};

    MsgStream&    dumpevent     (MsgStream&    out) const;

  };

  MsgStream&    operator << (MsgStream&   ,const GNNTrackReaderTool&);
  std::ostream& operator << (std::ostream&,const GNNTrackReaderTool&); 

}

#endif
