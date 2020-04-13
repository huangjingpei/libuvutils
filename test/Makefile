$(shell export LD_LIBRARY_PATH=`pwd`/..:$LD_LIBRARY_PATH)
CC = gcc
CXX = g++
CFLAGS = -O -DDEBUG -I../include -I../
LDFLAGS = -L../lib -L../ -luv -luvutils -Wl,-rpath ../

#wildcard 对 c文件进行展开
SRCS = $(wildcard *.c) $(wildcard *.cpp) 
#去除路径信息
dir=$(notdir $(SRCS))
# 把后缀.c 替换成 .o
OBJS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o,$(dir))  )

DATALIBS = dblib/lib.a
ZIPLIBS = ZipCoder/ZipCoder.a  

TARGET = test_Thread test_Timer test_TcpServer test_TcpClient
all: $(TARGET)
	@echo $(SRCS)
	@echo $(OBJS)
test_Thread : test_Thread.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)
test_Timer :  test_Timer.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)
test_TcpServer :  test_TcpServer.o netstring.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)
test_TcpClient :  test_TcpClient.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)


%.o : %.c
	$(CC) -c $(CFLAGS)  $^ -o $@
%.o : %.cpp
	$(CXX) -c $(CFLAGS) $^ -o $@    

.PHONY : clean

clean :
	rm -f *.o
	rm -f $(TARGET)

# install:
#         mv Excute excute; cp -f ordermisd ../bin/;
