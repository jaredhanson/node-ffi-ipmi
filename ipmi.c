#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "ipmitool/ipmi_chassis.h"
#include "ipmitool/ipmi_user.h"
#include "lanplus.h"

int csv_output = 0;
int verbose = 0;

#define OK 1

extern struct ipmi_intf * ipmi_intf_load(char * name);

// Basic test stuff

void *intf_load(char* name)
{
	struct ipmi_intf *intf = NULL;

        if (!name) return NULL;
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
	if (!intf || !host) return -1;
	printf("interface: 0x%lx\n", (long unsigned int)intf);
	ipmi_intf_session_set_hostname(intf, host);
	return OK;
}


int intf_session_set_username(void* intf,char* username)
{
	if (!intf || !username) return -1;
	ipmi_intf_session_set_username(intf, username);
	return OK;
}


int intf_session_set_password(void* intf,char* password)
{
	if (!intf || !password) return -1;
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
