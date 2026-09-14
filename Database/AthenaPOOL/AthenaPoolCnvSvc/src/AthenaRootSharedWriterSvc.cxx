/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file AthenaRootSharedWriterSvc.cxx
 *  @brief This file contains the implementation for the AthenaRootSharedWriterSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "GaudiKernel/IAlgManager.h"
#include "AthenaRootSharedWriterSvc.h"

#include "TBranch.h"
#include "TClass.h"
#include "TFile.h"
#include "TFileMerger.h"
#include "TKey.h"
#include "TLeaf.h"
#include "TMemFile.h"
#include "TMessage.h"
#include "TMonitor.h"
#include "TServerSocket.h"
#include "TSocket.h"
#include "TString.h"
#include "TSystem.h"
#include "TTree.h"

#include <set>
#include <map>

void* getCachedDummyAddress(TClass* cl, std::unordered_map<TClass*, void*>& cache) 
{
   if (!cl) return nullptr;

   void*& map_ptr = cache[cl];
   if (!map_ptr) {
      map_ptr = cl->New();
   }
   
   return &map_ptr;
}

/* Code from ROOT tutorials/net/parallelMergeServer.C, reduced to handle TTrees only */

struct ParallelFileMerger : public TObject
{
   TString       fFilename;
   TFileMerger   fMerger;
   std::unordered_map<TClass*, void*> *m_cache;

   ParallelFileMerger(const char *filename, std::unordered_map<TClass*, void*>* cache, int compress = ROOT::RCompressionSetting::EDefaults::kUseCompiledDefault
         ) : fFilename(filename), fMerger(kFALSE, kTRUE), m_cache(cache)
   {
      fMerger.OutputFile(filename, "RECREATE", compress);
   }

   ~ParallelFileMerger()
   {
   }

   ULong_t Hash() const
   {
      return fFilename.Hash();
   }

   const char* GetName() const
   {
      return fFilename;
   }

// Add missing branches to client tree and BackFill before merging
   bool syncBranches(TTree* fromTree, TTree* toTree)
   {
      bool updated = false;
      const TObjArray* fromBranches = fromTree->GetListOfBranches();
      const TObjArray* toBranches = toTree->GetListOfBranches();
      int nBranches = fromBranches->GetEntriesFast();
      int nEntries = toTree->GetEntries();
      for (int k = 0; k < nBranches; ++k) {
         TBranch* branch = static_cast<TBranch*>(fromBranches->UncheckedAt(k));
         if (toBranches->FindObject(branch->GetName()) == nullptr) {
            TBranch* newBranch = nullptr;
            TClass* cl = TClass::GetClass(branch->GetClassName());
            if (cl != nullptr) {
               newBranch = toTree->Branch(branch->GetName(), branch->GetClassName(), nullptr, branch->GetBasketSize(), branch->GetSplitLevel());
               void* empty = getCachedDummyAddress(cl, *m_cache);
               newBranch->SetAddress(empty);
            } else {
               TObjArray* outLeaves = branch->GetListOfLeaves();
               TLeaf* leaf = static_cast<TLeaf*>(outLeaves->UncheckedAt(0));
               std::string_view attr = leaf->GetName();
               std::string_view type = leaf->GetTypeName();
               std::string branchSpec(attr);
               branchSpec += '/';
               if (type == "Int_t")        branchSpec += 'I';
               else if (type == "Short_t") branchSpec += 'S';
               else if (type == "Long_t")  branchSpec += 'L';
               else if (type == "UInt_t")  branchSpec +='i';
               else if (type == "UShort_t") branchSpec+='s';
               else if (type == "ULong_t") branchSpec +='l';
               else if (type == "Float_t") branchSpec +='F';
               else if (type == "Double_t") branchSpec+='D';
               else if (type == "Char_t")  branchSpec +='B';
               else if (type == "UChar_t") branchSpec +='b';
               else if (type == "Bool_t")  branchSpec +='O';
               else                       { branchSpec += type; }// fallback
               newBranch = toTree->Branch(branch->GetName(), static_cast<void*>(nullptr), branchSpec.c_str(), 2048);
            }
            for (int m = 0; m < nEntries; ++m) {
               newBranch->BackFill();
            }
            updated = true;
         }
      }
      return updated;
   }

