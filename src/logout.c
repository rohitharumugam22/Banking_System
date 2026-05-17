
#include "../include/bank.h"

void logout(void)
{
    printHeader("  Signing You Out  ","  Just a moment . . .  ");
    drawBox(25,7,50,10);

    setColor(CLR_INFO);
    printCentered(10,"  We're safely ending your session.  ");
    setColor(CLR_BORDER);
    printCentered(11,"  Your account and data are secure.  ");
    resetColor();

    animatedProgress("Signing out", 25, 50);

    setColor(CLR_SUCCESS);
    printCentered(15,"\x01  You've been signed out. See you soon!  \x01");
    resetColor();

    printFooter("Taking you back to the main screen . . .");
    Sleep(1000);
}
