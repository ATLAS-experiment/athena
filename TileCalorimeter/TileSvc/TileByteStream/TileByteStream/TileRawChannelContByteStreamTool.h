/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEBYTESTREAM_TILERAWCHANNELCONTRAWEVENTTOOL_H
#define TILEBYTESTREAM_TILERAWCHANNELCONTRAWEVENTTOOL_H

#include "TileByteStream/TileHid2RESrcID.h"

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/FullEventAssembler.h"
#include "StoreGate/ReadCondHandleKey.h"
#include <vector>

class TileHWID;
class TileRawChannelContainer;
class TileFastRawChannel;
class TileCondToolEmscale;
class ITileBadChanTool;

#include <string>

/**
 * @class TileRawChannelContByteStreamTool
 * @brief AlgTool class to provide conversion from TileRawChannelContainer to ByteStream,
 * and fill it in RawEvent. <p>
 * @author Hong Ma
 * @version  Created, Sept 25, 2002 <p>
 *  requirements:   typedef for CONTAINER class method: <p> 
 *   StatusCode convert(CONTAINER* cont, RawEvent* re); 
 */

class TileRawChannelContByteStreamTool: public AthAlgTool {
  public:

    typedef TileRawChannelContainer CONTAINER;

    /** constructor
     */
    TileRawChannelContByteStreamTool(const std::string& type, const std::string& name,
        const IInterface* parent);

    /** destructor
     */
    virtual ~TileRawChannelContByteStreamTool();

    /** AlgTool InterfaceID
     */
    static const InterfaceID& interfaceID();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    /** Provides conversion from TileRawChannelContainer to BS
     */
    StatusCode convert(CONTAINER* cont, FullEventAssembler<TileHid2RESrcID> *fea) const;

  private:

    Gaudi::Property<bool> m_doFragType4{this, "DoFragType4", true, "Do frag type 4"};
    Gaudi::Property<bool> m_doFragType5{this, "DoFragType5", false, "Do frag type 5"};
    Gaudi::Property<bool> m_initializeForWriting{this, "InitializeForWriting", false, "Initialize for writing"};

    SG::ReadCondHandleKey<TileHid2RESrcID> m_hid2RESrcIDKey{this,
        "TileHid2RESrcID", "TileHid2RESrcIDHLT", "TileHid2RESrcID key"};

    const TileHWID* m_tileHWID;
    bool m_verbose;

    /** Handle to Tile calibration tool */
    ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,
        "TileCondToolEmscale", "TileCondToolEmscale", "Tile EM scale conditions tool"};

    /** Handle to Tile bad channel tool */
    ToolHandle<ITileBadChanTool> m_tileBadChanTool{this,
        "TileBadChanTool", "TileBadChanTool", "Tile bad channel tool"};

    /** maximum number of channels in a drawer */
    int m_maxChannels;

    std::vector<int> m_demoFragIDs;
    std::vector<int> m_legacy2DemoChannelLB;
    std::vector<int> m_legacy2DemoChannelEB;
};

#endif
