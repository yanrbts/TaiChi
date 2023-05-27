# Copyright (C) Yan Ruibing

CORE_INCS="src/core \
           src/fmq \
           include"

CORE_DEPS="include/zyre_event.h  \
           include/zyre_library.h \
           include/zyre.h \
           src/fmq/tch_fmqmsg.h \
           src/fmq/tch_client.h \
           src/fmq/tch_server.h \
           src/core/taichi.h \
           src/core/tch_config.h \
           src/core/tch_core.h   \
           src/core/tch_define.h \
           src/core/tch_string.h \
           src/core/tch_logo.h   \
           src/core/tch_module.h \
           src/core/tch_linenoise.h \
           src/core/tch_cmd.h    \
           src/core/tch_log.h \
           src/core/tch_file.h \
           src/core/tch_console.h \
           src/core/tch_until.h \
           src/core/tch_node.h"

CORE_SRCS="src/core/taichi.c   \
           src/core/tch_string.c \
           src/core/tch_logo.c \
           src/core/tch_cmd.c  \
           src/core/tch_linenoise.c \
           src/core/tch_file.c \
           src/core/tch_module.c \
           src/core/tch_console.c \
           src/core/tch_until.c \
           src/core/tch_log.c \
           src/fmq/tch_fmqmsg.c \
           src/fmq/tch_client.c \
           src/fmq/tch_server.c \
           src/core/tch_node.c"

UNIX_INCS="$CORE_INCS src/os/unix"

UNIX_DEPS="$CORE_DEPS \
            src/os/unix/tch_files.h"

UNIX_SRCS="$CORE_SRCS"

LINUX_DEPS="src/os/unix/tch_linux_config.h"