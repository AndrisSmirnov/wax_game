#include <eosio/eosio.hpp>

class [[eosio::contract]] my_collection : public eosio::contract
{
private:
  struct [[eosio::table]] Collections
  {
    name collection_name;
    name author;
    bool allow_notify;
    vector<name> authorized_accounts;
    vector<name> notify_accounts;
    double market_fee;

    uint64_t primary_key() const { return collection_name.value; };
  };

  typedef multi_index<"collections"_n, Collections> collections_table;
  collections_table _collections;
}