# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Python side of the codecs round-trip test utilities

This module is the registry of the AthRemoteExecCodecs integration tests:
CodecsTopCfg accumulates one sequence per codec, and the generic client
runner picks a client configuration function from here by name (--client-cfg).
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthRemoteExecCodecs.Codecs import ByteStream
from AthRemoteExecCore.RemoteExecCodecs import Codec, Direction
from AthRemoteExecCore.RemoteExecFragments import (
    REMOTE_EXEC_REQUEST_KEY,
    RemoteExecTop,
    RequestAlg,
)
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg

TEST_INTS = "test_ints"

# Sub-detector groups the selection test keeps; all present in the RAW_RUN3 sample
SELECTION_GROUPS = ["PIXEL", "TILECAL", "MUON"]


class test_NamedIntsCodec(Codec):
    """Python helper for test_NamedIntsCodec"""

    encoding = "protobuf"
    tool_type = CompFactory.AthRemoteExec.test_NamedIntsCodec

    def __init__(self, values: list[str]):
        if not values:
            raise ValueError("test_NamedIntsCodec carries at least one named integer")
        super().__init__(TEST_INTS)
        self.values = list(values)

    def sgKeys(self, key: str) -> list[str]:
        return list(self.values)

    def directionProperties(self, direction: Direction) -> dict[str, list[str]]:
        if direction is Direction.WriteToSG:
            return {"OutputInts": list(self.values)}
        return {"InputInts": list(self.values)}

    def __repr__(self):
        return f"test_NamedIntsCodec({self.values!r})"


def test_RawEventDigestCfg(
    flags, name: str = "RemoteExecSeqByteStreamDigest"
) -> ComponentAccumulator:
    """Server side: digest the decoded RawEvent into two ints"""

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.test_RawEventDigestAlg(
            name,
            Cache="bs_in",
            Size="bsSize",
            Hash="bsHash",
        )
    )

    return cfg


def CodecsTopCfg(flags) -> RemoteExecTop:
    """Top level configuration. Attach all sequences that this package serves.

    Add one remote.add(...) block per codec.
    """
    remote = RemoteExecTop()

    remote.add(
        "RemoteExecSeqByteStream",
        test_RawEventDigestCfg(flags),
        outputs=["bsCheck"],
        boundaries={
            "bs_in": ByteStream(),
            "bsCheck": test_NamedIntsCodec(["bsSize", "bsHash"]),
        },
    )

    remote.add(
        "RemoteExecSeqByteStreamSelection",
        test_RawEventDigestCfg(flags, name="RemoteExecSeqByteStreamSelectionDigest"),
        outputs=["bsCheck"],
        boundaries={
            "bs_in": ByteStream(KeepDetectors=SELECTION_GROUPS),
            "bsCheck": test_NamedIntsCodec(["bsSize", "bsHash"]),
        },
    )

    return remote


def CodecsLoopCfg(flags, **kwargs) -> ComponentAccumulator:
    """Configure the entire remote (server) job serving every sequence of this package"""

    if flags.Concurrency.NumThreads < 1:
        raise ValueError("CodecsLoopCfg needs Concurrency.NumThreads >= 1")

    cfg = MainServicesCfg(flags)
    cfg.setAppProperty(
        "EventLoop",
        "AthRemoteExec::RemoteExecHiveEventLoopMgr/RemoteExecHiveEventLoopMgr",
        overwrite=True,
    )

    cfg.getEventAlgo("SGInputLoader").ExtraOutputs.add(
        (
            "AthRemoteExec::RequestDescriptor",
            f"StoreGateSvc+{REMOTE_EXEC_REQUEST_KEY}",
        )
    )

    remote = CodecsTopCfg(flags)
    cfg.merge(remote.build(flags))

    for key, value in remote.properties.items():
        kwargs.setdefault(key, value)
    kwargs.setdefault("SchedulerSvc", "AvalancheSchedulerSvc")
    cfg.addService(
        CompFactory.AthRemoteExec.RemoteExecHiveEventLoopMgr(
            "RemoteExecHiveEventLoopMgr", **kwargs
        )
    )

    # The server reads no bytestream input of its own, but the codec installs
    # the events it decodes into the provider.
    cfg.addService(CompFactory.ROBDataProviderSvc())

    return cfg


def ByteStreamClientCfg(flags, target, **kwargs) -> ComponentAccumulator:
    """Configure the bytestream round-trip client job"""

    cfg = MainServicesCfg(flags)
    cfg.merge(ByteStreamReadCfg(flags))

    # Take the remote server configuration, declare that it's viewed by the
    # client and ask for the remote sequence
    fragment = CodecsTopCfg(flags).onClient().fragment("RemoteExecSeqByteStream")
    cfg.addEventAlgo(
        RequestAlg("RemoteExecByteStreamRequest", fragment, target=target, **kwargs)
    )

    # Algorithm that compares the digests of the original and the returned event
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.test_RawEventVerifyAlg(
            "RemoteExecByteStreamVerify", Size="bsSize", Hash="bsHash"
        )
    )

    return cfg


def ByteStreamSelectionClientCfg(flags, target, **kwargs) -> ComponentAccumulator:
    """Configure the bytestream selection client job: only the ROBs of the kept
    sub-detector groups travel to the server"""

    cfg = MainServicesCfg(flags)
    cfg.merge(ByteStreamReadCfg(flags))

    # Take the remote server configuration, declare that it's viewed by the
    # client and ask for the remote sequence
    fragment = (
        CodecsTopCfg(flags).onClient().fragment("RemoteExecSeqByteStreamSelection")
    )
    cfg.addEventAlgo(
        RequestAlg(
            "RemoteExecByteStreamSelectionRequest", fragment, target=target, **kwargs
        )
    )

    # Algorithm that compares the digests of the filtered original and the returned event
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.test_RawEventVerifyAlg(
            "RemoteExecByteStreamSelectionVerify",
            Size="bsSize",
            Hash="bsHash",
            KeepDetectors=SELECTION_GROUPS,
        )
    )

    return cfg
