#include <stdio.h>
#include <string.h>
#include "sm3hash.h"
#include <stdlib.h>
#include <time.h>
#define coll_num 1

char* genRandomString(int length)
{
	int flag, i;
	char* string;
	if ((string = (char*)malloc(length)) == NULL)
	{
		printf("Malloc failed!\n");
		return NULL;
	}

	for (i = 0; i < length - 1; i++)
	{
		flag = rand() % 3;
		switch (flag)
		{
		case 0:
			string[i] = 'A' + rand() % 26;
			break;
		case 1:
			string[i] = 'a' + rand() % 26;
			break;
		case 2:
			string[i] = '0' + rand() % 10;
			break;
		default:
			string[i] = 'x';
			break;
		}
	}
	string[length - 1] = '\0';
	return string;
}


int main(void)
{
	char* buff1;
	char* buff2;
	char* buff2_;

	srand((unsigned)time(NULL));
	buff1 = genRandomString(33);//x0
	buff2 = genRandomString(33);
	buff2_ = genRandomString(33);

	//unsigned char hash_value[64];//h_0
	unsigned char hash_value1[64];
	unsigned char hash_value2[64];
	unsigned char hash_value2_[64];
	unsigned int i, hash_len;


	const unsigned char* tmp1 = (const unsigned char*)buff1;
	const unsigned char* tmp2 = (const unsigned char*)buff2;
	const unsigned char* tmp2_= (const unsigned char*)buff2_;

	memcpy((void*)tmp2, tmp1, 33);

	while (true) {
	//for(int j =0;j<3;j++){

		//unsigned int tmp1_len = strlen((char*)tmp1);
		unsigned int tmp1_len = 32;
		sm3_hash1(tmp1, tmp1_len, hash_value1, &hash_len);//x1,x2,x3...
		//unsigned int tmp2_len = strlen((char*)tmp2);
		unsigned int tmp2_len = 32;
		sm3_hash1(tmp2, tmp2_len, hash_value2, &hash_len);
		sm3_hash1(hash_value2, hash_len, hash_value2_, &hash_len);//x2,x4,x6...

		int counter = 0;
		for (int i = 0;i < 1;i++) {
			if (hash_value1[i] == hash_value2_[i]) {
				counter++;
			}
		}
		if (counter == coll_num) {
			printf("find a one Byte partial collision \n");

			printf("x_i-1:\n");
			for (i = 0; i < hash_len; i++)
			{
				printf("0x%x  ", tmp1[i]);
			}
			printf("\n\n");

			printf("H(x_2(i-1)):\n");
			for (i = 0; i < hash_len; i++)
			{
				printf("0x%x  ", hash_value2[i]);
			}
			printf("\n\n");

			printf("first sm3\n");
			for (i = 0; i < hash_len; i++)
			{
				printf("0x%x  ", hash_value1[i]);
			}
			printf("\n\n");


			printf("last sm3\n");
			for (i = 0; i < hash_len; i++)
			{
				printf("0x%x  ", hash_value2_[i]);
			}
			printf("\n\n");
			free (buff1);
			free (buff2);
			free (buff2_);

			break;
		}



		memcpy((void*)tmp2, hash_value2_, 33);//x2,x4,x6...
		memcpy((void*)tmp1, hash_value1, 33);//x1,x2,x3...
	}
	return 0;
}