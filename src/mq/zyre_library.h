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
 * Zyre - Local Area Clustering for Peer-to-Peer 
 * Zyre provides reliable group messaging over local area networks. It has these key characteristics:

    Zyre needs no administration or configuration.
    Peers may join and leave the network at any time.
    Peers talk to each other without any central brokers or servers.
    Peers can talk directly to each other.
    Peers can join groups, and then talk to groups.
    Zyre is reliable, and loses no messages even when the network is heavily loaded.
    Zyre is fast and has low latency, requiring no consensus protocols.
    Zyre is designed for WiFi networks, yet also works well on Ethernet networks.
    Time for a new peer to join a network is about one second.

 *
 */

#ifndef ZYRE_LIBRARY_H_INCLUDED
#define ZYRE_LIBRARY_H_INCLUDED

#include <czmq.h>               //  External dependencies

//  ZYRE version macros for compile-time API detection
#define ZYRE_VERSION_MAJOR      2
#define ZYRE_VERSION_MINOR      0
#define ZYRE_VERSION_PATCH      1
//  Public constants
#define ZRE_DISCOVERY_PORT      5670               //  IANA-assigned UDP port for ZRE

#ifdef  ZYRE_BUILD_DRAFT_API
#define ZAP_DOMAIN_DEFAULT      "global"           //  Default ZAP domain (auth)
#endif 

#define ZYRE_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define ZYRE_VERSION \
    ZYRE_MAKE_VERSION(ZYRE_VERSION_MAJOR, ZYRE_VERSION_MINOR, ZYRE_VERSION_PATCH)



#if defined (__WINDOWS__)
#   if defined ZYRE_STATIC
#       define ZYRE_EXPORT
#   elif defined ZYRE_INTERNAL_BUILD
#       if defined DLL_EXPORT
#           define ZYRE_EXPORT __declspec(dllexport)
#       else
#           define ZYRE_EXPORT
#       endif
#   elif defined ZYRE_EXPORTS
#       define ZYRE_EXPORT __declspec(dllexport)
#   else
#       define ZYRE_EXPORT __declspec(dllimport)
#   endif
#   define ZYRE_PRIVATE
#elif defined (__CYGWIN__)
#   define ZYRE_EXPORT
#   define ZYRE_PRIVATE
#else
#   if (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER
#       define ZYRE_PRIVATE __attribute__ ((visibility ("hidden")))
#       define ZYRE_EXPORT __attribute__ ((visibility ("default")))
#   else
#       define ZYRE_PRIVATE
#       define ZYRE_EXPORT
#   endif
#endif

//  Opaque class structures to allow forward references
//  These classes are stable or legacy and built in all releases
//typedef struct _zyre_t          zyre_t;
//typedef struct _zyre_event_t    zyre_event_t;

//  Public constants
#define  ZRE_DISCOVERY_PORT         5670               //  IANA-assigned UDP port for ZRE
#define  ZAP_DOMAIN_DEFAULT         "global"           //  Default ZAP domain (auth)
//  Private constants
#define REAP_INTERVAL	            1000               // Once per second

#endif