   Bool_t MergeTrees(TFile *input)
   {
      fMerger.AddFile(input);
      TIter nextKey(input->GetListOfKeys());
      while (TKey* key = static_cast<TKey*>(nextKey())) {
         TClass* cl = TClass::GetClass(key->GetClassName());
         if (cl != nullptr && cl->InheritsFrom("TTree")) {
            TTree* outCollTree = static_cast<TTree*>(fMerger.GetOutputFile()->Get(key->GetName()));
            TTree* inCollTree = static_cast<TTree*>(input->Get(key->GetName()));
            if (inCollTree != nullptr && outCollTree != nullptr) {
               if (syncBranches(outCollTree, inCollTree)) {
                  input->Write();
               }
               syncBranches(inCollTree, outCollTree);
            }
         }
      }

      Bool_t result = fMerger.PartialMerge(TFileMerger::kIncremental | TFileMerger::kResetable | TFileMerger::kKeepCompression);
      nextKey = input->GetListOfKeys();
      while (TKey* key = static_cast<TKey*>(nextKey())) {
         TClass* cl = TClass::GetClass(key->GetClassName());
         if (cl != nullptr && 0 != cl->GetResetAfterMerge()) {
            key->Delete();
            input->GetListOfKeys()->Remove(key);
            delete key;
         }
      }
      return result;
   }
};

