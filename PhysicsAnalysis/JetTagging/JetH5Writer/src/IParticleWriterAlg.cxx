/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "src/IParticleWriterAlg.h"
#include "JetH5Writer/IParticleWriterConfig.h"

#include "AlgHelpers.h"

#include "H5Cpp.h"

IParticleWriterAlg::IParticleWriterAlg(const std::string& name,
                                       ISvcLocator* loc):
  AthAlgorithm(name, loc),
  m_writer(nullptr)
{
}

StatusCode IParticleWriterAlg::initialize() {
  ATH_CHECK(m_partKey.initialize());
  ATH_CHECK(m_output_svc.retrieve());

  IParticleWriterConfig cfg;
  cfg.name = m_dsName.value();
  cfg.maximum_size = m_maxSize.value();
  cfg.format = getArrayFormat(m_arrayFormat);
  if (cfg.name.empty()) {
    ATH_MSG_ERROR("datasetName isn't specified in particle writer");
    return StatusCode::FAILURE;
  }
  for (const std::string& prim: m_primitives) {
    if (!m_primToType.value().count(prim)) {
      ATH_MSG_ERROR(prim << " not specified in type mapping");
      return StatusCode::FAILURE;
    }
    auto type = getPrimitiveType(m_primToType.value().at(prim));
    if (m_primToAssociation.value().count(prim)) {
      std::string path = m_primToAssociation.value().at(prim);
      size_t pos = path.find('/');
      if (pos == std::string::npos) {
        ATH_MSG_ERROR("no '/' in " << path);
        return StatusCode::FAILURE;
      }
      std::string link_name = path.substr(0, pos);
      std::string source_name = path.substr(pos+1);
      Primitive newprim {
        type,
        std::move(source_name),
        prim
      };
      cfg.inputs.emplace_back(std::move(link_name), std::move(newprim));
    } else {
      Primitive newprim {
        type,
        prim,
        prim
      };
      cfg.inputs.emplace_back("",std::move(newprim));
    }
  }
  m_writer.reset(new IParticleWriter(*m_output_svc->group(), cfg));

  return StatusCode::SUCCESS;
}

StatusCode IParticleWriterAlg::execute(const EventContext& ctx) {
  SG::ReadHandle ipc(m_partKey, ctx);
  std::vector<const xAOD::IParticle*> parts(ipc->begin(), ipc->end());
  m_writer->fill(parts);
  return StatusCode::SUCCESS;
}

StatusCode IParticleWriterAlg::finalize() {
  m_writer->flush();
  return StatusCode::SUCCESS;
}
