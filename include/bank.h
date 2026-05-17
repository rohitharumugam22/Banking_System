#ifndef BANK_H
#define BANK_H


#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <ctype.h>      

/* ─── layout ─────────────────────────────────────────────── */
#define CONSOLE_WIDTH   100
#define CONSOLE_ROWS     32
#define FOOTER_ROW       30   

/* ─── colours ────────────────────────────────────────────── */
#define CLR_DEFAULT    7
#define CLR_HEADER    11
#define CLR_SUCCESS   10
#define CLR_ERROR     12
#define CLR_WARNING   14
#define CLR_TITLE     15
#define CLR_MENU       9
#define CLR_ACCENT    13
#define CLR_INFO       3
#define CLR_BORDER     8

/* ─── files ──────────────────────────────────────────────── */
#define USER_FILE     "users.dat"
#define TRANS_FILE    "transactions.dat"

/* ─── limits ─────────────────────────────────────────────── */
#define MAX_USERNAME   50
#define MAX_PASSWORD   50
#define MAX_NAME       30
#define MAX_PHONE      15
#define MAX_ADHAR      20
#define MAX_ADDRESS    80
#define MAX_ACCTYPE    20
#define ACC_NO_LEN     14    
#define TIMESTAMP_LEN  30
#define MIN_BALANCE   500.0
#define MIN_PWD_LEN     6

/* ─── transaction types ──────────────────────────────────── */
#define TXN_CREDIT    "CREDIT"
#define TXN_DEBIT     "DEBIT"
#define TXN_DEPOSIT   "DEPOSIT"
#define TXN_WITHDRAW  "WITHDRAW"

/* ─── structures ─────────────────────────────────────────── */

struct UserAccount {
    char   username[MAX_USERNAME];
    char   password[MAX_PASSWORD];
    char   accountno[ACC_NO_LEN];
    char   fname[MAX_NAME];
    char   lname[MAX_NAME];
    char   fathname[MAX_NAME];
    char   mothname[MAX_NAME];
    char   address[MAX_ADDRESS];
    char   typeaccount[MAX_ACCTYPE];
    char   pnumber[MAX_PHONE];
    char   adharnum[MAX_ADHAR];
    int    date, month, year;
    double balance;
    int    active;           /* 1=active  0=closed */
};

struct Transaction {
    char   accountno[ACC_NO_LEN];
    char   username[MAX_USERNAME];
    char   counterpart[MAX_USERNAME];
    double amount;
    double balanceAfter;     /* running balance snapshot per txn */
    char   type[12];
    char   description[100];
    char   timestamp[TIMESTAMP_LEN];
};

/* ─── prototypes ─────────────────────────────────────────── */

/* utils.c */
void   gotoxy(int x, int y);
void   setColor(int color);
void   resetColor(void);
void   hideCursor(void);
void   showCursor(void);
void   setConsoleSize(int cols, int rows);
void   drawBox(int x, int y, int w, int h);
void   printDivider(int row);
void   printCentered(int row, const char *text);
void   printHeader(const char *title, const char *subtitle);
void   printFooter(const char *hint);
void   showAlert(int row, const char *msg, int isError);
int    readLine(char *buf, int maxlen);
int    getPasswordInput(char *buf, int maxlen);
int    isValidPhone(const char *s);
int    isValidAadhar(const char *s);
int    isValidDOB(int d, int m, int y);
int    isStrongPassword(const char *pwd);
void   generateAccountNo(char *buf);
void   getCurrentTimestamp(char *buf);
void   animatedProgress(const char *msg, int steps, int ms);
void   typingEffect(int col, int row, int color, const char *text, int ms);

/* transfer.c (shared) */
int    updateBalance(const char *username, double delta, double *newBalOut);
void   logTransaction(const char *accno, const char *username,
                      const char *counterpart, double amount,
                      double balAfter, const char *type,
                      const char *description);

/* account.c */
void   account(void);
void   accountcreated(const char *accno, const char *fname, const char *lname);

/* login.c */
void   login(void);
void   loginSuccessAnim(const char *username);

/* dashboard.c */
void   display(char username[]);

/* transfer.c */
void   transfermoney(char currentuser[]);

/* balance.c */
void   checkbalance(char username[]);
void   viewTransactionHistory(char username[]);

/* deposit.c */
void   depositMoney(char username[]);

/* withdraw.c */
void   withdrawMoney(char username[]);

/* settings.c */
void   accountSettings(char username[]);

/* logout.c */
void   logout(void);

#endif
