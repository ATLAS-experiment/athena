/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "JetH5Writer/EventInfoWriter.h"
#include "JetH5Writer/EventInfoWriterConfig.h"

#include "PrimitiveHelpers.h"

#include "xAODEventInfo/EventInfo.h"
#include "HDF5Utils/Writer.h"

EventInfoWriter::EventInfoWriter(
  H5::Group& group,
  const EventInfoWriterConfig& cfg)
{
  Writer_t::consumer_type c;
  for (const auto& input: cfg.inputs) {
    detail::addInput(c, input);
  }
  m_writer = std::make_unique<Writer_t>(group, cfg.name, c);
}

EventInfoWriter::~EventInfoWriter() = default;

void EventInfoWriter::fill(const xAOD::EventInfo& info) {
  m_writer->fill(info);
}

void EventInfoWriter::flush() {
  m_writer->flush();
}
