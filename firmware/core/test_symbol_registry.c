#include "symbol_registry.h"

#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

int main(void)
{
    static const char *factory_keys[] = { "possui", "caderno", "Gustavo" };
    srreg_factory_t factory = { 3u, factory_keys, 3u };
    srreg_personal_t personal;
    srreg_personal_t migrated;
    srreg_handle_t gustavo = 0u;
    srreg_handle_t gh = 0u;
    srreg_handle_t ne = 0u;
    srreg_handle_t old_export[2] = { 0u, 0u };
    uint16_t legacy_factory = 0u;
    uint16_t legacy_personal = 0u;
    score_t score = { 0, 0 };
    int result;

    check(&score, srreg_hash16("gh", 2u) == srreg_hash16("ne", 2u),
          "known folded-hash collision remains reproducible");
    result = srreg_factory_resolve(&factory, "GUSTAVO", 7u, &gustavo);
    check(&score, result == SRREG_OK &&
                    srreg_handle_namespace(gustavo) == SRREG_NAMESPACE_FACTORY &&
                    srreg_handle_version(gustavo) == 3u,
          "factory membership resolves into a versioned handle");
    check(&score, srreg_factory_resolve(&factory, "unknown", 7u, &gustavo) ==
                    SRREG_UNKNOWN,
          "factory unknown key is rejected by exact membership");
    check(&score, srreg_project_legacy(gustavo, 3u, &legacy_factory) == SRREG_OK &&
                    (legacy_factory & 0x8000u) == 0u,
          "factory handle projects to the reserved legacy namespace");

    check(&score, srreg_personal_init(&personal, 7u, 2u) == SRREG_OK,
          "personal registry initializes with bounded capacity");
    check(&score, srreg_personal_resolve(&personal, "gh", 2u, 0u, &gh) ==
                    SRREG_AUTH && srreg_personal_count(&personal) == 0u,
          "personal identity requires explicit confirmation");
    check(&score, srreg_personal_resolve(&personal, "gh", 2u, 1u, &gh) ==
                    SRREG_OK && srreg_personal_resolve(&personal, "ne", 2u, 1u,
                                                         &ne) == SRREG_OK &&
                    gh != ne,
          "exact interning separates colliding hash inputs");
    check(&score, srreg_personal_resolve(&personal, "GH", 2u, 0u, &gustavo) ==
                    SRREG_OK && gustavo == gh,
          "case variant reuses the same personal handle");
    check(&score, srreg_project_legacy(gh, 7u, &legacy_personal) == SRREG_OK &&
                    (legacy_personal & 0x8000u) != 0u &&
                    legacy_personal != legacy_factory,
          "personal handle projects without colliding with factory IDs");
    check(&score, srreg_project_legacy(gh, 8u, &legacy_personal) ==
                    SRREG_VERSION_MISMATCH,
          "legacy projection rejects an inactive registry version");
    check(&score, srreg_personal_resolve(&personal, "third", 5u, 1u, &gustavo) ==
                    SRREG_FULL && srreg_personal_count(&personal) == 2u,
          "personal overflow fails closed without silent reuse");
    check(&score, srreg_personal_accept(&personal,
                                        srreg_handle_make(SRREG_NAMESPACE_PERSONAL,
                                                          8u, 1u)) ==
                    SRREG_VERSION_MISMATCH,
          "unknown personal version is rejected");
    check(&score, srreg_personal_export(&personal, old_export, 2u) == 2u &&
                    old_export[0] != 0u && old_export[1] != 0u,
          "personal export contains numeric handles only");
    result = srreg_personal_migrate(&personal, 8u, &migrated);
    check(&score, result == SRREG_OK && srreg_personal_count(&migrated) == 2u,
          "explicit migration preserves authorized identities");
    check(&score, srreg_personal_accept(&migrated,
                                        srreg_handle_make(SRREG_NAMESPACE_PERSONAL,
                                                          7u, 1u)) ==
                    SRREG_VERSION_MISMATCH,
          "old version cannot be silently reused after migration");

    printf("SYMBOL REGISTRY C: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
