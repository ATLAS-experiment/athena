/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CrestContainer.h"
#include "CrestApi/CrestCondException.h"
#include <cstdarg>
#include <fstream>
#include <iomanip>

using json = nlohmann::json;

Crest::CrestContainer::CrestContainer(Crest::ModeId mode) : m_modeId(mode) { ; }
Crest::CrestContainer::~CrestContainer()
{
  flush();
}

bool Crest::CrestContainer::isVectorPayload()
{
  return m_isVectorPayload;
}
void Crest::CrestContainer::setVectorPayload(bool isVectorPayload)
{
  m_isVectorPayload = isVectorPayload;
}

bool Crest::compareStrTimestamp(std::string& as, std::string& bs) {
    auto a = std::stol(as);
    auto b = std::stol(bs);   
    return a < b;
}

void Crest::CrestContainer::addColumn(const std::string &name, TypeId type)
{
  m_payload_spec.emplace_back(name, type);
}

void Crest::CrestContainer::addColumn(const std::string &name, const std::string& type)
{
	
  for(auto& t:  s_typeToString){
    if(t.second.compare(type)==0){
      addColumn(name, t.first);
      return;
    }
  }
  throw CommonCrestException("The type of parameter is not defined.");	
}

void Crest::CrestContainer::addNullRecord(const std::string&name)
{
  auto it = std::find_if(m_payload_spec.begin(), m_payload_spec.end(),
                         [&name](const auto &column)
                         { return column.first == name; });

  if (it != m_payload_spec.end())
  {
    m_row[name] = nlohmann::json::value_t::null;
  }
}

void Crest::CrestContainer::addRecord(const std::string&name, int number, ...)
{

  va_list ap;
  va_start(ap, number);

  for (auto &column : m_payload_spec)
  {
    if (column.first != name)
    {
      continue;
    }

    switch (column.second)
    {
    case TypeId::Bool:
      m_row[name] = va_arg(ap, int);
      break;
    case TypeId::UChar:
      m_row[name] = static_cast<unsigned char>(va_arg(ap, int));
      break;
    case TypeId::Int16:
    case TypeId::Int32:
      m_row[name] = va_arg(ap, int);
      break;
    case TypeId::UInt16:
    case TypeId::UInt32:
      m_row[name] = va_arg(ap, unsigned int);
      break;
    case TypeId::UInt63:
      m_row[name] = va_arg(ap, uint64_t);
      break;
    case TypeId::Int64:
      m_row[name] = va_arg(ap, int64_t);
      break;
    case TypeId::Float:
    case TypeId::Double:
      m_row[name] = va_arg(ap, double);
      break;
    case TypeId::String255:
    case TypeId::String4k:
    case TypeId::String64k:
    case TypeId::String16M:
    case TypeId::Blob64k:
    case TypeId::Blob16M:
      m_row[name] = std::string(va_arg(ap, const char *));
      break;
    default:
      throw CommonCrestException("Unsupported column type.");
    }
  }
  va_end(ap);
}

void Crest::CrestContainer::addData(const std::string&channel_id)
{
  addExternalData(channel_id, getRow());
}

void Crest::CrestContainer::addExternalData(const std::string& channel_id, const nlohmann::json &data)
{
  nlohmann::json arr_data = m_isVectorPayload ? nlohmann::json::array() : nlohmann::json();
  if (m_isVectorPayload){
    for (const auto &data_row : data)
    {
        arr_data.emplace_back(createRowArray(data_row));
    }
    m_payload[channel_id] = std::move(arr_data);
    }
  else{
    m_payload[channel_id] = createRowArray(data);
  }
  m_vector_data.clear();
  m_row.clear();
}

nlohmann::json Crest::CrestContainer::createRowArray(const nlohmann::json &data_row) const
{
  nlohmann::json row_arr_data = nlohmann::json::array();
  for (const auto &column : m_payload_spec)
  {
    if (data_row.find(column.first) == data_row.end())
    {
      std::string msg = "The data does not contain the column: " + column.first;
      throw CommonCrestException(msg.c_str());
    }
    row_arr_data.push_back(data_row[column.first]);
  }
  return row_arr_data;
}

