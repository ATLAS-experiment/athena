#ifndef SCTRAWCONTBYTESTREAMTOOLPROVIDERTOOL_H
#define SCTRAWCONTBYTESTREAMTOOLPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "SCT_RawDataByteStreamCnv/ISCTRawContByteStreamTool.h"
#include "SCT_RawDataByteStreamCnv/ISCTRawContByteStreamToolProviderTool.h"

  class SCTRawContByteStreamToolProviderTool
    : public extends<AthAlgTool, ISCTRawContByteStreamToolProviderTool>
  {
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
    public:

      /// Constructor with parameters:
     SCTRawContByteStreamToolProviderTool(const std::string& type, const std::string& name,
                                          const IInterface* parent)
        : base_class(type, name, parent)
     {}

      /// Destructor:
     virtual ~SCTRawContByteStreamToolProviderTool() override = default;

     StatusCode  initialize() override {
        ATH_CHECK(m_rawContByteStreamTool.retrieve());
        ATH_MSG_INFO( "retrieved " << m_rawContByteStreamTool.name() );
        return StatusCode::SUCCESS;
     }

     virtual const ISCTRawContByteStreamTool &getTool() const override{
        return *m_rawContByteStreamTool;
     }

private:
     /** Tool to do coversion from SCT RDO container to ByteStream */
     ToolHandle<ISCTRawContByteStreamTool> m_rawContByteStreamTool
        {this, "RawContByteStreamTool",""};
  };
#endif
