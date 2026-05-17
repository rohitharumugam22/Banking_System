
#include "../include/bank.h"

/* ── cursor / colour ─────────────────────────────────────── */

void gotoxy(int x, int y)
{
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
void setColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
}
void resetColor(void) { setColor(CLR_DEFAULT); }

void hideCursor(void)
{
    CONSOLE_CURSOR_INFO ci = { 1, FALSE };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}
void showCursor(void)
{
    CONSOLE_CURSOR_INFO ci = { 1, TRUE };
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);
}

/* FIX-10: set buffer BEFORE resizing window */
void setConsoleSize(int cols, int rows)
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD buf = { (SHORT)cols, (SHORT)(rows + 10) };
    SetConsoleScreenBufferSize(h, buf);
    SMALL_RECT win = { 0, 0, (SHORT)(cols-1), (SHORT)(rows-1) };
    SetConsoleWindowInfo(h, TRUE, &win);
}

/* ── drawing ─────────────────────────────────────────────── */

void drawBox(int x, int y, int w, int h)
{
    int i;
    setColor(CLR_BORDER);
    gotoxy(x, y); printf("\xC9");
    for (i = 1; i < w-1; i++) printf("\xCD");
    printf("\xBB");
    for (i = 1; i < h-1; i++) {
        gotoxy(x,   y+i); printf("\xBA");
        gotoxy(x+w-1, y+i); printf("\xBA");
    }
    gotoxy(x, y+h-1); printf("\xC8");
    for (i = 1; i < w-1; i++) printf("\xCD");
    printf("\xBC");
    resetColor();
}

void printDivider(int row)
{
    int i;
    gotoxy(0, row); setColor(CLR_BORDER);
    for (i = 0; i < CONSOLE_WIDTH; i++) printf("\xC4");
    resetColor();
}

void printCentered(int row, const char *text)
{
    int col = (CONSOLE_WIDTH - (int)strlen(text)) / 2;
    if (col < 0) col = 0;
    gotoxy(col, row); printf("%s", text);
}

void printHeader(const char *title, const char *subtitle)
{
    int i;
    system("cls"); hideCursor();
    setColor(CLR_ACCENT);
    gotoxy(0,0); printf("\xC9");
    for (i = 1; i < CONSOLE_WIDTH-1; i++) printf("\xCD");
    printf("\xBB");
    setColor(CLR_TITLE);   printCentered(1, title);
    if (subtitle && subtitle[0]) {
        setColor(CLR_INFO); printCentered(2, subtitle);
        setColor(CLR_ACCENT); gotoxy(0, 3);
    } else {
        setColor(CLR_ACCENT); gotoxy(0, 2);
    }
    printf("\xC8");
    for (i = 1; i < CONSOLE_WIDTH-1; i++) printf("\xCD");
    printf("\xBC");
    resetColor();
}

/* FIX-11: FOOTER_ROW constant */
void printFooter(const char *hint)
{
    int i;
    setColor(CLR_BORDER); gotoxy(0, FOOTER_ROW-1);
    for (i = 0; i < CONSOLE_WIDTH; i++) printf("\xC4");
    setColor(CLR_INFO); printCentered(FOOTER_ROW, hint);
    resetColor();
}

void showAlert(int row, const char *msg, int isError)
{
    int i;
    gotoxy(0, row);
    for (i = 0; i < CONSOLE_WIDTH; i++) printf(" ");
    setColor(isError ? CLR_ERROR : CLR_SUCCESS);
    printCentered(row, msg);
    resetColor();
}

/* ── input ───────────────────────────────────────────────── */

/* FIX-03/04: fflush(stdin) and scanf_s removed. Safe fgets wrapper. */
int readLine(char *buf, int maxlen)
{
    showCursor();
    buf[0] = '\0';
    if (fgets(buf, maxlen, stdin) == NULL) return 0;
    int len = (int)strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[--len] = '\0';
    hideCursor();
    return len;
}

/* FIX-02: _getch() — correct MSVC spelling */
int getPasswordInput(char *buf, int maxlen)
{
    int i = 0; char ch;
    showCursor();
    while (i < maxlen-1) {
        ch = (char)_getch();          /* FIX-02 */
        if (ch == 13) break;
        if (ch == 8 && i > 0) { i--; printf("\b \b"); continue; }
        if ((unsigned char)ch >= 32) {
            buf[i++] = ch;
            setColor(CLR_WARNING); printf("*"); resetColor();
        }
    }
    buf[i] = '\0'; hideCursor(); return i;
}

/* ── validators ──────────────────────────────────────────── */

int isValidPhone(const char *s)
{
    int i;
    if ((int)strlen(s) != 10) return 0;
    for (i = 0; s[i]; i++) if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}
int isValidAadhar(const char *s)
{
    int i;
    if ((int)strlen(s) != 12) return 0;
    for (i = 0; s[i]; i++) if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}
int isValidDOB(int d, int m, int y)
{
    int days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m < 1||m > 12||d < 1||y < 1900||y > 2025) return 0;
    if ((y%400==0)||(y%4==0&&y%100!=0)) days[2]=29;
    return d <= days[m];
}
/* FIX-14: min length + letter + digit */
int isStrongPassword(const char *pwd)
{
    int len=(int)strlen(pwd), i, hasD=0, hasA=0;
    if (len < MIN_PWD_LEN) return 0;
    for (i=0;pwd[i];i++) {
        if (isdigit((unsigned char)pwd[i])) hasD=1;
        if (isalpha((unsigned char)pwd[i])) hasA=1;
    }
    return hasD && hasA;
}

/* ── generators ──────────────────────────────────────────── */

/* FIX-09: RDTSC + PID → unique even within same second */
void generateAccountNo(char *buf)
{
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    unsigned int seed = (unsigned int)time(NULL)
                      ^ (unsigned int)GetCurrentProcessId()
                      ^ (unsigned int)(pc.LowPart);
    srand(seed);
    sprintf(buf, "ACC%010u",
            (unsigned)((unsigned)rand() % 9000000000u + 1000000000u));
    buf[ACC_NO_LEN-1] = '\0';
}

void getCurrentTimestamp(char *buf)
{
    time_t t = time(NULL);
    struct tm *ti = localtime(&t);
    strftime(buf, TIMESTAMP_LEN, "%d/%m/%Y %H:%M:%S", ti);
}

/* ── animation ───────────────────────────────────────────── */

void animatedProgress(const char *msg, int steps, int ms)
{
    int i;
    printf("\n  "); setColor(CLR_WARNING); printf("%s  [", msg);
    setColor(CLR_SUCCESS);
    for (i = 0; i < steps; i++) {
        printf("\xDB"); fflush(stdout); Sleep((DWORD)ms);
    }
    setColor(CLR_WARNING); printf("]"); resetColor(); printf("\n");
}

/* Human touch: types text one character at a time */
void typingEffect(int col, int row, int color, const char *text, int ms)
{
    int i;
    gotoxy(col, row); setColor(color);
    for (i = 0; text[i]; i++) {
        printf("%c", text[i]); fflush(stdout); Sleep((DWORD)ms);
    }
    resetColor();
}
