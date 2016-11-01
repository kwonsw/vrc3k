/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 *  Command Processor Table
 */

#include <stdio.h>
#include <string.h>

#include "command.h"

#undef getc
#define getc()      uart_getc()
#undef putc
#define putc(ch)    xputc(ch)
#define puts(ch)    xputs(ch)
#undef printf
#define printf		xprintf

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

int _do_help (cmd_tbl_t *cmd_start, int cmd_items, cmd_tbl_t * cmdtp, int
	      flag, int argc, char *argv[])
{
	int i;
	int rcode = 0;

	if (argc == 1) {	/*show list of commands */
		cmd_tbl_t *cmd_array[cmd_items];
		int i, j, swaps;

		/* Make array of commands from .shell_cmd section */
		cmdtp = cmd_start;
		for (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		for (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			for (j = 0; j < i; ++j) {
				if (strcmp (cmd_array[j]->name,
					    cmd_array[j + 1]->name) > 0) {
					cmd_tbl_t *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			if (!swaps)
				break;
		}

		/* print short help (usage) */
		for (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			if (ctrlc ())
				return 1;
			if (usage == NULL)
				continue;
#if 0
			printf("%-*s- %s\n", CONFIG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
#else
			{
				char* cmd_name = cmd_array[i]->name;
				int cmd_name_len = strlen(cmd_name);
				int blank_len = (cmd_name_len < CONFIG_SYS_HELP_CMD_WIDTH)?  (CONFIG_SYS_HELP_CMD_WIDTH - cmd_name_len):0;
				while(cmd_name_len--) putc(*cmd_name++);
				while(blank_len--) putc(' ');
				puts("- ");
				puts(usage);
				putc('\n');
			}
#endif
		}
		return 0;
	}
	/*
	 * command help (long version)
	 */
	for (i = 1; i < argc; ++i) {
		if ((cmdtp = find_cmd_tbl (argv[i], cmd_start, cmd_items )) != NULL) {
			rcode |= cmd_usage(cmdtp);
		} else {
			printf ("Unknown command '%s' - try 'help'"
				" without arguments for list of all"
				" known commands\n\n", argv[i]
					);
			rcode = 1;
		}
	}
	return rcode;
}

/***************************************************************************
 * find command table entry for a command
 */
cmd_tbl_t *find_cmd_tbl (const char *cmd, cmd_tbl_t *table, int table_len)
{
	cmd_tbl_t *cmdtp;
	cmd_tbl_t *cmdtp_temp = table;	/*Init value */
	const char *p;
	int len;
	int n_found = 0;

	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	for (cmdtp = table;
	     cmdtp != table + table_len;
	     cmdtp++) {
		if (strncmp (cmd, cmdtp->name, len) == 0) {
			if (len == strlen (cmdtp->name))
				return cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	if (n_found == 1) {			/* exactly one match */
		return cmdtp_temp;
	}

	return NULL;	/* not found or ambiguous command */
}

cmd_tbl_t *find_cmd (const char *cmd)
{
	int len = &__shell_cmd_end - &__shell_cmd_start;
	return find_cmd_tbl(cmd, &__shell_cmd_start, len);
}

int cmd_usage(cmd_tbl_t *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	if (!cmdtp->help) {
		puts ("- No additional help available.\n");
		return 1;
	}

	puts (cmdtp->help);
	putc ('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	return 0;
}

#ifdef CONFIG_AUTO_COMPLETE

static void install_auto_complete_handler(const char *cmd,
		int (*complete)(int argc, char *argv[], char last_char, int maxv, char *cmdv[]))
{
	cmd_tbl_t *cmdtp;

	cmdtp = find_cmd(cmd);
	if (cmdtp == NULL)
		return;

	cmdtp->complete = complete;
}

void install_auto_complete(void)
{
}

/*************************************************************************************/

static int complete_cmdv(int argc, char *argv[], char last_char, int maxv, char *cmdv[])
{
	cmd_tbl_t *cmdtp;
	const char *p;
	int len, clen;
	int n_found = 0;
	const char *cmd;

	/* sanity? */
	if (maxv < 2)
		return -2;

	cmdv[0] = NULL;

	if (argc == 0) {
		/* output full list of commands */
		for (cmdtp = &__shell_cmd_start; cmdtp != &__shell_cmd_end; cmdtp++) {
			if (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				break;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		return n_found;
	}

	/* more than one arg or one but the start of the next */
	if (argc > 1 || (last_char == '\0' || last_char == ' ' || last_char == '\t')) {
		cmdtp = find_cmd(argv[0]);
		if (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			return 0;
		}
		return (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	if (p == NULL)
		len = strlen(cmd);
	else
		len = p - cmd;

	/* return the partial matches */
	for (cmdtp = &__shell_cmd_start; cmdtp != &__shell_cmd_end; cmdtp++) {

		clen = strlen(cmdtp->name);
		if (clen < len)
			continue;

		if (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		if (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			break;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	return n_found;
}

static int make_argv(char *s, int argvsz, char *argv[])
{
	int argc = 0;

	/* split into argv */
	while (argc < argvsz - 1) {

		/* skip any white space */
		while ((*s == ' ') || (*s == '\t'))
			++s;

		if (*s == '\0')	/* end of s, no more args	*/
			break;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		while (*s && (*s != ' ') && (*s != '\t'))
			++s;

		if (*s == '\0')		/* end of s, no more args	*/
			break;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	return argc;
}

static void print_argv(const char *banner, const char *leader, const char *sep, int linemax, char *argv[])
{
	int ll = leader != NULL ? strlen(leader) : 0;
	int sl = sep != NULL ? strlen(sep) : 0;
	int len, i;

	if (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	while (*argv != NULL) {
		len = strlen(*argv) + sl;
		if (i + len >= linemax) {
			puts("\n");
			if (leader)
				puts(leader);
			i = ll - sl;
		} else if (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static int find_common_prefix(char *argv[])
{
	int i, len;
	char *anchor, *s, *t;

	if (*argv == NULL)
		return 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	while ((t = *argv++) != NULL) {
		s = anchor;
		for (i = 0; i < len; i++, t++, s++) {
			if (*t != *s)
				break;
		}
		len = s - anchor;
	}
	return len;
}

static char tmp_buf[CONFIG_SYS_CBSIZE];	/* copy of console I/O buffer	*/

int cmd_auto_complete(const char *const prompt, char *buf, int *np, int *colp)
{
	int n = *np, col = *colp;
	char *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	char *cmdv[20];
	char *s, *t;
	const char *sep;
	int i, j, k, len, seplen, argc;
	int cnt;
	char last_char;

	if (strcmp(prompt, CONFIG_SYS_PROMPT) != 0)
		return 0;	/* not in normal console */

	cnt = strlen(buf);
	if (cnt >= 1)
		last_char = buf[cnt - 1];
	else
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, sizeof(argv)/sizeof(argv[0]), argv);

	/* do the completion and return the possible completions */
	i = complete_cmdv(argc, argv, last_char, sizeof(cmdv)/sizeof(cmdv[0]), cmdv);

	/* no match; bell and out */
	if (i == 0) {
		if (argc > 1)	/* allow tab for non command */
			return 0;
		putc('\a');
		return 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	if (i == 1) { /* one match; perfect */
		k = strlen(argv[argc - 1]);
		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} else if (i > 1 && (j = find_common_prefix(cmdv)) != 0) {	/* more */
		k = strlen(argv[argc - 1]);
		j -= k;
		if (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	if (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		if (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			return 1;
		}

		t = buf + cnt;
		for (i = 0; i < len; i++)
			*t++ = *s++;
		if (sep != NULL)
			for (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		if (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} else {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	return 1;
}

#endif

#ifdef CMD_DATA_SIZE
int cmd_get_data_size(char* arg, int default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	int len = strlen(arg);
	if (len > 2 && arg[len-2] == '.') {
		switch(arg[len-1]) {
		case 'b':
			return 1;
		case 'w':
			return 2;
		case 'l':
			return 4;
		case 's':
			return -2;
		default:
			return -1;
		}
	}
	return default_size;
}
#endif

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
					? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}  
	if (endp)
		*endp = (char *)cp;
	return result;
}

long simple_strtol(const char *cp,char **endp,unsigned int base)
{
	if(*cp=='-')
		return -simple_strtoul(cp+1,endp,base);
	return simple_strtoul(cp,endp,base);
}

/* --------------------------------------------------------------------------- */
int do_help(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	return _do_help(&__shell_cmd_start,
			&__shell_cmd_end - &__shell_cmd_start,
			cmdtp, flag, argc, argv);
}

SHELL_CMD(
	help,   CONFIG_SYS_MAXARGS, 1,  do_help,
	"print command description/usage",
	"\n"
	"   - print brief description of all commands\n"
	"help command ...\n"
	"   - print detailed usage of 'command'"
);

/* This does not use the U_BOOT_CMD macro as ? can't be used in symbol names */
cmd_tbl_t __shell_cmd_question_mark Struct_Section = {
	"?",    CONFIG_SYS_MAXARGS, 1,  do_help,
	"alias for 'help'",
#ifdef  CONFIG_SYS_LONGHELP
	""
#endif /* CONFIG_SYS_LONGHELP */
};
