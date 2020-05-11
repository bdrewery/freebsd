/*
 * le.c
 *
 * Copyright (c) 2015 Takanori Watanabe <takawata@freebsd.org>
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: hccontrol.c,v 1.5 2003/09/05 00:38:24 max Exp $
 * $FreeBSD$
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/select.h>
#include <assert.h>
#include <bitstring.h>
#include <err.h>
#include <errno.h>
#include <netgraph/ng_message.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#define L2CAP_SOCKET_CHECKED
#include <bluetooth.h>
#include "hccontrol.h"

static int le_set_scan_param(int s, int argc, char *argv[]);
static int le_set_scan_enable(int s, int argc, char *argv[]);
static int parse_param(int argc, char *argv[], char *buf, int *len);
static int le_set_scan_response(int s, int argc, char *argv[]);
static int le_read_supported_states(int s, int argc, char *argv[]);
static int le_read_local_supported_features(int s, int argc ,char *argv[]);
static int set_le_event_mask(int s, uint64_t mask);
static int set_event_mask(int s, uint64_t mask);
static int le_enable(int s, int argc, char *argv[]);
static int le_set_advertising_enable(int s, int argc, char *argv[]);
static int le_set_advertising_param(int s, int argc, char *argv[]);
static int le_read_advertising_channel_tx_power(int s, int argc, char *argv[]);
static int le_scan(int s, int argc, char *argv[]);
static void handle_le_event(ng_hci_event_pkt_t* e, bool verbose);

static int
le_set_scan_param(int s, int argc, char *argv[])
{
	int type;
	int interval;
	int window;
	int adrtype;
	int policy;
	int n;

	ng_hci_le_set_scan_parameters_cp cp;
	ng_hci_le_set_scan_parameters_rp rp;

	if (argc != 5)
		return (USAGE);
	
	if (strcmp(argv[0], "active") == 0)
		type = 1;
	else if (strcmp(argv[0], "passive") == 0)
		type = 0;
	else
		return (USAGE);

	interval = (int)(atof(argv[1])/0.625);
	interval = (interval < 4)? 4: interval;
	window = (int)(atof(argv[2])/0.625);
	window = (window < 4) ? 4 : interval;
	
	if (strcmp(argv[3], "public") == 0)
		adrtype = 0;
	else if (strcmp(argv[3], "random") == 0)
		adrtype = 1;
	else
		return (USAGE);

	if (strcmp(argv[4], "all") == 0)
		policy = 0;
	else if (strcmp(argv[4], "whitelist") == 0)
		policy = 1;
	else
		return (USAGE);

	cp.le_scan_type = type;
	cp.le_scan_interval = interval;
	cp.own_address_type = adrtype;
	cp.le_scan_window = window;
	cp.scanning_filter_policy = policy;
	n = sizeof(rp);

	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
		NG_HCI_OCF_LE_SET_SCAN_PARAMETERS), 
		(void *)&cp, sizeof(cp), (void *)&rp, &n) == ERROR)
		return (ERROR);

	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	return (OK);
}

static int
le_set_scan_enable(int s, int argc, char *argv[])
{
	ng_hci_le_set_scan_enable_cp cp;
	ng_hci_le_set_scan_enable_rp rp;
	int n, enable = 0;

	if (argc != 1)
		return (USAGE);
	  
	if (strcmp(argv[0], "enable") == 0)
		enable = 1;
	else if (strcmp(argv[0], "disable") != 0)
		return (USAGE);

	n = sizeof(rp);
	cp.le_scan_enable = enable;
	cp.filter_duplicates = 0;
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
		NG_HCI_OCF_LE_SET_SCAN_ENABLE), 
		(void *)&cp, sizeof(cp),
		(void *)&rp, &n) == ERROR)
		return (ERROR);
			
	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	fprintf(stdout, "LE Scan: %s\n",
		enable? "Enabled" : "Disabled");

	return (OK);
}

static int
parse_param(int argc, char *argv[], char *buf, int *len)
{
	char *buflast  =  buf + (*len);
	char *curbuf = buf;
	char *token,*lenpos;
	int ch;
	int datalen;
	uint16_t value;
	optreset = 1;
	optind = 0;
	while ((ch = getopt(argc, argv , "n:f:u:")) != -1) {
		switch(ch){
		case 'n':
			datalen = strlen(optarg);
			if ((curbuf + datalen + 2) >= buflast)
				goto done;
			curbuf[0] = datalen + 1;
			curbuf[1] = 8;
			curbuf += 2;
			memcpy(curbuf, optarg, datalen);
			curbuf += datalen;
			break;
		case 'f':
			if (curbuf+3 > buflast)
				goto done;
			curbuf[0] = 2;
			curbuf[1] = 1;
			curbuf[2] = (uint8_t)strtol(optarg, NULL, 16);
			curbuf += 3;
			break;
		case 'u':
			if ((buf+2) >= buflast)
				goto done;
			lenpos = curbuf;
			curbuf[1] = 2;
			*lenpos = 1;
			curbuf += 2;
			while ((token = strsep(&optarg, ",")) != NULL) {
				value = strtol(token, NULL, 16);
				if ((curbuf+2) >= buflast)
					break;
				curbuf[0] = value &0xff;
				curbuf[1] = (value>>8)&0xff;
				curbuf += 2;
				*lenpos += 2;
			}
				
		}
	}
done:
	*len = curbuf - buf;

	return (OK);
}

static int
le_set_scan_response(int s, int argc, char *argv[])
{
	ng_hci_le_set_scan_response_data_cp cp;
	ng_hci_le_set_scan_response_data_rp rp;
	int n;
	int len;
	char buf[NG_HCI_ADVERTISING_DATA_SIZE];

	len = sizeof(buf);
	parse_param(argc, argv, buf, &len);
	memset(cp.scan_response_data, 0, sizeof(cp.scan_response_data));
	cp.scan_response_data_length = len;
	memcpy(cp.scan_response_data, buf, len);
	n = sizeof(rp);
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
			NG_HCI_OCF_LE_SET_SCAN_RESPONSE_DATA), 
			(void *)&cp, sizeof(cp),
			(void *)&rp, &n) == ERROR)
		return (ERROR);
			
	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	return (OK);
}

static int
le_read_local_supported_features(int s, int argc ,char *argv[])
{
	ng_hci_le_read_local_supported_features_rp rp;
	int n = sizeof(rp);

	union {
		uint64_t raw;
		uint8_t octets[8];
	} le_features;

	char buffer[2048];

	if (hci_simple_request(s,
			NG_HCI_OPCODE(NG_HCI_OGF_LE,
			NG_HCI_OCF_LE_READ_LOCAL_SUPPORTED_FEATURES), 
			(void *)&rp, &n) == ERROR)
		return (ERROR);

	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	le_features.raw = rp.le_features;

	fprintf(stdout, "LE Features: ");
	for(int i = 0; i < 8; i++)
                fprintf(stdout, " %#02x", le_features.octets[i]);
	fprintf(stdout, "\n%s\n", hci_le_features2str(le_features.octets, 
		buffer, sizeof(buffer)));
	fprintf(stdout, "\n");

	return (OK);
}

static int
le_read_supported_states(int s, int argc, char *argv[])
{
	ng_hci_le_read_supported_states_rp rp;
	int n = sizeof(rp);

	if (hci_simple_request(s, NG_HCI_OPCODE(
					NG_HCI_OGF_LE,
					NG_HCI_OCF_LE_READ_SUPPORTED_STATES),
			       		(void *)&rp, &n) == ERROR)
		return (ERROR);

	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	fprintf(stdout, "LE States: %jx\n", rp.le_states);
	
	return (OK); 
}

static int
set_le_event_mask(int s, uint64_t mask)
{
	ng_hci_le_set_event_mask_cp semc;
	ng_hci_le_set_event_mask_rp rp;  
	int i, n;
	
	n = sizeof(rp);
	
	for (i=0; i < NG_HCI_LE_EVENT_MASK_SIZE; i++) {
		semc.event_mask[i] = mask&0xff;
		mask >>= 8;
	}
	if(hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
			NG_HCI_OCF_LE_SET_EVENT_MASK),
			(void *)&semc, sizeof(semc), (void *)&rp, &n) == ERROR)
		return (ERROR);

	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}
	
	return (OK);
}

static int
set_event_mask(int s, uint64_t mask)
{
	ng_hci_set_event_mask_cp semc;
	ng_hci_set_event_mask_rp rp;  
	int i, n;
	
	n = sizeof(rp);
	
	for (i=0; i < NG_HCI_EVENT_MASK_SIZE; i++) {
		semc.event_mask[i] = mask&0xff;
		mask >>= 8;
	}
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_HC_BASEBAND,
			NG_HCI_OCF_SET_EVENT_MASK),
			(void *)&semc, sizeof(semc), (void *)&rp, &n) == ERROR)
		return (ERROR);

	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}
	
	return (OK);
}

static
int le_enable(int s, int argc, char *argv[])
{
        int result;

	if (argc != 1)
		return (USAGE);
	
	if (strcasecmp(argv[0], "enable") == 0) {
		result = set_event_mask(s, NG_HCI_EVENT_MASK_DEFAULT |
			       NG_HCI_EVENT_MASK_LE);
		if (result != OK)
			return result;
		result = set_le_event_mask(s, NG_HCI_LE_EVENT_MASK_ALL);
		if (result == OK) {
			fprintf(stdout, "LE enabled\n"); 
			return (OK);
		} else
			return result;
	} else if (strcasecmp(argv[0], "disable") == 0) {
		result = set_event_mask(s, NG_HCI_EVENT_MASK_DEFAULT);
		if (result == OK) {
			fprintf(stdout, "LE disabled\n"); 
			return (OK);
		} else
			return result;
	} else
		return (USAGE);
}

static int
le_set_advertising_enable(int s, int argc, char *argv[])
{
	ng_hci_le_set_advertise_enable_cp cp;
	ng_hci_le_set_advertise_enable_rp rp;
	int n, enable = 0;

	if (argc != 1)
		return USAGE;
	  
	if (strcmp(argv[0], "enable") == 0)
		enable = 1;
	else if (strcmp(argv[0], "disable") != 0)
		return USAGE;

	n = sizeof(rp);
	cp.advertising_enable = enable;
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
		NG_HCI_OCF_LE_SET_ADVERTISE_ENABLE), 
		(void *)&cp, sizeof(cp), (void *)&rp, &n) == ERROR)
		return (ERROR);
			
	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}
        fprintf(stdout, "LE Advertising %s\n", (enable ? "enabled" : "disabled"));

	return (OK);
}

static int
le_set_advertising_param(int s, int argc, char *argv[])
{
	ng_hci_le_set_advertising_parameters_cp cp;
	ng_hci_le_set_advertising_parameters_rp rp;

	int n, ch;

	cp.advertising_interval_min = 0x800;
	cp.advertising_interval_max = 0x800;
	cp.advertising_type = 0;
	cp.own_address_type = 0;
	cp.direct_address_type = 0;

	cp.advertising_channel_map = 7;
	cp.advertising_filter_policy = 0;

	optreset = 1;
	optind = 0;
	while ((ch = getopt(argc, argv , "m:M:t:o:p:a:c:f:")) != -1) {
		switch(ch) {
		case 'm':
			cp.advertising_interval_min =
				(uint16_t)(strtod(optarg, NULL)/0.625);
			break;
		case 'M':
			cp.advertising_interval_max =
				(uint16_t)(strtod(optarg, NULL)/0.625);
			break;
		case 't':
			cp.advertising_type =
				(uint8_t)strtod(optarg, NULL);
			break;
		case 'o':
			cp.own_address_type =
				(uint8_t)strtod(optarg, NULL);
			break;
		case 'p':
			cp.direct_address_type =
				(uint8_t)strtod(optarg, NULL);
			break;
		case 'a':
			if (!bt_aton(optarg, &cp.direct_address)) {
				struct hostent	*he = NULL;

				if ((he = bt_gethostbyname(optarg)) == NULL)
					return (USAGE);

				memcpy(&cp.direct_address, he->h_addr, sizeof(cp.direct_address));
			}
			break;
		case 'c':
			cp.advertising_channel_map =
				(uint8_t)strtod(optarg, NULL);
			break;
		case 'f':
			cp.advertising_filter_policy =
				(uint8_t)strtod(optarg, NULL);
			break;
		}
	}

	n = sizeof(rp);
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
		NG_HCI_OCF_LE_SET_ADVERTISING_PARAMETERS), 
		(void *)&cp, sizeof(cp), (void *)&rp, &n) == ERROR)
		return (ERROR);
			
	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	return (OK);
}

static int
le_read_advertising_channel_tx_power(int s, int argc, char *argv[])
{
	ng_hci_le_read_advertising_channel_tx_power_rp rp;
	int n;

	n = sizeof(rp);

	if (hci_simple_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
		NG_HCI_OCF_LE_READ_ADVERTISING_CHANNEL_TX_POWER), 
		(void *)&rp, &n) == ERROR)
		return (ERROR);
			
	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

        fprintf(stdout, "Advertising transmit power level: %d dBm\n",
		(int8_t)rp.transmit_power_level);

	return (OK);
}

static int
le_set_advertising_data(int s, int argc, char *argv[])
{
	ng_hci_le_set_advertising_data_cp cp;
	ng_hci_le_set_advertising_data_rp rp;
	int n, len;

	n = sizeof(rp);

	char buf[NG_HCI_ADVERTISING_DATA_SIZE];

	len = sizeof(buf);
	parse_param(argc, argv, buf, &len);
	memset(cp.advertising_data, 0, sizeof(cp.advertising_data));
	cp.advertising_data_length = len;
	memcpy(cp.advertising_data, buf, len);

	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
		NG_HCI_OCF_LE_SET_ADVERTISING_DATA), 
		(void *)&cp, sizeof(cp), (void *)&rp, &n) == ERROR)
		return (ERROR);
			
	if (rp.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.status), rp.status);
		return (FAILED);
	}

	return (OK);
}
static int
le_read_buffer_size(int s, int argc, char *argv[])
{
	union {
		ng_hci_le_read_buffer_size_rp 		v1;
		ng_hci_le_read_buffer_size_rp_v2	v2;
	} rp;

	int n, ch;
	uint8_t v;
	uint16_t cmd;

	optreset = 1;
	optind = 0;

	/* Default to version 1*/
	v = 1;
	cmd = NG_HCI_OCF_LE_READ_BUFFER_SIZE;

	while ((ch = getopt(argc, argv , "v:")) != -1) {
		switch(ch) {
		case 'v':
			v = (uint8_t)strtol(optarg, NULL, 16);	 
			if (v == 2) 
				cmd = NG_HCI_OCF_LE_READ_BUFFER_SIZE_V2;
			else if (v > 2)
				return (USAGE);
			break;
		default:
			v = 1;
		}
	}

	n = sizeof(rp);
	if (hci_simple_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE, cmd), 
		(void *)&rp, &n) == ERROR)
		return (ERROR);
			
	if (rp.v1.status != 0x00) {
		fprintf(stdout, "Status: %s [%#02x]\n", 
			hci_status2str(rp.v1.status), rp.v1.status);
		return (FAILED);
	}

	fprintf(stdout, "ACL data packet length: %d\n",
		rp.v1.hc_le_data_packet_length);
	fprintf(stdout, "Number of ACL data packets: %d\n",
		rp.v1.hc_total_num_le_data_packets);

	if (v == 2) {
		fprintf(stdout, "ISO data packet length: %d\n",
			rp.v2.hc_iso_data_packet_length);
		fprintf(stdout, "Number of ISO data packets: %d\n",
			rp.v2.hc_total_num_iso_data_packets);
	}

	return (OK);
}

