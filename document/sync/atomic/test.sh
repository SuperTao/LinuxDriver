#!/bin/sh
# 后台运行，同时访问通过一个资源
echo 11111 > /dev/my_misc &
echo 22222 > /dev/my_misc &