void Crest::CrestContainer::addIov(const uint64_t since)
{
  m_iov_data["since"] = since;
  m_iov_data["data"] = m_payload;
  m_full_data[since]=m_payload;
  m_payload.clear();
}

void Crest::CrestContainer::selectIov(const uint64_t since){
  m_iov_data["since"]=since;
  if(m_full_data.contains(since)){
    m_iov_data["data"]=m_full_data[since];
    m_payload=m_full_data[since];
  }
  else{
    m_iov_data["data"]={};
    m_payload={};
  }
}

uint64_t Crest::CrestContainer::getFirstTime(){
  if(m_full_data.size()==0)
  {
    throw CommonCrestException("No IOV in CrestContainer");
  }
  return m_full_data.begin()->first;
}

std::vector<std::string> Crest::CrestContainer::channelIds(){
  std::vector<std::string> chs;
  for (auto& x : m_iov_data["data"].items()){
    chs.push_back(x.key());
  }
  sort(chs.begin(), chs.end(), Crest::compareStrTimestamp);
  return chs;
}
const std::vector<std::pair<std::string, Crest::TypeId>> &Crest::CrestContainer::getMPayloadSpec()
{
  return m_payload_spec;
}

const nlohmann::json Crest::CrestContainer::getPayloadChannel(const std::string& channel_id)
{
  if (m_payload.empty())
  {
    m_payload = m_iov_data["data"];
  }
  auto it = m_payload.find(channel_id);
  if (it == m_payload.end())
  {
    std::string msg = "Channel id " + std::string(channel_id) + " is not found.";
    throw CommonCrestException(msg.c_str());
  }
  return it.value();
}

int Crest::CrestContainer::getColumnIndex(const std::string &name)
{
  auto it = std::find_if(m_payload_spec.begin(), m_payload_spec.end(),
                         [&name](const auto &column)
                         { return column.first == name; });

  if (it != m_payload_spec.end())
  {
    return std::distance(m_payload_spec.begin(), it);
  }
  else
  {
    throw CommonCrestException("The column name is not found.");
  }
}

void Crest::CrestContainer::putRow2Vector()
{
  if (m_isVectorPayload)
  { 
    m_vector_data.push_back(m_row);
    m_row.clear();
  }
  else
  {
    std::string msg = "The payload is not a vector.";
    throw CommonCrestException(msg.c_str());
  }
}

const nlohmann::json &Crest::CrestContainer::getRow()
{
  if (m_isVectorPayload)
    return m_vector_data;
  return m_row;
}

const nlohmann::json &Crest::CrestContainer::getPayload()
{
  if (m_payload.empty() && m_iov_data.empty())
  {
    std::string msg = "The payload is empty.";
    throw CommonCrestException(msg.c_str());
  }
  if (m_payload.empty())
  {
    m_payload = m_iov_data["data"];
  }
  return m_payload;
}

const nlohmann::json &Crest::CrestContainer::getIovData()
{
  if (m_iov_data.empty())
  {
    std::string msg = "The iov data is empty.";
    throw CommonCrestException(msg.c_str());
  }
  return m_iov_data;
}

void Crest::CrestContainer::setIovData(const nlohmann::json &j)
{
  m_iov_data = j;
}

void Crest::CrestContainer::setPayload(const nlohmann::json &j)
{
  m_payload = j;
}

std::string Crest::CrestContainer::getJsonPayload()
{
  return getPayload().dump();
}

std::string Crest::CrestContainer::getJsonIovData()
{
  return m_iov_data.dump();
}

json Crest::CrestContainer::getPayloadSpec()
{
  json pspec_data=json::array();
  for (auto &column : m_payload_spec)
  {
    json j={};
    std::map<TypeId, std::string>::const_iterator pos = Crest::s_typeToString.find(column.second);
    if (pos == Crest::s_typeToString.end()) {
      throw CommonCrestException("Type do not exist in the map.");
    } else {
      j[column.first] = pos->second;
    }
    pspec_data.push_back(j);
  }
  return pspec_data;
}

