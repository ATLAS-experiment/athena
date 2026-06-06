# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Serializable configuration objects for CA-based Powheg generation."""

from dataclasses import asdict, dataclass, field
import json


@dataclass(frozen=True)
class PowhegWeight:
    """One named variation in a Powheg weight group."""

    name: str
    values: tuple


@dataclass(frozen=True)
class PowhegWeightGroup:
    """Declarative description of a Powheg event-weight group."""

    name: str
    parameters: tuple
    combination_method: str = "none"
    weights: tuple = field(default_factory=tuple)


@dataclass(frozen=True)
class PowhegGenerationOptions:
    """Options controlling run-card creation and event generation."""

    create_run_card_only: bool = False
    save_integration_grids: bool = True
    use_external_run_card: bool = False
    remove_old_style_rwt_comments: bool = False
    is_bb4l_semilep: bool = False


@dataclass(frozen=True)
class PowhegRunConfig:
    """Complete input needed to execute one PowhegControl run."""

    process: str
    beam_energy: float
    max_events: int
    random_seed: int
    n_cores: int
    shower: bool
    output_lhe: str
    output_tarball: str = ""
    settings: dict = field(default_factory=dict)
    weight_groups: tuple = field(default_factory=tuple)
    parameter_stages: dict = field(default_factory=dict)
    generation_options: PowhegGenerationOptions = field(
        default_factory=PowhegGenerationOptions
    )
    schema_version: int = 1

    def to_json(self):
        """Return deterministic JSON suitable for storage in a CA property."""
        return json.dumps(asdict(self), sort_keys=True, separators=(",", ":"))

    @classmethod
    def from_json(cls, payload):
        """Reconstruct a run configuration from its JSON representation."""
        data = json.loads(payload)
        if data.get("schema_version") != 1:
            raise ValueError(
                f"Unsupported Powheg run-config schema "
                f"{data.get('schema_version')}"
            )

        groups = []
        for group in data.get("weight_groups", ()):
            weights = tuple(
                PowhegWeight(
                    name=weight["name"],
                    values=tuple(weight.get("values", ())),
                )
                for weight in group.get("weights", ())
            )
            groups.append(
                PowhegWeightGroup(
                    name=group["name"],
                    parameters=tuple(group.get("parameters", ())),
                    combination_method=group.get(
                        "combination_method", "none"
                    ),
                    weights=weights,
                )
            )

        return cls(
            process=data["process"],
            beam_energy=float(data["beam_energy"]),
            max_events=int(data["max_events"]),
            random_seed=int(data["random_seed"]),
            n_cores=int(data["n_cores"]),
            shower=bool(data["shower"]),
            output_lhe=data["output_lhe"],
            output_tarball=data.get("output_tarball", ""),
            settings=dict(data.get("settings", {})),
            weight_groups=tuple(groups),
            parameter_stages=dict(data.get("parameter_stages", {})),
            generation_options=PowhegGenerationOptions(
                **data.get("generation_options", {})
            ),
            schema_version=int(data["schema_version"]),
        )
