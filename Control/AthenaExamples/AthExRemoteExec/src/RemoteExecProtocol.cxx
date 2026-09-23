/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RemoteExecProtocolConvert.h"

#include <grpcpp/grpcpp.h>

#include <iomanip>
#include <sstream>

namespace AthExRemoteExec {

const char* toString( Status status )
{
  switch ( status ) {
  case Status::Unspecified:
    return "UNSPECIFIED";
  case Status::Ok:
    return "OK";
  case Status::UnknownSequence:
    return "UNKNOWN_SEQUENCE";
  case Status::InvalidRequest:
    return "INVALID_REQUEST";
  case Status::AlgFailure:
    return "ALG_FAILURE";
  case Status::Timeout:
    return "TIMEOUT";
  case Status::Overloaded:
    return "OVERLOADED";
  case Status::ShuttingDown:
    return "SHUTTING_DOWN";
  }
  return "UNSPECIFIED";
}

std::string describe( const Payload& payload )
{
  // Bounded, and never interprets the bytes. A payload can be tens of
  // megabytes and this is called on every reply, so an earlier version that
  // rendered container contents in full cost more than the rest of the round
  // trip put together. Small payloads get their bytes in hex, which keeps
  // scalar results checkable in a log without this having to know any types.
  constexpr size_t maxShown = 16;
  std::ostringstream out;
  out << payload.schema << " [" << payload.encoding << ", "
      << payload.data.size() << " bytes]";
  if ( !payload.data.empty() && payload.data.size() <= maxShown ) {
    out << " 0x" << std::hex << std::setfill( '0' );
    for ( unsigned char byte : payload.data ) {
      out << std::setw( 2 ) << static_cast<unsigned int>( byte );
    }
  }
  return out.str();
}

std::vector<std::string> EventId::missing() const
{
  std::vector<std::string> absent;
  if ( !runNumber ) {
    absent.push_back( "run_number" );
  }
  if ( !eventNumber ) {
    absent.push_back( "event_number" );
  }
  if ( !lumiBlock ) {
    absent.push_back( "lumi_block" );
  }
  if ( !timeStamp ) {
    absent.push_back( "time_stamp" );
  }
  return absent;
}

athremoteexec::v1::Status toProto( Status status )
{
  // A refusal has no proto value -- 6 and 7 are reserved -- because it leaves
  // as a transport status instead. Reaching here with one means a refusal
  // escaped that mapping, and answering UNSPECIFIED is better than emitting a
  // number the schema says does not exist.
  if ( isRefusal( status ) ) {
    return athremoteexec::v1::STATUS_UNSPECIFIED;
  }
  return static_cast<athremoteexec::v1::Status>( static_cast<int>( status ) );
}

Status fromProto( athremoteexec::v1::Status status )
{
  return static_cast<Status>( static_cast<int>( status ) );
}

void toProto( const Payload& payload, athremoteexec::v1::Payload& out )
{
  out.set_key( payload.key );
  out.set_encoding( payload.encoding );
  out.set_schema( payload.schema );
  out.set_data( payload.data );
}

void fromProto( const athremoteexec::v1::Payload& in, Payload& payload )
{
  payload.key = in.key();
  payload.encoding = in.encoding();
  payload.schema = in.schema();
  payload.data = in.data();
}

void toProto( const ExecuteRequest& request, athremoteexec::v1::ExecuteRequest& out )
{
  out.set_sequence( request.sequence );
  out.set_request_id( request.requestId );
  for ( const Payload& payload : request.inputs ) {
    toProto( payload, *out.add_inputs() );
  }

  // Only the fields that were actually set are written, so an incomplete
  // identity stays incomplete on the wire and the server can say which parts
  // are missing rather than "something was wrong".
  athremoteexec::v1::EventId& id = *out.mutable_event_id();
  const EventId& from = request.eventId;
  if ( from.runNumber ) {
    id.set_run_number( *from.runNumber );
  }
  if ( from.eventNumber ) {
    id.set_event_number( *from.eventNumber );
  }
  if ( from.lumiBlock ) {
    id.set_lumi_block( *from.lumiBlock );
  }
  if ( from.timeStamp ) {
    id.set_time_stamp( *from.timeStamp );
  }
  id.set_time_stamp_ns_offset( from.timeStampNsOffset );
  id.set_bunch_crossing_id( from.bunchCrossingId );
}

void fromProto( const athremoteexec::v1::ExecuteRequest& in, ExecuteRequest& request )
{
  request.sequence = in.sequence();
  request.requestId = in.request_id();

  request.eventId = EventId{};
  if ( in.has_event_id() ) {
    const athremoteexec::v1::EventId& id = in.event_id();
    EventId& to = request.eventId;
    if ( id.has_run_number() ) {
      to.runNumber = id.run_number();
    }
    if ( id.has_event_number() ) {
      to.eventNumber = id.event_number();
    }
    if ( id.has_lumi_block() ) {
      to.lumiBlock = id.lumi_block();
    }
    if ( id.has_time_stamp() ) {
      to.timeStamp = id.time_stamp();
    }
    to.timeStampNsOffset = id.time_stamp_ns_offset();
    to.bunchCrossingId = id.bunch_crossing_id();
  }
  request.inputs.clear();
  request.inputs.reserve( in.inputs_size() );
  for ( const athremoteexec::v1::Payload& payload : in.inputs() ) {
    Payload converted;
    fromProto( payload, converted );
    request.inputs.push_back( std::move( converted ) );
  }
}

void toProto( const ExecuteReply& reply, athremoteexec::v1::ExecuteReply& out )
{
  out.set_status( toProto( reply.status ) );
  out.set_detail( reply.detail );
  out.set_request_id( reply.requestId );
  for ( const Payload& payload : reply.outputs ) {
    toProto( payload, *out.add_outputs() );
  }
}

void toProto( ExecuteReply&& reply, athremoteexec::v1::ExecuteReply& out )
{
  out.set_status( toProto( reply.status ) );
  out.set_detail( std::move( reply.detail ) );
  out.set_request_id( reply.requestId );
  for ( Payload& payload : reply.outputs ) {
    athremoteexec::v1::Payload& target = *out.add_outputs();
    target.set_key( std::move( payload.key ) );
    target.set_encoding( std::move( payload.encoding ) );
    target.set_schema( std::move( payload.schema ) );
    // The one that matters: protobuf's string setters have rvalue overloads,
    // so the payload bytes are adopted rather than copied. On the way *in*
    // there is no equivalent -- gRPC owns the request message and hands it
    // over const -- so that copy remains, and the phase table shows it.
    target.set_data( std::move( payload.data ) );
  }
}

void fromProto( const athremoteexec::v1::ExecuteReply& in, ExecuteReply& reply )
{
  reply.status = fromProto( in.status() );
  reply.detail = in.detail();
  reply.requestId = in.request_id();
  reply.outputs.clear();
  reply.outputs.reserve( in.outputs_size() );
  for ( const athremoteexec::v1::Payload& payload : in.outputs() ) {
    Payload converted;
    fromProto( payload, converted );
    reply.outputs.push_back( std::move( converted ) );
  }
}

void toProto( const SequenceInfo& info, athremoteexec::v1::SequenceInfo& out )
{
  out.set_name( info.name );
  const auto fill = []( const BoundaryInfo& boundary,
                        athremoteexec::v1::BoundaryInfo& target ) {
    target.set_key( boundary.key );
    target.set_encoding( boundary.encoding );
    target.set_schema( boundary.schema );
  };
  for ( const BoundaryInfo& boundary : info.inputs ) {
    fill( boundary, *out.add_inputs() );
  }
  for ( const BoundaryInfo& boundary : info.outputs ) {
    fill( boundary, *out.add_outputs() );
  }
}

bool encode( const ExecuteRequest& request, std::string& wire )
{
  athremoteexec::v1::ExecuteRequest message;
  toProto( request, message );
  return message.SerializeToString( &wire );
}

bool decode( std::string_view wire, ExecuteRequest& request )
{
  athremoteexec::v1::ExecuteRequest message;
  if ( !message.ParseFromArray( wire.data(),
                                static_cast<int>( wire.size() ) ) ) {
    return false;
  }
  fromProto( message, request );
  return true;
}

bool encode( const ExecuteReply& reply, std::string& wire )
{
  athremoteexec::v1::ExecuteReply message;
  toProto( reply, message );
  return message.SerializeToString( &wire );
}

bool decode( std::string_view wire, ExecuteReply& reply )
{
  athremoteexec::v1::ExecuteReply message;
  if ( !message.ParseFromArray( wire.data(),
                                static_cast<int>( wire.size() ) ) ) {
    return false;
  }
  fromProto( message, reply );
  return true;
}

std::string grpcVersion()
{
  return grpc::Version();
}

}  // namespace AthExRemoteExec
