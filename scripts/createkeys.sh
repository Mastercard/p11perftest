#!/usr/bin/env bash

#
# Copyright (c) 2021 Mastercard
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

p11keygen -k des -b 128  -i des-128 encrypt
p11keygen -k des -b 192  -i des-192 encrypt
p11keygen -k aes -b 128  -i aes-128 encrypt
p11keygen -k aes -b 128  -i aes-192 encrypt
p11keygen -k aes -b 256  -i aes-256 encrypt
p11keygen -k rsa -b 2048 -i rsa-2048 sign verify wrap unwrap encrypt decrypt
p11keygen -k rsa -b 3072 -i rsa-3072 sign verify wrap unwrap encrypt decrypt
p11keygen -k rsa -b 4096 -i rsa-4096 sign verify wrap unwrap encrypt decrypt
p11keygen -k ec -q prime256v1 -i ecdsa-secp256r1 sign verify
p11keygen -k ec -q secp384r1 -i ecdsa-secp384r1 sign verify
p11keygen -k ec -q secp521r1 -i ecdsa-secp521r1 sign verify
p11keygen -k ec -q prime256v1 -i ecdh-secp256r1 derive
p11keygen -k ec -q secp384r1 -i ecdh-secp384r1 derive
p11keygen -k ec -q secp521r1 -i ecdh-secp521r1 derive
p11keygen -k generic -b 160 -i hmac-sha1 sign verify
p11keygen -k generic -b 256 -i hmac-sha256 sign verify
p11keygen -k generic -b 512 -i hmac-sha512 sign verify
p11keygen -k generic -b 128 -i xorder-128 derive
p11keygen -k aes -b 128 -i rand-128
