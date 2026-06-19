#include "src/EventInfoWriterAlg.h"
#include "JetH5Writer/EventInfoWriterConfig.h"

#include "AlgHelpers.h"

#include "H5Cpp.h"

EventInfoWriterAlg::EventInfoWriterAlg(const std::string& name,
                                       ISvcLocator* loc):
  AthAlgorithm(name, loc),
  m_writer(nullptr)
{
}

StatusCode EventInfoWriterAlg::initialize() {
  ATH_CHECK(m_infoKey.initialize());
  ATH_CHECK(m_output_svc.retrieve());

  EventInfoWriterConfig cfg;
  cfg.name = m_dsName.value();
  if (cfg.name.empty()) {
    ATH_MSG_ERROR("datasetName isn't specified in EventInfo writer");
    return StatusCode::FAILURE;
  }

  for (const std::string& prim: m_primitives) {
    if (!m_primToType.value().count(prim)) {
      ATH_MSG_ERROR(prim << " not specified in type mapping");
    }
    std::string type = m_primToType.value().at(prim);
    cfg.inputs.push_back(Primitive{getPrimitiveType(type), prim, prim});
  }
  m_writer.reset(new EventInfoWriter(*m_output_svc->group(), cfg));

  return StatusCode::SUCCESS;
}

StatusCode EventInfoWriterAlg::execute(const EventContext& ctx) {
  SG::ReadHandle event_info(m_infoKey, ctx);
  ATH_CHECK(event_info.isValid());
  m_writer->fill(*event_info);
  return StatusCode::SUCCESS;
}

StatusCode EventInfoWriterAlg::finalize() {
  m_writer->flush();
  return StatusCode::SUCCESS;
}
