//
// Created by Manor on 5/14/2019.
//

#ifndef HW2_T_ACCOUNT_H
#define HW2_T_ACCOUNT_H
#include <iostream>
#include <stdlib.h> /* atoi */
#include <cstring>
#include <fstream>
#include <pthread.h>
#include <unistd.h> /* usleep */
#include <string>
#include <vector>
#include <algorithm> /* std::reverse */
#include <iterator>
#include <sstream>
#include <pthread.h>
#include <math.h>

using namespace std;

extern pthread_mutex_t log_lock;
extern ofstream log_file;


class Account {
public:

    Account(int new_id, int new_password, int initial_amount, int atm_id);

    /* Access methods */
    int get_id() const;
    int get_password() const;
    int get_vip_status() const;
    int get_balance() const;

    void log_balance(int atm_id, int given_password);
    void log_change_vip(int atm_id, int given_password);
    void log_deposit(int atm_id, int given_password, int amount);
    void log_withdrew(int atm_id, int given_password, int amount);
    void log_transfer(Account& target_account, int atm_id, int given_password, int amount);
    void log_deposit_from_transfer(int amount);

    int give_commission(double percentage);
    void get_commission(int amount);

    void print();

    bool operator==(const Account &other_account) const;
    bool operator<(const Account &other_account) const;

private:

    int id_;
    int password_;
    int balance_;
    bool isVIP_;

    /* readers-writers on balance attribute */
    pthread_mutex_t write_lock;
    pthread_mutex_t read_lock;
    int read_count_;

    /* readers-writers on isVIP_ attribute */
    pthread_mutex_t write_vip_lock;
    pthread_mutex_t read_vip_lock;
    int read_vip_count_;

    /* no need for lock mechanism on id_ and password_ attributes
     * because it is being written only once at construction */

};

#endif //HW2_T_ACCOUNT_H
