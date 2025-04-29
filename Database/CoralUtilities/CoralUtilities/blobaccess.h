/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CORALUTILITIES_BLOBACCESS_H
#define CORALUTILITIES_BLOBACCESS_H

#include <string>
#include <memory>
#include <string_view>
#include "nlohmann/json.hpp"

class TTree;
class TMemFile;

namespace coral {
    class Blob;
}

namespace CoralUtilities {

    bool compressBlob        (const char*           , coral::Blob&);
    bool writeBlobFromString (const std::string_view, coral::Blob&);
    bool writeBlobFromJson   (const nlohmann::json& , coral::Blob&);
    bool writeBlobFromTTree  (      TTree*          , coral::Blob&);

    bool uncompressBlob      (const coral::Blob&, std::unique_ptr<unsigned char[]>&, unsigned long&);
    bool readBlobAsString    (const coral::Blob&, std::string&                   );
    bool readBlobAsJson      (const coral::Blob&, nlohmann::json&                );
    /** @brief Interprets the coral::Blob as a TTree instance. If the parsing was successful,
     *            the parsed tree pointer is assigned to an object and true is returned. Otherwise,
     *            the unique_ptr is set to nullptr with false
     *  @param blob: Coral database blob to interpret
     *  @param tree: Reference to the unique_ptr to which the TTree is interpreted
     *  @param name: Name of the TTree inside the memory blob
     */
    bool readBlobAsTTree(const coral::Blob& blob, std::unique_ptr<TTree>& tree,
                         const std::string_view name = "tree");

}

#endif
