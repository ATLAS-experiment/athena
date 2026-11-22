# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Everything thats needed to configure Athena for remote execution"""

from AthenaCommon.CFElements import parAND, parOR, seqAND
from AthenaCommon.Configurable import Configurable
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthRemoteExecCore.RemoteExecCodecs import Codec, Direction, Source

log = logging.getLogger("RemoteExecFragments")

_NON_EVENT_STORES = frozenset(("ConditionStore+", "DetectorStore+"))


def handlePath(dataHandle) -> str:
    """Get key full path in StoreGate"""

    return str(getattr(dataHandle, "Path", dataHandle))


def splitStore(path: str) -> tuple[str, str]:
    """'StoreGateSvc+a' -> ('StoreGateSvc+', 'a'); 'a' -> ('', 'a')."""

    store, plus, key = path.rpartition("+")
    return store + plus, key


def isConditions(dataHandle, store: str) -> bool:
    """Check if key in the store is conditions container"""

    return store in _NON_EVENT_STORES or getattr(dataHandle, "IsCondition", False)


def deriveBoundaries(
    remoteCfg: ComponentAccumulator,
    outputs: list[str],
    request_key: str,
    boundaries: dict[str, Codec],
    extra_inputs: list[str],
    ignore_inputs: list[str],
    produces: list[str],
):
    """Work out which keys cross a fragment's boundary, and their types.

    :param remoteCfg: the ComponentAccumulator, configured exactly as it would be for a normal job
    :param outputs: the keys to send back to the client
    :param request_key: RequestDescriptor key
    :param boundaries: ``{key: Codec}`` pairs for every key that its transfered over
    :param extra_inputs: extra inputs treated as a client inputs - for a remote sequence whose consumer is added later
    :param ignore_inputs: extra inputs that can be ignored
    :param produces: extra keys the remoteCfg writes via WriteHandles

    Raises ValueError if a declared output is not produced, if two consumers of
    one key disagree about its type, or if an input's type cannot be determined.
    """

    reads = {}  # {remoteCfg input key: {type}}
    writes = set()  # {type}
    types = {}  # {remoteCfg output key: type}
    for handle in remoteCfg.getIO():
        store, key = splitStore(handlePath(handle["key"]))
        if not key or isConditions(handle["key"], store):
            continue
        if handle["mode"] == "W":
            writes.add(key)
            types[key] = handle["type"]
        else:
            reads.setdefault(key, set()).add(handle["type"])

    stale_writes = [k for k in produces if k in writes]
    if stale_writes:
        raise ValueError(f"Redeclaration of writes: {sorted(stale_writes)}")
    writes.update(produces)

    carried = {}
    # {boundary key: Codec} -> {remoteCfg key: boundary key}
    for boundary_key, codec in boundaries.items():
        for sg_key in codec.sgKeys(boundary_key):
            if sg_key != boundary_key:
                carried[sg_key] = boundary_key

    unmet = set(carried.get(k, k) for k in reads if k not in writes)
    stale = [k for k in ignore_inputs if k not in unmet]
    if stale:
        raise ValueError(f"Unmet inputs found: {sorted(stale)}")
    inputs = sorted(unmet - {request_key} - set(ignore_inputs))
    inputs += [k for k in extra_inputs if k not in inputs]

    for key in inputs:
        codec = boundaries.get(key)
        if codec is not None and codec.schema_source is not Source.derived:
            # This boundary states its own schema, or needs none, and the
            # scheduler learns the key from the codec.
            continue
        if key in types:
            continue
        candidates = reads.get(key, set())
        if len(candidates) != 1:
            raise ValueError(
                f"Cannot derive input type for '{key}': {sorted(candidates) if candidates else 'nothing reads it'}"
            )
        types[key] = next(iter(candidates))

    def _produced(key: str) -> bool:
        """Check if remoteCfg produces keys that Codec needs"""

        codec = boundaries.get(key)
        wanted = codec.sgKeys(key) if codec is not None else [key]
        return all(sg_key in writes for sg_key in wanted)

    missing = [key for key in outputs if not _produced(key)]
    if missing:
        raise ValueError(
            f"The remoteCfg does not produce keys that Codec expects: {missing}. It writes {sorted(writes)}"
        )

    return inputs, types


def boundarySpecs(declarations: list[tuple[str, str, Codec]]) -> list[str]:
    """List of complete BoundaryInfo: 'key#encoding#schema' from given declarations"""

    return [codec.spec(key, sg_type) for key, sg_type, codec in declarations]


def boundaryTools(prefix, declarations, direction: Direction) -> list[Configurable]:
    """Create codec tools from given declarations.

    :param prefix: prefix for tool name
    :param declarations: ``(key, sg_type, codec)`` definitions of what keys with which Codecs are used
    :param direction: direction which data in declarations pass
    """
    return [
        codec.tool(f"{prefix}_{key}", key, direction, sg_type)
        for key, sg_type, codec in declarations
    ]


