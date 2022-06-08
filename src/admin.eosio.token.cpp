#include <eosio.token.hpp>
#include <eosio.token.hpp>
#include <atomicdata.hpp>
#include <SimpleAssets.hpp>
#include <map>

namespace eosio
{
   using std::map;

   //! ~~~ admin part ~~~
   void token::adminadd(const name &account)
   {
      require_auth(get_self());

      admin admin_table(get_self(), account.value);
      auto existing = admin_table.find(account.value);
      check(existing == admin_table.end(), "row with that name already exists");

      admin_table.emplace(get_self(), [&](auto &row)
                          { row.account = account; });
   }

   void token::admindel(const name &account)
   {
      require_auth(get_self());
      admin admin_table(get_self(), account.value);
      auto itr = admin_table.begin();
      while (itr != admin_table.end())
      {
         itr = admin_table.erase(itr);
      }
   }

   //! ~~~ resource part ~~~

   void token::resadd(const name &issuer, const asset &name, const asset &price)
   {
      check(is_admin(get_self(), issuer), "access denied");
      require_auth(issuer);

      auto res_name = name.symbol;
      resources resources_t(get_self(), res_name.code().raw());
      auto existing = resources_t.find(res_name.code().raw());
      check(existing == resources_t.end(), "resource with name already exists");

      resources_t.emplace(get_self(), [&](auto &row)
                          {
         row.name = name;
         row.price = price; });
   }

   void token::reschange(const name &issuer, const asset &name, const asset &price)
   {
      check(is_admin(get_self(), issuer), "access denied");
      require_auth(issuer);

      auto res_name = name.symbol;
      resources resources_t(get_self(), res_name.code().raw());
      auto existing = resources_t.find(res_name.code().raw());
      const auto &st = *existing;

      resources_t.modify(st, same_payer, [&](auto &row)
                         { row.price = price; });
   }

   void token::resdel(const name &issuer, const asset &name)
   {
      check(is_admin(get_self(), issuer), "access denied");

      require_auth(issuer);

      auto res_name = name.symbol;
      resources resources_t(get_self(), res_name.code().raw());
      auto itr = resources_t.begin();
      while (itr != resources_t.end())
      {
         itr = resources_t.erase(itr);
      }
   }

   //! ~~~ building part ~~~

   void token::checkbldres(const string &building, const asset &wood, const asset &clay, const asset &stone, const asset &gold)
   {
      check(check_map(token::buildings_map, building), concat_map(token::error_building, token::buildings_map));
      check(check_resource(token::resource_map, "wood", wood.symbol), concat_map(token::error_resources, token::resource_map));
      check(check_resource(token::resource_map, "clay", clay.symbol), concat_map(token::error_resources, token::resource_map));
      check(check_resource(token::resource_map, "stone", stone.symbol), concat_map(token::error_resources, token::resource_map));
      check(check_resource(token::resource_map, "gold", gold.symbol), concat_map(token::error_resources, token::resource_map));
   }

   void token::buildingadd(const name &issuer, const string &building, const asset &wood, const asset &clay, const asset &stone, const asset &gold)
   {
      check(is_admin(get_self(), issuer), "access denied");
      require_auth(issuer);
      checkbldres(building, wood, clay, stone, gold);
      symbol building_symbol(token::buildings_map[building], 4);

      buildres buildings_table(get_self(), building_symbol.code().raw());
      auto existing = buildings_table.find(building_symbol.code().raw());
      check(existing == buildings_table.end(), "building with name already exists");

      asset new_building;
      new_building.symbol = building_symbol;
      new_building.amount = 1e4;

      buildings_table.emplace(get_self(), [&](auto &row)
                              {
         row.building = new_building;
         row.wood = wood;
         row.clay = clay;
         row.stone = stone;
         row.gold = gold; });
   }

