# 简单扫描SANE API 使用
:earth_asia:
:cn:

中英文切换 :arrow_right: [English](README_EN.md)

经过对sane-backends, xsane源码的研究，已经能满足扫描仪基础功能。

目前测试通过了的设备有佳能(canon)210型号扫描仪,其他类型设备处理逻辑也类似，相信很有参考价值。

此扫描仪分析源码支持的特性有:

1. 支持获取当前扫描仪的设备类型(平板式, 馈纸式);

2. 支持设置不同的分辨率(4800, 2400, 1200, 600, 300, 150, 100, 75)扫描;

3. 支持设置不同的纸张尺寸(A4, A3)扫描;

4. 支持设置不同的色彩模式(黑白, 彩色, 灰度)扫描;


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

## API文档在线生成
``` bash
doxygen -g
./autodoxygen.sh   # 更改Doxygen配置
doxygen Doxygen

```


## 参考文档
* [SANE - Documentation][2]
* [SANE - Other github][3]
* [SANE - Documentation CN][4]

[2]:http://www.sane-project.org/docs.html
[3]:https://github.com/yushulx/linux-document-scanning
[4]:https://blog.csdn.net/weixin_39743893/article/details/83350568


