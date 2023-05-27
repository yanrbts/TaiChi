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
#include <tch_config.h>
#include <tch_core.h>

/*
 * If the directory exists or the creation is successful, 
 * it returns 0 directly, if the creation of the directory 
 * fails, it returns -1
 */
int 
tch_mkdir(const char *s)
{
    int         status;
    struct stat st = {0};

    if (stat(s, &st) == 0 && (st.st_mode & S_IFDIR)) {
        return TCH_OK;
    }

    status = mkdir(s, S_IRWXU | S_IRWXG);
    if (status == -1 && errno != EEXIST) {
        TCHLOGE("mkdir() error : %s", strerror(errno));
        return TCH_ERROR;
    }

    return TCH_OK;
}

int 
tch_gethost(char *name, size_t nlen)
{
    // To retrieve hostname
    if (gethostname(name, nlen) != 0) {
        tch_error("gethostname()");
        tch_memcpy(name, "unknown", nlen);
        return -1;
    }
    return 0;
}

/*C program to get IP Address of Linux Computer System.*/
int 
tch_getip(char *ip, size_t ilen)
{
    int fd;
    struct ifreq ifr;

    /*AF_INET - to define network interface IPv4*/
    /*Creating soket for it.*/
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /*AF_INET - to define IPv4 Address type.*/
    ifr.ifr_addr.sa_family = AF_INET;

    /*eth0 - define the ifr_name - port name
    where network attached.*/
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    /*Accessing network interface information by
    passing address using ioctl.*/
    ioctl(fd, SIOCGIFADDR, &ifr);
    /*closing fd*/
    close(fd);

    /*Extract IP Address*/
    tch_memcpy(ip, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr), ilen);

    return 0;
}

int 
tch_space_parsecmd(char *s, const char *d, char ***a)
{
    if (s == NULL || d == NULL)
        return 0;

    int     i = 0;
    int     space = 0;
    char    *sp = s;
    char    **ap;
    char    *p;

    /* count the number of spaces */
    while (*sp != '\0') {
        /*if (*sp == *d && *(sp+1) != *d)
            space++;*/
        if (isspace(*sp))
            space++;
        sp++;
    }
    
    *a = (char**)malloc((space+1) * sizeof(char*));
    ap = *a;

    p = strtok(s, d);
    while (p != NULL) {
        int n = strlen(p) + 1;

        *ap = (char*)malloc(n);
        memcpy(*ap, p, strlen(p));
        (*ap)[n-1] = '\0';

        p = strtok(NULL, d);
        ap++;
        i++;
    }
    
    return i;
}

void 
tch_array_free(char **a, int size) {
    for (int i = 0; i < size; i++)
        free(*(a+i));
    free(a);
}