#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Account
{
    int accountNumber;
    char name[50];
    double balance;
    struct Account *left;
    struct Account *right;
} Account;

typedef enum
{
    DEPOSIT,
    WITHDRAW
} TransactionType;

typedef struct Transaction
{
    int transactionId;
    int accountNumber;
    TransactionType type;
    double amount;
    struct Transaction *next;
} Transaction;

typedef struct
{
    int accountNumber;
    char name[50];
    double balance;
} AccountRecord;

typedef struct
{
    int transactionId;
    int accountNumber;
    TransactionType type;
    double amount;
} TransactionRecord;

Account *createAccount(int accountNumber, const char *name, double balance)
{
    Account *acc = malloc(sizeof(Account));
    acc->accountNumber = accountNumber;
    strcpy(acc->name, name);
    acc->balance = balance;
    acc->left = acc->right = NULL;
    return acc;
}

Account *insertAccount(Account *root, int accountNumber, const char *name, double balance)
{
    if (!root)
        return createAccount(accountNumber, name, balance);

    if (accountNumber < root->accountNumber)
        root->left = insertAccount(root->left, accountNumber, name, balance);
    else if (accountNumber > root->accountNumber)
        root->right = insertAccount(root->right, accountNumber, name, balance);
    else
        printf("Account %d already exists\n", accountNumber);

    return root;
}

Account *findAccount(Account *root, int accountNumber)
{
    if (!root)
        return NULL;
    if (accountNumber == root->accountNumber)
        return root;
    if (accountNumber < root->accountNumber)
        return findAccount(root->left, accountNumber);
    return findAccount(root->right, accountNumber);
}

void traverseAccounts(Account *root)
{
    if (!root)
        return;
    traverseAccounts(root->left);
    printf("Account #%d | Name: %s | Balance: %.2f\n", root->accountNumber, root->name, root->balance);
    traverseAccounts(root->right);
}

Transaction *addTransaction(Transaction *head, int id, int accountNumber, TransactionType type, double amount)
{
    Transaction *t = malloc(sizeof(Transaction));
    t->transactionId = id;
    t->accountNumber = accountNumber;
    t->type = type;
    t->amount = amount;
    t->next = NULL;

    if (!head)
        return t;

    Transaction *curr = head;
    while (curr->next)
        curr = curr->next;
    curr->next = t;
    return head;
}

void printTransactions(Transaction *head)
{
    Transaction *curr = head;
    while (curr)
    {
        printf("Transaction #%d | Account #%d | Type: %s | Amount: %.2f\n",
               curr->transactionId, curr->accountNumber,
               curr->type == DEPOSIT ? "DEPOSIT" : "WITHDRAW",
               curr->amount);
        curr = curr->next;
    }
}

void saveAccountsToFile(Account *root, const char *filename);
void saveTransactionsToFile(Transaction *head, const char *filename);
Account *loadAccountsFromFile(const char *filename);
Transaction *loadTransactionsFromFile(const char *filename);

static void exportAccounts(FILE *fp, Account *root)
{
    if (!root)
        return;

    exportAccounts(fp, root->left);

    AccountRecord rec;
    rec.accountNumber = root->accountNumber;
    strcpy(rec.name, root->name);
    rec.balance = root->balance;

    fwrite(&rec, sizeof(AccountRecord), 1, fp);

    exportAccounts(fp, root->right);
}

void saveAccountsToFile(Account *root, const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        perror("Failed to open accounts file");
        return;
    }
    exportAccounts(fp, root);
    fclose(fp);
}

Account *loadAccountsFromFile(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return NULL;

    AccountRecord rec;
    Account *root = NULL;
    while (fread(&rec, sizeof(AccountRecord), 1, fp) == 1)
        root = insertAccount(root, rec.accountNumber, rec.name, rec.balance);

    fclose(fp);
    return root;
}

void saveTransactionsToFile(Transaction *head, const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        perror("Failed to open transactions file");
        return;
    }

    Transaction *curr = head;
    while (curr)
    {
        TransactionRecord rec;
        rec.transactionId = curr->transactionId;
        rec.accountNumber = curr->accountNumber;
        rec.type = curr->type;
        rec.amount = curr->amount;
        fwrite(&rec, sizeof(TransactionRecord), 1, fp);
        curr = curr->next;
    }
    fclose(fp);
}

Transaction *loadTransactionsFromFile(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return NULL;

    TransactionRecord rec;
    Transaction *head = NULL;
    Transaction *tail = NULL;

    while (fread(&rec, sizeof(TransactionRecord), 1, fp) == 1)
    {
        Transaction *node = addTransaction(NULL, rec.transactionId, rec.accountNumber, rec.type, rec.amount);
        if (!head)
            head = tail = node;
        else
        {
            tail->next = node;
            tail = node;
        }
    }

    fclose(fp);
    return head;
}

int main()
{
    Account *bank = loadAccountsFromFile("accounts.bin");
    Transaction *ledger = loadTransactionsFromFile("transactions.bin");
    int choice, accNum, transId = 1;
    char name[50];
    double amount;

    Transaction *tmp = ledger;
    while (tmp)
    {
        if (tmp->transactionId >= transId)
            transId = tmp->transactionId + 1;
        tmp = tmp->next;
    }

    while (1)
    {
        printf("\n--- Bank Menu ---\n");
        printf("1. Create Account\n2. Deposit\n3. Withdraw\n4. Show Accounts\n5. Show Transactions\n6. Exit\nChoice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf("Enter account number: ");
            scanf("%d", &accNum);
            printf("Enter name: ");
            scanf(" %49[^\n]", name);
            printf("Enter initial balance: ");
            scanf("%lf", &amount);
            bank = insertAccount(bank, accNum, name, amount);
            break;

        case 2:
            printf("Enter account number: ");
            scanf("%d", &accNum);
            printf("Enter deposit amount: ");
            scanf("%lf", &amount);
            {
                Account *acc = findAccount(bank, accNum);
                if (acc)
                {
                    acc->balance += amount;
                    ledger = addTransaction(ledger, transId++, accNum, DEPOSIT, amount);
                }
                else
                    printf("Account not found.\n");
            }
            break;

        case 3:
            printf("Enter account number: ");
            scanf("%d", &accNum);
            printf("Enter withdrawal amount: ");
            scanf("%lf", &amount);
            {
                Account *acc = findAccount(bank, accNum);
                if (acc)
                {
                    if (acc->balance >= amount)
                    {
                        acc->balance -= amount;
                        ledger = addTransaction(ledger, transId++, accNum, WITHDRAW, amount);
                    }
                    else
                        printf("Insufficient balance.\n");
                }
                else
                    printf("Account not found.\n");
            }
            break;

        case 4:
            traverseAccounts(bank);
            break;
        case 5:
            printTransactions(ledger);
            break;
        case 6:
            saveAccountsToFile(bank, "accounts.bin");
            saveTransactionsToFile(ledger, "transactions.bin");
            exit(0);
        default:
            printf("Invalid choice.\n");
        }
    }
}
