/*
 * Created: 23 August 2015
 * Name...: csrstat.c
 * Author.: Pike R. Alpha
 * Edited.: 10 November 2018
 * Author.: Joss Brown
 * Purpose: Command line tool for El Capitan and greater to get the active SIP status.
 *
 * Compile with: cc csrstat.c -o csrstat
 *
 * Updates:
 *			-added full flags to output
 *			-added csrutil arguments to output
 *			-added CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE (xnu-4903.221.2)
 *
 * See also: https://github.com/opensource-apple/xnu/blob/master/bsd/sys/csr.h
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <strings.h>

typedef uint32_t csr_config_t;

char *text = NULL;
double gVersion = 2.0;
csr_config_t config = 0;

/* Rootless configuration flags */
#define CSR_ALLOW_UNTRUSTED_KEXTS		(1 << 0)	// 1
#define CSR_ALLOW_UNRESTRICTED_FS		(1 << 1)	// 2
#define CSR_ALLOW_TASK_FOR_PID			(1 << 2)	// 4
#define CSR_ALLOW_KERNEL_DEBUGGER		(1 << 3)	// 8
#define CSR_ALLOW_APPLE_INTERNAL		(1 << 4)	// 16
#define CSR_ALLOW_UNRESTRICTED_DTRACE	(1 << 5)	// 32
#define CSR_ALLOW_UNRESTRICTED_NVRAM	(1 << 6)	// 64
#define CSR_ALLOW_DEVICE_CONFIGURATION	(1 << 7)	// 128
#define CSR_ALLOW_ANY_RECOVERY_OS		(1 << 8)	// 256
#define CSR_ALLOW_UNAPPROVED_KEXTS		(1 << 9)	// 512
#define CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE	(1 << 10)	// 1024

#define CSR_VALID_FLAGS (CSR_ALLOW_UNTRUSTED_KEXTS | \
	CSR_ALLOW_UNRESTRICTED_FS | \
	CSR_ALLOW_TASK_FOR_PID | \
	CSR_ALLOW_KERNEL_DEBUGGER | \
	CSR_ALLOW_APPLE_INTERNAL | \
	CSR_ALLOW_UNRESTRICTED_DTRACE | \
	CSR_ALLOW_UNRESTRICTED_NVRAM  | \
	CSR_ALLOW_DEVICE_CONFIGURATION | \
	CSR_ALLOW_ANY_RECOVERY_OS | \
	CSR_ALLOW_UNAPPROVED_KEXTS | \
	CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE)

/* Syscalls */
extern int csr_get_active_config(csr_config_t *config);

//==============================================================================

char * _csr_check(int aMask, bool aFlipflag)
{
	bool stat = false;
	bool bit = (config & aMask);

	if (aFlipflag)
	{
		if (bit)
		{
			sprintf(text, "%d (disabled)", bit);
		}
		else
		{
			sprintf(text, "%d (enabled)", bit);
		}
	}
	else
	{
		if (bit)
		{
			sprintf(text, "%d (enabled)", bit);
		}
		else
		{
			sprintf(text, "%d (disabled)", bit);
		}
	}

	return text;
}

//==============================================================================

int main(int argc, const char * argv[])
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	text = malloc(12);
	bzero(text, 12);
	// Syscall
	csr_get_active_config(&config);

	printf("csrstat v%.1f Copyright (c) 2015-2017 by Pike R. Alpha, 2017-%d by Joss Brown\n", gVersion, (tm.tm_year + 1900));
	//
	// Note: boot.efi is no longer using 0x67 but 0x77 for csrutil disabled!!!
	//
	printf("System Integrity Protection status: %s (0x%08x) ", (config == CSR_VALID_FLAGS) ? "\33[1mdisabled\33[0m": "enabled", config);

	if (config)
	{
		if (config == CSR_ALLOW_APPLE_INTERNAL)
		{
			printf("(Apple Internal).");
		}
		else
		{
			printf("(Custom Configuration).");
		}
	}

	printf("\n\nCurrent Configuration:\n");

	printf("\tApple Internal\t\t\t%s\t[--no-internal]\t\tCSR_ALLOW_APPLE_INTERNAL\n", _csr_check(CSR_ALLOW_APPLE_INTERNAL, 0));
	printf("\tKext Signing\t\t\t%s\t[--without kext]\tCSR_ALLOW_UNTRUSTED_KEXTS\n", _csr_check(CSR_ALLOW_UNTRUSTED_KEXTS, 1));
	printf("\tDebugging Restrictions\t\t%s\t[--without debug]\tCSR_ALLOW_TASK_FOR_PID\n", _csr_check(CSR_ALLOW_TASK_FOR_PID, 1));
	printf("\tFilesystem Protections\t\t%s\t[--without fs]\t\tCSR_ALLOW_UNRESTRICTED_FS\n", _csr_check(CSR_ALLOW_UNRESTRICTED_FS, 1));
	printf("\tKernel Debugging Restrictions\t%s\t<n/a>\t\t\tCSR_ALLOW_KERNEL_DEBUGGER\n", _csr_check(CSR_ALLOW_KERNEL_DEBUGGER, 1));
	printf("\tDTrace Restrictions\t\t%s\t[--without dtrace]\tCSR_ALLOW_UNRESTRICTED_DTRACE\n", _csr_check(CSR_ALLOW_UNRESTRICTED_DTRACE, 1));
	printf("\tNVRAM Protections\t\t%s\t[--without nvram]\tCSR_ALLOW_UNRESTRICTED_NVRAM\n", _csr_check(CSR_ALLOW_UNRESTRICTED_NVRAM, 1));
	printf("\tDevice Configuration\t\t%s\t<n/a>\t\t\tCSR_ALLOW_DEVICE_CONFIGURATION\n", _csr_check(CSR_ALLOW_DEVICE_CONFIGURATION, 0));
	printf("\tBaseSystem Verification\t\t%s\t[--without basesystem]\tCSR_ALLOW_ANY_RECOVERY_OS\n", _csr_check(CSR_ALLOW_ANY_RECOVERY_OS, 1));
	printf("\tUnapproved Kexts Restrictions\t%s\t<n/a>\t\t\tCSR_ALLOW_UNAPPROVED_KEXTS\n", _csr_check(CSR_ALLOW_UNAPPROVED_KEXTS, 1));
	printf("\tExecutable Policy\t\t%s\t<n/a>\t\t\tCSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE\n", _csr_check(CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE, 1));
	printf("\nBoot into Recovery Mode and modify with: 'csrutil enable [arguments]'\n");
	printf("<Note: some flags are not accessible using the csrutil CLI.>\n");

	if (config && (config != CSR_ALLOW_APPLE_INTERNAL))
	{
		printf("\nThis is an unsupported configuration, likely to break in the future and leave your machine in an unknown state.\n");
	}
	if (text)
	{
		free(text);
	}
	exit(-1);
}
