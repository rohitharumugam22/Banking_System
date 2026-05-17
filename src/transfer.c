
#include "../include/bank.h"

/* [PERSISTENT STORAGE] In-place balance update — fseek+fwrite */
int updateBalance(const char *username, double delta, double *newBalOut)
{
    FILE *fp = fopen(USER_FILE,"r+b");
    if (!fp) return 0;
    struct UserAccount u;
    long pos; int ok=0;
    while (1) {                          /* FIX-05: fread loop */
        pos = ftell(fp);
        if (fread(&u,sizeof(u),1,fp)!=1) break;
        if (u.active && strcmp(username,u.username)==0) {
            u.balance += delta;
            if (newBalOut) *newBalOut = u.balance;
            fseek(fp,pos,SEEK_SET);
            fwrite(&u,sizeof(u),1,fp);  /* [PERSISTENT STORAGE] */
            fflush(fp); ok=1; break;
        }
    }
    fclose(fp); return ok;
}

/* [PERSISTENT STORAGE] Append transaction record — fwrite */
void logTransaction(const char *accno, const char *username,
                    const char *counterpart, double amount,
                    double balAfter, const char *type,
                    const char *description)
{
    FILE *fm = fopen(TRANS_FILE,"ab");
    if (!fm) return;
    struct Transaction t;
    memset(&t,0,sizeof(t));
    strncpy(t.accountno,   accno,       ACC_NO_LEN-1);
    strncpy(t.username,    username,    MAX_USERNAME-1);
    strncpy(t.counterpart, counterpart, MAX_USERNAME-1);
    t.amount = amount; t.balanceAfter = balAfter;
    strncpy(t.type,        type,        sizeof(t.type)-1);
    strncpy(t.description, description, sizeof(t.description)-1);
    getCurrentTimestamp(t.timestamp);
    fwrite(&t,sizeof(t),1,fm);          /* [PERSISTENT STORAGE] */
    fclose(fm);
}

