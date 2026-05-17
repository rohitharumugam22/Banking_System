
#include "../include/bank.h"

static int usernameExists(const char *uname)
{
    /* [PERSISTENT STORAGE] fread() scans all saved user records */
    FILE *fp = fopen(USER_FILE, "rb");
    if (!fp) return 0;
    struct UserAccount u;
    while (fread(&u, sizeof(u), 1, fp) == 1)  /* FIX-05 */
        if (u.active && strcmp(uname, u.username) == 0) {
            fclose(fp); return 1;
        }
    fclose(fp); return 0;
}

/* Helper: prints a labelled prompt in the form box */
static void fieldLabel(int col, int row, const char *label)
{
    gotoxy(col, row);
    setColor(CLR_INFO);    printf("  %-22s", label);
    setColor(CLR_WARNING); printf(": ");
    resetColor();
}

void account(void)
{
    struct UserAccount u;
    char   confirmPwd[MAX_PASSWORD], tmp[100], accno[ACC_NO_LEN];
    int    row;
    memset(&u, 0, sizeof(u));

    printHeader("  Let's Open Your New Account  ",
                "  It only takes a couple of minutes!  ");

    /* greeting */
    typingEffect(4, 5, CLR_SUCCESS,
        "  Hi there! I'll guide you through setting up your account step by step.", 12);

    /* ── Personal Info box ─────────────────────────────── */
    drawBox(2, 7, 47, 20);
    setColor(CLR_TITLE); gotoxy(4,7); printf(" Tell us about yourself "); resetColor();

    row = 8;
#define REQUIRED(label, buf, maxlen)                        \
    do {                                                    \
        fieldLabel(3, row, (label));                        \
        gotoxy(28, row); readLine((buf), (maxlen));         \
        if ((buf)[0] == '\0') {                             \
            showAlert(row+1,                                \
              "  Oops! This field can't be empty.  ", 1);  \
            Sleep(700); goto RETRY_##buf;                   \
        }                                                   \
    } while(0); RETRY_##buf:

    do { fieldLabel(3,row,"Your First Name");
         gotoxy(28,row); readLine(u.fname,MAX_NAME);
    } while(u.fname[0]=='\0'); row++;

    do { fieldLabel(3,row,"Your Last Name");
         gotoxy(28,row); readLine(u.lname,MAX_NAME);
    } while(u.lname[0]=='\0'); row++;

    do { fieldLabel(3,row,"Father's Name");
         gotoxy(28,row); readLine(u.fathname,MAX_NAME);
    } while(u.fathname[0]=='\0'); row++;

    do { fieldLabel(3,row,"Mother's Name");
         gotoxy(28,row); readLine(u.mothname,MAX_NAME);
    } while(u.mothname[0]=='\0'); row++;

    /* FIX-03: readLine accepts spaces in address */
    do { fieldLabel(3,row,"Home Address");
         gotoxy(28,row); readLine(u.address,MAX_ADDRESS);
    } while(u.address[0]=='\0'); row++;

    do { fieldLabel(3,row,"Account Type");
         gotoxy(28,row);
         setColor(CLR_BORDER); printf("(Savings/Current) "); resetColor();
         gotoxy(28,row); readLine(u.typeaccount,MAX_ACCTYPE);
    } while(u.typeaccount[0]=='\0'); row++;

    /* Phone — FIX-13 */
    do {
        fieldLabel(3,row,"Mobile Number");
        gotoxy(28,row); readLine(u.pnumber,MAX_PHONE);
        if (!isValidPhone(u.pnumber)) {
            showAlert(row+1,
                "  Please enter a valid 10-digit mobile number.  ",1);
            Sleep(900);
        } else break;
    } while(1); row++;

    /* Aadhar — FIX-13 */
    do {
        fieldLabel(3,row,"Aadhar Number");
        gotoxy(28,row); readLine(u.adharnum,MAX_ADHAR);
        if (!isValidAadhar(u.adharnum)) {
            showAlert(row+1,
                "  Aadhar should be exactly 12 digits. Let's try again.  ",1);
            Sleep(900);
        } else break;
    } while(1); row++;

    /* DOB — FIX-13 */
    do {
        fieldLabel(3,row,"Date of Birth");
        gotoxy(28,row);
        setColor(CLR_BORDER); printf("(DD/MM/YYYY) "); resetColor();
        gotoxy(28,row); readLine(tmp,sizeof(tmp));
        if (sscanf(tmp,"%d/%d/%d",&u.date,&u.month,&u.year)==3
            && isValidDOB(u.date,u.month,u.year)) break;
        showAlert(row+1,
            "  That date doesn't look right. Try DD/MM/YYYY, e.g. 15/08/2000  ",1);
        Sleep(900);
    } while(1); row++;

    /* ── Credentials box ───────────────────────────────── */
    drawBox(51, 7, 47, 17);
    setColor(CLR_TITLE); gotoxy(53,7); printf(" Choose Your Login Details "); resetColor();
    int cr = 9;

    /* Username */
    do {
        int i; gotoxy(52,cr); for(i=0;i<45;i++) printf(" ");
        fieldLabel(52,cr,"Pick a username");
        gotoxy(76,cr); readLine(u.username,MAX_USERNAME);
        if (u.username[0]=='\0') {
            showAlert(cr+1,"  Username can't be blank!  ",1); Sleep(600); continue;
        }
        if (usernameExists(u.username)) {
            showAlert(cr+1,
                "  That username is taken. How about something else?  ",1);
            Sleep(900); continue;
        }
        showAlert(cr+1,"  Great choice!  ",0); Sleep(500); break;
    } while(1); cr+=2;

    /* Password — FIX-14: strength enforced */
    do {
        int i; gotoxy(52,cr); for(i=0;i<45;i++) printf(" ");
        fieldLabel(52,cr,"Create a password");
        gotoxy(76,cr); getPasswordInput(u.password,MAX_PASSWORD);
        if (!isStrongPassword(u.password)) {
            showAlert(cr+1,
                "  Use at least 6 characters with a letter and a digit.  ",1);
            Sleep(1000);
        } else {
            showAlert(cr+1,"  Strong password!  ",0); Sleep(500); break;
        }
    } while(1); cr+=2;

    /* Confirm */
    do {
        int i; gotoxy(52,cr); for(i=0;i<45;i++) printf(" ");
        fieldLabel(52,cr,"Confirm password");
        gotoxy(76,cr); getPasswordInput(confirmPwd,MAX_PASSWORD);
        if (strcmp(u.password,confirmPwd)!=0) {
            showAlert(cr+1,
                "  Passwords don't match. Let's try that again.  ",1);
            Sleep(800); cr+=2;
        } else { showAlert(cr+1,"  Matched!  ",0); Sleep(500); break; }
    } while(1); cr+=2;

    /* Opening deposit — FIX-15 */
    double initDep=0;
    do {
        int i; gotoxy(52,cr); for(i=0;i<45;i++) printf(" ");
        fieldLabel(52,cr,"Opening Deposit (Rs.)");
        gotoxy(76,cr); showCursor(); readLine(tmp,sizeof(tmp));
        if (sscanf(tmp,"%lf",&initDep)!=1 || initDep<MIN_BALANCE) {
            char em[90];
            snprintf(em,sizeof(em),
              "  Minimum opening deposit is Rs. %.0f. You're almost there!  ",
              MIN_BALANCE);
            showAlert(cr+1,em,1); Sleep(900);
        } else break;
    } while(1);
    u.balance = initDep;

    /* ── Save — [PERSISTENT STORAGE] fwrite() ─────────── */
    generateAccountNo(accno);
    strncpy(u.accountno, accno, ACC_NO_LEN-1);
    u.accountno[ACC_NO_LEN-1] = '\0';  /* FIX-16 */
    u.active = 1;

    FILE *fp = fopen(USER_FILE, "ab");
    if (!fp) {
        showAlert(28,"  Hmm, we couldn't save your account. Please try again.  ",1);
        getch(); return;
    }
    fwrite(&u, sizeof(u), 1, fp);  /* [PERSISTENT STORAGE] */
    fclose(fp);

    /* Log opening deposit — [PERSISTENT STORAGE] fwrite to transactions.dat */
    logTransaction(u.accountno, u.username, "BANK",
                   initDep, initDep, TXN_DEPOSIT,
                   "Opening deposit");

    accountcreated(accno, u.fname, u.lname);
}

