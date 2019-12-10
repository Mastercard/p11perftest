#!/bin/bash

p11keygen -k rsa -b 2048 -i rsa-2048 CKA_SIGN=true CKA_VERIFY=true
p11keygen -k rsa -b 4096 -i rsa-4096 CKA_SIGN=true CKA_VERIFY=true
p11keygen -k ec -q prime256v1 -i ecdsa-secp256r1 CKA_SIGN=true CKA_VERIFY=true
p11keygen -k ec -q secp384r1 -i ecdsa-secp384r1 CKA_SIGN=true CKA_VERIFY=true
p11keygen -k ec -q secp521r1 -i ecdsa-secp521r1 CKA_SIGN=true CKA_VERIFY=true
p11keygen -k des -b 128  -i des-128 CKA_ENCRYPT=true
p11keygen -k des -b 192  -i des-192 CKA_ENCRYPT=true
p11keygen -k aes -b 128  -i aes-128 CKA_ENCRYPT=true
p11keygen -k aes -b 256  -i aes-256 CKA_ENCRYPT=true

