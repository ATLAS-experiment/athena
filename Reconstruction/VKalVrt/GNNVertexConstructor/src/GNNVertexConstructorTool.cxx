// Header include
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
//


namespace Rec {
    
    GNNVertexConstructorTool::GNNVertexConstructorTool(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type,name,parent)
    {
//
// Declare additional interface
//
    declareInterface< IGNNVertexConstructorInterface >(this);
    ATH_MSG_DEBUG("GNNVertexConstructorTool constructor called");
    }   
     /* Destructor */
    GNNVertexConstructorTool::~GNNVertexConstructorTool()
    {
    ATH_MSG_DEBUG("GNNVertexConstructorTool destructor called");
    }



    StatusCode GNNVertexConstructorTool::initialize()
    {

    ATH_MSG_DEBUG("GNNVertexConstructor Tool in initialize()");

    return StatusCode::SUCCESS;
    }
    
    StatusCode GNNVertexConstructorTool::finalize()
    {

    ATH_MSG_DEBUG("GNNVertexConstructor Tool in finalize()");
    
    return StatusCode::SUCCESS;
    }


    unsigned int GNNVertexConstructorTool::addTwoNumbers( const unsigned int & NoOne,
                                                     const unsigned int & NoTwo) const
    
    {
    unsigned int sum=NoOne+NoTwo;
    return sum;
    }


}  // end Rec namespace
