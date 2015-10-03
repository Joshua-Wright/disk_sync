#ifndef SHA512
#define SHA512

#include <cstdint>

// typedef uint64_t sha512_digest[8];

// refers to number of unsigned char values
const int SHA512_DIGEST_SIZE = 64;

void sha512_hash(const uint8_t *message, uint32_t len, uint64_t hash[8]);

#endif //SHA512