void transfermoney(char currentuser[])
{
    struct UserAccount sender, receiver;
    char   recipientUser[MAX_USERNAME]={0}, amtBuf[30];
    double amount=0;
    int    senderFound=0, receiverFound=0;

    printHeader("  Send Money  ",
                "  Fast, safe transfers to anyone on the system  ");

    /* [PERSISTENT STORAGE] fread() — load sender fresh from file */
    FILE *fp = fopen(USER_FILE,"rb");
    if (!fp){ showAlert(14,"  Can't reach the database right now. Try again.  ",1); getch(); return; }
    while (fread(&sender,sizeof(sender),1,fp)==1)  /* FIX-05 */
        if (sender.active && strcmp(currentuser,sender.username)==0)
            { senderFound=1; break; }
    fclose(fp);
    if (!senderFound){
        showAlert(14,"  Session issue — please sign in again.  ",1); getch(); login(); return;
    }

    /* FIX-18: spendable = balance minus reserve */
    double spendable = sender.balance - MIN_BALANCE;
    if (spendable<0) spendable=0;

    /* ── Form ────────────────────────────────────────────── */
    drawBox(14,5,72,20);
    setColor(CLR_TITLE); gotoxy(16,5); printf(" Transfer Details "); resetColor();

    gotoxy(16,7);
    setColor(CLR_INFO);    printf("  From your account  : ");
    setColor(CLR_TITLE);   printf("%-22s",sender.username);
    setColor(CLR_BORDER);  printf(" (%s)",sender.accountno);

    gotoxy(16,8);
    setColor(CLR_INFO);    printf("  Your balance       : ");
    setColor(CLR_SUCCESS); printf("Rs. %.2f",sender.balance);

    gotoxy(16,9);
    setColor(CLR_BORDER);  printf("  You can send up to : ");
    setColor(CLR_WARNING); printf("Rs. %.2f  (Rs. %.0f kept as your reserve)",
                                  spendable,MIN_BALANCE);
    resetColor();

    printDivider(11);

    /* Recipient */
    gotoxy(16,12);
    setColor(CLR_INFO); printf("  Who are you sending to? (username): "); resetColor();
    gotoxy(55,12); readLine(recipientUser,MAX_USERNAME);

    if (recipientUser[0]=='\0'){
        showAlert(26,"  Please enter the recipient's username.  ",1);
        Sleep(1000); display(currentuser); return;
    }
    if (strcmp(currentuser,recipientUser)==0){
        showAlert(26,"  You can't send money to yourself!  ",1);
        Sleep(1200); display(currentuser); return;
    }

    fp = fopen(USER_FILE,"rb");
    if (fp){
        while (fread(&receiver,sizeof(receiver),1,fp)==1)
            if (receiver.active && strcmp(recipientUser,receiver.username)==0)
                { receiverFound=1; break; }
        fclose(fp);
    }
    if (!receiverFound){
        char em[80];
        snprintf(em,sizeof(em),
            "  We couldn't find an account for \"%s\".  ",recipientUser);
        showAlert(26,em,1); Sleep(1400); display(currentuser); return;
    }

    gotoxy(16,13);
    setColor(CLR_INFO);    printf("  Sending to         : ");
    setColor(CLR_SUCCESS); printf("%s %s",receiver.fname,receiver.lname);
    setColor(CLR_BORDER);  printf("  (%s)",receiver.accountno);
    resetColor();

    printDivider(15);

    /* Amount */
    gotoxy(16,16);
    setColor(CLR_INFO); printf("  How much would you like to send? (Rs.): "); resetColor();
    gotoxy(58,16); readLine(amtBuf,sizeof(amtBuf));

    if (sscanf(amtBuf,"%lf",&amount)!=1 || amount<=0){
        showAlert(26,"  Please enter a valid positive amount.  ",1);
        Sleep(1000); display(currentuser); return;
    }
    if (amount>spendable){
        char em[100];
        snprintf(em,sizeof(em),
            "  You can send up to Rs. %.2f right now.  ",spendable);
        showAlert(26,em,1); Sleep(1800); display(currentuser); return;
    }

    /* Confirm — FIX-06: amount arg was missing in original */
    drawBox(14,23,72,4);
    gotoxy(16,24);
    setColor(CLR_WARNING); printf("  Send ");
    setColor(CLR_TITLE);   printf("Rs. %.2f",amount);  /* FIX-06 */
    setColor(CLR_WARNING); printf(" to %s %s?",receiver.fname,receiver.lname);
    setColor(CLR_INFO);    printf("   [Y = Yes  /  N = Cancel]: ");
    resetColor(); showCursor();
    char c=(char)_getch(); hideCursor();

    if (c!='Y'&&c!='y'){
        showAlert(28,"  No problem! Transfer cancelled. You're back to the dashboard.  ",0);
        Sleep(1000); display(currentuser); return;
    }

    animatedProgress("Sending your money securely", 28, 35);

    /* [SECURITY] Atomic transfer with rollback */
    double senderNew=0, receiverNew=0;
    if (!updateBalance(currentuser,-amount,&senderNew)){
        showAlert(28,"  Transfer failed — couldn't update your balance. Nothing was sent.  ",1);
        Sleep(1400); display(currentuser); return;
    }
    if (!updateBalance(recipientUser,+amount,&receiverNew)){
        updateBalance(currentuser,+amount,NULL); /* rollback sender */
        showAlert(28,"  Transfer failed — rolled back. Your balance is unchanged.  ",1);
        Sleep(1400); display(currentuser); return;
    }

    /* [PERSISTENT STORAGE] Log both sides */
    char desc[100];
    snprintf(desc,sizeof(desc),"Sent to %s",recipientUser);
    logTransaction(sender.accountno,  currentuser, recipientUser,
                   amount,senderNew,  TXN_DEBIT,  desc);
    snprintf(desc,sizeof(desc),"Received from %s",currentuser);
    logTransaction(receiver.accountno,recipientUser,currentuser,
                   amount,receiverNew,TXN_CREDIT,desc);

    /* ── Success screen — FIX-17: senderNew not stale value */
    printHeader("  Money Sent!  ","  The transfer went through  ");
    drawBox(14,6,72,14);
    setColor(CLR_SUCCESS);
    printCentered(8,"\x01  Your transfer was successful!  \x01");

    setColor(CLR_INFO);    gotoxy(17,10); printf("  Amount sent        : ");
    setColor(CLR_WARNING); printf("Rs. %.2f",amount);

    setColor(CLR_INFO);    gotoxy(17,12); printf("  Sent to            : ");
    setColor(CLR_TITLE);   printf("%s %s  (%s)",receiver.fname,receiver.lname,receiver.accountno);

    setColor(CLR_INFO);    gotoxy(17,14); printf("  Your new balance    : ");
    setColor(CLR_SUCCESS); printf("Rs. %.2f",senderNew); /* FIX-17 */

    char ts[TIMESTAMP_LEN]; getCurrentTimestamp(ts);
    setColor(CLR_INFO);    gotoxy(17,16); printf("  Completed at        : ");
    setColor(CLR_DEFAULT); printf("%s",ts);

    setColor(CLR_BORDER);  gotoxy(17,17);
    printf("  This transfer has been saved to your transaction history.");

    resetColor();
    printFooter("Press any key to go back to your dashboard . . .");
    getch();
    display(currentuser);
}
