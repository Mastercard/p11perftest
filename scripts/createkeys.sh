#!/bin/bash

p11keygen -k rsa -b 2048 -i rsa-2048 CKA_SIGN=true CKA_VERIFY=true
p11keygen -k rsa -b 4096 -i rsa-4096 CKA_SIGN=true CKA_VERIFY=true
p11keygen -k des -b 128  -i des-128 CKA_ENCRYPT=true
p11keygen -k des -b 192  -i des-192 CKA_ENCRYPT=true
p11keygen -k aes -b 128  -i aes-128 CKA_ENCRYPT=true
p11keygen -k aes -b 256  -i aes-256 CKA_ENCRYPT=true