static int
le_scan(int s, int argc, char *argv[])
{
	int n, bufsize, scancount, numscans;
	bool verbose;
	uint8_t active = 0;
	char ch;

	char			 b[512];
	ng_hci_event_pkt_t	*e = (ng_hci_event_pkt_t *) b;

	ng_hci_le_set_scan_parameters_cp scan_param_cp;
	ng_hci_le_set_scan_parameters_rp scan_param_rp;

	ng_hci_le_set_scan_enable_cp scan_enable_cp;
	ng_hci_le_set_scan_enable_rp scan_enable_rp;

	optreset = 1;
	optind = 0;
	verbose = false;
	numscans = 1;

	while ((ch = getopt(argc, argv , "an:v")) != -1) {
		switch(ch) {
		case 'a':
			active = 1;
			break;
		case 'n':
			numscans = (uint8_t)strtol(optarg, NULL, 10);
			break;
		case 'v':
			verbose = true;
			break;
		}
	}

	scan_param_cp.le_scan_type = active;
	scan_param_cp.le_scan_interval = (uint16_t)(100/0.625);
	scan_param_cp.le_scan_window = (uint16_t)(50/0.625);
	/* Address type public */
	scan_param_cp.own_address_type = 0;
	/* 'All' filter policy */
	scan_param_cp.scanning_filter_policy = 0;
	n = sizeof(scan_param_rp);

	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
		NG_HCI_OCF_LE_SET_SCAN_PARAMETERS), 
		(void *)&scan_param_cp, sizeof(scan_param_cp),
		(void *)&scan_param_rp, &n) == ERROR)
		return (ERROR);

	if (scan_param_rp.status != 0x00) {
		fprintf(stdout, "LE_Set_Scan_Parameters failed. Status: %s [%#02x]\n", 
			hci_status2str(scan_param_rp.status),
			scan_param_rp.status);
		return (FAILED);
	}

	/* Enable scanning */
	n = sizeof(scan_enable_rp);
	scan_enable_cp.le_scan_enable = 1;
	scan_enable_cp.filter_duplicates = 1;
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
		NG_HCI_OCF_LE_SET_SCAN_ENABLE), 
		(void *)&scan_enable_cp, sizeof(scan_enable_cp),
		(void *)&scan_enable_rp, &n) == ERROR)
		return (ERROR);
			
	if (scan_enable_rp.status != 0x00) {
		fprintf(stdout, "LE_Scan_Enable enable failed. Status: %s [%#02x]\n", 
			hci_status2str(scan_enable_rp.status),
			scan_enable_rp.status);
		return (FAILED);
	}

	scancount = 0;
	while (scancount < numscans) {
		/* wait for scan events */
		bufsize = sizeof(b);
		if (hci_recv(s, b, &bufsize) == ERROR) {
			return (ERROR);
		}

		if (bufsize < sizeof(*e)) {
			errno = EIO;
			return (ERROR);
		}
		scancount++;
		if (e->event == NG_HCI_EVENT_LE) {
		 	fprintf(stdout, "Scan %d\n", scancount);	
			handle_le_event(e, verbose);
		}
	}

	fprintf(stdout, "Scan complete\n");

	/* Disable scanning */
	n = sizeof(scan_enable_rp);
	scan_enable_cp.le_scan_enable = 0;
	if (hci_request(s, NG_HCI_OPCODE(NG_HCI_OGF_LE,
		NG_HCI_OCF_LE_SET_SCAN_ENABLE), 
		(void *)&scan_enable_cp, sizeof(scan_enable_cp),
		(void *)&scan_enable_rp, &n) == ERROR)
		return (ERROR);
			
	if (scan_enable_rp.status != 0x00) {
		fprintf(stdout, "LE_Scan_Enable disable failed. Status: %s [%#02x]\n", 
			hci_status2str(scan_enable_rp.status),
			scan_enable_rp.status);
		return (FAILED);
	}

	return (OK);
}

