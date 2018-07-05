#!/bin/bash

p11keygen -k rsa -b 2048 -i rsa-1 CKA_SIGN=true CKA_VERIFY=true
p11keygen -k rsa -b 4096 -i rsa-2 CKA_SIGN=true CKA_VERIFY=true
p11keygen -k des -b 128  -i des-1 CKA_ENCRYPT=true
p11keygen -k des -b 192  -i des-2 CKA_ENCRYPT=true
p11keygen -k aes -b 128  -i aes-1 CKA_ENCRYPT=true
p11keygen -k aes -b 256  -i aes-2 CKA_ENCRYPT=true

