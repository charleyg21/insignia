//------------------------------------------------------------------------------
//   Programmer: Adam M Phillippy, CBCB - University of Maryland
//         File: mcover-intersect.cc
//         Date: 10 / 05 / 2005
//
//   Try 'mcover-intersect -h' for more information
//   Beware of char*'s in hash_maps
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#include <mcover.hh>


//================================================================= Options ====
FILE * OPT_Matches = NULL;
FILE * OPT_TGS     = NULL;
FILE * OPT_BGS     = NULL;
FILE * OPT_BGX     = NULL;


//============================================================ Fuction Decs ====
void GetContext (SeqCoverMap_t & tgs, SeqMap_t & bgs, TaxMap_t & txs);
void IntersectCovers (SeqCoverMap_t & tgs, const TaxMap_t & txs);
void OutputCovers (const SeqCoverMap_t & tgs);
void ProcessCovers (SeqCoverMap_t & tgs, SeqMap_t & bgs, TaxMap_t & txs);
void ParseArgs (int argc, char ** argv);
void PrintHelp (const char * s);
void PrintUsage (const char * s);


//=========================================================== Function Defs ====
int main (int argc, char ** argv)
{
  TaxMap_t txs;                // covers for backgrounds grouped by taxonomy
  SeqCoverMap_t tgs;           // covers for targets
  SeqMap_t bgs;                // background include

  ParseArgs (argc, argv);
  GetContext (tgs, bgs, txs);
  ProcessCovers (tgs, bgs, txs);
  IntersectCovers (tgs, txs);
  OutputCovers (tgs);

  return EXIT_SUCCESS;
}


//-------------------------------------------------------------- GetContext ----
void GetContext (SeqCoverMap_t & tgs, SeqMap_t & bgs, TaxMap_t & txs)
{
  char line[PATH_MAX], id[PATH_MAX], tax[PATH_MAX];
  unsigned long len;
  int scan;
  Seq_t seq;

  TaxMap_t::iterator xi;
  SeqCoverMap_t::iterator ti, xti;
  SeqMap_t::iterator bi;

  //-- Get Target list
  if ( OPT_TGS )
    while ( fgets (line, PATH_MAX, OPT_TGS) )
      {
        scan = sscanf (line, "%s %s %lu", id, tax, &len);
        assert ( scan == 3 );

        if ( (ti = tgs.find (id)) == tgs.end() )
          {
            SeqCover_t tg (strdup(id), strdup(tax), len);
            tgs.insert (make_pair (tg.id, tg));
          }

        //-- Automatically include the Target in the Background
        if ( (bi = bgs.find (id)) == bgs.end() )
          {
            Seq_t bg (strdup(id), strdup(tax), len);
            bgs.insert (make_pair (bg.id, bg));
          }
      }

  //-- Get Background include list
  if ( OPT_BGS )
    while ( fgets (line, PATH_MAX, OPT_BGS) )
      {
        scan = sscanf (line, "%s %s %lu", id, tax, &len);
        assert ( scan == 3 );

        if ( (bi = bgs.find (id)) == bgs.end() )
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

        if ( (bi = bgs.find (id)) != bgs.end() )
          {
            seq = bi->second;
            bgs.erase (bi);
            seq.destroy();    // need to free the char*'s ourselves
          }
      }

  //-- Insert shallow copies of tax->ti into the TaxMap for all bi
  for ( bi = bgs.begin(); bi != bgs.end(); ++ bi )
    {
      xi = txs.insert (make_pair (bi->second.tax, SeqCoverMap_t())).first;
      for ( ti = tgs.begin(); ti != tgs.end(); ++ ti )
        xi->second.insert (make_pair (ti->second.id, ti->second));
    }

  /*
    .REMOVED.
    A Target->Target cover must be provided with the input. This will
    eliminate non-matching N's (and other non-acgt sequence) from being
    dubbed signatures because of uniquess with the background.

  //-- Make all self covers for each Target in TaxMap
  for ( ti = tgs.begin(); ti != tgs.end(); ++ ti )
    {
      xi = txs.find (ti->second.tax);
      xti = xi->second.find (ti->second.id);
      AppendMatch (Match_t (1, ti->second.len), xti->second.matches);
    }
  */
}


