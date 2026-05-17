
#include "../include/bank.h"

#define MAX_ATTEMPTS 3

void login(void)
{
    char username[MAX_USERNAME]={0}, password[MAX_PASSWORD]={0};
    int  attempts=0, found=0;

    while (attempts < MAX_ATTEMPTS) {

        printHeader("  Sign In to Your Account  ",
                    "  Good to see you again!  ");

        drawBox(25, 5, 50, 18);
        setColor(CLR_TITLE); gotoxy(27,5); printf(" Sign In "); resetColor();

        /* Warm greeting changes each attempt */
        gotoxy(27,7);
        setColor(CLR_INFO);
        if      (attempts==0) printf("  Please enter your details below.");
        else if (attempts==1) printf("  No worries, let's try that again.");
        else                  printf("  Last attempt — take it slow!");
        resetColor();

        gotoxy(27,8); setColor(CLR_BORDER);
        printf("  Attempt %d of %d", attempts+1, MAX_ATTEMPTS);
        resetColor();

        printDivider(10);

        /* Username */
        gotoxy(27,12); setColor(CLR_INFO); printf("  Username  : "); resetColor();
        gotoxy(41,12); memset(username,0,sizeof(username));
        readLine(username,MAX_USERNAME);

        /* Password — [SECURITY] masked with * via _getch() */
        gotoxy(27,14); setColor(CLR_INFO); printf("  Password  : "); resetColor();
        gotoxy(41,14); memset(password,0,sizeof(password));
        getPasswordInput(password,MAX_PASSWORD);  /* FIX-02: _getch inside */

        animatedProgress("Checking your details", 18, 35);

        /* [PERSISTENT STORAGE] fread() — validate against saved records */
        FILE *fp = fopen(USER_FILE,"rb");
        if (!fp) {
            printHeader("  Something went wrong  ","");
            typingEffect(20,12,CLR_ERROR,
                "  We couldn't find the user database. Please contact support.", 10);
            printFooter("Press any key to go back . . .");
            getch(); return;
        }
        struct UserAccount u;
        while (fread(&u,sizeof(u),1,fp)==1) {  /* FIX-05 */
            if (u.active &&
                strcmp(username,u.username)==0 &&
                strcmp(password,u.password)==0) { found=1; break; }
        }
        fclose(fp);

        if (found) break;

        attempts++;
        if (attempts < MAX_ATTEMPTS)
            showAlert(25,
                "  That doesn't match our records. Double-check and try again.  ",1);
        Sleep(1000);
    }

    if (found) {
        loginSuccessAnim(username);
        display(username);
    } else {
        /* FIX: dedicated lockout screen */
        printHeader("  Too Many Attempts  ","");
        drawBox(24,8,52,10);
        setColor(CLR_ERROR);
        printCentered(11,"  We had to lock this session for your safety.  ");
        setColor(CLR_DEFAULT);
        printCentered(13,"  Please wait a moment and try again.  ");
        printCentered(14,"  If you forgot your password, contact support.  ");
        resetColor();
        printFooter("Press any key to return to the main screen . . .");
        getch();
    }
}

void loginSuccessAnim(const char *username)
{
    char fname[MAX_NAME]={0}, lname[MAX_NAME]={0}, accno[ACC_NO_LEN]={0};
    char ts[TIMESTAMP_LEN];

    /* [PERSISTENT STORAGE] fread() to get display name */
    FILE *fp = fopen(USER_FILE,"rb");
    if (fp) {
        struct UserAccount u;
        while (fread(&u,sizeof(u),1,fp)==1) {
            if (strcmp(username,u.username)==0) {
                strncpy(fname,u.fname,MAX_NAME-1);
                strncpy(lname,u.lname,MAX_NAME-1);
                strncpy(accno,u.accountno,ACC_NO_LEN-1);
                break;
            }
        }
        fclose(fp);
    }

    printHeader("  You're in!  ","  Authentication successful  ");
    drawBox(25,6,50,14);

    char welcome[80];
    snprintf(welcome,sizeof(welcome),"  Hey %s! Great to see you.", fname[0]?fname:username);
    typingEffect(26,9,CLR_SUCCESS,welcome,18);

    setColor(CLR_INFO);    gotoxy(27,11); printf("  Signed in as  :  ");
    setColor(CLR_TITLE);   printf("%s %s", fname, lname);

    setColor(CLR_INFO);    gotoxy(27,12); printf("  Account No.   :  ");
    setColor(CLR_WARNING); printf("%s", accno);

    getCurrentTimestamp(ts);
    setColor(CLR_INFO);    gotoxy(27,13); printf("  Signed in at  :  ");
    setColor(CLR_DEFAULT); printf("%s", ts);

    setColor(CLR_INFO);    gotoxy(27,15); printf("  Tip           :  ");
    setColor(CLR_BORDER);  printf("Never share your password with anyone.");

    resetColor();
    printFooter("Press any key to go to your dashboard . . .");
    animatedProgress("Loading your dashboard", 22, 35);
    getch();
}
