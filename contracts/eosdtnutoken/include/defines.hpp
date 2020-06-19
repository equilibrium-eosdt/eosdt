#pragma once


#define NUT_SYMBOL_STR "NUT"
#define NUT_SYMBOL_PAD_STR " " NUT_SYMBOL_STR
#define NUT_SYMBOL_DECIMAL 9
#define NUT_SYMBOL ds_symbol(NUT_SYMBOL_STR,NUT_SYMBOL_DECIMAL)
#define NUT_SYMBOL_VALUE NUT_SYMBOL.value()
#define SINGLETON_ID 0

#ifndef ds_account
#define ds_account eosio::name
#endif

#ifndef SAME_PAYER
#define SAME_PAYER ds_account(0)
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

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include "info.hpp"
