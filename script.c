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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define  INCL_ERRORS
#define  INCL_PM
#define  INCL_DOS
#include <os2.h>

#define  NEWLINE "\n"

#include "title.h"
#include "nls/amptreng.rch"
#include "script.h"
#include "mptrptr.h"
#include "mptrutil.h"
#include "mptrset.h"
#include "macros.h"
#include "dll.h"
#include "debug.h"

static PSZ pszCommentChars = ";";

// Helper routines

PSZ
STRIP( PSZ pszStr )
{
  PSZ p = pszStr;

  if( pszStr == NULL ) {
    return NULL;
  }

  // search first non-whitespace
  while(( *p <= 0x20 ) && ( *p != 0 )) {
    p++;
  }
  if( p != pszStr ) {
    strcpy( pszStr, p );
  }

  // cut off trailing whitespace
  p = pszStr + strlen( pszStr ) - 1;
  while(( *p <= 0x20 ) && ( p >= pszStr )) {
    *p-- = 0;
  }

  return pszStr;
}

/*旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
 *� Name      : LoadPointerFromAnimouseScriptFile                          �
 *� Comment   :                                                            �
 *� Author    : C.Langanke                                                 �
 *� Date      : 25.03.1998                                                 �
 *� Update    : 25.03.1998                                                 �
 *� called by : app                                                        �
 *� calls     : Dos*                                                       �
 *� Input     : ###                                                        �
 *� Tasks     : - read pointer animations from Animouse Script files       �
 *�             - return values and file offsets                           �
 *� returns   : APIRET - OS/2 error code                                   �
 *읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
 */

