
# define AES_MAXNR 14
# define AES_BLOCK_SIZE 16


struct aes_key_st {
    uint32_t rd_key[4 * (AES_MAXNR + 1)];
    uint32_t rounds;
};
typedef struct aes_key_st AES_KEY;


extern "C" {


int aes_v8_set_encrypt_key(const unsigned char *userKey, const int bits,
                           AES_KEY *key);


int aes_v8_set_decrypt_key(const unsigned char *userKey, const int bits,
                           AES_KEY *key);

void aes_v8_encrypt(const unsigned char *in, unsigned char *out,
                    const AES_KEY *key);

void aes_v8_decrypt(const unsigned char *in, unsigned char *out,
                    const AES_KEY *key);

}