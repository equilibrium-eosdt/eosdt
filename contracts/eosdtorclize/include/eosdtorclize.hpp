#pragma once
#define EOSDTCURRENT "eosdtorclize"_n

#include "info.hpp"
#include "../../eosdt/eosdt.hpp"


namespace eosdt {
    using namespace eosio;

    /**
     * The eosdtorclize contract defines the structures and actions that allow Equilibrium contracts, services and
     * users to manage and use price information from different datasources.
     */
    class [[contract("eosdtorclize")]] eosdtorclize : public eosdt::contract {

    public:
        using eosdt::contract::contract;

        /**
         * Allows to get last repository update as an assertion message.
         */
        void currentver();

        /**
         * Allows to get current block time and last repository update in standard output.
         */
        void getcurrent();

        /**
         * Allows for _self to perform setting initialization or update.
         *
         * @param rate_timeout - timeout for median derivation.
         * @param query_timeout - timeout for Provable queries.
         * @param provablecb1a_interval - deferred transaction interval for onchain update scheduler.
         * @param delphioracle_interval - suggested interval for offchain delphi update scheduler.
         * @param equilibriumdsp_interval - interval for LiquidApps cron updates.
         * @param validity_timeout - timeout for filtering rate updates.
         */
        void settingset(const ds_int &rate_timeout, const ds_int &query_timeout, const ds_int &provablecb1a_interval,
                        const ds_int &delphioracle_interval, const ds_int &equilibriumdsp_interval,
                        const ds_int &validity_timeout);

        /**
         * Allows `from` account to send NUT tokens to subscribe to price change events.
         * `memo` should contain base and quote symbols.
         *
         * @param from - account performing `subscribe`.
         * @param to - always _self.
         * @param quantity - stake amount, at least 2 NUT.
         * @param memo - string with subscription arguments.
         */
        void transfer(const ds_account &from, const ds_account &to, ds_asset &quantity, const ds_string &memo);

        /**
         * This action is the opposite for `transfer`/`subscribe`.
         * Allows `contract` account to unsubscribe from price change events and unstake NUT tokens.
         *
         * @param contract - account performing `unsubscribe`.
         * @param asset_symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         */
        void unsubscribe(const ds_account &contract, const ds_symbol &asset_symbol, const ds_symbol &base);

        /**
         * Allows _self to send ratechanged event to subscriber's contract.
         * Subscriber should check eosdtorclize auth and access just updated `oraclerates` table.
         *
         * @param contract - subscribed account.
         * @param asset_symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         */
        void ratechanged(const ds_account &contract, const ds_symbol &asset_symbol, const ds_symbol &base);

        /**
         * Allows _self to add query with parameters of datasource needed.
         *
         * @param symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param source_contract - account of datasource, to access tables and check auth when receiving price callbacks.
         * @param query - query string in datasource format.
         * @param price_type - math transformation of source price to stored in `oraclerates` table.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         * @param filter - price change considered void to prevent resource draining.
         */
        void queryadd(const ds_symbol &symbol, const ds_account &source_contract, const ds_string &query,
                const uint8_t price_type, const ds_symbol &base, const double filter);

        /**
         * This action is the opposite for `queryadd`, it removes previously added query.
         *
         * @param symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param source_contract - account of datasource.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         */
        void querydel(const ds_symbol &symbol, const ds_account &source_contract, const ds_symbol &base);

        /**
         * Provable API specific action, it called from Provable auth and supplies query result with proof.
         *
         * @param query_id - checksum of previously sent query.
         * @param result - binary data representing query result.
         * @param proof - binary data representing query proof.
         */
        void callback(const checksum256 &query_id, const std::vector<unsigned char> &result,
                      const std::vector<unsigned char> &proof);

        /**
         * Allows any third party account to initiate price refresh for Provable and Delphioracle sources.
         *
         * @param symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         */
        void comonrefresh(const ds_symbol &symbol, const ds_symbol &base);

        /**
         * Allows any third party account to initiate price refresh for Provable and Delphioracle sources.
         *
         * @param symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         */
        void refreshutil(const ds_symbol &symbol, const ds_symbol &base);

        /**
         * Allows any third party account to initiate price refresh for Provable and Delphioracle sources.
         *
         * @param symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         */
        void masterefresh(const ds_symbol &symbol, const ds_symbol &base);

