/*
 * fasta-chop.c
 *
 * Breaks a fasta file into bite size chunks. File will be broken into
 * chunks of atleast (but not much more than) the chunk size argument.
 *
 * 100  = 100 bytes
 * 100B = 100 bytes
 * 100K = 100 kilobytes
 * 100M = 100 megabytes
 * 100G = 100 gigabytes
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

/*-- breakchar must occur immediately after a newline --*/
#define BREAKCHAR '>'
#define MAXDIGITS 100
#define MAXMESSAGE 1024
static char Message[MAXMESSAGE];

#define BYTES 'B'
#define KILOBYTES 'K'
#define MEGABYTES 'M'
#define GIGABYTES 'G'



FILE* FileOpen(const char *file, const char *mode)
{
  FILE* retval = fopen(file,mode);
  if ( retval == NULL )
    {
      snprintf(Message, MAXMESSAGE, "Could not open %s", file);
      perror(Message);
      exit(EXIT_FAILURE);
    }
  return retval;
}


int Seek(FILE *stream, off_t offset, int whence)
{
  int retval = fseeko(stream, offset, whence);
  if ( retval != 0 )
    {
      snprintf(Message, MAXMESSAGE, "Could not fseek: ");
      perror(Message);
      exit(EXIT_FAILURE);
    }
  return retval;
}


off_t Tell(FILE *stream)
{
  int retval = ftello(stream);
  if ( retval == -1 )
    {
      snprintf(Message, MAXMESSAGE, "Could not ftell: ");
      perror(Message);
      exit(EXIT_FAILURE);
    }
  return retval;
}



/*-- Copy file 'fpi' from beg -> end to file 'fpo' --*/
void Copy (off_t beg, off_t end, FILE * fpi, FILE * fpo)
{
  static char BUFF[4096];
  static int BUFFLEN = 4096;

  int chunk;
  int remain = end - beg;

  Seek (fpi, beg, SEEK_SET);
  while ( remain )
    {
      chunk = remain > BUFFLEN ? BUFFLEN : remain;
      if ( !fread  (BUFF, chunk, 1, fpi) )
        {
          fprintf (stderr, "ERROR: reading chunk\n");
          exit (EXIT_FAILURE);
        }
      if ( !fwrite (BUFF, chunk, 1, fpo) )
        {
          fprintf (stderr, "ERROR: writing chunk\n");
          exit (EXIT_FAILURE);
        }
      remain -= chunk;
    }
}



int main (int argc, char ** argv)
{
  register int ch, pch;
  off_t beg, cur, pre, chunk;
  int maxname, first, suffix = 0;
  char xbyte = BYTES;

  char * namei, * nameo;
  FILE * fpi = NULL;
  FILE * fpo = NULL;

  if ( argc != 3 )
    {
      fprintf (stderr, "Usage: %s FASTA CHUNKSIZE\n", argv[0]);
      fprintf (stderr, "Chunk size may be abbreviated B,K,M,G.\n");
      return EXIT_FAILURE;
    }

  /*-- get arguments --*/
  maxname = strlen (argv[1]);
  namei = (char *) malloc (maxname + 1);
  strncpy (namei, argv[1], maxname + 1);
  sscanf (argv[2], "%lld%c", &chunk, &xbyte);

  maxname += MAXDIGITS;
  nameo = (char *) malloc (maxname + 1);

  switch ( xbyte )
    {
    case BYTES:
      break;
    case KILOBYTES:
      chunk <<= 10;
      break;
    case MEGABYTES:
      chunk <<= 20;
      break;
    case GIGABYTES:
      chunk <<= 30;
      break;
    default:
      fprintf (stderr, "ERROR: invalid chunk size identifier\n");
      return EXIT_FAILURE;
    }

  /*-- open input --*/
  fpi = FileOpen (namei, "r");
  beg = Tell (fpi);

  /*-- output input stats --*/
  fprintf (stderr,"# chopping into %lld byte chunks\n",chunk);

  /*-- open output --*/
  snprintf (nameo, maxname, "%s.%d", namei, suffix ++);
  fpo = FileOpen (nameo, "w");
  fprintf (stderr,"# writing %s\n",nameo);

  /*-- start chopping --*/
  pch = 0;
  first = 1;
  cur = pre = beg;
  do
    {
      ch = fgetc (fpi);
      cur ++;

      /*-- possible break point --*/
      if ( ch == EOF || (ch == BREAKCHAR  &&  pch == '\n') )
        {
          /*-- open new output --*/
          if ( cur - beg > chunk  &&  !first )
            {
              fclose (fpo);

              snprintf (nameo, maxname, "%s.%d", namei, suffix ++);
              fpo = FileOpen (nameo, "w");
              fprintf (stderr,"# writing %s\n",nameo);

              beg = pre;
              first = 1;
            }

          /*-- copy previous record --*/
          Copy (pre, cur - 1, fpi, fpo);
          fgetc (fpi);

          pre = cur - 1;
          first = 0;
        }

      pch = ch;
    } while ( ch != EOF );

  /*-- close em down --*/
  fclose (fpo);
  fclose (fpi);

  return EXIT_SUCCESS;
}
