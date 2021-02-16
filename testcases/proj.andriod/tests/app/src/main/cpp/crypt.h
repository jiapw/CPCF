#pragma once


#define AES_MAXNR 14
#define AES_BLOCK_SIZE 16

struct aes_key_st {
    unsigned int rd_key[4 * (AES_MAXNR + 1)];
    unsigned int rounds;
};
typedef struct aes_key_st AES_KEY;


#define SHA_LONG unsigned int
#define SHA_LBLOCK      16
#define SHA256_DIGEST_LENGTH    32
#define SHA224_DIGEST_LENGTH    28

typedef struct SHA256state_st {
    SHA_LONG h[8];
    SHA_LONG Nl, Nh;
    SHA_LONG data[SHA_LBLOCK];
    unsigned int num, md_len;
} SHA256_CTX;


# define SHA_LBLOCK      16
# define SHA_CBLOCK      (SHA_LBLOCK*4)/* SHA treats input data as a
                                        * contiguous array of 32 bit wide
                                        * big-endian values. */

#define HASH_LONG               SHA_LONG
#define HASH_FINAL              SHA256_Final
#define HASH_CBLOCK             SHA_CBLOCK
#define HASH_CTX                SHA256_CTX

#define HASH_MAKE_STRING(c,s)   do {    \
        unsigned long ll;               \
        unsigned int  nn;               \
        switch ((c)->md_len)            \
        {   case SHA224_DIGEST_LENGTH:  \
                for (nn=0;nn<SHA224_DIGEST_LENGTH/4;nn++)       \
                {   ll=(c)->h[nn]; (void)HOST_l2c(ll,(s));   }  \
                break;                  \
            case SHA256_DIGEST_LENGTH:  \
                for (nn=0;nn<SHA256_DIGEST_LENGTH/4;nn++)       \
                {   ll=(c)->h[nn]; (void)HOST_l2c(ll,(s));   }  \
                break;                  \
            default:                    \
                if ((c)->md_len > SHA256_DIGEST_LENGTH) \
                    return 0;                           \
                for (nn=0;nn<(c)->md_len/4;nn++)                \
                {   ll=(c)->h[nn]; (void)HOST_l2c(ll,(s));   }  \
                break;                  \
        }                               \
        } while (0)


#define HASH_UPDATE             SHA256_Update
#define HASH_TRANSFORM          SHA256_Transform
#define HASH_FINAL              SHA256_Final
#define HASH_BLOCK_DATA_ORDER   sha256_block_data_order

//#define DATA_ORDER_IS_LITTLE_ENDIAN 1
#define DATA_ORDER_IS_BIG_ENDIAN 1

extern "C" {

int aes_v8_set_encrypt_key(const unsigned char *userKey, const int bits,
                           AES_KEY *key);

int aes_v8_set_decrypt_key(const unsigned char *userKey, const int bits,
                           AES_KEY *key);

void aes_v8_encrypt(const unsigned char *in, unsigned char *out,
                    const AES_KEY *key);

void aes_v8_decrypt(const unsigned char *in, unsigned char *out,
                    const AES_KEY *key);

void sha256_block_data_order(SHA256_CTX *ctx, const void *in, size_t num);
}
