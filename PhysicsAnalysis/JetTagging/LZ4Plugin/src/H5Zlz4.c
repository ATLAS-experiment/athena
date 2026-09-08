/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 *
 * This file is an example of an HDF5 filter plugin.
 * The plugin can be used with the HDF5 library vesrion 1.8.11+ to read
 * HDF5 datasets compressed with lz4.
 *


HDF5 LZ4 compression filter plugin
Copyright 2013-2015 by The HDF Group.

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted for any purpose (including commercial purposes)
provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions, and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions, and the following disclaimer in the documentation
   and/or materials provided with the distribution.

3. In addition, redistributions of modified forms of the source or binary
   code must carry prominent notices stating that the original code was
   changed and the date of the change.

4. All publications or advertising materials mentioning features or use of
   this software are asked, but not required, to acknowledge that it was
   developed by The HDF Group and credit the contributors.

5. Neither the name of The HDF Group, nor the name of any Contributor may
   be used to endorse or promote products derived from this software
   without specific prior written permission from The HDF Group or the
   Contributor, respectively.

DISCLAIMER:
THIS SOFTWARE IS PROVIDED BY THE HDF GROUP AND THE CONTRIBUTORS
"AS IS" WITH NO WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED.  In no
event shall The HDF Group or the Contributors be liable for any damages
suffered by the users arising out of the use of this software, even if
advised of the possibility of such damage.


HDF5 FILTER IMPLEMENTATION

   License for Dectris lz4-hdf5 filter plugin
   Copyright (C) 2011-2013, Dectris Ltd.
   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   Dectris homepage : http://www.dectris.com


Note that this code was _mostly_ taken from DiamondLightSource (see
README for more info). Places where it has been modified are commented
with "dguest".

*/

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "H5PLextern.h"
#include "lz4.h"
#include "LZ4Plugin/H5Zlz4.h"

/* dguest: had to add this */
#include <arpa/inet.h>

static size_t H5Z_filter_lz4(unsigned int flags, size_t cd_nelmts,
        const unsigned int cd_values[], size_t nbytes,
        size_t *buf_size, void **buf);

#define htonll(x) ( ( (uint64_t)(htonl( (uint32_t)((x << 32) >> 32)))<< 32) | htonl( ((uint32_t)(x >> 32)) ))
#define ntohll(x) htonll(x)

#define htobe16t(x) htons(x)
#define htobe32t(x) htonl(x)
#define htobe64t(x) htonll(x)
#define be16toht(x) ntohs(x)
#define be32toht(x) ntohl(x)
#define be64toht(x) ntohll(x)


#define DEFAULT_BLOCK_SIZE 1<<30; /* 1GB. LZ4 needs blocks < 1.9GB. */

const H5Z_class2_t H5Z_LZ4[1] = {{
        H5Z_CLASS_T_VERS,       /* H5Z_class_t version */
        (H5Z_filter_t)H5Z_FILTER_LZ4,         /* Filter id number             */
        1,              /* encoder_present flag (set to true) */
        1,              /* decoder_present flag (set to true) */
        "HDF5 lz4 filter; see http://www.hdfgroup.org/services/contributions.html",
        /* Filter name for debugging    */
        NULL,                       /* The "can apply" callback     */
        NULL,                       /* The "set local" callback     */
        (H5Z_func_t)H5Z_filter_lz4,         /* The actual filter function   */
}};

H5PL_type_t   H5PLget_plugin_type(void) {return H5PL_TYPE_FILTER;}
const void *H5PLget_plugin_info(void) {return H5Z_LZ4;}

