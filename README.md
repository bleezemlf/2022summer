# 2022summer

[TOC]

## 生日攻击

### 实验目的

实现对国密算法sm3的生日攻击

### 生日攻击原理：生日悖论

如果一个班级中的人数大于等于`23`，可以计算出有两个人生日相同的概率要大于`50%`。应用于密码学散列函数中可以得到结论：对于`n`位长的哈希表，概率上仅需要$2^\frac{n}{2}$次检测就可以找到一对碰撞，而非$2^n$次。

本实验通过随机生成字符串作为`SM3`的消息输入，实现消息散列值的部分碰撞。

### 实验过程

#### 代码说明

首先借鉴一个随机生成指定长度字符串的函数`genRandomString()`，实现代码如下：

```c++
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
```

然后调用`OPENSSL`的`EVP`框架里的`SM3`加密，封装的`sm3_hash()`如下：

```c++
int sm3_hash(char* message, size_t len, unsigned char* hash, unsigned int* hash_len)
{
    EVP_MD_CTX* md_ctx;
    const EVP_MD* md;

    md = EVP_sm3();
    md_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(md_ctx, md, NULL);
    EVP_DigestUpdate(md_ctx, message, len);
    EVP_DigestFinal_ex(md_ctx, hash, hash_len);
    EVP_MD_CTX_free(md_ctx);
    return 0;
}
```

在`demo`中，设置碰撞位置为前三个字节。调用`genRandomString()`函数随机生成两个字符串`buff1,buff2`，调用`sm3_hash()`分别计算`buff1`和`buff2`的散列值`hash_value1,hash_value2`，如果碰撞位置的散列值相同则输出结果，否则重复上述过程直至找到部分碰撞。该过程代码如下：

```c++
while (true) {
		buff1 = genRandomString(64);
		buff2 = genRandomString(64);
		unsigned int buff1_len = strlen((char*)buff1);
		unsigned int buff2_len = strlen((char*)buff2);
		unsigned char hash_value1[64];
		unsigned char hash_value2[64];
		unsigned int i, hash_len;

		sm3_hash(buff1, buff1_len, hash_value1, &hash_len);
		sm3_hash(buff2, buff2_len, hash_value2, &hash_len);
		int counter = 0;
		for (int i = 0;i < coll_num;i++) {
			if (hash_value1[i] == hash_value2[i]) {
				counter++;
			}
		}
```

#### 运行指导

可以修改`coll_num`的值决定碰撞位数，默认从散列值的第一位开始计数，也可以修改为碰撞散列值中离散的位置。

直接运行代码即可。

下图为碰撞散列值前三个字节的实验结果：

![image-20220728160411667](resource/实验1.assets/image-20220728160411667.png)

### 实验改进

#### 改进的生日攻击

上述的生日攻击需要攻击者存储所有的散列值，因为攻击者不知道事先哪一对输入会造成碰撞，存储下之前所有问询过的散列值，可以减少计算散列值的次数，但是相应的将增加检查是否碰撞的次数（本实验中实验设备有限，故没有存储所有的散列值）。但是相对于时间来说，空间通常是更稀缺的资源，所以有必要将生日攻击的空间需求降低，即采用一个固定数量的存储。具体方法如下：

采取一个随机的初始值$x_0$，对于$i>0$，计算出$x_i:=H(x_{i-1})$和$x_{2i}:=H(H(x_{2(i-1)}))$，在每一步中，比较$x_i$和$x_{2i}$，如果相同，则找到一对碰撞。如果$H$被定义为随机函数，则发生碰撞的概率和简单的生日攻击相同，而`SM3`的加密过程可以视为伪随机函数，因此也可以在$n$位长的哈希表中，概率上以需要$2^\frac{n}{2}$次检测找到一对碰撞。

#### 代码说明

`genRandomString()`和`sm3_hash()`函数定义与简单的生日攻击相同。

随机生成一个`32`字节的字符串`buff1`作为$x_0$，对于$i>0$，用`tmp1`表示$x_{i-1}$，用`tmp2`表示$x_{2(i-1)}$，用`tmp2`表示$H(x_{2(i-1)})$。

调用`sm3_hash()`函数计算`tmp1`的散列值`hash_value1`，计算`tmp2`的散列值`hash_value2`也就是`tmp_2`，计算`tmp_2`的散列值`hash_value2`。

比较`hash_value1`和`hash_value2`的部分位置是否相同，如果设定的碰撞位置上的字符相同，则输出结果并停止循环，不相同则将`hash_value1`的值代入`tmp1`，将`hash_value2`的值代入`tmp2`中，循环进行上述计算。实现代码如下：

```c++
while (true) {

		unsigned int tmp1_len = 32;
		sm3_hash1(tmp1, tmp1_len, hash_value1, &hash_len);//x1,x2,x3...

		unsigned int tmp2_len = 32;
		sm3_hash1(tmp2, tmp2_len, hash_value2, &hash_len);
		sm3_hash1(hash_value2, hash_len, hash_value2_, &hash_len);//x2,x4,x6...
		
		memcpy((void*)tmp2, hash_value2_, 33);//x2,x4,x6...
		memcpy((void*)tmp1, hash_value1, 33);//x1,x2,x3...
```

举例说明，例如第一轮用$x_0$加密得到$x_1$和$x_2$，如果$x_1$和$x_2$没有发生期待的碰撞，则将$x_1$和$x_2$作为下一轮计算的初始值，在第二轮中得到$x_2:=H(x_{1})$和$x_4:=H(H(x_2))$。如果$x_2$和$x_4$没有发生期待的碰撞，则将$x_2$和$x_4$作为下一轮计算的初始值，在第二轮中得到$x_3:=H(x_{2})$和$x_6:=H(H(x_4))$，比较$x_3$和$x_6$是否发生期待碰撞。以此类推，直到发生期待的碰撞为止，计算过程形似$\rho$。

![C{%%%$[Z`Z%@NH`F]TJOFOG](resource/实验1.assets/C{%%%$[Z`Z%@NH`F]TJOFOG.png)

#### 运行指导

可以修改`coll\_num`的值决定碰撞位数，默认从散列值的第一位开始计数，也可以修改为碰撞散列值中离散的位置。

直接运行代码即可。

下图为碰撞散列值前一个字节的实验结果：

![](resource/实验1.assets/3.png)

#### 实验问题

本实验中改进后的生日攻击比简单的生日攻击速度慢了许多，原因是在计算$x_{2i}:=H(H(x_{2(i-1)}))$的时候进行了两次`SM3`的计算，但是其中的$H(x_{2(i-1)})$已经在之前某一轮的$x_j$中计算过，证明如下，若存在$H(x_{2(i-1)})=H(x_{j-1})$，可以计算出$j=2i-1$，其中$i,j>0$，所以有$i\le j$，得证。因此可以发现问题$\rho$的“尾巴”上的值都被重复计算了一次，浪费了大量的时间，因此本实验的改进的生日攻击速度远不如相同碰撞位数的简单的生日攻击。实验的下一步将对此进行改进。