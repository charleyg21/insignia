//------------------------------------------------------------------------------
//   Programmer: Adam M Phillippy, CBCB - University of Maryland
//         File: mcover-prog.cc
//         Date: 09 / 07 / 2005
//
//   Try 'mcover -h' for more information
//   Beware of char*'s in hash_maps
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#include <mcover.hh>


//================================================================= Options ====
FILE * OPT_Matches  = NULL;
bool OPT_Reference  = false;
bool OPT_Query      = false;


//============================================================ Fuction Decs ====
void FlushCover (const char * idB, const MatchSetMap_t & ref);
void ProcessMatches();
void ParseArgs (int argc, char ** argv);
void PrintHelp (const char * s);
void PrintUsage (const char * s);


//=========================================================== Function Defs ====
int main (int argc, char ** argv)
{
  ParseArgs (argc, argv);
  ProcessMatches();

  return EXIT_SUCCESS;
}


//-------------------------------------------------------------- FlushCover ----
void FlushCover (const char * idB, MatchSetMap_t & matches)
{
  char * idT;
  MatchSet_t::const_iterator si;
  MatchSetMap_t::iterator mi (matches.begin());

  //-- For each Target matching idB
  while ( mi != matches.end() )
    {
      idT = mi->first;

      //-- For idT->idB
      if ( ! mi->second.empty() )
        {
          printf (">%s %s\n", idT, idB);
          for ( si  = mi->second.begin();
                si != mi->second.end(); ++ si )
            printf ("%lu\t%lu\n", si->b, si->e - si->b + 1);
        }

      //-- Clean up the memory for this reference
      matches.erase (mi ++);
      free (idT);
    }
}


//---------------------------------------------------------- ProcessMatches ----
void ProcessMatches()
{
  char line[PATH_MAX], idT[PATH_MAX], idB[PATH_MAX], tmp[PATH_MAX];
  unsigned long pR, pQ, len;
  unsigned long count = 0;
  bool init = false;
  int scan;
  char ch;

  MatchSetMap_t matches;          // target->background match cover
  MatchSetMap_t::iterator mi;

  *line = *idT = *idB = '\0';
  while ( fgets (line, PATH_MAX, OPT_Matches) )
    {
      //-- If a match header
      if ( *line == '>' )
        {
          scan = sscanf (line + 1, "%s %c", tmp, &ch);
          assert ( scan >= 1 );

          //-- New forward matches
          if ( scan == 1  ||  ch != REVERSE )
            {
              FlushCover (idB, matches);
              strcpy (idB, tmp);
              ch = FORWARD;
            }
          // else, reverse matches from the same Background sequence

          init = true;
        }
      //-- If a match
      else if ( init )
        {
          scan = sscanf (line, "%s %lu %lu %lu", idT, &pR, &pQ, &len);
          assert ( scan == 4 );

          //-- Find or insert the reference
          if ( (mi = matches.find (idT)) == matches.end() )
            mi = (matches.insert(make_pair(strdup(idT),MatchSet_t()))).first;

          //-- Merge the new match with the existing matches
          MergeMatch (Match_t (pR, pR + len - 1), mi->second);
          ++ count;
        }
    }

  FlushCover (idB, matches);

  if ( !init )
    {
      fprintf (stderr, "ERROR: empty input\n");
      exit (EXIT_FAILURE);
    }

  fprintf (stderr, "# %lu matches processed\n", count);
}


//--------------------------------------------------------------- ParseArgs ----
void ParseArgs (int argc, char ** argv)
{
  int ch, errflg = 0;
  optarg = NULL;

  while ( !errflg  &&  ((ch = getopt (argc, argv, "h")) != EOF) )
    switch (ch)
      {
      case 'h':
        PrintHelp (argv[0]);
        exit (EXIT_SUCCESS);
        break;

      default:
        errflg ++;
      }

  //-- Mandatory arguments
  if ( optind != argc )
    errflg ++;

  if ( errflg > 0 )
    {
      PrintUsage (argv[0]);
      fprintf (stderr, "Try '%s -h' for more information.\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  //-- Matches coming on stdin or file?
  if ( optind == argc )
    OPT_Matches = stdin;
  else if ( (OPT_Matches = fopen (argv[optind ++], "r")) == NULL )
    {
      fprintf (stderr, "ERROR: Could not open match file %s\n", argv[optind-1]);
      exit (EXIT_FAILURE);
    }
}


//--------------------------------------------------------------- PrintHelp ----
void PrintHelp (const char * s)
{
  PrintUsage (s);
  fprintf (stdout,
"-h            Display help information\n"
           "\n");

  fprintf (stdout,
"  Reads 4 column mummer (-b -c -n -F -maxmatch) output from stdin or\n"
"file and, for each Target/Background sequence pair, outputs a match\n"
"cover for the Target sequence. A match cover is a minimum set of\n"
"intervals such all matches to the Target are contained by some\n"
"interval, and every subinterval matches contiguously to the\n"
"Background. This implies that no interval equals or shadows another,\n"
"but intervals may overlap.\n"
"  It is assumed that the Target and Background have been partitioned\n"
"and fed to mummer as Reference and Query. The Target sequence set is\n"
"assumed to be a subset of the Background, and match covers are only\n"
"generated for the Target sequences. Target self-covers are computed to\n"
"identify unmatchable regions, e.g. N's.\n"
"  Match covers begin with a '>' header line, with the Target and\n"
"Background sequence IDs listed, followed by the interval listing as\n"
"(start, len) pairs.\n"
           "\n");
  return;
}


//-------------------------------------------------------------- PrintUsage ----
void PrintUsage (const char * s)
{
  fprintf (stdout, "\n"
"Usage: %s MATCHES\n"
"   or  %s < MATCHES\n"
           "\n", s, s);
  return;
}

