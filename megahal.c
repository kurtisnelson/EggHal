/*===========================================================================*/
/*
 *  Copyright (C) 1998-1999 Jason Hutchens
 *  Copyright (C) 2000      Steve Huston
 *  Copyright (C) 2001      SegFault
 *  Copyright (C) 2002-2005 BarkerJr
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the license or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the Gnu Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*===========================================================================*/

/*
 *		Please note, that I don't know much about licensing and legalities.  So
 *		far as I know, what I'm doing is fine; but if someone cares to point out
 *		my shortsightedness, feel free :P
 *
 *		This is a minor rewrite of the original megahal.c and megahal.h files
 *		to compile them in as a module for Eggdrop.  Guarantees: None,
 *		Warranties: none, etc.  Use at your own risk, if it makes your hdd
 *		become possessed, I don't care, yadda yadda yadda.  I'm not big on
 *		comments, but I'll try to note where changes were made.  In order to
 *		keep this as simple as possible, I'm leaving all previous comments and
 *		remarks in the code.
 *		
 *		        Steve Huston (huston@elvis.rowan.edu)
 */

/*
 *		Edited by BarkerJr 2005-04-16
 *			<http://barkerjr.net>
 */

/*		Edited by Johan Segernas 2002-10-22
 *		Changes commented with 'sege'
 *			<http://sege.nu>
 */

/*
 *		$Id: megahal.c,v 1.25 1999/10/21 03:42:48 hutch Exp hutch $
 *
 *		File:			megahal.c
 *
 *		Program:		MegaHAL v8r6
 *
 *		Purpose:		To simulate a natural language conversation with a psychotic
 *						computer.  This is achieved by learning from the user's
 *						input using a third-order Markov model on the word level.
 *						Words are considered to be sequences of characters separated
 *						by whitespace and punctuation.  Replies are generated
 *						randomly based on a keyword, and they are scored using
 *						measures of surprise.
 *
 *		Author:		Mr. Jason L. Hutchens
 *
 *		WWW:			http://ciips.ee.uwa.edu.au/~hutch/hal/
 *
 *		E-Mail:		hutch@ciips.ee.uwa.edu.au
 *
 *		Contact:		The Centre for Intelligent Information Processing Systems
 *						Department of Electrical and Electronic Engineering
 *						The University of Western Australia
 *						AUSTRALIA 6907
 *
 *		Phone:		+61-8-9380-3856
 *
 *		Facsimile:	+61-8-9380-1168
 *
 *		Notes:		- This file is best viewed with tabstops set to three spaces.
 *						- To the Debian guys, yes, it's only one file, so shoot me!
 *						  I had to get it to work on DOS with crappy compilers and
 *						  I didn't want to spend more time than was neccessary.
 *						  Hence it's rather monolithic.  Also, an email would be
 *						  appreciated whenever bugs were fixed/discovered.  I've
 *						  terminated all of the memory leakage bugs AFAICT.  But
 *						  it does allocate a helluva lot of memory, I'll admit!
 *
 *		Compilation Notes
 *		=================
 *
 *		When compiling, be sure to link with the maths library so that the
 *		log() function can be found.
 *
 *		On the Macintosh, add the library SpeechLib to your project.  It is
 *		very important that you set the attributes to Import Weak.  You can
 *		do this by selecting the lib and then use Project Inspector from the
 *		Window menu.
 *
 *		CREDITS
 *		=======
 *
 *		Amiga (AmigaOS)
 *		---------------
 *		Dag Agren (dagren@ra.abo.fi)
 *
 *		DEC (OSF)
 *		---------
 *		Jason Hutchens (hutch@ciips.ee.uwa.edu.au)
 *
 *		Macintosh
 *		---------
 *		Paul Baxter (pbaxter@assistivetech.com)
 *		Doug Turner (dturner@best.com)
 *
 *		PC (Linux - Debian package)
 *		---------------------------
 *		Joey Hess (joeyh@master.debian.org)
 *
 *		PC (OS/2)
 *		---------
 *		Bjorn Karlowsky (?)
 *
 *		PC (Windows 3.11)
 *		-----------------
 *		Jim Crawford (pfister_@hotmail.com)
 *
 *		PC (Windows '95)
 *		----------------
 *		Jason Hutchens (hutch@ciips.ee.uwa.edu.au)
 *
 *		PPC (Linux)
 *		-----------
 *		Lucas Vergnettes (Lucasv@sdf.lonestar.org)
 *
 *		SGI (Irix)
 *		----------
 *		Jason Hutchens (hutch@ciips.ee.uwa.edu.au)
 *
 *		Sun (SunOS)
 *		-----------
 *		Jason Hutchens (hutch@ciips.ee.uwa.edu.au)
 */
/*===========================================================================*/

#define MAKING_MEGAHAL
#define MODULE_NAME "megahal"
#include <stdlib.h> 
/* megahal preproc directives */
#include <stdio.h>
#include <stdarg.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <sys/malloc.h>
#endif
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <ctype.h> 
#include <sys/types.h> 
#include "megahal.h"
#include <time.h>
/* End megahal preproc directives */
#include "../module.h"
#include "../channels.mod/channels.h"
#include "../irc.mod/irc.h"
#include "../server.mod/server.h"

#undef global

static void add_aux(MODEL *, DICTIONARY *, STRING);
static void add_key(MODEL *, DICTIONARY *, STRING);
static void add_node(TREE *, TREE *, int);
static TREE *add_symbol(TREE *, BYTE2);
static int babble(MODEL *, DICTIONARY *, DICTIONARY *);
static bool boundary(char *, int);
static void capitalize(char *);
static void change_personality(DICTIONARY *, int, MODEL **);
static bool dissimilar(DICTIONARY *, DICTIONARY *);
static float evaluate_reply(MODEL *, DICTIONARY *, DICTIONARY *);
static TREE *find_symbol(TREE *, int);
static TREE *find_symbol_add(TREE *, int);
static void free_tree(TREE *);
static void free_word(STRING);
static void free_words(DICTIONARY *);
static char *generate_reply(MODEL *, DICTIONARY *);
static void initialize_context(MODEL *);
static void learn(MODEL *, DICTIONARY *);
static void load_word(FILE *, DICTIONARY *);
static DICTIONARY *make_keywords(MODEL *, DICTIONARY *);
static char *make_output(DICTIONARY *);
static void make_words(char *, DICTIONARY *);
static DICTIONARY *new_dictionary(void);
static DICTIONARY *reply(MODEL *, DICTIONARY *);
static void save_model(char *, MODEL *);
static void save_tree(FILE *, TREE *);
static void save_word(FILE *, STRING);
static int search_dictionary(DICTIONARY *, STRING, bool *);
static int search_node(TREE *, int, bool *);
static int seed(MODEL *, DICTIONARY *);
static void upper(char *);
static int wordcmp(STRING, STRING);
static bool word_exists(DICTIONARY *, STRING);
static int rnd(int);

static int order=5, timeout=2;
static bool used_key;
static DICTIONARY *ban=NULL, *aux=NULL, *grt=NULL, *words=NULL;
static SWAP *swp=NULL;
static char *directory=NULL, *last=NULL;
static MODEL *model=NULL;

static Function *global = NULL;
static Function *server_funcs = NULL;
static Function *irc_funcs = NULL;
static Function *channels_funcs = NULL;
static void megahal_hook_userfile(void);
static int mem=0, wpm=120, quiet_save=0, backup_brain=1, auto_save=1, save_ctr=1;
static char mega_file_name[13];

typedef struct theq
{
  int idx;
  char *text;
  struct theq *next;
} QUEUE;

static QUEUE *delQueue(QUEUE *);
static QUEUE *theq;
int _myrand = 0;

static int megahal_expmem()
{
  return mem;
}

/*
 * Next, a hacked attempt at strdup(), since I can't use malloc() anywhere.
 */

static char *mystrdup(const char *s) {
	char *mytmp = nmalloc(strlen(s)+1);

	if (mytmp==NULL) return NULL;
	else strcpy(mytmp, s);

	return mytmp;
}

#if EGG_IS_MAX_VER(10615)
/*
 * Copied from Eggdrop's src/dcc.c
 * Copyright (C) 1997 Robey Pointer
 * Copyright (C) 1999, 2000, 2001, 2002, 2003 Eggheads Development Team
 */
static void strip_mirc_codes(int flags, char *text)
{
  char *dd = text;

  while (*text) {
    switch (*text) {
    case 2:                    /* Bold text */
      if (flags & STRIP_BOLD) {
        text++;
        continue;
      }
      break;
    case 3:                    /* mIRC colors? */
      if (flags & STRIP_COLOR) {
        if (isdigit(text[1])) { /* Is the first char a number? */
          text += 2;            /* Skip over the ^C and the first digit */
          if (isdigit(*text))
            text++;             /* Is this a double digit number? */
          if (*text == ',') {   /* Do we have a background color next? */
            if (isdigit(text[1]))
              text += 2;        /* Skip over the first background digit */
            if (isdigit(*text))
              text++;           /* Is it a double digit? */
          }
        } else
          text++;
        continue;
      }
      break;
    case 7:
      if (flags & STRIP_BELLS) {
        text++;
        continue;
      }
      break;
    case 0x16:                 /* Reverse video */
      if (flags & STRIP_REV) {
        text++;
        continue;
      }
      break;
    case 0x1f:                 /* Underlined text */
      if (flags & STRIP_UNDER) {
        text++;
        continue;
      }
      break;
    case 033:
      if (flags & STRIP_ANSI) {
        text++;
        if (*text == '[') {
          text++;
          while ((*text == ';') || isdigit(*text))
            text++;
          if (*text)
            text++;             /* also kill the following char */
        }
        continue;
      }
      break;
    }
    *dd++ = *text++;            /* Move on to the next char */
  }
  *dd = 0;
}
#endif

	/* Here it is, the attempt at MegaHAL for Eggdrop bots... */
