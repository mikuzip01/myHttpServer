HTTP_SERVER_EXEC_NAME:=				httpserver
CGI_EXEC_NAME:=						postdataecho.cgi

http-obj-m:=						src/AsyncLoger.o
http-obj-m+=						src/Error.o
http-obj-m+=						src/HttpData.o
http-obj-m+=						src/httpserver.o
http-obj-m+=						src/Mutex.o
http-obj-m+=						src/Pages.o
http-obj-m+=						src/Responser.o
http-obj-m+=						src/ThreadPool.o
http-obj-m+=						src/Timer.o
http-obj-m+=						src/utils.o

cgi-obj-m:= 						src/cgidemo.o

GCC:=								gcc
GXX:=								g++

CPP_LINK_PTTHREAD:=					-pthread

CPPFLAGS:=							-std=c++20
CPPFLAGS+=							-D__PRINT_INFO_TO_DISP__
CPPFLAGS+=							-D__LOG_INFO__
CPPFLAGS+=							-Iinclude/

Q		:= 							

.PHONY: all clean test

.PHONY: http cgi

all: http cgi

# http server 主程序
http: $(HTTP_SERVER_EXEC_NAME)
# CGI 程序
cgi: $(CGI_EXEC_NAME)

$(HTTP_SERVER_EXEC_NAME): $(http-obj-m)
	$(Q)$(GXX) -o $(HTTP_SERVER_EXEC_NAME) $(CPPFLAGS) $(CPP_LINK_PTTHREAD) $(http-obj-m)

$(http-obj-m): %.o: %.cpp
# 指定 -c 选项使其只生成目标文件
	$(Q)$(GXX) -c $(CPPFLAGS) -o $@ $(patsubst %.o,%.cpp,$@)

$(CGI_EXEC_NAME): $(cgi-obj-m)
	$(Q)$(GCC) -o $(CGI_EXEC_NAME) $(cgi-obj-m)

$(cgi-obj-m): %.o: %.c
# 指定 -c 选项使其只生成目标文件
	$(Q)$(GCC) -c -o $@ $(patsubst %.o,%.c,$@)

clean:
	$(Q)rm -f $(http-obj-m) $(cgi-obj-m)
	$(Q)rm -f $(HTTP_SERVER_EXEC_NAME)
	$(Q)rm -f $(CGI_EXEC_NAME)
