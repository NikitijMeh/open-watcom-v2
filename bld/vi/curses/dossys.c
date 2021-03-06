/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2015-2016 The Open Watcom Contributors. All Rights Reserved.
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
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include "vi.h"
#include <curses.h>
#include "win.h"
#include "dosx.h"
#include "osidle.h"


#define PHAR_SCRN_SEL   0x34
extern int PageCnt;

WINDOW *CursesWindow;

int FileSysNeedsCR( int handle )
{
    return( true );
}

/*
 * NewCursor - change cursor to insert mode type
 */
void NewCursor( window_id wid, cursor_type ct )
{
    // could do a curs_set() here

} /* NewCursor */

void StupidCursesWrite( int y, int x, char c )
{
    int row, col;

    getyx( CursesWindow, row, col );
    wmove( CursesWindow, y, x );
    addch( c );
    wmove( CursesWindow, row, col );
}

/*
 * MyBeep - ring beeper
 */
void MyBeep( void )
{
    if( EditFlags.BeepFlag ) {
        beep();
    }

} /* MyBeep */

/*
 * ScreenInit - get screen info
 */
void ScreenInit( void )
{
    size_t size;

    CursesWindow = initscr();
    EditVars.WindMaxWidth = COLS;
    EditVars.WindMaxHeight = LINES;

    size = EditVars.WindMaxWidth * EditVars.WindMaxHeight * sizeof( char_info );
    Scrn = MemAlloc( size );

    // EditFlags.Monocolor = true;
    EditFlags.HasSystemMouse = false;
    curson();

} /* ScreenInit */

/*
 * ScreenFini
 */
void ScreenFini( void )
{
    cursoff();
    endwin();

} /* ScreenFini */

/*
 * ChkExtendedKbd - look for extended keyboard type
 */
void ChkExtendedKbd( void )
{
    EditFlags.ExtendedKeyboard = false;

} /* ChkExtendedKbd */

/*
 * MemSize - return amount of dos memory left (in 16 byte paragraphs)
 */
long MemSize( void )
{
    short       x;

    x = DosMaxAlloc();
#ifdef _M_I86
    return( 16L * (long)x );
#else
    return( 4096L * (long)x );
#endif

} /* MemSize */

/*
 * ScreenPage - set the screen page to active/inactive
 */
void ScreenPage( int page )
{
    PageCnt += page;

} /* ScreenPage */

#if defined( _M_I86 )
    #define KEY_PTR (char _FAR *)0x00400017;
#elif defined( __4G__ )
    #define KEY_PTR (char _FAR *)0x00000417;
#else
    #define KEY_PTR MK_FP( PHAR_SCRN_SEL, 0x417 );
#endif

/*
 * ShiftDown - test if shift key is down
 */
bool ShiftDown( void )
{
    char _FAR   *kptr;

    kptr = KEY_PTR;
    if( kptr[0] & KEY_SHIFT ) {
        return( true );
    }
    return( false );

} /* ShiftDown */

static bool hadCapsLock;

/*
 * TurnOffCapsLock - switch off caps lock
 */
void TurnOffCapsLock( void )
{
    char _FAR   *kptr;

    kptr = KEY_PTR;

    if( kptr[0] & KEY_CAPS_LOCK ) {
        hadCapsLock = true;
        kptr[0] &= ~KEY_CAPS_LOCK;
    } else {
        hadCapsLock = false;
    }

} /* TurnOffCapsLock */

/*
 * DoGetDriveType - get the type of drive A-Z
 */
drive_type DoGetDriveType( int drv )
{
    return( (drive_type)CheckRemovable( (unsigned char)( drv - 'A' + 1 ) ) );

} /* DoGetDriveType */

/*
 * MyDelay - delay a specified number of milliseconds
 */
void MyDelay( int ms )
{
    int         final_ticks;

    final_ticks = ClockTicks + ((ms * 182L + 5000L) / 10000L);
    do {
        ReleaseVMTimeSlice();
    } while( ClockTicks < final_ticks );

} /* MyDelay */

/*
 * SetCursorBlinkRate - set the current blink rate for the cursor
 */
void SetCursorBlinkRate( int cbr )
{
    EditVars.CursorBlinkRate = cbr;

} /* SetCursorBlinkRate */

void MyVioShowBuf( size_t offset, unsigned short nchars )
{
    int         line, column;
    char_info   _FAR *info;
    int         i, x, y;

    if( PageCnt > 0 || EditFlags.Quiet ) {
        return;
    }
    info = Scrn + offset;
    getyx( CursesWindow, y, x );

    line = offset / EditVars.WindMaxWidth;
    column = offset % EditVars.WindMaxWidth;
    wmove( CursesWindow, line, column );
    for( i = 0; i < nchars; i++, info++ ) {
        addch( info->cinfo_char | (info->cinfo_attr << 8) );
    }
    wmove( CursesWindow, y, x );
    refresh();

} /* MyVioShowBuf */

// void            BIOSSetColorRegister( unsigned short, unsigned char, unsigned char, unsigned char );
// void            BIOSGetColorPalette( void _FAR * );
// void            BIOSSetBlinkAttr( unsigned char );
// short           BIOSTestKeyboard( void );
// short           BIOSGetKeyboard( char );
// short           BIOSKeyboardHit( char );
// unsigned char   BIOSGetRowCount( void );
// uint_32         BIOSGetVideoMode( void );
// uint_32         BIOSGetColorRegister( unsigned short );

unsigned short BIOSGetCursorPos( unsigned char type )
{
    short   x, y;

    getyx( CursesWindow, x, y );
    return( ( y << 8 ) | ( x & 0xFF ) );
}

void BIOSSetCursorPos( unsigned char page, unsigned char row, unsigned char col )
{
    wmove( CursesWindow, row, col );
    refresh();
}

unsigned BIOSGetKeyboard( unsigned *scan )
{
    if( scan != NULL )
        *scan = 0;
    return( VI_KEY( NULL ) );
}

/*
 * come up with a correct curses attribute given a color combo and a
 * window.
 */
viattr_t WindowAttr( window *w, vi_color foreground, vi_color background )
{
    unsigned long   attr = A_NORMAL;

    if( w != NULL ) {
        /* reverse video */
        if( background == w->text_color && foreground == w->background_color ) {
            attr |= A_REVERSE;
        } else {
            if( foreground != w->text_color || background != w->background_color ) {
                attr |= A_BOLD;
            }
        }
    }
    return( (viattr_t)( attr >> 8 ) );
}
