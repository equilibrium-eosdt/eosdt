#pragma once
#define EOSDTCURRENT "eosdtreserve"_n

#include "info.hpp"
#include "../../eosdt/eosdt.hpp"


namespace eosdt {

    class [[eosio::contract("eosdtreserve")]] eosdtreserve : public eosdt::contract {

    public:
        using eosdt::contract::contract;

        void currentver();
        void settingset(const ds_account &eosdtcntract_account, const ds_uint &unstake_period,
                const ds_asset &mindeposit, const ds_uint &maxdepositors);
        void paraminit();
        void transfer(const ds_account &from, const ds_account &to,
                                    ds_asset &quantity, const ds_string &memo);
        void withdraw(const ds_account &depositor);
        void sendeos();
    private:
        void remove_redundant_depositors();
        void assert_if_settlement(bool value);
        void fundssent();
        uint8_t fundssentget();

    };
} /// namespace eosdt

