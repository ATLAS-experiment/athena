/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file
 * @author Sebastian Boeser <sboeser --at-- hep.ucl.ac.uk>
 * Standalone JiveXML server
 * This application provides a standalone ONCRPC (SunRPC) server that receives
 * events (in the JiveXML format) from an (athena) application and servers
 * these same events to the AtlantisJava client.
 * The actual serving thread is part of the JiveXML package, which also provides a
 * serving thread controlled by Athena (JiveXML/ONCRCPServerSvc).
 * This package is intended to be used only in the Atlas Online environment.
 */

#include <JiveXMLServer/JiveXMLServer.h>

//tdaq includes
#include <ipc/core.h>
#include <ers/ers.h>


/**
 * Main routine
 */
int main(int argc, char ** argv)
{
  // Initialise IPC
  try {
     IPCCore::init(argc,argv);
  } catch (daq::ipc::Exception & ex) {
     ers::fatal( ex );
  }

  //Say hello
  ERS_INFO("Starting JiveXML server");
  
  //Create the object and start the server
  JiveXML::JiveXMLServer server;
  
  //Now wait for the server to finish
  server.Wait();

  //Leaving the scope will call ~JiveXMLServer and destroy the server
  //so no explicit shutdown needed

  return 0;
}

