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
#include <ipmitool/log.h>

#include "lanplus.h"

int csv_output = 0;
int verbose = 0;

#define STDOUT_TEMPLATE (".stdout.XXXXXX")
#define STDOUT_F_LN (sizeof(char)*strlen(STDOUT_TEMPLATE))
#define STDERR_TEMPLATE (".stderr.XXXXXX")
#define STDERR_F_LN (sizeof(char)*strlen(STDERR_TEMPLATE))

#if !defined(__minlen)
# define __minlen(a, b) ({ int x=strlen(a); int y=strlen(b); (x < y) ? x : y;})
#endif

#if !defined(__maxlen)
# define __maxlen(a, b) ({ int x=strlen(a); int y=strlen(b); (x > y) ? x : y;})
#endif

/*
 * @brief The global ipmi command list
 *        matches ipmitool/src/ipmitool.c
 */
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

/*
 * @brief Wrapper to initialize extended logger
 */
void initLog()
{
    log_init("libipmi", 0, verbose);
}

/*
 * @brief Wrapper to load the specified interface name
 */
void *intfLoad( char * name )
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

/*
 * @brief Wrapper to open the loaded interface
 */
int intfOpen( struct ipmi_intf *intf ) 
{
    intf->my_addr = IPMI_BMC_SLAVE_ADDR;
    if (intf->open != NULL) {
        if (intf->open(intf) < 0) {
            return -1;
        }
    }
    return 0;
}

/*
 * @brief Wrapper to set the session host (ip address)
 */
int intfSessionSetHostname( void * intf, char * host )
{
    if (!intf || !host)
        return -1;
	ipmi_intf_session_set_hostname(intf, host);
	return 0;
}

/*
 * @brief Wrapper to set the session username
 */
int intfSessionSetUsername( void * intf, char * username )
{
	if (!intf || !username)
        return -1;
	ipmi_intf_session_set_username(intf, username);
	return 0;
}

/*
 * @brief Wrapper to set the session password
 */
int intfSessionSetPassword( void * intf, char * password )
{
    if (!intf || !password)
        return -1;
	ipmi_intf_session_set_password(intf, password);
	return 0;
}

/*
 * @brief Wrapper to set the session privilage level
 */
int intfSessionSetPrvLvl( void * intf, unsigned char lvl )
{
    ipmi_intf_session_set_privlvl(intf, lvl);
    return 0;
}

/*
 * @brief Wrapper to set the session lookup bit parameter
 */
int intfSessionSetLookupBit( void * intf, unsigned char lubit )
{
    ipmi_intf_session_set_lookupbit(intf, lubit);
    return 0;
}

/*
 * @brief Wrapper to set the session cipher suite ID
 */
int intfSessionSetCipherSuiteID( void * intf, unsigned char cid )
{
    ipmi_intf_session_set_cipher_suite_id(intf, cid);
    return 0;
}

/*
 * @brief Wrapper to set the session's SOL escape charactor
 */
int intfSessionSetSOLEscChar( void * intf, char ch )
{
    ipmi_intf_session_set_sol_escape_char(intf, ch);
    return 0;
}

/*
 * @brief Wrapper to set the session timeout
 */
int intfSessionSetTimeout( void * intf, int timeout )
{
    ipmi_intf_session_set_timeout(intf, timeout);
    return 0;
}

/*
 * @brief Wrapper to get interfaces username
 */
#define MAX_USER_STR_LN 17
int getUserName( void *intf, int id, char *buf )
{
	struct user_name_t un;
	memset(&un, 0, sizeof(struct user_name_t));
	un.user_id = id;

	if (_ipmi_get_user_name(intf, &un) == 0) {
		memset(buf, 0, MAX_USER_STR_LN);
		strcpy(buf,un.user_name);
		return strlen(un.user_name);
	}
	return -1;
}

/*
 * @brief Cleanup interface session and close the device
 */
int finishInterface( struct ipmi_intf *intf )
{
    if (!intf)
        return -1;
    
    ipmi_cleanup(intf);
    if (intf->opened > 0 && intf->close != NULL)
        intf->close(intf);
	return 0;
}

/*
 * @brief Lookup the ipmi command in the global command table
 */
struct ipmi_cmd *ipmiCmdLookup( const char *name )
{
    struct ipmi_cmd *cmd = ipmitool_cmd_list;
    while (cmd) { 
        if (cmd->func) 
            if (0 == strncmp(name, cmd->name, __maxlen(cmd->name,name)))
                return cmd;
        cmd++;
    }
    return NULL;
}
    
/*
 * @brief Check and deallocate output buffer
 */
void freeOutBuf( char *buf )
{
    if (buf) {
        free(buf);
        buf = NULL;
    }
}

/*
 * @brief Create output buffer with size of len
 */
char *createOutBuf( const size_t len )
{
    char *buffer = (char*)malloc(len);
    if (!buffer)
        printf("error creating output buffer (%s)\n",
            strerror(errno));
    return buffer;
}

/*
 * @brief Wrapper to unlink/remove a file
 */
#undef PRESERVE_TMPFILE
int funlink( const char *fn )
{
#if !defined(PRESERVE_TMPFILE)
    return unlink(fn);    
#else
    return 0;
#endif
}

