//------------------------------------------------------------------------------
//   Programmer: Adam M Phillippy, CBCB - University of Maryland
//         File: mcover-gi.cc
//         Date: 09 / 30 / 2005
//
//   Try 'mcover-gi -h' for more information
//   Beware of char*'s in hash_maps
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#include <mcover.hh>
#include <cstring>


//================================================================= Options ====
FILE * OPT_Matches = NULL;


//============================================================ Fuction Decs ====
void OutputCovers (const SeqCoverSetMap_t & tgs);
void ProcessCovers (SeqCoverSetMap_t & tgs,
                    const SeqMap_t & bgs,
                    const SeqMap_t & bgx);
void ParseArgs (int argc, char ** argv);
void PrintHelp (const char * s);
void PrintUsage (const char * s);


//=========================================================== Function Defs ====
int main (int argc, char ** argv)
{
  SeqCoverSetMap_t tgs;           // merged target covers
  SeqMap_t bgs;                // background include
  SeqMap_t bgx;                // background exclude

  ParseArgs (argc, argv);
  ProcessCovers (tgs, bgs, bgx);
  OutputCovers (tgs);

  return EXIT_SUCCESS;
}


//----------------------------------------------------------- ProcessCovers ----
void ProcessCovers (SeqCoverSetMap_t & tgs,
                    const SeqMap_t & bgs,
                    const SeqMap_t & bgx)
{
  char line[PATH_MAX], idT[PATH_MAX], idB[PATH_MAX];
  unsigned long bT, len;
  int scan;

  MatchSet_t::iterator mi;
  SeqCoverSetMap_t::iterator ti;

  SeqMap_t::const_iterator bgs_end = bgs.end();
  SeqMap_t::const_iterator bgx_end = bgx.end();

  bool skip = true;

  *line = *idT = *idB = '\0';

  while ( fgets (line, PATH_MAX, OPT_Matches) )
    {
      //-- If a new Target/Background pair
      if ( *line == '>' )
        {
          scan = sscanf (line + 1, "%s %s", idT, idB);
          assert ( scan == 2 );

          //-- Not a gi cover
          if ( idB[0] != 'g' || idB[1] != 'i' )
            {
              fputs(line, stdout);
              skip = true;
            }
          //-- A gi cover
          else
            {
              //-- New Target
              if ( (ti = tgs.find (idT)) == tgs.end() )
                {
                  //-- Beware: tax and len left unpopulated
                  SeqCoverSet_t tg (strdup(idT), NULL, 0);
                  ti = tgs.insert (make_pair (tg.id, tg)).first;
                }
 
              skip = false;
            }
        }
      //-- Non gi match
      else if ( skip )
        {
          fputs(line, stdout);
        }
      //-- gi match to merge
      else if ( !skip )
        {
          scan = sscanf (line, "%lu %lu", &bT, &len);
          assert ( scan == 2 );

          //-- Merge this new match with the current Target cover
          MergeMatch (Match_t (bT, bT + len - 1), ti->second.matches);
        }
    }
}


//------------------------------------------------------------ OutputCovers ----
void OutputCovers (const SeqCoverSetMap_t & tgs)
{
  //-- Output all merged gi covers
  MatchSet_t::const_iterator mi;
  SeqCoverSetMap_t::const_iterator ti;

  for ( ti = tgs.begin(); ti != tgs.end(); ++ ti )
    {
      const SeqCoverSet_t & tg = ti->second;
      printf (">%s gi|*|\n", tg.id);
      for ( mi = tg.matches.begin(); mi != tg.matches.end(); ++ mi )
        printf ("%lu\t%lu\n", mi->b, mi->e - mi->b + 1);
    }
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
  if ( optind != argc && optind != argc - 1 )
    errflg ++;

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

  fprintf (stdout, "\n"
"  Reads pairwise match cover information, as output by mcover, from\n"
"stdin or file, and merges all GenBank backgrounds into a single match\n"
"cover for each Target. Leaves non GenBank backgrounds untouched. Any\n"
"ID beginning with 'gi' will be assumed to be a GenBank background.\n"
"The resulting merged background cover will have the ID 'gi'\n"
           "\n");

  return;
}


//-------------------------------------------------------------- PrintUsage ----
void PrintUsage (const char * s)
{
  fprintf (stdout,"\n"
"Usage: %s [options] MCOVER\n"
"   or  %s [options] < MCOVER\n"
           "\n", s, s);
  return;
}
