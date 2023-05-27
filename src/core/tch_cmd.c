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
 */

#include <tch_config.h>
#include <tch_core.h>

static int tch_help(int argc, char **argv);
static int tch_back(int argc, char **argv);

tch_command_t tch_cmds[] = {
	{"help",     "help command.",       				tch_help},
    {"back",     "Back to previous.",              		tch_back},
    {"select",   "select accepting file host node.",   	tch_select_node},
	{"list",     "List LAN connection hosts.",   		tch_list_node},
    {"quit",     "launch program.",             		tch_quit},
    {NULL,        NULL,                      			NULL}
};

static int 
tch_help(int argc, char **argv)
{
	TCH_UNUSED(argc);
	TCH_UNUSED(argv);

	size_t      i;
    printf("\n");

    if (tch_current_module == NULL) {
		tch_write_stderr("cmds options:"TCH_LINEFEED);
		for (i = 0; tch_cmds[i].name != NULL; i++) {
			printf("  %-10s  %-10s\n", tch_cmds[i].name, tch_cmds[i].description);
		}

		printf("\n");
        tch_write_stderr("modules options:"TCH_LINEFEED);
        for (i = 0; tch_modules[i] != NULL; i++) {
            printf("  %-10s  %-10s\n", tch_modules[i]->name, tch_modules[i]->description);
        }

    } else {

        tch_write_stderr("cmd options:"TCH_LINEFEED);
        tch_command_t   *pcmd;
        for (pcmd = tch_current_module->commands ; pcmd != NULL && pcmd->name != NULL; pcmd++) {
            printf("  %-10s  %-10s\n", pcmd->name, pcmd->description);
        }
    }

    printf("\n");

	return 0;
}

static int
tch_back(int argc, char **argv)
{
	tch_cmd_prompt = tch_prompt;
    tch_current_module = NULL;

	return 0;
}
