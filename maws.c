// WoW custom-sounds generator
// Links only against Windows system DLLs;
// Build (MinGW cross-gcc):
//   x86_64-w64-mingw32-gcc -O2 -Wall -Wextra -Wno-format-truncation -mwindows -o MuteWoWSounds.exe maws.c -static -luser32 -lgdi32
#define _WIN32_WINNT  0x0601
#define _RICHEDIT_VER 0x0500
#include <windows.h>
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- Identity / output strings --------------------------------------------
#define APP_TITLE   "Mute WoW Sounds Generator"
#define APP_VERSION "1.5"

#define CSV_NAME    "SoundKitEntry.csv"
#define TARGETS_DIR "SoundTargets"
#define OUTPUT_DIR  "Output"
#define LUA_NAME    "CustomSounds.lua"
#define ERR_NAME    "ErrorLog.txt"

#define TABLE_NAME  "MAWowSoundsCustom"

// ---- Log colours ----------------------------------------------------------
#define COL_BLACK   RGB(0,0,0)
#define COL_GREEN   RGB(26,138,26)
#define COL_ORANGE  RGB(224,138,0)
#define COL_RED     RGB(204,0,0)
#define COL_TEAL    RGB(0,102,110)

// ---- Control IDs / messages -----------------------------------------------
#define ID_START 1001
#define ID_EXIT  1002
#define ID_CLEAR 1003

#define WM_APP_LOG  (WM_APP + 1)
#define WM_APP_DONE (WM_APP + 2)

#define MAX_FILES 256

// ---- UI tuning ------------------------------------------------------------
// Font face and size (in points) for the UI controls and for the log window.
#define UI_FONT       "Segoe UI"
#define UI_FONT_SIZE  10
#define LOG_FONT      "Calibri"
#define LOG_FONT_SIZE 10

// ---- Globals (single-window app) ------------------------------------------
static HINSTANCE g_hInst;
static HWND   g_hMain, g_hStart, g_hExit, g_hClear, g_hVersion, g_hLog;
static HFONT  g_font, g_fontBold, g_logFont;
static HMODULE g_hRichEdit;
static DWORD  g_guiThread;
static HANDLE g_worker;
static volatile LONG g_running = 0;
static volatile LONG g_cancel  = 0;
static char   g_buildDate[16];   // DD/MM/YYYY, derived from the compile date
static BOOL   g_refreshMode = FALSE;   // TRUE when Start has become "Refresh" (CSV missing)

// ===========================================================================
//  In-memory CSV table
//  Flat array of (SoundKitID, FileDataID, seq) sorted by (SoundKitID, seq).
//  `seq` is the original row index, used as a tie-breaker so qsort behaves
//  like a stable sort -- each SoundKitID's FileDataIDs stay in CSV row order.
// ===========================================================================

typedef struct {
    int soundKitId;
    int fileDataId;
    int seq;
} Entry;

static Entry *g_entries = NULL;
static long   g_entryCount = 0;

static int EntryCmp(const void *lhsRaw, const void *rhsRaw) {
    const Entry *lhs = (const Entry *)lhsRaw;
    const Entry *rhs = (const Entry *)rhsRaw;
    if (lhs->soundKitId != rhs->soundKitId) {
        return (lhs->soundKitId < rhs->soundKitId) ? -1 : 1;
    }
    if (lhs->seq != rhs->seq) {
        return (lhs->seq < rhs->seq) ? -1 : 1;
    }
    return 0;
}

