#!/usr/bin/env python

#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
import datetime, time
import uuid

version_str = ('Unused', 'Gregorian time based', 'DCE', 'MD5 based', 'RNG based', 'SHA',
               'Reordered Gregorian', 'Epoch time based', 'Custom') + ('Reserved',)*7

def uuid_to_ts( myuuid ) :
    # from web.archive.org/web/20110817134817/http://www.gossamer-threads.com:80/lists/python/dev/670680
    DTD_DELTA = ( datetime.datetime( *time.gmtime(0)[0:3] ) - 
                  datetime.datetime( 1582, 10, 15 ) )
    DTD_SECS_DELTA = DTD_DELTA.days * 86400 # 0x01b21dd213814000/1e7
    secs = myuuid.time / 1e7 
    secs_epoch = secs - DTD_SECS_DELTA 
    return datetime.datetime.fromtimestamp( secs_epoch )

def uuid_split( guid ):
    fields = guid.split('-')
    if len(fields) != 5: raise ValueError( f"Wrong number of fields in '{guid}'" )
    if len(fields[0]) != 8: raise ValueError( f"Wrong length of field#0: '{fields[0]}'" )
    if len(fields[1]) != 4: raise ValueError( f"Wrong length of field#1: '{fields[1]}'" )
    if len(fields[2]) != 4: raise ValueError( f"Wrong length of field#2: '{fields[2]}'" )
    if len(fields[3]) != 4: raise ValueError( f"Wrong length of field#3: '{fields[3]}'" )
    if len(fields[4]) != 12: raise ValueError( f"Wrong length of field#4: '{fields[4]}'" )
    return (fields[0], fields[1], fields[2], fields[3], fields[4])
    
# DDCCBBAA-FFEE-HHGG-IIJJ-KKLLMMNNOOPP to AABBCCDD-EEFF-GGHH-IIJJ-KKLLMMNNOOPP
def pool_to_uuid( mypool ) :
    print('POOL guid:', mypool)
    f0, f1, f2, f3, f4 = uuid_split(mypool)
    myuuid = '%s%s%s%s-%s%s-%s%s-%s-%s'\
             %(f0[6:8],f0[4:6],f0[2:4],f0[0:2],\
               f1[2:4],f1[0:2],f2[2:4],f2[0:2],f3,f4)
    print('UUID guid:', myuuid)
    return myuuid

# AABBCCDD-EEFF-GHIJ-xxxx-xxxxxxxxxxxx to HIJEEFFAABBCCDD
def timorder_uuid( myuuid ) :
    f0, f1, f2, f3, f4 = uuid_split(myuuid)
    myuuidto = '%s%s%s'%(f2[1:4],f1,f0)
    myUUID=uuid.UUID(myuuid)
    print('UUID:', myUUID)
    print('UUID time (100ns intervals since 15 Oct 1582):', myUUID.time, "hex:", hex(myUUID.time))
    print('UUID timeordered:', myuuidto)
    print('UUID timestamp:', uuid_to_ts(myUUID))
    return myuuidto

# DDCCBBAA-FFEE-IJGH-xxxx-xxxxxxxxxxxx to HIJEEFFAABBCCDD
def timorder_pool( mypool ) :
    print('POOL guid:', mypool)
    f0, f1, f2, f3, f4 = uuid_split(mypool)
    myuuidto = '%s%s%s%s%s%s%s%s'\
               %(f2[2:3],f2[0:2],f1[2:4],f1[0:2],\
                 f0[6:8],f0[4:6],f0[2:4],f0[0:2],)
    print('UUID timeordered:', myuuidto)
    return myuuidto


def printuuid( myuuid ) :
    myUUID=uuid.UUID(myuuid)
    print('UUID:', myUUID)
    print('UUID variant:', myUUID.variant)
    print('UUID version:', myUUID.version, '-',version_str[myUUID.version])
    print('UUID field#0 (time_low):             %12.8x (%12.8x)'\
          %( myUUID.fields[0], myUUID.time_low ))
    print('UUID field#1 (time_mid):             %12.4x (%12.4x)'\
          %( myUUID.fields[1], myUUID.time_mid ))
    print('UUID field#2 (time_hi_version):      %12.4x (%12.4x)'\
          %( myUUID.fields[2], myUUID.time_hi_version ))
    print('UUID field#3 (clock_seq_hi_variant): %12.2x (%12.2x)'\
          %( myUUID.fields[3], myUUID.clock_seq_hi_variant ))
    print('UUID field#4 (clock_seq_low):        %12.2x (%12.2x)'\
          %( myUUID.fields[4], myUUID.clock_seq_low ))
    print('UUID field#5 (node):                 %12.12x (%12.12x)'\
          %( myUUID.fields[5], myUUID.node ))
    if myUUID.version == 1:
        timorder_uuid( myuuid )
    print('================================================')

def usage():
    print(sys.argv[0],"-h | [-u] UUID")
    print("  -u : assume uuidgen standard format (not POOL)")
    sys.exit(0)

def example():
    # [avalassi@lxplus248 tcsh] ~ > date ; pool_gen_uuid ; uuidgen -t
    # A2E338DD-2265-DF11-AFE1-001E4F3E5C33
    # DD3A5AAC-6522-11DF-82EB-001E4F3E5C33
    
    guid1='DD3A5AAC-6522-11DF-82EB-001E4F3E5C33' # from uuidgen
    guid2='A2E338DD-2265-DF11-AFE1-001E4F3E5C33' # from POOL
    guid3='A2E338DD-2265-D011-AFE1-001E4F3E5C33' # POOL token for the year 1997
    printuuid( guid1 )
    printuuid( pool_to_uuid( guid2 ) )
    printuuid( pool_to_uuid( guid3 ) )


if __name__ == "__main__":
    regular = False
    if len(sys.argv) > 1:
        if sys.argv[1] == '-u':
            sys.argv.pop(1)
            regular = True
        if sys.argv[1] == '-h':
            usage()
        if len(sys.argv) < 2:
            usage()
        guid = sys.argv[1]
        if not regular: 
            # assume POOL format
            guid = pool_to_uuid( guid )
        printuuid( guid )
    else:
        example()

