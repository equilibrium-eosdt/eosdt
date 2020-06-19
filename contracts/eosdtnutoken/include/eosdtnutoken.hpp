#pragma once
#include "defines.hpp"

namespace eosdt {

    /**
     * The eosdtnutoken contract defines the structures and actions that allow users to create, issue, and manage
     * NUT tokens.
     */
    class [[eosio::contract("eosdtnutoken")]] eosdtnutoken : public eosio::contract {

    public:
        using contract::contract;

        /**
          * Allows `issuer` account to create a token in supply of `maximum_supply`. If validation is successful a new entry in statstable for token symbol scope gets created.
          *
          * @param issuer - the account that creates the token,
          * @param maximum_supply - the maximum supply set for the token created.
          *
          * @pre Token symbol has to be valid,
          * @pre Token symbol must not be already created,
          * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 1^62 - 1.
          * @pre Maximum supply must be positive;
          */
        [[eosio::action]]
        void create(ds_account issuer,
                    ds_asset maximum_supply);

        /**
          *  This action issues to `to` account a `quantity` of tokens.
          *
          * @param to - the account to issue tokens to, it must be the same as the issuer,
          * @param quntity - the amount of tokens to be issued,
          * @memo - the memo string that accompanies the token issue transaction.
          */
        [[eosio::action]]
        void issue(ds_account to, ds_asset quantity, ds_string memo);

        /**
          * Allows `from` account to transfer to `to` account the `quantity` tokens.
          * One account is debited and the other is credited with quantity tokens.
          *
          * @param from - the account to transfer from,
          * @param to - the account to be transferred to,
          * @param quantity - the quantity of tokens to be transferred,
          * @param memo - the memo string to accompany the transaction.
          */
        [[eosio::action]]
        void transfer(ds_account from,
                      ds_account to,
                      ds_asset quantity,
                      ds_string memo);

        /**
         * Allows `ram_payer` to create an account `owner` with zero balance for
         * token `symbol` at the expense of `ram_payer`.
         *
         * @param owner - the account to be created,
         * @param symbol - the token to be payed with by `ram_payer`,
         * @param ram_payer - the account that supports the cost of this action.
         *
         * More information can be read [here](https://github.com/EOSIO/eosio.contracts/issues/62)
         * and [here](https://github.com/EOSIO/eosio.contracts/issues/61).
         */
        [[eosio::action]]
        void open(const ds_account &owner, const ds_symbol &symbol, const ds_account &ram_payer);

        /**
         * This action is the opposite for open, it closes the account `owner`
         * for token `symbol`.
         *
         * @param owner - the owner account to execute the close action for,
         * @param symbol - the symbol of the token to execute the close action for.
         *
         * @pre The pair of owner plus symbol has to exist otherwise no action is executed,
         * @pre If the pair of owner plus symbol exists, the balance has to be zero.
         */
        [[eosio::action]]
        void close(const ds_account &owner, const ds_symbol &symbol);

        /**
         *
         * @param sym - token symbol to get the supply for.
         * @return - returns the supply by token symbol.
         */
        inline ds_asset get_supply(ds_symbol sym) const;

        /**
         *
         * @param owner - account to get the balance for.
         * @param sym - token symbol to get the balance for.
         * @return - return the balance by account.
         */
        inline ds_asset get_balance(ds_account owner, ds_symbol sym) const;

        // Accounts table.
        // `balance` - token balance for given account scope.
        struct [[eosio::table]] account {
            ds_asset balance;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
        };
        typedef eosio::multi_index<"accounts"_n, account> accounts;

        // Currency statistics table.
        // `supply` - current supply of NUT tokens.
        // `max_supply` - theoretical maximum supply of NUT tokens.
        // `issuer` - creator account of NUT tokens.
        struct [[eosio::table]] currency_stats {
            ds_asset supply;
            ds_asset max_supply;
            ds_account issuer;

            uint64_t primary_key() const { return supply.symbol.code().raw(); }
        };
        typedef eosio::multi_index<"stat"_n, currency_stats> stats;

        /**
          * The opposite for create action, if all validations succeed,
          * it debits the statstable.supply amount and credits nutparams.burnt_supply.
          * The difference with retire is that tokens can't be issued once again,
          * they are burnt forever.
          *
          * @param from - account willing to burn tokens.
          * @param quantity - the quantity of tokens to burn,
          * @param memo - the memo string to accompany the transaction.
          */
        [[eosio::action]]
        void burn(ds_account from,
                  ds_asset quantity,
                  ds_string memo);

