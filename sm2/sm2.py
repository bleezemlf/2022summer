# 算法实现参考https://blog.csdn.net/qq_33439662/article/details/122590298
# 参考国家密码管理局sm2标准 https://www.oscca.gov.cn/sca/xxgk/2010-12/17/1002386/files/b791a9f908bb4803875ab6aeeb7b4e03.pdf
# 依赖包gmssl,使用命令 pip install gmssl安装

from gmssl import sm3
from random import randint
import math


# 模逆运算
def invp(a, m):
    x1, x2, x3 = 1, 0, a
    y1, y2, y3 = 0, 1, m
    while y3 != 0:
        q = x3 // y3
        t1, t2, t3 = x1 - q * y1, x2 - q * y2, x3 - q * y3
        x1, x2, x3 = y1, y2, y3
        y1, y2, y3 = t1, t2, t3
    return x1 % m


# str or bytes -> str
def sm3_hash(s):
    if isinstance(s, str):
        s = s.encode()
    return sm3.sm3_hash([i for i in s])


# 该参数为国家密码管理局推荐参数 https://www.oscca.gov.cn/sca/xxgk/2010-12/17/1002386/files/b965ce832cc34bc191cb1cde446b860d.pdf
p = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
a = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
b = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93
n = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
Gx = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
Gy = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0


def add(x1, y1, x2, y2, a, p):
    if x1 == x2 and y1 == p - y2:
        return False
    if x1 != x2:
        tmp = ((y2 - y1) * invp(x2 - x1, p)) % p
    else:
        tmp = (((3 * x1 * x1 + a) % p) * invp(2 * y1, p)) % p
    x3 = (tmp * tmp - x1 - x2) % p
    y3 = (tmp * (x1 - x3) - y1) % p
    return x3, y3


def mult(x, y, k, a, p):
    k = bin(k)[2:]
    qx, qy = x, y
    for i in range(1, len(k)):
        qx, qy = add(qx, qy, qx, qy, a, p)
        if k[i] == '1':
            qx, qy = add(qx, qy, x, y, a, p)
    return qx, qy


def KDF(z, klen):
    k = ''
    tmp = 1
    for _ in range(math.ceil(klen / 256)):
        k = k + sm3_hash(hex(int(z + '{:032b}'.format(tmp), 2))[2:])
        tmp = tmp + 1
    k = '0' * ((256 - (len(bin(int(k, 16))[2:]) % 256)) % 256) + bin(int(k, 16))[2:]
    return k[:klen]


dB = randint(1, n - 1)
xB, yB = mult(Gx, Gy, dB, a, p)


def Enc(m: str):
    plen = len(hex(p)[2:])
    m = '0' * ((4 - (len(bin(int(m.encode().hex(), 16))[2:]) % 4)) % 4) + bin(int(m.encode().hex(), 16))[2:]
    klen = len(m)
    while True:
        k = randint(1, n)
        while k == dB:
            k = randint(1, n)
        x2, y2 = mult(xB, yB, k, a, p)
        x2, y2 = '{:0256b}'.format(x2), '{:0256b}'.format(y2)
        t = KDF(x2 + y2, klen)
        if int(t, 2) != 0:
            break
    x1, y1 = mult(Gx, Gy, k, a, p)
    x1, y1 = (plen - len(hex(x1)[2:])) * '0' + hex(x1)[2:], (plen - len(hex(y1)[2:])) * '0' + hex(y1)[2:]
    c1 = '04' + x1 + y1
    c2 = ((klen // 4) - len(hex(int(m, 2) ^ int(t, 2))[2:])) * '0' + hex(int(m, 2) ^ int(t, 2))[2:]
    c3 = sm3_hash(hex(int(x2 + m + y2, 2))[2:])
    return c1, c2, c3


def decrypt(c1, c2, c3, a, b, p):
    c1 = c1[2:]
    x1, y1 = int(c1[:len(c1) // 2], 16), int(c1[len(c1) // 2:], 16)
    if pow(y1, 2, p) != (pow(x1, 3, p) + a * x1 + b) % p:
        return False
    x2, y2 = mult(x1, y1, dB, a, p)
    x2, y2 = '{:0256b}'.format(x2), '{:0256b}'.format(y2)
    klen = len(c2) * 4
    t = KDF(x2 + y2, klen)
    if int(t, 2) == 0:
        return False
    m = '0' * (klen - len(bin(int(c2, 16) ^ int(t, 2))[2:])) + bin(int(c2, 16) ^ int(t, 2))[2:]
    u = sm3_hash(hex(int(x2 + m + y2, 2))[2:])
    if u != c3:
        return False
    return hex(int(m, 2))[2:]

msg = "sm2"
print("msg:", msg)
c1, c2, c3 = Enc(msg)
c = (c1 + c2 + c3).upper()
print('encrypted : ',end="")
for i in range(len(c)):
    print(c[i * 8:(i + 1) * 8], end='')
print('\ndecrypted: ',end="")
m1 = decrypt(c1, c2, c3, a, b, p)
if m1:
    m1 = str(bytes.fromhex(m1))
    m1 = '\n'.join(m1[2:-1].split('\\n'))
    print(m1)
    print(msg == m1)
else:
    print(False)
