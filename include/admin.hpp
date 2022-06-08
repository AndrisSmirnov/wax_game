#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <atomicdata.hpp>

namespace eosiosystem
{
  class system_contract;
}

namespace eosio
{

  using std::map;
  using std::string;

  /**
   * The `eosio.token` sample system contract defines the structures and actions that allow users to create, issue, and manage tokens for EOSIO based blockchains. It demonstrates one way to implement a smart contract which allows for creation and management of tokens. It is possible for one to create a similar contract which suits different needs. However, it is recommended that if one only needs a token with the below listed actions, that one uses the `eosio.token` contract instead of developing their own.
   *
   * The `eosio.token` contract class also implements two useful public static methods: `get_supply` and `get_balance`. The first allows one to check the total supply of a specified token, created by an account and the second allows one to check the balance of a token for a specified account (the token creator account has to be specified as well).
   *
   * The `eosio.token` contract manages the set of tokens, accounts and their corresponding balances, by using two internal multi-index structures: the `accounts` and `stats`. The `accounts` multi-index table holds, for each row, instances of `account` object and the `account` object holds information about the balance of one token. The `accounts` table is scoped to an eosio account, and it keeps the rows indexed based on the token's symbol.  This means that when one queries the `accounts` multi-index table for an account name the result is all the tokens that account holds at the moment.
   *
   * Similarly, the `stats` multi-index table, holds instances of `currency_stats` objects for each row, which contains information about current supply, maximum supply, and the creator account for a symbol token. The `stats` table is scoped to the token symbol.  Therefore, when one queries the `stats` table for a token symbol the result is one single entry/row corresponding to the queried symbol token if it was previously created, or nothing, otherwise.
   */
  class [[eosio::contract("game.admin")]] admin : public contract
  {
  public:
    using contract::contract;

    string error_building = "you can buy only: ";
    string error_resources = "you can set only: ";
    map<string, string> resource_map = {{"clay", "CLY"}, {"gold", "GLD"}, {"stone", "STN"}, {"wood", "WOD"}};
    map<string, string> buildings_map = {{"clay mine", "CLM"}, {"sawmill", "SWM"}, {"stone mine", "STM"}, {"bakery", "BKR"}};

    [[eosio::action]] void resadd(const name &issuer, const asset &resource_name, const asset &price);

    [[eosio::action]] void reschange(const name &issuer, const asset &resource_name, const asset &price);

    [[eosio::action]] void resdel(const name &issuer, const asset &resource_name);

    [[eosio::action]] void checkbldres(const string &building, const asset &wood, const asset &clay, const asset &stone, const asset &gold);

    [[eosio::action]] void buildingadd(const name &issuer, const string &building, const asset &wood, const asset &clay, const asset &stone, const asset &gold);

    [[eosio::action]] void buildingchng(const name &issuer, const string &building, const asset &wood, const asset &clay, const asset &stone, const asset &gold);

    [[eosio::action]] void buildingdel(const name &issuer, const string &building);

    [[eosio::action]] void createcol(
        const name &author,
        const name &collection_name,
        const bool &allow_notify,
        const vector<name> authorized_accounts,
        const vector<name> notify_accounts,
        const double &market_fee,
        const atomicdata::ATTRIBUTE_MAP data);

    /**
     * @brief This action issues to add `account` into `admin list`
     *
     * @param account - the name to be added to the `admin list`
     */
    [[eosio::action]] void
    adminadd(const name &account);

    /**
     * @brief This action issues to delete `account` from `admin list`
     *
     * @param account - the name to be removed from the admin list`
     */
    [[eosio::action]] void admindel(const name &account);

    /**
     * @brief This payable action issues to give to caller some resources
     *
     * @param caller - the account that created the transfer
     * @param receiver - the account that receives the transfer
     * @param value - information about the transfer, including the amount and currency
     * @param memo - the resource what player wanna get
     */
    [[eosio::on_notify("eosio.token::transfer")]] void
    received(const name &caller, const name &receiver, const asset &value, const string &memo);

    using adminadd_action = eosio::action_wrapper<"adminadd"_n, &eosio::admin::adminadd>;
    using admindel_action = eosio::action_wrapper<"admindel"_n, &eosio::admin::admindel>;

    using resadd_action = eosio::action_wrapper<"resadd"_n, &eosio::admin::resadd>;
    using reschange_action = eosio::action_wrapper<"reschange"_n, &eosio::admin::reschange>;
    using resdel_action = eosio::action_wrapper<"resdel"_n, &eosio::admin::resdel>;

    using buildingadd_action = eosio::action_wrapper<"buildingadd"_n, &eosio::admin::buildingadd>;
    using buildingchng_action = eosio::action_wrapper<"buildingchng"_n, &eosio::admin::buildingchng>;
    using buildingdel_action = eosio::action_wrapper<"buildingdel"_n, &eosio::admin::buildingdel>;

    using createcol_action = eosio::action_wrapper<"createcol"_n, &eosio::admin::createcol>;

  private:
    struct [[eosio::table]] admin_list
    {
      name account;

      uint64_t primary_key() const { return account.value; }
    };

    struct [[eosio::table]] buildres_table
    {
      asset building;
      asset wood;
      asset clay;
      asset stone;
      asset gold;

      uint64_t primary_key() const { return building.symbol.code().raw(); }
    };

    struct [[eosio::table]] resources_table
    {
      asset name;
      asset price;

      uint64_t primary_key() const { return name.symbol.code().raw(); }
    };

    static bool check_map(const map<string, string> &name_map, const string &key_to_find)
    {
      if (name_map.count(key_to_find))
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    static bool check_resource(const map<string, string> &resource_map, const string resourse_name, const symbol &data)
    {
      for (auto item : resource_map)
      {
        if (item.first == resourse_name)
        {
          symbol token_symbol(item.second, 4);
          if (token_symbol.code() == data.code())
          {
            return true;
          }
          else
          {
            return false;
          }
        };
      }
      return false;
    };

    static string concat_map(const string &error, const map<string, string> &name_map)
    {
      string result = error;
      for (auto item : name_map)
      {
        result += item.first + ", ";
      }
      return result.substr(0, result.length() - 2);
    };

    static bool is_admin(const name &get_self, const name &account)
    {
      admin admin_table(get_self, account.value);
      auto existing = admin_table.find(account.value);
      if (existing != admin_table.end())
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    typedef eosio::multi_index<"admin"_n, admin_list> admin;
    typedef eosio::multi_index<"buildings"_n, buildres_table> buildres;
    typedef eosio::multi_index<"resource"_n, resources_table> resources;
  };
}
