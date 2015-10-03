#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "sha512.h"
/*
g++ -Wall -Wextra -std=gnu++11 -c sha512-64.S -O3
g++ -Wall -Wextra -std=gnu++11    sha512-64.S -c sha512.cpp -O3

g++ -Wall -Wextra -std=gnu++11 -c lib/sha512-64.S -O3
g++ -Wall -Wextra -std=gnu++11    lib/sha512-64.S -c lib/sha512.cpp -O3
*/
// remember to include the assembly sha512 code
extern "C" void sha512_compress(uint64_t state[8], const uint8_t block[128]);

void sha512_hash(const uint8_t *message, uint32_t len, uint64_t hash[8]) {
	hash[0] = UINT64_C(0x6A09E667F3BCC908);
	hash[1] = UINT64_C(0xBB67AE8584CAA73B);
	hash[2] = UINT64_C(0x3C6EF372FE94F82B);
	hash[3] = UINT64_C(0xA54FF53A5F1D36F1);
	hash[4] = UINT64_C(0x510E527FADE682D1);
	hash[5] = UINT64_C(0x9B05688C2B3E6C1F);
	hash[6] = UINT64_C(0x1F83D9ABFB41BD6B);
	hash[7] = UINT64_C(0x5BE0CD19137E2179);
	
	uint32_t i;
	for (i = 0; len - i >= 128; i += 128)
		sha512_compress(hash, message + i);
	
	uint8_t block[128];
	uint32_t rem = len - i;
	memcpy(block, message + i, rem);
	
	block[rem] = 0x80;
	rem++;
	if (128 - rem >= 16)
		memset(block + rem, 0, 120 - rem);
	else {
		memset(block + rem, 0, 128 - rem);
		sha512_compress(hash, block);
		memset(block, 0, 120);
	}
	
	uint64_t longLen = ((uint64_t)len) << 3;
	for (i = 0; i < 8; i++)
		block[128 - 1 - i] = (uint8_t)(longLen >> (i * 8));
	sha512_compress(hash, block);
}
