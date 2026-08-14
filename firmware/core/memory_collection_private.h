/* memory_collection_private.h — internal HERUS core seam for bounded index evaluation.
 *
 * Not an application/public-product header. The function is restricted by source
 * convention to memory_collection_index.c. It copies authenticated cards into a
 * caller-owned transient array only after canonical physical access, so matching
 * can remain local without adding a UI/API that enumerates collection contents.
 */
#ifndef HERUS_MEMORY_COLLECTION_PRIVATE_H
#define HERUS_MEMORY_COLLECTION_PRIVATE_H

#include <stdint.h>
#include "memory_collection.h"

int memory_collection_copy_cards_for_index(memory_collection_t *collection,
                                           const memory_collection_access_t *access,
                                           memory_vault_card_t out[MEMORY_COLLECTION_MAX_CARDS],
                                           uint8_t *out_count);

#endif /* HERUS_MEMORY_COLLECTION_PRIVATE_H */