class RemoteExecFragment:
    """The declaration of fragmen, which represents one remote sequence and its input/output codecs"""

    def __init__(
        self,
        name: str,
        inputs: list[tuple[str, str, Codec]],
        outputs: list[tuple[str, str, Codec]],
        reply_key: str | None = None,
    ):
        """
        :param name: the sequence name
        :param inputs: ``(key, sg_type, codec)`` defitions of what the client needs to supply
        :param outputs: ``(key, sg_type, codec)`` defitions of what is returned to the client
        :param reply_key: ReplyStaging key
        """

        self.name = name
        self.inputs = inputs
        self.outputs = outputs
        if outputs and reply_key is None:
            reply_key = "RemoteExecReply" + name.removeprefix("RemoteExecSeq")
        self.reply_key = reply_key

    @property
    def input_specs(self) -> list[str]:
        return boundarySpecs(self.inputs)

    @property
    def output_specs(self) -> list[str]:
        return boundarySpecs(self.outputs)

    def inputCnvAlg(self, request_key: str) -> Configurable:
        """Define input conversion algorithm and attach codec tools to it"""

        alg = CompFactory.AthRemoteExec.InputCnvAlg(
            f"{self.name}InputCnvAlg",
            SequenceName=self.name,
            Request=request_key,
        )

        alg.Decoders = boundaryTools(alg.getName(), self.inputs, Direction.WriteToSG)

        return alg

    def outputCnvAlg(self, request_key: str) -> Configurable | None:
        """Define output conversion algorithm (if sequence declares outputs) and attach codec tools to it"""

        if not self.outputs:
            return None

        alg = CompFactory.AthRemoteExec.OutputCnvAlg(
            f"{self.name}OutputCnvAlg",
            Request=request_key,
            Reply=self.reply_key,
        )

        alg.Encoders = boundaryTools(alg.getName(), self.outputs, Direction.ReadFromSG)

        return alg


def RequestAlg(
    name: str, fragment: RemoteExecFragment, target: str, **kwargs
) -> Configurable:
    """Configuration of client-side algorithm

    :param name: Algorithm name
    :param fragment: RemoteExecFragment obtanined from the server via .onClient().fragment("SequenceName")
    :param target: ``address:port`` of a server client should connect to
    """

    alg = CompFactory.AthRemoteExec.RequestAlg(
        name,
        SequenceName=fragment.name,
        Target=target,
        Decoders=boundaryTools(name, fragment.inputs, Direction.ReadFromSG),
        Encoders=boundaryTools(name, fragment.outputs, Direction.WriteToSG),
        **kwargs,
    )

    return alg


def RemoteExecFragmentCfg(
    flags,
    fragment: RemoteExecFragment,
    remoteCfg: ComponentAccumulator,
    parent,
    request_key: str,
) -> ComponentAccumulator:
    """Assemble remote sequence into ComponentAccumulator

    :param fragment: Entry containing sequence name, input and output algorithm definitions
    :param remoteCfg: the ComponentAccumulator, configured exactly as it would be for a normal job
    :param parent: parent sequence under which remote sequence will live
    :param request_key: RequestDescriptor key
    """

    cfg = ComponentAccumulator()
    cfg.addSequence(parent, parentName="AthAlgSeq")

    # Sequence everything here will live under
    cfg.addSequence(seqAND(fragment.name), parentName=parent.name)

    # Input algorithm always goes first
    cfg.addEventAlgo(fragment.inputCnvAlg(request_key), sequenceName=fragment.name)

    # The remoteCfg inside it's own parallel sequence
    cfg.addSequence(parAND(f"{fragment.name}Payload"), parentName=fragment.name)
    cfg.merge(remoteCfg, sequenceName=f"{fragment.name}Payload")

    # Output algorithm always goes last
    outputCnvAlg = fragment.outputCnvAlg(request_key)
    if outputCnvAlg is not None:
        cfg.addEventAlgo(outputCnvAlg, sequenceName=fragment.name)

    return cfg


#: Defaults for a job that serves one remote configuration, which is every job so far.
REMOTE_EXEC_TOP_SEQ_NAME = "RemoteExecTop"
REMOTE_EXEC_REQUEST_KEY = "RemoteExecRequest"


