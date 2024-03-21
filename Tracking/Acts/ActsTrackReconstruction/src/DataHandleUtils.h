/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/CondHandleKeyArray.h"

/** Macro to create a data handle and get the pointer to the container.
 * @param ctx the event context
 * @param handle_key the data handle key for which the data handle should be created which must not be empty.
 * @param data_name_for_msg a string which describes the container e.g. "tracks" for a TrackContainer
 * @param ptr an allocated const ptr of the container (base) type set to  the const pointer returned by the handle.
 * The macro can be used for a ReadHandleKey or ReadCondHandleKey e.g. in the following way:
 * <pre>
 *   <code>
 *   const ActsTrk::TrackContainer *tracksContainer=nullptr;
 *   MAKE_CHECKED_HANDLE(ctx, m_tracksContainerKey, "tracks", tracksContainer);
 *   </code>
 * </pre>
 */
#define MAKE_CHECKED_HANDLE(ctx, handle_key, data_name_for_msg, ptr ) { auto handle=SG::makeHandle(handle_key,ctx); \
     if (!handle.isValid()) { \
        ATH_MSG_ERROR("No " data_name_for_msg " tracks for key " << handle_key.key() ); \
        return StatusCode::FAILURE; \
     } \
     ptr = handle.cptr(); \
   } \
   do {} while (0)

/** Macro to create an optional data handle and get the pointer to the container.
 * @param ctx the event context
 * @param handle_key the data handle key for which the data handle should be created which may be empty.
 * @param data_name_for_msg a string which describes the container e.g. "tracks" for a TrackContainer
 * @param ptr an allocated const ptr of the container (base) type set to  the const pointer returned by the handle.
 * The macro can be used for a  ReadHandleKey or ReadCondHandleKey e.g. in the following way:
 * <pre>
 *   <code>
 *   const ActsTrk::TrackContainer *tracksContainer=nullptr;
 *   MAKE_CHECKED_OPTIONAL_HANDLE(ctx, m_tracksContainerKey, "tracks", tracksContainer);
 *   </code>
 * </pre>
 */
#define MAKE_CHECKED_OPTIONAL_HANDLE(ctx, handle_key, data_name_for_msg, ptr) if (!handle_key.empty()) { \
   MAKE_CHECKED_HANDLE(ctx,handle_key,data_name_for_msg,ptr); \
   } \
   do {} while (0)

