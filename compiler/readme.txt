复现的步骤：

需要安装：
1. g++
2. Mars mips simulator 
http://courses.missouristate.edu/kenvollmar/mars/
3. C++的boost库 (ubuntu下安装）
sudo apt-get install libboost-all-dev

将in.txt放到代码所在的目录下（grammar1.txt和grammar2.txt也要在该目录下），运行：
g++ main.cpp -o main
./main in.txt

词法分析的结果在in.txt.la1和in.txt.la2中；
语法分析的结果直接输出在console；
语义分析的结果在symbol_table.txt和qt_sequence.txt中；
目标代码在mips.asm中；
使用Mars模拟器执行MIPS代码。