static void handle_le_event(ng_hci_event_pkt_t* e, bool verbose) 
{
	int rc;
	ng_hci_le_ep	*leer = 
			(ng_hci_le_ep *)(e + 1);
	ng_hci_le_advertising_report_ep *advrep = 
		(ng_hci_le_advertising_report_ep *)(leer + 1); 
	ng_hci_le_advreport	*reports =
		(ng_hci_le_advreport *)(advrep + 1);

	if (leer->subevent_code == NG_HCI_LEEV_ADVREP) {
		fprintf(stdout, "Scan result, num_reports: %d\n",
			advrep->num_reports);
		for(rc = 0; rc < advrep->num_reports; rc++) {
			uint8_t length = (uint8_t)reports[rc].length_data;	
			fprintf(stdout, "\tBD_ADDR %s \n",
				hci_bdaddr2str(&reports[rc].bdaddr));
			fprintf(stdout, "\tAddress type: %s\n",
				hci_addrtype2str(reports[rc].addr_type));
			if (length > 0 && verbose) {
				dump_adv_data(length, reports[rc].data);
				print_adv_data(length, reports[rc].data);
				fprintf(stdout,
					"\tRSSI: %d dBm\n",
					(int8_t)reports[rc].data[length]);
				fprintf(stdout, "\n");
			}
		}
	}
}

