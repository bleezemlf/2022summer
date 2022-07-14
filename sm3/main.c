#include "sm3.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
int main()
{
    printf("testOpensslSm3\n");
    testOpensslSm3();
    printf("\n\ntestOpensslSm3\n");
    testSimpleSm3();
    return 0;
}
