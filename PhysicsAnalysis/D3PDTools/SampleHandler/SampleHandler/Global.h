/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef SAMPLE_HANDLER_GLOBAL_HH
#define SAMPLE_HANDLER_GLOBAL_HH

/// This module provides a lot of global definitions, forward
/// declarations and includes that are used by all modules.

namespace SH
{
  class DiskList;
  class DiskListEOS;
  class DiskListLocal;
  class DiskListSRM;
  class DiskListXRD;
  class DiskOutput;
  class DiskOutputLocal;
  class DiskOutputXRD;
  class DiskWriter;
  class DiskWriterLocal;
  class DiskWriterXRD;
  class Meta;
  template<class T> class MetaData;
  struct MetaDataQuery;
  struct MetaDataSample;
  struct MetaFields;
  struct MetaNames;
  class MetaObject;
  template<class T> class MetaVector;
  class Sample;
  class SampleComposite;
  class SampleGrid;
  class SampleHandler;
  class SampleHist;
  class SampleLocal;
  class SampleMeta;
  struct ScanDir;
  class TagList;
}

#endif
