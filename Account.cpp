//
// Created by Manor on 5/14/2019.
//

#include "Account.h"


Account::Account(int new_id, int new_password, int initial_amount, int atm_id) : id_(new_id), password_(new_password),
                                                                                 balance_(initial_amount), isVIP_(false),
                                                                                 read_count_(0), read_vip_count_(0) {
    pthread_mutex_init(&write_lock, NULL);
    pthread_mutex_init(&read_lock, NULL);
    pthread_mutex_init(&read_vip_lock, NULL);
    pthread_mutex_init(&write_vip_lock, NULL);


}

int Account::get_id() const {return id_;};
int Account::get_balance() const { return balance_;};
int Account::get_password() const {return password_;};

bool Account::operator==(const Account &other_account) const { return id_<other_account.get_id(); }
bool Account::operator<(const Account &other_account) const { return id_<other_account.get_id(); }

void Account::log_balance(int atm_id, int given_password) {

    /* only readers for password and it have never changed after creation,
     * therefore we can allow multiple readers at parallel - means:
     * we don't need to lock it at all! */
    if (given_password != password_) {
        /* but obviously we do need to lock the logging operation... */
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – password for account id " << id_ << " is incorrect" << endl;
        pthread_mutex_unlock(&(log_lock));
    } else {
        /* readers-writers mechanism on the balance.
         * we are readers here so: */
        pthread_mutex_lock(&read_lock);
        read_count_ ++;
        if (read_count_ == 1) {
            pthread_mutex_lock(&write_lock);
        }
        pthread_mutex_unlock(&read_lock);

        pthread_mutex_lock(&log_lock);
        log_file << atm_id << ": Account " << id_ << " balance is " << balance_ << endl;
        pthread_mutex_unlock(&log_lock);
        usleep(1000000);

        pthread_mutex_lock(&read_lock);
        read_count_ --;
        if (read_count_ == 0) {
            pthread_mutex_unlock(&write_lock);
        }
        pthread_mutex_unlock(&read_lock);
    }
}

void Account::log_change_vip(int atm_id, int given_password) {
    /* only readers for password and it have never changed after creation,
     * therefore we can allow multiple readers at parallel - means:
     * we don't need to lock it at all! */
    if (given_password != password_) {
        /* but obviously we do need to lock the logging operation... */
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – password for account id " << id_ << " is incorrect" << endl;
        pthread_mutex_unlock(&(log_lock));
    } else {
        /* readers-writers mechanism on the balance.
         * we are writers here so: */
        pthread_mutex_lock(&write_vip_lock);

        isVIP_ = true;
        usleep(1000000);

        pthread_mutex_unlock(&write_vip_lock);
    }
}

void Account::log_deposit(int atm_id, int given_password, int amount) {
    /* only readers for password and it have never changed after creation,
     * therefore we can allow multiple readers at parallel - means:
     * we don't need to lock it at all! */
    if (given_password != password_) {
        /* but obviously we do need to lock the logging operation... */
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – password for account id " << id_ << " is incorrect" << endl;
        pthread_mutex_unlock(&(log_lock));
    } else {
        /* readers-writers mechanism on the balance.
         * we are writers here so: */
        pthread_mutex_lock(&write_lock);

        balance_ += amount;
        pthread_mutex_lock(&log_lock);
        log_file << atm_id << ": Account " << id_ << " new balance is " << balance_ << " after " << amount << " $ was deposited" << endl;
        pthread_mutex_unlock(&(log_lock));
        usleep(1000000);

        pthread_mutex_unlock(&write_lock);
    }
}

