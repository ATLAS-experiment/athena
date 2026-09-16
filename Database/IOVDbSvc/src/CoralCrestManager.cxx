/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// @file CoralCrestManager.cxx
// Implementation for CrestFunctions utilities
// @author Evgeny Alexandrov
// @date 24 February 2025

#include "CxxUtils/checker_macros.h"
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
#include <chai/Converter.h>
#include <chai/Types.h>
#include <typeinfo>
namespace{
  // The COOL storage type whose C++ type CORAL should use for each CHAI payload type.
  // Going through COOL keeps the CORAL column types identical to those the folder
  // carried in COOL. Only one representative per CHAI type is needed: an
  // AttributeListSpecification records the C++ type alone, and every String variant
  // yields std::string while every Blob variant yields coral::Blob, so the COOL width
  // never reaches CORAL.
  const std::map<chai::Type, cool::StorageType::TypeId> coolTypeForChaiType={
      {chai::Bool, cool::StorageType::Bool},
      {chai::UInt8, cool::StorageType::UChar},
      {chai::Int16, cool::StorageType::Int16},
      {chai::UInt16, cool::StorageType::UInt16},
      {chai::Int32, cool::StorageType::Int32},
      {chai::UInt32, cool::StorageType::UInt32},
      {chai::UInt64, cool::StorageType::UInt63},
      {chai::Int64, cool::StorageType::Int64},
      {chai::Float, cool::StorageType::Float},
      {chai::Double, cool::StorageType::Double},
      {chai::String, cool::StorageType::String16M},
      {chai::Blob, cool::StorageType::Blob128M}
    };

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief  Find the C++ type CORAL uses for a payload spec column
  /// @param  type - CHAI type parsed from the CREST tag's payload spec
  /// @return Pointer to the type_info an AttributeListSpecification is extended
  ///         with, or nullptr if the type has no CORAL equivalent.
  ////////////////////////////////////////////////////////////////////////////////
  const std::type_info * coralTypeFor(chai::Type type){
    // Int8 is the one CHAI type COOL cannot express, so it never arrives from a
    // COOL-migrated tag. Map it directly for the sake of natively created CREST tags.
    if (type == chai::Int8) return &typeid(char);
    auto it = coolTypeForChaiType.find(type);
    if (it == coolTypeForChaiType.end()) return nullptr;
    return &cool::StorageType::storageType(it->second).cppType();
  }