        /**
         * Allows _self to stop onchain deferred updates.
         *
         * @param symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         */
        void stoprefresh(const ds_symbol &symbol, const ds_symbol &base);

        /**
         * Allows _self to start onchain deferred updates.
         *
         * @param symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         */
        void startrefresh(const ds_symbol &symbol, const ds_symbol &base);

        /**
         * Allows any third party account to initiate price refresh for Delphioracle source.
         */
        void delphirefres();

        /**
         * LiquidApps API specific action.
         *
         * @param uri
         * @param data
         */
        void filter(std::vector<char> uri,std::vector<char> data);

        /**
         * LiquidApps API specific action.
         *
         * @param current_provider
         * @param package
         * @param size
         * @param uri
         * @param data
         */
        void xgeturi(const ds_account &current_provider, const ds_account &package, const ds_uint &size,
                     const std::vector<char> &uri, const std::vector<char> &data);

        /**
         * LiquidApps API specific action.
         *
         * @param current_provider
         * @param package
         * @param size
         * @param uri
         */
        void xorcclean(const ds_account &current_provider, const ds_account &package, const ds_uint &size,
                       const std::vector<char> &uri);

        /**
         * LiquidApps API specific action.
         *
         * @param service
         * @param action
         * @param provider
         * @param package
         * @param signalRawData
         */
        void xsignal(const ds_account &service, const ds_account &action, const ds_account &provider,
                     const ds_account &package, const std::vector<char> &signalRawData);
        /**
         * LiquidApps API specific action.
         *
         * @param current_provider
         * @param package
         * @param timer
         * @param payload
         * @param seconds
         */
        void xschedule(const ds_account &current_provider, const ds_account &package, const ds_account &timer,
                       const std::vector<char> &payload, ds_uint seconds);
        /**
         * Allows _self to initiate or recover LiquidApps cron service.
         */
        void orarecover();

        /**
         * Allows any third party account to initiate price refresh for LiquidApps source.
         * This action should be executed only on LiquidApps DSP node, e.g. https://dsp.eosdt.com
         * @param symbol - quote symbol of the price pair, e.g. USD in NUT/USD.
         * @param base - base symbol of the price pair, e.g. NUT in NUT/USD.
         */
        void liquidrefres(const ds_symbol &symbol, const ds_symbol &base);

    private:
        void on_rate_changed(const ds_symbol &asset_symbol, const ds_symbol &base);

        auto parse_price(const ds_symbol &symbol, const char *data);

        auto parse_rate(const ds_symbol &token_symbol, const ds_string &data);

        auto get_usd_rate();

        void set_median(const ds_time &threshold, orarate &orarate);

        auto ratebyprtype(const price_type &price_type, const ds_asset &data);

        void rate_set(const ds_symbol &token_symbol, const source_type &source, const price_type &price_type,
                const ds_string &data);
        void rate_set(const ds_symbol &token_symbol, const source_type &source, const price_type &price_type,
                      const ds_symbol &base, const ds_string &data);
        void rate_set(const source_type &source, const price_type &price_type, const ds_asset &data);
        void rate_set(const source_type &source, const price_type &price_type, const ds_symbol &base,
                const ds_asset &data);

        bool is_query_running(const oraqueries & query, const ds_time & curr_now);

        void provablerefresh_internal(const oraqueries & query);

        void create_tran(const ds_symbol &symbol, const ds_symbol &base, const ds_int &interval);

        void cancel_tran(const ds_symbol &symbol, const ds_symbol &base);

        // Holds the last datapoints_count datapoints from qualified Delphi oracles
        struct datapoints {
            ds_ulong id;
            ds_account owner;
            ds_ulong value;
            ds_ulong median;
            time_point timestamp;

            ds_ulong primary_key() const { return id; }
            ds_ulong by_timestamp() const {return timestamp.elapsed.to_seconds();}
            ds_ulong by_value() const {return value;}
        };
        // Multi index Delphi types definition
        typedef multi_index<"datapoints"_n, datapoints,
            indexed_by<"value"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_value>>,
            indexed_by<"timestamp"_n, const_mem_fun<datapoints, uint64_t, &datapoints::by_timestamp>>> delphi_datapoints;

        void delphirefresh_internal(const oraqueries & query);

    };
} /// namespace eosdt

