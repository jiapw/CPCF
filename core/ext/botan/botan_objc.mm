
#include <objc/objc.h>

//////////////////////////////////////////////////////
// iOS Stuffs
#if defined(PLATFORM_IOS)
#import <Foundation/Foundation.h>
#import <CommonCrypto/CommonDigest.h>
#import <CommonCrypto/CommonCryptor.h>

// sha

void _objc_sha1(unsigned char *input, int len, unsigned char output[20]);
void _objc_sha1(unsigned char *input, int len, unsigned char output[20])
{
    CC_SHA1(input, len, output);
}

void _objc_sha256(unsigned char *input, int len, unsigned char output[32]);
void _objc_sha256(unsigned char *input, int len, unsigned char output[32])
{
    CC_SHA256(input, len, output);
}

void _objc_sha384(unsigned char *input, int len, unsigned char output[48]);
void _objc_sha384(unsigned char *input, int len, unsigned char output[48])
{
    CC_SHA384(input, len, output);
}

void _objc_sha512(unsigned char *input, int len, unsigned char output[64]);
void _objc_sha512(unsigned char *input, int len, unsigned char output[64])
{
    CC_SHA512(input, len, output);
}

// aes

enum BTEncryptMode{
     BTEncryptModeAES128 = 0,
     BTEncryptModeAES192,
     BTEncryptModeAES256,
     BTEncryptModeDES,
     BTEncryptMode3DES,
};

NSData* _objc_utf8Data(NSString *string) {
    return [string dataUsingEncoding:NSUTF8StringEncoding];
}
NSString* _objc_utf8String(NSData *data) {
    return [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
}

NSData* _objc_base64EncodeData(NSData *data) {
    return [data base64EncodedDataWithOptions:NSDataBase64Encoding64CharacterLineLength];
}
NSData* _objc_base64DecodeData(NSString *string) {
    return [[NSData alloc] initWithBase64EncodedString:string options:NSDataBase64DecodingIgnoreUnknownCharacters];
}

NSData* _objc_encrypt(CCOperation operation, enum BTEncryptMode mode, NSData *data, NSString *key, CCOptions options, NSString *iv) {
    NSUInteger keySize;
    CCAlgorithm algorithm;
    NSUInteger blockSize;
    switch (mode) {
        case BTEncryptModeAES128: {
            keySize = kCCKeySizeAES128;
            algorithm = kCCAlgorithmAES128;
            blockSize = kCCBlockSizeAES128;
            break;
        }
        case BTEncryptModeAES192: {
            keySize = kCCKeySizeAES192;
            algorithm = kCCAlgorithmAES128;
            blockSize = kCCBlockSizeAES128;
            break;
        }
        case BTEncryptModeAES256: {
            keySize = kCCKeySizeAES256;
            algorithm = kCCAlgorithmAES128;
            blockSize = kCCBlockSizeAES128;
            break;
        }
        case BTEncryptModeDES: {
            keySize = kCCKeySizeDES;
            algorithm = kCCAlgorithmDES;
            blockSize = kCCBlockSizeDES;
            break;
        }
        case BTEncryptMode3DES: {
            keySize = kCCKeySize3DES;
            algorithm = kCCAlgorithm3DES;
            blockSize = kCCBlockSize3DES;
            break;
        }
        default: {
            return nil;
        }
    }
    NSInteger keyLength = MAX(keySize, key.length);
    char keyPtr[keyLength + 1];
    bzero(keyPtr, sizeof(keyPtr));
    [key getCString:keyPtr maxLength:sizeof(keyPtr) encoding:NSUTF8StringEncoding];
    
    NSUInteger dataLength = data.length;
    size_t bufferSize = dataLength + blockSize;
    void * buffer = malloc(bufferSize);
    size_t numBytesDecrypted = 0;
    char ivPtr[blockSize+1];
    memset(ivPtr, 0, sizeof(ivPtr));
    if (iv != nil) {
        [iv getCString:ivPtr maxLength:sizeof(ivPtr) encoding:NSUTF8StringEncoding];
    }
    CCCryptorStatus cryptStatus = CCCrypt(operation,
                                          algorithm,
                                          options,
                                          keyPtr,
                                          keySize,
                                          ivPtr,
                                          data.bytes,
                                          dataLength,
                                          buffer,
                                          bufferSize,
                                          &numBytesDecrypted);
    if (cryptStatus == kCCSuccess) {
        NSData * result = [NSData dataWithBytesNoCopy:buffer length:numBytesDecrypted];
        if (result != nil) {
            return result;
        }
    } else {
        if (buffer) {
            free(buffer);
            buffer = NULL;
        }
    }
    return nil;
}

NSData* _objc_dataByEncrypt(NSData *data, NSString *key, enum BTEncryptMode mode, CCOptions options, NSString *iv) {
    NSData *encodeData = _objc_encrypt(kCCEncrypt, mode, data, key, options, iv);
    return encodeData;
}

NSData* _objc_dataByDecrypt(NSData *data, NSString *key, enum BTEncryptMode mode, CCOptions options, NSString *iv) {
    NSData *decryptData = _objc_encrypt(kCCDecrypt, mode, data, key, options, iv);
    return decryptData;
}

int _objc_AES128Encrypt(const char *input, const char *key, char output[24]);
int _objc_AES128Encrypt(const char *input, const char *key, char output[24])
{
    NSString *inputString = [NSString stringWithUTF8String:input];
    NSString *keyString = [NSString stringWithUTF8String:key];
    NSData *inputData = _objc_utf8Data(inputString);
    
    NSData *result = _objc_dataByEncrypt(inputData, keyString, BTEncryptModeAES128, kCCOptionPKCS7Padding|kCCOptionECBMode, nil);

    if (result == nil) {
        return 1;
    }
    
    NSData *base64Encode = _objc_base64EncodeData(result);
    NSString *outputString = _objc_utf8String(base64Encode);
    const char *outputCString = [outputString UTF8String];
    strcpy(output, outputCString);
    
    return 0;
}

int _objc_AES128Decrypt(const char *input, const char *key, const char **output, unsigned long *outputLen);
int _objc_AES128Decrypt(const char *input, const char *key, const char **output, unsigned long *outputLen)
{
    NSString *inputString = [NSString stringWithUTF8String:input];
    NSString *keyString = [NSString stringWithUTF8String:key];
    NSData *inputData = _objc_base64DecodeData(inputString);

    NSData *result = _objc_dataByDecrypt(inputData, keyString, BTEncryptModeAES128, kCCOptionPKCS7Padding|kCCOptionECBMode, nil);

    if (result == nil) {
        return 1;
    }

    NSString *outputString = _objc_utf8String(result);
    const char *outputCString=[outputString UTF8String];
    *output = outputCString;
    *outputLen = strlen(outputCString);
    
    return 0;
}

#endif
