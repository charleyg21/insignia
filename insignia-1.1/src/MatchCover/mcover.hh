//------------------------------------------------------------------------------
//   Programmer: Adam M Phillippy, CBCB - University of Maryland
//         File: mcover.hh
//         Date: 10 / 12 / 2005
//
//   Some structures for dealing with match covers.
//
//------------------------------------------------------------------------------

#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <climits>

#include <set>
#include <list>
#include <vector>
#include <utility>

//-- Include hash_map & hash_set
#ifdef __GNUC__
#if __GNUC__ < 3
  #include <hash_map.h>
  #include <hash_set.h>
  namespace Sgi { using ::hash_map; using ::hash_set; };
  #define HASH std
#elif __GNUC__ == 3
  #include <ext/hash_map>
  #include <ext/hash_set>
  #if __GNUC_MINOR__ == 0
    namespace Sgi = std;               // GCC 3.0
    #define HASH std
  #else
    namespace Sgi = ::__gnu_cxx;       // GCC 3.1 and later
    #define HASH __gnu_cxx
  #endif
#elif __GNUC__ > 3
  #include <ext/hash_map>
  #include <ext/hash_set>
  namespace Sgi = ::__gnu_cxx;         // GCC 4.0 and later
  #define HASH __gnu_cxx
#endif
#else      // ...  there are other compilers, right?
  namespace Sgi = std;
  #define HASH std
#endif

using namespace std;
using namespace HASH;

const char FORWARD = 'F';
const char REVERSE = 'R';



//=================================================================== Types ====

struct StrCmp_t
{
  bool operator() (const char *s1, const char *s2) const
  { return ( !strcmp (s1, s2) ); }
};

struct Match_t
{
  unsigned long b, e;

  Match_t() : b(0), e(0) {};
  Match_t (long begin, long end) : b(begin), e(end) {};
  bool empty() { return ( e < b ); }
};

struct MatchCmp_t
{
  bool operator() (const Match_t &i, const Match_t &j) const
  { return ( i.b < j.b ); }
};

typedef list<Match_t> MatchList_t;
typedef set<Match_t, MatchCmp_t> MatchSet_t;
typedef hash_map<char*, MatchSet_t, hash<char *>, StrCmp_t> MatchSetMap_t;
typedef pair<MatchList_t::const_iterator, MatchList_t::const_iterator> MI_t;

struct MICmp_t
{
  bool operator() (const MI_t & x, const MI_t & y) const
  { return (  x.first->e > y.first->e ); }
};

struct Seq_t
{
  char *id;
  char *tax;
  unsigned long len;

  Seq_t() : id(NULL), tax(NULL), len(0) {}
  Seq_t (char *i, char *t, unsigned long l) : id(i), tax(t), len(l) {}
  void destroy();
};

typedef hash_map<char *, Seq_t, hash<char *>, StrCmp_t> SeqMap_t;
typedef hash_set<const char *, hash<const char *>, StrCmp_t> SeqSet_t;


struct SeqCover_t : public Seq_t
{
  MatchList_t matches;

  SeqCover_t (char *i, char *t, unsigned long l) : Seq_t (i, t, l) {}
};

struct SeqCoverSet_t : public Seq_t
{
  MatchSet_t matches;

  SeqCoverSet_t (char *i, char *t, unsigned long l) : Seq_t (i, t, l) {}
};

typedef hash_map<char*, SeqCover_t, hash<char*>, StrCmp_t> SeqCoverMap_t;
typedef hash_map<char*, SeqCoverSet_t, hash<char*>, StrCmp_t> SeqCoverSetMap_t;

typedef hash_map<char*, SeqCoverMap_t, hash<char*>, StrCmp_t> TaxMap_t;
typedef hash_map<char*, SeqCoverSetMap_t, hash<char*>, StrCmp_t> TaxSetMap_t;


//=============================================================== Functions ====

MatchList_t::iterator MergeMatch (Match_t match,
                                  MatchList_t &matches,
                                  MatchList_t::iterator mi);

void MergeMatch (Match_t match, MatchSet_t &matches);

inline void AppendMatch (Match_t match, MatchList_t &matches)
{ MergeMatch (match, matches, -- matches.end()); }
