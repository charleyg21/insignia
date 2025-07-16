//------------------------------------------------------------------------------
//   Programmer: Adam M Phillippy, CBCB - University of Maryland
//         File: kmer-intersect.cc
//         Date: 10 / 11 / 2005
//
//   Try 'kmer-intersect -h' for more information
//   Beware of char*'s in hash_maps
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#include <mcover.hh>


//================================================================= Options ====
vector<FILE *> OPT_Files;


//------------------------------------------------------------- SeqkCover_t ----
//! \brief A sequence plus a match cover plus a kmer size
//!
//! \warning Be careful, the id and tax pointers are left for the user to
//!          the user to manage and never free'd unless destroy is called.
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
struct SeqkCover_t : public SeqCover_t
{
  unsigned k;

  SeqkCover_t (char * pid, unsigned pk)
    : SeqCover_t (pid, NULL, 0), k(pk)
  { }
};


typedef hash_map<char *, SeqkCover_t, hash<char *>, StrCmp_t> SeqkCoverMap_t;
typedef vector<SeqkCoverMap_t> MapVector_t;


//============================================================ Fuction Decs ====
void IntersectCovers (SeqkCoverMap_t & res, const MapVector_t & maps);
void OutputCovers (const SeqkCoverMap_t & res);
void ProcessCovers (SeqkCoverMap_t & res, MapVector_t & maps);
void ParseArgs (int argc, char ** argv);
void PrintHelp (const char * s);
void PrintUsage (const char * s);


//=========================================================== Function Defs ====
int main (int argc, char ** argv)
{
  MapVector_t maps;
  SeqkCoverMap_t res;

  ParseArgs (argc, argv);
  ProcessCovers (res, maps);
  IntersectCovers (res, maps);
  OutputCovers (res);

  return EXIT_SUCCESS;
}


//--------------------------------------------------------- IntersectCovers ----
void IntersectCovers (SeqkCoverMap_t & tgs, const MapVector_t & txs)
{
  MapVector_t::const_iterator xi;
  SeqkCoverMap_t::const_iterator txi;
  SeqkCoverMap_t::iterator ti;

  vector<MI_t> mheap; // heap of match start positions

  //-- For each Target sequence
  for ( ti = tgs.begin(); ti != tgs.end(); ++ ti )
    {
      MatchList_t::iterator mi = ti->second.matches.begin();
      Match_t curr;
      mheap.clear();

      //-- Add the first hit from each taxon to the heap
      for ( xi = txs.begin(); xi != txs.end(); ++ xi )
        {
          txi = xi->find (ti->second.id);

          if ( txi == xi->end() || txi->second.matches.empty() )
            break;

          //-- Push new hit iterator, initialized to first hit
          mheap.push_back (make_pair (txi->second.matches.begin(),
                                      txi->second.matches.end()));

          //-- Check if new hit has max start pos
          if ( mheap.rbegin()->first->b > curr.b )
            curr.b = mheap.rbegin()->first->b;

          push_heap (mheap.begin(), mheap.end(), MICmp_t());
        }

      //-- If no heap or not all taxons on the heap, no intersection exists
      if ( mheap.empty() || mheap.size() != txs.size() )
        continue;

      //-- Intersect the taxon groups
      while ( true )
        {
          //-- Pop the match with the smallest end position
          curr.e = mheap.begin()->first->e;
          pop_heap (mheap.begin(), mheap.end(), MICmp_t());

          //-- Merge the intersection while taking care of empties and shadows
          mi = MergeMatch (curr, ti->second.matches, mi);

          //-- Advance the popped match, quit if we run out of matches
          if ( ++ mheap.rbegin()->first == mheap.rbegin()->second )
            break;

          //-- Check for a new max start position
          if ( mheap.rbegin()->first->b > curr.b )
            curr.b = mheap.rbegin()->first->b;

          //-- Push the advanced match back onto the heap
          push_heap (mheap.begin(), mheap.end(), MICmp_t());
        }
    }
}


//----------------------------------------------------------- ProcessCovers ----
void ProcessCovers (SeqkCoverMap_t & res, MapVector_t & maps)
{
  char line[PATH_MAX], id[PATH_MAX];
  unsigned long b, l;
  unsigned k;
  int scan;

  vector<FILE *>::const_iterator fp;
  MatchList_t::iterator mi;
  SeqkCoverMap_t::iterator si;

  bool init;

  for ( fp = OPT_Files.begin(); fp != OPT_Files.end(); ++ fp )
    {
      init = false;
      *line = *id = '\0';
      maps.push_back (SeqkCoverMap_t());
      SeqkCoverMap_t & cmap = maps.back();

      while ( fgets (line, PATH_MAX, *fp) )
        {
          if ( *line == '>' )
            {
              scan = sscanf (line + 1, "%s %u", id, &k);
              assert ( scan == 2 );

              //-- Insert new cover for current file
              if ( (si = cmap.find (id)) == cmap.end() )
                {
                  SeqkCover_t sc (strdup (id), k);
                  si = cmap.insert (make_pair (sc.id, sc)).first;
                  res.insert (make_pair (sc.id, sc));
                  mi = si->second.matches.begin();
                }
              else
                {
                  fprintf (stderr,
                    "ERROR: Cannot have duplicate seq ID %s in the same file\n",
                           id);
                  exit (EXIT_FAILURE);
                }

              init = true;
            }
          //-- If a Match
          else if ( init )
            {
              scan = sscanf (line, "%lu %lu", &b, &l);
              assert ( scan == 2 );

              //-- Merge this new match with the current cover
              mi = MergeMatch (Match_t (b, b + l - 1), si->second.matches, mi);
            }
        }
    }
}


//------------------------------------------------------------ OutputCovers ----
void OutputCovers (const SeqkCoverMap_t & res)
{
  SeqkCoverMap_t::const_iterator ri;
  MatchList_t::const_iterator mi;

  for ( ri = res.begin(); ri != res.end(); ++ ri )
    {
      const SeqkCover_t & sc = ri->second;
      printf (">%s %u\n", sc.id, sc.k);
      for ( mi = sc.matches.begin(); mi != sc.matches.end(); ++ mi )
        printf ("%lu\t%lu\n", mi->b, mi->e - mi->b + 1);
    }
}


//---------------------------------------==---------------------- ParseArgs ----
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

  if ( optind == argc )
    errflg ++;

  if ( errflg > 0 )
    {
      PrintUsage (argv[0]);
      fprintf (stderr, "Try '%s -h' for more information.\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  while ( optind != argc )
    {
      OPT_Files.push_back (fopen (argv[optind ++], "r"));
      if ( OPT_Files.back() == NULL )
        {
          fprintf (stderr,
                   "ERROR: Could not open k-mer file %s\n", argv[optind-1]);
          exit (EXIT_FAILURE);
        }
    }
}


//---------------------------------------------==---------------- PrintHelp ----
void PrintHelp (const char * s)
{
  PrintUsage (s);
  fprintf (stdout,
"-h            Display help information\n"
           "\n");

  fprintf (stdout,
"  Intersects k-mer lists as output by unique/common-mer.\n"
           "\n");

  return;
}


//---------------------------------------------==--------------- PrintUsage ----
void PrintUsage (const char * s)
{
  fprintf (stdout,"\n"
"Usage: %s [options] KMERS [KMERS...]\n"
           "\n", s);
  return;
}
