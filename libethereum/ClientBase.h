/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file ClientBase.h
 * @author Gav Wood <i@gavwood.com>
 * @author Marek Kotewicz <marek@ethdev.com>
 * @date 2015
 */

#pragma once

#include <chrono>
#include "Interface.h"
#include "LogFilter.h"
#include "TransactionQueue.h"
#include "Block.h"
#include "CommonNet.h"

namespace dev
{

namespace eth
{

struct InstalledFilter
{
	InstalledFilter(LogFilter const& _f): filter(_f) {}

	LogFilter filter;
	unsigned refCount = 1;
	LocalisedLogEntries changes;
};

static const h256 PendingChangedFilter = u256(0);
static const h256 ChainChangedFilter = u256(1);

static const LogEntry SpecialLogEntry = LogEntry(Address(), h256s(), bytes());
static const LocalisedLogEntry InitialChange(SpecialLogEntry);

struct ClientWatch
{
	ClientWatch(): lastPoll(std::chrono::system_clock::now()) {}
	explicit ClientWatch(h256 _id, Reaping _r): id(_id), lastPoll(_r == Reaping::Automatic ? std::chrono::system_clock::now() : std::chrono::system_clock::time_point::max()) {}

	h256 id;
#if INITIAL_STATE_AS_CHANGES
	LocalisedLogEntries changes = LocalisedLogEntries{ InitialChange };
#else
	LocalisedLogEntries changes;
#endif
	mutable std::chrono::system_clock::time_point lastPoll = std::chrono::system_clock::now();
};

struct WatchChannel: public LogChannel { static const char* name(); static const int verbosity = 7; };
#define cwatch LogOutputStream<WatchChannel, true>()
struct WorkInChannel: public LogChannel { static const char* name(); static const int verbosity = 16; };
struct WorkOutChannel: public LogChannel { static const char* name(); static const int verbosity = 16; };
struct WorkChannel: public LogChannel { static const char* name(); static const int verbosity = 21; };
#define cwork LogOutputStream<WorkChannel, true>()
#define cworkin LogOutputStream<WorkInChannel, true>()
#define cworkout LogOutputStream<WorkOutChannel, true>()

class ClientBase: public Interface
{
public:
	ClientBase(TransactionQueue::Limits const& _l = TransactionQueue::Limits{1024, 1024}): m_tq(_l) {}
	virtual ~ClientBase() {}

	/// Submits the given transaction.
	/// @returns the new transaction's hash.
	virtual std::pair<h256, Address> submitTransaction(TransactionSkeleton const& _t, AccountKeys::Secret const& _secret) override;
	using Interface::submitTransaction;

	/// Makes the given call. Nothing is recorded into the state.
	virtual ExecutionResult call(Address const& _secret, u256 _value, Address _dest, bytes const& _data, u256 _gas, u256 _gasPrice, BlockNumber _blockNumber, FudgeFactor _ff = FudgeFactor::Strict) override;
	using Interface::call;

	/// Estimate gas usage for call/create.
	/// @param _maxGas An upper bound value for estimation, if not provided default value of c_maxGasEstimate will be used.
	/// @param _callback Optional callback function for progress reporting
	virtual std::pair<u256, ExecutionResult> estimateGas(Address const& _from, u256 _value, Address _dest, bytes const& _data, int64_t _maxGas, u256 _gasPrice, BlockNumber _blockNumber, GasEstimationCallback const& _callback) override;

	using Interface::balanceAt;
	using Interface::countAt;
	using Interface::stateAt;
	using Interface::codeAt;
	using Interface::codeHashAt;
	using Interface::storageAt;

	virtual u256 balanceAt(Address _a, BlockNumber _block) const override;
	virtual u256 countAt(Address _a, BlockNumber _block) const override;
	virtual u256 stateAt(Address _a, u256 _l, BlockNumber _block) const override;
	virtual h256 stateRootAt(Address _a, BlockNumber _block) const override;
	virtual bytes codeAt(Address _a, BlockNumber _block) const override;
	virtual h256 codeHashAt(Address _a, BlockNumber _block) const override;
	virtual std::map<h256, std::pair<u256, u256>> storageAt(Address _a, BlockNumber _block) const override;

	virtual LocalisedLogEntries logs(unsigned _watchId) const override;
	virtual LocalisedLogEntries logs(LogFilter const& _filter) const override;
	virtual void prependLogsFromBlock(LogFilter const& _filter, h256 const& _blockHash, BlockPolarity _polarity, LocalisedLogEntries& io_logs) const;

	/// Install, uninstall and query watches.
	virtual unsigned installWatch(LogFilter const& _filter, Reaping _r = Reaping::Automatic) override;
	virtual unsigned installWatch(h256 _filterId, Reaping _r = Reaping::Automatic) override;
	virtual unsigned installWatchWS(LogFilter _filterId, Reaping _r = Reaping::Automatic) override;

