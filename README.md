# Banking Account Management System — Enhanced Edition

> A console-based banking application in C featuring persistent file storage,
> modular architecture, coloured UI, real-time balance tracking, and secure
> masked-password authentication.

---

## Features

| Feature | Details |
|---|---|
| **Account Creation** | Full profile + initial deposit, auto-generated account number |
| **Secure Login** | Masked password input, max 3 attempts before lockout |
| **Fund Transfer** | Real balance deduction & credit, self-transfer guard, confirmation step |
| **Balance Dashboard** | Live balance from file, recent 10 transactions displayed |
| **Transaction Log** | Every credit/debit/deposit logged with timestamp and counterpart |
| **Full History View** | Scrollable history of all account activity |
| **Console UI** | 100-column coloured layout, box-drawing characters, progress bars |
| **Persistent Storage** | Binary flat-file (fread/fwrite) for users and transactions |

---

## Project Structure

```
Banking_System/
│
├── include/
│   └── bank.h           ← Structs, constants, prototypes
│
├── src/
│   ├── main.c           ← Entry point, main menu
│   ├── utils.c          ← Console helpers (colour, boxes, input)
│   ├── account.c        ← Account creation
│   ├── login.c          ← Authentication (masked password, attempt limit)
│   ├── dashboard.c      ← User dashboard
│   ├── transfer.c       ← Fund transfer with balance update
│   ├── balance.c        ← Balance view + transaction history
│   └── logout.c         ← Session termination
│
└── README.md
```

---

## Data Files (created at runtime)

| File | Contents |
|---|---|
| `users.dat` | Binary records of `struct UserAccount` |
| `transactions.dat` | Binary records of `struct Transaction` |

---

## Compile & Run

```bash
# MinGW / GCC on Windows
gcc -I include src/*.c -o Banking_System.exe -luser32

# Run
Banking_System.exe
```

> **Requirement:** Windows (uses `<conio.h>`, `<windows.h>` for `SetConsoleTextAttribute`, `getch`, `Sleep`).

---

## Architecture Notes

### Structures (`bank.h`)

```c
struct UserAccount {
    char   username[50], password[50];
    char   accountno[13];        // Auto-generated: "ACC" + 9 digits
    char   fname[30], lname[30], fathname[30], mothname[30];
    char   address[80], typeaccount[20], pnumber[15], adharnum[20];
    int    date, month, year;
    double balance;              // Always up-to-date
    int    active;               // Soft-delete flag
};

struct Transaction {
    char   accountno[13], username[50], counterpart[50];
    double amount;
    char   type[10];             // "CREDIT", "DEBIT", "DEPOSIT"
    char   description[100];
    char   timestamp[30];        // "DD/MM/YYYY HH:MM:SS"
};
```

### Balance Tracking
- Initial deposit credited on account creation as a `DEPOSIT` transaction.
- Every `transfermoney()` call uses `fseek + fwrite` to update both parties'
  `balance` field in-place inside `users.dat`.
- `checkbalance()` reads the balance directly from the user record — no
  summation loop needed.

### Security
- Passwords are masked with `*` on input via `getPasswordInput()`.
- Login allows a maximum of **3 attempts** before returning to main menu.
- Transfers require an explicit Y/N confirmation before committing.

---

## Developer
Built and enhanced by the Rohith A.
# Banking_System
