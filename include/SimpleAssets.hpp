#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>

#include <checkformat.hpp>
#include <atomicdata.hpp>

using namespace eosio;
using namespace std;
using namespace atomicdata;

static constexpr double MAX_MARKET_FEE = 0.15;

CONTRACT atomicassets : public contract
{
public:
    using contract::contract;

    ACTION init();

    ACTION admincoledit(vector<FORMAT> collection_format_extension);

    ACTION setversion(string new_version);

    ACTION addconftoken(name token_contract, symbol token_symbol);

    ACTION transfer(
        name from,
        name to,
        vector<uint64_t> asset_ids,
        string memo);

    TABLE collections_s
    {
        name collection_name;
        name author;
        bool allow_notify;
        vector<name> authorized_accounts;
        vector<name> notify_accounts;
        double market_fee;
        vector<uint8_t> serialized_data;

        uint64_t primary_key() const { return collection_name.value; };
    };

    typedef multi_index<name("collections"), collections_s> collections_t;

    // Scope: collection_name
    TABLE schemas_s
    {
        name schema_name;
        vector<FORMAT> format;

        uint64_t primary_key() const { return schema_name.value; }
    };

    typedef multi_index<name("schemas"), schemas_s> schemas_t;

    // Scope: collection_name
    TABLE templates_s
    {
        int32_t template_id;
        name schema_name;
        bool transferable;
        bool burnable;
        uint32_t max_supply;
        uint32_t issued_supply;
        vector<uint8_t> immutable_serialized_data;

        uint64_t primary_key() const { return (uint64_t)template_id; }
    };

    typedef multi_index<name("templates"), templates_s> templates_t;

    // Scope: owner
    TABLE assets_s
    {
        uint64_t asset_id;
        name collection_name;
        name schema_name;
        int32_t template_id;
        name ram_payer;
        vector<asset> backed_tokens;
        vector<uint8_t> immutable_serialized_data;
        vector<uint8_t> mutable_serialized_data;

        uint64_t primary_key() const { return asset_id; };
    };

    typedef multi_index<name("assets"), assets_s> assets_t;

    TABLE offers_s
    {
        uint64_t offer_id;
        name sender;
        name recipient;
        vector<uint64_t> sender_asset_ids;
        vector<uint64_t> recipient_asset_ids;
        string memo;
        name ram_payer;

        uint64_t primary_key() const { return offer_id; };

        uint64_t by_sender() const { return sender.value; };

        uint64_t by_recipient() const { return recipient.value; };
    };

    typedef multi_index<name("offers"), offers_s,
                        indexed_by<name("sender"), const_mem_fun<offers_s, uint64_t, &offers_s::by_sender>>,
                        indexed_by<name("recipient"), const_mem_fun<offers_s, uint64_t, &offers_s::by_recipient>>>
        offers_t;

    TABLE balances_s
    {
        name owner;
        vector<asset> quantities;

        uint64_t primary_key() const { return owner.value; };
    };

    typedef multi_index<name("balances"), balances_s> balances_t;

    TABLE config_s
    {
        uint64_t asset_counter = 1099511627776; // 2^40
        int32_t template_counter = 1;
        uint64_t offer_counter = 1;
        vector<FORMAT> collection_format = {};
        vector<extended_symbol> supported_tokens = {};
    };
    typedef singleton<name("config"), config_s> config_t;
    // https://github.com/EOSIO/eosio.cdt/issues/280
    typedef multi_index<name("config"), config_s> config_t_for_abi;

    TABLE tokenconfigs_s
    {
        name standard = name("atomicassets");
        std::string version = string("1.3.1");
    };
    typedef singleton<name("tokenconfigs"), tokenconfigs_s> tokenconfigs_t;
    // https://github.com/EOSIO/eosio.cdt/issues/280
    typedef multi_index<name("tokenconfigs"), tokenconfigs_s> tokenconfigs_t_for_abi;

    collections_t collections = collections_t(get_self(), get_self().value);
    offers_t offers = offers_t(get_self(), get_self().value);
    balances_t balances = balances_t(get_self(), get_self().value);
    config_t config = config_t(get_self(), get_self().value);
    tokenconfigs_t tokenconfigs = tokenconfigs_t(get_self(), get_self().value);

    void internal_transfer(
        name from,
        name to,
        vector<uint64_t> asset_ids,
        string memo,
        name scope_payer);

    void internal_back_asset(
        name ram_payer,
        name asset_owner,
        uint64_t asset_id,
        asset back_quantity);

    void internal_decrease_balance(
        name owner,
        asset quantity);

    void notify_collection_accounts(
        name collection_name);

    void check_has_collection_auth(
        name account_to_check,
        name collection_name,
        string error_message);

    void check_name_length(ATTRIBUTE_MAP data);

    assets_t get_assets(name acc);

    schemas_t get_schemas(name collection_name);

    templates_t get_templates(name collection_name);
};