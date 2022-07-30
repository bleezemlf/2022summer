#include "sm3.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
int main()
{
    printf("testOpensslWSm3\n");
    testOpensslSm3();
    printf("\n\ntestSimSm3\n");
    testSimpleSm3();
    printf("\n\ntestOptiSm3\n");
    testOptiSm3();
    return 0;
}