        /** Allows `to` account to receive from `from` account the `quantity` tokens.
          * One account is debited and the other is credited with quantity tokens.
          *
          * @param from - the account to receive from,
          * @param to - the account to be received to, must be whitelisted in `nutstable` table
          * @param quantity - the quantity of tokens to be received,
          * @param memo - the memo string to accompany the transaction.
          */
        [[eosio::action]]
        void receive(ds_account from,
                     ds_account to,
                     ds_asset quantity,
                     ds_string memo);

        /**
         * Allows for _self to lock transfers for given list of NUT accounts.
         * Accounts' balances increase nutparams.locked_supply.
         *
         * @param lock_list - list of accounts to be locked.
         */
        [[eosio::action]]
        void lockacc(std::vector <ds_account> &lock_list);

        /**
         * Allows for _self to unlock transfers for given list of NUT accounts.
         * Accounts' balances decrease nutparams.locked_supply.
         *
         * @param unlock_list - list of accounts to be unlocked.
         */
        [[eosio::action]]
        void unlockacc(std::vector <ds_account> &unlock_list);

        /**
         * Allows for _self to perform one-time data migration.
         *
         * @param checksumm
         */
        [[eosio::action]]
        void paraminit(const ds_asset &checksumm);

        // A NUT lock struct.
        // `account` - an account that the lock is for.
        struct [[eosio::table]] nutlock
        {
            ds_account account;

            uint64_t
            primary_key() const { return account.value; }
        };
        typedef eosio::multi_index<"nutlocks"_n, nutlock> nutlocks_table;

        // A NUT param struct.
        // `param_id` - singleton table, so always zero.
        // `burnt_supply` - NUT supply that has been burnt.
        // `locked_supply` - NUT supply currently held by locked accounts.
        struct [[eosio::table]] nutparam {
            uint64_t param_id;
            ds_asset burnt_supply;
            ds_asset locked_supply;

            uint64_t primary_key() const { return param_id; }
        };
        typedef eosio::multi_index<"nutparams"_n, nutparam> nutparams_table;

        // A NUT stable table.
        // `account` - whitelisted account of a stablecoin token.
        // `is_own` - true if account is owned by Equilibrium, false for partner account.
        // `peg` - the currency peg is maintained for.
        struct [[eosio::table]] nutstable {
            ds_account account;
            uint8_t is_own;
            ds_string peg;

            uint64_t
            primary_key() const { return account.value; }
        };
        typedef eosio::multi_index<"nutstables"_n, nutstable> nutstables_table;

        /**
         * Allows _self to add another stablecoin token account to nutstables table.
         * This account should be a valid eosio.token contract and have tokissuers table for further checks.
         * Example: Equilibrium's own EOSDT stablecoin pegged to USD and represented by eosdtsttoken contract.
         *
         * @param account - the account to be whitelisted in `nutstable` table.
         * @param is_own - true if account is owned by Equilibrium, false for partner account.
         * @param peg - the currency peg is maintained for.
         */
        [[eosio::action]]
        void stableadd(ds_account account, uint8_t is_own, ds_string peg);

        /**
         * Allows _self to remove existing stablecoin token account from nutstables table.
         *
         * @param account - the account to be delisted from `nutstable` table.
         */
        [[eosio::action]]
        void stablerem(ds_account account);

        // A token issuers table stored on NUT stables accounts.
        // `issuer` - account allowed to issue stablecoin tokens.
        // `max_supply` - theoretical maximum supply of stablecoin tokens.
        // `supply` - current supply of stablecoin tokens.
        // `is_active` - false for deactivated issuer, no longer issuing stablecoin tokens.
        struct [[eosio::table]] tokissuer
        {
            ds_account issuer;
            ds_asset max_supply;
            ds_asset supply;
            bool is_active;
            uint64_t
            primary_key() const { return issuer.value; }
        };
        typedef eosio::multi_index<"tokissuers"_n, tokissuer> tokissuers_table;

    protected:
        void sub_balance(ds_account owner, ds_asset value);

        void add_balance(ds_account owner, ds_asset value, ds_account ram_payer);

        ds_asset hash_by_nutlocks_balance();
    };
}
