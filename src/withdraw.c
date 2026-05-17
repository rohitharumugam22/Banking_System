
#include "../include/bank.h"

void withdrawMoney(char username[])
{
    struct UserAccount u;
    char tmp[40]; double amount=0, newBal=0;

    printHeader("  Withdraw Cash  ","  Take out what you need  ");

    FILE *fp=fopen(USER_FILE,"rb");
    if (!fp){ showAlert(14,"  Couldn't reach the database.  ",1); getch(); return; }
    int found=0;
    while(fread(&u,sizeof(u),1,fp)==1)
        if(u.active&&strcmp(username,u.username)==0){found=1;break;}
    fclose(fp);
    if (!found){ showAlert(14,"  Session issue.  ",1); getch(); login(); return; }

    double spendable=u.balance-MIN_BALANCE;
    if (spendable<=0){
        char em[100];
        snprintf(em,sizeof(em),
            "  Your balance is at the Rs. %.0f reserve minimum — nothing to withdraw right now.  ",
            MIN_BALANCE);
        showAlert(14,em,1); Sleep(2200); display(username); return;
    }

    drawBox(16,5,68,16);
    setColor(CLR_TITLE); gotoxy(18,5); printf(" Withdrawal Details "); resetColor();

    gotoxy(18,7);
    setColor(CLR_INFO);    printf("  Account         : ");
    setColor(CLR_TITLE);   printf("%-22s",username);
    setColor(CLR_BORDER);  printf(" (%s)",u.accountno);

    gotoxy(18,8);
    setColor(CLR_INFO);    printf("  Total Balance   : ");
    setColor(CLR_SUCCESS); printf("Rs. %.2f",u.balance);

    gotoxy(18,9);
    setColor(CLR_BORDER);  printf("  You can take out: ");
    setColor(CLR_WARNING); printf("Rs. %.2f  (Rs. %.0f stays as your safety reserve)",
                                  spendable,MIN_BALANCE);
    resetColor();

    printDivider(11);

    do {
        int i; gotoxy(18,12); for(i=0;i<66;i++) printf(" ");
        gotoxy(18,12);
        setColor(CLR_INFO); printf("  How much would you like to withdraw? (Rs.): "); resetColor();
        gotoxy(64,12); readLine(tmp,sizeof(tmp));
        if (sscanf(tmp,"%lf",&amount)!=1||amount<=0){
            showAlert(18,"  Please enter a valid positive amount.  ",1); Sleep(700); continue;
        }
        if (amount>spendable){
            char em[100];
            snprintf(em,sizeof(em),"  You can withdraw up to Rs. %.2f right now.  ",spendable);
            showAlert(18,em,1); Sleep(1000); continue;
        }
        break;
    } while(1);

    drawBox(16,20,68,4);
    gotoxy(18,21);
    setColor(CLR_WARNING); printf("  Withdraw ");
    setColor(CLR_TITLE);   printf("Rs. %.2f",amount);
    setColor(CLR_WARNING); printf(" from your account?");
    setColor(CLR_INFO);    printf("   [Y/N]: "); resetColor();
    showCursor(); char c=(char)_getch(); hideCursor();
    if (c!='Y'&&c!='y'){
        showAlert(25,"  Withdrawal cancelled — nothing was changed.  ",0);
        Sleep(900); display(username); return;
    }

    animatedProgress("Processing your withdrawal", 22, 40);

    if (!updateBalance(username,-amount,&newBal)){
        showAlert(25,"  Something went wrong. Your balance is unchanged.  ",1);
        Sleep(1400); display(username); return;
    }

    logTransaction(u.accountno,username,"ATM / COUNTER",
                   amount,newBal,TXN_WITHDRAW,"Cash withdrawal");

    printHeader("  Withdrawal Complete  ","");
    drawBox(16,6,68,12);
    setColor(CLR_SUCCESS); printCentered(8,"\x01  Your cash is ready!  \x01");

    setColor(CLR_INFO); gotoxy(19,10); printf("  Amount Withdrawn : ");
    setColor(CLR_WARNING); printf("Rs. %.2f",amount);

    setColor(CLR_INFO); gotoxy(19,11); printf("  Remaining Balance: ");
    setColor(CLR_SUCCESS); printf("Rs. %.2f",newBal);

    char ts[TIMESTAMP_LEN]; getCurrentTimestamp(ts);
    setColor(CLR_INFO); gotoxy(19,12); printf("  Time             : ");
    setColor(CLR_DEFAULT); printf("%s",ts);

    setColor(CLR_BORDER); gotoxy(19,14);
    printf("  Please count your cash before leaving the counter.");
    resetColor();

    printFooter("Press any key to return to your dashboard . . .");
    getch(); display(username);
}
