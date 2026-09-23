/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file RDBVersionAccessor.cxx
 *
 * @brief Implementation of RDBVersionAccessor class
 *
 * @author Vakho Tsulaia <Vakhtang.Tsulaia@cern.ch>
 */

#include "RDBVersionAccessor.h"

#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ITable.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/SchemaException.h"

#include "CxxUtils/checker_macros.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"

#include "GaudiKernel/MsgStream.h"

#include <format>
#include <stdexcept>
#include <sstream>

#define RDB_MSG(lvl, fmt, ...)      \
  if (m_msgStream.level() <= lvl) {                                     \
    m_msgStream << lvl << std::format (fmt __VA_OPT__(, __VA_ARGS__)) << endmsg; \
  }
#define RDB_MSG_VERBOSE(fmt, ...) RDB_MSG(MSG::VERBOSE, fmt __VA_OPT__(, __VA_ARGS__))
#define RDB_MSG_DEBUG(fmt, ...) RDB_MSG(MSG::DEBUG, fmt __VA_OPT__(, __VA_ARGS__))
#define RDB_MSG_ERROR(fmt, ...) RDB_MSG(MSG::ERROR, fmt __VA_OPT__(, __VA_ARGS__))

RDBVersionAccessor::RDBVersionAccessor(const std::string& childNode
				       , const std::string& parentNode
				       , const std::string& parentTag
				       , coral::ISessionProxy* session
				       , MsgStream& msgStream)
  : m_session(session)
  , m_childNode(childNode)
  , m_parentNode(parentNode)
  , m_parentTag(parentTag)
  , m_tagName("")
  , m_tagID("")
  , m_isChildLeaf(false)
  , m_msgStream(msgStream)
{
}

