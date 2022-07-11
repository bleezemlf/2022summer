#include "sm3.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
int main()
{
    testOpensslSm3();
    /* testTrivialSm3(); */
    return 0;
}
