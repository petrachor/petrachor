//
// Created by ruell on 26/08/2021.
//

#include "TxPool.h"
#include "JsonCustomWriter.h"
#include "libethereum/Transaction.h"
#include "JsonHelper.h"

using namespace dev::rpc;
using namespace dev::eth;

TxPool::TxPool(eth::Interface& _eth)
	: m_eth(_eth)
{
	//...
}

Json::Value TxPool::txpool_content()
{
	//Return list of transactions from transactions pool
	Transactions ours;

	Json::Value finalJson;
	Json::Value pendingAccounts;

	// 1. collect all the accounts
	std::map<Address, Transactions> accountTransactions;
	for (const auto& pending :client()->pending())
	{
		auto iterM = accountTransactions.find(pending.sender());
		if( iterM == accountTransactions.end())
		{
			accountTransactions[pending.sender()] = {};
		}

		accountTransactions[pending.sender()].push_back(pending);
	}

	// 2. iterate thru the resulting map and form the content using nonce
	for (const auto& at : accountTransactions)
	{
		// 2.1 loop thru account transactions
		Json::Value detail;
		for (const auto& transactions : at.second)
		{
			auto nonce = transactions.nonce().convert_to<int64_t>();
			std::string nonceKey = std::to_string(nonce);
			detail[nonceKey] = toJson(transactions);
		}

		pendingAccounts["0x" + at.first.hex()] = detail;
	}

	Json::Value queuedJson;
	Json::Value queuedAccountsJson;

	// 3. Get the queued transactions
	for (const auto& queued:client()->queued())
	{
		Json::Value detail;
		for (const auto& transactions : queued.second)
		{
			auto nonce = transactions.second.transaction.nonce().convert_to<int64_t>();
			std::string nonceKey = std::to_string(nonce);
			detail[nonceKey] = toJson(transactions.second.transaction);
		}

		queuedAccountsJson["0x" + queued.first.hex()] = detail;
	}

	finalJson["pending"] = pendingAccounts;
	finalJson["queued"] = queuedAccountsJson;

	//Json::CustomStreamWriterBuilder wbuilder;
	//wbuilder["indentation"] = "";

	//return Json::writeString(wbuilder, finalJson);;
	return finalJson;
}