/*
 * @brief Wrapper to handle call and execution of a specified ipmi command
 *        enables control of the command input and interface session.
 */
int runCommand(struct ipmi_intf *intf, 
               int argc, 
               char **argv, 
               char **ppBuf, 
               int *length )
{
    FILE *outfp = NULL, *errfp = NULL;
    size_t outfilesz, errfilesz;
    fpos_t outpos, errpos;
    struct ipmi_cmd *cmd = NULL;
    int stdout_sv = dup(fileno(stdout)),
        stderr_sv = dup(fileno(stderr)),
        stdout_fd, stderr_fd, 
        argflag,
        argc_sv = 0, rv = 0;
    char **argv_sv;
    char *buf = NULL;
    
    char outfn[STDOUT_F_LN], errfn[STDERR_F_LN];
    strncpy(outfn, STDOUT_TEMPLATE, STDOUT_F_LN);
    strncpy(errfn, STDERR_TEMPLATE, STDERR_F_LN);

    /* redirect stdout and stderr */
    stdout_fd = mkostemp(outfn, O_CREAT|O_SYNC);
    if (0 > stdout_fd) {
        printf("error opening stdout (%s)\n",
            strerror(errno));
        return -1;
    }
    outfp = fdopen(stdout_fd, "w+");

    fflush(stdout);
    fgetpos(stdout, &outpos);
    if (0 > dup2(stdout_fd, fileno(stdout))) {
        printf("error redirecting stdout (%s)\n",
            strerror(errno));
        fclose(outfp);
        return -1;
    }

    stderr_fd = mkostemp(errfn, O_CREAT|O_SYNC);
    if (0 > stderr_fd) {
        printf("error opening stderr (%s)\n",
            strerror(errno));
        fclose(outfp);
        return -1;
    }
    errfp = fdopen(stderr_fd, "w+");

    fflush(stderr);
    fgetpos(stderr, &outpos);
    if (0 > dup2(stderr_fd, fileno(stderr))) {
        printf("error redirecting stderr (%s)\n",
            strerror(errno));
        fclose(outfp);
        fclose(errfp);
        return -1;
    }
    
    /* handle special command options */
    optind = 0;
    verbose = 0;
    csv_output = 0;
    while ((argflag = getopt(argc, (char **)argv, "vc")) != -1) {
        switch (argflag) {
            case 'v': verbose++; break;
            case 'c': csv_output = 1; break;
            default: 
                printf("invalid option %s\n", argv[optind]);
                rv = -1;
                goto cleanup;
        }
    }

    /* verify the interface and setup the command */
    if (!intf) {
        printf("invalid interface\n");
        rv = -1;
        goto cleanup;
    }

    initLog();
    intf->cmdlist = ipmitool_cmd_list;
    
    /* handle command args and lookup the command entry */
    argv_sv = argv + optind;
    argc_sv = argc - optind;

    cmd = ipmiCmdLookup(argv_sv[0]);
    if (!cmd) {
        printf("command entry lookup error %s\n",
            argv_sv[0]);
        rv = -1;
        goto cleanup;
    }
    
    /* exec the command */
    rv = cmd->func(intf,argc_sv-1,argv_sv+1);
    if (0 > rv) {
        printf("error executing %s command\n", 
            cmd->name);
        goto cleanup;
    }

    /* get the redirected stdout and stderr size */
    fseek(outfp, 0, SEEK_END);
    outfilesz = ftell(outfp);
    fseek(outfp, 0, SEEK_SET);

    fseek(errfp, 0, SEEK_END);
    errfilesz = ftell(errfp);
    fseek(errfp, 0, SEEK_SET);

    if (!(outfilesz+errfilesz)) {
        printf("redirect file size was zero\n");
        rv = -1;
        goto cleanup;
    }

    /* generate an output buffer and copy the redirected stdout and stderr output */
    buf = (char*)createOutBuf(sizeof(char)*outfilesz+errfilesz);
    if (!buf) {
        printf("error creating output buffer (%s)\n",
            strerror(errno));
        rv = -1;
        goto cleanup;
    }

    if (outfilesz != fread(buf, 1, outfilesz, outfp)) {
        printf("error reading stdout with size %lu\n", 
            outfilesz);
        freeOutBuf(buf);
        rv = -1;
        goto cleanup;
    }
    if (errfilesz != fread(buf+outfilesz, 1, errfilesz , errfp)) {
        printf("error reading stderr with size %lu\n", 
            errfilesz);
        freeOutBuf(buf);
        rv = -1;
        goto cleanup;
    }
    if (buf)
        buf[outfilesz+errfilesz] = '\0';

    *ppBuf = buf;
    *length = strlen(buf);

cleanup:

    /* restore stdout and stderr */
    dup2(stdout_sv, fileno(stdout));
    dup2(stderr_sv, fileno(stderr));
    close(stdout_sv);
    close(stderr_sv);
    fflush(stdout);
    fflush(stderr);
    clearerr(stdout);
    clearerr(stderr);
    fsetpos(stdout, &outpos);
    fsetpos(stderr, &errpos);
    fclose(outfp);
    fclose(errfp);
    funlink(outfn);
    funlink(errfn);

    return rv;
}