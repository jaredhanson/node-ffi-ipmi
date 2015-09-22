#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

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

int csv_output = 0;
int verbose = 0;

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
	{ ipmi_hpmfwupg_main,"hpm", "Update HPM components using PICMG HPM.1 file"},
	{ ipmi_ekanalyzer_main,"ekanalyzer", "run FRU-Ekeying analyzer using FRU files"},
	{ ipmi_ime_main,     "ime", "Update Intel Manageability Engine Firmware"},
	{ NULL },
};

void *intf_load(char* name)
{
	struct ipmi_intf *intf = NULL;
    if (!name)
        return NULL;
        
	intf = ipmi_intf_load(name);
	if (!intf) {
		printf("no interface for %s'\n",name);
		return NULL;
	}
	return (void*)intf;
}

int intf_session_set_hostname(void* intf,char* host)
{
	if (!intf || !host)
        return -1;
	ipmi_intf_session_set_hostname(intf, host);
	return 0;
}

int intf_session_set_username(void* intf,char* username)
{
	if (!intf || !username)
        return -1;
	ipmi_intf_session_set_username(intf, username);
	return 0;
}

int intf_session_set_password(void* intf,char* password)
{
	if (!intf || !password)
        return -1;
	ipmi_intf_session_set_password(intf, password);
	return 0;
}

int get_user_name(void *intf, int id, char *buf)
{
	struct user_name_t un;
	memset(&un, 0, sizeof(struct user_name_t));
	un.user_id = id;

	if (_ipmi_get_user_name(intf, &un) == 0) {
		memset(buf, 0, 17);
		strcpy(buf,un.user_name);
		return strlen(un.user_name);
	}
	return -1;
}

int finish_interface(struct ipmi_intf *intf)
{
    if (!intf)
        return -1;
    ipmi_cleanup(intf);
	return 0;
}

struct ipmi_cmd *ipmicmd_lookup( const char *name )
{
    int i;
	for (i = 0; i < sizeof(ipmitool_cmd_list) / 
                    sizeof(ipmitool_cmd_list[0]); i++) {
		if (NULL != ipmitool_cmd_list[i].name) {
			if (!strncmp(name, 
                         ipmitool_cmd_list[i].name, 
                         strlen(ipmitool_cmd_list[i].name))) 
				return ((struct ipmi_cmd*)&ipmitool_cmd_list[i]);
		}
	}
	return NULL;
}

void free_out_buffer( unsigned char *buf )
{
    if (buf)
        free(buf);
}

unsigned char *create_out_buffer( const size_t len )
{
    unsigned char *buffer = (unsigned char*)malloc(len);
    if (!buffer)
        printf("error creating output buffer (%s)\n",
            strerror(errno));
    return buffer;
}

#define F_TEMPLATE    \
    ("libipmi.XXXXXX")
#define F_TEMPLATE_LN \
    (sizeof(char)*strlen(F_TEMPLATE))

unsigned char * run_command(struct ipmi_intf *intf, int argc, char **argv)
{
    FILE *fp = NULL;
    size_t filesz;
    fpos_t pos;
	struct ipmi_cmd *cmd = NULL;
    unsigned char *buf = NULL;
    int stdout_sv = dup(fileno(stdout)),
        tmpfd, 
        i,
        argc_sv = 0;
    char *argv_sv[256];
    char tmpfn[F_TEMPLATE_LN];
    strncpy(tmpfn, F_TEMPLATE, F_TEMPLATE_LN);
    
    if (!intf) {
        printf("invalid interface\n");
        return NULL;
	}

    for (i=0; i<argc; i++) {
        /* handle special options */
        if (argv[i][0] == '-') { 
            switch (argv[i][1]) { 
                case 'c': csv_output = 1; break;
                case 'v': verbose = 1; break;
                default: break;
            }
        /* save the valid command args */
        } else { 
            argv_sv[argc_sv] = argv[i];
            struct ipmi_cmd *entry = ipmicmd_lookup(argv[i]);
            if (entry)
                cmd = entry;
            argc_sv++;
        }
    }
    if (!cmd) {
        printf("command entry lookup error\n");
        return NULL;
    }

    tmpfd = mkostemp(tmpfn, O_CREAT|O_SYNC);
    if (0 > tmpfd) {
        printf("error opening tmpfile (%s)\n",
            strerror(errno));
        return NULL;
    }
    fp = fdopen(tmpfd, "w+");

    fflush(stdout);
    fgetpos(stdout, &pos);
    if (0 > dup2(tmpfd, fileno(stdout))) {
        printf("error redirecting stdout (%s)\n",
            strerror(errno));
        fclose(fp);
        unlink(tmpfn);
        return NULL;
    }
    
    /* exec the command */
    if (0 > cmd->func(intf,argc_sv-1,argv_sv+1)) {
        printf("error executing command %s\n", 
            cmd->name);
        fclose(fp);
        unlink(tmpfn);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    filesz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (!filesz) {
        printf("file size was zero\n");
        fclose(fp);
        unlink(tmpfn);
        return NULL;
    }

    buf = (char*)create_out_buffer(sizeof(char)*filesz);
    if (!buf) {
        printf("error creating output buffer (%s)\n",
            strerror(errno));
        fclose(fp);
        unlink(tmpfn);
        return NULL;
    }

    if (filesz != fread(buf, 1, filesz, fp)) {
        printf("error reading buffer with size %lu\n", 
            filesz);
        buf = NULL;
    }
    if (buf)
        buf[filesz] = '\0';

    fflush(stdout);
    dup2(stdout_sv, fileno(stdout));
    close(stdout_sv);
    clearerr(stdout);
    fsetpos(stdout, &pos);
    fclose(fp);
    unlink(tmpfn);
    return buf;
}




