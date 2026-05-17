
#include "../include/bank.h"

static void showWelcomeScreen(void)
{
    int i;
    system("cls"); hideCursor();

    /* Top banner */
    setColor(CLR_ACCENT);
    gotoxy(0,0); printf("\xC9");
    for (i=1;i<CONSOLE_WIDTH-1;i++) printf("\xCD");
    printf("\xBB");

    setColor(CLR_TITLE);    printCentered(1, "  Welcome to  Your Personal Bank  ");
    setColor(CLR_INFO);     printCentered(2, "  Safe  |  Simple  |  Always Here for You  ");
    setColor(CLR_ACCENT);
    gotoxy(0,3); printf("\xC8");
    for (i=1;i<CONSOLE_WIDTH-1;i++) printf("\xCD");
    printf("\xBC");
    resetColor();

    /* Friendly art */
    setColor(CLR_ACCENT);
    printCentered(5,  "  +-------------------------------------------------+");
    printCentered(6,  "  |   $$$$$$$$  |  Your money, your rules.          |");
    printCentered(7,  "  |   $$     $  |  We keep it safe so you           |");
    printCentered(8,  "  |   $$$$$$$   |  can focus on what matters.       |");
    printCentered(9,  "  |   $$     $  |                                   |");
    printCentered(10, "  |   $$$$$$$$  |  Open 24/7 \x03 Just for you.        |");
    printCentered(11, "  +-------------------------------------------------+");
    resetColor();

    printDivider(13);

    /* Menu */
    drawBox(30, 15, 40, 12);
    setColor(CLR_TITLE); printCentered(16, "What would you like to do?"); resetColor();
    printDivider(17);

    gotoxy(33, 19); setColor(CLR_MENU);    printf("  [ 1 ]");
    setColor(CLR_DEFAULT); printf("  Open a New Account");

    gotoxy(33, 21); setColor(CLR_MENU);    printf("  [ 2 ]");
    setColor(CLR_DEFAULT); printf("  Sign In to My Account");

    gotoxy(33, 23); setColor(CLR_MENU);    printf("  [ 3 ]");
    setColor(CLR_DEFAULT); printf("  Exit");

    resetColor();
    printFooter("We're glad you're here  \x03  Press 1, 2, or 3 and hit Enter");

    showCursor();
    gotoxy(33, 27);
    setColor(CLR_WARNING); printf("  Your choice: "); resetColor();
}

int main(void)
{
    setConsoleSize(CONSOLE_WIDTH, CONSOLE_ROWS);
    SetConsoleTitle("My Personal Bank");
    hideCursor();

    while (1) {
        showWelcomeScreen();

        /* FIX-12: readLine avoids stale '\n' */
        char buf[8]; readLine(buf, sizeof(buf));
        int choice;
        if (sscanf(buf, "%d", &choice) != 1) continue;

        switch (choice) {
            case 1: account(); break;
            case 2: login();   break;
            case 3:
                system("cls");
                setColor(CLR_SUCCESS);
                printCentered(13, "  Thanks for banking with us today!  ");
                printCentered(15, "  Your money is safe. See you soon!  \x01  ");
                resetColor();
                Sleep(1800); exit(0);
            default:
                showAlert(28,
                    "  Hmm, that's not one of the options. Try 1, 2, or 3 :)  ", 1);
                Sleep(1200);
        }
    }
    return 0;
}
