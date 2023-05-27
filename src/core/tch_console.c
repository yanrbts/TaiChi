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

char             *tch_cmd_prompt = NULL;
tch_module_t     *tch_current_module = NULL;

static void  tch_do_completion(char const *prefix, linenoiseCompletions* lc);
static void  tch_do_parse_cmd(char *cmdline);

void
tch_console_loop()
{
    char       *line;
    //char       *prompt = tch_prompt;
    tch_cmd_prompt = tch_prompt;

    const char *file = "./history";

    linenoiseHistoryLoad(file);
    linenoiseSetCompletionCallback(tch_do_completion);

    while ((line = linenoise(tch_cmd_prompt)) != NULL) {

        line = tch_right_trim(line);

        if (line[0] != '\0' && line[0] != '/') {
            
            tch_do_parse_cmd(line);

        } else {
            //tch_cmd_prompt = tch_prompt;
            //printf(" %sUnreconized command: %s %s\n", TCH_COLOR_RED, line, TCH_COLOR_END);
        }

        free(line);
    }

    linenoiseHistorySave(file);
}

static void  
tch_do_completion(char const *prefix, linenoiseCompletions* lc)
{
    size_t      i;

    if (tch_current_module == NULL) {

        for (i = 0; tch_modules[i] != NULL; i++) {
            if (tch_strncmp(prefix, tch_modules[i]->name, tch_strlen(prefix)) == 0) {
                linenoiseAddCompletion(lc, tch_modules[i]->name);
            }
        }

    } else {

        tch_command_t   *pcmd;

        for (pcmd = tch_current_module->commands ; pcmd->name != NULL; pcmd++) {
            if (tch_strncmp(prefix, pcmd->name, tch_strlen(prefix)) == 0) {
                linenoiseAddCompletion(lc, pcmd->name);
            }
        }
    }

    // show command
    for (i = 0; tch_cmds[i].name != NULL; i++) {
        if (tch_strncmp(prefix, tch_cmds[i].name, tch_strlen(prefix)) == 0) {
            linenoiseAddCompletion(lc, tch_cmds[i].name);
        }
    }
}

static void
tch_do_parse_cmd(char *cmdline)
{
    size_t          i;

    if (tch_current_module == NULL) {
        char    **cmd = NULL;
        int     size = tch_space_parsecmd(cmdline, " ", &cmd);
        // parse modules
        for (i = 0; tch_modules[i] != NULL; i++) {
            
            if (tch_strcmp(cmd[0], tch_modules[i]->name) == 0) {

                //tch_splice_prefix(tch_cmd_prompt, cmdline);
                if (tch_modules[i]->execmd != NULL) {
                    tch_modules[i]->execmd(size, cmd);
                } else {
                    tch_cmd_prompt = (char *)tch_modules[i]->cmdshell;
                    tch_current_module = tch_modules[i];
                }
                tch_array_free(cmd, size);
                return;
            } 
        }

        for (i = 0; tch_cmds[i].name != NULL; i++) {
            if (tch_strcmp(cmd[0], tch_cmds[i].name) == 0) {
                tch_cmds[i].func(size, cmd);
                tch_array_free(cmd, size);
                return;
            }
        }
        tch_array_free(cmd, size);
        
        printf("%s [-] Unreconized module %s\n", TCH_COLOR_RED,TCH_COLOR_END); 

    } else {
        //parse cmdline
        tch_command_t   *pcmd;
        char            **cmd = NULL;
        int size = tch_space_parsecmd(cmdline, " ", &cmd);

        for (pcmd = tch_current_module->commands; pcmd->name != NULL; pcmd++ ) {
            
            if (tch_strcmp(cmd[0], pcmd->name) == 0) {
                pcmd->func(size, cmd);

                tch_array_free(cmd, size);
                return;
            }
        }

        for (i = 0; tch_cmds[i].name != NULL; i++) {
            if (tch_strcmp(cmd[0], tch_cmds[i].name) == 0) {
                tch_cmds[i].func(size, cmd);
                tch_array_free(cmd, size);
                return;
            }
        }

        tch_array_free(cmd, size);
        printf("%s [-] Unreconized command %s\n", TCH_COLOR_RED,TCH_COLOR_END);
    }
}