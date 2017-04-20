/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  Implementation of strnicmp() and wcsnicmp().
*
****************************************************************************/


#include "widechar.h"
#include "variety.h"
#include <ctype.h>
#include <string.h>
#include "riscstr.h"

/* return <0 if s<t, 0 if s==t, >0 if s>t */


#if defined(__RISCSTR__) && defined(__WIDECHAR__)
 int __simple__wcsnicmp( const CHAR_TYPE *s, const CHAR_TYPE *t, size_t n )
#else
 _WCRTLINK int __F_NAME(strnicmp,_wcsnicmp)( const CHAR_TYPE *s, const CHAR_TYPE *t, size_t n )
#endif
{
    UCHAR_TYPE      c1;
    UCHAR_TYPE      c2;

    for( ; n > 0; --n ) {
        c1 = *s;
        c2 = *t;
        if( IS_ASCII_CHAR( c1 ) && IS_ASCII_CHAR( c2 ) ) {
            if( c1 >= STRING( 'A' ) && c1 <= STRING( 'Z' ) )
                c1 += STRING( 'a' ) - STRING( 'A' );
            if( c2 >= STRING( 'A' ) && c2 <= STRING( 'Z' ) ) {
                c2 += STRING( 'a' ) - STRING( 'A' );
            }
        }
        if( c1 != c2 )
            return( c1 - c2 );      /* less than or greater than */
        if( c1 == NULLCHAR )
            break;          /* equal */
        ++s;
        ++t;
    }
    return( 0 );            /* equal */
}
