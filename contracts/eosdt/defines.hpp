#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/system.hpp>
#include <eosio/crypto.hpp>
#include <eosio/privileged.hpp>
#include <eosio/binary_extension.hpp>

#define ds_int int
#define ds_uint unsigned int
#define ds_ulong unsigned long long
#define ds_long long long
#define ds_llong __int128_t
#define ds_string std::string
#define ds_nullptr std::nullptr_t

constexpr static ds_uint MONTHS_IN_SECONDS = (ds_uint) (365.25 * 24 * 60 * 60 / 12);
#define UNSUBSCRIBE_FREEZE_PERIOD 86400
#define STOP_REFRESH ds_time(0)
#define BEFORE_LISTING_RATE 30.0L
#define QUERY_TIMEOUT_SECONDS 300
#define RATES_UPDATE_SECONDS 300
#define RATES_OUTDATED_SECONDS 21600
#define POSITIVEINFINITY 999999999.0L

#define ds_time eosio::time_point_sec
#define ds_account eosio::name
#define ds_symbol eosio::symbol
#define ds_asset eosio::asset
#define ds_checksum eosio::checksum256
#define ds_public_key eosio::public_key
#define N(X) name{#X}

#define PRINT_STARTED(ACTION)
#define PRINT_FINISHED(ACTION)


#define EOS_SYMBOL_STR "EOS"
#define EOS_SYMBOL_DECIMAL 4
#define REX_SYMBOL_STR "REX"
#define REX_SYMBOL_DECIMAL 4
#define USD_SYMBOL_STR "USD"
#define USD_SYMBOL_DECIMAL 4
#define STABLE_SYMBOL_STR "EOSDT"
#define STABLE_SYMBOL_PAD_STR " " STABLE_SYMBOL_STR
#define STABLE_SYMBOL_DECIMAL 9
#define UTILITY_SYMBOL_STR "NUT"
#define UTILITY_SYMBOL_PAD_STR " " UTILITY_SYMBOL_STR
#define UTILITY_SYMBOL_DECIMAL 9
#define NUT_SYMBOL_STR "NUT"
#define NUT_SYMBOL_PAD_STR " " NUT_SYMBOL_STR
#define NUT_SYMBOL_DECIMAL 9
#define DAPP_SYMBOL_STR "DAPP"
#define DAPP_SYMBOL_PAD_STR " " DAPP_SYMBOL_STR
#define DAPP_SYMBOL_DECIMAL 4
#define KG_SYMBOL_STR "KGRAM"
#define KG_SYMBOL_PAD_STR " " KG_SYMBOL_STR
#define KG_SYMBOL_DECIMAL 4

#define EOSCTRACT_STR "eosio.token"
#define EOSDTCNTRACT_STR "eosdtcntract"
#define EOSDTORCLIZE_STR "eosdtorclize"
#define ORACLERATES_STR "orarates"


#define EOSSYSTEM "eosio"_n
#define EOSCTRACT "eosio.token"_n
#define EOSSTAKE "eosio.stake"_n
#define EOSDTCNTRACT "eosdtcntract"_n
#define EOSDTORCLIZE "eosdtorclize"_n
#define EOSDTLIQDATR "eosdtliqdatr"_n
#define EOSDTNUTOKEN "eosdtnutoken"_n
#define EOSDTSTTOKEN "eosdtsttoken"_n
#define EOSDTBPPROXY "eosdtbpproxy"_n
#define EOSDTGOVERNC "eosdtgovernc"_n
#define EOSDTRESERVE "eosdtreserve"_n
#define EOSDTLOCKUPP "eosdtlockupp"_n
#define EOSDTKGTOKEN "eosdtkgtoken"_n
#define EOSDTPOSCONT "eosdtposcont"_n

#define BLOCKPRODUCEPROPOSAL "blockproduce"_n

#define ORACLERATES "orarates"_n
#define ORACLEREFRESH "refreshutil"_n

#define ORACLESOURCE "provablecb1a"_n

#define EOS_SYMBOL ds_symbol(EOS_SYMBOL_STR,EOS_SYMBOL_DECIMAL)
#define EOS_SYMBOL_VALUE EOS_SYMBOL.value()
#define REX_SYMBOL ds_symbol(REX_SYMBOL_STR,REX_SYMBOL_DECIMAL)
#define REX_SYMBOL_VALUE REX_SYMBOL.value()
#define USD_SYMBOL ds_symbol(USD_SYMBOL_STR,USD_SYMBOL_DECIMAL)
#define USD_SYMBOL_VALUE USD_SYMBOL.value()
#define STABLE_SYMBOL ds_symbol(STABLE_SYMBOL_STR,STABLE_SYMBOL_DECIMAL)
#define STABLE_SYMBOL_VALUE STABLE_SYMBOL.value()
#define EOSDT_SYMBOL ds_symbol(STABLE_SYMBOL_STR,STABLE_SYMBOL_DECIMAL)
#define EOSDT_SYMBOL_VALUE EOSDT_SYMBOL.value()
#define UTILITY_SYMBOL ds_symbol(UTILITY_SYMBOL_STR,UTILITY_SYMBOL_DECIMAL)
#define UTILITY_SYMBOL_VALUE UTILITY_SYMBOL.value()
#define NUT_SYMBOL ds_symbol(NUT_SYMBOL_STR,NUT_SYMBOL_DECIMAL)
#define NUT_SYMBOL_VALUE NUT_SYMBOL.value()
#define DAPP_SYMBOL ds_symbol(DAPP_SYMBOL_STR,DAPP_SYMBOL_DECIMAL)
#define DAPP_SYMBOL_VALUE DAPP_SYMBOL.value()
#define EMPTY_SYMBOL (ds_symbol(0ull))
#define KG_SYMBOL ds_symbol(KG_SYMBOL_STR,KG_SYMBOL_DECIMAL)
#define KG_SYMBOL_VALUE KG_SYMBOL.value()

#define NEED_TO_SET_UP "need to set up"
#define ALREADY_STORED "already stored"
#define DOES_NOT_EXIST "does not exist"
#define AS_AN_ACCOUNT "as an account"
#define DELETE "delete"
#define STARTED "started"
#define FINISHED "finished"

#define EOSIO_DISPATCH_EX(TYPE, MEMBERS) \
extern "C" { \
   void apply( ds_ulong receiver, ds_ulong code, ds_ulong action ) { \
        if( code != receiver && action != "transfer"_n.value && action != "ratechanged"_n.value )return;\
        switch( action ) {EOSIO_DISPATCH_HELPER( TYPE, MEMBERS )} \
   } \
}
