/**
 * \addtogroup resolv
 * @{
 */
/**
 * \file
 * DNS resolver code header file.
 * \author Adam Dunkels <adam@dunkels.com>
 */

/*
 * Copyright (c) 2002-2003, Adam Dunkels.
 * Copyright (c) 2007, Christian Dietrich <stettberger@dokucode.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: resolv.h,v 1.4 2006/06/11 21:46:37 adam Exp $
 *
 */
#ifndef __RESOLV_H__
#define __RESOLV_H__

#include "protocols/uip/uip.h"

/**
 * Callback function which is called when a hostname is found.
 *
 * This callback can be passed to resolv_query, and is called after the
 * resolving of the hostname.
 *
 * \param name A pointer to the name that was looked up.  \param
 * ipaddr A pointer to a 4-byte array containing the IP address of the
 * hostname, or NULL if the hostname could not be found.
 */
typedef void (*resolv_found_callback_t)(char *name, uip_ipaddr_t *ip);

/* Functions. */
void resolv_periodic(void);
void resolv_newdata(void);

void resolv_conf(uip_ipaddr_t *dnsserver);
uip_ipaddr_t *resolv_getserver(void);
void resolv_init(void);
uip_ipaddr_t *resolv_lookup(const char *name);
void resolv_query(const char *name, resolv_found_callback_t callback);

#endif /* __RESOLV_H__ */

/** @} */