void accountcreated(const char *accno, const char *fname, const char *lname)
{
    printHeader("  You're all set!  ",
                "  Your account is ready to use  ");

    drawBox(22, 6, 56, 16);

    setColor(CLR_SUCCESS);
    printCentered(8,  "\x01  Welcome to the family!  \x01");

    setColor(CLR_INFO);    gotoxy(24,10); printf("  Account Holder  :  ");
    setColor(CLR_TITLE);   printf("%s %s", fname, lname);

    setColor(CLR_INFO);    gotoxy(24,12); printf("  Account Number  :  ");
    setColor(CLR_WARNING); printf("%s", accno);

    setColor(CLR_INFO);    gotoxy(24,14); printf("  Min. Balance    :  ");
    setColor(CLR_BORDER);  printf("Rs. %.0f (we keep this as your reserve)", MIN_BALANCE);

    setColor(CLR_INFO);    gotoxy(24,16); printf("  Status          :  ");
    setColor(CLR_SUCCESS); printf("ACTIVE  \x01");

    setColor(CLR_INFO);    gotoxy(24,18); printf("  Note            :  ");
    setColor(CLR_DEFAULT); printf("Please remember your username and password!");

    resetColor();
    printFooter("Press any key to sign in for the first time . . .");
    animatedProgress("Activating your account", 28, 40);
    getch();
    login();
}
