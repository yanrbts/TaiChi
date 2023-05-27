/*
 * Project:
 *  ___________      .___________ .__    .__ 
 * \__    ___/____  |__\_   ___ \|  |__ |__|
 *   |    |  \__  \ |  /    \  \/|  |  \|  |
 *   |    |   / __ \|  \     \___|   Y  \  |
 *   |____|  (____  /__|\______  /___|  /__|
 *                \/           \/     \/   
 *
 * Copyright (C) 2021 - 2022, Yan RuiBing, <772166784@qq.com>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 */

#ifndef _TCH_CORE_H_INCLUDED_
#define _TCH_CORE_H_INCLUDED_

#include <tch_config.h>

typedef struct tch_command_s    tch_command_t;
typedef struct tch_module_s     tch_module_t;
typedef struct tch_cycle_s      tch_cycle_t;
typedef struct tch_lannode_s    tch_lannode_t;
typedef struct tch_fmq_s        tch_fmq_t;
typedef struct tch_data_s       tch_data_t;
typedef struct tch_file_s       tch_file_t;
typedef struct tch_fmq_cs       tch_fmq_cs_t;


#include <zyre.h>
#include <tch_fmqmsg.h>
#include <tch_client.h>
#include <tch_server.h>
#include <zyre_event.h>
#include <zyre_library.h>
#include <tch_define.h>
#include <tch_errno.h>
#include <tch_thread.h>
#include <tch_logo.h>
#include <tch_files.h>
#include <tch_process.h>
#include <tch_string.h>
#include <tch_log.h>
#include <tch_until.h>
#include <tch_cmd.h>
#include <tch_module.h>
#include <tch_cycle.h>
#include <tch_file.h>
#include <tch_linenoise.h>
#include <tch_console.h>
#include <tch_node.h>
#include <taichi.h>

#define  TCH_OK          0
#define  TCH_ERROR      -1

#define LF     (u_char) '\n'
#define CR     (u_char) '\r'
#define CRLF   "\r\n"

#define tch_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))

#define tch_prompt      "taichi:>"
#define TCH_FMQ_PORT    ":5670"         /* Build fmq client accept port */
#define TCH_FMQ_TCP     "tcp://*:5670"  /* FMQ SERVICE bind local tcp port*/
#define TCH_FMQ_SERVER  "FMQSERVER"     /* Notify fmq service setup complete */
#define TCH_FMQ_SVPATH  "./fmq"         /* fmq server File Directory*/
#define TCH_FMQ_CLPATH  "./clfmq"       /* fmq client File Directory*/

#endif