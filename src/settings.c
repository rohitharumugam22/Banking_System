
#include "../include/bank.h"

typedef void (*UserModifier)(struct UserAccount *);

static int rewriteUser(const char *username, UserModifier fn)
{
    FILE *fp=fopen(USER_FILE,"r+b");
    if (!fp) return 0;
    struct UserAccount u; long pos; int ok=0;
    while(1){                        /* FIX-05: fread loop */
        pos=ftell(fp);
        if(fread(&u,sizeof(u),1,fp)!=1) break;
        if(u.active&&strcmp(username,u.username)==0){
            fn(&u);
            fseek(fp,pos,SEEK_SET);
            fwrite(&u,sizeof(u),1,fp);  /* [PERSISTENT STORAGE] */
            fflush(fp); ok=1; break;
        }
    }
    fclose(fp); return ok;
}

/* ── Change Password ──────────────────────────────────────── */
static char g_newPwd[MAX_PASSWORD];
static void applyPwd(struct UserAccount *u)
{
    strncpy(u->password,g_newPwd,MAX_PASSWORD-1);
    u->password[MAX_PASSWORD-1]='\0';
}

static void changePassword(char username[])
{
    char oldPwd[MAX_PASSWORD]={0}, newPwd[MAX_PASSWORD]={0}, conf[MAX_PASSWORD]={0};

    printHeader("  Change Your Password  ","  Keep it secure — don't share it with anyone  ");
    drawBox(22,6,56,18);
    setColor(CLR_TITLE); gotoxy(24,6); printf(" Password Change "); resetColor();

    typingEffect(24,8,CLR_INFO,
        "  First, let us verify it's really you.", 12);

    gotoxy(24,10); setColor(CLR_INFO); printf("  Current password  : "); resetColor();
    gotoxy(46,10); getPasswordInput(oldPwd,MAX_PASSWORD);

    FILE *fp=fopen(USER_FILE,"rb"); int ok=0;
    if (fp){
        struct UserAccount u;
        while(fread(&u,sizeof(u),1,fp)==1)
            if(u.active&&strcmp(username,u.username)==0&&
               strcmp(oldPwd,u.password)==0){ok=1;break;}
        fclose(fp);
    }
    if (!ok){
        showAlert(26,"  That password doesn't match what we have on file.  ",1);
        Sleep(1400); display(username); return;
    }
    showAlert(11,"  Verified! Now choose a new password.  ",0); Sleep(700);

    int cr=13;
    do {
        int i; gotoxy(24,cr); for(i=0;i<52;i++) printf(" ");
        gotoxy(24,cr);
        setColor(CLR_INFO); printf("  New password      : "); resetColor();
        gotoxy(46,cr); getPasswordInput(newPwd,MAX_PASSWORD);
        if (!isStrongPassword(newPwd)){
            showAlert(cr+1,
                "  Please use at least 6 characters, including a letter and a digit.  ",1);
            Sleep(1000);
        } else { showAlert(cr+1,"  Looks strong!  ",0); Sleep(500); break; }
    } while(1); cr+=2;

    do {
        int i; gotoxy(24,cr); for(i=0;i<52;i++) printf(" ");
        gotoxy(24,cr);
        setColor(CLR_INFO); printf("  Confirm new password: "); resetColor();
        gotoxy(48,cr); getPasswordInput(conf,MAX_PASSWORD);
        if (strcmp(newPwd,conf)!=0){
            showAlert(cr+1,"  Passwords don't match — let's try again.  ",1);
            Sleep(800); cr+=2;
        } else { showAlert(cr+1,"  Perfect match!  ",0); Sleep(500); break; }
    } while(1);

    gotoxy(24,cr+2);
    setColor(CLR_WARNING); printf("  Save your new password?  [Y/N]: "); resetColor();
    showCursor(); char c=(char)_getch(); hideCursor();
    if (c!='Y'&&c!='y'){
        showAlert(cr+4,"  No changes made.  ",0); Sleep(900); display(username); return;
    }

    strncpy(g_newPwd,newPwd,MAX_PASSWORD-1);
    if (rewriteUser(username,applyPwd))
        showAlert(cr+4,"  Password updated! Remember to keep it safe.  ",0);
    else
        showAlert(cr+4,"  Something went wrong. Password was not changed.  ",1);
    Sleep(1600); display(username);
}

