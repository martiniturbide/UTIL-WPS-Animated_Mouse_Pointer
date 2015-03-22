/*
    Animated Mouse Pointer
    Copyright (C) 1997 Christian Langanke

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/
#ifndef MPTRAMIM_H
#define MPTRAMIM_H

// some defaults
#define DEFAULT_ANIMATION_TIMEOUT   150L
#define TIMEOUT_MIN                  50L
#define TIMEOUT_MAX                2000L
#define TIMEOUT_STEP                 50L

// prototypes
VOID _Optlink AnimationThread( PVOID Parms );
MRESULT EXPENTRY ObjectWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

HAB  QueryAnimationHab ( void );
HWND QueryAnimationHwnd( void );

#endif // MPTRAMIM_H