static void do_megahal(int idx, char *prefix, char *text, char *channel)
{
  /* Make a copy of text so we don't mess up the original */
  char buff[strlen(text) + 1];

  /* Is there anything to parse? */
  if (!text[0]) {
    dprintf(idx, "%sSo tell me something already.\n", prefix);
    return;
  }

  strcpy(buff, text);
  text = buff;
  strip_mirc_codes(STRIP_BOLD | STRIP_REV | STRIP_UNDER | STRIP_COLOR, text);

  putlog(LOG_DEBUG, "*", "MegaHAL: learning: %s", text);

	upper(text);
	make_words(text, words);
	learn(model, words);
	if (idx)
	{
		QUEUE *q;
		char *halreply = generate_reply(model, words);

		capitalize(halreply);

		/* if wpm is zero or less, spit out the text right now */
		if (wpm < 1)
		{
			dprintf(idx, "%s%s\n", prefix, halreply);
			return;
		}

		if (theq)
		{
			q = theq;
			while (q->next) q = q->next;
			if ((q->next = (QUEUE *)nmalloc(sizeof(QUEUE))) == NULL)
			{
				putlog("MegaHAL: Could not allocate memory to reply.");
				return;
			}
			q = q->next;
		}
		else
		{
			if ((theq = (QUEUE *)nmalloc(sizeof(QUEUE))) == NULL)
			{
				putlog("MegaHAL: Could not allocate memory to reply.");
				return;
			}
			q = theq;
		}
		mem += sizeof(QUEUE);

		q->idx = idx;
		q->next = NULL;
		if ((q->text = (char *)nmalloc(strlen(prefix)+strlen(halreply)+1)) == NULL)
		{
			putlog("MegaHAL: Could not allocate memory to reply.");
			return;
		}
		mem += strlen(prefix) + strlen(halreply) + 1;
		sprintf(q->text, "%s%s", prefix, halreply);
	}
}

static int pubm_megahal(const char *nick, const char *host, const char *hand,
	char *channel, char *text)
{
  /* reply if the name is  the first word in a phrase */
  if (ngetudef("megahal", channel) && (strlen(text) > strlen(botname)) &&
      !egg_strncasecmp(botname, text, strlen(botname)))
  {
    struct chanset_t *chan = findchan_by_dname(channel);
    /* channel length + nick length plus 13 characters */
    char prefix[strlen(chan->name) + strlen(nick) + 13], *p;

    /* sege: Delete starting blankspace */
    p = &text[strlen(botname)+1];
    if (*p == ' ') p++;

    sprintf(prefix, "PRIVMSG %s :", chan->name);
    do_megahal(DP_HELP, prefix, p, channel);
  }
  /* reply if the name is NOT the first word in a phrase */
  else if (ngetudef("megahal", channel) && strstr (text, botname))
  {
    struct chanset_t *chan = findchan_by_dname(channel);
    /* channel length + nick length plus 13 characters */
    char prefix[strlen(chan->name) + strlen(nick) + 13];

    sprintf(prefix, "PRIVMSG %s :", chan->name);
    do_megahal(DP_HELP, prefix, text, channel);
  }
  /* endof (reply if the name is NOT the first word in a phrase) */
  /* reply randomly */
  else if (ngetudef ("freespeak", channel) && (_myrand == 1)
	   && (!strstr (text, botname)))
  {
    struct chanset_t *chan = findchan_by_dname(channel);
    /* channel length + nick length plus 13 characters */
    char prefix[strlen(chan->name) + strlen(nick) + 13];

    sprintf(prefix, "PRIVMSG %s :", chan->name);
    do_megahal(DP_HELP, prefix, text, channel);
  }
  /* endof (reply randomly) */
  else if (ngetudef("learnall", channel))
  {           
    char *p = &text[0];
    struct chanset_t *chan = findchan_by_dname(channel);
    memberlist *m = chan->channel.member;

    for (; m && m->nick[0]; m = m->next)
      if ((strlen(text) > strlen(m->nick)) &&
        !egg_strncasecmp(m->nick, text, strlen(m->nick)))
      {
        p = &text[strlen(m->nick)+1];
        if (*p == ' ') p++;
        break;
      }

    do_megahal(0, NULL, p, channel);
  }

  /* generate a random number for random replies
     even though the name is not included */
  if (ngetudef ("freespeak", channel))
  {
    srand ((unsigned) time (NULL));
    _myrand = rand ();
    _myrand = _myrand % 2;
  }

  return 0;
}

static int msg_brainsave(const char *nick, const char *host, struct userrec *u,
	char *text)
{
	char *pass = newsplit(&text);

	/* Did they enter a password at all? */
	if (!*pass)
	{
		putlog(LOG_CMDS, "*", "(%s!%s) !%s! failed BRAINSAVE", nick,
			host, u->handle);
		if (!quiet_reject)
			dprintf(DP_HELP, "NOTICE %s :Syntax: /msg %s %s", nick,
				botname, "BRAINSAVE <password>\n");
		return 0;
	}

	/* Is there no password set for that handle? */
	if (u_pass_match(u, "-"))
	{
		putlog(LOG_CMDS, "*", "(%s!%s) !%s! failed BRAINSAVE", nick,
			host, u->handle);
		if (!quiet_reject)
			dprintf(DP_HELP, "NOTICE %s :%s\n", nick, IRC_NOPASS);
		return 0;
	}

	/* Is the password correct? */
	if (u_pass_match(u, pass))
	{
		putlog(LOG_CMDS, "*", "(%s!%s) !%s! BRAINSAVE", nick, host,
			u->handle);
		dprintf(DP_HELP, "NOTICE %s :Saving brain file...\n", nick);
		megahal_hook_userfile();
		return 0;
	}

	/* Bad Password */
	putlog(LOG_CMDS, "*", "(%s!%s) !%s! failed BRAINSAVE", nick, host,
		u->handle);
	if (!quiet_reject)
		dprintf(DP_HELP, "NOTICE %s :%s\n", nick, IRC_FAILPASS);
	return 0;
}

static int msgm_megahal(const char *nick, const char *host,
	const struct userrec *u, char *text)
{
	/* nick length plus 13 characters for this prefix */
	char prefix[strlen(nick) + 12];

	if (!egg_strncasecmp(botname, text, strlen(botname)))
	{
		sprintf(prefix, "PRIVMSG %s : ", nick);
		do_megahal(DP_HELP, prefix, &text[strlen(botname)+1], (char *)NULL);
	}

	return 0;
}


static int dcc_megahal(const struct userrec *u, const int idx, char *par)
{
	do_megahal(idx, "", par, (char *)NULL);
	return 0;
} 

static int dcc_brainsave(const struct userrec *u, const int idx, const char *par)
{
	putlog(LOG_CMDS, "*", "#%s# brainsave", u->handle);
	megahal_hook_userfile();
	return 0;
}

static int tcl_learn STDVAR
{
  BADARGS(2, 2, " text");

  do_megahal(0, NULL, argv[1], (char *)NULL);

  return TCL_OK;
}

static int tcl_getreply STDVAR
{
  char *halreply;

  BADARGS(2, 2, " text");

  upper(argv[1]);
  make_words(argv[1], words);
  halreply = generate_reply(model, words);
  capitalize(halreply);
  Tcl_SetObjResult(irp, Tcl_NewStringObj(halreply, -1));

  return TCL_OK;
}

/* a report on the module status */
static void megahal_report(const int idx, const int details)
{
  if (details)
  {
    unsigned char ctr=0;
    if (theq)
    {
      QUEUE *q=theq;
      for (ctr=1; q->next; q=q->next) ctr++;
    }
    dprintf(idx, "%d %s in queue taking up %d bytes of memory\n", ctr,
      ((ctr == 1)? "line": "lines"), mem);
  }
}

static cmd_t mega_dcc[] =
{
  {"megahal",	"",	dcc_megahal,	"megahal"},
  {"brainsave",	"m",	dcc_brainsave,	"brainsave"},
  {NULL,	NULL,	NULL,		NULL}
};

static cmd_t mega_msg[] =
{
  {"brainsave",	"m",	msg_brainsave,	"brainsave"},
  {NULL,	NULL,	NULL,		NULL}
};

static cmd_t mega_pubm[] =
{
  {"*",		"",	pubm_megahal,	"megahal"},
  {NULL,	NULL,	NULL,		NULL}
};

static cmd_t mega_msgm[] =
{
  {"*",		"",	msgm_megahal,	"megahal"},
  {NULL,	NULL,	NULL,		NULL}
};

static tcl_cmds mega_cmds[] =
{
  {"learn",	tcl_learn},
  {"getreply",	tcl_getreply},
  {NULL,	NULL}
};

static tcl_ints mega_ints[] =
{
  {"mega_backup_brain",	&backup_brain,	0},
  {"mega_auto_save",	&auto_save,	0},
  {"mega_wpm",		&wpm,		0},
  {"quiet-save",	&quiet_save,	0},
  {NULL,		NULL,		0}
};

static tcl_strings mega_strings[] = {
  {"mega_file_name",	mega_file_name,	12,	0},
  {NULL,		NULL,		0,	0}
};

static int server_megahal_setup(char *mod)
{
	module_entry *me;

	if ((me = module_find("server", 1, 0)))
	{
		module_depend(MODULE_NAME, "server", 1, 0);
		server_funcs = me->funcs;
		add_builtins(H_msg, mega_msg);
		add_builtins(H_msgm, mega_msgm);
	}
	return 0;
}

