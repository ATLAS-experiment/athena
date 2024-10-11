#ifndef MUONCALIBSTREAMCNVSVC_MUONCALIBSTREAMCNVSVC_H
#define MUONCALIBSTREAMCNVSVC_MUONCALIBSTREAMCNVSVC_H

#include "CoralBase/Attribute.h"
#include "AthenaBaseComps/AthCnvSvc.h"

class MuonCalibStreamCnvSvc : public AthCnvSvc {
public:

    MuonCalibStreamCnvSvc(const std::string &name, ISvcLocator *svc);
    virtual ~MuonCalibStreamCnvSvc();
    virtual StatusCode initialize() override;
    /// Update state of the service
    virtual StatusCode updateServiceState(IOpaqueAddress *pAddress) override;

protected:
    // initialize the converters for these data classes.
    std::vector<std::string> m_initCnvs;
};
#endif
