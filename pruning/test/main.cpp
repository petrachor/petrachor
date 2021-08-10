#include <libp2p/Host.h>
#include <memory>
#include <iostream>
#include <thread>

#include <libdevcore/OverlayDB.h>
#include <libethereum/State.h>
#include <libethereum/Account.h>

#include "../Pruner.h"

using namespace dev;
using namespace std;

int main()
{
	ldb::Options o;
	o.max_open_files = 256;
	o.create_if_missing = true;
	ldb::DB* db = nullptr;
	std::string currentPath = boost::filesystem::current_path().generic_string();

	ldb::Status status = ldb::DB::Open(o, currentPath + "/data/", &db);

	OverlayDB m_db(db);								///< Our overlay for the state tree.
	eth::SecureTrieDB<Address, OverlayDB> m_state(&m_db);	///< Our state tree, as an OverlayDB DB.

	eth::AccountMap _cache;
	eth::Account account;

	account.setNonce(0);
	account.addBalance(100);

	auto k = KeyPair<AccountKeys::KeyType::Type>::create(true);
	std::cout << "Address " << k.address().hex() << std::endl;

	_cache[k.address()] = account;

	RLPStream s(4);
	s << account.nonce() << account.balance();
	s.append(account.baseRoot());
	s << account.codeHash();

	m_state.init();
	m_state.insert(k.address(), &s.out());

	account.addBalance(200);

	RLPStream s200(4);
	s200 << account.nonce() << account.balance();
	s200.append(account.baseRoot());
	s200 << account.codeHash();

	m_state.insert(k.address(), &s200.out());

	pruning::init(currentPath + "/data", h256(), 1);

	// perform pruning before commit
	// insert two in prune db, no delete will happen
	pruning::prune(0, &m_db);

	// commit the cache but cache is cleared after, so comment out if needed only
    m_db.commit();

    account.addBalance(200);

    RLPStream s400(4);
    s400 << account.nonce() << account.balance();
    s400.append(account.baseRoot());
    s400 << account.codeHash();

    m_state.insert(k.address(), &s400.out());

    // perform pruning before commit
    // insert 1 in db, no prune delete will happen as the two inserted above is not in state db
    pruning::prune(1, &m_db);

    // commit the cache but cache is cleared after, so comment out if needed only
    m_db.commit();

    // prune the record inserted at block 1, delete 1 record
    pruning::prune(2, &m_db);
}