static int irc_megahal_setup(char *mod)
{
	module_entry *me;

	if ((me = module_find("irc", 1, 0)))
	{
		module_depend(MODULE_NAME, "irc", 1, 0);
		irc_funcs = me->funcs;
		add_builtins(H_pubm, mega_pubm);
	}
	return 0;
}

static int channels_megahal_setup(char *mod)
{
  module_entry *me = module_find("channels", 1, 0);

  if (me)
  {
    module_depend(MODULE_NAME, "channels", 1, 0);
    channels_funcs = me->funcs;
    initudef(UDEF_FLAG, "megahal", 1);
    initudef(UDEF_FLAG, "learnall", 1);
    initudef(UDEF_FLAG, "freespeak", 1);
  }

  return 0;
}

static cmd_t megahal_load[] = 
{
	{"server",	"",	server_megahal_setup,	"megahal:server"},
	{"irc",		"",	irc_megahal_setup,	"megahal:irc"},
	{"channels",	"",	channels_megahal_setup,	"megahal:channels"},
	{NULL,		NULL,	NULL,			NULL}
};

static void megahal_hook_userfile(void)
{
  if (auto_save && !save_ctr)
  {
    if (!quiet_save) putlog(LOG_MISC, "*", "Writing brain file...");
    save_model(mega_file_name, model);
    save_ctr = auto_save;
  }
  else if (save_ctr > 0) save_ctr--;
  else save_ctr = auto_save;
}

static void megahal_hook_backup(void)
{
  if (backup_brain)
  {
    char bakfile[13];
    putlog(LOG_MISC, "*", "Backing up brain file...");
    strcpy(bakfile, mega_file_name);
    strcpy(&bakfile[strlen(bakfile) - 3], "bak");
    copyfile(mega_file_name, bakfile);
  }
}

static void megahal_hook_secondly(void)
{
  static char sec2say=-1;

  if (!theq) return;

  if (!sec2say || (wpm < 1))
  {
    if (theq->text) dprintf(theq->idx, "%s\n", theq->text);
    theq = delQueue(theq);
    sec2say = -1;
  }
  else
    if (sec2say == -1)
    {
      if (theq)
      {
        char *p, wordcount=0;
        for (p=theq->text; *p; p++)
          if (*p == ' ') wordcount++;
        sec2say = 60 * wordcount / wpm;
      }
    }
    else sec2say--;
}

static QUEUE *delQueue(QUEUE *queue)
{
  QUEUE *next = queue->next;
  if (queue->text != NULL)
  {
    mem -= strlen(queue->text);
    nfree(queue->text);
  }
  mem -= sizeof(QUEUE);
  nfree(queue);
  return next;
}

static char *megahal_close()
{
  while (theq) theq = delQueue(theq);
  megahal_hook_userfile();
  rem_builtins(H_load, megahal_load);
  rem_builtins(H_dcc, mega_dcc);
  rem_builtins(H_msg, mega_msg);
  rem_builtins(H_pubm, mega_pubm);
  rem_builtins(H_msgm, mega_msgm);
  del_hook(HOOK_USERFILE, (Function)megahal_hook_userfile);
  del_hook(HOOK_BACKUP, (Function)megahal_hook_backup);
  del_hook(HOOK_SECONDLY, (Function)megahal_hook_secondly);
  rem_tcl_commands(mega_cmds);
  rem_tcl_ints(mega_ints);
  rem_tcl_strings(mega_strings);
  module_undepend(MODULE_NAME);
  return NULL;
}

char *megahal_start();

static Function megahal_table[] =
{
  (Function) megahal_start,
  (Function) megahal_close,
  (Function) megahal_expmem,
  (Function) megahal_report,
};

char *megahal_start(Function * global_funcs)
{
	global = global_funcs;
	strcpy(mega_file_name, "megahal.brn");
	module_register(MODULE_NAME, megahal_table, 2, 7);
	if (!module_depend(MODULE_NAME, "eggdrop", 106, 5)) {
		module_undepend(MODULE_NAME);
		return "This module requires Eggdrop 1.6.5 or later.";
	}
	add_builtins(H_load, megahal_load);
	add_builtins(H_dcc, mega_dcc);
	server_megahal_setup(0);
	irc_megahal_setup(0);
	channels_megahal_setup(0);
	words=new_dictionary();

	/*
	 * Load the default personality.
	 */
	change_personality(NULL, 0, &model);
	add_hook(HOOK_USERFILE, (Function)megahal_hook_userfile);
	add_hook(HOOK_BACKUP, (Function)megahal_hook_backup);
	add_hook(HOOK_SECONDLY, (Function)megahal_hook_secondly);
	add_tcl_commands(mega_cmds);
	add_tcl_ints(mega_ints);
	add_tcl_strings(mega_strings);

	return NULL;
}

/*
 * The remainder of this code was just pasted from the rest of megahal.c, and
 * modified where necessary to get messages put in the right place, etc.
 */

/*---------------------------------------------------------------------------*/
/*
 *		Function:	Error
 *
 *		Purpose:		Print the specified message to the error file.
 */
static void error(char *title, char *fmt, ...)
{
	va_list argp;
	char stuff[512];

	sprintf(stuff, "%s: ", title);
	va_start(argp, fmt);
	vsprintf(stuff, fmt, argp);
	va_end(argp);
	sprintf(stuff, ".\n");
	putlog(LOG_MISC, "*", "%s", stuff);
 	dprintf("PRIVMESG #lugatgt: My brain exploded. kurtisnelson: Can you come clean it up?\n");
	/* FIXME - I think I need to die here */
}

/*---------------------------------------------------------------------------*/