//--------------------------------------------------------- IntersectCovers ----
//!   The wording here is not precise, needs to be rewritten...
//!
//!   The intersection algorithm works as follows: Let M be a set of
//!   minimal match covers M[1]..M[N], where a minimal match cover is
//!   an interval set such that intervals may overlap, but no interval
//!   may equal or be shadowed by another. Let b and e represent the
//!   start and end positions of a match (interval). Each match set must
//!   be sorted by b positions. Let m[i] be the i'th match in the sorted
//!   set, and b[i] be the b position of m[i] and e[i] be the end position
//!   of m[i]. As an extention let b[x][i] and e[x][i] be the start and
//!   end positions in the i'th match in the set M[x].  Note that
//!   b[x][i] < b[x][i+1] and b[x][i] != b[x][j] for all i, j and x where
//!   i != j.
//!     Now, our goal is to intersect a set M of minimal match covers
//!   such that our result R is also a minimal match cover, and all
//!   matches in R are shadowed by or equal to a match in M[x] for all x.
//!   In addition, no additional matches could be added to M[x] without
//!   violating the previous conditions.
//!     To accomplish this, we first build a heap H of e[x][0] for all x,
//!   where the minimum e [min(e)] will be the heap top. While building
//!   the heap we identify and retain the maximum b [max(b)]. An
//!   intersection exists between the matches on the heap iff
//!   max(b) <= min(e). If an intersection exists, we append it to our
//!   result R. We then pop the top of the heap m[x][0], where e[x][0] ==
//!   min(e), and push m[x][1] on the heap. If b[x][1] > max(b), we update
//!   max(b) with b[x][1]. The process is now repeated by popping m[y][i]
//!   off the heap, where e[y][i] == min(e). If an intersection exists, we
//!   append it to R, but we must now check that it does not contain and
//!   is not contained by the last match we added to R. Since max(b) and
//!   min(e) never decrease, it is sufficient to check if min(e) has
//!   increased to determine that the current intersection is not shadowed
//!   by the previous. Similarly, it is sufficient to check that max(b)
//!   has increased to determine that the previous intersection is not
//!   shadowed by the current. If either of these cases is not true, the
//!   shadowed match must be removed. max(b) is then updated with b[y][i+1]
//!   if necessary and b[y][i+1] is pushed onto the heap. This process of
//!   popping, intersecting, advancing, pushing is repeated until some
//!   m[x][i] is reached where i == |M[x]|, i.e. we ran out of matches for
//!   some set M[x]. The resulting match set satisfies our goal.
//!     For a n minimal match covers and m total matches, this algorithm
//!   has a worst case complexity of O(m log n).
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void IntersectCovers (SeqCoverMap_t & tgs, const TaxMap_t & txs)
{
  TaxMap_t::const_iterator xi;
  SeqCoverMap_t::const_iterator xti;
  SeqCoverMap_t::iterator ti;

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
          xti = xi->second.find (ti->second.id);

          if ( xti == xi->second.end() || xti->second.matches.empty() )
            break;

          //-- Push new hit iterator, initialized to first hit
          mheap.push_back (make_pair (xti->second.matches.begin(),
                                      xti->second.matches.end()));

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
void ProcessCovers (SeqCoverMap_t & tgs, SeqMap_t & bgs, TaxMap_t & txs)
{
  char line[PATH_MAX], idT[PATH_MAX], idB[PATH_MAX];
  unsigned long bT, len;
  int scan;

  MatchList_t::iterator mi;
  SeqCoverMap_t::iterator ti, xti;
  SeqMap_t::iterator bi;
  TaxMap_t::iterator xi;

  bool skip = true;

  *line = *idT = *idB = '\0';

  while ( fgets (line, PATH_MAX, OPT_Matches) )
    {
      //-- If a new Target/Background pair
      if ( *line == '>' )
        {
          scan = sscanf (line + 1, "%s %s", idT, idB);
          assert ( scan == 2 );

          //-- Intersect this cover?
          if (
              ( (ti = tgs.find (idT)) == tgs.end() )
              ||
              ( (bi = bgs.find (idB)) == bgs.end() )
              ||
              ( (xi = txs.find (bi->second.tax)) == txs.end() )
              ||
              ( (xti = xi->second.find (ti->second.id)) == xi->second.end() )
              )
            {
              skip = true;
            }
          else
            {
              mi = xti->second.matches.begin();
              skip = false;
            }
        }
      //-- If a Match
      else if ( !skip )
        {
          scan = sscanf (line, "%lu %lu", &bT, &len);
          assert ( scan == 2 );

          //-- Merge this new match with the current Tax/Target cover
          mi = MergeMatch (Match_t (bT, bT + len - 1), xti->second.matches, mi);
        }
    }
}


//------------------------------------------------------------ OutputCovers ----
void OutputCovers (const SeqCoverMap_t & tgs)
{
  MatchList_t::const_iterator mi;
  SeqCoverMap_t::const_iterator ti;

  for ( ti = tgs.begin(); ti != tgs.end(); ++ ti )
    {
      const SeqCover_t & tg = ti->second;
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

  if ( !OPT_TGS || !OPT_BGS )
    {
      fprintf (stderr, "ERROR: The -B and -T options are mandatory\n");
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
"stdin or file, and intersects the given Background covers into a\n"
"single match cover for the Target. In order to account for draft\n"
"sequence and multichromosomal organisms, background covers from the\n"
"same taxon are first merged and then intersected with covers from\n"
"other taxons. Match cover headers must include both the Target and\n"
"Background sequence ID (in that order), and the match list must be\n"
"sorted, low to hi, by match position. The Target and Background lists\n"
"must be included on the command line.  ID lists should be a single ID\n"
"per line with the ID appearing at the beginning of the line, followed\n"
"by the taxon ID, and finally the sequence length. If a Background\n"
"exclude list is given, it will be applied to the Background include\n"
"list. The Target will be automatically added to the Background.\n"
"  Important! Target->Target self match covers must be provided by\n"
"the input match cover, so that non-matching Target sequence can be\n"
"excluded (e.g. N's or other non-ACGT sequence).\n"
           "\n");

  return;
}


//------------------------------------------------------------ PrintUsage ----//
void PrintUsage (const char * s)
{
  fprintf (stdout,"\n"
"Usage: %s [options] -T tg -B bg MCOVER\n"
"   or  %s [options] -T tg -B bg < MCOVER\n"
           "\n", s, s);
  return;
}
