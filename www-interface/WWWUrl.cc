
// (c) COPYRIGHT URI/MIT 1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// Implementation for WWWUrl. See WWWByte.cc
//
// 4/7/99 jhrg

// $Log: WWWUrl.cc,v $
// Revision 1.1  1999/04/20 00:21:05  jimg
// First version
//

#ifdef __GNUG__
#pragma implementation
#endif

#include <String.h>

#include "WWWUrl.h"

WWWUrl *
NewUrl(const String &n)
{
    return new WWWUrl(n);
}

WWWUrl::WWWUrl(const String &n) : WWWStr(n)
{
}

BaseType *
WWWUrl::ptr_duplicate()
{
    return new WWWUrl(*this);
}