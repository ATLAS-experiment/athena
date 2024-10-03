/*
  Copyright (C) 2020-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CrestApi/CrestContainer.h"
#include "CrestApi/CrestCondException.h"
#include <stdarg.h>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iomanip>

using json = nlohmann::json;

Crest::CrestContainer::CrestContainer() : m_modeId(ModeId::Standard) { ; }
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

Crest::TypeId Crest::CrestContainer::stringToTypeId(const std::string &type) const
{
  for(auto& t:	s_typeToString){
    if(t.second.compare(type)==0)
      return t.first;	    
  }
  throw CommonCrestException("The type of parameter is not defined.");
}

// Function to convert an integer to a TypeId
Crest::TypeId Crest::CrestContainer::intToTypeId(int value) const
{
    if (value < 0 || value >= static_cast<int>(Crest::TypeId::TypeIdCount)) {
        throw std::out_of_range("Invalid integer value for TypeId");
    }
    return static_cast<Crest::TypeId>(value);
}

std::string Crest::CrestContainer::base64_encode(const uint8_t *data, unsigned int len)
{
  using namespace boost::archive::iterators;
  typedef base64_from_binary<transform_width<const char *, 6, 8>> base64_encoder;

  std::string encodedData;
  std::copy(base64_encoder(data), base64_encoder(data + len), std::back_inserter(encodedData));

  size_t numPaddingChars = (3 - len % 3) % 3;
  encodedData.append(numPaddingChars, '=');

  return encodedData;
}

// Decode base64 data to binary
std::vector<unsigned char> Crest::CrestContainer::base64_decode(const std::string &encodedData)
{
  using namespace boost::archive::iterators;
  typedef transform_width<binary_from_base64<std::string::const_iterator>, 8, 6> base64_decoder;

  std::string encodedDataNoPadding = encodedData;
  boost::algorithm::erase_all(encodedDataNoPadding, "=");

  std::vector<unsigned char> decodedData;
  std::copy(base64_decoder(encodedDataNoPadding.begin()), base64_decoder(encodedDataNoPadding.end()), std::back_inserter(decodedData));

  return decodedData;
}

void Crest::CrestContainer::addColumn(const std::string &name, TypeId type)
{
  m_payload_spec.emplace_back(name, type);
}

void Crest::CrestContainer::addColumn(const std::string &name, const char *type)
{
  addColumn(name, stringToTypeId(type));
}

void Crest::CrestContainer::addColumn(const std::string &name, uint32_t type)
{
  addColumn(name, static_cast<TypeId>(type));
}

void Crest::CrestContainer::addNullRecord(const char *name)
{
  auto it = std::find_if(m_payload_spec.begin(), m_payload_spec.end(),
                         [&name](const auto &column)
                         { return column.first == name; });

  if (it != m_payload_spec.end())
  {
    m_row[name] = nlohmann::json::value_t::null;
  }
}

void Crest::CrestContainer::addRecord(const char *name, int number, ...)
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

void Crest::CrestContainer::addData(const char *channel_id)
{
  addExternalData(channel_id, getRow());
}

void Crest::CrestContainer::addExternalData(const char *channel_id, const nlohmann::json &data)
{

  validatePayloadSize(data);
  nlohmann::json arr_data = m_isVectorPayload ? nlohmann::json::array() : nlohmann::json();

  for (const auto &data_row : data)//m_vector_data)
  {
    auto row_arr_data = createRowArray(data_row);

    if (m_isVectorPayload)
    {
      arr_data.push_back(row_arr_data);
    }
    else
    {
      arr_data = row_arr_data;
    }
  }
  m_payload[channel_id] = arr_data;
  m_vector_data.clear();
  m_row.clear();
}

void Crest::CrestContainer::validatePayloadSize(const nlohmann::json &data) const
{
  if (!data.is_array())
    throw CommonCrestException("The format of data is wrong. It should be the vector (size 1 for non vector payload)");
  if (!m_isVectorPayload && m_vector_data.size() > 1)
  {
    throw CommonCrestException("The payload is not a vector, but the size seems to be larger than 1....");
  }
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
  m_full_data[std::to_string(since)]=m_payload;
  m_payload.clear();
}

void Crest::CrestContainer::selectIov(const uint64_t since){
  m_iov_data["since"]=since;
  m_iov_data["data"]=m_full_data[std::to_string(since)];
}

std::vector<std::string> Crest::CrestContainer::channelIds(){
  std::vector<std::string> chs;
  for (auto& x : m_iov_data["data"].items()){
    chs.push_back(x.key());
  }
  return chs;
}
const std::vector<std::pair<std::string, Crest::TypeId>> &Crest::CrestContainer::getMPayloadSpec()
{
  return m_payload_spec;
}

const nlohmann::json &Crest::CrestContainer::getPayloadChannel(const char *channel_id)
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
  if (m_row.empty())
  {
    for (const auto &column : m_payload_spec)
    {
      int index = getColumnIndex(column.first);
      if (index == -1)
      {
        throw std::runtime_error("Column name not found.");
      }
      m_row[column.first] = it.value()[index];
    }
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
  m_vector_data.push_back(m_row);
  return m_vector_data;
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
void Crest::CrestContainer::dump_json_to_file(const nlohmann::json &j, const std::string &filename)
{
  std::ofstream file(filename);
  if (file.is_open())
  {
    file << std::setprecision(6) << j.dump(4);
    file.close();
  }
  else
  {
    std::cerr << "CondContainer::dump_json_to_file: Error opening file: " << filename << std::endl;
  }
}

// Function to read file and create JSON object
nlohmann::json Crest::CrestContainer::read_json_from_file(const std::string &filename, const std::string &spec_filename)
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
    std::cerr << "CondContainer::read_json_from_file: Error opening file: " << spec_filename << std::endl;
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
    std::cerr << "CondContainer::read_json_from_file: Error opening file: " << filename << std::endl;
  }
  return j;
}

void Crest::CrestContainer::parseOldFormat(std::string& colName, TypeId& typespec,const nlohmann::json & thisVal){
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
	    break;
	  }
	}

  }
  catch(json::exception& e){
    std::cerr << e.what() << std::endl;
    throw std::runtime_error(e.what());	  
  }
}
void Crest::CrestContainer::from_json(const uint64_t since,const nlohmann::json &j_in)
{
  nlohmann::json j = j_in;	
  if (j.is_string()){
    std::istringstream ss(to_string(j));
    std::string st;
    ss >> std::quoted(st);
    j = json::parse(st);	  
  }
  if(m_modeId!=ModeId::Standard)
    throw CommonCrestException("Unsupported type of payload.");
  // Accessing the "data" object
  if (j.contains("data") && j["data"].is_object())
  {
    const auto &data = j["data"];

    // Loop over each channel in the data
    for (const auto &channel : data.items())
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
      // TODO: this will not work for vector payloads
      // Check if the number of values in the array matches the expected number of columns
        if (values.is_array() && values.size() == m_payload_spec.size())
        {
          for (size_t i = 0; i < values.size(); ++i)
          {
            const auto &spec = m_payload_spec[i];
            std::string colName = spec.first;
            TypeId colType = spec.second;
            if(values[i].is_string() &&( colType==TypeId::Bool || colType==TypeId::Int16 || colType==TypeId::UInt16
             || colType==TypeId::Int32 || colType==TypeId::UInt32 || colType==TypeId::Int64 || colType==TypeId::UInt63 || colType==TypeId::Float || colType==TypeId::Double))
	    {
	      parseOldFormat(colName,colType,values[i]);
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
              throw CommonCrestException("Unsupported column type.");          
            }
	  }
        }
	else
	{
	  std::cerr << "CrestContainer::from_json: Mismatch in number of values for channel " << channelKey << std::endl;
	}
	if(m_isVectorPayload)
	  putRow2Vector();
      }
      addData(channelKey.c_str());
    }
  }
  else
  {
    std::cerr << "CrestContainer::from_json: JSON does not contain a 'data' object or it is not a JSON object." << std::endl;
    std::cerr << "CrestContainer::from_json json:"<<j<<std::endl;
  }
  addIov(since);
}