static int LoadCsv(const char *path) {
    FILE *file;
    long fileSize;
    char *buffer, *cursor, *bufferEnd;
    long capacity;

    if (g_entries) {
        free(g_entries);
        g_entries = NULL;
        g_entryCount = 0;
    }

    file = fopen(path, "rb");
    if (!file) {
        return 0;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc((size_t)fileSize + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    if (fread(buffer, 1, (size_t)fileSize, file) != (size_t)fileSize) {
        // short read tolerated
    }
    fclose(file);
    buffer[fileSize] = '\0';

    capacity = 1300000; // Midnights file is huge! Start with a big capacity
    g_entries = (Entry *)malloc((size_t)capacity * sizeof(Entry));
    if (!g_entries) {
        free(buffer);
        return 0;
    }

    cursor = buffer;
    bufferEnd = buffer + fileSize;

    while (cursor < bufferEnd && *cursor != '\n') {   // skip header line
        cursor++;
    }
    if (cursor < bufferEnd) {
        cursor++;
    }

    while (cursor < bufferEnd) {
        int fields[6];
        int fieldCount = 0;

        while (cursor < bufferEnd && *cursor != '\n' && fieldCount < 6) {
            int value = 0;
            if (*cursor < '0' || *cursor > '9') {
                break;
            }
            while (cursor < bufferEnd && *cursor >= '0' && *cursor <= '9') {
                value = value * 10 + (*cursor - '0');
                cursor++;
            }
            fields[fieldCount++] = value;
            if (cursor < bufferEnd && *cursor == ',') {
                cursor++;
            }
        }
        while (cursor < bufferEnd && *cursor != '\n') {
            cursor++;
        }
        if (cursor < bufferEnd) {
            cursor++;
        }

        if (fieldCount >= 3) {
            if (g_entryCount >= capacity) {
                capacity *= 2;
                g_entries = (Entry *)realloc(g_entries, (size_t)capacity * sizeof(Entry));
            }
            g_entries[g_entryCount].soundKitId = fields[1];
            g_entries[g_entryCount].fileDataId = fields[2];
            g_entries[g_entryCount].seq        = (int)g_entryCount;
            g_entryCount++;
        }
    }
    free(buffer);

    qsort(g_entries, (size_t)g_entryCount, sizeof(Entry), EntryCmp);
    return 1;
}

// Binary search: first index whose SoundKitID == soundKitId, with *outCount
// entries in the contiguous run. Returns -1 if not found.
static long FindRun(int soundKitId, long *outCount) {
    long low = 0;
    long high = g_entryCount - 1;
    long runLength;

    while (low <= high) {
        long middle = (low + high) / 2;
        if (g_entries[middle].soundKitId < soundKitId) {
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    if (low < g_entryCount && g_entries[low].soundKitId == soundKitId) {
        runLength = 0;
        while (low + runLength < g_entryCount && g_entries[low + runLength].soundKitId == soundKitId) {
            runLength++;
        }
        *outCount = runLength;
        return low;
    }
    *outCount = 0;
    return -1;
}

// ===========================================================================
//  Target files
// ===========================================================================

typedef struct {
    char  label[256];
    int  *ids;
    int   idCount;
} TLine;

typedef struct {
    char   fileName[MAX_PATH];
    TLine *lines;
    int    lineCount;
    int   *errors;
    int    errorCount;
} TFile;

static int ParseTargetFile(const char *path, TFile *targetFile) {
    FILE *file = fopen(path, "r");
    char line[8192];
    int linesCapacity = 16;

    if (!file) {
        return 0;
    }

    targetFile->lines = (TLine *)malloc(linesCapacity * sizeof(TLine));
    targetFile->lineCount = 0;
    targetFile->errors = NULL;
    targetFile->errorCount = 0;

    while (fgets(line, sizeof(line), file)) {
        char *text = line;
        char *dash;
        char *cursor;
        int length, labelLength, idCapacity, idCount;
        TLine *targetLine;

        while (*text == ' ' || *text == '\t') {   // trim left
            text++;
        }
        length = (int)strlen(text);
        while (length > 0 && (text[length-1]=='\n' || text[length-1]=='\r' || text[length-1]==' ' || text[length-1]=='\t')) {
            text[--length] = '\0';                 // trim right
        }
        if (length == 0) {
            continue;
        }
        if (text[0] == '#') {                      // comment line
            continue;
        }

        dash = strchr(text, '-');                  // labels never contain '-'
        if (!dash) {
            continue;
        }

        if (targetFile->lineCount >= linesCapacity) {
            linesCapacity *= 2;
            targetFile->lines = (TLine *)realloc(targetFile->lines, linesCapacity * sizeof(TLine));
        }
        targetLine = &targetFile->lines[targetFile->lineCount];

        labelLength = (int)(dash - text);          // label = text before dash
        while (labelLength > 0 && (text[labelLength-1]==' ' || text[labelLength-1]=='\t')) {
            labelLength--;
        }
        if (labelLength > 255) {
            labelLength = 255;
        }
        memcpy(targetLine->label, text, (size_t)labelLength);
        targetLine->label[labelLength] = '\0';

        idCapacity = 8;
        idCount = 0;
        targetLine->ids = (int *)malloc(idCapacity * sizeof(int));
        cursor = dash + 1;
        while (*cursor) {
            int value;
            while (*cursor && (*cursor < '0' || *cursor > '9')) {   // skip commas/spaces
                cursor++;
            }
            if (!*cursor) {
                break;
            }
            value = 0;
            while (*cursor >= '0' && *cursor <= '9') {
                value = value * 10 + (*cursor - '0');
                cursor++;
            }
            if (idCount >= idCapacity) {
                idCapacity *= 2;
                targetLine->ids = (int *)realloc(targetLine->ids, idCapacity * sizeof(int));
            }
            targetLine->ids[idCount++] = value;
        }
        targetLine->idCount = idCount;
        targetFile->lineCount++;
    }
    fclose(file);
    return 1;
}

static int CmpFileName(const void *lhsRaw, const void *rhsRaw) {
    return strcmp((const char *)lhsRaw, (const char *)rhsRaw);
}

// ===========================================================================
//  Logging (thread-safe via SendMessage marshalling to the GUI thread)
// ===========================================================================

typedef struct {
    const char *text;
    COLORREF    color;
    BOOL        bold;
    BOOL        newline;
} LogReq;

static void DoLog(const LogReq *request) {
    CHARFORMAT2 format;
    int textLength = (int)SendMessageA(g_hLog, WM_GETTEXTLENGTH, 0, 0);

    SendMessageA(g_hLog, EM_SETSEL, (WPARAM)textLength, (LPARAM)textLength);

    ZeroMemory(&format, sizeof(format));
    format.cbSize      = sizeof(format);
    format.dwMask      = CFM_COLOR | CFM_BOLD;
    format.crTextColor = request->color;
    format.dwEffects   = request->bold ? CFE_BOLD : 0;
    SendMessageA(g_hLog, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&format);

    SendMessageA(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)request->text);
    if (request->newline) {
        SendMessageA(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
    }

    SendMessageA(g_hLog, WM_VSCROLL, SB_BOTTOM, 0);
}

static void LogMessage(const char *text, COLORREF color, BOOL bold, BOOL newline) {
    LogReq request;
    request.text = text;
    request.color = color;
    request.bold = bold;
    request.newline = newline;

    if (GetCurrentThreadId() == g_guiThread) {
        DoLog(&request);
    } else {
        SendMessageA(g_hMain, WM_APP_LOG, 0, (LPARAM)&request);   // blocks: &request stays valid
    }
}

// ===========================================================================
//  Output writers
// ===========================================================================

static int WriteLua(const char *path, TFile *files, int fileCount, long totalGroups) {
    FILE *outFile = fopen(path, "wb");
    SYSTEMTIME now;
    int hour12, fileIndex, lineIndex, idIndex;
    long fileDataIndex, emittedGroups = 0;
    const char *amPm;

    if (!outFile) {
        return 0;
    }

    GetLocalTime(&now);
    hour12 = now.wHour % 12;
    if (hour12 == 0) {
        hour12 = 12;
    }
    amPm = (now.wHour < 12) ? "AM" : "PM";

    fprintf(outFile, "-- Created with %s %s, Build Date %s.\r\n", APP_TITLE, APP_VERSION, g_buildDate);
    fprintf(outFile, "-- Creation Date: %02d/%02d/%04d %d:%02d %s -- (day/month/year)\r\n",
            now.wDay, now.wMonth, now.wYear, hour12, now.wMinute, amPm);
    fprintf(outFile, "%s = {\r\n", TABLE_NAME);

    for (fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        TFile *targetFile = &files[fileIndex];
        fprintf(outFile, "   -- File: %s\r\n", targetFile->fileName);

        for (lineIndex = 0; lineIndex < targetFile->lineCount; lineIndex++) {
            TLine *targetLine = &targetFile->lines[lineIndex];
            fprintf(outFile, "   -- %s\r\n", targetLine->label);

            for (idIndex = 0; idIndex < targetLine->idCount; idIndex++) {
                long runCount;
                long runStart = FindRun(targetLine->ids[idIndex], &runCount);
                if (runStart < 0) {                  // unmatched -> error log only
                    continue;
                }

                emittedGroups++;
                fprintf(outFile, "   ");
                for (fileDataIndex = 0; fileDataIndex < runCount; fileDataIndex++) {
                    if (fileDataIndex) {
                        fputc(',', outFile);
                    }
                    fprintf(outFile, "%d", g_entries[runStart + fileDataIndex].fileDataId);
                }
                if (emittedGroups != totalGroups) {  // last element: no comma
                    fputc(',', outFile);
                }
                fprintf(outFile, " -- %d\r\n", targetLine->ids[idIndex]);
            }
        }
    }
    fprintf(outFile, "}\r\n");
    fclose(outFile);
    return 1;
}

static int WriteErrorLog(const char *path, TFile *files, int fileCount) {
    FILE *outFile = fopen(path, "wb");
    int fileIndex, errorIndex;

    if (!outFile) {
        return 0;
    }

    fprintf(outFile, "FileDataID matching errors:\r\n");
    fprintf(outFile, "------------------------\r\n");        // 24 dashes

    for (fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        TFile *targetFile = &files[fileIndex];
        if (targetFile->errorCount == 0) {
            continue;
        }

        fprintf(outFile, "File: %s\r\n", targetFile->fileName);
        fprintf(outFile, "No matches found for: ");
        for (errorIndex = 0; errorIndex < targetFile->errorCount; errorIndex++) {
            if (errorIndex) {
                fprintf(outFile, ", ");
            }
            fprintf(outFile, "'%d'", targetFile->errors[errorIndex]);
        }
        fprintf(outFile, ".\r\n\r\n");
    }
    fprintf(outFile, "\r\n");
    fclose(outFile);
    return 1;
}

// ===========================================================================
//  Paths
// ===========================================================================

static void GetExeDir(char *out, int size) {
    char path[MAX_PATH];
    char *lastSlash;
    GetModuleFileNameA(NULL, path, MAX_PATH);
    lastSlash = strrchr(path, '\\');
    if (lastSlash) {
        *lastSlash = '\0';
    }
    snprintf(out, (size_t)size, "%s", path);
}

// Build date = the compile date (__DATE__, "Mmm DD YYYY") reformatted to DD/MM/YYYY.
// Auto-updates every time the program is recompiled.
static void GetBuildDate(char *out, int size) {
    static const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char monthAbbrev[4] = {0};
    int day = 0, year = 0, monthNumber = 0;
    const char *monthPos;

    sscanf(__DATE__, "%3s %d %d", monthAbbrev, &day, &year);   // __DATE__ skips the pad space
    monthPos = strstr(months, monthAbbrev);
    if (monthPos) {
        monthNumber = (int)((monthPos - months) / 3) + 1;
    }
    snprintf(out, (size_t)size, "%02d/%02d/%04d", day, monthNumber, year);
}

// TRUE if SoundKitEntry.csv exists next to the executable.
static BOOL CsvPresent(void) {
    char exeDir[MAX_PATH], csvPath[MAX_PATH];
    GetExeDir(exeDir, sizeof(exeDir));
    snprintf(csvPath, sizeof(csvPath), "%s\\%s", exeDir, CSV_NAME);
    return GetFileAttributesA(csvPath) != INVALID_FILE_ATTRIBUTES;
}

// ===========================================================================
//  The generation pipeline (runs on the worker thread)
// ===========================================================================

static void FreeTargets(TFile *files, int fileCount) {
    int fileIndex, lineIndex;
    for (fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        for (lineIndex = 0; lineIndex < files[fileIndex].lineCount; lineIndex++) {
            free(files[fileIndex].lines[lineIndex].ids);
        }
        free(files[fileIndex].lines);
        free(files[fileIndex].errors);
    }
}

static int RunGeneration(void) {
    char exeDir[MAX_PATH], csvPath[MAX_PATH], targetsDir[MAX_PATH];
    char outputDir[MAX_PATH], luaPath[MAX_PATH], errorLogPath[MAX_PATH];
    char searchPattern[MAX_PATH], message[512];
    char fileNames[MAX_FILES][MAX_PATH];
    TFile targetFiles[MAX_FILES];
    WIN32_FIND_DATAA findData;
    HANDLE findHandle;
    int fileCount = 0, fileIndex, lineIndex, idIndex, succeeded = 0;
    long totalGroups = 0, totalErrors = 0;

    GetExeDir(exeDir, sizeof(exeDir));
    snprintf(csvPath, sizeof(csvPath),      "%s\\%s", exeDir, CSV_NAME);
    snprintf(targetsDir, sizeof(targetsDir),   "%s\\%s", exeDir, TARGETS_DIR);
    snprintf(outputDir, sizeof(outputDir),    "%s\\%s", exeDir, OUTPUT_DIR);
    snprintf(luaPath, sizeof(luaPath),      "%s\\%s", outputDir, LUA_NAME);
    snprintf(errorLogPath, sizeof(errorLogPath), "%s\\%s", outputDir, ERR_NAME);

    LogMessage("Starting table generation.", COL_GREEN, TRUE, TRUE);

    // --- 1. enumerate + parse target files ------------------------------
    LogMessage("\r\nParsing sound target files.", COL_BLACK, TRUE, TRUE);

    if (GetFileAttributesA(targetsDir) == INVALID_FILE_ATTRIBUTES) {
        LogMessage(TARGETS_DIR " folder not found.", COL_RED, FALSE, TRUE);
        return 0;
    }

    snprintf(searchPattern, sizeof(searchPattern), "%s\\*.txt", targetsDir);
    findHandle = FindFirstFileA(searchPattern, &findData);
    if (findHandle != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && fileCount < MAX_FILES) {
                snprintf(fileNames[fileCount], MAX_PATH, "%s", findData.cFileName);
                fileCount++;
            }
        } while (FindNextFileA(findHandle, &findData));
        FindClose(findHandle);
    }
    qsort(fileNames, (size_t)fileCount, MAX_PATH, CmpFileName);   // alphabetical order

    for (fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        char fullPath[MAX_PATH];
        snprintf(targetFiles[fileIndex].fileName, MAX_PATH, "%s", fileNames[fileIndex]);

        snprintf(message, sizeof(message), "File: %s ", targetFiles[fileIndex].fileName);
        LogMessage(message, COL_BLACK, FALSE, FALSE);

        snprintf(fullPath, sizeof(fullPath), "%s\\%s", targetsDir, fileNames[fileIndex]);
        ParseTargetFile(fullPath, &targetFiles[fileIndex]);

        LogMessage("...done!", COL_GREEN, FALSE, TRUE);
    }

    // --- 2. load the CSV ------------------------------------------------
    LogMessage("\r\nLoading SoundKitEntry data ", COL_BLACK, TRUE, FALSE);
    if (GetFileAttributesA(csvPath) == INVALID_FILE_ATTRIBUTES) {
        LogMessage("...not found!", COL_RED, FALSE, TRUE);
        FreeTargets(targetFiles, fileCount);
        return 0;
    }
    if (!LoadCsv(csvPath)) {
        LogMessage("...failed to read!", COL_RED, FALSE, TRUE);
        FreeTargets(targetFiles, fileCount);
        return 0;
    }
    LogMessage("...done!", COL_GREEN, FALSE, TRUE);

    // --- 3. match -------------------------------------------------------
    LogMessage("\r\nMatching FileDataIDs:", COL_BLACK, TRUE, TRUE);

    for (fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        TFile *targetFile = &targetFiles[fileIndex];
        int errorCapacity = 16;
        targetFile->errors = (int *)malloc(errorCapacity * sizeof(int));
        targetFile->errorCount = 0;

        snprintf(message, sizeof(message), "File: %s ", targetFile->fileName);
        LogMessage(message, COL_BLACK, FALSE, FALSE);

        for (lineIndex = 0; lineIndex < targetFile->lineCount; lineIndex++) {
            TLine *targetLine = &targetFile->lines[lineIndex];
            for (idIndex = 0; idIndex < targetLine->idCount; idIndex++) {
                long runCount;
                long runStart = FindRun(targetLine->ids[idIndex], &runCount);
                if (runStart < 0) {
                    if (targetFile->errorCount >= errorCapacity) {
                        errorCapacity *= 2;
                        targetFile->errors = (int *)realloc(targetFile->errors, errorCapacity * sizeof(int));
                    }
                    targetFile->errors[targetFile->errorCount++] = targetLine->ids[idIndex];
                } else {
                    totalGroups++;
                }
            }
        }

        if (targetFile->errorCount == 0) {
            LogMessage("...done!", COL_GREEN, FALSE, TRUE);
        } else {
            totalErrors += targetFile->errorCount;
            snprintf(message, sizeof(message), "...done with (%d) error(s).", targetFile->errorCount);
            LogMessage(message, COL_ORANGE, FALSE, TRUE);
        }
    }

    // --- 4. write outputs ----------------------------------------------
    CreateDirectoryA(outputDir, NULL);   // harmless if it already exists

    snprintf(message, sizeof(message), "\r\nWriting '%s'.", LUA_NAME);
    LogMessage(message, COL_GREEN, FALSE, TRUE);
    if (!WriteLua(luaPath, targetFiles, fileCount, totalGroups)) {
        snprintf(message, sizeof(message), "Could not write %s.", LUA_NAME);
        LogMessage(message, COL_RED, FALSE, TRUE);
        goto done;
    }

    snprintf(message, sizeof(message), "\r\nWriting error log -> %s.", ERR_NAME);
    LogMessage(message, totalErrors ? COL_RED : COL_GREEN, TRUE, TRUE);
    if (!WriteErrorLog(errorLogPath, targetFiles, fileCount)) {
        snprintf(message, sizeof(message), "Could not write %s.", ERR_NAME);
        LogMessage(message, COL_RED, FALSE, TRUE);
        goto done;
    }

    LogMessage("\r\nComplete!", COL_GREEN, TRUE, TRUE);
    succeeded = 1;

done:
    FreeTargets(targetFiles, fileCount);
    if (g_entries) {
        free(g_entries);
        g_entries = NULL;
        g_entryCount = 0;
    }
    return succeeded;
}

static DWORD WINAPI WorkerProc(LPVOID param) {
    (void)param;
    RunGeneration();
    PostMessage(g_hMain, WM_APP_DONE, 0, 0);
    return 0;
}

// ===========================================================================
//  Window
// ===========================================================================

static void CreateControls(HWND hwnd) {
    LOGFONTA boldFontInfo;
    HFONT fallbackFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    char versionText[64];
    HDC hdc = GetDC(hwnd);
    int uiFontHeight  = -MulDiv(UI_FONT_SIZE,  GetDeviceCaps(hdc, LOGPIXELSY), 72);
    int logFontHeight = -MulDiv(LOG_FONT_SIZE, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(hwnd, hdc);

    g_font = CreateFontA(uiFontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                         OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                         DEFAULT_PITCH | FF_DONTCARE, UI_FONT);
    if (!g_font) {
        g_font = fallbackFont;
    }

    GetObjectA(g_font, sizeof(boldFontInfo), &boldFontInfo);
    boldFontInfo.lfWeight = FW_BOLD;
    g_fontBold = CreateFontIndirectA(&boldFontInfo);

    g_logFont = CreateFontA(logFontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                            DEFAULT_PITCH | FF_DONTCARE, LOG_FONT);
    if (!g_logFont) {
        g_logFont = g_font;
    }

    g_hStart   = CreateWindowA("BUTTON", "Start", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,
                               0,0,0,0, hwnd, (HMENU)ID_START, g_hInst, NULL);
    g_hExit    = CreateWindowA("BUTTON", "Exit",  WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
                               0,0,0,0, hwnd, (HMENU)ID_EXIT, g_hInst, NULL);
    g_hClear   = CreateWindowA("BUTTON", "Clear", WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,
                               0,0,0,0, hwnd, (HMENU)ID_CLEAR, g_hInst, NULL);
    snprintf(versionText, sizeof(versionText), "%s - %s", APP_VERSION, g_buildDate);
    g_hVersion = CreateWindowA("STATIC", versionText, WS_CHILD|WS_VISIBLE|SS_CENTER,
                               0,0,0,0, hwnd, NULL, g_hInst, NULL);
    g_hLog     = CreateWindowExA(WS_EX_CLIENTEDGE, "RICHEDIT50W", "",
                               WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL,
                               0,0,0,0, hwnd, NULL, g_hInst, NULL);

    SendMessageA(g_hStart,   WM_SETFONT, (WPARAM)g_font,     TRUE);
    SendMessageA(g_hExit,    WM_SETFONT, (WPARAM)g_font,     TRUE);
    SendMessageA(g_hClear,   WM_SETFONT, (WPARAM)g_font,     TRUE);
    SendMessageA(g_hVersion, WM_SETFONT, (WPARAM)g_fontBold, TRUE);
    SendMessageA(g_hLog,     WM_SETFONT, (WPARAM)g_logFont,  TRUE);

    SendMessageA(g_hLog, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(255,255,255));
    SetFocus(g_hStart);
}

static void LayoutControls(HWND hwnd, int width, int height) {
    int padding = 8, buttonWidth = 70, buttonHeight = 24, rowY = 8;
    int midLeft  = padding + 2 * (buttonWidth + 6);
    int midRight = width - padding - buttonWidth - 6;

    (void)hwnd;
    MoveWindow(g_hStart,   padding,                        rowY,     buttonWidth, buttonHeight, TRUE);
    MoveWindow(g_hExit,    padding + buttonWidth + 6,      rowY,     buttonWidth, buttonHeight, TRUE);
    MoveWindow(g_hVersion, midLeft,                        rowY + 5, midRight - midLeft, buttonHeight, TRUE);
    MoveWindow(g_hClear,   width - padding - buttonWidth,  rowY,     buttonWidth, buttonHeight, TRUE);
    MoveWindow(g_hLog,     padding, rowY + buttonHeight + 8,
               width - 2 * padding, height - (rowY + buttonHeight + 8) - padding, TRUE);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    char message[300];

    switch (msg) {

    case WM_CREATE: {
        CreateControls(hwnd);
        LogMessage("Ready!", COL_BLACK, TRUE, TRUE);

        if (!CsvPresent()) {
            snprintf(message, sizeof(message), "Warning: %s not found next to the executable.", CSV_NAME);
            LogMessage(message, COL_RED, FALSE, TRUE);
            SetWindowTextA(g_hStart, "Refresh");   // click to re-check for the file
            g_refreshMode = TRUE;
        }
        return 0;
    }

    case WM_SIZE: {
        LayoutControls(hwnd, LOWORD(lParam), HIWORD(lParam));
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        if ((HWND)lParam == g_hVersion) {
            SetTextColor((HDC)wParam, COL_TEAL);
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }
        break;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_START: {
            if (g_refreshMode) {
                // Acting as "Refresh": re-check for the CSV instead of generating.
                if (CsvPresent()) {
                    snprintf(message, sizeof(message), "%s found.", CSV_NAME);
                    LogMessage(message, COL_GREEN, FALSE, TRUE);
                    SetWindowTextA(g_hStart, "Start");
                    g_refreshMode = FALSE;
                } else {
                    snprintf(message, sizeof(message), "Still no %s next to the executable.", CSV_NAME);
                    LogMessage(message, COL_RED, FALSE, TRUE);
                }
                return 0;
            }
            if (InterlockedCompareExchange(&g_running, 1, 0) == 0) {
                g_cancel = 0;
                EnableWindow(g_hStart, FALSE);
                g_worker = CreateThread(NULL, 0, WorkerProc, NULL, 0, NULL);
            }
            return 0;
        }
        case ID_CLEAR: {
            SetWindowTextA(g_hLog, "");
            LogMessage("Ready!", COL_BLACK, TRUE, TRUE);
            return 0;
        }
        case ID_EXIT: {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
        }
        break;
    }

    case WM_APP_LOG: {
        DoLog((const LogReq *)lParam);
        return 0;
    }

    case WM_APP_DONE: {
        if (g_worker) {
            CloseHandle(g_worker);
            g_worker = NULL;
        }
        g_running = 0;
        EnableWindow(g_hStart, TRUE);
        return 0;
    }

    case WM_CLOSE: {
        if (g_running && g_worker) {
            g_cancel = 1;
            // Pump messages while waiting so the worker's logging SendMessage
            // calls can be serviced -- otherwise we'd deadlock.
            while (WaitForSingleObject(g_worker, 50) == WAIT_TIMEOUT) {
                MSG queuedMsg;
                while (PeekMessage(&queuedMsg, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&queuedMsg);
                    DispatchMessage(&queuedMsg);
                }
            }
        }
        DestroyWindow(hwnd);
        return 0;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR commandLine, int showCmd) {
    WNDCLASSA windowClass;
    MSG msg;

    (void)hPrevInstance;
    (void)commandLine;
    g_hInst = hInstance;
    g_guiThread = GetCurrentThreadId();

    GetBuildDate(g_buildDate, sizeof(g_buildDate));   // before any control uses it

    g_hRichEdit = LoadLibraryA("Msftedit.dll");   // registers the RICHEDIT50W class

    ZeroMemory(&windowClass, sizeof(windowClass));
    windowClass.lpfnWndProc   = WndProc;
    windowClass.hInstance     = hInstance;
    windowClass.lpszClassName = "CsvToolMain";
    windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassA(&windowClass);

    g_hMain = CreateWindowExA(0, "CsvToolMain", APP_TITLE,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,   // fixed window, only the close box
        CW_USEDEFAULT, CW_USEDEFAULT, 432, 322,
        NULL, NULL, hInstance, NULL);

    ShowWindow(g_hMain, showCmd);
    UpdateWindow(g_hMain);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (!IsDialogMessage(g_hMain, &msg)) {   // Tab navigation + Enter = default button
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (g_hRichEdit) {
        FreeLibrary(g_hRichEdit);
    }
    return (int)msg.wParam;
}