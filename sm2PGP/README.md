# sm2PGP

文件依赖于gmssl 使用命令`pip install gmssl`安装

生成随机会话密钥用来加密消息，然后使用sm2加密该密钥，实现时调用gmssl的sm2和sm4库进行加密，结果如下 ：

![image-20220731101843172](README.assets/image-20220731101843172.png)