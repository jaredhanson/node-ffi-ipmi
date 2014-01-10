#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <ipmitool/ipmi.h>
#include <ipmitool/ipmi_intf.h>
#include <ipmitool/ipmi_main.h>

#include <ipmitool/ipmi_sdr.h>
#include <ipmitool/ipmi_gendev.h>
#include <ipmitool/ipmi_sel.h>
#include <ipmitool/ipmi_fru.h>
#include <ipmitool/ipmi_sol.h>
#include <ipmitool/ipmi_isol.h>
#include <ipmitool/ipmi_tsol.h>
#include <ipmitool/ipmi_lanp.h>
#include <ipmitool/ipmi_chassis.h>
#include <ipmitool/ipmi_mc.h>
#include <ipmitool/ipmi_sensor.h>
#include <ipmitool/ipmi_channel.h>
#include <ipmitool/ipmi_session.h>
#include <ipmitool/ipmi_event.h>
#include <ipmitool/ipmi_user.h>
#include <ipmitool/ipmi_raw.h>
#include <ipmitool/ipmi_pef.h>
#include <ipmitool/ipmi_oem.h>
#include <ipmitool/ipmi_sunoem.h>
#include <ipmitool/ipmi_fwum.h>
#include <ipmitool/ipmi_picmg.h>
#include <ipmitool/ipmi_kontronoem.h>
#include <ipmitool/ipmi_firewall.h>
#include <ipmitool/ipmi_hpmfwupg.h>
#include <ipmitool/ipmi_delloem.h>
#include <ipmitool/ipmi_ekanalyzer.h>
#include <ipmitool/ipmi_ime.h>
#include <ipmitool/ipmi_dcmi.h>

#include "lanplus.h"

// Bob: These are referened as global in ipmitool lib
int csv_output = 0;
int verbose = 0;

// Bob: This file ipmi.c has to exist to be compiled into 
// shared lib .so because the ipmitool lib is static.
// ipmitool autoconf is not 100% well and it's not
// immediately obvious when attempting to create a .so
// Besides, we need additional C level wrapper to
// do more sane API with additional error checking.

#define OK 1

extern struct ipmi_intf * ipmi_intf_load(char * name);

struct ipmi_intf * ipmi_start_interface(char *intf_name, char *host, char *user, char *pass);
extern int ipmi_finish_interface(struct ipmi_intf *intf);
extern int ipmi_run_command(struct ipmi_intf *intf, int argc, char *argv[]);

// Bob: from ipmitool.c
//    ipmi lib requires it and we also have it here to extend ipmi tool command sets (if needed).


