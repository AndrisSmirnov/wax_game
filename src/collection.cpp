#include <my_game.hpp>

void my_collection::createCollection(collection_name name, author name, allow_notify bool)
{
  require_auth(get_self());

    collections.emplace(author, [&](auto &_collection) {
    _collection.collection_name = collection_name;
    _collection.author = author;
    _collection.allow_notify = allow_notify;
    _collection.authorized_accounts = authorized_accounts;
    _collection.notify_accounts = notify_accounts;
    _collection.market_fee = market_fee;
    }

        action{
        permission_level {
          get_self(),
          "active"_n},
          PUBLISHER_TOKEN,
          "transfer"_n,
          make_tuple(get_self(), COMMISSION_RECIPIENT, asset(((amount.amount * WITHDRAWAL_FEE) / 100), amount.symbol), string("Fee " + token_names[resource] + " Token"))
        }.send();
}
