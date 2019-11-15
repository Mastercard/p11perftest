#!/usr/bin/env python

LIB = '/opt/safenet/lunaclient/lib/libCryptoki2_64.so'
SPYLIB= '/usr/lib64/pkcs11/pkcs11-spy.so'
SLOTINDEX = 2
PASSWORD = 'changeit'


import pkcs11
import os
import sys
import argparse
from pkcs11 import KeyType, Attribute, MechanismFlag, Mechanism


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


    with token.open(user_pin=args.password) as session:

        print("Generating des-1 key")
        des1 = session.generate_key( KeyType.DES2,
                                     128, 
                                     label='des-1', 
                                     store=store, 
                                     capabilities = MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT )

        print("Generating des-2 key")
        des2 = session.generate_key( KeyType.DES3,
                                     192, 
                                     label='des-2', 
                                     store=store, 
                                     capabilities = MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT )

        print("Generating aes-1 key")
        aes1 = session.generate_key( KeyType.AES,
                                     128, 
                                     label='aes-1', 
                                     store=store, 
                                     capabilities = MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT )

        print("Generating aes-2 key")
        aes2 = session.generate_key( KeyType.AES,
                                     128, 
                                     label='aes-2', 
                                     store=store, 
                                     capabilities = MechanismFlag.ENCRYPT | MechanismFlag.DECRYPT )
        

        print("Generating rsa-1 key")
        rsa1 = session.generate_keypair( KeyType.RSA,
                                         2048, 
                                         label='rsa-1', 
                                         store=store, 
                                         capabilities = MechanismFlag.SIGN | MechanismFlag.VERIFY )

        print("Generating rsa-2 key")
        rsa2 = session.generate_keypair( KeyType.RSA,
                                         4096, 
                                         label='rsa-2', 
                                         store=store, 
                                         capabilities = MechanismFlag.SIGN | MechanismFlag.VERIFY )



if __name__ == '__main__':


    parser = argparse.ArgumentParser(description='Test if PKCS#11 CT can support wrapping RSA private keys')
    parser.add_argument('library', metavar='PKCS11LIB', help='Path to PKCS#11 library', default=LIB, nargs='?')
    parser.add_argument('slotindex', type=int, metavar='SLOTINDEX', help='Slot index', default=SLOTINDEX, nargs='?')
    parser.add_argument('password', metavar='PASSWORD', help='Token password', default=PASSWORD, nargs='?')
    parser.add_argument('-d','--debug', action='store_true', help='Using pkcs11-spy, debug API calls')
    parser.add_argument('--pkcs11spy', metavar='FILE', help='Path to PKCS#11 pkcs11-spy library', default=SPYLIB)
    parser.add_argument('--pkcs11spyoutput', metavar='FILE', help='Path to file where to write pkcs11-spy logs')
    parser.add_argument('-n', '--noop', action='store_true', help='Perform operations, but do not write keys on token')
    args = parser.parse_args()

    # if we use pkcs11-spy, setup pkcs11-spy environment accordingly
    if args.debug==True:
        os.environ['PKCS11SPY'] = args.library
        if args.pkcs11spyoutput:
            os.environ['PKCS11SPY_OUTPUT'] = args.pkcs11spyoutput
            libname = args.pkcs11spy
    else:
        libname = args.library
    

    generate_p11perftest_keys(libname, args.slotindex, args.password, not args.noop)








