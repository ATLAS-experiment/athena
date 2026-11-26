# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Configuration for Codec tools"""

from AthenaConfiguration.ComponentFactory import CompFactory
from AthRemoteExecCore.RemoteExecCodecs import Codec, Source


class ByteStream(Codec):
    """Codec for sending over complete RawEvent.

    An example of using this codec:
    ```python
    remote.add(
        ...
        boundaries={
            "bytestream": ByteStream(),
            ...
        },
        ...
    )
    ```
    or if only a subset of detectors is needed:
    ```python
    remote.add(
        ...
        boundaries={
            "bytestream": ByteStream(KeepDetectors=["PIXEL", "SCT"]),
            ...
        },
        ...
    )
    ```
    """

    encoding = "bytestream"
    tool_type = CompFactory.AthRemoteExec.BytestreamCodec
    schema_source = Source.none
