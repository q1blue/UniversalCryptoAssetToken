#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <math.h>

#define max(a,b) (a>b?a:b)


namespace eosiosystem {
   class system_contract;
}

using namespace eosio;

#define SYSPARAM_TOKEN_COUNT		1
#define SYSPARAM_CONTRACT_ENABLE	2

#define SYSPARAM_ADMIN_ACCOUNT		4
#define API_URL				5
#define CONTRACT_NAME			6
#define CONTRACT_LOGO			7
#define MIGRATE_COUNT			8


#define HTML_TEMPLATE			10
#define SYSPARAM_VERSION		11


#define LOOT_SYMBOL symbol("LOOT", 4)
#define TIME_SYMBOL symbol("TIME", 8)

#define ASSET_USDT_SYMBOL S(4, USDT)
#define USDT_ACCOUNT name("tethertether")

#define ASSET_TIME_SYMBOL S(8, TIME)
#define TIME_ACCOUNT name("xpettimetest")

#define ASSET_LOOT_SYMBOL S(4, LOOT)
#define LOOT_ACCOUNT name("lootglobcore")


class [[eosio::contract("eosnft")]] eosnft : public contract
{   

public:

   using contract::contract;

   TABLE sysparam{
	uint64_t	id;
	std::string	tag;
	std::string	val;

	uint64_t primary_key() const { return id; }
   };
   typedef multi_index<"sysparams"_n, sysparam> sysparam_index;
    
 
    TABLE accounts {
        name id;

        uint64_t count;

        uint64_t primary_key() const { return id.value; }
    };
    typedef multi_index<"accounts"_n, accounts> account_index;


    TABLE token {
        uint64_t id;
	std::string uuid;
        std::string title;
	name owner;
	std::string imageUrl;
	std::string category;
	std::string ext;
	std::string meta;

	bool	   lock;
	asset	   stackasset;
	uint64_t   level;

        uint64_t primary_key() const { return id; }
	uint64_t get_secondary_1() const { return owner.value;}
    };
   typedef multi_index<"tokens"_n, token> token_index;


    TABLE logs {
        uint64_t id;
	name from;
	name to;
	std::string memo;
	uint64_t acttime;
	uint64_t tokenId;
        uint64_t primary_key() const { return id; }
    };
    typedef multi_index<"logs"_n, logs> log_index;

    TABLE blacklist{
        name	id;

        uint64_t primary_key() const { return id.value; }
    };
    typedef multi_index<"blacklists"_n, blacklist> blacklist_index;

    TABLE notifylist{
	name acc;

	uint64_t primary_key() const { return acc.value; }
    };
    typedef multi_index<"notifylists"_n, notifylist> notifylist_index;

 private:

     double stringtodouble(std::string str)
     {
	 double dTmp = 0.0;
	 int iLen = str.length();
	 int iPos = str.find(".");
	 std::string strIntege = str.substr(0,iPos);
	 std::string strDecimal = str.substr(iPos + 1,iLen - iPos - 1 );
	 for (int i = 0; i < iPos;i++)
	 {
	  if (strIntege[i] >= '0' && strIntege[i] <= '9')
	  {
	   dTmp = dTmp * 10 + strIntege[i] - '0';
	  }
	 }
	 for (int j = 0; j < strDecimal.length(); j++)
	 {
	  if (strDecimal[j] >= '0' && strDecimal[j] <= '9')
	  {
	   dTmp += (strDecimal[j] - '0') * pow(10.0,(0 - j - 1));
	  }
	 }
	 return dTmp;
     }


     name get_admin() const {
         const std::string adminAccount = getsysparam(SYSPARAM_ADMIN_ACCOUNT);
         if (adminAccount.empty()) {
             return _self;
         }
         else {
             return name(adminAccount.c_str());
         }
     }

     void require_auth_admin() const { require_auth(get_admin()); }

     void require_auth_contract() const { require_auth( _self );}
     
     inline std::string getsysparam(const uint64_t& key) const {
	sysparam_index sysparams(_self, _self.value);
	auto iter = sysparams.find(key);
	if(iter == sysparams.end()){
	        return std::string("");
	}else{
	        return iter->val;
	}
    }

    
    inline void setsysparam(const uint64_t& id, const std::string& tag, const std::string& val){
	    sysparam_index sysparams(_self, _self.value);
	    auto iter = sysparams.find(id);
	    if(iter == sysparams.end()){
		    sysparams.emplace(_self, [&](auto& p) {
		        p.id  = id;
		        p.val = val;
			p.tag = tag;
		    });
	    }else{
		    sysparams.modify(iter, _self, [&](auto& p) {
			    p.val = val;
			    p.tag = tag;
		    });
	    }
    }

    inline void addaccounttoken(const name user) {
	account_index accounts(_self, _self.value);
        auto iter = accounts.find(user.value);
        if (iter == accounts.end()) {
            accounts.emplace(_self, [&](auto& p) {
                p.id = user;
                p.count = 1;
                });
        }
        else {
            accounts.modify(iter, _self, [&](auto& p) {
                p.count += 1;
            });
        }
    }

    inline void subaccounttoken(const name user) {
	account_index accounts(_self, _self.value);
        auto iter = accounts.find(user.value);
        if (iter == accounts.end()) {
            return ;
        }
        
	if(iter->count > 1){
            accounts.modify(iter, _self, [&](auto& p) {
                if (p.count > 1){
                    p.count -= 1;
                }
           });
        }else{
	   accounts.erase(iter);
	}
    }
    
