#include <string.h>
#include "crypt.h"

void OPENSSL_cleanse(void *ptr, size_t len)
{
    memset(ptr, 0, len);
}

#include "md32_common.h"

int SHA256_Init(SHA256_CTX *c)
{
    memset(c, 0, sizeof(*c));
    c->h[0] = 0x6a09e667UL;
    c->h[1] = 0xbb67ae85UL;
    c->h[2] = 0x3c6ef372UL;
    c->h[3] = 0xa54ff53aUL;
    c->h[4] = 0x510e527fUL;
    c->h[5] = 0x9b05688cUL;
    c->h[6] = 0x1f83d9abUL;
    c->h[7] = 0x5be0cd19UL;
    c->md_len = SHA256_DIGEST_LENGTH;
    return 1;
}


unsigned char *SHA256(const unsigned char *d, size_t n, unsigned char *md)
{
    SHA256_CTX c;
    static unsigned char m[SHA256_DIGEST_LENGTH];

    if (md == NULL)
        md = m;
    SHA256_Init(&c);
    SHA256_Update(&c, d, n);
    SHA256_Final(md, &c);
    OPENSSL_cleanse(&c, sizeof(c));
    return md;
}