    const std::string colonDelimiter{" : "};
}

 CoralCrestManager::CoralCrestManager(const std::string & crest_path, const std::string & crestTag):m_crestTag(crestTag),m_id(Crest::ModeId::Standard){ //AthMessaging("CoralCrestManager")
    if(crest_path.length()==0)
      return;
    if (crest_path.starts_with(CoralCrestManager::prefix1) || crest_path.starts_with(CoralCrestManager::prefix2)){
      m_crestCl = std::make_unique<Crest::CrestApi>(Crest::CrestApi(crest_path));
    }
    else{
      m_crestCl = std::make_unique<Crest::CrestApiFs>(Crest::CrestApiFs(false,crest_path));
    }
  }

  std::map<std::string, std::string> CoralCrestManager::getGlobalTagMap(const std::string & crest_path, const std::string& globaltag){
    std::unique_ptr<Crest::CrestApiBase> crestCl;
    if (crest_path.starts_with(CoralCrestManager::prefix1) || crest_path.starts_with(CoralCrestManager::prefix2)){
      crestCl.reset(new Crest::CrestApi(crest_path));
    }
    else{
      crestCl.reset(new Crest::CrestApiFs(true,crest_path));
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
      // Match on any string width: a POOL reference is String4k in COOL, but a CREST
      // payload spec may name the type generically as "String".
      if(p.first=="PoolRef" && chai::typeFromString(p.second)==chai::String){
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
      const std::type_info * coralType = coralTypeFor(chai::typeFromString(p.second));
      if (coralType == nullptr){
        // Skipping the column instead would leave an AttributeList missing a field that
        // the consuming algorithm looks up by name, which surfaces far downstream as an
        // opaque CORAL "variable does not exist" error. Fail at the source instead.
        spec->release();
        const std::string errorMessage("Unsupported type \"" + p.second + "\" for payload spec column \"" + p.first + "\" of CREST tag " + m_crestTag);
        MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
        gLog << MSG::ERROR << "getAttributeListSpec: " << errorMessage << endmsg;
        throw std::runtime_error(errorMessage);
      }
      spec->extend(p.first,*coralType);
    }
    return spec;
  }
  void CoralCrestManager::initChaiContainer(){
    if(m_chai_cont.has_value()){
      return;
    }
    Crest::TagInfoDto info = getTagInfoDto();
    Crest::TagDto tag = getTagDto();
    const std::string & objectType = tag.getObjectType();
    if(objectType=="crest-json-multi-iov-sparse"){
      m_id=Crest::ModeId::DCS_FULL_SPARSE;
    }
    else if(objectType=="crest-json-multi-iov"){
      // Transitional heuristic, mirroring chai::Tag::initConverterFromObjectType():
      // a multi-channel block in the unsuffixed format is delta encoded all the same,
      // so it still needs the sparse converter. Only a single-channel block is dense.
      // Retire this branch once every such tag in CREST carries the "-sparse" suffix
      // and the objectType alone is authoritative.
      const bool multiChannel = info.getChannels().getChannels().size() > 1;
      m_id = multiChannel ? Crest::ModeId::DCS_FULL_SPARSE : Crest::ModeId::DCS_FULL;
    }
    m_chai_cont.emplace();
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
    initChaiContainer();	  
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
    initChaiContainer();
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
      Crest::TagInfoDto info = getTagInfoDto();
      nlohmann::json j_spec=info.getPayloadSpec().toJson();
      nlohmann::json j_chs=info.getChannels().toJson();
      chai::PayloadSpec chaiSpec(j_spec,j_chs);
      nlohmann::json j = reply;
      if (j.is_string()){
         std::istringstream ss(to_string(j));
         std::string st;
         ss >> std::quoted(st);
         j = json::parse(st);
      }
      m_since=since;
      if(m_id==Crest::ModeId::Standard){
      	if(isVectorPayload()){
          std::shared_ptr<chai::VectorContainer> cont = std::make_shared<chai::VectorContainer>(chai::VectorContainer::fromJson(j,chaiSpec));
          m_chai_cont->insert(std::pair<uint64_t,chai::ContainerBasePtr>(since,cont));
    	}
	else{
          std::shared_ptr<chai::Container> cont = std::make_shared<chai::Container>(chai::Container::fromJson(j,chaiSpec));
          m_chai_cont->insert(std::pair<uint64_t,chai::ContainerBasePtr>(since,cont));
  	}
	std::vector<uint64_t> res;
	res.push_back(m_since);
	return res;
      }
      else{
        std::unique_ptr<chai::ClobMultiIovConverter> converter;
        if(m_id==Crest::ModeId::DCS_FULL_SPARSE)
          converter = std::make_unique<chai::JsonMultiIovSparseConverter>(chaiSpec);
        else
          converter = std::make_unique<chai::JsonMultiIovConverter>(chaiSpec);
        std::unique_ptr<chai::ContainerMapBase> chai_map_cont = converter->deserialize(j.dump());
        std::vector<uint64_t> res=chai_map_cont->keys();
        for(auto const& key: res){
	  chai::ConstContainerPtr const_cont = chai_map_cont->getContainer(key);
	  // Share the map's control block rather than building a second one over the same
	  // object: chai_map_cont is destroyed on leaving this scope, and a shared_ptr built
	  // from the bare pointer would own the Container all over again and free it twice.
	  // Dropping const is safe because every consumer of m_chai_cont only reads. It has
	  // to stay read-only: a sparse block shares its Values between sub-IOVs, so writing
	  // through one Container would silently alter the others.
	  chai::ContainerPtr cont ATLAS_THREAD_SAFE = std::const_pointer_cast<chai::Container>(const_cont);
	  m_chai_cont->insert(std::pair<uint64_t,chai::ContainerBasePtr>(key,cont));
	}
        return res;
      }
    } catch (std::exception & e){ 
	MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
        gLog << MSG::ERROR << "LoadPayloadForHash:"<<e.what()<<" while trying to parse the payload. Since="<<since<<", hash="<<hash<<endmsg;
        throw std::runtime_error(e.what());
    }
  }

  //put payload for selected since in json string
  std::string CoralCrestManager::dumpPayload(cool::ValidityKey since){
    std::vector<std::string> chIds=channelIds(since);
    IOVDbNamespace::FolderType ftype=determineFolderType();
    std::stringstream res;
    res<<"[";
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
        const auto separatorPosition = str.find(colonDelimiter);
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
    chai::ContainerBasePtr cont=m_chai_cont->operator[](m_since);
    if(cont==nullptr){
       std::string errorMessage("Timestamp not found! timestamp=" + std::to_string(m_since));
       MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
       gLog << MSG::ERROR << "getPayload:" <<errorMessage<<endmsg;
       throw std::runtime_error(errorMessage);
    }
    uint64_t id = std::stoul(chId);
    chai::Container* cont2 = dynamic_cast<chai::Container*>(cont.get());
    chai::Values& row = cont2->at(id);
    return createAttributeList(pSpec,row);

  }

  std::vector<coral::AttributeList> CoralCrestManager::getVectorPayload(coral::AttributeListSpecification*  pSpec,const std::string & chId){
    std::vector<coral::AttributeList> res;
    uint64_t id = std::stoul(chId);
    chai::ContainerBasePtr cont=m_chai_cont->operator[](m_since);
    if(cont==nullptr){
      std::string errorMessage("Timestamp not found! timestamp=" + std::to_string(m_since));
      MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
      gLog << MSG::ERROR << "getPayload:" <<errorMessage<<endmsg;
      throw std::runtime_error(errorMessage);
    }
    chai::VectorContainer* cont2 = dynamic_cast<chai::VectorContainer*>(cont.get());
    const std::vector<chai::ValuesPtr>& rows = cont2->rows(id);
    for (auto &row : rows){
      chai::Values* val = row.get();
      coral::AttributeList att=createAttributeList(pSpec,*val);
      res.push_back(att);
    }
    return res;
  }

  void CoralCrestManager::selectIov(cool::ValidityKey since){
    m_since=since;
  }

  std::vector<std::string> CoralCrestManager::channelIds(cool::ValidityKey since){
    selectIov(since);
    std::vector<std::string>	chIds;
    std::vector<uint64_t> channels;
    chai::ContainerBasePtr  cont=m_chai_cont->operator[](since);
    if(cont==nullptr)
      return chIds;
    if(chai::Container* v = dynamic_cast<chai::Container*>(cont.get())) 
      channels=v->channelIds();
    else if(chai::VectorContainer* v = dynamic_cast<chai::VectorContainer*>(cont.get()))
      channels=v->channelIds();    
    //channels=cont->channelSpec().ids();    
    /*if(isVectorPayload()){
      ContainerBasePtr  cont=m_chai_cont->getVectorContainer(since);
      if(cont==nullptr)
        return chIds;
      channels = cont->channelIds();     
    }
    else{
      chai::ConstContainerPtr cont=m_chai_cont->getContainer(since);
      if(cont==nullptr)
        return chIds;
      channels = cont->channelIds();		;      
    }*/
    for (auto id : channels) {
      //if(cont.get()->at(id).size()==0)
	//continue;
      chIds.push_back(std::to_string(id));
    }
    return chIds;
  }

  coral::AttributeList CoralCrestManager::createAttributeList(coral::AttributeListSpecification * pSpec, chai::Values& row){
    coral::AttributeList attr(*pSpec,true);
    unsigned int s=attr.size();
    for (unsigned int i(0);i!=s;++i){
      //cool::Record does not provide non-const access to AttributeList.
      // But this is safe because we are filling a local instance.	    
      auto & att ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(attr[i]);
      if (row.isNull(i)){
        att.setNull();
        continue;
      }
      chai::Type type = row.type(att.specification().name());
      switch (type) {
        case chai::Bool:
        {
          att.setValue<bool>(row.get<bool>(att.specification().name()));
          break;
        }
        case chai::Int8:
	{
	  att.setValue<char>(row.get<int8_t>(att.specification().name()));
	  break;
	}
	case chai::UInt8:
	{
	  att.setValue<unsigned char>(row.get<uint8_t>(att.specification().name()));
	  break;
	}
	case chai::UInt16:
	{
	  att.setValue<unsigned short>(row.get<uint16_t>(att.specification().name()));
	  break;
	}
	case chai::Int16:
	{
	  att.setValue<short>(row.get<int16_t>(att.specification().name()));
	  break;
	}
        case chai::UInt32:
        {
          att.setValue<unsigned int>(row.get<uint32_t>(att.specification().name()));
          break;
        }
        case chai::Int32:
        {
          att.setValue<int>(row.get<int32_t>(att.specification().name()));
          break;
        }
        case chai::UInt64:
        {
          att.setValue<unsigned long long>(row.get<uint64_t>(att.specification().name()));
          break;
        }
        case chai::Int64:
        {
          att.setValue<long long>(row.get<int64_t>(att.specification().name()));
          break;
        }
        case chai::Float:
        {
          att.setValue<float>(row.get<float>(att.specification().name()));
          break;
        }
        case chai::Double:
        {
          att.setValue<double>(row.get<double>(att.specification().name()));
          break;
        }
        case chai::String:
        {
          att.setValue<std::string>(row.get<std::string>(att.specification().name()));
          break;
        }
        case chai::Blob:
        {
          const auto &charVec = row.get<chai::BlobData>(att.specification().name()).m_bytes;//CxxUtils::base64_decode(strVal);
          coral::Blob blob(charVec.size());
          if (!charVec.empty()) {  // Avoid ubsan warning.
            memcpy(blob.startingAddress(), charVec.data(), charVec.size());
          }
          att.setValue<coral::Blob>(blob);		  
          break;
        }
        case chai::Unknown:
        {
          std::string errorMessage("UNTREATED TYPE! " + std::to_string(type));
          MsgStream gLog(Athena::getMessageSvc(), "CoralCrestManager");
          gLog << MSG::ERROR << "LoadPayloadForHash:" <<errorMessage<<endmsg;
          throw std::runtime_error(errorMessage);
        }
      }
    }
    return attr; 
  }