    inline uint64_t toInt(const std::string& str) {
        if (str.empty() || str == "") {
            return 0;
        }
        else {
            std::string::size_type sz = 0;
            return std::stoull(str, &sz, 0);
        }
    }
    
    void log(const uint64_t& id, const name& oldowner, const name& newowner, const std::string& opcode, const std::string& ext);

    
    inline uint64_t gettokencount(){
	return toInt(getsysparam(SYSPARAM_TOKEN_COUNT));
    }


    inline void addtokencount(){
	setsysparam(SYSPARAM_TOKEN_COUNT, "SYSPARAM_TOKEN_COUNT", std::to_string(gettokencount() + 1));
    }

    inline void subtokencount(){
        uint64_t tokencount = gettokencount();
	if(tokencount > 0){
	    setsysparam(SYSPARAM_TOKEN_COUNT, "SYSPARAM_TOKEN_COUNT", std::to_string(tokencount - 1));
	}
    }

    inline void blackcheck(const name acc){
	blacklist_index blacklists(_self, _self.value);
	auto iter = blacklists.find(acc.value);
	check(iter == blacklists.end(), "acc is in black");
    }


 public:

     [[eosio::action]]
    void init(const std::string adminacc, const std::string apiUrl, const std::string title, const std::string image){
	require_auth_contract();
	setsysparam(SYSPARAM_ADMIN_ACCOUNT, "SYSPARAM_ADMIN_ACCOUNT", adminacc);
	setsysparam(API_URL, "API_URL", apiUrl);
	setsysparam(CONTRACT_NAME, "CONTRACT_NAME", title);	
	setsysparam(CONTRACT_LOGO, "CONTRACT_LOGO", image);	
    }
 
    [[eosio::action]]
    void assign(const uint64_t id, const name newowner);

    [[eosio::action]]
    void reassign(const uint64_t id, const name newowner);
    
    [[eosio::action]]
    void create(const uint64_t id, const std::string uuid, const std::string category, const std::string title, const std::string imageUrl, const std::string meta, const bool lock, const std::string ext);

    [[eosio::action]]
    void updatemeta(const uint64_t id, const std::string title, const std::string category, const std::string imageUrl, const std::string meta);

    [[eosio::action]]
    void updatelock(const uint64_t id, const bool lock);

    [[eosio::action]]
    void updateext(const uint64_t id, const std::string ext);

    [[eosio::action]]
    void transfer(const uint64_t id, const name newowner, const std::string memo);

    [[eosio::action]]
    void transmk(const uint64_t id, const name newowner, const std::string memo);

    [[eosio::action]]
    void setparam(const uint64_t id, const std::string tag, const std::string val){
	    require_auth_contract();
	    setsysparam(id,tag,val);
    }

    [[eosio::action]]
    void burn(const uint64_t id){
	token_index tokens(_self, _self.value);

	auto iter = tokens.find(id);
	check(iter != tokens.end(), "token not found");

	require_auth( iter->owner);

	rmtoken_(id);
    }

    [[eosio::action]]
    void addblack(const name acc){
	require_auth_contract();
	blacklist_index blacklists(_self, _self.value);
	blacklists.emplace(_self, [&](auto& p) {
		p.id = acc;
	});
    }

    [[eosio::action]]
    void rmblack(const name acc){
	require_auth_contract();
	blacklist_index blacklists(_self, _self.value);
	blacklists.erase(blacklists.find(acc.value));
    }
    //---- for debug 
    
    [[eosio::action]]
    void rmtoken(const uint64_t id){
	require_auth_contract();
	rmtoken_(id);
    }

    [[eosio::action]]
    void rmaccount(const name acc){
	require_auth_contract();
	account_index accounts(_self, _self.value);
	accounts.erase(accounts.find(acc.value));
    }

    [[eosio::action]]
    void rmparam(const uint64_t id){
	require_auth_contract();
	sysparam_index sysparams(_self, _self.value);
	sysparams.erase(sysparams.find(id));
    }

private:
	void rmtoken_(const uint64_t id){	
		token_index tokens(_self, _self.value);
		auto iter = tokens.find(id);
		check(iter != tokens.end(), "token not found");

		if(iter->stackasset.amount > 0){
			transfer(_self, iter->owner, iter->stackasset, "burn nft return asset");
		}

		subtokencount();
		subaccounttoken(iter->owner);

		log(id, _self, _self, "BURN", "");
	}

    inline void transfer(const name from,
        const name to,
        const eosio::asset& amount,
        const std::string& memo)
    {
        if (from == to)
            return;
	if (amount.symbol == TIME_SYMBOL)
		action(permission_level{_self, "active"_n}, TIME_ACCOUNT, "transfer"_n, std::make_tuple(from, to, amount, memo)).send();
	else if (amount.symbol == LOOT_SYMBOL)
		action(permission_level{_self, "active"_n}, LOOT_ACCOUNT, "transfer"_n, std::make_tuple(from, to, amount, memo)).send();
        else
            check(false, "only LOOT/TIME supported.");
    }

	void notifyall(){
		notifylist_index notifys(_self, _self.value);
		auto iter = notifys.begin();
		while(iter != notifys.end()){
			require_recipient(iter->acc);
			iter ++;
		}
	}

	void clearlog();

};
