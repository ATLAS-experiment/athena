/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Package pool/POOLCore - some Athena default settings
#include "POOLCore/APRDefaults.h"

// Package pool/StorageSvc - interfaces
#include "StorageSvc/IStorageSvc.h"
#include "StorageSvc/IOODatabase.h"
#include "StorageSvc/IDbContainer.h"
#include "StorageSvc/IDbDatabase.h"
#include "StorageSvc/IDbDomain.h"
#include "StorageSvc/FileDescriptor.h"

// Package pool/StorageSvc - implementation
#include "StorageSvc/DbArray.h"
#include "StorageSvc/DbBlob.h"
#include "StorageSvc/DbConnection.h"

#include "StorageSvc/DbObject.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbDomain.h"
#include "StorageSvc/DbSession.h"

#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbToken.h"
#include "StorageSvc/DbString.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbTypeInfo.h"

