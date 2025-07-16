//------------------------------------------------------------------------------
//   Programmer: Adam M Phillippy, CBCB - University of Maryland
//         File: unique-mer.c
//         Date: 09 / 30 / 2005
//
//   Try 'unique-mer -h' for more information
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>


//================================================================= Options ====
FILE *   OPT_Matches  = NULL;
unsigned OPT_K        = 0;


//============================================================ Fuction Decs ====
//------------------------------------------------------------- OutputKmers ----
void OutputKmers (unsigned long p, unsigned long b, unsigned long s);
void ProcessCovers();
void ParseArgs (int argc, char ** argv);
void PrintHelp (const char * s);
void PrintUsage (const char * s);


//=========================================================== Function Defs ====
int main (int argc, char ** argv)
{
  ParseArgs (argc, argv);
  ProcessCovers();

  return EXIT_SUCCESS;
}


//------------------------------------------------------------- OutputKmers ----
//! Output an inclusive interval of unique k-mer positions between two matches
//! as a start, length pair. Interval will be vaild for both p < b and p >= b,
//! i.e. both gap and overlap scenarios between adjacent matches. If no interval
//! is output, no unique k-mers exist off the front of the match, e.g. overlap
//! is to big to be spanned by a k-mer.
//!
//! p - inclusive end position of previous, lesser start pos match
//! b - inclusive start position of current, greater start pos match
//! s - length of sequence the matches reside on
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void OutputKmers (unsigned long p, unsigned long b, unsigned long s)
{
  if ( !s ) return;

  long i = p - OPT_K + 2;
  if ( i < 1 )
    i = 1;

  long j = b;
  if ( j > (long)s - OPT_K + 2 )
    j = s - OPT_K + 2;

  if ( i < j )
    printf ("%ld\t%ld\n", i, j - i);
}


//----------------------------------------------------------- ProcessCovers ----
void ProcessCovers()
{
  char line[PATH_MAX], idT[PATH_MAX];
  unsigned long len, lenT, b, p;
  int scan;

  unsigned init = 0;

  lenT = p = 0;
  *line = *idT = '\0';
  while ( fgets (line, PATH_MAX, OPT_Matches) )
    {
      //-- If a new Target mcover
      if ( *line == '>' )
        {
          OutputKmers (p, lenT + 1, lenT);

          scan = sscanf (line+1, "%s %lu", idT, &lenT);
          assert ( scan == 2 );

          printf (">%s %u\n", idT, OPT_K);

          p = 0;
          init = 1;
        }
      //-- If a match
      else if ( init )
        {
          scan = sscanf (line, "%lu %lu", &b, &len);
          assert ( scan == 2 );

          //-- Skip match if smaller than k
          if ( len < OPT_K )
            continue;

          //-- Output unique k-mers before this match
          OutputKmers (p, b, lenT);

          p = b + len - 1; // inclusive end of the now previous match
        }
    }

  OutputKmers (p, lenT + 1, lenT);
}


//--------------------------------------------------------------- ParseArgs ----
void ParseArgs (int argc, char ** argv)
{
  int ch, errflg = 0;
  optarg = NULL;

  while ( !errflg  &&  ((ch = getopt (argc, argv, "hk:")) != EOF) )
    switch (ch)
      {
      case 'h':
        PrintHelp (argv[0]);
        exit (EXIT_SUCCESS);
        break;

      case 'k':
        OPT_K = (unsigned) atoi (optarg);
        break;

      default:
        errflg ++;
      }

  //-- Mandatory arguments
  if ( optind != argc && optind != argc - 1 )
    errflg ++;

  if ( !OPT_K )
    {
      fprintf (stderr, "ERROR: -k option is mandatory\n");
      errflg ++;
    }

  if ( errflg > 0 )
    {
      PrintUsage (argv[0]);
      fprintf (stderr, "Try '%s -h' for more information.\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  //-- Match cover coming on stdin or file?
  if ( optind == argc )
    OPT_Matches = stdin;
  else if ( (OPT_Matches = fopen (argv[optind ++], "r")) == NULL )
    {
      fprintf (stderr, "ERROR: Could not open cover file %s\n", argv[optind-1]);
      exit (EXIT_FAILURE);
    }
}


//--------------------------------------------------------------- PrintHelp ----
void PrintHelp (const char * s)
{
  PrintUsage (s);
  fprintf (stdout,
"-h            Display help information\n"
"-k int        Desired unique k-mer size\n"
           "\n");

  fprintf (stdout,
"  Reads match cover information, as output by mcover-merge, from stdin\n"
"or file, and generates a set of unique k-mers for each each\n"
"Target. Match cover headers must include both the Target ID and\n"
"sequence length (in that order), and the match list must be sorted,\n"
"low to hi, by match position. k-mer size specified by -k must be\n"
"greater than or equal to the minimum match size used to generate the\n"
"match cover. Output will be a list of intervals (start, length pairs)\n"
"such that an interval of length L will contain exactly L unique k-mer\n"
"positions. Outputs the inverse of of the common-mer program.\n"
           "\n");

  return;
}


//-------------------------------------------------------------- PrintUsage ----
void PrintUsage (const char * s)
{
  fprintf (stdout,"\n"
"Usage: %s [options] -k int MCOVER\n"
"   or  %s [options] -k int < MCOVER\n"
           "\n",
           s, s);
  return;
}
