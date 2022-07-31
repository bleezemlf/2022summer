import base64
import random
from gmssl import sm2
from gmssl.sm4 import CryptSM4, SM4_ENCRYPT, SM4_DECRYPT

iv = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'


def PGP_enc(m, k):
    if isinstance(m,str):
        m  = m.encode()
    if isinstance(k,str):
        k  = k.encode()
    print("original message：", base64.b16encode(m))
    print("original key：", base64.b16encode(k))

    sm4 = CryptSM4()
    sm4.set_key(k, SM4_ENCRYPT)
    c1 = sm4.crypt_cbc(iv, m)
    print("encrypted message：", base64.b16encode(c1))

    c2 = sm2_crypt.encrypt(k)
    print("encrypted key ", base64.b16encode(c2))
    return c1, c2


def pgp_dec(c1, c2):
    k = sm2_crypt.decrypt(c2)
    sm4 = CryptSM4()
    sm4.set_key(k, SM4_DECRYPT)
    m = sm4.crypt_cbc(iv, c1)

    print("decrypted key", base64.b16encode(k))
    print("decrtpted message", base64.b16encode(m))


if __name__ == '__main__':
    p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
    a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
    b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
    n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
    x = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
    y = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
    g = [x, y]
    private_key = '00B9AB0B828FF68872F21A837FC303668428DEA11DCD1B24429D0C99E24EED83D5'
    public_key = 'B9C9A6E04E9C91F7BA880429273747D7EF5DDEB0BB2FF6317EB00BEF331A83081A6994B8993F3F5D6EADDDB81872266C87C018FB4162F5AF347B483E24620207'


    sm2_crypt = sm2.CryptSM2(public_key=public_key, private_key=private_key)

    m = "sm2_PGP"

    k = hex(random.randint(2 ** 127, 2 ** 128))[2:]

    r1, r2 = PGP_enc(m, k)
    pgp_dec(r1, r2)