void Crest::CrestContainer::setPayloadSpec(const nlohmann::json &j)
{
  if (j.is_array())
  {
    for (const auto &column : j)
    {
      for (const auto &[name, type] : column.items())
      {
        addColumn(name, static_cast<TypeId>(std::stoi(type.get<std::string>())));
      }
    }
  }
  else
  {
    for (const auto &[name, type] : j.items())
    {
      addColumn(name, static_cast<TypeId>(type.get<int>()));
    }
  }
}

void Crest::CrestContainer::flush()
{
  m_iov_data.clear();
  m_row.clear();
  m_vector_data.clear();
  m_full_data={};
}

void Crest::CrestContainer::clear(){
  m_payload.clear();
  m_iov_data.clear();
  m_row.clear();
  m_vector_data.clear();
  m_full_data={};
}

// Function to dump JSON object into a file
void Crest::CrestContainer::dumpJsonToFile(const nlohmann::json &j, const std::string &filename)
{
  std::ofstream file(filename);
  if (file.is_open())
  {
    file << std::setprecision(6) << j.dump(4);
    file.close();
  }
  else
  {
    std::cerr << "CondContainer::dumpJsonToFile: Error opening file: " << filename << std::endl;
    throw std::runtime_error("CondContainer::dumpJsonToFile: Error opening file.");
  }
}

// Function to read file and create JSON object
nlohmann::json Crest::CrestContainer::readJsonFromFile(const std::string &filename, const std::string &spec_filename)
{

  std::ifstream specfile(spec_filename);
  nlohmann::json jspec;
  if (specfile.is_open())
  {
    specfile >> jspec;
    specfile.close();
  }
  else
  {
    std::cerr << "CondContainer::readJsonFromFile: Error opening file: " << spec_filename << std::endl;
    throw std::runtime_error("CondContainer::readJsonFromFile: Error opening file. "); 
  }
  // Set the payload spec
  setPayloadSpec(jspec);
  // Read data file
  std::ifstream file(filename);
  nlohmann::json j;
  if (file.is_open())
  {
    file >> j;
    file.close();
  }
  else
  {
    std::cerr << "CondContainer::readJsonFromFile: Error opening file: " << filename << std::endl;
    throw std::runtime_error("CondContainer::readJsonFromFile: Error opening file. ");
  }
  return j;
}

