/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
#include "BasicFolder.h"
#include <iostream>
#include "FolderTypes.h"
#include "IOVDbStringFunctions.h"
#include "IOVDbJsonStringFunctions.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/Attribute.h"

namespace IOVDbNamespace{

static const std::pair<cool::ValidityKey, cool::ValidityKey> infiniteIov(0,cool::ValidityKeyMax);
static const coral::AttributeList invalidPayload{};
static const std::vector<coral::AttributeList> invalidVectorPayload{};


  BasicFolder::BasicFolder():
    m_payload{},
    m_vectorPayload{},
    m_name2Channel{},
    m_channel2Name{},
    m_isVectorPayload{},
    m_channels{},
    m_iov{infiniteIov} {

  }
    bool 
    BasicFolder::empty() const{
     return m_channels.empty();
    }
  
    void 
    BasicFolder::setVectorPayloadFlag(const bool flag){
      m_isVectorPayload=flag;
    }
    
    bool 
    BasicFolder::isVectorPayload() const {
      return m_isVectorPayload;
    }
    //
    void 
    BasicFolder::setIov(const std::pair<cool::ValidityKey, cool::ValidityKey> & iov){
      m_iov=iov;
    }
  
    //add attributeList
    void 
    BasicFolder::addChannelPayload(const cool::ChannelId & channelId, const std::string & name, const coral::AttributeList & payload){
      m_name2Channel[name]=channelId;
      m_channel2Name[channelId]=name;
      addChannelPayload(channelId, payload);
    }
  
    void 
    BasicFolder::addChannelPayload(const cool::ChannelId & channelId, const coral::AttributeList& payload){

      m_channels.push_back(channelId);
      m_payload[channelId]=payload;
    }
  
    //add vector payload
    void 
    BasicFolder::addChannelPayload(const cool::ChannelId & channelId, const std::string & name, const std::vector<coral::AttributeList> & payload){ 
      m_name2Channel[name]=channelId;
      m_channel2Name[channelId]=name;
      addChannelPayload(channelId, payload);
    }
  
    void 
    BasicFolder::addChannelPayload(const cool::ChannelId & channelId, const std::vector<coral::AttributeList> & payload){ 
      m_channels.push_back(channelId);
      m_vectorPayload[channelId]=payload;
    }
  
    //
    coral::AttributeList 
    BasicFolder::getPayload(const cool::ChannelId & channelId){ 
      return m_payload.find(channelId)->second;
    }
  
    coral::AttributeList 
    BasicFolder::getPayload(const std::string & channelName){
      if (m_isVectorPayload) return invalidPayload;
      const auto chanPtr= m_name2Channel.find(channelName);
      bool nameExists = (chanPtr != m_name2Channel.end());
      if (not nameExists) return invalidPayload;
      const cool::ChannelId channel=chanPtr->second;
      return getPayload(channel);
    }
  
    std::vector<coral::AttributeList> 
    BasicFolder::getVectorPayload(const cool::ChannelId & channelId){
      return m_vectorPayload.find(channelId)->second;
    }
  
    std::vector<coral::AttributeList> 
    BasicFolder::getVectorPayload(const std::string & channelName){
      if (not m_isVectorPayload) return invalidVectorPayload;
      const auto chanPtr= m_name2Channel.find(channelName);
      bool nameExists = (chanPtr != m_name2Channel.end());
      if (not nameExists) return invalidVectorPayload;
      const cool::ChannelId channel=chanPtr->second;
      return getVectorPayload(channel);
    }
  
    std::vector<cool::ChannelId> 
    BasicFolder::channelIds(){ 
      return m_channels;
    }
  
    std::pair<cool::ValidityKey, cool::ValidityKey> 
    BasicFolder::iov(){ 
      return m_iov;
    }
    std::string BasicFolder::jsonPayload(const std::string & folderDescription, const std::string & spec){
       IOVDbNamespace::FolderType ftype = determineFolderType(folderDescription,spec,m_channels);
       std::string sep="";
       std::string result("\"data_array\" : [");
       std::sort(m_channels.begin(), m_channels.end());
       for (auto &chId : m_channels){
         result+=sep;
         result+=s_openJson+quote(std::to_string(chId))+" : ";
         switch  (ftype){
           case IOVDbNamespace::CoolVector:
           {
             const std::vector<coral::AttributeList>& vAtt = getVectorPayload(chId);
             std::string os="";
             std::string sep="";
             os+='[';//vector of vectors
             for (auto &att : vAtt){
               os+=sep;
               os+=jsonAttributeList(att);
               if (sep.empty()) sep =s_delimiterJson;
             }
             os+=']';
             result+=os;
             break;
           }
           case IOVDbNamespace::AttrList:
           case IOVDbNamespace::AttrListColl:
           case IOVDbNamespace::PoolRefColl:
           {
             const coral::AttributeList& att = getPayload(chId);
             result +=jsonAttributeList(att);
             break;
           }
           case IOVDbNamespace::PoolRef:
           {
              const coral::AttributeList& att = getPayload(chId);
              std::ostringstream os;
              att[0].toOutputStream(os);
              auto res=os.str();
              const std::string sep(" : ");
              const auto separatorPosition = res.find(sep);
              const std::string payloadOnly=res.substr(separatorPosition+3);
              result += quote(payloadOnly);
              break;
           }
           case IOVDbNamespace::CoraCool:
           {
             result += " CoraCool";
             break;
           }
           default:
              result+=" a_data_value";
           }
           if (sep.empty()) sep=",";
           result+=s_closeJson;
        }
        result+=']';
        return result;
     }
 
  
  }//namespace end