/* ── Close Account ────────────────────────────────────────── */
static void markInactive(struct UserAccount *u) { u->active=0; }

static void closeUserAccount(char username[])
{
    printHeader("  Close Your Account  ",
                "  We're sorry to see you go  ");
    drawBox(18,6,64,16);

    setColor(CLR_ERROR);
    printCentered(8,"  Please read this carefully before you continue.  ");
    setColor(CLR_DEFAULT);
    gotoxy(20,10); printf("  \xFE  Your account will be permanently deactivated.");
    gotoxy(20,11); printf("  \xFE  You will not be able to log in afterwards.");
    gotoxy(20,12); printf("  \xFE  Your remaining balance will be forfeited.");
    gotoxy(20,13); printf("  \xFE  Your transaction history will be kept on record.");
    resetColor();

    gotoxy(20,15); setColor(CLR_WARNING);
    printf("  Enter your password to confirm it's you: "); resetColor();
    gotoxy(65,15);
    char pwd[MAX_PASSWORD]={0}; getPasswordInput(pwd,MAX_PASSWORD);

    FILE *fp=fopen(USER_FILE,"rb"); int ver=0;
    if (fp){
        struct UserAccount u;
        while(fread(&u,sizeof(u),1,fp)==1)
            if(u.active&&strcmp(username,u.username)==0&&
               strcmp(pwd,u.password)==0){ver=1;break;}
        fclose(fp);
    }
    if (!ver){
        showAlert(18,"  Incorrect password. Account not closed.  ",1);
        Sleep(1500); display(username); return;
    }

    gotoxy(20,17); setColor(CLR_ERROR);
    printf("  Type CLOSE to confirm you want to close your account: ");
    resetColor(); gotoxy(75,17); showCursor();
    char confirm[10]={0}; readLine(confirm,sizeof(confirm));

    if (strcmp(confirm,"CLOSE")!=0){
        showAlert(19,"  Closure cancelled — your account is still active.  ",0);
        Sleep(1100); display(username); return;
    }

    if (rewriteUser(username,markInactive)){
        printHeader("  Account Closed  ","");
        setColor(CLR_ERROR);
        printCentered(12,"  Your account has been closed.  ");
        setColor(CLR_DEFAULT);
        printCentered(14,"  We wish you all the best.  ");
        printCentered(15,"  Thank you for banking with us.  ");
        resetColor();
        printFooter("Press any key to return to the main screen . . .");
        getch();
    } else {
        showAlert(19,"  Something went wrong. Please try again.  ",1);
        Sleep(1400); display(username);
    }
}

/* ── Menu ─────────────────────────────────────────────────── */
void accountSettings(char username[])
{
    printHeader("  Account Settings  ","  Manage your account preferences  ");
    drawBox(27,6,46,14);
    setColor(CLR_TITLE); gotoxy(29,6); printf(" What would you like to change? "); resetColor();

    gotoxy(29,9);  setColor(CLR_MENU); printf("  [ 1 ]");
    setColor(CLR_DEFAULT); printf("  Change My Password");

    gotoxy(29,11); setColor(CLR_MENU); printf("  [ 2 ]");
    setColor(CLR_ERROR);   printf("  Close My Account");

    gotoxy(29,13); setColor(CLR_MENU); printf("  [ 3 ]");
    setColor(CLR_DEFAULT); printf("  Go Back");

    resetColor();
    printFooter("Enter 1, 2, or 3 and press Enter");

    gotoxy(29,16); setColor(CLR_WARNING); printf("  Your choice: "); resetColor();
    showCursor(); char buf[8]; readLine(buf,sizeof(buf));
    int ch; if (sscanf(buf,"%d",&ch)!=1){display(username);return;}

    switch(ch){
        case 1: changePassword(username);   break;
        case 2: closeUserAccount(username); break;
        case 3: display(username);          break;
        default:
            showAlert(18,"  Please enter 1, 2, or 3.  ",1);
            Sleep(700); accountSettings(username);
    }
}
