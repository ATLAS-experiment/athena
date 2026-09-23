/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DemoSchema.h"

#include "athexremoteexec_demo.pb.h"

namespace AthExRemoteExec::DemoSchema {

const char* const ints = "athexremoteexec.demo.v1.Ints";
const char* const doubles = "athexremoteexec.demo.v1.Doubles";

bool encodeInts( const std::vector<NamedInt>& values, std::string& bytes,
                 std::string& error )
{
  athexremoteexec::demo::v1::Ints message;
  for ( const NamedInt& value : values ) {
    athexremoteexec::demo::v1::Ints_Entry* entry = message.add_entry();
    entry->set_name( value.name );
    entry->set_value( value.value );
  }
  if ( !message.SerializeToString( &bytes ) ) {
    error = "could not serialise an Ints message";
    return false;
  }
  return true;
}

bool decodeInts( std::string_view bytes, std::vector<NamedInt>& values,
                 std::string& error )
{
  athexremoteexec::demo::v1::Ints message;
  if ( !message.ParseFromArray( bytes.data(),
                                static_cast<int>( bytes.size() ) ) ) {
    error = "not a well-formed Ints message (" +
            std::to_string( bytes.size() ) + " bytes)";
    return false;
  }
  values.clear();
  values.reserve( message.entry_size() );
  for ( const athexremoteexec::demo::v1::Ints_Entry& entry : message.entry() ) {
    values.push_back( NamedInt{entry.name(), entry.value()} );
  }
  return true;
}

bool encodeDoubles( const std::string& name, const std::vector<double>& values,
                    std::string& bytes, std::string& error )
{
  athexremoteexec::demo::v1::Doubles message;
  message.set_name( name );
  message.mutable_value()->Add( values.begin(), values.end() );
  if ( !message.SerializeToString( &bytes ) ) {
    error = "could not serialise a Doubles message";
    return false;
  }
  return true;
}

bool decodeDoubles( std::string_view bytes, std::string& name,
                    std::vector<double>& values, std::string& error )
{
  athexremoteexec::demo::v1::Doubles message;
  if ( !message.ParseFromArray( bytes.data(),
                                static_cast<int>( bytes.size() ) ) ) {
    error = "not a well-formed Doubles message (" +
            std::to_string( bytes.size() ) + " bytes)";
    return false;
  }
  name = message.name();
  values.assign( message.value().begin(), message.value().end() );
  return true;
}

}  // namespace AthExRemoteExec::DemoSchema
