/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonCablingData/Nsw_CablingMap.h>

std::ostream& operator<<(std::ostream& ostr,
                         const NswZebraData& connector) {
    ostr << "First channel: " << connector.firstChannel
         << ", last channel: " << connector.lastChannel
         << " shift: " << connector.shiftChannel;
    return ostr;
}


Nsw_CablingMap::Nsw_CablingMap(const Muon::IMuonIdHelperSvc* svc)
    : m_idHelperSvc{svc} {}

std::optional<Identifier> Nsw_CablingMap::correctChannel(const Identifier& id, 
                                                         MsgStream& msg) const {
    bool debug = msg.level() <= MSG::DEBUG;

    const MmIdHelper& mmHelper{m_idHelperSvc->mmIdHelper()};
    const sTgcIdHelper& sTgcHelper{m_idHelperSvc->stgcIdHelper()};
    LookUpMap::const_iterator itr = m_cablingMap.find(m_idHelperSvc->layerId(id));
    if (itr == m_cablingMap.end()) {
        if (debug)
            msg << MSG::DEBUG << "The gas gap " << m_idHelperSvc->toString(id)
                << " has no known cabling correction" << endmsg;
        return std::make_optional<Identifier>(id);
    }
    const NswZebraSet& correctionSet{itr->second};
    const int channel = m_idHelperSvc->isMM(id) ? mmHelper.channel(id) :
                                                  sTgcHelper.channel(id);
    NswZebraSet::const_iterator zebra_conn = correctionSet.find(channel);
    if (zebra_conn == correctionSet.end()) {
        if (debug)
            msg << MSG::DEBUG << "The zebra connector associated to "
                << m_idHelperSvc->toString(id)
                << " was mounted correctly -> no correction needed " << endmsg;
        return std::make_optional<Identifier>(id);
    }
    const int newChannel = channel + zebra_conn->shiftChannel;
    if (newChannel < zebra_conn->firstChannel ||
        newChannel > zebra_conn->lastChannel) {
        if (debug)
            msg << MSG::DEBUG << "The associated channel "
                << m_idHelperSvc->toString(id)
                << " is shifted outside of the zebra connector range: "
                << newChannel << ". Mask the channel." << endmsg;
        return std::nullopt;
    }
    if (m_idHelperSvc->isMM(id)) {
        const Identifier newId = mmHelper.channelID(id, 
                                            mmHelper.multilayer(id), 
                                            mmHelper.gasGap(id), 
                                            newChannel);
        if (debug)
            msg << MSG::DEBUG << "The input identifier "
                << m_idHelperSvc->toString(id) << " is shifted to a new one "
                << m_idHelperSvc->toString(newId) << endmsg;
        return std::make_optional<Identifier>(std::move(newId));
    }
    const Identifier newId = sTgcHelper.channelID(id, 
                                            sTgcHelper.multilayer(id), 
                                            sTgcHelper.gasGap(id), 
                                            sTgcHelper.channelType(id),
                                            newChannel);
    if (debug)
        msg << MSG::DEBUG << "The input identifier "
            << m_idHelperSvc->toString(id) << " is shifted to a new one "
            << m_idHelperSvc->toString(newId) << endmsg;
        
    return std::make_optional<Identifier>(std::move(newId));
}
bool Nsw_CablingMap::addConnector(const Identifier& gapID,
                                  const NswZebraData& connector,
                                  MsgStream& msg) {
    if (connector.firstChannel >= connector.lastChannel ||
        !connector.shiftChannel) {
        msg << MSG::ERROR << "Invalid zebra definition has been parsed for "
            << m_idHelperSvc->toString(gapID) << ". " << connector << endmsg;
        return false;
    }
    NswZebraSet& correctionSet = m_cablingMap[gapID];
    if (correctionSet.count(connector.firstChannel) ||
        correctionSet.count(connector.lastChannel)) {
        msg << MSG::ERROR << "Zebra " << m_idHelperSvc->toString(gapID)
            << " is already partially covered " << connector << endmsg;
        return false;
    }
    msg << MSG::DEBUG << "Add new zebra connector " << connector << " to "
        << m_idHelperSvc->toString(gapID) << "." << endmsg;
    correctionSet.insert(connector);
    return true;
}