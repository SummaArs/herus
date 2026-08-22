from symbol_registry_model import (
    FactoryLexicon,
    Handle,
    Namespace,
    PersonalInterner,
    RegistryStatus,
    symbol_hash16,
)


class Score:
    def __init__(self):
        self.pass_count = 0
        self.fail_count = 0

    def check(self, condition, label):
        print(f"  {'PASS' if condition else 'FAIL'}  {label}")
        if condition:
            self.pass_count += 1
        else:
            self.fail_count += 1


def main():
    score = Score()
    score.check(symbol_hash16("gh") == symbol_hash16("ne"),
                "known folded-hash collision remains reproducible")

    factory = FactoryLexicon(3, ["possui", "caderno", "Gustavo"])
    first = factory.resolve("GUSTAVO")
    score.check(first.status is RegistryStatus.OK and first.handle is not None,
                "factory membership resolves ASCII case variants")
    score.check(factory.resolve("desconhecido").status is RegistryStatus.UNKNOWN,
                "factory unknown key is not accepted by slot coincidence")
    score.check(factory.resolve("GUSTAVO").handle == first.handle,
                "factory handle is stable for the same version")
    score.check(factory.export_manifest() == {
        "namespace": int(Namespace.FACTORY), "version": 3, "count": 3
    }, "factory audit manifest contains metadata only")

    personal = PersonalInterner(7, 2)
    unauthorized = personal.resolve("gh")
    score.check(unauthorized.status is RegistryStatus.AUTH and
                personal.private_count() == 0,
                "new personal identity requires explicit authority")
    gh = personal.resolve("gh", authorized=True)
    ne = personal.resolve("ne", authorized=True)
    score.check(gh.status is RegistryStatus.OK and ne.status is RegistryStatus.OK and
                gh.handle != ne.handle,
                "exact interning separates colliding hash inputs")
    score.check(personal.resolve("GH").handle == gh.handle,
                "personal interning reuses canonical case-insensitive identity")
    full = personal.resolve("third", authorized=True)
    score.check(full.status is RegistryStatus.FULL and personal.private_count() == 2,
                "personal capacity fails closed without silent reuse")
    score.check(personal.accept_handle(Handle(Namespace.PERSONAL, 8, 1))
                is RegistryStatus.VERSION_MISMATCH,
                "unknown personal version is rejected")

    migrated, mapping = personal.migrate(8)
    score.check(len(mapping) == 2 and migrated.private_count() == 2,
                "explicit migration preserves all authorized identities")
    score.check(migrated.accept_handle(Handle(Namespace.PERSONAL, 7, 1))
                is RegistryStatus.VERSION_MISMATCH,
                "old handle cannot be silently reused after migration")
    score.check(all(isinstance(value, int) for value in personal.export_records()) and
                all("gh" not in str(value) and "ne" not in str(value)
                    for value in personal.export_records()),
                "export boundary contains handles, never raw private text")

    print(f"SYMBOL REGISTRY MODEL: {score.pass_count} pass, {score.fail_count} fail")
    return 1 if score.fail_count else 0


if __name__ == "__main__":
    raise SystemExit(main())
