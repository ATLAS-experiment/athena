/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// OutputConditionsAlg.cxx
// Algorithm to provide simple writing of conditions data to outputstream
// and optional registration in IOV database
// Richard Hawkings, started 1/9/05, from skeleton by Walter Lampl
// Added Crest output, September 2026, Walter Lampl 


#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "PersistentDataModel/AthenaAttributeList.h"
#include "RegistrationServices/IIOVRegistrationSvc.h"

#include "CoralUtilities/ChaiCoralConverter.h"  

#include "SGTools/DataProxy.h"
#include "OutputConditionsAlg.h"
#include <GaudiKernel/StatusCode.h>
#include "GaudiKernel/IAddressCreator.h"

#include <chai/Container.h>
#include <chai/Database.h>
#include <chai/GlobalTag.h>
#include <chai/PayloadSpec.h>


#include <exception>
#include <ranges>

#include "AthenaPoolUtilities/CondAttrListCollAddress.h"

OutputConditionsAlg::~OutputConditionsAlg() 
{}


StatusCode OutputConditionsAlg::initialize() {
  ATH_MSG_DEBUG ("in initialize()");

  // get pointer to ClassIDSvc
  if (StatusCode::SUCCESS!= p_clidsvc.retrieve()) {
    ATH_MSG_FATAL ("ClassIDSvc not found");
    return StatusCode::FAILURE;
  }
  if (m_par_writeIOV && m_par_crestDir.empty()) {
    // get pointer to IOVRegistrationSvc
    if (StatusCode::SUCCESS!=p_regsvc.retrieve()) {
      ATH_MSG_FATAL ("IOVRegistrationSvc not found");
      return StatusCode::FAILURE;
    }
  }
  m_streamer = IAthenaOutputStreamTool_t("AthenaOutputStreamTool/"+
					 m_streamName);
  StatusCode sc = m_streamer.retrieve();
  if (sc.isFailure()) {
    ATH_MSG_ERROR ("Unable to find AthenaOutputStreamTool with name " << 
                   m_streamName);
    return StatusCode::FAILURE;
  }  

  if (!m_par_crestDir.empty()) {
    ATH_MSG_INFO(m_objectList.size() << "," << m_par_iovtags.size());
     //CREST mode:
     ATH_CHECK(m_persSvc.retrieve()); 
    //Sanity check of properties, m_object list and m_par_iovtags need to be index parallel
    if (m_objectList.size() > m_par_iovtags.size()) {
      ATH_MSG_ERROR("CREST mode: Database tag required for each object");
      for (size_t i= m_objectList.size()-m_par_iovtags.size();i<m_objectList.size();++i) {
        ATH_MSG_ERROR ("  No database tag set for object " << m_objectList[i]);
      }
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode OutputConditionsAlg::execute(const EventContext& /*ctx*/) {

  return StatusCode::SUCCESS;
}

StatusCode OutputConditionsAlg::finalize() {
  ATH_MSG_INFO ("Finalize: preparing to write conditions objects ");

  StatusCode sc = m_streamer->connectOutput();
  if (sc.isFailure()) {
    ATH_MSG_ERROR ("Could not connect stream to output");
    return( StatusCode::FAILURE);
  }
  // create list of objects
  
  struct obj_t{
    std::string type;
    std::string key;
    std::string folder;
    std::string tag;
    std::string tagDescr;
    SG::DataProxy* proxy=nullptr;
  };

  std::vector<obj_t> objs;
  //Use old-style indexed-based loop because the string-vector properties m_objectList, m_par_iovtags and are index-parallel
  for (unsigned int iobj = 0; iobj < m_objectList.size(); ++iobj) {
    //add a new element and keep reference to it
    obj_t& objt=objs.emplace_back();
    const std::string& objStr = m_objectList[iobj];
    objt.tag = iobj < m_par_iovtags.size() ? m_par_iovtags[iobj] : "";
    objt.tagDescr = iobj < m_par_tagDescr.size() ? m_par_tagDescr[iobj] : "";

    //Object-string definition: typename#key#folder (where key and folder are optional)
    //split object-string by '#' 
    auto hashSplit = std::ranges::views::split(objStr,std::string_view("#"));
    for (const auto [iHash, ss] : std::views::enumerate(hashSplit)) {
      switch(iHash) {
        case 0:
          objt.type=std::string_view(ss);
          break;
        case 1: 
          objt.key=std::string_view(ss);
          break;
        case 2:
          objt.folder=std::string_view(ss);
          break;
        default:
          ATH_MSG_ERROR("Ill-formed object list " << objStr);
          ATH_MSG_ERROR("Expect format typename#key#folder (where key and folder are optional)");
          return StatusCode::FAILURE;
      }
    }

    CLID clid;
    ATH_CHECK(p_clidsvc->getIDOfTypeName(objt.type, clid));
    if (objt.key.empty()) {
      //Try to get Key from proxy:
      objt.proxy = detStore()->proxy(clid);
      if (!objt.proxy) {
        ATH_MSG_ERROR("Could not get default proxy for CLID " << clid << " typename " << objt.type);
        return StatusCode::FAILURE;
      }
      objt.key = objt.proxy->name();
    }
    else {
      objt.proxy = detStore()->proxy(clid, objt.key);
      if (!objt.proxy) {
        ATH_MSG_ERROR("Could not get proxy for CLID " << clid << " typename " << objt.type << ", key " << objt.key);
        return StatusCode::FAILURE;
      }

    }

    if (objt.folder.empty()) {
      objt.folder=objt.key;
    }

  } //end loop over m_objectList

  // list out all typename/key pairs to be written and construct TypeKeyPairs for the streamer
  const size_t nObjects = objs.size();
  IAthenaOutputStreamTool::TypeKeyPairs typeKeys;
  ATH_MSG_INFO("Identified a total of " << nObjects << " objects to write out:");
  // leave now if nothing to write
  if (nObjects == 0)
    return StatusCode::SUCCESS;

  for (unsigned i=0;const auto& objt : objs) {
    typeKeys.emplace_back(std::make_pair(objt.type, objt.key));
    ATH_MSG_INFO(i++ << ": " << objt.type << "#" << objt.key << "#" << objt.folder);
  }
   

  // stream output (write objects)
  sc = m_streamer->streamObjects(typeKeys);
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Could not stream out objects");
    return StatusCode::FAILURE;
  }
  // commit output
  sc = m_streamer->commitOutput();
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Could not commit output stream");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Written " << nObjects << " objects to output stream");

  if (m_par_writeIOV) {
    // ======== COOL writing part (legacy) ===========:

    if (m_par_crestDir.empty()) {
      msg() << MSG::INFO << "Register objects in IOV database, interval of validity ";
      if (m_par_timestamp) {
        msg() << "[time] from [" << m_par_time1.value() << "] to [" << m_par_time2.value() << "]" << endmsg;
      } else {
        msg() << "[run,LB] from [" << m_par_run1.value() << "," << m_par_lumib1.value() << "] to [" << m_par_run2.value() << "," << m_par_lumib2.value() << "]"
              << endmsg;
      }
      int nreg = 0;
      for (const auto& objt : objs) {
        msg() << MSG::INFO << "Register object " << objt.type << "#" << objt.key << " in IOV database folder " << objt.folder << " ";
        if (objt.tag == "") {
          msg() << MSG::INFO << "without tagging" << endmsg;
        } else {
          msg() << MSG::INFO << "with tag " << objt.tag << endmsg;
        }
        if (m_par_timestamp) {
          sc = p_regsvc->registerIOV(objt.type, objt.key, objt.folder, objt.tag, timeToNano(m_par_time1), timeToNano(m_par_time2));
        } else {
          sc = p_regsvc->registerIOV(objt.type, objt.key, objt.folder, objt.tag, m_par_run1, m_par_run2, m_par_lumib1, m_par_lumib2);
        }
        if (sc == StatusCode::SUCCESS) {
          ++nreg;
        } else {
          ATH_MSG_ERROR("Registration failed!");
        }
      }
      ATH_MSG_INFO("Registered " << nreg << " objects in COOL IOV database");
    } else {
      // ======== CREST writing part ===========:

      ATH_MSG_INFO("Opening crest local directroy " << m_par_crestDir);
      chai::Database db = chai::Database("crest_fs:" + m_par_crestDir);
      // Create a global tag. Simplifes reading this local crest dir by athena
      const std::string globalTagName = "GlobalTestTag";
      chai::GlobalTagPtr gt;
      std::vector<std::string> existingGlobalTags;
      try {
        existingGlobalTags = db.findGlobalTags(globalTagName);
      }
      catch (std::exception& e) {
        //no global tag exists, not a problem
      } 
      if (existingGlobalTags.empty()) {
        gt = db.createGlobalTag("GlobalTestTag", "test", "test");    
        ATH_MSG_INFO("Created Global Tag for testing (" << globalTagName << ") in " << m_par_crestDir);
      } else {
        gt = db.getGlobalTag(globalTagName);
        ATH_MSG_INFO("Global Tag for testing (" << globalTagName << ") exists already in " << m_par_crestDir);
      }

      // Some of the code here is taken from RegistrationSvc. Once we deprecate COOL, we can also delete (I)RegistrationSvc
      std::string address_data;
      // Loop over objects  ....
      for (const auto& objt : objs) {
        chai::TagIovType iovType;
        uint64_t since;
        if (m_par_timestamp) {
          iovType = chai::Tag::IovType::Time;
          since = timeToNano(m_par_time1);
        } else {
          iovType = chai::Tag::IovType::RunNumberLumiBlock;
          since = m_par_run1 << 32 + m_par_lumib1;
        }
        const CLID clid = objt.proxy->clID();

        ATH_MSG_INFO("Working on object clid " << clid << ", " << objt.folder << " " << objt.type << " " << objt.key);

        // First, deal with teh folder description:
        std::string description;
        if (clid == 40774348 || clid == 1238547719) {
          // AthenaAttibuteList or CondAttrListCollection. The description can be build by chai ...
          description = chai::Tag::buildNodeDescription(iovType, objt.type, clid);
        } else {
          // Pool referenced storage, need to build the node description by ourselves
          IOpaqueAddress* addr = objt.proxy->address();
          if (!addr) {
            ATH_MSG_ERROR("No IOpaqueAddress from Type/Key [" << objt.type << "/" << objt.key << "]");
            return StatusCode::FAILURE;
          }
          std::string saddr;
          ATH_CHECK(m_persSvc->convertAddress(addr, saddr));
          // Split the string address into header and data parts
          std::string address_header;

          if (splitAddress(saddr, address_header, address_data).isFailure()) {
            ATH_MSG_ERROR("Could not split address: " << "addr: " << saddr << "\n"
                                                      << "hdr:  " << address_header << "\n"
                                                      << "data  " << address_data);
            return StatusCode::FAILURE;
          }
          ATH_MSG_DEBUG("split address: " << saddr << endmsg << "  hdr:  " << address_header << endmsg << "  data: " << address_data);
          // We store extra information in the folder description.
          // This info is:
          //   typeName       - required information
          //   symlinks       - the extra StoreGate keys, if any
          //   key            - optional, only needed if key != folder name
          //   timeStamp      - either run-lumi (default) or time
          //   address_header - added by convention
          //
          //
          // The convention is that the address_header is stored
          // in the description, and the IOV interval data
          // payload is just the pool reference in string form.
          // The address_header and address_data can be obtained
          // from the string address returned from the
          // persistency service, using splitAddress

          // Add symlinks (actually base-class types):
          std::string symLinkTypes;
          for (const CLID& c : objt.proxy->transientID()) {
            if (c != clid) {
              std::string symType;
              ATH_CHECK(p_clidsvc->getTypeNameOfID(c, symType));
              if (!symLinkTypes.empty())
                symLinkTypes += ":";  // Separate type names by colons
              symLinkTypes.append(symType);
            }
          }  // end loop over transientIDs
          if (!symLinkTypes.empty()) {
            buildDescription("symLink", symLinkTypes, description);
          }

          // IOV type:
          if (iovType == chai::Tag::IovType::Time) {
            buildDescription("timeStamp", "time", description);
          } else {
            buildDescription("timeStamp", "run-lumi", description);
          }

          // Address header:
          buildDescription("addrHeader", address_header, description);

          // SG key if needed:
          if (objt.key != objt.folder) {
            buildDescription("key", objt.key, description);
          }
        }  // end if POOL referenced storage

        chai::Tag::Metadata chaiMD{.iovType = iovType,
                                   .objectType = "crest-json-single-iov",
                                   .synchronization = chai::Tag::Synchronization::All,
                                   .status = chai::Tag::Status::Unlocked,
                                   .nodeDescription = description};

        

        // Description and metadat ready, now deal with the payload
        IOpaqueAddress* addr = objt.proxy->address();

        CondAttrListCollAddress* collAddr = dynamic_cast<CondAttrListCollAddress*>(addr);
        if (collAddr) {

          // Multi-channel inline storage or multi-channel POOL storage
          const CondAttrListCollection* attrListColl = collAddr->attrListColl();
          // convert to multi-channel chai::container and ....
          chai::Container chaiCont=ChaiCoralConverter::toContainer(*attrListColl);
          ATH_MSG_INFO("Created chai container with " << chaiCont.numChannels() << " channels.");
          const chai::PayloadSpec& chaiSpec=chaiCont.payloadSpec();
          auto chaiTag=db.createTag(objt.tag, objt.tagDescr, chaiSpec,chaiMD);
          chaiTag->addPayload(chaiCont,since);


        } else if (clid == 40774348) { 
          // Attribute list (single-channel inline).. retrieve from detStore

          const AthenaAttributeList* attrList;
          ATH_CHECK(detStore()->retrieve(attrList, objt.key));
          // convert to single-channel chai::container
          chai::Container chaiCont=ChaiCoralConverter::toContainer(*attrList);
          const chai::PayloadSpec& chaiSpec=chaiCont.payloadSpec();
          auto chaiTag=db.createTag(objt.tag, objt.tagDescr, chaiSpec,chaiMD);
          gt->addTag(objt.folder,objt.tag); 
          chaiTag->addPayload(chaiCont,since);
        } else {

          //Single channel pool storage case:
          chai::PayloadSpec spec(chai::FieldSpec({{"PoolRef", chai::Type::String}}), chai::ChannelSpec({{0, ""}}));
          auto tag = db.createTag(objt.tag, objt.tagDescr, spec,chaiMD);
          gt->addTag(objt.folder, objt.tag);
          chai::Container container = tag->buildContainer();
          container[0].push(address_data);
          tag->addPayload(container, since);

        }  // end else single-channel pool storage
      }  // end loop over objects
      ATH_MSG_INFO("Registered " << objs.size() << " objects in CREST IOV database");
    }  // end if write crest
  }  // end doWriteIOV
  else {
    ATH_MSG_INFO("Objects NOT registered in IOV database");
  }
  return StatusCode::SUCCESS;
}

uint64_t OutputConditionsAlg::timeToNano(unsigned long int timesec) const
{
  // convert time specified in seconds to ns used by COOL
  // use the magic value MAXEVENT to signal full range
  if (timesec==IOVTime::MAXEVENT) {
    return IOVTime::MAXTIMESTAMP;
  } else {
    return static_cast<uint64_t>(timesec)*1000000000;
  }
}

void OutputConditionsAlg::buildDescription(const std::string& identifier, const std::string& value, std::string& description) const {

  std::string fragment="<"+identifier+">"+value+"<\\"+identifier+">";
  description.insert(0,fragment);
  return;  
}


StatusCode OutputConditionsAlg::splitAddress(const std::string& address,
                                  std::string& address_header,
                                  std::string& address_data ) const {
  // Deals with address of form
  // <address_header service_type="256" clid="1238547719" /> POOLContainer_CondAttrListCollection][CLID=x
  // return header as part up to and including />, trailer as rest

  std::string::size_type p1=address.find(" />");
  if (p1!=std::string::npos) {
    address_header=address.substr(0,p1+3);
    address_data=address.substr(p1+4);
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}