void Account::log_withdrew(int atm_id, int given_password, int amount) {
    /* only readers for password and it have never changed after creation,
     * therefore we can allow multiple readers at parallel - means:
     * we don't need to lock it at all! */
    if (given_password != password_) {
        /* but obviously we do need to lock the logging operation... */
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – password for account id " << id_ << " is incorrect" << endl;
        pthread_mutex_unlock(&(log_lock));
    } else {
        /* readers-writers mechanism on the balance.
         * here we also read from balance (to check that we have enough money to withdrew),
         * but it's one small action to perform, so we'll leave it under write_lock */
        pthread_mutex_lock(&write_lock);

        if (amount > balance_) {
            pthread_mutex_lock(&log_lock);
            log_file << "Error " << atm_id << ": Your transaction failed – account id " << id_ << " balance is lower than " << amount << endl;
            pthread_mutex_unlock(&(log_lock));
        } else {
            balance_ -= amount;
            pthread_mutex_lock(&log_lock);
            log_file << atm_id << ": Account " << id_ << " new balance is " << balance_ << " after " << amount
                     << " $ was withdrew" << endl;
            pthread_mutex_unlock(&(log_lock));
            usleep(1000000);
        }

        pthread_mutex_unlock(&write_lock);
    }
}

void Account::log_transfer(Account &target_account, int atm_id, int given_password, int amount) {
    /* only readers for password and it have never changed after creation,
     * therefore we can allow multiple readers at parallel - means:
     * we don't need to lock it at all! */
    if (given_password != password_) {
        /* but obviously we do need to lock the logging operation... */
        pthread_mutex_lock(&log_lock);
        log_file << "Error " << atm_id << ": Your transaction failed – password for account id " << id_
                 << " is incorrect" << endl;
        pthread_mutex_unlock(&(log_lock));
    } else {
        /* readers-writers mechanism on the balance.
         * here we also read from balance (to check that we have enough money to withdrew),
         * but it's one small action to perform, so we'll leave it under write_lock */
        pthread_mutex_lock(&write_lock);

        if (amount > balance_) {
            pthread_mutex_lock(&log_lock);
            log_file << "Error " << atm_id << ": Your transaction failed – account id " << id_ << " balance is lower than " << endl;
            pthread_mutex_unlock(&(log_lock));
        } else {
            balance_ -= amount;
            target_account.log_deposit_from_transfer(amount);
            pthread_mutex_lock(&log_lock);
            log_file << atm_id << ": Transfer " << amount << " from account " << id_ << " to account "
                     << target_account.get_id() << " new account balance is " << balance_
                     << " new target account balance is " << target_account.get_balance() << endl;
            pthread_mutex_unlock(&(log_lock));
            usleep(1000000);
        }

        pthread_mutex_unlock(&write_lock);
    }
}

void Account::log_deposit_from_transfer(int amount) {
    pthread_mutex_lock(&write_lock);

    balance_ += amount;

    pthread_mutex_unlock(&write_lock);
}

int Account::give_commission(double percentage) {

    int amount;

    /* reader on isVIP attribute */
    pthread_mutex_lock(&read_vip_lock);
    read_vip_count_++;
    if (read_vip_count_ == 1) {
        pthread_mutex_lock(&write_vip_lock);
    }
    pthread_mutex_unlock(&read_vip_lock);

    if (!isVIP_) {
        pthread_mutex_lock(&write_lock);
        amount = round(balance_ * percentage);
        balance_ -= amount;
        pthread_mutex_unlock(&write_lock);
        /* ------------------------------END WRITE TRANSACTION------------------------------ */
    }

    pthread_mutex_lock(&read_vip_lock);
    read_vip_count_--;
    if (read_vip_count_ == 0) {
        pthread_mutex_unlock(&write_vip_lock);
    }
    pthread_mutex_unlock(&read_vip_lock);

    return amount;
}

void Account::get_commission(int amount) {

    pthread_mutex_lock(&write_lock);

    balance_ += amount;

    pthread_mutex_unlock(&write_lock);

}

void Account::print() {

    pthread_mutex_lock(&read_lock);
    read_count_ ++;
    if (read_count_ == 1) {
        pthread_mutex_lock(&write_lock);
    }
    pthread_mutex_unlock(&read_lock);

    cout << "Account " << id_ << ": Balance - " << balance_ << " $ , Account Password - " << password_ << endl;

    pthread_mutex_lock(&read_lock);
    read_count_ --;
    if (read_count_ == 0) {
        pthread_mutex_unlock(&write_lock);
    }
    pthread_mutex_unlock(&read_lock);
}