static size_t H5Z_filter_lz4(unsigned int flags, size_t cd_nelmts,
        const unsigned int cd_values[], size_t nbytes,
        size_t *buf_size, void **buf)
{
    void * outBuf = NULL;
    size_t ret_value;

    if (flags & H5Z_FLAG_REVERSE)
    {
        uint32_t *i32Buf;
        uint32_t blockSize;
        char *roBuf;   /* pointer to current write position */
        uint64_t decompSize;
        const char* rpos = (char*)*buf; /* pointer to current read position */
        const uint64_t * const i64Buf = (uint64_t *) rpos;
        const uint64_t origSize = (uint64_t)(be64toht(*i64Buf));/* is saved in be format */
        rpos += 8; /* advance the pointer */

        i32Buf = (uint32_t*)rpos;
        blockSize = (uint32_t)(be32toht(*i32Buf));
        rpos += 4;
        if(blockSize>origSize)
            blockSize = origSize;

        if (NULL==(outBuf = malloc(origSize)))
        {
            printf("cannot malloc\n");
            goto error;
        }
        roBuf = (char*)outBuf;   /* pointer to current write position */
        decompSize     = 0;
        /// start with the first block ///
        while(decompSize < origSize)
        {
            uint32_t compressedBlockSize;  /// is saved in be format

            if(origSize-decompSize < blockSize) /* the last block can be smaller than blockSize. */
                blockSize = origSize-decompSize;
            i32Buf = (uint32_t*)rpos;
            compressedBlockSize =  be32toht(*i32Buf);  /// is saved in be format
            rpos += 4;
            if(compressedBlockSize == blockSize) /* there was no compression */
            {
                memcpy(roBuf, rpos, blockSize);
            }
            else /* do the decompression */
            {
              /* dguest: removed this, replaced with line below
                int compressedBytes = LZ4_uncompress(rpos, roBuf, blockSize);
              */
              /* LZ4_decompress_safe returns the number of bytes written to
                 the destination, where LZ4_uncompress returned the number
                 read from the source. The check is against blockSize, as in
                 the HDF Group version of this filter. */
              int decompressedBytes = LZ4_decompress_safe(rpos, roBuf, compressedBlockSize, blockSize);
              if(decompressedBytes != (int)blockSize)
                {
                    printf("decompressed size not the same: %d, != %d\n",
                           decompressedBytes, (int)blockSize);
                    goto error;
                }
            }

            rpos += compressedBlockSize;   /* advance the read pointer to the next block */
            roBuf += blockSize;            /* advance the write pointer */
            decompSize += blockSize;
        }
        free(*buf);
        *buf = outBuf;
        outBuf = NULL;
        ret_value = (size_t)origSize;  // should always work, as orig_size cannot be > 2GB (sizeof(size_t) < 4GB)
    }
    else /* forward filter */
    {
        size_t blockSize;
        size_t nBlocks;
        size_t outSize; /* size of the output buffer. Header size (12 bytes) is included */
        size_t block;
        uint64_t *i64Buf;
        uint32_t *i32Buf;
        char *rpos;      /* pointer to current read position */
        char *roBuf;    /* pointer to current write position */

        if (nbytes > INT32_MAX)
        {
            /* can only compress chunks up to 2GB */
            goto error;
        }

        if(cd_nelmts > 0 && cd_values[0] > 0)
        {
            blockSize = cd_values[0];
        }
        else
        {
            blockSize = DEFAULT_BLOCK_SIZE;
        }
        if(blockSize > nbytes)
        {
            blockSize = nbytes;
        }
        nBlocks = (nbytes-1)/blockSize +1;
        /* Each block is compressed with a destination capacity of
           LZ4_compressBound(blockSize), so the buffer has to hold that much
           per block, not LZ4_COMPRESSBOUND(nbytes) overall. Plus 4 bytes of
           block size per block and the 12 byte header. */
        if (NULL==(outBuf = malloc(nBlocks*((size_t)LZ4_COMPRESSBOUND(blockSize) + 4)
                + 4+8)))
        {
            goto error;
        }

        rpos  = (char*)*buf;      /* pointer to current read position */
        roBuf = (char*)outBuf;    /* pointer to current write position */
        /* header */
        i64Buf = (uint64_t *) (roBuf);
        i64Buf[0] = htobe64t((uint64_t)nbytes); /* Store decompressed size in be format */
        roBuf += 8;

        i32Buf =  (uint32_t *) (roBuf);
        i32Buf[0] = htobe32t((uint32_t)blockSize); /* Store the block size in be format */
        roBuf += 4;

        outSize = 12; /* size of the output buffer. Header size (12 bytes) is included */
        const unsigned int maxPossibleSize = 4294967295u - 4u;
        for(block = 0; block < nBlocks; ++block)
        {
            uint32_t compBlockSize; /// reserve space for compBlockSize
            size_t origWritten = block*blockSize;
            if(nbytes - origWritten < blockSize) /* the last block may be < blockSize */
                blockSize = nbytes - origWritten;

            /* dguest: removed this, replaced with line below
              compBlockSize = LZ4_compress(rpos, roBuf+4, blockSize); /// reserve space for compBlockSize */
            compBlockSize = LZ4_compress_default(
                rpos, roBuf + 4, blockSize, LZ4_compressBound(blockSize)); /// reserve space for compBlockSize
            if(!compBlockSize)
                goto error;
            if(compBlockSize >= blockSize) /* compression did not save any space, do a memcpy instead */
            {
                compBlockSize = blockSize;
                memcpy(roBuf+4, rpos, blockSize);
            }

            i32Buf =  (uint32_t *) (roBuf);
            i32Buf[0] = htobe32t((uint32_t)compBlockSize);  /* write blocksize */
            roBuf += 4;

            rpos += blockSize;     	/* advance read pointer */
            roBuf += compBlockSize;       /* advance write pointer */
            if (outSize >= maxPossibleSize){
              goto error;
            }
            outSize += compBlockSize + 4;
        }

        free(*buf);
        *buf = outBuf;
        *buf_size = outSize;
        outBuf = NULL;
        ret_value = outSize;

    }
    //outBuf = NULL at this point, having been set above
    return ret_value;


    error:
    if(outBuf)
        free(outBuf);
    outBuf = NULL;
    return 0;
}