class RemoteExecTop:
    """Every remote sequence that server offers and their configuration."""

    def __init__(
        self,
        request_key: str = REMOTE_EXEC_REQUEST_KEY,
        top: str = REMOTE_EXEC_TOP_SEQ_NAME,
    ):
        self.request_key = request_key
        self.top = top
        self.fragments: list[RemoteExecFragment] = []
        self.remoteCfgs: dict[str, ComponentAccumulator] = {}
        self.isClient = False

    def add(
        self,
        name: str,
        remoteCfg: ComponentAccumulator,
        outputs: list[str] | None = None,
        boundaries: dict[str, Codec] | None = None,
        types: dict | None = None,
        inputs: list[str] | None = None,
        ignore_inputs: list[str] | None = None,
        produces: list[str] | None = None,
        reply_key: str | None = None,
    ) -> RemoteExecFragment:
        """Add new remote sequence, figure out dependencies and types, and validate inputs and outputs.

        :param name: the sequence name
        :param remoteCfg: the ComponentAccumulator, configured exactly as it would be for a normal job
        :param outputs: the keys to send back to the client
        :param boundaries: ``{key: Codec}`` pairs for every key that its transfered over
        :param types: ``{key: type name}`` pairs overrides for derived StoreGate types, needed when a remoteCfg reads a base class
        :param inputs: an assertion which keys will be used as an inputs
        :param ignore_inputs: extra inputs that can be ignored
        :param produces: extra keys the remoteCfg writes via WriteHandles
        """

        outputs = list(outputs or [])
        boundaries = dict(boundaries or {})
        types = dict(types or {})
        ignore_inputs = list(ignore_inputs or [])
        produces = list(produces or [])

        derived_keys, derived_types = deriveBoundaries(
            remoteCfg,
            outputs,
            self.request_key,
            boundaries=boundaries,
            extra_inputs=[k for k in list(inputs or []) if k in types],
            ignore_inputs=ignore_inputs,
            produces=produces,
        )
        derived_types.update(types)

        if inputs is not None and sorted(inputs) != sorted(derived_keys):
            raise ValueError(
                f"Fragment '{name}' declares inputs {sorted(inputs)} but its remoteCfg leaves {sorted(derived_keys)} unmet"
            )

        crossing = list(derived_keys) + outputs
        undeclared_outputs = [k for k in crossing if k not in boundaries]
        if undeclared_outputs:
            raise ValueError(
                f"Fragment '{name}' does not say how {sorted(undeclared_outputs)} are (de)serialised: give each a Codec in boundaries={{}}"
            )

        unused_outputs = [k for k in boundaries if k not in crossing]
        if unused_outputs:
            raise ValueError(
                f"Fragment '{name}' declares unused boundaries for {sorted(unused_outputs)}; it crosses {sorted(crossing)}"
            )

        def declare(keys):
            """For each given key return its type and Codec"""

            return [(key, derived_types.get(key, ""), boundaries[key]) for key in keys]

        fragment = RemoteExecFragment(
            name,
            inputs=declare(derived_keys),
            outputs=declare(list(outputs)),
            reply_key=reply_key,
        )

        log.info(
            "RemoteExec fragment %s: in %s, out %s",
            name,
            fragment.input_specs or "-",
            fragment.output_specs or "-",
        )

        self.fragments.append(fragment)
        self.remoteCfgs[name] = remoteCfg

        return fragment

    def onClient(self) -> "RemoteExecTop":
        """Declare that we are working on the client-side -> mark CAs as merged so it won't raise error for client"""

        self.isClient = True
        for _, remoteCfg in self.remoteCfgs.items():
            remoteCfg.wasMerged()

        return self

    def fragment(self, name: str) -> RemoteExecFragment:
        """Return fragment with a given name"""

        for fragment in self.fragments:
            if fragment.name == name:
                return fragment

        raise KeyError(
            f"This remote configuration offers no fragment '{name}'. It has: {[f.name for f in self.fragments]}"
        )

    def build(self, flags):
        """Build top sequence and add all declared remote sequences under it. Called on server-side."""

        if self.isClient:
            raise RuntimeError(
                "Attempting to build remote configuration on client side!"
            )

        cfg = ComponentAccumulator()
        cfg.addSequence(parOR(self.top), parentName="AthAlgSeq")
        for fragment in self.fragments:
            cfg.merge(
                RemoteExecFragmentCfg(
                    flags,
                    fragment,
                    self.remoteCfgs[fragment.name],
                    parOR(self.top),
                    self.request_key,
                )
            )
        return cfg

    @property
    def properties(self):
        """Return all properties required to configure RemoteExecHiveEventLoopMgr"""

        props = RemoteExecTopProperties(self.fragments)
        props["RequestKey"] = self.request_key

        return props


def RemoteExecTopProperties(fragments):
    """Return all properties required to configure RemoteExecHiveEventLoopMgr"""

    return {
        "Sequences": [f.name for f in fragments],
        "Decoders": {f.name: f.input_specs for f in fragments if f.inputs},
        "Encoders": {f.name: f.output_specs for f in fragments if f.outputs},
        "ReplyKeys": {f.name: f.reply_key for f in fragments if f.reply_key},
    }
