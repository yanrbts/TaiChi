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

#include <tch_core.h>

/* Display some info to the user at the start */
void tch_show_logo(const char *ip, const char *name){
	printf("___________      .___________ .__    .__ \n");
	printf("\\__    ___/____  |__\\_   ___ \\|  |__ |__|\n");
	printf("  |    |  \\__  \\ |  /    \\  \\/|  |  \\|  |\n");
	printf("  |    |   / __ \\|  \\     \\___|   Y  \\  |\n");
	printf("  |____|  (____  /__|\\______  /___|  /__|\n");
	printf("               \\/           \\/     \\/   \n");
    printf("\n");
    printf("                     Version : 1.0.0\n");
    printf("                     Author  : 0xffff\n");
	printf("                     host    : %s\n", name);
	printf("                     ip : %s\n", ip);
}