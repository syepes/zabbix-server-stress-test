/*
** Zabbix module for stress testing
** Copyright (C) 2001-2015 Jan Garaj - www.jangaraj.com
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include "sysinc.h"
#include "module.h"
#include "common.h"

/* the variable keeps timeout setting for item processing */
static int	item_timeout = 0;

int	zbx_module_stress_ping(AGENT_REQUEST *request, AGENT_RESULT *result);
int	zbx_module_stress_echo(AGENT_REQUEST *request, AGENT_RESULT *result);
int	zbx_module_stress_file(AGENT_REQUEST *request, AGENT_RESULT *result);
int	zbx_module_stress_random(AGENT_REQUEST *request, AGENT_RESULT *result);
int	zbx_module_stress_random_int(AGENT_REQUEST *request, AGENT_RESULT *result);
int	zbx_module_stress_random_double(AGENT_REQUEST *request, AGENT_RESULT *result);
int	zbx_module_stress_random_str(AGENT_REQUEST *request, AGENT_RESULT *result);
int	zbx_module_stress_random_txt(AGENT_REQUEST *request, AGENT_RESULT *result);
void	rand_str(char *dest, size_t length);

static ZBX_METRIC keys[] =
/*      KEY                     FLAG		FUNCTION        	TEST PARAMETERS */
{
	{"stress.ping",		CF_HAVEPARAMS,	zbx_module_stress_ping,"anything"},
	{"stress.echo",		CF_HAVEPARAMS,	zbx_module_stress_echo,"a message"},
	{"stress.file",		CF_HAVEPARAMS,	zbx_module_stress_file,"anything"},
	{"stress.random",	CF_HAVEPARAMS,	zbx_module_stress_random,"1,1000"},
	{"stress.random.int",	CF_HAVEPARAMS,	zbx_module_stress_random_int,"anything"},
	{"stress.random.double",	CF_HAVEPARAMS,	zbx_module_stress_random_double,"anything"},
	{"stress.random.str",	CF_HAVEPARAMS,	zbx_module_stress_random_str,"anything"},
	{"stress.random.txt",	CF_HAVEPARAMS,	zbx_module_stress_random_txt,"anything"},
	{NULL}
};

/******************************************************************************
 *                                                                            *
 * Function: zbx_module_api_version                                           *
 *                                                                            *
 * Purpose: returns version number of the module interface                    *
 *                                                                            *
 * Return value: ZBX_MODULE_API_VERSION_ONE - the only version supported by   *
 *               Zabbix currently                                             *
 *                                                                            *
 ******************************************************************************/
int	zbx_module_api_version()
{
	return ZBX_MODULE_API_VERSION_ONE;
}

/******************************************************************************
 *                                                                            *
 * Function: zbx_module_item_timeout                                          *
 *                                                                            *
 * Purpose: set timeout value for processing of items                         *
 *                                                                            *
 * Parameters: timeout - timeout in seconds, 0 - no timeout set               *
 *                                                                            *
 ******************************************************************************/
void	zbx_module_item_timeout(int timeout)
{
	item_timeout = timeout;
}

/******************************************************************************
 *                                                                            *
 * Function: zbx_module_item_list                                             *
 *                                                                            *
 * Purpose: returns list of item keys supported by the module                 *
 *                                                                            *
 * Return value: list of item keys                                            *
 *                                                                            *
 ******************************************************************************/
ZBX_METRIC	*zbx_module_item_list()
{
	return keys;
}

int	zbx_module_stress_ping(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	SET_UI64_RESULT(result, 1);

	return SYSINFO_RET_OK;
}

int	zbx_module_stress_echo(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	char	*param;

	if (1 != request->nparam)
	{
		/* set optional error message */
		SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
		return SYSINFO_RET_FAIL;
	}

	param = get_rparam(request, 0);

	SET_STR_RESULT(result, strdup(param));

	return SYSINFO_RET_OK;
}

// Checks if file "/tmp/stress_file" exists (unsigned)
int	zbx_module_stress_file(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	zbx_stat_t	buf;

	if (0 != zbx_stat("/tmp/stress_file", &buf)) {
		SET_UI64_RESULT(result, 0);
	} else {
		SET_UI64_RESULT(result, 1);
	}

	return SYSINFO_RET_OK;
}