struct hci_command le_commands[] = {
{
	"le_enable",
	"le_enable [enable|disable] \n"
	"Enable LE event ",
	&le_enable,
},
  {
	  "le_read_local_supported_features",
	  "le_read_local_supported_features\n" 
	  "read local supported features mask",
	  &le_read_local_supported_features,
  },
  {
	  "le_read_supported_states",
	  "le_read_supported_states\n"
	  "read supported status"	  
	  ,
	  &le_read_supported_states,
  },
  {
	  "le_set_scan_response",
	  "le_set_scan_response -n $name -f $flag -u $uuid16,$uuid16 \n"
	  "set LE scan response data"
	  ,
	  &le_set_scan_response,
  },
  {
	  "le_set_scan_enable",
	  "le_set_scan_enable [enable|disable] \n"
	  "enable or disable LE device scan",
	  &le_set_scan_enable
  },
  {
	  "le_set_scan_param",
	  "le_set_scan_param [active|passive] interval(ms) window(ms) [public|random] [all|whitelist] \n"
	  "set LE device scan parameter",
	  &le_set_scan_param
  },
  {
	  "le_set_advertising_enable",
	  "le_set_advertising_enable [enable|disable] \n"
	  "start or stop advertising",
	  &le_set_advertising_enable
  },
  {
	  "le_read_advertising_channel_tx_power",
	  "le_read_advertising_channel_tx_power\n"
	  "read host advertising transmit poser level (dBm)",
	  &le_read_advertising_channel_tx_power
  },
  {
	  "le_set_advertising_param",
	  "le_set_advertising_param  [-m min_interval(ms)] [-M max_interval(ms)]\n"
	  "[-t advertising_type] [-o own_address_type] [-p peer_address_type]\n"
	  "[-c advertising_channel_map] [-f advertising_filter_policy]\n"
	  "[-a peer_address]\n"
	  "set LE device advertising parameters",
	  &le_set_advertising_param
  },
  {
	  "le_set_advertising_data",
	  "le_set_advertising_data -n $name -f $flag -u $uuid16,$uuid16 \n"
	  "set LE device advertising packed data",
	  &le_set_advertising_data
  },
  {
	  "le_read_buffer_size",
	  "le_read_buffer_size [-v 1|2]\n"
	  "Read the maximum size of ACL and ISO data packets",
	  &le_read_buffer_size
  },
  {
	  "le_scan",
	  "le_scan [-a] [-v] [-n number_of_scans]\n"
	  "Do an LE scan",
	  &le_scan
  },
};
