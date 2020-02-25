#pragma once

#define EOSDTCURRENT "eosdtnutoken"_n
#define NUT_SYMBOL_STR "NUT"
#define NUT_SYMBOL_PAD_STR " " NUT_SYMBOL_STR
#define NUT_SYMBOL_DECIMAL 9
#define NUT_SYMBOL ds_symbol(NUT_SYMBOL_STR,NUT_SYMBOL_DECIMAL)
#define NUT_SYMBOL_VALUE NUT_SYMBOL.value()

#ifndef ds_account
#define ds_account eosio::name
#endif

#ifndef ds_asset
#define ds_asset eosio::asset
#endif

#ifndef ds_symbol
#define ds_symbol eosio::symbol
#endif

#ifndef ds_string
#define ds_string std::string
#endif

#ifndef ds_checksum
#define ds_checksum eosio::checksum256
#endif

#define DELETEDATA

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include "info.hpp"