static bool warn(char *title, char *fmt, ...)
{
	va_list argp;
	char stuff[512];

	sprintf(stuff, "%s: ", title);
	va_start(argp, fmt);
	vsprintf(stuff, fmt, argp);
	va_end(argp);
	sprintf(stuff, ".\n");
	putlog(LOG_MISC, "*", "%s", stuff);
	return(TRUE);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Capitalize
 *
 *		Purpose:		Convert a string to look nice.
 */
static void capitalize(char *string)
{
	register int i,j=0;
	bool start=TRUE;

	for(i=0; i<(int)strlen(string); ++i) {
		if(isalpha(string[i])) {
			if(start==TRUE) { string[i]=(char)toupper((int)string[i]); j=i; }
			else string[i]=(char)tolower((int)string[i]);
			start=FALSE;
		}
		if(strchr(":",string[i])!=NULL)
			string[j]=(char)tolower((int)string[j]);
		if((i>2)&&(strchr("!.?", string[i-1])!=NULL)&&(isspace(string[i])))
			start=TRUE;
	}
}
 
/*---------------------------------------------------------------------------*/

/*
 *		Function:	Upper
 *
 *		Purpose:		Convert a string to its uppercase representation.
 */
static void upper(char *string)
{
	register int i;

	for(i=0; i<(int)strlen(string); ++i) string[i]=(char)toupper((int)string[i]);
}
 
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Word
 *
 *		Purpose:		Add a word to a dictionary, and return the identifier
 *						assigned to the word.  If the word already exists in
 *						the dictionary, then return its current identifier
 *						without adding it again.
 */
static BYTE2 add_word(DICTIONARY *dictionary, STRING word)
{
	register int i;
	int position;
	bool found;

	/* 
	 *		If the word's already in the dictionary, there is no need to add it
	 */
	position=search_dictionary(dictionary, word, &found);
	if(found==TRUE) goto succeed;

	/* 
	 *		Increase the number of words in the dictionary
	 */
	dictionary->size+=1;

	/*
	 *		Allocate one more entry for the word index
	 */
	if(dictionary->index==NULL) {
		dictionary->index=(BYTE2 *)nmalloc(sizeof(BYTE2)*
		(dictionary->size));
	} else {
		dictionary->index=(BYTE2 *)realloc((BYTE2 *)
		(dictionary->index),sizeof(BYTE2)*(dictionary->size));
	}
	if(dictionary->index==NULL) {
		error("add_word", "Unable to reallocate the index.");
		goto fail;
	}

	/*
	 *		Allocate one more entry for the word array
	 */
	if(dictionary->entry==NULL) {
		dictionary->entry=(STRING *)nmalloc(sizeof(STRING)*(dictionary->size));
	} else {
		dictionary->entry=(STRING *)realloc((STRING *)(dictionary->entry),
		sizeof(STRING)*(dictionary->size));
	}
	if(dictionary->entry==NULL) {
		error("add_word", "Unable to reallocate the dictionary to %d elements.", dictionary->size);
		goto fail;
	}

	/*
	 *		Copy the new word into the word array
	 */
	dictionary->entry[dictionary->size-1].length=word.length;
	dictionary->entry[dictionary->size-1].word=(char *)nmalloc(sizeof(char)*
	(word.length));
	if(dictionary->entry[dictionary->size-1].word==NULL) {
		error("add_word", "Unable to allocate the word.");
		goto fail;
	}
	for(i=0; i<word.length; ++i)
		dictionary->entry[dictionary->size-1].word[i]=word.word[i];

	/*
	 *		Shuffle the word index to keep it sorted alphabetically
	 */
	for(i=(dictionary->size-1); i>position; --i)
		dictionary->index[i]=dictionary->index[i-1];

	/*
	 *		Copy the new symbol identifier into the word index
	 */
	dictionary->index[position]=dictionary->size-1;

succeed:
	return(dictionary->index[position]);

fail:
	return(0);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Search_Dictionary
 *
 *		Purpose:		Search the dictionary for the specified word, returning its
 *						position in the index if found, or the position where it
 *						should be inserted otherwise.
 */
static int search_dictionary(DICTIONARY *dictionary, STRING word, bool *find)
{
	int position;
	int min;
	int max;
	int middle;
	int compar;

	/*
	 *		If the dictionary is empty, then obviously the word won't be found
	 */
	if(dictionary->size==0) {
		position=0;
		goto notfound;
	}

	/*
	 *		Initialize the lower and upper bounds of the search
	 */
	min=0;
	max=dictionary->size-1;
	/*
	 *		Search repeatedly, halving the search space each time, until either
	 *		the entry is found, or the search space becomes empty
	 */
	while(TRUE) {
		/*
		 *		See whether the middle element of the search space is greater
		 *		than, equal to, or less than the element being searched for.
		 */
		middle=(min+max)/2;
		compar=wordcmp(word, dictionary->entry[dictionary->index[middle]]);
		/*
		 *		If it is equal then we have found the element.  Otherwise we
		 *		can halve the search space accordingly.
		 */
		if(compar==0) {
			position=middle;
			goto found;
		} else if(compar>0) {
			if(max==middle) {
				position=middle+1;
				goto notfound;
			}
			min=middle+1;
		} else {
			if(min==middle) {
				position=middle;
				goto notfound;
			}
			max=middle-1;
		}
	}

found:
	*find=TRUE;
	return(position);

notfound:
	*find=FALSE;
	return(position);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Find_Word
 *
 *		Purpose:		Return the symbol corresponding to the word specified.
 *						We assume that the word with index zero is equal to a
 *						NULL word, indicating an error condition.
 */
static BYTE2 find_word(DICTIONARY *dictionary, STRING word)
{
	int position;
	bool found;

	position=search_dictionary(dictionary, word, &found);

	if(found==TRUE) return(dictionary->index[position]);
	else return(0);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Wordcmp
 *
 *		Purpose:		Compare two words, and return an integer indicating whether
 *						the first word is less than, equal to or greater than the
 *						second word.
 */
static int wordcmp(STRING word1, STRING word2)
{
	register int i;
	int bound;

	bound=MEGA_MIN(word1.length,word2.length);

	for(i=0; i<bound; ++i)
		if(toupper(word1.word[i])!=toupper(word2.word[i]))
			return((int)(toupper(word1.word[i])-toupper(word2.word[i])));

	if(word1.length<word2.length) return(-1);
	if(word1.length>word2.length) return(1);

	return(0);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Free_Dictionary
 *
 *		Purpose:		Release the memory consumed by the dictionary.
 */
static void free_dictionary(DICTIONARY *dictionary)
{
	if(dictionary==NULL) return;
	if(dictionary->entry!=NULL) {
		nfree(dictionary->entry);
		dictionary->entry=NULL;
	}
	if(dictionary->index!=NULL) {
		nfree(dictionary->index);
		dictionary->index=NULL;
	}
	dictionary->size=0;
}

/*---------------------------------------------------------------------------*/

static void free_model(MODEL *model)
{
	if(model==NULL) return;
	if(model->forward!=NULL) {
		free_tree(model->forward);
	}
	if(model->backward!=NULL) {
		free_tree(model->backward);
	}
	if(model->halcontext!=NULL) {
		nfree(model->halcontext);
	}
	if(model->dictionary!=NULL) {
		free_dictionary(model->dictionary);
		nfree(model->dictionary);
	}
	nfree(model);
}

/*---------------------------------------------------------------------------*/

static void free_tree(TREE *tree)
{
	static int level=0;
	register int i;

	if(tree==NULL) return;

	if(tree->tree!=NULL) {
		for(i=0; i<tree->branch; ++i) {
			++level;
			free_tree(tree->tree[i]);
			--level;
		}
		nfree(tree->tree);
	}
	nfree(tree);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_Dictionary
 *
 *		Purpose:		Add dummy words to the dictionary.
 */
static void initialize_dictionary(DICTIONARY *dictionary)
{
	STRING word={ 7, "<ERROR>" };
	STRING end={ 5, "<FIN>" };

	(void)add_word(dictionary, word);
	(void)add_word(dictionary, end);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	New_Dictionary
 *
 *		Purpose:		Allocate room for a new dictionary.
 */
static DICTIONARY *new_dictionary(void)
{
	DICTIONARY *dictionary=NULL;

	dictionary=(DICTIONARY *)nmalloc(sizeof(DICTIONARY));
	if(dictionary==NULL) {
		error("new_dictionary", "Unable to allocate dictionary.");
		return(NULL);
	}

	dictionary->size=0;
	dictionary->index=NULL;
	dictionary->entry=NULL;

	return(dictionary);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Save_Dictionary
 *
 *		Purpose:		Save a dictionary to the specified file.
 */
static void save_dictionary(FILE *file, DICTIONARY *dictionary)
{
	register int i;

	fwrite(&(dictionary->size), sizeof(BYTE4), 1, file);
	for(i=0; i<dictionary->size; ++i) {
		save_word(file, dictionary->entry[i]);
	}
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Load_Dictionary
 *
 *		Purpose:		Load a dictionary from the specified file.
 */
static void load_dictionary(FILE *file, DICTIONARY *dictionary)
{
	register int i;
	int size;

	fread(&size, sizeof(BYTE4), 1, file);
	for(i=0; i<size; ++i) {
		load_word(file, dictionary);
	}
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Save_Word
 *
 *		Purpose:		Save a dictionary word to a file.
 */
static void save_word(FILE *file, STRING word)
{
	register int i;

	fwrite(&(word.length), sizeof(BYTE1), 1, file);
	for(i=0; i<word.length; ++i)
		fwrite(&(word.word[i]), sizeof(char), 1, file);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Load_Word
 *
 *		Purpose:		Load a dictionary word from a file.
 */
static void load_word(FILE *file, DICTIONARY *dictionary)
{
	register int i;
	STRING word;

	fread(&(word.length), sizeof(BYTE1), 1, file);
	word.word=(char *)nmalloc(sizeof(char)*word.length);
	if(word.word==NULL) {
		error("load_word", "Unable to allocate word");
		return;
	}
	for(i=0; i<word.length; ++i)
		fread(&(word.word[i]), sizeof(char), 1, file);
	add_word(dictionary, word);
	nfree(word.word);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	New_Node
 *
 *		Purpose:		Allocate a new node for the n-gram tree, and initialise
 *						its contents to sensible values.
 */
static TREE *new_node(void)
{
	TREE *node=NULL;

	/*
	 *		Allocate memory for the new node
	 */
	node=(TREE *)nmalloc(sizeof(TREE));
	if(node==NULL) {
		error("new_node", "Unable to allocate the node.");
		goto fail;
	}

	/*
	 *		Initialise the contents of the node
	 */
	node->symbol=0;
	node->usage=0;
	node->count=0;
	node->branch=0;
	node->tree=NULL;

	return(node);

fail:
	if(node!=NULL) nfree(node);
	return(NULL);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	New_Model
 *
 *		Purpose:		Create and initialise a new ngram model.
 */
static MODEL *new_model(int order)
{
	MODEL *model=NULL;

	model=(MODEL *)nmalloc(sizeof(MODEL));
	if(model==NULL) {
		error("new_model", "Unable to allocate model.");
		goto fail;
	}

	model->order=order;
	model->forward=new_node();
	model->backward=new_node();
	model->halcontext=(TREE **)nmalloc(sizeof(TREE *)*(order+2));
	if(model->halcontext==NULL) {
		error("new_model", "Unable to allocate context array.");
		goto fail;
	}
	initialize_context(model);
	model->dictionary=new_dictionary();
	initialize_dictionary(model->dictionary);

	return(model);

fail:
	return(NULL);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Update_Model
 *
 *		Purpose:		Update the model with the specified symbol.
 */
static void update_model(MODEL *model, int symbol)
{
	register int i;

	/*
	 *		Update all of the models in the current context with the specified
	 *		symbol.
	 */
	for(i=(model->order+1); i>0; --i)
		if(model->halcontext[i-1]!=NULL)
			model->halcontext[i]=add_symbol(model->halcontext[i-1], (BYTE2)symbol);

	return;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Update_Context
 *
 *		Purpose:		Update the context of the model without adding the symbol.
 */
static void update_context(MODEL *model, int symbol)
{
	register int i;

	for(i=(model->order+1); i>0; --i)
		if(model->halcontext[i-1]!=NULL)
			model->halcontext[i]=find_symbol(model->halcontext[i-1], symbol);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Symbol
 *
 *		Purpose:		Update the statistics of the specified tree with the
 *						specified symbol, which may mean growing the tree if the
 *						symbol hasn't been seen in this context before.
 */
static TREE *add_symbol(TREE *tree, BYTE2 symbol)
{
	TREE *node=NULL;

	/*
	 *		Search for the symbol in the subtree of the tree node.
	 */
	node=find_symbol_add(tree, symbol);

	/*
	 *		Increment the symbol counts
	 */
	if((node->count<65535)) {
		node->count+=1;
		tree->usage+=1;
	}

	return(node);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Find_Symbol
 *
 *		Purpose:		Return a pointer to the child node, if one exists, which
 *						contains the specified symbol.
 */
static TREE *find_symbol(TREE *node, int symbol)
{
	register int i;
	TREE *found=NULL;
	bool found_symbol=FALSE;

	/* 
	 *		Perform a binary search for the symbol.
	 */
	i=search_node(node, symbol, &found_symbol);
	if(found_symbol==TRUE) found=node->tree[i];

	return(found);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Find_Symbol_Add
 *
 *		Purpose:		This function is conceptually similar to find_symbol,
 *						apart from the fact that if the symbol is not found,
 *						a new node is automatically allocated and added to the
 *						tree.
 */
static TREE *find_symbol_add(TREE *node, int symbol)
{
	register int i;
	TREE *found=NULL;
	bool found_symbol=FALSE;

	/* 
	 *		Perform a binary search for the symbol.  If the symbol isn't found,
	 *		attach a new sub-node to the tree node so that it remains sorted.
	 */
	i=search_node(node, symbol, &found_symbol);
	if(found_symbol==TRUE) {
		found=node->tree[i];
	} else {
		found=new_node();
		found->symbol=symbol;
		add_node(node, found, i);
	}

	return(found);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Node
 *
 *		Purpose:		Attach a new child node to the sub-tree of the tree
 *						specified.
 */
static void add_node(TREE *tree, TREE *node, int position)
{
	register int i;

	/*
	 *		Allocate room for one more child node, which may mean allocating
	 *		the sub-tree from scratch.
	 */
	if(tree->tree==NULL) {
		tree->tree=(TREE **)nmalloc(sizeof(TREE *)*(tree->branch+1));
	} else {
		tree->tree=(TREE **)realloc((TREE **)(tree->tree),sizeof(TREE *)*
		(tree->branch+1));
	}
	if(tree->tree==NULL) {
		error("add_node", "Unable to reallocate subtree.");
		return;
	}

	/*
	 *		Shuffle the nodes down so that we can insert the new node at the
	 *		subtree index given by position.
	 */
	for(i=tree->branch; i>position; --i)
		tree->tree[i]=tree->tree[i-1];

	/*
	 *		Add the new node to the sub-tree.
	 */
	tree->tree[position]=node;
	tree->branch+=1;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Search_Node
 *
 *		Purpose:		Perform a binary search for the specified symbol on the
 *						subtree of the given node.  Return the position of the
 *						child node in the subtree if the symbol was found, or the
 *						position where it should be inserted to keep the subtree
 *						sorted if it wasn't.
 */
static int search_node(TREE *node, int symbol, bool *found_symbol)
{
	register int position;
	int min;
	int max;
	int middle;
	int compar;

	/*
	 *		Handle the special case where the subtree is empty.
	 */ 
	if(node->branch==0) {
		position=0;
		goto notfound;
	}

	/*
	 *		Perform a binary search on the subtree.
	 */
	min=0;
	max=node->branch-1;
	while(TRUE) {
		middle=(min+max)/2;
		compar=symbol-node->tree[middle]->symbol;
		if(compar==0) {
			position=middle;
			goto found;
		} else if(compar>0) {
			if(max==middle) {
				position=middle+1;
				goto notfound;
			}
			min=middle+1;
		} else {
			if(min==middle) {
				position=middle;
				goto notfound;
			}
			max=middle-1;
		}
	}

found:
	*found_symbol=TRUE;
	return(position);

notfound:
	*found_symbol=FALSE;
	return(position);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_Context
 *
 *		Purpose:		Set the context of the model to a default value.
 */
static void initialize_context(MODEL *model)
{
	register int i;

	for(i=0; i<=model->order; ++i) model->halcontext[i]=NULL;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Learn
 *
 *		Purpose:		Learn from the user's input.
 */
static void learn(MODEL *model, DICTIONARY *words)
{
	register int i;
	BYTE2 symbol;

	/*
	 *		We only learn from inputs which are long enough
	 */
	if(words->size<=(model->order)) return;

	/*
	 *		Train the model in the forwards direction.  Start by initializing
	 *		the context of the model.
	 */
	initialize_context(model);
	model->halcontext[0]=model->forward;
	for(i=0; i<words->size; ++i) {
		/*
		 *		Add the symbol to the model's dictionary if necessary, and then
		 *		update the forward model accordingly.
		 */
		symbol=add_word(model->dictionary, words->entry[i]);
		update_model(model, symbol);
	}
	/*
	 *		Add the sentence-terminating symbol.
	 */
	update_model(model, 1);

	/*
	 *		Train the model in the backwards direction.  Start by initializing
	 *		the context of the model.
	 */
	initialize_context(model);
	model->halcontext[0]=model->backward;
	for(i=words->size-1; i>=0; --i) {
		/*
		 *		Find the symbol in the model's dictionary, and then update
		 *		the backward model accordingly.
		 */
		symbol=find_word(model->dictionary, words->entry[i]);
		update_model(model, symbol);
	}
	/*
	 *		Add the sentence-terminating symbol.
	 */
	update_model(model, 1);

	return;
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Train
 *
 *		Purpose:	 	Infer a MegaHAL brain from the contents of a text file.
 */
static void train(MODEL *model, char *filename)
{
	FILE *file;
	char buffer[1024];
	DICTIONARY *words=NULL;
	int length;

	if(filename==NULL) return;

	file=fopen(filename, "r");
	if(file==NULL) {
		putlog(LOG_MISC, "*", "Unable to find the personality %s\n", filename);
		return;
	}

	fseek(file, 0, 2);
   length=ftell(file);
   rewind(file);

	words=new_dictionary();

	while(!feof(file)) {

		if(fgets(buffer, 1024, file)==NULL) break;
		if(buffer[0]=='#') continue;

		buffer[strlen(buffer)-1]='\0';

		upper(buffer);
		make_words(buffer, words);
		learn(model, words);


	}

	free_dictionary(words);
	fclose(file);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Show_Dictionary
 *
 *		Purpose:		Display the dictionary for training purposes.
 */
static void show_dictionary(DICTIONARY *dictionary)
{
	register int i;
	register int j;
	FILE *file;

	file=fopen("megahal.dic", "w");
	if(file==NULL) {
		warn("show_dictionary", "Unable to open file");
		return;
	}

	for(i=0; i<dictionary->size; ++i) {
		for(j=0; j<dictionary->entry[i].length; ++j)
			fprintf(file, "%c", dictionary->entry[i].word[j]);
		fprintf(file, "\n");
	}

	fclose(file);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Save_Model
 *
 *		Purpose:		Save the current state to a MegaHAL brain file.
 */
static void save_model(char *modelname, MODEL *model)
{
	FILE *file;
	static char *filename=NULL;
	
	if(filename==NULL) filename=(char *)nmalloc(sizeof(char)*1);

	/*
	 *    Allocate memory for the filename
	 */
	filename=(char *)realloc(filename,
		sizeof(char)*(strlen(directory)+strlen(SEP)+12));
	if(filename==NULL) error("save_model","Unable to allocate filename");

	show_dictionary(model->dictionary);
	if(filename==NULL) return;

	sprintf(filename, "%s%s%s", directory, SEP, mega_file_name);
	file=fopen(filename, "wb");
	if(file==NULL) {
		warn("save_model", "Unable to open file `%s'", filename);
		return;
	}

	fwrite(COOKIE, sizeof(char), strlen(COOKIE), file);
	fwrite(&(model->order), sizeof(BYTE1), 1, file);
	save_tree(file, model->forward);
	save_tree(file, model->backward);
	save_dictionary(file, model->dictionary);

	fclose(file);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Save_Tree
 *
 *		Purpose:		Save a tree structure to the specified file.
 */
static void save_tree(FILE *file, TREE *node)
{
	static int level=0;
	register int i;

	fwrite(&(node->symbol), sizeof(BYTE2), 1, file);
	fwrite(&(node->usage), sizeof(BYTE4), 1, file);
	fwrite(&(node->count), sizeof(BYTE2), 1, file);
	fwrite(&(node->branch), sizeof(BYTE2), 1, file);

	for(i=0; i<node->branch; ++i) {
		++level;
		save_tree(file, node->tree[i]);
		--level;
	}
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Load_Tree
 *
 *		Purpose:		Load a tree structure from the specified file.
 */
static void load_tree(FILE *file, TREE *node)
{
	static int level=0;
	register int i;

	fread(&(node->symbol), sizeof(BYTE2), 1, file);
	fread(&(node->usage), sizeof(BYTE4), 1, file);
	fread(&(node->count), sizeof(BYTE2), 1, file);
	fread(&(node->branch), sizeof(BYTE2), 1, file);

	if(node->branch==0) return;

	node->tree=(TREE **)nmalloc(sizeof(TREE *)*(node->branch));
	if(node->tree==NULL) {
		error("load_tree", "Unable to allocate subtree");
		return;
	}

	for(i=0; i<node->branch; ++i) {
		node->tree[i]=new_node();
		++level;
		load_tree(file, node->tree[i]);
		--level;
	}
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Load_Model
 *
 *		Purpose:		Load a model into memory.
 */
static bool load_model(char *filename, MODEL *model)
{
	FILE *file;
	char cookie[16];

	if(filename==NULL) return(FALSE);

	file=fopen(filename, "rb");
	if(file==NULL) {
		warn("load_model", "Unable to open file `%s'", filename);
		return(FALSE);
	}

	fread(cookie, sizeof(char), strlen(COOKIE), file);
	if(strncmp(cookie, COOKIE, strlen(COOKIE))!=0) {
		warn("load_model", "File `%s' is not a MegaHAL brain", filename);
		goto fail;
	}

	fread(&(model->order), sizeof(BYTE1), 1, file);
	load_tree(file, model->forward);
	load_tree(file, model->backward);
	load_dictionary(file, model->dictionary);

	return(TRUE);
fail:
	fclose(file);

	return(FALSE);
}

/*---------------------------------------------------------------------------*/

/*
 *    Function:   Make_Words
 *
 *    Purpose:    Break a string into an array of words.
 */
static void make_words(char *input, DICTIONARY *words)
{
	int offset=0;

	/*
	 *		Clear the entries in the dictionary
	 */
	free_dictionary(words);

	/*
	 *		If the string is empty then do nothing, for it contains no words.
	 */
	if(strlen(input)==0) return;

	/*
	 *		Loop forever.
	 */
	while(1) {

		/*
		 *		If the current character is of the same type as the previous
		 *		character, then include it in the word.  Otherwise, terminate
		 *		the current word.
		 */
		if(boundary(input, offset)) {
			/*
			 *		Add the word to the dictionary
			 */
			if(words->entry==NULL)
				words->entry=(STRING *)nmalloc((words->size+1)*sizeof(STRING));
			else
				words->entry=(STRING *)realloc(words->entry, (words->size+1)*sizeof(STRING));

			if(words->entry==NULL) {
				error("make_words", "Unable to reallocate dictionary");
				return;
			}

			words->entry[words->size].length=offset;
			words->entry[words->size].word=input;
			words->size+=1;

			if(offset==(int)strlen(input)) break;
			input+=offset;
			offset=0;
		} else {
			++offset;
		}
	}

	/*
	 *		If the last word isn't punctuation, then replace it with a
	 *		full-stop character.
	 */
	if(isalnum(words->entry[words->size-1].word[0])) {
		if(words->entry==NULL)
			words->entry=(STRING *)nmalloc((words->size+1)*sizeof(STRING));
		else
			words->entry=(STRING *)realloc(words->entry, (words->size+1)*sizeof(STRING));
		if(words->entry==NULL) {
			error("make_words", "Unable to reallocate dictionary");
			return;
		}

		words->entry[words->size].length=1;
		words->entry[words->size].word=".";
		++words->size;
	}
	else if(strchr("!.?", words->entry[words->size-1].word[words->entry[words->size-1].length-1])==NULL) {
		words->entry[words->size-1].length=1;
		words->entry[words->size-1].word=".";
	}

   return;
}
 
/*---------------------------------------------------------------------------*/ 
/*
 *		Function:	Boundary
 *
 *		Purpose:		Return whether or not a word boundary exists in a string
 *						at the specified location.
 */
static bool boundary(char *string, int position)
{
	if(position==0)
		return(FALSE);

	if(position==(int)strlen(string))
		return(TRUE);

	if(
		(string[position]=='\'')&&
		(isalpha(string[position-1])!=0)&&
		(isalpha(string[position+1])!=0)
	)
		return(FALSE);

	if(
		(position>1)&&
		(string[position-1]=='\'')&&
		(isalpha(string[position-2])!=0)&&
		(isalpha(string[position])!=0)
	)
		return(FALSE);

	if(
		(isalpha(string[position])!=0)&&
		(isalpha(string[position-1])==0)
	)
		return(TRUE);
	
	if(
		(isalpha(string[position])==0)&&
		(isalpha(string[position-1])!=0)
	)
		return(TRUE);
	
	if(isdigit(string[position])!=isdigit(string[position-1]))
		return(TRUE);

	return(FALSE);
}
 
/*---------------------------------------------------------------------------*/ 
/*
 *    Function:   Generate_Reply
 *
 *    Purpose:    Take a string of user input and return a string of output
 *                which may vaguely be construed as containing a reply to
 *                whatever is in the input string.
 */
static char *generate_reply(MODEL *model, DICTIONARY *words)
{
	static DICTIONARY *dummy=NULL;
	DICTIONARY *replywords;
	DICTIONARY *keywords;
	float surprise;
	float max_surprise;
	char *output;
	static char *output_none=NULL;
	int count;
	int basetime;

	/*
	 *		Create an array of keywords from the words in the user's input
	 */
	keywords=make_keywords(model, words);

	/*
	 *		Make sure some sort of reply exists
	 */
	if(output_none==NULL) {
		output_none=nmalloc(40);
		if(output_none!=NULL)
			strcpy(output_none, "I don't know enough to answer you yet!");
	}
	output=output_none;
	if(dummy==NULL) dummy=new_dictionary();
	replywords=reply(model, dummy);
	if(dissimilar(words, replywords)==TRUE) output=make_output(replywords);

	/*
	 *		Loop for the specified waiting period, generating and evaluating
	 *		replies
	 */
	max_surprise=(float)-1.0;
	count=0;
	basetime=time(NULL);
	do {
		replywords=reply(model, keywords);
		surprise=evaluate_reply(model, keywords, replywords);
		++count;
		if((surprise>max_surprise)&&(dissimilar(words, replywords)==TRUE)) {
			max_surprise=surprise;
			output=make_output(replywords);
		}
	} while((time(NULL)-basetime)<timeout);

	/*
	 *		Return the best answer we generated
	 */
	return(output);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Dissimilar
 *
 *		Purpose:		Return TRUE or FALSE depending on whether the dictionaries
 *						are the same or not.
 */
static bool dissimilar(DICTIONARY *words1, DICTIONARY *words2)
{
	register int i;

	if(words1->size!=words2->size) return(TRUE);
	for(i=0; i<words1->size; ++i)
		if(wordcmp(words1->entry[i], words2->entry[i])!=0) return(TRUE);
	return(FALSE);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Make_Keywords
 *
 *		Purpose:		Put all the interesting words from the user's input into
 *						a keywords dictionary, which will be used when generating
 *						a reply.
 */
static DICTIONARY *make_keywords(MODEL *model, DICTIONARY *words)
{
	static DICTIONARY *keys=NULL;
	register int i;
	register int j;
	int c;

	if(keys==NULL) keys=new_dictionary();
	for(i=0; i<keys->size; ++i) nfree(keys->entry[i].word);
	free_dictionary(keys);

	for(i=0; i<words->size; ++i) {
		/*
		 *		Find the symbol ID of the word.  If it doesn't exist in
		 *		the model, or if it begins with a non-alphanumeric
		 *		character, or if it is in the exclusion array, then
		 *		skip over it.
		 */
		c=0;
		for(j=0; j<swp->size; ++j)
			if(wordcmp(swp->from[j], words->entry[i])==0) {
				add_key(model, keys, swp->to[j]);
				++c;
			}
		if(c==0) add_key(model, keys, words->entry[i]);
	}

	if(keys->size>0) for(i=0; i<words->size; ++i) {

		c=0;
		for(j=0; j<swp->size; ++j)
			if(wordcmp(swp->from[j], words->entry[i])==0) {
				add_aux(model, keys, swp->to[j]);
				++c;
			}
		if(c==0) add_aux(model, keys, words->entry[i]);
	}

	return(keys);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Key
 *
 *		Purpose:		Add a word to the keyword dictionary.
 */
static void add_key(MODEL *model, DICTIONARY *keys, STRING word)
{
	int symbol;

	symbol=find_word(model->dictionary, word);
	if(symbol==0) return;
	if(isalnum(word.word[0])==0) return;
	symbol=find_word(ban, word);
	if(symbol!=0) return;
	symbol=find_word(aux, word);
	if(symbol!=0) return;

	add_word(keys, word);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Aux
 *
 *		Purpose:		Add an auxilliary keyword to the keyword dictionary.
 */
static void add_aux(MODEL *model, DICTIONARY *keys, STRING word)
{
	int symbol;

	symbol=find_word(model->dictionary, word);
	if(symbol==0) return;
	if(isalnum(word.word[0])==0) return;
	symbol=find_word(aux, word);
	if(symbol==0) return;

	add_word(keys, word);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Reply
 *
 *		Purpose:		Generate a dictionary of reply words appropriate to the
 *						given dictionary of keywords.
 */
static DICTIONARY *reply(MODEL *model, DICTIONARY *keys)
{
	static DICTIONARY *replies=NULL;
	register int i;
	int symbol;
	bool start=TRUE;

	if(replies==NULL) replies=new_dictionary();
	free_dictionary(replies);

	/*
	 *		Start off by making sure that the model's context is empty.
	 */
	initialize_context(model);
	model->halcontext[0]=model->forward;
	used_key=FALSE;

	/*
	 *		Generate the reply in the forward direction.
	 */
	while(TRUE) {
		/*
		 *		Get a random symbol from the current context.
		 */
		if(start==TRUE) symbol=seed(model, keys);
		else symbol=babble(model, keys, replies);
		if((symbol==0)||(symbol==1)) break;
		start=FALSE;

		/*
		 *		Append the symbol to the reply dictionary.
		 */
		if(replies->entry==NULL)
			replies->entry=(STRING *)nmalloc((replies->size+1)*sizeof(STRING));
		else
			replies->entry=(STRING *)realloc(replies->entry, (replies->size+1)*sizeof(STRING));
		if(replies->entry==NULL) {
			error("reply", "Unable to reallocate dictionary");
			return(NULL);
		}

		replies->entry[replies->size].length=
			model->dictionary->entry[symbol].length;
		replies->entry[replies->size].word=
			model->dictionary->entry[symbol].word;
		replies->size+=1;

		/*
		 *		Extend the current context of the model with the current symbol.
		 */
		update_context(model, symbol);
	}

	/*
	 *		Start off by making sure that the model's context is empty.
	 */
	initialize_context(model);
	model->halcontext[0]=model->backward;

	/*
	 *		Re-create the context of the model from the current reply
	 *		dictionary so that we can generate backwards to reach the
	 *		beginning of the string.
	 */
	if(replies->size>0) for(i=MEGA_MIN(replies->size-1, model->order); i>=0; --i) {
		symbol=find_word(model->dictionary, replies->entry[i]);
		update_context(model, symbol);
	}

	/*
	 *		Generate the reply in the backward direction.
	 */
	while(TRUE) {
		/*
		 *		Get a random symbol from the current context.
		 */
		symbol=babble(model, keys, replies);
		if((symbol==0)||(symbol==1)) break;

		/*
		 *		Prepend the symbol to the reply dictionary.
		 */
		if(replies->entry==NULL)
			replies->entry=(STRING *)nmalloc((replies->size+1)*sizeof(STRING));
		else
			replies->entry=(STRING *)realloc(replies->entry, (replies->size+1)*sizeof(STRING));
		if(replies->entry==NULL) {
			error("reply", "Unable to reallocate dictionary");
			return(NULL);
		}

		/*
		 *		Shuffle everything up for the prepend.
		 */
		for(i=replies->size; i>0; --i) {
			replies->entry[i].length=replies->entry[i-1].length;
			replies->entry[i].word=replies->entry[i-1].word;
		}

		replies->entry[0].length=model->dictionary->entry[symbol].length;
		replies->entry[0].word=model->dictionary->entry[symbol].word;
		replies->size+=1;

		/*
		 *		Extend the current context of the model with the current symbol.
		 */
		update_context(model, symbol);
	}

	return(replies);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Evaluate_Reply
 *
 *		Purpose:		Measure the average surprise of keywords relative to the
 *						language model.
 */
static float evaluate_reply(MODEL *model, DICTIONARY *keys, DICTIONARY *words)
{
	register int i;
	register int j;
	int symbol;
	float probability;
	int count;
	float entropy=(float)0.0;
	TREE *node;
	int num=0;

	if(words->size<=0) return((float)0.0);
	initialize_context(model);
	model->halcontext[0]=model->forward;
	for(i=0; i<words->size; ++i) {
		symbol=find_word(model->dictionary, words->entry[i]);

		if(find_word(keys, words->entry[i])!=0) {
			probability=(float)0.0;
			count=0;
			++num;
			for(j=0; j<model->order; ++j) if(model->halcontext[j]!=NULL) {
	
				node=find_symbol(model->halcontext[j], symbol);
				probability+=(float)(node->count)/
					(float)(model->halcontext[j]->usage);
				++count;
	
			}

			if(count>0.0) entropy-=(float)log(probability/(float)count);
		}

		update_context(model, symbol);
	}

	initialize_context(model);
	model->halcontext[0]=model->backward;
	for(i=words->size-1; i>=0; --i) {
		symbol=find_word(model->dictionary, words->entry[i]);

		if(find_word(keys, words->entry[i])!=0) {
			probability=(float)0.0;
			count=0;
			++num;
			for(j=0; j<model->order; ++j) if(model->halcontext[j]!=NULL) {
	
				node=find_symbol(model->halcontext[j], symbol);
				probability+=(float)(node->count)/
					(float)(model->halcontext[j]->usage);
				++count;
	
			}

			if(count>0.0) entropy-=(float)log(probability/(float)count);
		}

		update_context(model, symbol);
	}

	if(num>=8) entropy/=(float)sqrt(num-1);
	if(num>=16) entropy/=(float)num;

	return(entropy);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Make_Output
 *
 *		Purpose:		Generate a string from the dictionary of reply words.
 */
static char *make_output(DICTIONARY *words)
{
	static char *output=NULL;
	register int i;
	register int j;
	int length;
	static char *output_none=NULL;
	
	if(output_none==NULL) output_none=nmalloc(40);

	if(output==NULL) {
		output=(char *)nmalloc(sizeof(char));
		if(output==NULL) {
			error("make_output", "Unable to allocate output");
			return(output_none);
		}
	}

	if(words->size==0) {
		if(output_none!=NULL)
			strcpy(output_none, "I am utterly speechless!");
		return(output_none);
	}

	length=1;
	for(i=0; i<words->size; ++i) length+=words->entry[i].length;

	output=(char *)realloc(output, sizeof(char)*length);
	if(output==NULL) {
		error("make_output", "Unable to reallocate output.");
		if(output_none!=NULL)
			strcpy(output_none, "I forgot what I was going to say!");
		return(output_none);
	}

	length=0;
	for(i=0; i<words->size; ++i)
		for(j=0; j<words->entry[i].length; ++j)
			output[length++]=words->entry[i].word[j];
			
	output[length]='\0';

	return(output);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Babble
 *
 *		Purpose:		Return a random symbol from the current context, or a
 *						zero symbol identifier if we've reached either the
 *						start or end of the sentence.  Select the symbol based
 *						on probabilities, favouring keywords.  In all cases,
 *						use the longest available context to choose the symbol.
 */
static int babble(MODEL *model, DICTIONARY *keys, DICTIONARY *words)
{
	/* Changed by BarkerJr to prevent compile warnings */
	/* TREE *node; */
	TREE *node=NULL;
	register int i;
	int count;
	/* Changed by BarkerJr to prevent compile warnings */
	/* int symbol; */
	int symbol=0;

	/*
	 *		Select the longest available context.
	 */
	for(i=0; i<=model->order; ++i)
		if(model->halcontext[i]!=NULL)
			node=model->halcontext[i];

	if(node->branch==0) return(0);

	/*
	 *		Choose a symbol at random from this context.
	 */
	i=rnd(node->branch);
	count=rnd(node->usage);
	while(count>=0) {
		/*
		 *		If the symbol occurs as a keyword, then use it.  Only use an
		 *		auxilliary keyword if a normal keyword has already been used.
		 */
		symbol=node->tree[i]->symbol;

		if(
			(find_word(keys, model->dictionary->entry[symbol])!=0)&&
			((used_key==TRUE)||
			(find_word(aux, model->dictionary->entry[symbol])==0))&&
			(word_exists(words, model->dictionary->entry[symbol])==FALSE)
		) {
			used_key=TRUE;
			break;
		}
		count-=node->tree[i]->count;
		i=(i>=(node->branch-1))?0:i+1;
	}

	return(symbol);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Word_Exists
 *
 *		Purpose:		A silly brute-force searcher for the reply string.
 */
static bool word_exists(DICTIONARY *dictionary, STRING word)
{
	register int i;

	for(i=0; i<dictionary->size; ++i)
		if(wordcmp(dictionary->entry[i], word)==0)
			return(TRUE);
	return(FALSE);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Seed
 *
 *		Purpose:		Seed the reply by guaranteeing that it contains a
 *						keyword, if one exists.
 */
static int seed(MODEL *model, DICTIONARY *keys)
{
	register int i;
	int symbol;
	int stop;

	/*
	 *		Fix, thanks to Mark Tarrabain
	 */
	if(model->halcontext[0]->branch==0) symbol=0;
	else symbol=model->halcontext[0]->tree[rnd(model->halcontext[0]->branch)]->symbol;

	if(keys->size>0) {
		i=rnd(keys->size);
		stop=i;
		while(TRUE) {
			if(
				(find_word(model->dictionary, keys->entry[i])!=0)&&
				(find_word(aux, keys->entry[i])==0)
			) {
				symbol=find_word(model->dictionary, keys->entry[i]);
				return(symbol);
			}
			++i;
			if(i==keys->size) i=0;
			if(i==stop) return(symbol);
		}
	}

	return(symbol);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	New_Swap
 *
 *		Purpose:		Allocate a new swap structure.
 */
static SWAP *new_swap(void)
{
	SWAP *list;

	list=(SWAP *)nmalloc(sizeof(SWAP));
	if(list==NULL) {
		error("new_swap", "Unable to allocate swap");
		return(NULL);
	}
	list->size=0;
	list->from=NULL;
	list->to=NULL;

	return(list);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Add_Swap
 *
 *		Purpose:		Add a new entry to the swap structure.
 */
static void add_swap(SWAP *list, char *s, char *d)
{
	list->size+=1;

	if(list->from==NULL) {
		list->from=(STRING *)nmalloc(sizeof(STRING));
		if(list->from==NULL) {
			error("add_swap", "Unable to allocate list->from");
			return;
		}
	}

	if(list->to==NULL) {
		list->to=(STRING *)nmalloc(sizeof(STRING));
		if(list->to==NULL) {
			error("add_swap", "Unable to allocate list->to");
			return;
		}
	}

	list->from=(STRING *)realloc(list->from, sizeof(STRING)*(list->size));
	if(list->from==NULL) {
		error("add_swap", "Unable to reallocate from");
		return;
	}

	list->to=(STRING *)realloc(list->to, sizeof(STRING)*(list->size));
	if(list->to==NULL) {
		error("add_swap", "Unable to reallocate to");
		return;
	}

	list->from[list->size-1].length=strlen(s);
	list->from[list->size-1].word=mystrdup(s);
	list->to[list->size-1].length=strlen(d);
	list->to[list->size-1].word=mystrdup(d);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_Swap
 *
 *		Purpose:		Read a swap structure from a file.
 */
static SWAP *initialize_swap(char *filename)
{
	SWAP *list;
	FILE *file=NULL;
	char buffer[1024];
	char *from;
	char *to;

	list=new_swap();

	if(filename==NULL) return(list);

	file=fopen(filename, "r");
	if(file==NULL) return(list);

	while(!feof(file)) {

		if(fgets(buffer, 1024, file)==NULL) break;
		if(buffer[0]=='#') continue;
		from=strtok(buffer, "\t ");
		to=strtok(NULL, "\t \n#");

		add_swap(list, from, to);
	}

	fclose(file);
	return(list);
}

/*---------------------------------------------------------------------------*/

static void free_swap(SWAP *swap)
{
	register int i;

	if(swap==NULL) return;

	for(i=0; i<swap->size; ++i) {
		free_word(swap->from[i]);
		free_word(swap->to[i]);
	}
	nfree(swap->from);
	nfree(swap->to);
	nfree(swap);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Initialize_List
 *
 *		Purpose:		Read a dictionary from a file.
 */
static DICTIONARY *initialize_list(char *filename)
{
	DICTIONARY *list;
	FILE *file=NULL;
	STRING word;
	char *string;
	char buffer[1024];

	list=new_dictionary();

	if(filename==NULL) return(list);

	file=fopen(filename, "r");
	if(file==NULL) return(list);

	while(!feof(file)) {

		if(fgets(buffer, 1024, file)==NULL) break;
		if(buffer[0]=='#') continue;
		string=strtok(buffer, "\t \n#");

		if((string!=NULL)&&(strlen(string)>0)) {
			word.length=strlen(string);
			word.word=mystrdup(buffer);
			add_word(list, word);
		}
	}

	fclose(file);
	return(list);
}

/*---------------------------------------------------------------------------*/

/*
 *		Function:	Rnd
 *
 *		Purpose:		Return a random integer between 0 and range-1.
 */
static int rnd(int range)
{
	static bool flag=FALSE;

	if(flag==FALSE) {
		srand48(time(NULL));
	}
	flag=TRUE;
	return(floor(drand48()*(double)(range)));
}

/*---------------------------------------------------------------------------*/

static void load_personality(MODEL **model)
{
	FILE *file;
	static char *filename=NULL;

	if(filename==NULL) filename=(char *)nmalloc(sizeof(char)*1);

	/*
	 *		Allocate memory for the filename
	 */
	filename=(char *)realloc(filename,
		sizeof(char)*(strlen(directory)+strlen(SEP)+12));
	if(filename==NULL) error("load_personality","Unable to allocate filename");

	/*
	 *		Check to see if the brain exists
	 */
	if(strcmp(directory, DEFAULT)!=0) {
	sprintf(filename, "%s%s%s", directory, SEP, mega_file_name);
	file=fopen(filename, "r");
	if(file==NULL) {
		sprintf(filename, "%s%smegahal.trn", directory, SEP);
		file=fopen(filename, "r");
		if(file==NULL) {
			nfree(directory);
			directory=mystrdup(last);
			return;
		}
	}
	fclose(file);
	putlog(LOG_MISC, "*", "Changing to MegaHAL personality \"%s\".\n", directory);
	}

	/*
	 *		Free the current personality
	 */
	free_model(*model);
	free_words(ban);
	free_dictionary(ban);
	free_words(aux);
	free_dictionary(aux);
	free_words(grt);
	free_dictionary(grt);
	free_swap(swp);

	/*
	 *		Create a language model.
	 */
	*model=new_model(order);

	/*
	 *		Train the model on a text if one exists
	 */
	sprintf(filename, "%s%s%s", directory, SEP, mega_file_name);
	if(load_model(filename, *model)==FALSE) {
		sprintf(filename, "%s%smegahal.trn", directory, SEP);
		train(*model, filename);
	}

	/*
	 *		Read a dictionary containing banned keywords, auxiliary keywords,
	 *		greeting keywords and swap keywords
	 */
	sprintf(filename, "%s%smegahal.ban", directory, SEP);
	ban=initialize_list(filename);
	sprintf(filename, "%s%smegahal.aux", directory, SEP);
	aux=initialize_list(filename);
	sprintf(filename, "%s%smegahal.grt", directory, SEP);
	grt=initialize_list(filename);
	sprintf(filename, "%s%smegahal.swp", directory, SEP);
	swp=initialize_swap(filename);
}

/*---------------------------------------------------------------------------*/

static void change_personality(DICTIONARY *command, int position, MODEL **model)
{
	if(last!=NULL) { nfree(last); last=NULL; }
	if(directory!=NULL) last=mystrdup(directory);
	else directory=(char *)nmalloc(sizeof(char)*1);
	if(directory==NULL)
		error("change_personality", "Unable to allocate directory");
	if((command==NULL)||((position+2)>=command->size)) {
		directory=(char *)realloc(directory, sizeof(char)*(strlen(DEFAULT)+1));
		if(directory==NULL)
			error("change_personality", "Unable to allocate directory");
		strcpy(directory, DEFAULT);
		if(last==NULL) last=mystrdup(directory);
	} else {
		directory=(char *)realloc(directory,
			sizeof(char)*(command->entry[position+2].length+1));
		if(directory==NULL)
			error("change_personality", "Unable to allocate directory");
		strncpy(directory, command->entry[position+2].word,
			command->entry[position+2].length);
		directory[command->entry[position+2].length]='\0';
	}

	load_personality(model);
}

/*---------------------------------------------------------------------------*/

static void free_words(DICTIONARY *words)
{
	register int i;

	if(words==NULL) return;

	if(words->entry!=NULL)
		for(i=0; i<words->size; ++i) free_word(words->entry[i]);
}

/*---------------------------------------------------------------------------*/

static void free_word(STRING word)
{
	nfree(word.word);
}

/*===========================================================================*/
/*
 *		$Log: megahal.c,v $
 *		Revision 1.25  1999/10/21 03:42:48  hutch
 *		Fixed problem on some operating systems caused by stderr and stdout not
 *		being of type FILE *.
 *
 * Revision 1.24  1998/09/03  03:07:09  hutch
 * Don't know.
 *
 *		Revision 1.23  1998/05/19 03:02:02  hutch
 *		Removed a small nmalloc() bug, and added a progress display for
 *		generate_reply().
 *
 *		Revision 1.22  1998/04/24 03:47:03  hutch
 *		Quick bug fix to get sunos version to work.
 *
 *		Revision 1.21  1998/04/24 03:39:51  hutch
 *		Added the BRAIN command, to allow user to change MegaHAL personalities
 *		on the fly.
 *
 *		Revision 1.20  1998/04/22 07:12:37  hutch
 *		A few small changes to get the DOS version to compile.
 *
 *		Revision 1.19  1998/04/21 10:10:56  hutch
 *		Fixed a few little errors.
 *
 *		Revision 1.18  1998/04/06 08:02:01  hutch
 *		Added debugging stuff, courtesy of Paul Baxter.
 *
 *		Revision 1.17  1998/04/02 01:34:20  hutch
 *		Added the help function and fixed a few errors.
 *
 *		Revision 1.16  1998/04/01 05:42:57  hutch
 *		Incorporated Mac code, including speech synthesis, and attempted
 *		to tidy up the code for multi-platform support.
 *
 *		Revision 1.15  1998/03/27 03:43:15  hutch
 *		Added AMIGA specific changes, thanks to Dag Agren.
 *
 *		Revision 1.14  1998/02/20 06:40:13  hutch
 *		Tidied up transcript file format.
 *
 *		Revision 1.13  1998/02/20 06:26:19  hutch
 *		Fixed random number generator and Seed() function (thanks to Mark
 *		Tarrabain), removed redundant code left over from the Loebner entry,
 *		prettied things up a little and destroyed several causes of memory
 *		leakage (although probably not all).
 *
 *		Revision 1.12  1998/02/04 02:55:11  hutch
 *		Fixed up memory allocation error which caused SunOS versions to crash.
 *
 *		Revision 1.11  1998/01/22 03:16:30  hutch
 *		Fixed several memory leaks, and the frustrating bug in the
 *		Write_Input routine.
 *
 *		Revision 1.10  1998/01/19 06:44:36  hutch
 *		Fixed MegaHAL to compile under Linux with a small patch credited
 *		to Joey Hess (joey@kitenet.net).  MegaHAL may now be included as
 *		part of the Debian Linux distribution.
 *
 *		Revision 1.9  1998/01/19 06:37:32  hutch
 *		Fixed a minor bug with end-of-sentence punctuation.
 *
 *		Revision 1.8  1997/12/24 03:17:01  hutch
 *		More bug fixes, and hopefully the final contest version!
 *
 *		Revision 1.7  1997/12/22  13:18:09  hutch
 *		A few more bug fixes, and non-repeating implemented.
 *
 *		Revision 1.6  1997/12/22 04:27:04  hutch
 *		A few minor bug fixes.
 *
 *		Revision 1.5  1997/12/15 04:35:59  hutch
 *		Final Loebner version!
 *
 *		Revision 1.4  1997/12/11 05:45:29  hutch
 *		The almost finished version.
 *
 *		Revision 1.3  1997/12/10 09:08:09  hutch
 *		Now Loebner complient (tm).
 *
 *		Revision 1.2  1997/12/08 06:22:32  hutch
 *		Tidied up.
 *
 *		Revision 1.1  1997/12/05  07:11:44  hutch
 *		Initial revision (lots of files were merged into one, RCS re-started)
 *
 *		Revision 1.7  1997/12/04 07:07:13  hutch
 *		Added load and save functions, and tidied up some code.
 *
 *		Revision 1.6  1997/12/02 08:34:47  hutch
 *		Added the ban, aux and swp functions.
 *
 *		Revision 1.5  1997/12/02 06:03:04  hutch
 *		Updated to use a special terminating symbol, and to store only
 *		branches of maximum depth, as they are the only ones used in
 *		the reply.
 *
 *		Revision 1.4  1997/10/28 09:23:12  hutch
 *		MegaHAL is babbling nicely, but without keywords.
 *
 *		Revision 1.3  1997/10/15  09:04:03  hutch
 *		MegaHAL can parrot back whatever the user says.
 *
 *		Revision 1.2  1997/07/21 04:03:28  hutch
 *		Fully working.
 *
 *		Revision 1.1  1997/07/15 01:55:25  hutch
 *		Initial revision.
 */
/*===========================================================================*/
