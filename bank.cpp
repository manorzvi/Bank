#include <iostream>
#include <stdlib.h> /* atoi */
#include <cstring>
#include <fstream>
#include <pthread.h>
#include <unistd.h> /* usleep */
#include <vector>
#include <set>
#include <algorithm> /* std::reverse */
#include <iterator>
#include <sstream>
#include <iostream>
#include <string>
#include <typeinfo>

#include "Account.h"

#define MAX_COMMAND 10
#define MAX_LINE_SIZE 100
#define MAX_BANK_ITERATIONS 50

void* atm_thread_func(void* arg);

void create_account(char* tokens[MAX_COMMAND], int atm_id);
void balance(char* tokens[MAX_COMMAND], int atm_id);
void vip(char* tokens[MAX_COMMAND], int atm_id);
void deposit(char* tokens[MAX_COMMAND], int atm_id);
void withdrew(char* tokens[MAX_COMMAND], int atm_id);
void transfer(char* tokens[MAX_COMMAND], int atm_id);
void read_locker();
void read_unlocker();

using namespace std;

ofstream log_file;
pthread_mutex_t log_lock;

set<Account> accounts;
pthread_mutex_t write_accounts_lock;
pthread_mutex_t read_accounts_lock;
int accounts_read_count=0;

Account bank_account(0,-1,-1,-1);
pthread_mutex_t write_bank_lock;
pthread_mutex_t read_bank_lock;
int bank_read_count=0;

int dones = 0;
pthread_mutex_t write_dones_lock;
pthread_mutex_t read_dones_lock;
int dones_read_count=0;

typedef struct _atm_data_t {
    int threadid;
    string atm_input_file;

} atm_data_t;

void read_locker() {
    pthread_mutex_lock(&read_accounts_lock);
    accounts_read_count++;
    if (accounts_read_count == 1) {
        pthread_mutex_lock(&write_accounts_lock);
    }
    pthread_mutex_unlock(&read_accounts_lock);
}
void read_unlocker() {
    pthread_mutex_lock(&read_accounts_lock);
    accounts_read_count--;
    if (accounts_read_count == 0) {
        pthread_mutex_unlock(&write_accounts_lock);
    }
    pthread_mutex_unlock(&read_accounts_lock);
}

