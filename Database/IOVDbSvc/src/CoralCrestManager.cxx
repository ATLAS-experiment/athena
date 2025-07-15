/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// @file CoralCrestManager.cxx
// Implementation for CrestFunctions utilities
// @author Evgeny Alexandrov
// @date 24 February 2025

#include "CoralCrestManager.h"
#include "CrestApi/CrestApi.h"
#include "CrestApi/CrestApiFs.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoolKernel/StorageType.h"
#include "CoolKernel/RecordSpecification.h"
#include "CoolKernel/Record.h"
#include "IOVDbJsonStringFunctions.h"
#include "CxxUtils/base64.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"

  const std::map<std::string, cool::StorageType::TypeId> typeCorrespondance={
      {"Bool", cool::StorageType::Bool},
      {"UChar",cool::StorageType::UChar},
      {"Int16", cool::StorageType::Int16},
      {"UInt16", cool::StorageType::UInt16},
      {"Int32", cool::StorageType::Int32},
      {"UInt32", cool::StorageType::UInt32},
      {"UInt63",cool::StorageType::UInt63},
      {"Int64", cool::StorageType::Int64},
      {"Float", cool::StorageType::Float},
      {"Double", cool::StorageType::Double},
      {"String255", cool::StorageType::String255},
      {"String4k", cool::StorageType::String4k},
      {"String64k", cool::StorageType::String64k},
      {"String16M", cool::StorageType::String16M},
      {"String128M", cool::StorageType::String128M},
      {"Blob64k", cool::StorageType::Blob64k},
      {"Blob16M", cool::StorageType::Blob16M},
      {"Blob128M", cool::StorageType::Blob128M}
    };

 CoralCrestManager::CoralCrestManager(const std::string & crest_path, const std::string & crestTag):m_crestTag(crestTag){ //AthMessaging("CoralCrestManager")
    if(crest_path.length()==0)
      return;
    if (crest_path.starts_with(CoralCrestManager::prefix1) || crest_path.starts_with(CoralCrestManager::prefix2)){
      m_crestCl = std::make_unique<Crest::CrestApi>(Crest::CrestApi(crest_path));
    }
    else{
      m_crestCl = std::make_unique<Crest::CrestApiFs>(Crest::CrestApiFs(true,crest_path));
    }

  }

  std::map<std::string, std::string> CoralCrestManager::getGlobalTagMap(const std::string & crest_path, const std::string& globaltag){
    Crest::CrestApiBase* crestCl=NULL;
    if (crest_path.starts_with(CoralCrestManager::prefix1) || crest_path.starts_with(CoralCrestManager::prefix2)){
      crestCl = new Crest::CrestApi(crest_path);
    }
    else{
      crestCl = new Crest::CrestApiFs(true,crest_path);
    }
    std::map<std::string, std::string> tagmap;
    try{
      Crest::GlobalTagMapSetDto dto = crestCl->findGlobalTagMap(globaltag,"Trace");
      for (const auto &tagMapDto : dto.getResources()){
	tagmap[tagMapDto.getLabel()]=tagMapDto.getTagName();
      }
    } catch (std::exception & e){
      MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
      gLog << MSG::ERROR << __FILE__<<":"<<__LINE__<< ": " << e.what() << " Cannot get a global tag map for " << globaltag<<endmsg;
    }
    if(crestCl!=NULL){
      delete crestCl;
      crestCl=NULL;
    }
    return tagmap;
  }

  std::string CoralCrestManager::parseTypeName(const std::string & description){
    std::string regex=R"delim(<typeName>\s*([^\s]+)\s*</typeName>)delim";
    std::regex re(regex);
    std::smatch typeMatch;
    bool match=std::regex_search(description, typeMatch,re);
    return (match) ? std::string(typeMatch[1]) : std::string("");
  }

 IOVDbNamespace::FolderType CoralCrestManager::determineFolderType(){
    Crest::TagInfoDto info = getTagInfoDto();
    std::string folderDescription = info.getFolderDescription();
    if (folderDescription.find("<coracool>") != std::string::npos) return IOVDbNamespace::CoraCool;
    const std::string typeName = parseTypeName(folderDescription);
    if (typeName=="CondAttrListVec"){
        m_isVectorPayload = true;
        return IOVDbNamespace::CoolVector;
    }
    m_isVectorPayload = false;
    std::vector< std::pair<std::string,std::string> > spec= info.getPayloadSpec().getColumns();
    std::vector< std::pair<std::string,std::string> > chs = info.getChannels().getChannels();
    for (auto &p : spec){
      if(p.first=="PoolRef" && p.second=="String4k"){
        int id=std::stoll(chs[0].first);
        if(chs.size()==1 && id==0)
          return IOVDbNamespace::PoolRef;
        else
          return IOVDbNamespace::PoolRefColl;
      }
    }
    if (typeName == "CondAttrListCollection") return IOVDbNamespace::AttrListColl;
    return IOVDbNamespace::AttrList;
  }

  bool CoralCrestManager::isVectorPayload(){
    if(!m_isVectorPayload.has_value())
       determineFolderType();
    return m_isVectorPayload.value();
  }

  void CoralCrestManager::loadTagInfo(){
    if(m_TagMeta.has_value()){
      return;
    }

    try{
      m_TagMeta = m_crestCl->findTagMeta(m_crestTag);
    } catch (std::exception & e){
      MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
      gLog << MSG::ERROR << __FILE__<<":"<<__LINE__<< ": " << e.what() << " Cannot get a tag meta info " << m_crestTag<<endmsg;
    }
    return;
  }

  Crest::TagInfoDto CoralCrestManager::getTagInfoDto(){
    if(!m_TagMeta.has_value())
      loadTagInfo();
    return m_TagMeta.value().getTagInfoDto();
  }

  Crest::TagDto& CoralCrestManager::getTagDto(){
    if(!m_Tag.has_value()){
      m_Tag = m_crestCl->findTag(m_crestTag);
    }
    return m_Tag.value();
  }

  std::string CoralCrestManager::getPayloadSpec(){
    return getTagInfoDto().getPayloadSpec().toJson().dump();
  }

  std::string CoralCrestManager::getFolderDescription(){
    Crest::TagInfoDto info = getTagInfoDto();
    return info.getFolderDescription();
  }

  std::pair<std::vector<cool::ChannelId> , std::vector<std::string>> CoralCrestManager::getChannelList(){
    if(!m_TagMeta.has_value())
      loadTagInfo();
    Crest::TagInfoDto info = getTagInfoDto();
    std::vector<cool::ChannelId> list;
    std::vector<std::string> names;
    std::vector< std::pair<std::string,std::string> > res = info.getChannels().getChannels();
    for (auto &p : res){
      list.push_back(std::stoll(p.first));
      names.push_back(p.second);
    }
    return std::make_pair(std::move(list), std::move(names));
  }

  coral::AttributeListSpecification* CoralCrestManager::getAttributeListSpec(){
    Crest::TagInfoDto info = getTagInfoDto();
    std::vector< std::pair<std::string,std::string> > spec_vec= info.getPayloadSpec().getColumns();
    auto * spec = new coral::AttributeListSpecification();
    for (auto &p : spec_vec){
      spec->extend(p.first,cool::StorageType::storageType(typeCorrespondance.find(p.second)->second).cppType());
    }
    return spec;
  }

  void CoralCrestManager::initCrestContainer(){
    if(m_crest_cont.has_value())
      return;
    Crest::TagInfoDto info = getTagInfoDto();
    Crest::TagDto tag = getTagDto();
    Crest::ModeId mId=Crest::ModeId::Standard;
    if(tag.getObjectType()=="crest-json-multi-iov")
      mId=Crest::ModeId::DCS_FULL;
    Crest::CrestContainer cr_cont(mId);
    std::vector< std::pair<std::string,std::string> > spec= info.getPayloadSpec().getColumns();
    for (auto &p : spec){
      cr_cont.addColumn(p.first,p.second.c_str());
    }
    if(!m_isVectorPayload.has_value()) determineFolderType();
    cr_cont.setVectorPayload(m_isVectorPayload.value());
    m_crest_cont.emplace(cr_cont);
    return;
  }

  std::pair<uint64_t,uint64_t>
  CoralCrestManager::getSinceUntilPair(std::vector<uint64_t>& v, const uint64_t since, const uint64_t until){
    uint64_t new_since = 0;
    uint64_t new_until = 0;

    if (until < since){
        MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
        gLog << MSG::ERROR << "Wrong since/until."<<endmsg;
        return std::make_pair(0,0);
    }
    const std::size_t N = v.size();
    std::size_t i = 0;
    for (; i < N; ++i) {
      if(v[i] <= since && since < v[i+1]){
        new_since = v[i];
        break;
      }
    }

    for (; i < N; ++i) {
      if(v[i] < until && until <= v[i+1]){
        new_until = v[i+1];
        break;
      }
    }

    return std::make_pair(new_since,new_until);
  }

  std::pair<uint64_t,uint64_t>
  CoralCrestManager::getIovInterval(const std::string&  tag, const uint64_t since, const uint64_t until){
    Crest::IovSetDto dto = m_crestCl->selectGroups(tag, 0, 10000, 0, "id.since:ASC");
    std::vector<uint64_t> v = dto.getListSince();
    v.push_back(cool::ValidityKeyMax); // added "infinity" as the last item  9223372036854775807
    return getSinceUntilPair(v, since, until);
  }

  std::vector<std::pair<cool::ValidityKey,std::string>> CoralCrestManager::getIovsForTag(uint64_t since, uint64_t until){
    initCrestContainer();
    int iovNumber = m_crestCl->getSize(m_crestTag);
    std::vector<std::pair<cool::ValidityKey,std::string>> res;
    Crest::IovSetDto dto;
    if(iovNumber <=1000){
      dto = m_crestCl->selectIovs(m_crestTag, 0, -1, 0, 10000, 0, "id.since:ASC");
    }
    else{
      std::pair<uint64_t,uint64_t> ppt = getIovInterval(m_crestTag, since, until);
      uint64_t s_time = ppt.first;
      uint64_t u_time = ppt.second;

      if (s_time == 0 && u_time == 0){ // data out of range
        return res;
      }
      else {
        dto = m_crestCl->selectIovs(m_crestTag, s_time, u_time, 0, 10000, 0, "id.since:ASC");
      }
    }
    for (auto &p : dto.getResources()){
      res.push_back(std::make_pair((cool::ValidityKey)p.getSince(),p.getPayloadHash()));
    }
    return res;
  }
  
  std::vector<uint64_t> CoralCrestManager::loadPayloadForHash(uint64_t since, const std::string & hash){
    initCrestContainer();
    std::string reply;
    try{
	// get payload from Crest server
        reply = m_crestCl->getPayload(hash);
    } catch (std::exception & e){
	MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
        gLog << MSG::ERROR << __FILE__<<":"<<__LINE__<< ": "<<e.what()<<" while trying to find the payload"<<endmsg;    
        throw std::runtime_error(e.what());
    }
    try{
      // parse payload according to type of payload  and put it to CrestConteiner. 
      // Store only one value before 'since'. 
      // Returns a list of timestamp for which data has been loaded 	    
      return m_crest_cont.value().fromJson(since,reply);
    } catch (std::exception & e){ 
	MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
        gLog << MSG::ERROR << "LoadPayloadForHash:"<<e.what()<<" while trying to parse the payload. Since="<<since<<", hash="<<hash<<endmsg;
        throw std::runtime_error(e.what());
    }
  }

  //put payload for selected since in json string
  std::string CoralCrestManager::dumpPayload(cool::ValidityKey since){
    m_crest_cont.value().selectIov(since);
    IOVDbNamespace::FolderType ftype=determineFolderType();
    std::stringstream res;
    res<<"[";
    std::vector<std::string> chIds = m_crest_cont.value().channelIds();
    auto* pspec=getAttributeListSpec();
    std::string sep="";
    for (auto &ch : chIds){
      res<<sep;
      res<<IOVDbNamespace::s_openJson<<"\""<<ch<<"\" : ";
      switch  (ftype){
      case IOVDbNamespace::CoolVector:
      {
        std::vector<coral::AttributeList> attr=getVectorPayload(pspec,ch);
        res<<"[";
        std::string sep2="";
        for (const auto & vitr:attr){
          res<<sep2<<IOVDbNamespace::jsonAttributeList(vitr);
          if (sep2.empty()) sep2 =IOVDbNamespace::s_delimiterJson;
        }
        res<<"]";
        break;
      }
      case IOVDbNamespace::AttrList:
      case IOVDbNamespace::AttrListColl:
      case IOVDbNamespace::PoolRefColl:
      {
        coral::AttributeList attr=getPayload(pspec,ch);
        res<<IOVDbNamespace::jsonAttributeList(attr);
        break;
      }
      case IOVDbNamespace::PoolRef:
      {
        coral::AttributeList attr=getPayload(pspec,ch);
        std::ostringstream os;
        attr[0].toOutputStream(os);
        auto str=os.str();
        const std::string del(" : ");
        const auto separatorPosition = str.find(del);
        const std::string payloadOnly=str.substr(separatorPosition+3);
        res<<"\""<<payloadOnly<<"\"";
        break;
      }
      case IOVDbNamespace::CoraCool:
        res<< " CoraCool";
        break;
      default:
        res<<" a_data_value";
      }
      if (sep.empty()) sep=",";
      res<<IOVDbNamespace::s_closeJson;
    }
    res<<"]";
    pspec->release();
    return res.str();
  }

  coral::AttributeList CoralCrestManager::getPayload(coral::AttributeListSpecification * pSpec,const std::string & chId){
    nlohmann::json j=m_crest_cont.value().getPayloadChannel(chId.c_str());
    return createAttributeList(pSpec,j,m_crest_cont.value().getMPayloadSpec());
  }

  std::vector<coral::AttributeList> CoralCrestManager::getVectorPayload(coral::AttributeListSpecification*  pSpec,const std::string & chId){
    std::vector<coral::AttributeList> res;
    nlohmann::json vecJ=m_crest_cont.value().getPayloadChannel(chId.c_str());
    for (auto &p : vecJ){
      coral::AttributeList att=createAttributeList(pSpec,p,m_crest_cont.value().getMPayloadSpec());
      res.push_back(att);
    }
    return res;
  }

  void CoralCrestManager::selectIov(cool::ValidityKey since){
    m_crest_cont.value().selectIov(since);
  }

  std::vector<std::string> CoralCrestManager::channelIds(cool::ValidityKey since){
    selectIov(since);
    return m_crest_cont.value().channelIds();
  }

  coral::AttributeList CoralCrestManager::createAttributeList(coral::AttributeListSpecification * pSpec,nlohmann::json& j,const std::vector<std::pair<std::string, Crest::TypeId>> & tSpec){
    coral::AttributeList attr(*pSpec,true);
    unsigned int s=attr.size();

    json::const_iterator it = j.begin();
    for (unsigned int i(0);i!=s;++i){
      // cool::Record does not provide non-const access to AttributeList.
      // But this is safe because we are filling a local instance.    
      auto & att = const_cast<coral::Attribute&>(attr[i]);
      if (it == j.end()){
        continue;
      }
      const auto  thisVal = it.value();
      ++it;

      try{
        if (thisVal.is_null()){
          att.setNull();
          continue;
        }
        cool::StorageType::TypeId typespec=cool::StorageType::Bool;
        for(auto &p : tSpec){
          if(p.first.compare(att.specification().name())==0){
            auto pElement = Crest::s_typeToString.find(p.second);
            if (pElement == Crest::s_typeToString.end()){
              throw std::runtime_error("CoralCrestManager::createAttributeList: name not found.");
            }
            std::string str_spec = pElement ->second;
            auto pTypespec = typeCorrespondance.find(str_spec);
            if (pTypespec == typeCorrespondance.end()){
              throw std::runtime_error("CoralCrestManager::createAttributeList: typespec not found.");
            }
            typespec=pTypespec->second;
            break;
          }
        }
        std::string strVal = to_string(thisVal);
        if(strVal.size()>2&& strVal[0]=='"'&& strVal[strVal.size()-1]=='"')
          strVal=strVal.substr(1,strVal.size()-2);

        if((strVal.compare("NULL")==0||strVal.compare("null")==0)&&
          (typespec==cool::StorageType::Bool || typespec==cool::StorageType::Int16 || typespec==cool::StorageType::UInt16
          || typespec==cool::StorageType::Int32 || typespec==cool::StorageType::UInt32
          || typespec==cool::StorageType::Int64 || typespec==cool::StorageType::UInt63
          || typespec==cool::StorageType::Float || typespec==cool::StorageType::Double)){
          att.setNull();
          continue;
        }
        switch (typespec) {
        case cool::StorageType::Bool:
          {
            const bool newVal=(strVal == "true");
            att.setValue<bool>(newVal);
            break;
          }
        case cool::StorageType::UChar:
          {
            const unsigned char newVal=std::stoul(strVal);
            att.setValue<unsigned char>(newVal);
            break;
          }
        case cool::StorageType::Int16:
          {
            const short newVal=std::stol(strVal);
            att.setValue<short>(newVal);
            break;
          }
        case cool::StorageType::UInt16:
          {
            const unsigned short newVal=std::stoul(strVal);
            att.setValue<unsigned short>(newVal);
            break;
          }
        case cool::StorageType::Int32:
          {
            const int newVal=std::stoi(strVal);
            att.setValue<int>(newVal);
            break;
          }
        case cool::StorageType::UInt32:
          {
            const unsigned int newVal=std::stoull(strVal);
            att.setValue<unsigned int>(newVal);
            break;
          }
        case cool::StorageType::UInt63:
          {
            const  unsigned long long newVal=std::stoull(strVal);
            att.setValue<unsigned long long>(newVal);
            break;
          }
        case cool::StorageType::Int64:
          {
            const  long long newVal=std::stoll(strVal);
            att.setValue< long long>(newVal);
            break;
          }
        case cool::StorageType::Float:
          {
            const  float newVal=std::stof(strVal);
            att.setValue<float>(newVal);
            break;
          }
        case cool::StorageType::Double:
          {
            const  double newVal=std::stod(strVal);
            att.setValue<double>(newVal);
            break;
          }
        case cool::StorageType::String255:
        case cool::StorageType::String4k:
        case cool::StorageType::String64k:
        case cool::StorageType::String16M:
        case cool::StorageType::String128M:
          {
            att.setValue<std::string>(thisVal.get<std::string>());
            break;
          }
        case cool::StorageType::Blob128M:
        case cool::StorageType::Blob16M:
        case cool::StorageType::Blob64k:
          {
            const auto &charVec = CxxUtils::base64_decode(strVal);
            coral::Blob blob(charVec.size());
            memcpy(blob.startingAddress(), charVec.data(), charVec.size());
            att.setValue<coral::Blob>(blob);
            break;
          }
        default:
          {
	    std::string errorMessage("UNTREATED TYPE! " + std::to_string(typespec));  
            MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
	    gLog << MSG::ERROR << "LoadPayloadForHash:" <<errorMessage<<endmsg;
            throw std::runtime_error(errorMessage);
          }
        }
      }
      catch (json::exception& e){
        MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
        gLog << MSG::ERROR << "Error CoralCrestManager::createAttributeList: "<<e.what()<<endmsg;
        throw std::runtime_error(e.what());
      }
    }
    return attr;
  }




