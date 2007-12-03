/**********************************************************
 Copyright(C) 2007 Jochen Roessner <jochen@lugrot.de>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

*/
#include "rfm12_net.h"
#include "rfm12_state.h"
#include "../uip/uip.h"

#include <string.h>

#include "../config.h"

#if defined(RFM12_SUPPORT)
#ifdef ENC28J60_SUPPORT

union rx_buffer rx;

#ifndef RFM12_BRIDGE_SUPPORT

void rfm12_get_receive()
{
  int i;
  for (i = 0; i < UIP_CONNS; i ++) 
    if (uip_conns[i].callback == rfm12_net_main && uip_conn_active(i) &&
        uip_conns[i].appstate.rfm12.out_len > 0)
      return; //neu daten nicht abrufen wir sind auf tcp noch nicht alle los geworden
  
  int recv_len = rfm12_rxfinish (&rx.rxdata.toaddr, &rx.rxdata.fromaddr,
				 rx.rxdata.data);
  if (recv_len > 0 && recv_len < 254) {
    rfm12_rxstart ();
    rx.rxdata.len = recv_len;

    for (i = 0; i < UIP_CONNS; i ++) 
      if (uip_conns[i].callback == rfm12_net_main && uip_conn_active(i) && 
          (uip_conns[i].appstate.rfm12.txdata.fromaddr == rx.rxdata.toaddr ||
           (uip_conns[i].appstate.rfm12.txdata.fromaddr == 0
	    && rx.rxdata.toaddr == RFADDR)))
        uip_conns[i].appstate.rfm12.out_len = recv_len + 3;
  }

}

void rfm12_net_init()
{
  uip_listen(HTONS(RFM12_NET_PORT), rfm12_net_main);
}


void rfm12_net_main(void)
{
  struct rfm12_connection_state_t *state = &uip_conn->appstate.rfm12;
  
  if(uip_connected()) {
    state->in_len = 0;
    state->in_state = 0;
    state->out_len = 0;
    state->rfaddr = 0;
    memset(state->inbuf, 0, RFM12_DataLength);
  }
  
  if(uip_acked()) {
    state->out_len = 0;
    
  }
  
  if(uip_newdata()) {
    if (uip_datalen() > 4 && uip_datalen() <= RFM12_DataLength){
      state->in_len = uip_datalen();
      memcpy(state->inbuf, uip_appdata, state->in_len);
      if(state->txdata.len + 3 < uip_datalen()){
        uip_abort();
      }
      else{
        if(state->rfaddr != 0 && state->rfaddr != state->txdata.fromaddr){
          rfm12_addr_del(state->rfaddr);
          state->rfaddr = 0;
        }
        if(state->rfaddr == 0 && state->txdata.fromaddr != 0){
          if(!rfm12_addr_add(state->txdata.fromaddr))
            uip_abort();
          state->rfaddr = state->txdata.fromaddr;
        }
        state->in_state =
	  rfm12_txto (state->txdata.fromaddr, state->txdata.toaddr,
		      state->txdata.data, state->txdata.len);
      }
    }
    else{
      uip_abort();
    }
  }
  if (uip_poll()){
    if(state->in_state >= 1)
      state->in_state =
	rfm12_txto (state->txdata.fromaddr, state->txdata.toaddr,
		    state->txdata.data, state->txdata.len);
    if(state->in_state == 1)
      uip_abort();
  }
  if(uip_rexmit() ||
     uip_newdata() ||
     uip_acked() ||
     uip_connected() ||
     uip_poll()) {
       if (state->out_len > 0) {
         uip_send(rx.rxbuffer, state->out_len);
       }
     }
  
  if(uip_closed() || uip_aborted() || uip_timedout() && state->rfaddr != 0){
    rfm12_addr_del(state->rfaddr);
    state->rfaddr = 0;
  }
  
}

#endif /* not RFM12_BRIDGE_SUPPORT */
#endif /* ENC28J60_SUPPORT */
#endif /* RFM12_SUPPORT */
