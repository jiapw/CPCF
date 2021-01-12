@echo off
set MODULES=auto_rng,system_rng,rsa,sha1_sse2,sha1,sha2_32,sha2_64,sha3,sm2,sm3,sm4,emsa1,emsa_pkcs1,emsa_pssr,emsa_raw,dh,ecdh,emsa_x931,dev_random,md5,crc32,cbc,des,blowfish,curve25519,ed25519,dsa,aes,aes_armv8,tls

configure.py --amalgamation --disable-shared --cc=clang --no-autoload --enable-modules=%MODULES% --cpu=x86_32 --os=darwin --with-local-config=config_mac.h
move /Y botan_all*.h ..\platforms\mac_x86\
move /Y botan_all*.cpp ..\platforms\mac_x86\

configure.py --amalgamation --disable-shared --cc=clang --no-autoload --enable-modules=%MODULES% --cpu=x86_64 --os=darwin --with-local-config=config_mac.h
move /Y botan_all*.h ..\platforms\mac_x64\
move /Y botan_all*.cpp ..\platforms\mac_x64\