void create_account(char* tokens[MAX_COMMAND], int atm_id) {

    /* readers-writers on accounts vector.
     * here - we are looking for account with a given id, therefore we are readers. */
    read_locker();

    Account new_account(atoi(tokens[1]), atoi(tokens[2]), atoi(tokens[3]), atm_id);
    if (accounts.find(new_account) != accounts.end()) {
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – account with the same id ("
                 << new_account.get_id() << ") exists" << endl;
        pthread_mutex_unlock(&log_lock);
        usleep(1000000);

        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */
    } else {
        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */

        /* readers-writers on accounts vector.
         * here - we are writing new account to accounts vector, therefore we are writers. */
        pthread_mutex_lock(&write_accounts_lock);

        accounts.insert(new_account);
        pthread_mutex_lock(&log_lock);
        log_file << atm_id << ": New account id is " << new_account.get_id() << " with password "
                 << new_account.get_password() << " and initial balance " <<  new_account.get_balance() << endl;
        pthread_mutex_unlock(&log_lock);
        usleep(1000000);

        pthread_mutex_unlock(&write_accounts_lock);
        /* ------------------------------END WRITE TRANSACTION------------------------------ */
    }
}

void balance(char* tokens[MAX_COMMAND], int atm_id) {
    /* readers-writers on accounts vector.
     * here - we are looking for account with a given id, therefore we are readers. */
    read_locker();

    Account fake_account(atoi(tokens[1]), -1, -1, -1);
    set<Account>::iterator this_account_it;
    this_account_it = accounts.find(fake_account);

    if (this_account_it == accounts.end()) {
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – account id " << tokens[1] << " does not exist" << endl;
        pthread_mutex_unlock(&log_lock);
        usleep(1000000);

        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */
    } else { /* there is another account with that id */
        /* first we want to release the lock because we are about
         * to become readers in specific account,
         * so we don't care that other thread will interacts with accounts in parallel */
        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */

        /* one thread can write to account while another thread can read from another account
         * therefore, we implemented the lock inside the account class */
        /* we lock the accounts vector only while we looking for account iterator */
        Account &non_const_account = const_cast<Account&> (*this_account_it);
        non_const_account.log_balance(atm_id, atoi(tokens[2])); /* lock inside, sleep inside too */
    }
}

void vip(char* tokens[MAX_COMMAND], int atm_id) {
    /* readers-writers on accounts vector.
     * here - we are looking for account with a given id, therefore we are readers. */
    read_locker();

    Account fake_account(atoi(tokens[1]), -1, -1, -1);
    set<Account>::iterator this_account_it;
    this_account_it = accounts.find(fake_account);

    if (this_account_it == accounts.end()) {
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – account id " << tokens[1] << " does not exist" << endl;
        pthread_mutex_unlock(&log_lock);
        usleep(1000000);

        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */
    } else { /* there is an account with that id */
        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */

        /* here - we are about to change vip status of account, therefore we'll become writers */
        Account &non_const_account = const_cast<Account&> (*this_account_it);
        non_const_account.log_change_vip(atm_id, atoi(tokens[2])); /* lock inside, sleep inside too */
    }
}

void deposit(char* tokens[MAX_COMMAND], int atm_id) {
    /* readers-writers on accounts vector.
     * here - we are looking for account with a given id, therefore we are readers. */
    read_locker();

    Account fake_account(atoi(tokens[1]), -1, -1, -1);
    set<Account>::iterator this_account_it;
    this_account_it = accounts.find(fake_account);

    if (this_account_it == accounts.end()) {
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – account id " << tokens[1] << " does not exist" << endl;
        pthread_mutex_unlock(&log_lock);
        usleep(1000000);

        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */
    } else { /* there is an account with that id */
        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */

        /* here - we about to change one account's balance, therefore we'll become writers */
        Account &non_const_account = const_cast<Account&> (*this_account_it);
        non_const_account.log_deposit(atm_id, atoi(tokens[2]), atoi(tokens[3])); /* lock inside, sleep inside too */
    }
}

void withdrew(char* tokens[MAX_COMMAND], int atm_id) {
    /* readers-writers on accounts vector.
     * here - we are looking for account with a given id, therefore we are readers. */
    read_locker();

    Account fake_account(atoi(tokens[1]), -1, -1, -1);
    set<Account>::iterator this_account_it;
    this_account_it = accounts.find(fake_account);

    if (this_account_it == accounts.end()) {
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – account id " << tokens[1] << " does not exist" << endl;
        pthread_mutex_unlock(&log_lock);
        usleep(1000000);

        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */
    } else { /* there is an account with that id */
        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */

        /* here - we about to change one account's balance, therefore we'll become writers */
        Account &non_const_account = const_cast<Account&> (*this_account_it);
        non_const_account.log_withdrew(atm_id, atoi(tokens[2]), atoi(tokens[3]));
    }
}

void transfer(char* tokens[MAX_COMMAND], int atm_id) {
    /* readers-writers on accounts vector.
     * here - we are looking for account with a given id, therefore we are readers. */
    read_locker();

    Account fake_source_account(atoi(tokens[1]), -1, -1, -1);
    set<Account>::iterator fake_source_account_it;
    fake_source_account_it = accounts.find(fake_source_account);

    if (fake_source_account_it == accounts.end()) {
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – account id " << tokens[1] << " does not exist" << endl;
        pthread_mutex_unlock(&log_lock);
        usleep(1000000);

        read_unlocker();
        /* ------------------------------END READ TRANSACTION------------------------------ */
    } else { /* there is an account with that id */
        /* before we release the lock, we want to look if target account exist */
        Account fake_target_account(atoi(tokens[3]), -1, -1, -1);
        set<Account>::iterator fake_target_account_it;
        fake_target_account_it = accounts.find(fake_target_account);

        if (fake_target_account_it == accounts.end()) {
            pthread_mutex_lock(&log_lock);
            log_file << "Error " << atm_id << ": Your transaction failed – account id " << tokens[3] << " does not exist" << endl;
            pthread_mutex_unlock(&log_lock);

            read_unlocker();
            /* ------------------------------END READ TRANSACTION------------------------------ */
        } else {
            /* we want to release the lock because we are about to become writers in specific two accounts */
            read_unlocker();
            /* ------------------------------END READ TRANSACTION------------------------------ */

            /* we are about to become writers in those two accounts. we don't care about accounts vector.
             * we lock first account's balance write lock, and inside that code section we lock also the second account's
             * balance for write */
            Account &non_const_source_account = const_cast<Account&> (*fake_source_account_it);
            Account &non_const_target_account = const_cast<Account&> (*fake_target_account_it);
            non_const_source_account.log_transfer(non_const_target_account, atm_id, atoi(tokens[2]), atoi(tokens[4]));
        }
    }
}

void* atm_thread_func(void* arg) {
    atm_data_t *data = (atm_data_t*) arg;

    ifstream transactions_file;
    transactions_file.open(data->atm_input_file.c_str());
    string transaction;
    vector<string> transactions;
    while(getline(transactions_file,transaction)) {
        transactions.push_back(transaction);
    }
    transactions_file.close();

//    pthread_mutex_lock(&log_lock);
//    log_file << "[DEBUG] - " << data->threadid << ": " << "transactions read ended" << endl;
//    pthread_mutex_unlock(&log_lock);
    usleep(100000);

    for (vector<string>::const_iterator i = transactions.begin(); i != transactions.end(); ++i) {
        char* c_transaction = new char [(*i).length()+1];
        strcpy(c_transaction, (*i).c_str());
        /* c_transaction now contains a c-string copy of transaction */

        char* tokens[MAX_COMMAND];
        int num_arg=0;
        tokens[0] = strtok (c_transaction," ");
        for (int i=1; i<MAX_COMMAND; i++){
            tokens[i] = strtok(NULL, " ");
            if (tokens[i] != NULL)
                num_arg++;
        }

        char op_o[] = "O";
        if (strcmp(tokens[0], op_o) == 0) {
            create_account(tokens, data->threadid);
            continue;
        }
        char op_b[] = "B";
        if (strcmp(tokens[0], op_b) == 0) {
            balance(tokens, data->threadid);
            continue;
        }
        char op_vip[] = "L";
        if (strcmp(tokens[0], op_vip) == 0) {
            vip(tokens, data->threadid);
            continue;
        }
        char op_d[] = "D";
        if (strcmp(tokens[0], op_d) == 0) {
            deposit(tokens, data->threadid);
            continue;
        }
        char op_w[] = "W";
        if (strcmp(tokens[0], op_w) == 0) {
            withdrew(tokens, data->threadid);
            continue;
        }
        char op_t[] = "T";
        if (strcmp(tokens[0], op_t) == 0) {
            transfer(tokens, data->threadid);
            continue;
        }
        usleep(100000);
    }

    pthread_mutex_lock(&write_dones_lock);
    dones--;
    pthread_mutex_unlock(&write_dones_lock);

    pthread_exit(NULL);
}



void* commissioner_thread_func(void *arg) {

    while(1) {

        read_locker();

        for (set<Account>::iterator it = accounts.begin(); it != accounts.end(); ++it) {

            double perc=((rand())%3+2)*0.01; /* get a random number between 2%-4%. */

            Account &non_const_it = const_cast<Account&> (*it);
            int commission = non_const_it.give_commission(perc);

            pthread_mutex_lock(&write_bank_lock);
            bank_account.get_commission(commission);
            pthread_mutex_unlock(&write_bank_lock);

            pthread_mutex_lock(&log_lock);
            log_file << "Bank: commissions of " << perc*100 << " % were charged, the bank gained " << commission << " $ from account " << non_const_it.get_id() << endl;
            pthread_mutex_unlock(&log_lock);
        }

        read_unlocker();

        bool alldone = false;

        pthread_mutex_lock(&read_dones_lock);
        dones_read_count++;
        if (dones_read_count == 1) {
            pthread_mutex_lock(&write_dones_lock);
        }
        pthread_mutex_unlock(&read_dones_lock);

        if (dones == 0) {
            alldone = true;
        }

        pthread_mutex_lock(&read_dones_lock);
        dones_read_count--;
        if (dones_read_count == 0) {
            pthread_mutex_unlock(&write_dones_lock);
        }
        pthread_mutex_unlock(&read_dones_lock);

        if (alldone) {
            break;
        }

        usleep(3000000);
    }
    pthread_exit(NULL);
}

void* printer_thread_func(void *arg) {

    while (1) {
        printf("\033[2J");
        printf("\033[1;1H");

        read_locker();

        for (set<Account>::iterator it = accounts.begin(); it != accounts.end(); ++it) {
            Account &non_const_it = const_cast<Account&> (*it);
            non_const_it.print();
        };

        read_unlocker();


        bool alldone = false;

        pthread_mutex_lock(&read_dones_lock);
        dones_read_count++;
        if (dones_read_count == 1) {
            pthread_mutex_lock(&write_dones_lock);
        }
        pthread_mutex_unlock(&read_dones_lock);

        if (dones == 0) {
            alldone = true;
        }

        pthread_mutex_lock(&read_dones_lock);
        dones_read_count--;
        if (dones_read_count == 0) {
            pthread_mutex_unlock(&write_dones_lock);
        }
        pthread_mutex_unlock(&read_dones_lock);

        if (alldone) {
            break;
        }
        usleep(500000);
    }
    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {

    if (argc == 1) {
        cout << "[Error] - illegal arguments - specify ATMs number and input files" << endl;
        return -1;
    }
    int N = atoi(argv[1]);
    if (argc-2 != N) {
        cout << "[Error] - illegal arguments - ATMs number (" << N << ") is not equal input files number (" << argc-2 << ")" << endl;
        return -2;
    }

    vector<string> atm_input_files;
    for (int i=2;i<argc;++i) {
        atm_input_files.push_back(argv[i]);
    }



    cout <<  "  /$$$$$$                  /$$   /$$                 /$$$$$$$                                                                                                                                      \n"
             " /$$__  $$                | $$$ | $$                | $$__  $$                                                                                                                                     \n"
             "| $$  \\__/                | $$$$| $$                | $$  \\ $$                                                                                                                                     \n"
             "| $$ /$$$$                | $$ $$ $$                | $$$$$$$                                                                                                                                      \n"
             "| $$|_  $$                | $$  $$$$                | $$__  $$                                                                                                                                     \n"
             "| $$  \\ $$                | $$\\  $$$                | $$  \\ $$                                                                                                                                     \n"
             "|  $$$$$$/       /$$      | $$ \\  $$       /$$      | $$$$$$$/                                                                                                                                     \n"
             " \\______/       |__/      |__/  \\__/      |__/      |_______/                                                                                                                                      \n"
             "  /$$$$$$   /$$$$$$  /$$      /$$$$$$  /$$$$$$  /$$$$$$$$/$$   /$$       /$$   /$$  /$$$$$$  /$$$$$$$$/$$$$$$  /$$$$$$  /$$   /$$  /$$$$$$  /$$             /$$$$$$$   /$$$$$$  /$$   /$$ /$$   /$$\n"
             " /$$__  $$ /$$__  $$| $$     |_  $$_/ /$$__  $$|__  $$__/ $$  | $$      | $$$ | $$ /$$__  $$|__  $$__/_  $$_/ /$$__  $$| $$$ | $$ /$$__  $$| $$            | $$__  $$ /$$__  $$| $$$ | $$| $$  /$$/\n"
             "| $$  \\__/| $$  \\ $$| $$       | $$  | $$  \\ $$   | $$  | $$  | $$      | $$$$| $$| $$  \\ $$   | $$    | $$  | $$  \\ $$| $$$$| $$| $$  \\ $$| $$            | $$  \\ $$| $$  \\ $$| $$$$| $$| $$ /$$/ \n"
             "| $$ /$$$$| $$  | $$| $$       | $$  | $$$$$$$$   | $$  | $$$$$$$$      | $$ $$ $$| $$$$$$$$   | $$    | $$  | $$  | $$| $$ $$ $$| $$$$$$$$| $$            | $$$$$$$ | $$$$$$$$| $$ $$ $$| $$$$$/  \n"
             "| $$|_  $$| $$  | $$| $$       | $$  | $$__  $$   | $$  | $$__  $$      | $$  $$$$| $$__  $$   | $$    | $$  | $$  | $$| $$  $$$$| $$__  $$| $$            | $$__  $$| $$__  $$| $$  $$$$| $$  $$  \n"
             "| $$  \\ $$| $$  | $$| $$       | $$  | $$  | $$   | $$  | $$  | $$      | $$\\  $$$| $$  | $$   | $$    | $$  | $$  | $$| $$\\  $$$| $$  | $$| $$            | $$  \\ $$| $$  | $$| $$\\  $$$| $$\\  $$ \n"
             "|  $$$$$$/|  $$$$$$/| $$$$$$$$/$$$$$$| $$  | $$   | $$  | $$  | $$      | $$ \\  $$| $$  | $$   | $$   /$$$$$$|  $$$$$$/| $$ \\  $$| $$  | $$| $$$$$$$$      | $$$$$$$/| $$  | $$| $$ \\  $$| $$ \\  $$\n"
             " \\______/  \\______/ |________/______/|__/  |__/   |__/  |__/  |__/      |__/  \\__/|__/  |__/   |__/  |______/ \\______/ |__/  \\__/|__/  |__/|________/      |_______/ |__/  |__/|__/  \\__/|__/  \\__/\n" <<endl;

    int atm_threads = N;
    dones = N;

    pthread_t ATMs[atm_threads];
    pthread_t commissioner;
    pthread_t printer;

    pthread_mutex_init(&log_lock, NULL);

    pthread_mutex_init(&read_accounts_lock, NULL);
    pthread_mutex_init(&write_accounts_lock, NULL);

    pthread_mutex_init(&read_dones_lock, NULL);
    pthread_mutex_init(&write_dones_lock, NULL);

    pthread_mutex_init(&read_bank_lock, NULL);
    pthread_mutex_init(&write_bank_lock, NULL);

    /* create a thread_data_argument array */
    atm_data_t atm_data[atm_threads];

    log_file.open("log.txt");

    /* create threads */
    int rc;
    for(int i = 0; i < atm_threads; ++i) {
        atm_data[i].threadid = i+1;
        atm_data[i].atm_input_file = atm_input_files[i];
        if ((rc = pthread_create(&ATMs[i], NULL, atm_thread_func, &atm_data[i]))) {
            cout << "[Error] - ATM pthread_create, rc=" << rc << endl;
            log_file.close();
            return -3;
        }
    }

    if ((rc = pthread_create(&commissioner, NULL, commissioner_thread_func, NULL))) {
        cout << "[Error] - commissioner pthread_create, rc=" << rc << endl;
        log_file.close();
        return -4;
    }

    if ((rc = pthread_create(&printer, NULL, printer_thread_func, NULL))) {
        cout << "[Error] - printer pthread_create, rc=" << rc << endl;
        log_file.close();
        return -5;
    }

    /* block until all threads complete */
    for(int i = 0; i < atm_threads; ++i) {
        pthread_join(ATMs[i], NULL);
    }
    pthread_join(commissioner,NULL);
    pthread_join(printer,NULL);


    log_file.close();


    return 0;
}