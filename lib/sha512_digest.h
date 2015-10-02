#ifndef SHA512_DIGEST
#define SHA512_DIGEST
#include <cstdint>
#include <string>

class sha512_digest {
    unsigned long long int digest[8];
public:
    unsigned int digest_size_long {  8};
    unsigned int digest_size_char { 64};
    unsigned int digest_size_bit  {512};
    sha512_digest();
    sha512_digest(unsigned long long int*);
    sha512_digest(unsigned char*);
    operator char*                   () {return reinterpret_cast<char*                  > (digest); };
    operator unsigned char*          () {return reinterpret_cast<unsigned char*         > (digest); };
    operator unsigned long long int* () {return reinterpret_cast<unsigned long long int*> (digest); };
    operator uint64_t*               () {return reinterpret_cast<uint64_t*              > (digest); };
    bool operator==(sha512_digest &rhs);
    bool operator!=(sha512_digest &rhs);
    std::string to_hex_string();
};

#endif /*SHA512_DIGEST*/