	virtual bool uninstallWatch(unsigned _watchId) override;
	virtual bool uninstallWatchWS(unsigned _watchId) override;
	virtual LocalisedLogEntries peekWatch(unsigned _watchId) const override;
	virtual LocalisedLogEntries checkWatch(unsigned _watchId) override;

	virtual h256 hashFromNumber(BlockNumber _number) const override;
	virtual BlockNumber numberFromHash(h256 _blockHash) const override;
	virtual int compareBlockHashes(h256 _h1, h256 _h2) const override;
	virtual BlockHeader blockInfo(h256 _hash) const override;
	virtual BlockDetails blockDetails(h256 _hash) const override;
	virtual Transaction transaction(h256 _transactionHash) const override;
	virtual LocalisedTransaction localisedTransaction(h256 const& _transactionHash) const override;
	virtual Transaction transaction(h256 _blockHash, unsigned _i) const override;
	virtual LocalisedTransaction localisedTransaction(h256 const& _blockHash, unsigned _i) const override;
	virtual TransactionReceipt transactionReceipt(h256 const& _transactionHash) const override;
	virtual LocalisedTransactionReceipt localisedTransactionReceipt(h256 const& _transactionHash) const override;
	virtual std::pair<h256, unsigned> transactionLocation(h256 const& _transactionHash) const override;
	virtual Transactions transactions(h256 _blockHash) const override;
	virtual TransactionHashes transactionHashes(h256 _blockHash) const override;
	virtual BlockHeader uncle(h256 _blockHash, unsigned _i) const override;
	virtual UncleHashes uncleHashes(h256 _blockHash) const override;
	virtual unsigned transactionCount(h256 _blockHash) const override;
	virtual unsigned uncleCount(h256 _blockHash) const override;
	virtual unsigned number() const override;
	virtual Transactions pending() const override;
	virtual TransactionQueue::FutureTransactions& queued() override;
	virtual h256s pendingHashes() const override;
	virtual BlockHeader pendingInfo() const override;
	virtual BlockDetails pendingDetails() const override;

	virtual EVMSchedule evmSchedule() const override { return sealEngine()->evmSchedule(pendingInfo().number()); }

	virtual ImportResult injectTransaction(bytes const& _rlp, IfDropped _id = IfDropped::Ignore) override { prepareForTransaction(); return m_tq.import(_rlp, _id); }
	virtual ImportResult injectBlock(bytes const& _block) override;

	using Interface::addresses;
	virtual Addresses addresses(BlockNumber _block) const override;
	virtual u256 gasLimitRemaining() const override;
	virtual u256 gasBidPrice() const override { return DefaultGasPrice; }

	/// Get the block author
   // virtual Public authorPublicKey() const override;

	virtual bool isKnown(h256 const& _hash) const override;
	virtual bool isKnown(BlockNumber _block) const override;
	virtual bool isKnownTransaction(h256 const& _transactionHash) const override;
	virtual bool isKnownTransaction(h256 const& _blockHash, unsigned _i) const override;

	virtual void startSealing() override { BOOST_THROW_EXCEPTION(InterfaceNotSupported("ClientBase::startSealing")); }
	virtual void stopSealing() override { BOOST_THROW_EXCEPTION(InterfaceNotSupported("ClientBase::stopSealing")); }
	virtual bool wouldSeal() const override { BOOST_THROW_EXCEPTION(InterfaceNotSupported("ClientBase::wouldSeal")); }

	virtual SyncStatus syncStatus() const override { BOOST_THROW_EXCEPTION(InterfaceNotSupported("ClientBase::syncStatus")); }

	Block block(BlockNumber _h) const;

protected:
	/// The interface that must be implemented in any class deriving this.
	/// {
	virtual BlockChain& bc() = 0;
	virtual BlockChain const& bc() const = 0;
	virtual Block block(h256 const& _h) const = 0;
	virtual Block preSeal() const = 0;
	virtual Block postSeal() const = 0;
	virtual void prepareForTransaction() = 0;
	/// }

	TransactionQueue m_tq;							///< Maintains a list of incoming transactions not yet in a block on the blockchain.

	// filters
	mutable Mutex x_filtersWatches;							///< Our lock.
	std::unordered_map<h256, InstalledFilter> m_filters;	///< The dictionary of filters that are active.
	std::unordered_map<h256, h256s> m_specialFilters = std::unordered_map<h256, std::vector<h256>>{{PendingChangedFilter, {}}, {ChainChangedFilter, {}}};
															///< The dictionary of special filters and their additional data
	std::map<unsigned, ClientWatch> m_watches;				///< Each and every watch - these reference a filter.

	std::map<unsigned, bool> m_subscribedWatch;				///< this watch is used for subscription
};

}}
