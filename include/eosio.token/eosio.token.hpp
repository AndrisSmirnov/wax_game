#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <string>
#include <map>
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
   class [[eosio::contract("eosio.token")]] token : public contract
   {
   public:
      using contract::contract;

      string error_building = "you can buy only: ";
      string error_resources = "you can set only: ";
      map<string, string> resource_map = {{"clay", "CLY"}, {"gold", "GLD"}, {"stone", "STN"}, {"wood", "WOD"}};
      map<string, string> buildings_map = {{"clay mine", "CLM"}, {"sawmill", "SWM"}, {"stone mine", "STM"}, {"bakery", "BKR"}};

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
      [[eosio::action]] void create(const name &issuer,
                                    const asset &maximum_supply);
      /**
       *  This action issues to `to` account a `quantity` of tokens.
       *
       * @param to - the account to issue tokens to, it must be the same as the issuer,
       * @param quntity - the amount of tokens to be issued,
       * @memo - the memo string that accompanies the token issue transaction.
       */
      [[eosio::action]] void issue(const name &to, const asset &quantity, const string &memo);

      /**
       * The opposite for create action, if all validations succeed,
       * it debits the statstable.supply amount.
       *
       * @param quantity - the quantity of tokens to retire,
       * @param memo - the memo string to accompany the transaction.
       */
      [[eosio::action]] void retire(const asset &quantity, const string &memo);

      /**
       * Allows `from` account to transfer to `to` account the `quantity` tokens.
       * One account is debited and the other is credited with quantity tokens.
       *
       * @param from - the account to transfer from,
       * @param to - the account to be transferred to,
       * @param quantity - the quantity of tokens to be transferred,
       * @param memo - the memo string to accompany the transaction.
       */
      [[eosio::action]] void transfer(const name &from,
                                      const name &to,
                                      const asset &quantity,
                                      const string &memo);
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
      [[eosio::action]] void open(const name &owner, const symbol &symbol, const name &ram_payer);

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
      [[eosio::action]] void close(const name &owner, const symbol &symbol);

      //! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      [[eosio::action]] void resadd(const name &issuer, const asset &resource_name, const asset &price);

      [[eosio::action]] void reschange(const name &issuer, const asset &resource_name, const asset &price);

      [[eosio::action]] void resdel(const name &issuer, const asset &resource_name);

      [[eosio::action]] void checkbldres(const string &building, const asset &wood, const asset &clay, const asset &stone, const asset &gold);

      [[eosio::action]] void buildingadd(const name &issuer, const string &building, const asset &wood, const asset &clay, const asset &stone, const asset &gold);

      [[eosio::action]] void buildingchng(const name &issuer, const string &building, const asset &wood, const asset &clay, const asset &stone, const asset &gold);

      [[eosio::action]] void buildingdel(const name &issuer, const string &building);

      /**
       * @brief This action issues to add `account` into `admin list`
       *
       * @param account - the name to be added to the `admin list`
       */
      [[eosio::action]] void adminadd(const name &account);

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

      //! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      static asset get_supply(const name &token_contract_account, const symbol_code &sym_code)
      {
         stats statstable(token_contract_account, sym_code.raw());
         const auto &st = statstable.get(sym_code.raw());
         return st.supply;
      }

      static asset get_balance(const name &token_contract_account, const name &owner, const symbol_code &sym_code)
      {
         accounts accountstable(token_contract_account, owner.value);
         const auto &ac = accountstable.get(sym_code.raw());
         return ac.balance;
      }

      //! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

      //! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      using create_action = eosio::action_wrapper<"create"_n, &token::create>;
      using issue_action = eosio::action_wrapper<"issue"_n, &token::issue>;
      using retire_action = eosio::action_wrapper<"retire"_n, &token::retire>;
      using transfer_action = eosio::action_wrapper<"transfer"_n, &token::transfer>;
      using open_action = eosio::action_wrapper<"open"_n, &token::open>;
      using close_action = eosio::action_wrapper<"close"_n, &token::close>;

      //! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

      using adminadd_action = eosio::action_wrapper<"adminadd"_n, &token::adminadd>;
      using admindel_action = eosio::action_wrapper<"admindel"_n, &token::admindel>;

      using resadd_action = eosio::action_wrapper<"resadd"_n, &token::resadd>;
      using reschange_action = eosio::action_wrapper<"reschange"_n, &token::reschange>;
      using resdel_action = eosio::action_wrapper<"resdel"_n, &token::resdel>;

      using buildingadd_action = eosio::action_wrapper<"buildingadd"_n, &token::buildingadd>;
      using buildingchng_action = eosio::action_wrapper<"buildingchng"_n, &token::buildingchng>;
      using buildingdel_action = eosio::action_wrapper<"buildingdel"_n, &token::buildingdel>;

   private:
      struct [[eosio::table]] admin_list
      {
         name account;

         uint64_t primary_key() const { return account.value; }
      };

      struct [[eosio::table]] account
      {
         asset balance;

         uint64_t primary_key() const { return balance.symbol.code().raw(); }
      };

      struct [[eosio::table]] currency_stats
      {
         asset supply;
         asset max_supply;
         name issuer;

         uint64_t primary_key() const { return supply.symbol.code().raw(); }
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

      typedef eosio::multi_index<"admin"_n, admin_list> admin;
      typedef eosio::multi_index<"accounts"_n, account> accounts;
      typedef eosio::multi_index<"stat"_n, currency_stats> stats;
      typedef eosio::multi_index<"buildings"_n, buildres_table> buildres;
      typedef eosio::multi_index<"resource"_n, resources_table> resources;

      void sub_balance(const name &owner, const asset &value);
      void add_balance(const name &owner, const asset &value, const name &ram_payer);
   };
}
