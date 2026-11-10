#!/usr/bin/env python3

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

import pkcs11
import os
import sys
import argparse
from pkcs11 import KeyType, Attribute, MechanismFlag, Mechanism, MLDSAParameterSet
from pkcs11.util.ec import encode_named_curve_parameters


# ML-DSA parameter sets, indexed by the suffix of the key label.
MLDSA_PARAMETER_SETS = {
    '44': MLDSAParameterSet.ML_DSA_44,
    '65': MLDSAParameterSet.ML_DSA_65,
    '87': MLDSAParameterSet.ML_DSA_87,
}


def generate_key(session, keytype, size, label, *args, **kwargs):
    try:
        print(f"Generating {label} key")
        key = session.generate_key( keytype,
                                    size,
                                    label=label,
                                    *args,
                                    **kwargs)

    except Exception as e:
        print(f"Ouch caught an exception: {e}")
        print(f"key with label {label} has NOT been generated")


def generate_keypair(session, keytype, sizeorcurve, label, *args, **kwargs):
    try:
        if keytype == KeyType.RSA:
            print(f"Generating {label} RSA key pair")
            key = session.generate_keypair( keytype,
                                            sizeorcurve,
                                            label=label,
                                            *args,
                                            **kwargs)
        elif keytype == KeyType.EC:
            print(f"Generating {label} EC key pair")
            param = session.create_domain_parameters(KeyType.EC, {
                Attribute.EC_PARAMS: encode_named_curve_parameters(sizeorcurve)
            }, local=True)

            key = param.generate_keypair ( label=label,
                                           *args,
                                           **kwargs)

        elif keytype == KeyType.ML_DSA:
            print(f"Generating {label} ML-DSA key pair")
            # the parameter set is carried by the public key template, and the
            # token derives the private key one from it
            key = session.generate_keypair( keytype,
                                            label=label,
                                            public_template={
                                                Attribute.PARAMETER_SET: MLDSA_PARAMETER_SETS[sizeorcurve]
                                            },
                                            *args,
                                            **kwargs)

        else:
            raise ValueError(f"Unsupported keytype: {keytype}")

    except Exception as e:
        print(f"Ouch caught an exception: {e}")
        print(f"key with label {label} has NOT been generated")




def generate_p11perftest_keys(libname, slot, password, store):

    lib = pkcs11.lib(libname)
    token = lib.get_slots()[slot].get_token()
    print('-'*80)
    print(f"""
{lib}
Slot Index: {slot}
{token.slot}
Token: {token}""")
    print('-'*80)


    with token.open(user_pin=password, rw=True) as session:

        keystogenerate = [
            [ generate_key, KeyType.DES2, 128, 'des-128', { 'capabilities': MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT } ],
            [ generate_key, KeyType.DES3, 192, 'des-192', { 'capabilities': MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT } ],
            [ generate_key, KeyType.AES, 128, 'aes-128', { 'capabilities': MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT } ],
            [ generate_key, KeyType.AES, 192, 'aes-192', { 'capabilities': MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT } ],
            [ generate_key, KeyType.AES, 256, 'aes-256', { 'capabilities': MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT } ],

            [ generate_keypair, KeyType.RSA, 2048, 'rsa-2048', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY | MechanismFlag.WRAP | MechanismFlag.UNWRAP | MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT } ],
            [ generate_keypair, KeyType.RSA, 3072, 'rsa-3072', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY | MechanismFlag.WRAP | MechanismFlag.UNWRAP | MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT} ],
            [ generate_keypair, KeyType.RSA, 4096, 'rsa-4096', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY | MechanismFlag.WRAP | MechanismFlag.UNWRAP | MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT } ],

            [ generate_keypair, KeyType.EC, 'secp256r1', 'ecdsa-secp256r1', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY } ],
            [ generate_keypair, KeyType.EC, 'secp384r1', 'ecdsa-secp384r1', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY } ],
            [ generate_keypair, KeyType.EC, 'secp521r1', 'ecdsa-secp521r1', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY } ],

            [ generate_keypair, KeyType.EC, 'secp256r1', 'ecdh-secp256r1', { 'capabilities': MechanismFlag.DERIVE } ],
            [ generate_keypair, KeyType.EC, 'secp384r1', 'ecdh-secp384r1', { 'capabilities': MechanismFlag.DERIVE } ],
            [ generate_keypair, KeyType.EC, 'secp521r1', 'ecdh-secp521r1', { 'capabilities': MechanismFlag.DERIVE } ],

            [ generate_key, KeyType.GENERIC_SECRET, 160, 'hmac-sha1', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY,
                                                                        'mechanism': Mechanism.GENERIC_SECRET_KEY_GEN } ],
            [ generate_key, KeyType.GENERIC_SECRET, 256, 'hmac-sha256', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY,
                                                                          'mechanism': Mechanism.GENERIC_SECRET_KEY_GEN } ],
            [ generate_key, KeyType.GENERIC_SECRET, 512, 'hmac-sha512', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY,
                                                                          'mechanism': Mechanism.GENERIC_SECRET_KEY_GEN } ],
            [ generate_key, KeyType.GENERIC_SECRET, 128, 'xorder-128', { 'capabilities': MechanismFlag.DERIVE,
                                                                         'mechanism': Mechanism.GENERIC_SECRET_KEY_GEN } ],
            [ generate_key, KeyType.AES, 128, 'rand-128', { } ],

            [ generate_keypair, KeyType.ML_DSA, '44', 'mldsa-44', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY } ],
            [ generate_keypair, KeyType.ML_DSA, '65', 'mldsa-65', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY } ],
            [ generate_keypair, KeyType.ML_DSA, '87', 'mldsa-87', { 'capabilities': MechanismFlag.SIGN | MechanismFlag.VERIFY } ],

        ]


        for item in keystogenerate:
            generator, keytype, sizeorcurve, label, kwargs = item
            # specify if the key is a token key or a session key

            kwargs['store'] = store
            generator(session, keytype, sizeorcurve, label, **kwargs)

        print()
        print("Note: ML-KEM and SLH-DSA key pairs are not generated, as python-pkcs11")
        print("      exposes neither CKK_ML_KEM nor CKK_SLH_DSA. Use createkeys.sh")
        print("      (pkcs11-tools v3.0.0 or later) to obtain them, or let p11perftest")
        print("      generate its own session keys by omitting --nogenerate.")



if __name__ == '__main__':


    parser = argparse.ArgumentParser(description='Generate key material needed for p11perftest')
    parser.add_argument('-l', '--library', metavar='PKCS11LIB', help='Path to PKCS#11 library', required=True)
    parser.add_argument('-s', '--slotindex', type=int, metavar='SLOTINDEX', help='Slot index', required=True)
    parser.add_argument('-p', '--pin', '--password', metavar='PASSWORD', help='Token PIN or password', required=True)
    parser.add_argument('-n', '--nostore', action='store_true', help='Dry run. Perform key generation, but do not write keys on token')
    args = parser.parse_args()

    generate_p11perftest_keys(args.library, args.slotindex, args.pin, not args.nostore)
