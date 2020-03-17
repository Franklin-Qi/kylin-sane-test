# 简单扫描SANE API 使用

## 开始安装SANE及对应的开发库
```
sudo apt-get update
sudo apt-get sane sane-utils libsane-dev

sudo ln -s /usr/lib/x86_64-linux-gnu/libsane.so.1 /usr/lib/libsane.so
```

## 源码演示
1. 更改Makefile的头文件路径
```
SANE_INCLUDE=/home/yusq/kylin-sane-test/include/
```

2. 编译并执行
```
make
./kylinSane
```

3. 在线文档查看
```
firefox docs/html/index.html
or
file:///home/yusq/sane-test/docs/html/index.html
```

## 参考文档
* [SANE - Documentation][2]
* [SANE - Other github][3]
* [SANE - Documentation CN][4]

[2]:http://www.sane-project.org/docs.html
[3]:https://github.com/yushulx/linux-document-scanning
[4]:https://blog.csdn.net/weixin_39743893/article/details/83350568

