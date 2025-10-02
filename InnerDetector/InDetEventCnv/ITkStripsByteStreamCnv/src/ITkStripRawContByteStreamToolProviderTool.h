#ifndef ITKSTRIPCONTBYTESTREAMTOOLPROVIDERTOOL_H
#define ITKSTRIPCONTBYTESTREAMTOOLPROVIDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "SCT_RawDataByteStreamCnv/ISCTRawContByteStreamTool.h"
#include "SCT_RawDataByteStreamCnv/ISCTRawContByteStreamToolProviderTool.h"

  class ITkStripRawContByteStreamToolProviderTool
    : public extends<AthAlgTool, ISCTRawContByteStreamToolProviderTool>
  {
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
    public:

      /// Constructor with parameters:
     ITkStripRawContByteStreamToolProviderTool(const std::string& type, const std::string& name,
                                          const IInterface* parent)
        : base_class(type, name, parent)
     {}

      /// Destructor:
     virtual ~ITkStripRawContByteStreamToolProviderTool() override = default;

     StatusCode  initialize() override {
        ATH_CHECK(m_rawContByteStreamTool.retrieve());
        ATH_MSG_INFO( "retrieved " << m_rawContByteStreamTool.name() );
        return StatusCode::SUCCESS;
     }

     virtual const ISCTRawContByteStreamTool &getTool() const override{
        return *m_rawContByteStreamTool;
     }

private:
     /** Tool to do coversion from ITk Strip RDO container to ByteStream */
     ToolHandle<ISCTRawContByteStreamTool> m_rawContByteStreamTool
        {this, "RawContByteStreamTool",""};
  };
#endif