   void token::buildingchng(const name &issuer, const string &building, const asset &wood, const asset &clay, const asset &stone, const asset &gold)
   {
      check(is_admin(get_self(), issuer), "access denied");
      require_auth(issuer);
      checkbldres(building, wood, clay, stone, gold);
      symbol building_symbol(token::buildings_map[building], 4);

      buildres buildings_table(get_self(), building_symbol.code().raw());
      auto existing = buildings_table.find(building_symbol.code().raw());
      check(existing != buildings_table.end(), "building with name don't exists");

      asset new_building;
      new_building.symbol = building_symbol;
      new_building.amount = 1e4;

      buildings_table.modify(existing, same_payer, [&](auto &row)
                             {
         row.building = new_building;
         row.wood = wood;
         row.clay = clay;
         row.stone = stone;
         row.gold = gold; });
   }

   void token::buildingdel(const name &issuer, const string &building)
   {
      check(is_admin(get_self(), issuer), "access denied");
      require_auth(issuer);
      check(check_map(token::buildings_map, building), concat_map(token::error_building, token::buildings_map));

      symbol building_symbol(token::buildings_map[building], 4);
      buildres buildings_table(get_self(), building_symbol.code().raw());
      auto it = buildings_table.find(building_symbol.code().raw());
      check(it != buildings_table.end(), "building with name don't exists");
      buildings_table.erase(it);
   }

   //! ~~~ NFT part ~~~

   void token::createcol(
       const name &author,
       const name &collection_name,
       const bool &allow_notify,
       const vector<name> authorized_accounts,
       const vector<name> notify_accounts,
       const double &market_fee,
       const atomicdata::ATTRIBUTE_MAP data)
   {
      check(is_admin(get_self(), author), "access denied");
      // require_auth(author);

      //!!!!!!!!!!!!!!!!!!!!!!
      atomicdata::ATTRIBUTE_MAP atmp = {};

      eosio::action create_collection = eosio::action(
          permission_level{get_self(), "active"_n},
          NFT_PUBLISHER,
          //  "atomicassets"_n,
          "createcol"_n,
          //  make_tuple("accgametown1", "accgametown1", 1, atmp, atmp, MAX_MARKET_FEE, atmp));
          make_tuple(author, collection_name, allow_notify, authorized_accounts, notify_accounts, market_fee, data));
      create_collection.send();

      //!!!!!!!!!!!!!!!!!!!!!!
   }

   // void game::craftnft(const name username, const name nft_name)
   // {
   //    require_auth(username);
   //    // BLOCK CONTRACT
   //    auto stat = _wrkcont.find(0);
   //    check(stat->stp_ctr, "Smart contract on pause.");
   //    // END BLOCK CONTRACT

   //    const auto user_iterator = _accounts.require_find(username.value, "Error: User not found!");
   //    auto craft = _confcraft.find(nft_name.value);

   //    check(craft != _confcraft.end(), "Error: NFT Not found!");

   //    for (int i = 0; i < craft->cost_craft.size(); i++)
   //    {
   //       check(craft->cost_craft[i].amount <= user_iterator->balance[i].amount, "Error: You don't have enough balance!");
   //    }

   //    const auto card_info = atomicassets::assets_t(NFT_PUBLISHER, get_self().value);

   //    ATTRIBUTE_MAP atmp = {};
   //    action{
   //        permission_level{
   //            get_self(),
   //            "active"_n},
   //        NFT_PUBLISHER,
   //        "mintasset"_n,
   //        make_tuple(get_self(), NFT_COLLECTION_NAME, "buildings"_n, craft->template_id, username, atmp, atmp, asset(0, symbol("WAX", 4)))}
   //        .send();

   //    _accounts.modify(user_iterator, username, [&](auto &modified_user)
   //                     {
   //    for (uint8_t i = 0; i < user_iterator->balance.size(); i++) {
   //      modified_user.balance[i].amount -= craft->cost_craft[i].amount;
   //    } });
   // }
}