void RDBVersionAccessor::getChildTagData()
{
  std::string parentNodeId, childNodeId, parentTagId;
  int nRows{};

  if(!m_session) {
    RDB_MSG_ERROR ("VersionAccessor: No connection to database!");
    return;
  }

  try {
    RDB_MSG_VERBOSE("VersionAccessor:  Version accessor for \n"
                    "ChildNode = {}     ParentNode = {}     ParentTag = {}",
                    m_childNode, m_parentNode, m_parentTag);

    coral::ITable& tableTag2Node = m_session->nominalSchema().tableHandle("HVS_TAG2NODE");
    coral::ITable& tableNode = m_session->nominalSchema().tableHandle("HVS_NODE");
    coral::ITable& tableLtag2Ltag = m_session->nominalSchema().tableHandle("HVS_LTAG2LTAG");

    //
    // STEP 1. Get TagId for parentTag 
    // 
    // Query: "SELECT TAG_ID FROM HVS_TAG2NODE WHERE TAG_NAME='m_parentTag'"
    //
    coral::IQuery *queryTag2Node = tableTag2Node.newQuery();
    queryTag2Node->addToOutputList("TAG_ID");
    queryTag2Node->setMemoryCacheSize(1);
    coral::AttributeList bindsTag2Node ATLAS_THREAD_SAFE;
    bindsTag2Node.extend<std::string>("tagN");
    queryTag2Node->setCondition("TAG_NAME=:tagN", bindsTag2Node);
    bindsTag2Node[0].data<std::string>() = m_parentTag;

    coral::ICursor& cursorTag2Node = queryTag2Node->execute();
    nRows = 0;
    while(cursorTag2Node.next()) {
      if(++nRows>1) {
	delete queryTag2Node;
        RDB_MSG_DEBUG("The tag {} is not unique in HVS_TAG2NODE table!",
                      m_parentTag);
        return;
      }      

      const coral::AttributeList& row = cursorTag2Node.currentRow();
      parentTagId = attribute2String(row,"TAG_ID");

    }
    if(nRows==0) {
      delete queryTag2Node;
      RDB_MSG_DEBUG("The tag {} not found in HVS_TAG2NODE table!",
                    m_parentTag);
      return;
    }

    delete queryTag2Node;
    RDB_MSG_VERBOSE("VersionAccessor:  Parent Tag Id = {}", parentTagId);

    //
    // STEP 2. Get NodeIDs for parentNode and child 
    //
    // Query: "SELECT NODE_NAME, NODE_ID, BRANCH_FLAG FROM HVS_NODE
    //         WHERE NODE_NAME='m_parentNode' OR NODE_NAME='m_childNode'"
    //

    coral::IQuery* queryNodeIDs = tableNode.newQuery();

    queryNodeIDs->addToOutputList("NODE_NAME");
    queryNodeIDs->addToOutputList("NODE_ID");
    queryNodeIDs->addToOutputList("BRANCH_FLAG");
    queryNodeIDs->setMemoryCacheSize(2);

    coral::AttributeList bindsNodeIDs ATLAS_THREAD_SAFE;
    bindsNodeIDs.extend<std::string>("parentN");
    bindsNodeIDs.extend<std::string>("childN");

    queryNodeIDs->setCondition( "NODE_NAME=:parentN OR NODE_NAME=:childN", bindsNodeIDs);

    bindsNodeIDs[0].data<std::string>() = m_parentNode;
    bindsNodeIDs[1].data<std::string>() = m_childNode;

    coral::ICursor& cursorNodeIDs = queryNodeIDs->execute();

    nRows = 0;
    while(cursorNodeIDs.next()) {
      if(++nRows>2) {
	break;
      }
      const coral::AttributeList& row = cursorNodeIDs.currentRow();

      if(attribute2String(row,"NODE_NAME")==m_childNode) {
	childNodeId = attribute2String(row,"NODE_ID");
	m_isChildLeaf = (attribute2String(row,"BRANCH_FLAG")=="0");
      }
      else {
	parentNodeId = attribute2String(row,"NODE_ID");
      }
    }
    if(nRows!=2 && m_childNode!=m_parentNode) {
      delete queryNodeIDs;
      RDB_MSG_DEBUG("Error processing HVS_NODE, Number of fetched records !=2");
      return;
    }

    RDB_MSG_VERBOSE("VersionAccessor:  Child Node Id = {}", childNodeId);
    RDB_MSG_VERBOSE("VersionAccessor:  Parent Node Id = {}", parentNodeId);

    delete queryNodeIDs;

    if(m_childNode==m_parentNode) {
      m_tagName = m_parentTag;
      m_tagID = std::move(parentTagId);
      return;
    }

    //
    // STEP 3.
    //
    // *********************** THIS IS A TEMPORARY SOLUTION ************************
    // ***                                                                       ***
    // *** 1. First we determine a path from childNode to parentNode querying    ***
    // ***    HVS_NODE few times                                                 ***
    // *** 2. Then we query HVS_LTAG2LTAG few times to get tag of childNode      ***
    // ***    of childNode                                                       ***
    // ***                                                                       ***
    // *********** ************ ********** ************* ************ **************

    std::vector<std::string> path;
    path.push_back(childNodeId);
    std::string currentParrent = childNodeId;
    std::string currentChild = std::move(childNodeId);

    coral::AttributeList bindsNode ATLAS_THREAD_SAFE;
    bindsNode.extend<std::string>("nodeId");
    const std::string parentIdStr{"PARENT_ID"};
    const std::string conditionStr{"NODE_ID=:nodeId"};
    while(currentParrent != parentNodeId) {
      //
      // Query: "SELECT PARENT_ID FROM HVS_NODE WHERE NODE_ID='currentChild'"
      //
      coral::IQuery* queryNode(tableNode.newQuery());

      queryNode->addToOutputList(parentIdStr);
      queryNode->setMemoryCacheSize(1);

      queryNode->setCondition(conditionStr, bindsNode);
      bindsNode[0].data<std::string>()=currentChild;

      coral::ICursor& cursorNode = queryNode->execute();

      nRows = 0;

      // Process Query results
      while(cursorNode.next()) {
	if(++nRows>1) {
	  delete queryNode;
          RDB_MSG_DEBUG("The node {} has more than one parent!", currentChild);
          return;
	}	

	const coral::AttributeList& row = cursorNode.currentRow();
	
	if(row[0].isNull()) {
	  delete queryNode;
          RDB_MSG_DEBUG("The requested child and parent nodes are not on the same branch!");
          return;
	}	

	currentParrent = attribute2String(row,parentIdStr);
	currentChild = currentParrent;
	path.push_back(currentParrent);
      }
      if(nRows==0) {
	delete queryNode;
        RDB_MSG_DEBUG("The node {} has no parent!", currentChild);
        return;
      }

      delete queryNode;
      RDB_MSG_VERBOSE("VersionAccessor:  Current Child = {}", currentChild);
      RDB_MSG_VERBOSE("VersionAccessor:  Current Parrent = {}", currentParrent);
    }

    coral::AttributeList bindsLtag2Ltag ATLAS_THREAD_SAFE;
    bindsLtag2Ltag.extend<std::string>("childN");
    bindsLtag2Ltag.extend<std::string>("parentT");
    bindsLtag2Ltag.extend<std::string>("parentN");

    const std::string childTagString{"CHILD_TAG"};
    const std::string childConditionStr = "CHILD_NODE =:childN AND PARENT_TAG =:parentT AND PARENT_NODE =:parentN";
    for(unsigned int ind=1; ind<path.size(); ind++) {
      // Query: "SELECT CHILD_TAG FROM HVS_LTAG2LTAG WHERE
      //         CHILD_NODE  = 'path[path.size()-ind-1]' AND
      //         PARENT_TAG  = 'parentTagId' AND
      //         PARENT_NODE = 'path[path.size()-ind]'
      coral::IQuery* queryLtag2Ltag(tableLtag2Ltag.newQuery());
      
      queryLtag2Ltag->addToOutputList(childTagString);
      queryLtag2Ltag->setMemoryCacheSize(1);

      queryLtag2Ltag->setCondition(childConditionStr, bindsLtag2Ltag);

      bindsLtag2Ltag[0].data<std::string>() = path[path.size()-ind-1];
      bindsLtag2Ltag[1].data<std::string>() = parentTagId;
      bindsLtag2Ltag[2].data<std::string>() = path[path.size()-ind];

      coral::ICursor& cursorLtag2Ltag = queryLtag2Ltag->execute();

      nRows = 0;
      // Process Query results
      while(cursorLtag2Ltag.next()) {
	if(++nRows>1) {
	  delete queryLtag2Ltag;
          RDB_MSG_DEBUG("Version {} has more than one child of type {}!",
                        parentTagId, path[path.size()-ind-1]);
          return;
	}

	const coral::AttributeList& row = cursorLtag2Ltag.currentRow();
	parentTagId = attribute2String(row,childTagString);
      }
      if(nRows==0) {
	delete queryLtag2Ltag;
        RDB_MSG_DEBUG("Version {} has no child of type {}!",
                      parentTagId, path[path.size()-ind-1]);
        return;
      }

      delete queryLtag2Ltag;

      RDB_MSG_VERBOSE("VersionAccessor:  Parent Tag Id = {}", parentTagId);
    }

    //
    // STEP 4. Get tag name for obtained TAG_ID
    //
    // Query: "SELECT TAG_NAME from HVS_TAG2NODE WHERE TAG_ID='parentTagId'"
    //
    coral::IQuery* queryTagName(tableTag2Node.newQuery());
    const std::string tagNameStr{"TAG_NAME"};
    queryTagName->addToOutputList(tagNameStr);
    queryTagName->setMemoryCacheSize(1);

    coral::AttributeList bindsTagName ATLAS_THREAD_SAFE;
    bindsTagName.extend<std::string>("tagId");

    queryTagName->setCondition("TAG_ID=:tagId", bindsTagName);
    bindsTagName[0].data<std::string>() = parentTagId;

    coral::ICursor& cursorTagName = queryTagName->execute();

    nRows = 0;
    while(cursorTagName.next()) {
      if(++nRows>1) {
	delete queryTagName;
        RDB_MSG_DEBUG("More than one record retrieved when getting tag name for given ID");
	return;
      }  

      const coral::AttributeList& row = cursorTagName.currentRow();
      
      m_tagName =attribute2String(row,tagNameStr); 
      m_tagID = parentTagId;

      RDB_MSG_VERBOSE("VersionAccessor:  Child Tag Name = {}", m_tagName);
    }
    delete queryTagName;

  }
  catch(coral::SchemaException& se) 
  {
     RDB_MSG_ERROR("VersionAccessor: Schema Exception : {}", se.what());
  }
  catch(std::exception& e)
  {
    RDB_MSG_DEBUG("{}", e.what());
  }
  catch(...) 
  {
    RDB_MSG_ERROR("VersionAccessor: Exception caught(...)");
  }
}

std::string RDBVersionAccessor::attribute2String(const coral::AttributeList& attList,
						 const std::string& fieldName)
{
  std::ostringstream streamValue;
  attList[fieldName].toOutputStream(streamValue);
  std::string stringValue = streamValue.str();
  unsigned int position = stringValue.find(") :");
  return stringValue.substr(position+4);
}
