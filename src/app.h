/* Copyright 2020 Jaakko Keränen <jaakko.keranen@iki.fi>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#pragma once

/* Application core: event loop, base event processing, audio synth. */

#include <the_Foundation/objectlist.h>
#include <the_Foundation/string.h>
#include <the_Foundation/time.h>

#include "prefs.h"
#include "ui/color.h"

iDeclareType(Bookmarks)
iDeclareType(DocumentWidget)
iDeclareType(GmCerts)
iDeclareType(MimeHooks)
iDeclareType(Visited)
iDeclareType(Window)

/* Command line options strings. */
#define listTabUrls_CommandLineOption   "list-tab-urls;L"

enum iAppDeviceType {
    desktop_AppDeviceType,
    tablet_AppDeviceType,
    phone_AppDeviceType,
};

enum iAppEventMode {
    waitForNewEvents_AppEventMode,
    postedEventsOnly_AppEventMode,
};

enum iUserEventCode {
    command_UserEventCode = 1,
    refresh_UserEventCode = 2,
    asleep_UserEventCode = 3,
};

const iString *execPath_App     (void);
const iString *dataDir_App      (void);
const iString *downloadDir_App  (void);
const iString *debugInfo_App    (void);

int         run_App                     (int argc, char **argv);
void        processEvents_App           (enum iAppEventMode mode);
iBool       handleCommand_App           (const char *cmd);
void        refresh_App                 (void);
iBool       isRefreshPending_App        (void);
uint32_t    elapsedSinceLastTicker_App  (void); /* milliseconds */

iBool               isLandscape_App     (void);
iLocalDef iBool     isPortrait_App      (void) { return !isLandscape_App(); }
enum iAppDeviceType deviceType_App      (void);
iGmCerts *          certs_App           (void);
iVisited *          visited_App         (void);
iBookmarks *        bookmarks_App       (void);
iMimeHooks *        mimeHooks_App       (void);
iDocumentWidget *   document_App        (void);
iObjectList *       listDocuments_App   (void);
iDocumentWidget *   newTab_App          (const iDocumentWidget *duplicateOf, iBool switchToNew);
void                trimCache_App       (void);

const iPrefs *      prefs_App           (void);
iBool               forceSoftwareRender_App(void);
enum iColorTheme    colorTheme_App      (void);
const iString *     schemeProxy_App     (iRangecc scheme);
iBool               willUseProxy_App    (const iRangecc scheme);
const iString *     searchQueryUrl_App  (const iString *queryStringUnescaped);
const iString *     downloadPathForUrl_App(const iString *url, const iString *mime);

typedef void (*iTickerFunc)(iAny *);

iAny *      findWidget_App      (const char *id);
void        addTicker_App       (iTickerFunc ticker, iAny *context);
void        removeTicker_App    (iTickerFunc ticker, iAny *context);
void        postRefresh_App     (void);
void        postCommand_App     (const char *command);
void        postCommandf_App    (const char *command, ...);

iLocalDef void postCommandString_App(const iString *command) {
    if (command) {
        postCommand_App(cstr_String(command));
    }
}

iDocumentWidget *   document_Command    (const char *cmd);

void        openInDefaultBrowser_App    (const iString *url);
void        revealPath_App              (const iString *path);
