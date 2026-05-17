
#include "../include/bank.h"

void depositMoney(char username[])
{
    struct UserAccount u;
    char tmp[40], source[40]={0};
    double amount=0, newBal=0;

    printHeader("  Add Money to Your Account  ",
                "  Top up anytime you like  ");

    FILE *fp=fopen(USER_FILE,"rb");
    if (!fp){ showAlert(14,"  Couldn't reach the database.  ",1); getch(); return; }
    int found=0;
    while(fread(&u,sizeof(u),1,fp)==1)
        if(u.active&&strcmp(username,u.username)==0){found=1;break;}
    fclose(fp);
    if (!found){ showAlert(14,"  Session issue — please sign in again.  ",1); getch(); login(); return; }

    drawBox(16,5,68,18);
    setColor(CLR_TITLE); gotoxy(18,5); printf(" Deposit Details "); resetColor();

    gotoxy(18,7);
    setColor(CLR_INFO);    printf("  Account         : ");
    setColor(CLR_TITLE);   printf("%-22s",username);
    setColor(CLR_BORDER);  printf(" (%s)",u.accountno);

    gotoxy(18,8);
    setColor(CLR_INFO);    printf("  Current Balance : ");
    setColor(CLR_SUCCESS); printf("Rs. %.2f",u.balance);
    resetColor();

    printDivider(10);

    gotoxy(18,11);
    setColor(CLR_INFO); printf("  How are you depositing? (Cash / Cheque / Online): ");
    resetColor(); gotoxy(18,12); readLine(source,sizeof(source));
    if (source[0]=='\0') strcpy(source,"Cash");

    do {
        int i; gotoxy(18,14); for(i=0;i<66;i++) printf(" ");
        gotoxy(18,14);
        setColor(CLR_INFO); printf("  How much would you like to add? (Rs.): "); resetColor();
        gotoxy(59,14); readLine(tmp,sizeof(tmp));
        if (sscanf(tmp,"%lf",&amount)!=1||amount<1.0){
            showAlert(20,"  Enter a valid amount (minimum Rs. 1).  ",1); Sleep(800);
        } else break;
    } while(1);

    drawBox(16,21,68,4);
    gotoxy(18,22);
    setColor(CLR_WARNING); printf("  Add ");
    setColor(CLR_TITLE);   printf("Rs. %.2f",amount);
    setColor(CLR_WARNING); printf(" via %s to your account?",source);
    setColor(CLR_INFO);    printf("   [Y/N]: "); resetColor();
    showCursor(); char c=(char)_getch(); hideCursor();
    if (c!='Y'&&c!='y'){
        showAlert(26,"  No problem! Deposit cancelled.  ",0); Sleep(900); display(username); return;
    }

    animatedProgress("Adding money to your account", 22, 40);

    if (!updateBalance(username,+amount,&newBal)){
        showAlert(26,"  Something went wrong. Your balance was not changed.  ",1);
        Sleep(1400); display(username); return;
    }

    char desc[100];
    snprintf(desc,sizeof(desc),"Deposit via %s",source);
    logTransaction(u.accountno,username,"BANK",amount,newBal,TXN_DEPOSIT,desc);

    printHeader("  Money Added!  ","  Your account has been topped up  ");
    drawBox(16,6,68,13);
    setColor(CLR_SUCCESS); printCentered(8,"\x01  Deposit Successful!  \x01");

    setColor(CLR_INFO); gotoxy(19,10); printf("  Amount Added     : ");
    setColor(CLR_WARNING); printf("Rs. %.2f",amount);

    setColor(CLR_INFO); gotoxy(19,11); printf("  Method           : ");
    setColor(CLR_DEFAULT); printf("%s",source);

    setColor(CLR_INFO); gotoxy(19,12); printf("  New Balance      : ");
    setColor(CLR_SUCCESS); printf("Rs. %.2f",newBal);

    char ts[TIMESTAMP_LEN]; getCurrentTimestamp(ts);
    setColor(CLR_INFO); gotoxy(19,13); printf("  Time             : ");
    setColor(CLR_DEFAULT); printf("%s",ts);

    setColor(CLR_BORDER); gotoxy(19,15);
    printf("  This deposit has been saved to your transaction history.");
    resetColor();

    printFooter("Press any key to go back to your dashboard . . .");
    getch(); display(username);
}
