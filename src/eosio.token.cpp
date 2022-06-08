#include <eosio.token.hpp>
#include <atomicdata.hpp>
#include "admin.eosio.token.cpp"
#include <map>

namespace eosio
{
   using std::map;

   void token::received(const name &caller, const name &receiver, const asset &value, const string &memo)
   {
      if (receiver != get_self() || caller == get_self())
      {
         return;
      }

      symbol token_symbol("WAX", 4);
      int multiplication = 100;

      //! check type and amount
      check(check_map(token::resource_map, memo), concat_map(token::error_resources, token::resource_map));
      check(value.amount > 100, "expects transfer bigger than 0.000001 WAX");
      check(value.symbol != token_symbol, "Illegal asset symbol");

      //! get price of resource name
      symbol resourse_symbol(resource_map[memo], 4);
      auto res_name = resourse_symbol;
      resources resources_table(get_self(), res_name.code().raw());
      auto existing_res = resources_table.find(res_name.code().raw());
      check(existing_res != resources_table.end(), "resource with name don't exists");

      //! mathematical conversion from received value to  resource price using help resource table
      asset need_to_send = value;
      need_to_send.symbol = resourse_symbol;
      need_to_send.amount = need_to_send.amount * (existing_res->name.amount * multiplication / existing_res->price.amount);
      need_to_send.amount = (need_to_send.amount / multiplication) / 10e3;

      stats stats_table_create(get_self(), need_to_send.symbol.code().raw());
      auto existing_create = stats_table_create.find(need_to_send.symbol.code().raw());

      //! write to stats table if symbol don't exist
      if (existing_create == stats_table_create.end())
      {
         stats_table_create.emplace(get_self(), [&](auto &row)
                                    {
       row.supply.symbol = need_to_send.symbol;
       row.max_supply = need_to_send;
       row.issuer = receiver; });
      }

      //! issue
      stats stats_table_issue(get_self(), need_to_send.symbol.code().raw());
      auto existing_issue = stats_table_issue.find(need_to_send.symbol.code().raw());
      check(existing_issue != stats_table_issue.end(), "token with symbol does not exist, create token before issue");
      const auto &st_issue = *existing_issue;

      if (need_to_send.amount > st_issue.max_supply.amount - st_issue.supply.amount)
      {
         stats_table_issue.modify(st_issue, receiver, [&](auto &row)
                                  { row.max_supply.amount += need_to_send.amount - st_issue.supply.amount; });
      }

      stats_table_issue.modify(st_issue, receiver, [&](auto &row)
                               { row.supply += need_to_send; });

      add_balance(st_issue.issuer, need_to_send, receiver);

      //! transfer
      check(receiver != caller, "cannot transfer to self");
      check(is_account(caller), "to account does not exist");

      require_recipient(caller);
      require_recipient(receiver);

      sub_balance(receiver, need_to_send);
      add_balance(caller, need_to_send, receiver);
   }

   void token::create(const name &issuer,
                      const asset &maximum_supply)
   {
      require_auth(get_self());

      auto sym = maximum_supply.symbol;
      check(sym.is_valid(), "invalid symbol name");
      check(maximum_supply.is_valid(), "invalid supply");
      check(maximum_supply.amount > 0, "max-supply must be positive");

      stats statstable(get_self(), sym.code().raw());
      auto existing = statstable.find(sym.code().raw());
      check(existing == statstable.end(), "token with symbol already exists");

      statstable.emplace(get_self(), [&](auto &s)
                         {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer; });
   }

   void token::issue(const name &to, const asset &quantity, const string &memo)
   {
      auto sym = quantity.symbol;
      check(sym.is_valid(), "invalid symbol name");
      check(memo.size() <= 256, "memo has more than 256 bytes");

      stats statstable(get_self(), sym.code().raw());
      auto existing = statstable.find(sym.code().raw());
      check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
      const auto &st = *existing;
      check(to == st.issuer, "tokens can only be issued to issuer account");

      require_auth(st.issuer);
      check(quantity.is_valid(), "invalid quantity");
      check(quantity.amount > 0, "must issue positive quantity");

      check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
      check(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

      statstable.modify(st, same_payer, [&](auto &s)
                        { s.supply += quantity; });

      add_balance(st.issuer, quantity, st.issuer);
   }

   void token::retire(const asset &quantity, const string &memo)
   {
      auto sym = quantity.symbol;
      check(sym.is_valid(), "invalid symbol name");
      check(memo.size() <= 256, "memo has more than 256 bytes");

      stats statstable(get_self(), sym.code().raw());
      auto existing = statstable.find(sym.code().raw());
      check(existing != statstable.end(), "token with symbol does not exist");
      const auto &st = *existing;

      require_auth(st.issuer);
      check(quantity.is_valid(), "invalid quantity");
      check(quantity.amount > 0, "must retire positive quantity");

      check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

      statstable.modify(st, same_payer, [&](auto &s)
                        { s.supply -= quantity; });

      sub_balance(st.issuer, quantity);
   }

   void token::transfer(const name &from,
                        const name &to,
                        const asset &quantity,
                        const string &memo)
   {
      check(from != to, "cannot transfer to self");
      require_auth(from);
      check(is_account(to), "to account does not exist");
      auto sym = quantity.symbol.code();
      stats statstable(get_self(), sym.raw());
      const auto &st = statstable.get(sym.raw());

      require_recipient(from);
      require_recipient(to);

      check(quantity.is_valid(), "invalid quantity");
      check(quantity.amount > 0, "must transfer positive quantity");
      check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
      check(memo.size() <= 256, "memo has more than 256 bytes");

      auto payer = has_auth(to) ? to : from;

      sub_balance(from, quantity);
      add_balance(to, quantity, payer);
   }

   void token::sub_balance(const name &owner, const asset &value)
   {
      accounts from_acnts(get_self(), owner.value);

      const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
      check(from.balance.amount >= value.amount, "overdrawn balance");

      from_acnts.modify(from, owner, [&](auto &a)
                        { a.balance -= value; });
   }

   void token::add_balance(const name &owner, const asset &value, const name &ram_payer)
   {
      accounts to_acnts(get_self(), owner.value);
      auto to = to_acnts.find(value.symbol.code().raw());
      if (to == to_acnts.end())
      {
         to_acnts.emplace(ram_payer, [&](auto &a)
                          { a.balance = value; });
      }
      else
      {
         to_acnts.modify(to, same_payer, [&](auto &a)
                         { a.balance += value; });
      }
   }

   void token::open(const name &owner, const symbol &symbol, const name &ram_payer)
   {
      require_auth(ram_payer);

      check(is_account(owner), "owner account does not exist");

      auto sym_code_raw = symbol.code().raw();
      stats statstable(get_self(), sym_code_raw);
      const auto &st = statstable.get(sym_code_raw, "symbol does not exist");
      check(st.supply.symbol == symbol, "symbol precision mismatch");

      accounts acnts(get_self(), owner.value);
      auto it = acnts.find(sym_code_raw);
      if (it == acnts.end())
      {
         acnts.emplace(ram_payer, [&](auto &a)
                       { a.balance = asset{0, symbol}; });
      }
   }

   void token::close(const name &owner, const symbol &symbol)
   {
      require_auth(owner);
      accounts acnts(get_self(), owner.value);
      auto it = acnts.find(symbol.code().raw());
      check(it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect.");
      check(it->balance.amount == 0, "Cannot close because the balance is not zero.");
      acnts.erase(it);
   }

}