BOOL
LoadPointerFromAnimouseScriptFile( PSZ pszName, ULONG ulPointerIndex,
                                   PHPOINTER pahptr, PICONINFO paiconinfo,
                                   PHPOINTER phptrStatic, PICONINFO piconinfoStatic,
                                   PULONG paulTimeout, PULONG pulEntries )
{
  APIRET     rc = NO_ERROR;
  FILE*      pfileAnimation = NULL;
  ULONG      ulMaxEntries;

  CHAR       szLine[ 1024];
  CHAR       szAnmPath[ _MAX_PATH];
  CHAR       szPointerFilePath[ _MAX_PATH];
  PSZ        pszFileName;

  CHAR       szPointerFile[ _MAX_PATH];

  PSZ        pszLine;
  PSZ        pszSectionToSearch;

  ULONG      ulLineCount = 0;

  BOOL       fSectionOpened = FALSE;
  BOOL       fSectionClosed = FALSE;
  BOOL       fPointerFound  = FALSE;

  ULONG      xAni = 0;

  PSZ        pszSection;
  PSZ        pszPointerFile;
  PSZ        pszTimeout;
  PSZ        pszInvalidToken;

  PICONINFO  piconinfo;
  PHPOINTER  phptr;
  PULONG     pulTimeout;

  static PSZ pszDelimiter = " \009";

  static PSZ pszHeaderLine      = "Animation_Script";
  static PSZ apszSectionNames[] = { "", "ARROW:", "TEXT:", "WAIT:", "MOVE:", "", "NWSE:", "NESW:", "WE:", "NS:", "ILLEGAL:" };
  static PSZ pszSectionEnd      = "END:";

  #define  SECTION_COUNT  ( sizeof( pszSectionName ) / sizeof( PSZ ))

  do {
    // Parameter pr갽en
    if(( pszName   == NULL )                    ||
       ( ulPointerIndex >=  NUM_OF_SYSCURSORS ) ||
       ( pulEntries     == NULL )) {
      break;
    }

    // Festlegen, welche Section gesucht wird
    xAni = GetAnimouseBasePointerId( ulPointerIndex ) / 100;
    pszSectionToSearch = apszSectionNames[ xAni];

    // MAximale Anzahl sichern und Zielfeld zur갷ksetzen
    ulMaxEntries = *pulEntries;
    *pulEntries  = 0;

    // Datei 봣fnen
    pfileAnimation = fopen( pszName, "r" );
    if( pfileAnimation == NULL ) {
      rc = ERROR_OPEN_FAILED;
      break;
    }

    // Pfad der Datei ermitteln
    rc = DosQueryPathInfo( pszName,
                           FIL_QUERYFULLNAME,
                           szAnmPath,
                           sizeof( szAnmPath ));
    if( rc != NO_ERROR ) {
      break;
    }
    pszFileName = Filespec( szAnmPath, FILESPEC_NAME );
    *( pszFileName - 1 ) = 0;

    // erste Zeile checken
    ulLineCount++;
    if(( pszLine = fgets( szLine, sizeof( szLine ), pfileAnimation )) == NULL ) {
      rc = ERROR_READ_FAULT;
      break;
    }
    STRIP( pszLine );
    if( strcmp( pszLine, pszHeaderLine ) != 0 ) {
      // ### ERROR: INVALID HEADER
      rc = ERROR_INVALID_DATA;
      break;
    }

    // read all lines
    while( !feof( pfileAnimation ) && ( rc == NO_ERROR ) && ( *pulEntries <= ulMaxEntries ))
    {
      // read in a line
      ulLineCount++;
      if(( pszLine = fgets( szLine, sizeof( szLine ), pfileAnimation )) == NULL ) {
        rc = ERROR_READ_FAULT;
        break;
      }

      // strip blanks
      STRIP( pszLine );

      // skip comment lines
      if( strchr( pszCommentChars, *pszLine ) == pszCommentChars ) {
        continue;
      }

      // skip epmtry lines
      if( *pszLine == 0 ) {
        // offene Section wird durch Leerzeile beendet
        if( fSectionOpened ) {
          break;
        } else {
          continue;
        }
      }

      // split line to components
      pszPointerFile  = strtok( pszLine, pszDelimiter );
      pszTimeout      = strtok( NULL,    pszDelimiter );
      pszInvalidToken = strtok( NULL,    pszDelimiter );
      pszSection      = pszPointerFile;

      // Section suchen
      if( !fSectionOpened ) {
        // search section name
        fSectionOpened = ( stricmp( pszSection, pszSectionToSearch ) == 0 );
        continue;
      }

      // Ende der section suchen
      fSectionClosed = ( stricmp( pszSection, pszSectionEnd ) == 0 );
      if( fSectionClosed ) {
        break;
      }

      // Beschreibungszeile f걊 Zeiger auswerten

      // - mehr als zwei elemente in einer Zeile sind ung걄tig
      if( pszInvalidToken != NULL ) {
        // ### ERROR: INVALID TOKEN
        rc = ERROR_INVALID_DATA;
        break;
      }

      // pr갽en, ob Datei existiert
      strcpy( szPointerFile, pszPointerFile );

      if( !FileExist( szPointerFile, FALSE )) {
        // bei relativem Pfad den der ANM Datei hinzuf갾en
        // - daf걊 darf kein Laufwerk angegeben sein !
        //   der erste Vergleich beinhaltet somit die Suche nach ":\\"
        if(( strchr( szPointerFile, ':' ) == NULL ) &&
           ( szPointerFile[ 0] != '\\' )) {
          strcpy( szPointerFilePath, szPointerFile );
          strcpy( szPointerFile, szAnmPath );
          strcat( szPointerFile, "\\" );
          strcat( szPointerFile, szPointerFilePath );
        }

        if( !FileExist( szPointerFile, FALSE )) {
          // ### ERROR: POINTERFILE NOT FOUND pszPointerFile !
          rc = ERROR_FILE_NOT_FOUND;
          break;
        }
      }

      // pr갽en, ob timeoutwert numerisch ist
      if(( pszTimeout != NULL ) && ( !isnumeric( pszTimeout ))) {
        // ### ERROR: TIMOUT VALUE NOT NUMERIC
        rc = ERROR_INVALID_DATA;
        break;
      }

      // statischer Zeiger ?
      if( pszTimeout == NULL ) {
        // statischen Zeiger nur an erster Stelle zulassen
        if( fPointerFound ) {
          // ### ERROR: INVALID STATIC POINTER
          rc = ERROR_INVALID_DATA;
          break;
        }

        // static pointer eintragen
        if( piconinfoStatic != NULL ) {
          fPointerFound                = TRUE;
          piconinfoStatic->cb          = sizeof( ICONINFO );
          piconinfoStatic->fFormat     = ICON_FILE;
          piconinfoStatic->pszFileName = strdup( szPointerFile );

          // Pointer erstelllen
          if( phptrStatic != NULL ) {
            *phptrStatic = CreatePtrFromIconInfo( piconinfoStatic );
          }
        }
      } else {
        // pointer in liste adressieren
        piconinfo  = paiconinfo  + *pulEntries;
        phptr      = pahptr      + *pulEntries;
        pulTimeout = paulTimeout + *pulEntries;
        ( *pulEntries )++;

        // pointer eintragen
        fPointerFound          = TRUE;
        piconinfo->cb          = sizeof( ICONINFO );
        piconinfo->fFormat     = ICON_FILE;
        piconinfo->pszFileName = strdup( szPointerFile );

        // Pointer erstelllen
        *phptr = CreatePtrFromIconInfo( piconinfo );

        // timeout value ermitteln
        *pulTimeout = atol( pszTimeout );
      }
    }   // while (!feof( pfileAnimation) && (rc == NO_ERROR))

    // ist Section unvollst꼗dig ?
    if(( fSectionOpened ) && ( !fSectionClosed )) {
      // ### ERROR: INCOMPLETE SECTION
      rc = ERROR_INVALID_DATA;
      break;
    }
  } while( FALSE );


  // cleanup
  if( pfileAnimation ) {
    fclose( pfileAnimation );
  }
  return rc == NO_ERROR;
}

