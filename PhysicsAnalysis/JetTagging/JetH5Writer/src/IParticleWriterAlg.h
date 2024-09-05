#ifndef IPARTICLE_WRITER_ALG_H
#define IPARTICLE_WRITER_ALG_H

#include "JetH5Writer/IParticleWriter.h"
#include "HDF5Utils/IH5GroupSvc.h"

#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODBase/IParticleContainer.h"
#include "GaudiKernel/ServiceHandle.h"

class IParticleWriterAlg: public AthAlgorithm
{
public:
  IParticleWriterAlg(const std::string& name, ISvcLocator* loc);

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:
  Gaudi::Property<std::vector<std::string>> m_primitives {
    this, "primitives", {}, "List of primatives to print"
  };
  Gaudi::Property<std::map<std::string, std::string>> m_primToType {
    this, "primitiveToType", {}, "Map from primitive to type"
  };
  Gaudi::Property<std::map<std::string, std::string>> m_primToAssociation {
    this, "primitiveToAssociation", {}, "Map from primitive to association"
  };
  Gaudi::Property<std::string> m_dsName {
    this, "datasetName", "", "Name of output dataset"
  };
  Gaudi::Property<unsigned long long> m_maxSize {
    this, "maximumSize", 10, "Maximum number of particles to store"
  };
  Gaudi::Property<std::string> m_arrayFormat {
    this, "arrayFormat", "PADDED", "Array format"
  };
  SG::ReadHandleKey<xAOD::IParticleContainer> m_partKey {
    this, "container", "", "IParticle container key"};
  ServiceHandle<IH5GroupSvc> m_output_svc {
    this, "output", "", "output file service"};

  std::unique_ptr<IParticleWriter> m_writer;

};


#endif