void Crest::CrestContainer::parseOldFormat(const std::string& colName, const TypeId& typespec,const nlohmann::json & thisVal){
  try{
    if (thisVal.is_null()){
      m_row[colName] ="NULL";
      return;
    }
    std::string strVal = to_string(thisVal);
    if(strVal.size()>2&& strVal[0]=='"'&& strVal[strVal.size()-1]=='"')
    strVal=strVal.substr(1,strVal.size()-2);
    if((strVal.compare("NULL")==0||strVal.compare("null")==0)&&
          (typespec==TypeId::Bool || typespec==TypeId::Int16 || typespec==TypeId::UInt16
          || typespec==TypeId::Int32 || typespec==TypeId::UInt32
          || typespec==TypeId::Int64 || typespec==TypeId::UInt63
          || typespec==TypeId::Float || typespec==TypeId::Double)){
          m_row[colName] ="NULL";
      return;
    }
    switch (typespec) {
	case TypeId::Bool:
	  {
	    const bool newVal=(strVal == "true");
	    m_row[colName] = newVal;
	    break;
	  }
	case TypeId::UChar:
	  {
	    m_row[colName]=std::stoul(strVal);
	    break;
	  }
	case TypeId::Int16:
	  {
	    m_row[colName]=std::stol(strVal);
	    break;
	  }
	case TypeId::UInt16:
	  {
	    m_row[colName]=std::stoul(strVal);
	    break;
	  }
	case TypeId::Int32:
	  {
	    m_row[colName]=std::stoi(strVal);
	    break;
	  }
	case TypeId::UInt32:
	  {
	    m_row[colName]=std::stoull(strVal);
	    break;
	  }
	case TypeId::UInt63:
	  {
	    m_row[colName]=std::stoull(strVal);
	    break;
	  }
	case TypeId::Int64:
	  {
	    m_row[colName]=std::stoll(strVal);
	    break;
	  }
	case TypeId::Float:
	  {
	    m_row[colName]=std::stof(strVal);
	    break;
	  }
	case TypeId::Double:
	  {
	    m_row[colName]=std::stod(strVal);
	    break;
	  }
	case TypeId::String255:
	case TypeId::String4k:
	case TypeId::String64k:
	case TypeId::String16M:
	case TypeId::String128M:
	  {
	    m_row[colName]=thisVal.get<std::string>();
	    break;
	  }
	case TypeId::Blob128M:
	case TypeId::Blob16M:
	case TypeId::Blob64k:
	  {
            m_row[colName]=thisVal.get<std::string>();
	    break;
	  }
	default:
	  {
	    throw std::runtime_error("UNTREATED TYPE!");
	  }
	}

  }
  catch(json::exception& e){
    std::cerr << e.what() << std::endl;
    throw std::runtime_error(e.what());	  
  }
}
void Crest::CrestContainer::parseData(const nlohmann::json & values){
        if (values.is_array() && values.size() == m_payload_spec.size())
        {
          for (size_t i = 0; i < values.size(); ++i)
          {
            const auto & [colName,colType] = m_payload_spec[i];
            //const std::string & colName = spec.first;
            //TypeId colType = spec.second;
            if(values[i].is_string() &&( colType== TypeId::UChar || colType==TypeId::Bool || colType==TypeId::Int16 || colType==TypeId::UInt16
             || colType==TypeId::Int32 || colType==TypeId::UInt32 || colType==TypeId::Int64 || colType==TypeId::UInt63 || colType==TypeId::Float || colType==TypeId::Double))
	    {
	      parseOldFormat(colName,colType,values[i]);
	      continue;
	    }
	    if(values[i].is_null()){
	      m_row[colName] =  nullptr;
              continue;	      
	    }
            switch (colType)
            {
            case TypeId::Bool:
              m_row[colName] = values[i].get<bool>();
              break;
            case TypeId::UChar:
              m_row[colName] = values[i].get<unsigned char>();
              break;
            case TypeId::Int16:
            case TypeId::Int32:
              m_row[colName] = values[i].get<int>();
              break;
            case TypeId::UInt16:
            case TypeId::UInt32:
              m_row[colName] = values[i].get<unsigned int>();
            break;
            case TypeId::UInt63:
              m_row[colName] = values[i].get<uint64_t>();
              break;
            case TypeId::Int64:
              m_row[colName] = values[i].get<int64_t>();
              break;
            case TypeId::Float:
            case TypeId::Double:
              m_row[colName] = values[i].get<double>();
              break;
            case TypeId::String255:
            case TypeId::String4k:
            case TypeId::String64k:
            case TypeId::String16M:
            case TypeId::String128M:
              m_row[colName] = values[i].get<std::string>();
              break;
            case TypeId::Blob128M:
            case TypeId::Blob64k:
            case TypeId::Blob16M:
              m_row[colName] = values[i].get<std::string>();
              break;
            default:
              throw CommonCrestException("CrestContainer::parseData: Unsupported column type.");
            }
	  }
        }
	else
	{
	  if(!values.is_array())
	    std::cerr << "CrestContainer::parseData values is not array"<<std::endl;
          else
            std::cerr << "CrestContainer::parseData values number="<<values.size() << " m_payload_spec.size()="<<m_payload_spec.size()<<std::endl;		  
	  throw CommonCrestException("CrestContainer::parseData: Mismatch in number of values.");	
	}

}
std::vector<uint64_t> Crest::CrestContainer::fromJson(uint64_t since,const nlohmann::json &j_in){
  if(m_modeId==Crest::ModeId::Standard)
  { 
    nlohmann::json j = j_in;
    if (j.is_string()){
      std::istringstream ss(to_string(j));
      std::string st;
      ss >> std::quoted(st);
      j = json::parse(st);
    }
    // Accessing the "data" object
    if (j.contains("data") && j["data"].is_object())
    {
      //const auto &data = j["data"];	  
      readCommonType(since,j_in);
    }
    else if(j.is_object()){
      readCommonType(since,j);	    
    }
    else
    {
      std::cerr << "CrestContainer::fromJson json:"<<j<<std::endl;
      throw CommonCrestException("CrestContainer::fromJson: JSON  is not a JSON object.");
    }
    std::vector<uint64_t> ret;
    ret.push_back(since);
    return ret;
  }
  else if(m_modeId==Crest::ModeId::DCS_FULL)
  {
    return readDcsFullType(j_in);
  }
  else
  {
    throw CommonCrestException("CrestContainer::fromJson: Unsupported type of payload.");	  
  }  
}
std::vector<uint64_t> Crest::CrestContainer::readDcsFullType(const nlohmann::json &j_in)
{
  nlohmann::json j = j_in;
  if (j.is_string()){
    std::istringstream ss(to_string(j));
    std::string st;
    ss >> std::quoted(st);
    j = json::parse(st);
  }	
  nlohmann::json dcs_data;
  if (j.contains("data") && j["data"].is_object())
  {
    dcs_data = j["data"];
  }
  else
    dcs_data=j;
  std::vector<uint64_t> ret;
  if(j.is_object())
  {  
    for (const auto& [key, value] : dcs_data.items()){
      uint64_t since=std::stoull(key);
      readCommonType(since,value);
      ret.push_back(since);
    }
  }
  return ret;
}
void Crest::CrestContainer::readCommonType(uint64_t since, const nlohmann::json &j_in)
{
   if(j_in.empty()){
     addIov(since);	   
     return;	   
   }	   
    // Loop over each channel in the data
    for (const auto &channel : j_in.items())
    {
      std::string channelKey = channel.key();
      const auto &data_ch = channel.value();
      nlohmann::json vecJson(json::value_t::array);
	if(m_isVectorPayload)
        vecJson=data_ch;
      else
        vecJson.push_back(data_ch);

      for (const auto &values : vecJson)
      {
        parseData(values);
	if(m_isVectorPayload)
	  putRow2Vector();
      }
      addData(channelKey);
    }
  addIov(since);
}

