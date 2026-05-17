
#include "../include/bank.h"

void display(char username[])
{
    struct UserAccount u;
    int found=0;

    /* [PERSISTENT STORAGE] fread() — always load fresh from file */
    FILE *fp = fopen(USER_FILE,"rb");
    if (!fp) {
        printHeader("  Something went wrong  ","");
        showAlert(14,"  We couldn't load your account. Please try again.  ",1);
        getch(); return;
    }
    while (fread(&u,sizeof(u),1,fp)==1) {  /* FIX-05 */
        if (u.active && strcmp(username,u.username)==0) { found=1; break; }
    }
    fclose(fp);

    if (!found) {
        printHeader("  Account not found  ","");
        showAlert(14,"  We couldn't find your profile. Please log in again.  ",1);
        getch(); return;
    }

    char subtitle[100];
    snprintf(subtitle,sizeof(subtitle),
             "  Hi %s! Here's everything you need.  ", u.fname);
    printHeader("  Your Dashboard  ", subtitle);

    /* ── Balance Panel ───────────────────────────────────── */
    drawBox(54,5,44,10);
    setColor(CLR_TITLE); gotoxy(56,5); printf(" Your Money "); resetColor();

    /* FIX-07 & FIX-08: explicit printf, no macro, standard %.2f */
    gotoxy(56,7);
    setColor(CLR_INFO);    printf("  Total Balance   : ");
    setColor(CLR_SUCCESS); printf("Rs. %.2f", u.balance);    /* FIX-07 */

    gotoxy(56,8);
    setColor(CLR_INFO);    printf("  Available       : ");
    setColor(CLR_WARNING);
    double spend = u.balance - MIN_BALANCE;
    printf("Rs. %.2f", spend>0?spend:0.0);

    gotoxy(56,9);
    setColor(CLR_BORDER);  printf("  Reserve kept    : Rs. %.0f (for your safety)", MIN_BALANCE);
    resetColor();

    /* ── Account Info ────────────────────────────────────── */
    drawBox(54,16,44,12);
    setColor(CLR_TITLE); gotoxy(56,16); printf(" Your Profile "); resetColor();

    /* FIX-08: plain printf per row, no variadic macro */
    gotoxy(56,18); setColor(CLR_INFO); printf("  Name          : ");
    setColor(CLR_DEFAULT); printf("%.20s %.10s",u.fname,u.lname); resetColor();

    gotoxy(56,19); setColor(CLR_INFO); printf("  Account Type  : ");
    setColor(CLR_DEFAULT); printf("%s",u.typeaccount); resetColor();

    gotoxy(56,20); setColor(CLR_INFO); printf("  Date of Birth : ");
    setColor(CLR_DEFAULT); printf("%02d/%02d/%04d",u.date,u.month,u.year); resetColor();

    gotoxy(56,21); setColor(CLR_INFO); printf("  Mobile        : ");
    setColor(CLR_DEFAULT); printf("%s",u.pnumber); resetColor();

    gotoxy(56,22); setColor(CLR_INFO); printf("  Aadhar        : ");
    setColor(CLR_DEFAULT); printf("%s",u.adharnum); resetColor();

    gotoxy(56,23); setColor(CLR_INFO); printf("  Status        : ");
    setColor(CLR_SUCCESS); printf("Active  \x01"); resetColor();

    gotoxy(56,24); setColor(CLR_INFO); printf("  Address       : ");
    setColor(CLR_DEFAULT);
    char addr[22]; strncpy(addr,u.address,21); addr[21]='\0';
    printf("%s",addr); resetColor();

    /* ── Action Menu ─────────────────────────────────────── */
    drawBox(2,5,49,24);
    setColor(CLR_TITLE); gotoxy(4,5); printf(" What would you like to do? "); resetColor();

    typedef struct{int k;const char*ic;const char*lbl;const char*dsc;}MI;
    MI items[]={
        {1,"\x10","Check My Balance",     "See balance & recent activity"   },
        {2,"\x1A","Send Money",           "Transfer to a friend or family"  },
        {3,"\x18","Add Money",            "Deposit cash into your account"  },
        {4,"\x19","Withdraw Cash",        "Take money out of your account"  },
        {5,"\xF0","Transaction History",  "See everything that's happened"  },
        {6,"\x1D","Account Settings",     "Change password or close account"},
        {7,"\x1E","Sign Out",             "Done for now? Sign out safely"   },
        {8,"\xFE","Exit App",             "Close the application"           },
    };
    int n=(int)(sizeof(items)/sizeof(items[0])), mr=6, mi;
    for(mi=0;mi<n;mi++){
        drawBox(4,mr,45,3);
        gotoxy(6,mr+1);
        setColor(CLR_MENU);   printf(" [%d] %s  ",items[mi].k,items[mi].ic);
        setColor(CLR_TITLE);  printf("%-22s",items[mi].lbl);
        setColor(CLR_BORDER); printf("%-23s",items[mi].dsc);
        resetColor(); mr+=3;
    }

    printFooter("Type a number (1-8) and press Enter  |  All activity is saved securely");

    gotoxy(4,28);
    setColor(CLR_WARNING); printf("  What'll it be? "); resetColor();
    showCursor();

    /* FIX-12: readLine+sscanf */
    char buf[8]; readLine(buf,sizeof(buf));
    int choice;
    if (sscanf(buf,"%d",&choice)!=1){display(username);return;}

    switch(choice){
        case 1: checkbalance(username);           break;
        case 2: transfermoney(username);          break;
        case 3: depositMoney(username);           break;
        case 4: withdrawMoney(username);          break;
        case 5: viewTransactionHistory(username); break;
        case 6: accountSettings(username);        break;
        case 7: logout(); login();                break;
        case 8:
            system("cls");
            setColor(CLR_SUCCESS);
            printCentered(13,"  Take care! Your money is safe with us.  ");
            printCentered(15,"  See you next time!  \x01  ");
            resetColor(); Sleep(1500); exit(0);
        default:
            showAlert(28,
                "  That's not one of the options. Please pick 1 through 8.  ",1);
            Sleep(900); display(username);
    }
}
