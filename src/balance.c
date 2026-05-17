
#include "../include/bank.h"

static int loadUser(const char *username, struct UserAccount *out)
{
    FILE *fp = fopen(USER_FILE,"rb");
    if (!fp) return 0;
    while (fread(out,sizeof(*out),1,fp)==1)   /* FIX-05 */
        if (out->active && strcmp(username,out->username)==0)
            { fclose(fp); return 1; }
    fclose(fp); return 0;
}

/* ── checkbalance ─────────────────────────────────────────── */
void checkbalance(char username[])
{
    struct UserAccount u;
    printHeader("  Your Balance  ","  Here's how things look  ");

    if (!loadUser(username,&u)){
        showAlert(14,"  Couldn't load your account. Please try again.  ",1);
        getch(); display(username); return;
    }

    /* Balance card */
    drawBox(14,5,72,9);
    setColor(CLR_TITLE); gotoxy(16,5); printf(" Account Summary — %s ",u.accountno); resetColor();

    gotoxy(16,7); setColor(CLR_INFO);    printf("  Account Holder  : ");
    setColor(CLR_TITLE);   printf("%s %s",u.fname,u.lname); resetColor();

    gotoxy(16,8); setColor(CLR_INFO);    printf("  Account Type    : ");
    setColor(CLR_DEFAULT); printf("%s",u.typeaccount); resetColor();

    gotoxy(16,9); setColor(CLR_INFO);    printf("  Total Balance   : ");
    setColor(CLR_SUCCESS); printf("Rs. %.2f",u.balance); resetColor();

    gotoxy(16,10); setColor(CLR_INFO);   printf("  Available Now   : ");
    setColor(CLR_WARNING);
    double spend=u.balance-MIN_BALANCE;
    printf("Rs. %.2f",spend>0?spend:0.0); resetColor();

    gotoxy(16,11); setColor(CLR_BORDER);
    printf("  Rs. %.0f is kept as your safety reserve — always there for you.",MIN_BALANCE);
    resetColor();

    /* [PERSISTENT STORAGE] fread() last-10 transactions */
    struct Transaction ring[10];
    int total=0;
    FILE *fm = fopen(TRANS_FILE,"rb");
    if (fm){
        struct Transaction t;
        while (fread(&t,sizeof(t),1,fm)==1)   /* FIX-05 */
            if (strcmp(username,t.username)==0)
                { ring[total%10]=t; total++; }
        fclose(fm);
    }
    int displayed=(total>10)?10:total;
    int startIdx =(total>10)?(total%10):0;

    drawBox(1,15,98,displayed+5);
    setColor(CLR_TITLE);
    gotoxy(3,15);
    printf(" Recent Activity (last %d of %d transactions) ",displayed,total);
    resetColor();

    gotoxy(3,16); setColor(CLR_BORDER);
    printf("  %-3s  %-19s  %-8s  %-10s  %-12s  %-12s  %-14s",
           "No.","When","Type","Amount","With","Balance After","What");
    printDivider(17);

    int row=18,i;
    if (displayed==0){
        setColor(CLR_BORDER);
        printCentered(row,"  No transactions yet — your history will appear here once you start using your account.  ");
        resetColor();
    }
    for(i=0;i<displayed;i++){
        struct Transaction *t=&ring[(startIdx+i)%10];
        int isCr=(strcmp(t->type,TXN_CREDIT)==0||strcmp(t->type,TXN_DEPOSIT)==0);
        int seq=total-displayed+i+1;   /* FIX-30 */
        gotoxy(3,row);
        setColor(CLR_BORDER);                     printf("  %-3d  ",seq);
        setColor(CLR_INFO);                       printf("%-19s  ",t->timestamp);
        setColor(isCr?CLR_SUCCESS:CLR_ERROR);     printf("%-8s  ",t->type);
        setColor(isCr?CLR_SUCCESS:CLR_ERROR);     printf("Rs.%-7.2f  ",t->amount);
        setColor(CLR_DEFAULT);                    printf("%-12s  ",t->counterpart);
        setColor(CLR_WARNING);                    printf("Rs.%-9.2f  ",t->balanceAfter);
        setColor(CLR_BORDER);                     printf("%-14s",t->description);
        resetColor(); row++;
    }

    printFooter("Press any key to return to your dashboard . . .");
    getch(); display(username);
}

/* ── viewTransactionHistory ───────────────────────────────── */
void viewTransactionHistory(char username[])
{
    printHeader("  Your Full Transaction History  ",
                "  Every move your money has ever made  ");

    int total=0;
    FILE *fm=fopen(TRANS_FILE,"rb");
    if (fm){
        struct Transaction t;
        while(fread(&t,sizeof(t),1,fm)==1)   /* FIX-05 */
            if(strcmp(username,t.username)==0) total++;
        fclose(fm);
    }

    int show=(total>20)?20:total;
    int skip=total-show;

    drawBox(1,5,98,show+6);
    setColor(CLR_TITLE);
    gotoxy(3,5);
    printf(" Your transaction history — %d shown of %d total ",show,total);
    resetColor();

    gotoxy(3,6); setColor(CLR_BORDER);
    printf("  %-4s  %-19s  %-8s  %-12s  %-12s  %-12s  %-14s",
           "No.","When","Type","Amount","With","Bal. After","Details");
    printDivider(7);

    int row=8,seq=0,shown=0;
    fm=fopen(TRANS_FILE,"rb");
    if (fm){
        struct Transaction t;
        while(fread(&t,sizeof(t),1,fm)==1){ /* FIX-05 */
            if(strcmp(username,t.username)!=0) continue;
            seq++;
            if(seq<=skip) continue;
            if(shown>=20||row>27) break;
            int isCr=(strcmp(t.type,TXN_CREDIT)==0||strcmp(t.type,TXN_DEPOSIT)==0);
            gotoxy(3,row);
            setColor(CLR_BORDER);                  printf("  %-4d  ",seq);
            setColor(CLR_INFO);                    printf("%-19s  ",t.timestamp);
            setColor(isCr?CLR_SUCCESS:CLR_ERROR);  printf("%-8s  ",t.type);
            setColor(isCr?CLR_SUCCESS:CLR_ERROR);  printf("Rs.%-9.2f  ",t.amount);
            setColor(CLR_DEFAULT);                 printf("%-12s  ",t.counterpart);
            setColor(CLR_WARNING);                 printf("Rs.%-9.2f  ",t.balanceAfter);
            setColor(CLR_BORDER);                  printf("%-14s",t.description);
            resetColor(); row++; shown++;
        }
        fclose(fm);
    }

    if (total==0){
        setColor(CLR_BORDER);
        printCentered(17,"  No transactions yet — start using your account to see history here.  ");
        resetColor();
    }
    if (skip>0){
        gotoxy(3,row+1); setColor(CLR_BORDER);
        printf("  ... %d older transaction(s) not shown. Displaying your most recent %d.",skip,show);
        resetColor();
    }

    printFooter("Press any key to return to your dashboard . . .");
    getch(); display(username);
}