#if 0 // Bob: temporarily disabled
extern int ipmi_echo_main(struct ipmi_intf * intf, int argc, char ** argv);
extern int ipmi_set_main(struct ipmi_intf * intf, int argc, char ** argv);
extern int ipmi_exec_main(struct ipmi_intf * intf, int argc, char ** argv);
#endif
struct ipmi_cmd ipmitool_cmd_list[] = {
	{ ipmi_raw_main,     "raw",     "Send a RAW IPMI request and print response" },
	{ ipmi_rawi2c_main,  "i2c",     "Send an I2C Master Write-Read command and print response" },
	{ ipmi_rawspd_main,  "spd",     "Print SPD info from remote I2C device" },
	{ ipmi_lanp_main,    "lan",     "Configure LAN Channels" },
	{ ipmi_chassis_main, "chassis", "Get chassis status and set power state" },
	{ ipmi_power_main,   "power",   "Shortcut to chassis power commands" },
	{ ipmi_event_main,   "event",   "Send pre-defined events to MC" },
	{ ipmi_mc_main,      "mc",      "Management Controller status and global enables" },
	{ ipmi_mc_main,      "bmc",     NULL },	/* for backwards compatibility */
	{ ipmi_sdr_main,     "sdr",     "Print Sensor Data Repository entries and readings" },
	{ ipmi_sensor_main,  "sensor",  "Print detailed sensor information" },
	{ ipmi_fru_main,     "fru",     "Print built-in FRU and scan SDR for FRU locators" },
	{ ipmi_gendev_main,  "gendev",  "Read/Write Device associated with Generic Device locators sdr" },
	{ ipmi_sel_main,     "sel",     "Print System Event Log (SEL)" },
	{ ipmi_pef_main,     "pef",     "Configure Platform Event Filtering (PEF)" },
	{ ipmi_sol_main,     "sol",     "Configure and connect IPMIv2.0 Serial-over-LAN" },
	{ ipmi_tsol_main,    "tsol",    "Configure and connect with Tyan IPMIv1.5 Serial-over-LAN" },
	{ ipmi_isol_main,    "isol",    "Configure IPMIv1.5 Serial-over-LAN" },
	{ ipmi_user_main,    "user",    "Configure Management Controller users" },
	{ ipmi_channel_main, "channel", "Configure Management Controller channels" },
	{ ipmi_session_main, "session", "Print session information" },
    { ipmi_dcmi_main,    "dcmi",    "Data Center Management Interface"},
	{ ipmi_sunoem_main,  "sunoem",  "OEM Commands for Sun servers" },
	{ ipmi_kontronoem_main, "kontronoem", "OEM Commands for Kontron devices"},
	{ ipmi_picmg_main,   "picmg",   "Run a PICMG/ATCA extended cmd"},
	{ ipmi_fwum_main,    "fwum",	"Update IPMC using Kontron OEM Firmware Update Manager" },
	{ ipmi_firewall_main,"firewall","Configure Firmware Firewall" },
	{ ipmi_delloem_main, "delloem", "OEM Commands for Dell systems" },
#if 0 // Bob: temporarily disabled
#ifdef HAVE_READLINE
	{ ipmi_shell_main,   "shell",   "Launch interactive IPMI shell" },
#endif
	{ ipmi_exec_main,    "exec",    "Run list of commands from file" },
	{ ipmi_set_main,     "set",     "Set runtime variable for shell and exec" },
	{ ipmi_echo_main,    "echo",    NULL }, /* for echoing lines to stdout in scripts */
#endif
	{ ipmi_hpmfwupg_main,"hpm", "Update HPM components using PICMG HPM.1 file"},
	{ ipmi_ekanalyzer_main,"ekanalyzer", "run FRU-Ekeying analyzer using FRU files"},
	{ ipmi_ime_main,          "ime", "Update Intel Manageability Engine Firmware"},
	{ NULL },
};

// Basic test stuff

void *intf_load(char* name)
{
	struct ipmi_intf *intf = NULL;

        if (!name)
                return NULL;
        intf = ipmi_intf_load(name);

	if (!intf) {
		printf("Can't get interface for '%s'\n",name);
		return NULL;
	}

	printf("returning interface 0x%lx\n",(long unsigned int)intf);

	return (void*)intf;
}

// hard coded API examples
// Do additional sanity checks in C code
// return OK (1) or negative number for error
// JS side should deal with what to do based on errors

int intf_session_set_hostname(void* intf,char* host)
{
	if (!intf || !host)
                return -1;

	printf("interface: 0x%lx\n", (long unsigned int)intf);

	ipmi_intf_session_set_hostname(intf, host);

	return OK;
}


int intf_session_set_username(void* intf,char* username)
{
	if (!intf || !username)
                return -1;

	ipmi_intf_session_set_username(intf, username);

	return OK;
}


int intf_session_set_password(void* intf,char* password)
{
	if (!intf || !password)
                return -1;

	ipmi_intf_session_set_password(intf, password);

	return OK;
}

int chassis_power_status(void* intf)
{
	if (!intf) return -1;
	return  ipmi_chassis_power_status(intf);
}

int get_user_name(void *intf, int id, char *buf)
{
	char name[200];

	name[0] = '\0';

	if (ipmi_get_user_name(intf, id, name) == 0) {
		printf("ipmi get user name returns 0 , '%s'\n", name);
		memset(buf, 0, 200);
		strcpy(buf,name);
		return strlen(name);
	}

	printf("ipmi get user name error\n");

	return -1;
}

// just to test argv[] passing between C and node via FFI
int test_argv(void *intf, int argc, char *argv[])
{
	int i;

	printf("argc %d\n",argc);

	for (i = 0; i < argc; i++)
		printf("argv[%d] : %s\n", i, argv[i]);

	printf("%d\n", atoi(argv[2]));

	return OK;
}



struct ipmi_intf * start_interface(char* intf_name, char *host, char *user, char *pass)
{
	if (!intf_name || !host || !user || !pass)
                return NULL;

	return  ipmi_start_interface(intf_name, host, user, pass);
}


int finish_interface(struct ipmi_intf *intf)
{
        if (!intf)
                return -1;

        return ipmi_finish_interface(intf);
}

int run_command_argv(struct ipmi_intf *intf, int argc, char *argv[])
{
        if (!intf)
                return -1;

        return ipmi_run_command(intf, argc,argv);
}

