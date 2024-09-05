#ifndef EVENT_INFO_WRITER_ALG_H
#define EVENT_INFO_WRITER_ALG_H

#include "JetH5Writer/EventInfoWriter.h"
#include "HDF5Utils/IH5GroupSvc.h"

#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"
#include "GaudiKernel/ServiceHandle.h"

class EventInfoWriterAlg: public AthAlgorithm
{
public:
  EventInfoWriterAlg(const std::string& name, ISvcLocator* loc);

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
  Gaudi::Property<std::string> m_dsName {
    this, "datasetName", "", "Name of output dataset"
  };
  SG::ReadHandleKey<xAOD::EventInfo> m_infoKey {
    this, "eventInfo", "EventInfo", "Event info key"};
  ServiceHandle<IH5GroupSvc> m_output_svc {
    this, "output", "", "output file service"};

  std::unique_ptr<EventInfoWriter> m_writer;

};


#endif