//___________________________________________________________________________
AthenaRootSharedWriterSvc::AthenaRootSharedWriterSvc(const std::string& name, ISvcLocator* pSvcLocator)
  : base_class(name, pSvcLocator)
  , m_rootServerSocket(nullptr), m_rootMonitor(nullptr), m_rootMergers(), m_rootClientIndex(0), m_rootClientCount(0), m_numberOfStreams(0) {
}
//___________________________________________________________________________
StatusCode AthenaRootSharedWriterSvc::initialize() {
   ATH_MSG_INFO("in initialize()");

   // Initialize IConversionSvc
   ATH_CHECK(m_cnvSvc.retrieve());
   IProperty* propertyServer = dynamic_cast<IProperty*>(m_cnvSvc.get());
   if (propertyServer == nullptr) {
      ATH_MSG_ERROR("Unable to cast conversion service to IProperty");
      return StatusCode::FAILURE;
   } else {
      std::string propertyName = "ParallelCompression";
      bool parallelCompression(false);
      BooleanProperty parallelCompressionProp(propertyName, parallelCompression);
      if (propertyServer->getProperty(&parallelCompressionProp).isFailure()) {
         ATH_MSG_INFO("Conversion service does not have ParallelCompression property");
      } else if (parallelCompressionProp.value()) {
         propertyName = "StreamPortString";
         std::string streamPortString("");
         StringProperty streamPortStringProp(propertyName, streamPortString);
         if (propertyServer->getProperty(&streamPortStringProp).isFailure()) {
            ATH_MSG_INFO("Conversion service does not have StreamPortString property, using default TCP port: 0");
            streamPortStringProp.setValue("?pmerge=localhost:0");
         }
         const std::string& pmergeProperty = streamPortStringProp.value();
         const std::size_t eqPos = pmergeProperty.find('=');
         if (eqPos == std::string::npos) {
            ATH_MSG_FATAL("Malformed StreamPortString property (missing '='): " << pmergeProperty);
            return StatusCode::FAILURE;
         }
         const std::string pmergePrefix = pmergeProperty.substr(0, eqPos + 1);
         const std::string pmergeArg = pmergeProperty.substr(eqPos + 1);
         if (pmergeArg.empty()) {
            ATH_MSG_FATAL("Malformed StreamPortString property (empty value after '='): " << pmergeProperty);
            return StatusCode::FAILURE;
         }
         std::string newStreamPortString;
         // "?pmerge=<host>:<port>" (default) selects a TCP socket.
         // Anything else (e.g. "?pmerge=<prefix>") is passed to gSystem->TempFileName() as a
         // (possibly relative) prefix for a UNIX domain socket file, created as $TMPDIR/<prefix>XXXXXX.
         if (pmergeArg.find(':') == std::string::npos) {
            TString socketPath = pmergeArg.c_str();
            FILE* dummy = gSystem->TempFileName(socketPath);
            if (dummy == nullptr) {
               ATH_MSG_FATAL("Could not create temporary file for UNIX domain socket: " << pmergeArg);
               return StatusCode::FAILURE;
            }
            m_socketPath = socketPath.Data();
            std::remove(m_socketPath.c_str());
            std::fclose(dummy);
            m_rootServerSocket = new TServerSocket(socketPath);
            if (m_rootServerSocket == nullptr || !m_rootServerSocket->IsValid()) {
               ATH_MSG_FATAL("Could not create ROOT TServerSocket (UNIX domain socket): " << m_socketPath);
               return StatusCode::FAILURE;
            }
            newStreamPortString = pmergePrefix + m_socketPath;
            ATH_MSG_DEBUG("Successfully created ROOT TServerSocket (UNIX domain socket) and added it to TMonitor: ready to accept connections, " << m_socketPath);
         } else {
            const std::size_t colonPos = pmergeArg.find(':');
            int streamPort = atoi(pmergeArg.substr(colonPos + 1).c_str());
            m_rootServerSocket = new TServerSocket(streamPort, (streamPort == 0 ? false : true), 100, -1, ESocketBindOption::kInaddrLoopback);
            if (m_rootServerSocket == nullptr || !m_rootServerSocket->IsValid()) {
               ATH_MSG_FATAL("Could not create ROOT TServerSocket: " << streamPort);
               return StatusCode::FAILURE;
            }
            streamPort = m_rootServerSocket->GetLocalPort();
            newStreamPortString = pmergePrefix + pmergeArg.substr(0, colonPos + 1) + std::to_string(streamPort);
            ATH_MSG_DEBUG("Successfully created ROOT TServerSocket and added it to TMonitor: ready to accept connections, " << streamPort);
         }
         if (propertyServer->setProperty(propertyName,newStreamPortString).isFailure()) {
            ATH_MSG_FATAL("Could not set Conversion Service property " << propertyName << " from " << streamPortString << " to " << newStreamPortString);
            return StatusCode::FAILURE;
         }
         m_rootMonitor = new TMonitor;
         m_rootMonitor->Add(m_rootServerSocket);
      }
   }
   // Count the number of output streams
   const IAlgManager* algMgr = Gaudi::svcLocator()->as<IAlgManager>();
   for (const auto& alg : algMgr->getAlgorithms()) {
      if (alg->type() == "AthenaOutputStream") {
         ATH_MSG_DEBUG("Counting " << alg->name() << " as an output stream algorithm");
         m_numberOfStreams++;
      }
   }
   if (m_numberOfStreams == 0) {
      ATH_MSG_WARNING("No output stream algorithm found, setting the number of streams to 1");
      m_numberOfStreams = 1;
   } else {
      ATH_MSG_INFO("Found a total of " << m_numberOfStreams << " output streams");
   }

   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode AthenaRootSharedWriterSvc::share(int numClients, bool motherClient) {
   ATH_MSG_DEBUG("Start commitOutput loop");
   StatusCode sc = m_cnvSvc->commitOutput("", false);

   // Allow ROOT clients to start up (by setting active clients)
   // and wait to stop the ROOT server until all clients are done and metadata is written (commitOutput fail).
   bool anyActiveClients = (m_rootServerSocket != nullptr);
   while (sc.isSuccess() || sc.isRecoverable() || anyActiveClients) {
      if (sc.isSuccess()) {
         ATH_MSG_VERBOSE("Success in commitOutput loop");
      } else if (m_rootMonitor != nullptr) {
         TSocket* socket = m_rootMonitor->Select(1);
         if (socket != nullptr && socket != (TSocket*)-1) {
            ATH_MSG_DEBUG("ROOT Monitor got: " << socket);
            if (socket->IsA() == TServerSocket::Class()) {
               TSocket* client = (static_cast<TServerSocket*>(socket))->Accept();
               client->Send(m_rootClientIndex, 0);
               client->Send(1, 1);
               ++m_rootClientIndex;
               ++m_rootClientCount;
               if (m_rootClientCount < (numClients-1)*m_numberOfStreams + 1) {
                  m_rootMonitor->Add(client);
                  ATH_MSG_INFO("ROOT Monitor add client: " << m_rootClientIndex << ", " << client);
               } else {
                  ATH_MSG_WARNING("ROOT Monitor do NOT add client: " << m_rootClientIndex << ", " << client);
                  client->Close("force");
                  --m_rootClientCount;
               }
            } else {
               TMessage* message = nullptr;
               Int_t result = socket->Recv(message);
               if (result < 0) {
                  ATH_MSG_ERROR("ROOT Monitor got an error while receiving the message from the socket: " << result);
                  return StatusCode::FAILURE;
               }
               if (message == nullptr) {
                  ATH_MSG_WARNING("ROOT Monitor got no message from socket: " << socket);
               } else if (message->What() == kMESS_STRING) {
                  char str[64];
                  message->ReadString(str, 64);
                  ATH_MSG_INFO("ROOT Monitor client: " << socket << ", " << str);
                  m_rootMonitor->Remove(socket);
                  ATH_MSG_DEBUG("ROOT Monitor client: " << socket << ", " << socket->GetBytesRecv() << ", " << socket->GetBytesSent());
                  socket->Close();
                  --m_rootClientCount;
                  if (m_rootMonitor->GetActive() == 0 || m_rootClientCount == 0) {
                     if (!motherClient) {
                        anyActiveClients = false;
                        ATH_MSG_INFO("ROOT Monitor: No more active clients...");
                     } else {
                        motherClient = false;
                        ATH_MSG_INFO("ROOT Monitor: Mother process is done...");
                        if (!m_cnvSvc->commitCatalog().isSuccess()) {
                           ATH_MSG_FATAL("Failed to commit file catalog.");
                           return StatusCode::FAILURE;
                        }
                     }
                  }
               } else if (message->What() == kMESS_ANY) {
                  long long length;
                  TString filename;
                  int clientId;
                  message->ReadInt(clientId);
                  message->ReadTString(filename);
                  message->ReadLong64(length);
                  ATH_MSG_DEBUG("ROOT Monitor client: " << socket << ", " << clientId << ": " << filename << ", " << length);
                  std::unique_ptr<TMemFile> transient(new TMemFile(filename, message->Buffer() + message->Length(), length, "UPDATE"));
                  message->SetBufferOffset(message->Length() + length);
                  ParallelFileMerger* info = static_cast<ParallelFileMerger*>(m_rootMergers.FindObject(filename));
                  if (!info) {
                     info = new ParallelFileMerger(filename, &m_dummyCache, transient->GetCompressionSettings());
                     m_rootMergers.Add(info);
                     ATH_MSG_INFO("ROOT Monitor ParallelFileMerger: " << info << ", for: " << filename);
                  }
                  info->MergeTrees(transient.get());
               }
               delete message; message = nullptr;
            }
         }
      } else if (m_rootMonitor == nullptr) {
         usleep(100);
      }
      // Once commitOutput failed all legacy clients are finished (writing metadata), do not call again.
      if (sc.isSuccess() || sc.isRecoverable()) {
         sc = m_cnvSvc->commitOutput("", false);
         if (sc.isFailure() && !sc.isRecoverable()) {
            ATH_MSG_INFO("commitOutput failed, metadata done.");
            if (anyActiveClients && m_rootClientCount == 0) {
              ATH_MSG_INFO("ROOT Monitor: No clients, terminating the loop...");
              anyActiveClients = false;
            }
         }
      }
   }
   ATH_MSG_INFO("End commitOutput loop");
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode AthenaRootSharedWriterSvc::stop() {
   m_rootMergers.Delete();
   return StatusCode::SUCCESS;
}
//___________________________________________________________________________
StatusCode AthenaRootSharedWriterSvc::finalize() {
   ATH_MSG_INFO("in finalize()");
   delete m_rootMonitor; m_rootMonitor = nullptr;
   delete m_rootServerSocket; m_rootServerSocket = nullptr;
   if (!m_socketPath.empty()) {
      std::remove(m_socketPath.c_str());
   }
   for (auto& [cl, ptr] : m_dummyCache) {
      if (cl && ptr) {
         cl->Destructor(ptr, false);
      }
   }
   m_dummyCache.clear();
   return StatusCode::SUCCESS;
}