nlohmann::json Crest::CrestContainer::getStoreSetDto(uint64_t period)
{
  nlohmann::json j;
  nlohmann::json resources = nlohmann::json::array();
  if(m_modeId==Crest::ModeId::Standard){
    nlohmann::json storeDto;
    // Loop over the iov list container to add the data to the storeDto
    for (const auto& [key, value] : m_full_data)
    {
      storeDto["since"] = key;
      storeDto["data"] = value.dump();
      storeDto["streamerInfo"] = "";//m_streamer_info_data.dump();
      resources.push_back(storeDto);
    }
  }
  else if(m_modeId==Crest::ModeId::DCS_FULL){
    uint64_t since = getFirstTime();
    nlohmann::json storeDto;
    nlohmann::json dcs_payload;
    nlohmann::json curPayload=m_full_data[since];
    for (const auto& [key, value] : m_full_data)
    {
      curPayload.update(value);	    
      dcs_payload[std::to_string(key)]=curPayload;
      if(period>0 && (key-since)>period){
        storeDto["since"]=since;
	storeDto["data"]=dcs_payload.dump();
	storeDto["streamerInfo"] = "";
	resources.push_back(storeDto);
	since=key;
	dcs_payload={};
      }
    }
    if(dcs_payload.size()>0){
      storeDto["since"]=since;
      storeDto["data"]=dcs_payload.dump();
      storeDto["streamerInfo"] = "";
      resources.push_back(storeDto);
    }    
  }
  j["resources"] = resources;
  j["format"] = "StoreSetDto";
  j["datatype"] = "data";
  j["size"] = resources.size();
  
  return j;
}

