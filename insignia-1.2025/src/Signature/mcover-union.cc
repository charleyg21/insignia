//------------------------------------------------------------------------------
//   Programmer: Adam M Phillippy, CBCB - University of Maryland
//         File: mcover-union.cc
//         Date: 09 / 30 / 2005
//
//   Try 'mcover-union -h' for more information
//   Beware of char*'s in hash_maps
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#include <mcover.hh>


//================================================================= Options ====
FILE * OPT_Matches = NULL;
FILE * OPT_TGS     = NULL;
FILE * OPT_BGS     = NULL;
FILE * OPT_BGX     = NULL;


//============================================================ Fuction Decs ====
void GetContext (SeqCoverSetMap_t & tgs, SeqMap_t & bgs, SeqMap_t & bgx);
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
  GetContext (tgs, bgs, bgx);
  ProcessCovers (tgs, bgs, bgx);
  OutputCovers (tgs);

  return EXIT_SUCCESS;
}


//-------------------------------------------------------------- GetContext ----
void GetContext (SeqCoverSetMap_t & tgs, SeqMap_t & bgs, SeqMap_t & bgx)
{
  char line[PATH_MAX], id[PATH_MAX], tax[PATH_MAX];
  unsigned long len;
  int scan;

  //-- Get Target list
  if ( OPT_TGS )
    while ( fgets (line, PATH_MAX, OPT_TGS) )
      {
        scan = sscanf (line, "%s %s %lu", id, tax, &len);
        assert ( scan == 3 );

        if ( tgs.find (id) == tgs.end() )
          {
            SeqCoverSet_t tg (strdup(id), strdup(tax), len);
            tgs.insert (make_pair (tg.id, tg));
          }
      }

  //-- Get Background include list
  if ( OPT_BGS )
    while ( fgets (line, PATH_MAX, OPT_BGS) )
      {
        scan = sscanf (line, "%s %s %lu", id, tax, &len);
        assert ( scan == 3 );

        if ( bgs.find (id) == bgs.end() )
          {
            Seq_t bg (strdup(id), strdup(tax), len);
            bgs.insert (make_pair (bg.id, bg));
          }
      }

  //-- Exclude Background exclude list from the Background
  if ( OPT_BGX )
    while ( fgets (line, PATH_MAX, OPT_BGX) )
      {
        scan = sscanf (line, "%s %s %lu", id, tax, &len);
        assert ( scan == 3 );

        if ( bgx.find (id) == bgx.end() )
          {
            Seq_t bg (strdup(id), strdup(tax), len);
            bgx.insert (make_pair (bg.id, bg));
          }
      }

  //-- Automatically exclude the Target from the Background
  SeqCoverSetMap_t::iterator ti;
  for ( ti = tgs.begin(); ti != tgs.end(); ++ ti )
    {
      //-- Careful! Not a deep copy of the Target info
      Seq_t bg (ti->second.id, ti->second.tax, ti->second.len);
      bgx.insert (make_pair (bg.id, bg));
    }
}


//----------------------------------------------------------- ProcessCovers ----
void ProcessCovers (SeqCoverSetMap_t & tgs,
                    const SeqMap_t & bgs,
                    const SeqMap_t & bgx)
{
  char line[PATH_MAX], idT[PATH_MAX], idB[PATH_MAX];
  unsigned long bT, len;
  int scan;

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

          //-- Merge this cover?
          if (
              ( (ti = tgs.find (idT)) == tgs.end() )
              ||
              ( bgx.find (idB) != bgx_end )
              ||
              ( OPT_BGS && bgs.find (idB) == bgs_end )
              )
            {
              skip = true;
            }
          else
            {
              skip = false;
            }
        }
      //-- If a Match
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
  MatchSet_t::const_iterator mi;
  SeqCoverSetMap_t::const_iterator ti;

  for ( ti = tgs.begin(); ti != tgs.end(); ++ ti )
    {
      const SeqCoverSet_t & tg = ti->second;
      printf (">%s %lu\n", tg.id, tg.len);
      for ( mi = tg.matches.begin(); mi != tg.matches.end(); ++ mi )
        printf ("%lu\t%lu\n", mi->b, mi->e - mi->b + 1);
    }
}


//--------------------------------------------------------------- ParseArgs ----
void ParseArgs (int argc, char ** argv)
{
  int ch, errflg = 0;
  optarg = NULL;

  while ( !errflg  &&  ((ch = getopt (argc, argv, "B:T:X:h")) != EOF) )
    switch (ch)
      {
      case 'h':
        PrintHelp (argv[0]);
        exit (EXIT_SUCCESS);
        break;

      case 'B':
        if ( (OPT_BGS = fopen (optarg, "r")) == NULL )
          {
            fprintf (stderr,
              "ERROR: Could not open background include list %s\n", optarg);
            errflg ++;
          }
        break;

      case 'T':
        if ( (OPT_TGS = fopen (optarg, "r")) == NULL )
          {
            fprintf (stderr,
              "ERROR: Could not open target list %s\n", optarg);
            errflg ++;
          }
        break;

      case 'X':
        if ( (OPT_BGX = fopen (optarg, "r")) == NULL )
          {
            fprintf (stderr,
             "ERROR: Could not open background exclude list %s\n", optarg);
            errflg ++;
          }
        break;

      default:
        errflg ++;
      }

  //-- Mandatory arguments
  if ( optind != argc && optind != argc - 1 )
    errflg ++;

  if ( ! OPT_TGS )
    {
      fprintf (stderr, "ERROR: The -T option is mandatory\n");
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
"-B            Background include list\n"
"-T            Target sequence list\n"
"-X            Background exclude list\n"
           "\n");

  fprintf (stdout,
"  Reads pairwise match cover information, as output by mcover, from\n"
"stdin or file, and merges the given Background covers into a single\n"
"match cover for the Target. Match cover headers must include both the\n"
"Target and Background sequence ID (in that order), and the match list\n"
"must be sorted, low to hi, by match position. The Target list must be\n"
"included on the command line.  Background include and exclude lists\n"
"are optional, and if none are given all Backgrounds will be merged.\n"
"ID lists should be a single ID per line with the ID appearing at the\n"
"beginning of the line, followed by the taxon ID, and finally the\n"
"sequence length. Targets will be automatically removed from the\n"
"Background.\n"
           "\n");

  return;
}


//-------------------------------------------------------------- PrintUsage ----
void PrintUsage (const char * s)
{
  fprintf (stdout,"\n"
"Usage: %s [options] -T tg MCOVER\n"
"   or  %s [options] -T tg < MCOVER\n"
           "\n", s, s);
  return;
}