/******************************************************************************
 *                                                                            *
 * Function: zbx_module_stress_random                                          *
 *                                                                            *
 * Purpose: a main entry point for processing of an item                      *
 *                                                                            *
 * Parameters: request - structure that contains item key and parameters      *
 *              request->key - item key without parameters                    *
 *              request->nparam - number of parameters                        *
 *              request->timeout - processing should not take longer than     *
 *                                 this number of seconds                     *
 *              request->params[N-1] - pointers to item key parameters        *
 *                                                                            *
 *             result - structure that will contain result                    *
 *                                                                            *
 * Return value: SYSINFO_RET_FAIL - function failed, item will be marked      *
 *                                 as not supported by zabbix                 *
 *               SYSINFO_RET_OK - success                                     *
 *                                                                            *
 * Comment: get_rparam(request, N-1) can be used to get a pointer to the Nth  *
 *          parameter starting from 0 (first parameter). Make sure it exists  *
 *          by checking value of request->nparam.                             *
 *                                                                            *
 ******************************************************************************/
int	zbx_module_stress_random(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	char	*param1, *param2;
	int	from, to;

	if (2 != request->nparam)
	{
		/* set optional error message */
		SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
		return SYSINFO_RET_FAIL;
	}

	param1 = get_rparam(request, 0);
	param2 = get_rparam(request, 1);

	/* there is no strict validation of parameters for simplicity sake */
	from = atoi(param1);
	to = atoi(param2);

	if (from > to)
	{
		SET_MSG_RESULT(result, strdup("Invalid range specified."));
		return SYSINFO_RET_FAIL;
	}

	SET_UI64_RESULT(result, from + rand() % (to - from + 1));

	return SYSINFO_RET_OK;
}

// Random Numetric (unsigned)
int	zbx_module_stress_random_int(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	int	from, to;
	from = 0;
	to = 2000;

	SET_UI64_RESULT(result, from + rand() % (to - from + 1));

	return SYSINFO_RET_OK;
}

// Random Numetric (float)
int	zbx_module_stress_random_double(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	double	from, to;
	from = 1.0;
	to = 2000.0;

	SET_DBL_RESULT(result, from + rand() / (to - from + 1));

	return SYSINFO_RET_OK;
}

// Random Character
int	zbx_module_stress_random_str(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	char str[250];
	rand_str(str, sizeof str - 1);

	SET_STR_RESULT(result, strdup(str)); // Max length 255

	return SYSINFO_RET_OK;
}

// Random Text
int	zbx_module_stress_random_txt(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	char str[507];
	rand_str(str, sizeof str - 1);

	SET_TEXT_RESULT(result, strdup(str)); // Max length DB Dependent

	return SYSINFO_RET_OK;
}


/******************************************************************************
 *                                                                            *
 * Function: zbx_module_init                                                  *
 *                                                                            *
 * Purpose: the function is called on agent startup                           *
 *          It should be used to call any initialization routines             *
 *                                                                            *
 * Return value: ZBX_MODULE_OK - success                                      *
 *               ZBX_MODULE_FAIL - module initialization failed               *
 *                                                                            *
 * Comment: the module won't be loaded in case of ZBX_MODULE_FAIL             *
 *                                                                            *
 ******************************************************************************/
int	zbx_module_init()
{
	/* initialization for stress.random */
	srand(time(NULL));

	return ZBX_MODULE_OK;
}

/******************************************************************************
 *                                                                            *
 * Function: zbx_module_uninit                                                *
 *                                                                            *
 * Purpose: the function is called on agent shutdown                          *
 *          It should be used to cleanup used resources if there are any      *
 *                                                                            *
 * Return value: ZBX_MODULE_OK - success                                      *
 *               ZBX_MODULE_FAIL - function failed                            *
 *                                                                            *
 ******************************************************************************/
int	zbx_module_uninit()
{
	return ZBX_MODULE_OK;
}

// Generate random strings
void rand_str(char *dest, size_t length) {
	char charset[] = "0123456789"
	"abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	while (length-- > 0) {
		size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
		*dest++ = charset[index];
	}
	*dest = '\0';
}
