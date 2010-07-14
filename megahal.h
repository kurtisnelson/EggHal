
/*===========================================================================*/

/*
 *  Copyright (C) 1998 Jason Hutchens
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
 *		$Id: megahal.h,v 1.3 1998/09/03 03:15:40 hutch Exp hutch $
 *
 *		File:			megahal.h
 *
 *		Program:		MegaHAL v8r5
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
 *		Notes:		This file is best viewed with tabstops set to three spaces.
 */

/*===========================================================================*/

#define P_THINK 40
#define D_KEY 100000
#define V_KEY 50000
#define D_THINK 500000
#define V_THINK 250000

#define MEGA_MIN(a,b) ((a)<(b))?(a):(b)

#define COOKIE "MegaHALv8"

#define DEFAULT "."

#define COMMAND_SIZE (sizeof(command)/sizeof(command[0]))

#define BYTE1 unsigned char
#define BYTE2 unsigned short
#define BYTE4 unsigned long

#define SEP "/"

/*===========================================================================*/

#undef FALSE
#undef TRUE
typedef enum { FALSE, TRUE } bool;

typedef struct {
	BYTE1 length;
	char *word;
} STRING;

typedef struct {
	BYTE4 size;
	STRING *entry;
	BYTE2 *index;
} DICTIONARY;

typedef struct {
	BYTE2 size;
	STRING *from;
	STRING *to;
} SWAP;

typedef struct NODE {
	BYTE2 symbol;
	BYTE4 usage;
	BYTE2 count;
	BYTE2 branch;
	struct NODE **tree;
} TREE;

typedef struct {
	BYTE1 order;
	TREE *forward;
	TREE *backward;
	TREE **halcontext;
	DICTIONARY *dictionary;
} MODEL;

typedef enum { UNKNOWN, QUIT, EXIT, SAVE, DELAY, HELP, SPEECH, VOICELIST, VOICE, BRAIN, PROGRESS, THINK } COMMAND_WORDS;

typedef struct {
	STRING word;
	char *helpstring;
	COMMAND_WORDS command;
} COMMAND;

/*===========================================================================*/

/*
 *		$Log: megahal.h,v $
 * Revision 1.3  1998/09/03  03:15:40  hutch
 * Dunno.
 *
 *		Revision 1.2  1998/04/21 10:10:56  hutch
 *		Fixed a few little errors.
 *
 *		Revision 1.1  1998/04/06 08:02:01  hutch
 *		Initial revision
 */

/*===========================================================================*/

