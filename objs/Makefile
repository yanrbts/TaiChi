
CC =	cc
CFLAGS =  -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g 
CPP =	cc -E
LINK =	$(CC)


ALL_INCS = -I src/core \
	-I src/fmq \
	-I include \
	-I src/os/unix \
	-I objs


CORE_DEPS = include/zyre_event.h \
	include/zyre_library.h \
	include/zyre.h \
	src/fmq/tch_fmqmsg.h \
	src/fmq/tch_client.h \
	src/fmq/tch_server.h \
	src/core/taichi.h \
	src/core/tch_config.h \
	src/core/tch_core.h \
	src/core/tch_define.h \
	src/core/tch_string.h \
	src/core/tch_logo.h \
	src/core/tch_module.h \
	src/core/tch_linenoise.h \
	src/core/tch_cmd.h \
	src/core/tch_log.h \
	src/core/tch_file.h \
	src/core/tch_console.h \
	src/core/tch_until.h \
	src/core/tch_node.h \
	src/os/unix/tch_files.h \
	src/os/unix/tch_linux_config.h \
	objs/tch_auto_config.h


CORE_INCS = -I src/core \
	-I src/fmq \
	-I include \
	-I src/os/unix \
	-I objs


build:	binary

binary:	objs/taichi

objs/taichi: objs/src/core/taichi.o \
	objs/src/core/tch_string.o \
	objs/src/core/tch_logo.o \
	objs/src/core/tch_cmd.o \
	objs/src/core/tch_linenoise.o \
	objs/src/core/tch_file.o \
	objs/src/core/tch_module.o \
	objs/src/core/tch_console.o \
	objs/src/core/tch_until.o \
	objs/src/core/tch_log.o \
	objs/src/fmq/tch_fmqmsg.o \
	objs/src/fmq/tch_client.o \
	objs/src/fmq/tch_server.o \
	objs/src/core/tch_node.o
	$(LINK) -o objs/taichi \
	objs/src/core/taichi.o \
	objs/src/core/tch_string.o \
	objs/src/core/tch_logo.o \
	objs/src/core/tch_cmd.o \
	objs/src/core/tch_linenoise.o \
	objs/src/core/tch_file.o \
	objs/src/core/tch_module.o \
	objs/src/core/tch_console.o \
	objs/src/core/tch_until.o \
	objs/src/core/tch_log.o \
	objs/src/fmq/tch_fmqmsg.o \
	objs/src/fmq/tch_client.o \
	objs/src/fmq/tch_server.o \
	objs/src/core/tch_node.o -lpthread -lzmq -lczmq -Wl,-rpath,./lib -L./lib/ -lzyre
	


modules:


objs/src/core/taichi.o:	$(CORE_DEPS) \
	src/core/taichi.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/taichi.o \
		src/core/taichi.c


objs/src/core/tch_string.o:	$(CORE_DEPS) \
	src/core/tch_string.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_string.o \
		src/core/tch_string.c


objs/src/core/tch_logo.o:	$(CORE_DEPS) \
	src/core/tch_logo.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_logo.o \
		src/core/tch_logo.c


objs/src/core/tch_cmd.o:	$(CORE_DEPS) \
	src/core/tch_cmd.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_cmd.o \
		src/core/tch_cmd.c


objs/src/core/tch_linenoise.o:	$(CORE_DEPS) \
	src/core/tch_linenoise.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_linenoise.o \
		src/core/tch_linenoise.c


objs/src/core/tch_file.o:	$(CORE_DEPS) \
	src/core/tch_file.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_file.o \
		src/core/tch_file.c


objs/src/core/tch_module.o:	$(CORE_DEPS) \
	src/core/tch_module.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_module.o \
		src/core/tch_module.c


objs/src/core/tch_console.o:	$(CORE_DEPS) \
	src/core/tch_console.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_console.o \
		src/core/tch_console.c


objs/src/core/tch_until.o:	$(CORE_DEPS) \
	src/core/tch_until.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_until.o \
		src/core/tch_until.c


objs/src/core/tch_log.o:	$(CORE_DEPS) \
	src/core/tch_log.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_log.o \
		src/core/tch_log.c


objs/src/fmq/tch_fmqmsg.o:	$(CORE_DEPS) \
	src/fmq/tch_fmqmsg.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/fmq/tch_fmqmsg.o \
		src/fmq/tch_fmqmsg.c


objs/src/fmq/tch_client.o:	$(CORE_DEPS) \
	src/fmq/tch_client.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/fmq/tch_client.o \
		src/fmq/tch_client.c


objs/src/fmq/tch_server.o:	$(CORE_DEPS) \
	src/fmq/tch_server.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/fmq/tch_server.o \
		src/fmq/tch_server.c


objs/src/core/tch_node.o:	$(CORE_DEPS) \
	src/core/tch_node.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/tch_node.o \
		src/core/tch_node.c

