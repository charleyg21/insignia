//------------------------------------------------------------------------------
//   Programmer: Adam M Phillippy, CBCB - University of Maryland
//         File: mcover.cc
//         Date: 09 / 07 / 2005
//
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#include <mcover.hh>


//----------------------------------------------------------------- Match_t ----
//! \struct Match_t
//! \brief A match interval with a beginning and end.
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


//------------------------------------------------------------------- Seq_t ----
//! \struct Seq_t
//!
//! Sequence ID, taxonomy ID, and sequence length.
//!
//! \warning Be careful, the id and tax pointers are left for the user to
//! the user to manage and never free'd unless destroy is called.
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Seq_t::destroy()
{
  free (id);
  free (tax);
  id = tax = NULL;
  len = 0;
}


//-------------------------------------------------------------- MergeMatch ----
//! \brief Merges a match with a match list
//!
//! Merges a new match with an existing match list. The match list must be
//! sorted by match start position (low to hi) and the list iterator must point
//! to a match with a smaller or equal start position or end() if no such match
//! exists. The match list must also be a valid minimal match cover, i.e. no
//! match shadows or equals any other match. The resulting match set will obey
//! these constraints. If the new match shadows other matches, those matches
//! will be removed and the new match added. If the new match is shadowed by
//! another match, the new match will not be added. An iterator to the inserted
//! or shadowing match will be returned.
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
MatchList_t::iterator MergeMatch (Match_t match,
                                  MatchList_t & matches,
                                  MatchList_t::iterator mi)
{
  if ( match.empty() ) return mi;   // if match is empty, do nothing

  if ( ! matches.empty() )
    {
      //-- For matches with smaller or equal start positions
      for ( ; mi != matches.end()  &&  mi->b <= match.b; ++ mi )
        if ( mi->e >= match.e )     // if current is shadowed, do nothing
          return mi;
        else if ( mi->b == match.b )
          break;

      //-- For matches with equal or larger start positions
      while ( mi != matches.end()  &&  mi->e <= match.e )
        if ( mi->e <= match.e )
          matches.erase (mi ++);    // if current shadows others, remove them
    }

  return matches.insert(mi,match);
}


//-------------------------------------------------------------- MergeMatch ----
//! \brief Merges a match with a match set
//!
//! Merges a new match with an existing match set. The match list must be
//! sorted by match start position (low to hi). The match list must also be a
//! valid minimal match cover, i.e. no match shadows or equals any other match.
//! The resulting match set will obey these constraints. If the new match
//! shadows other matches, those matches will be removed and the new match
//! added. If the new match is shadowed by a another match, the new match will
//! not be added.
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void MergeMatch (Match_t match, MatchSet_t & matches)
{
  if ( match.empty() ) return;      // if match is empty, no need to insert it

  if ( ! matches.empty() )
    {
      MatchSet_t::iterator mi (matches.lower_bound (match));
      MatchSet_t::iterator mj (mi == matches.begin() ? mi : mi --);

      //-- For matches with smaller or equal start positions
      for ( ; mi != matches.end()  &&  mi->b <= match.b; ++ mi )
        if ( mi->e >= match.e )     // if current is shadowed, do nothing
          return;

      //-- For matches with equal or larger start positions
      while ( mj != matches.end()  &&  mj->e <= match.e )
        matches.erase (mj ++);      // if current shadows others, remove them
    }

  matches.insert (match);
}
