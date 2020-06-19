#pragma once
#include "defines.hpp"

namespace eosdt {

    /**
     * The eosdtsttoken contract defines the structures and actions that allow users to create, issue, and manage
     * EOSDT tokens.
     */
    class [[eosio::contract("eosdtsttoken")]] eosdtsttoken :public eosio::contract {

    public:
        using contract::contract;

        /**
          * Allows _self account to create a token with symbol of `maximum_supply`. If validation is successful a new entry in statstable for token symbol scope gets created.
          *
          * @param issuer - backward compatibility with eosio.token, actual issuers are defined in `tokissuer` table
          * @param maximum_supply - backward compatibility with eosio.token, only token's symbol is used
          *
          * @pre Token symbol has to be valid,
          * @pre Token symbol must not be already created,
          */
        [[eosio::action]]
        void create(ds_account issuer,ds_asset maximum_supply);

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
          * it debits the statstable.supply and tokissuer.supply amount.
          * The difference with eosio.token retire is that tokens are retired
          * from specific tokissuer account to maintain it's own supply.
          *
          * @param from - tokissuer account willing to retire tokens.
          * @param quantity - the quantity of tokens to retire,
          * @param memo - the memo string to accompany the transaction.
          */
        [[eosio::action]]
        void retire(ds_account from,
                  ds_asset quantity,
                  ds_string memo);

        /**
         * Allows _self to add another overcollateralized positions account to `tokissuer` table.
         * Maximum supply argument increases the overall EOSDT statstable.maximum_supply.
         * Example: Equilibrium's eosdtcntract (EOS collateral) and eosdtpbtcpos (PBTC collateral) contracts.
         *
         * @param issuer - the account that manages the token,
         * @param maximum_supply - the maximum supply set for the issuer.
         */
        [[eosio::action]]
        void issueradd(ds_account issuer, ds_asset maximum_supply);

        /**
         * Allows _self to deactivate existing overcollateralized positions account.
         *
         * @param issuer - the account that manages the token.
         */
        [[eosio::action]]
        void issuerrem(ds_account issuer);

        // A token issuers table.
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
        void issuer_issue(ds_account issuer, ds_asset quantity);
        void issuer_retire(ds_account issuer, ds_asset quantity);
    };

} /// namespace eosio
