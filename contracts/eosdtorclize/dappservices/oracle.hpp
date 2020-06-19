#pragma once
#include "../dappservices/dappservices.hpp"

#define SVC_RESP_ORACLE(name) \
    SVC_RESP_X(oracle,name)



#ifdef LIQUIDX

#define SVC_CONTRACT_NAME_ORACLE SVC_CONTRACT_NAME_ORACLE_undefined 

#else
#define SVC_CONTRACT_NAME_ORACLE oracleservic 

#endif

#include "../dappservices/_oracle_impl.hpp"



#define ORACLE_DAPPSERVICE_BASE_ACTIONS \
SVC_ACTION(geturi, true, ((std::vector<char>)(uri)),     \
         ((uint32_t)(size))((std::vector<char>)(uri)), \
         ((uint32_t)(size))((std::vector<char>)(uri))((std::vector<char>)(data)),TONAME(SVC_CONTRACT_NAME_ORACLE) ) \
{ \
    _oracle_geturi(size, uri, data, current_provider); \
    SEND_SVC_SIGNAL(geturi, current_provider, package, size, uri)                         \
}; \
SVC_ACTION(orcclean, false, ((std::vector<char>)(uri)),     \
         ((uint32_t)(size))((std::vector<char>)(uri)), \
         ((uint32_t)(size))((std::vector<char>)(uri)),TONAME(SVC_CONTRACT_NAME_ORACLE) ) \
{ \
    _oracle_orcclean(size, uri, current_provider); \
    SEND_SVC_SIGNAL(orcclean, current_provider, package, size, uri)                         \
};  \
static void svc_oracle_geturi(std::vector<char> uri) { \
    SEND_SVC_REQUEST(geturi, uri) \
};\
static void svc_oracle_orcclean(std::vector<char> uri) { \
    SEND_SVC_REQUEST(orcclean, uri) \
};


#ifdef ORACLE_DAPPSERVICE_ACTIONS_MORE
#define ORACLE_DAPPSERVICE_ACTIONS \
  ORACLE_DAPPSERVICE_BASE_ACTIONS \
  ORACLE_DAPPSERVICE_ACTIONS_MORE() 


#else
#define ORACLE_DAPPSERVICE_ACTIONS \
  ORACLE_DAPPSERVICE_BASE_ACTIONS
#endif


#ifndef ORACLE_SVC_COMMANDS
#define ORACLE_SVC_COMMANDS() (xgeturi)(xorcclean)


#ifndef ORACLE_DAPPSERVICE_SKIP_HELPER
struct oracle_svc_helper{
    ORACLE_DAPPSERVICE_ACTIONS
};
#endif

#endif