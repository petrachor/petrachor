#include <json/json.h>
#include <leveldb/write_batch.h>
#include <boost/filesystem.hpp>
#include <libethcore/Common.h>
#include <libethereum/Defaults.h>
#include <mutex>
#include "Pruner.h"

#ifdef MULTI_THREADED_PRUNING
#include <thread>
#endif

using namespace dev;
using namespace dev::eth;

namespace dev {
	namespace pruning {
		static ldb::DB* f_db = nullptr;
		static int64_t f_range = MAX_RANGE;
		static bool f_enabled = false;
		static h256 f_stateRoot;

		using CacheType = dev::MemoryDB::CacheType;
		using DeathCacheType = dev::MemoryDB::DeathCacheType;

		#ifdef MULTI_THREADED_PRUNING
		static std::recursive_mutex f_mutex;
		#endif

		bool init(fs::path const& basePath, h256 const& _genesisHash, int64_t range)
		{
			f_range = range;
			f_enabled = true;

			fs::path path = basePath.empty() ? Defaults::get()->dbPath() : basePath;

			path /= fs::path(toHex(_genesisHash.ref().cropped(0, 4))) / fs::path(toString(c_databaseVersion));
			fs::create_directories(path);
			DEV_IGNORE_EXCEPTIONS(fs::permissions(path, fs::owner_all));

			ldb::Options o;
			o.max_open_files = 256;
			o.create_if_missing = true;
			ldb::Status status = ldb::DB::Open(o, (path / fs::path("prune")).string(), &f_db);
			if (!status.ok() || !f_db)
			{
				if (fs::space(path / fs::path("prune")).available < 1024)
				{
					cwarn << "Not enough available space found on hard drive. Please free some up and then re-run. Bailing.";
				}
				else
				{
					cwarn << status.ToString();
					cwarn <<
					"Database " <<
					(path / fs::path("prune")) <<
					"already open. You appear to have another instance of ethereum running. Bailing.";
				}

				return false;
			}

			return true;
		}

		void setStateRoot(h256 const stateRoot)
		{
			f_stateRoot = stateRoot;
		}

		void exec(int64_t blockNum, CacheType mainCache, DeathCacheType deathCache, ldb::DB* stateDb)
		{
			#ifdef MULTI_THREADED_PRUNING
			std::lock_guard<std::recursive_mutex> lock(f_mutex);
			#endif

			Json::Value deathList;
			ldb::Status status;
			ldb::ReadOptions readOptions;

			// 1. loop thru State and get the entry with 0 reference count

			// a. Main cache
			auto iter = mainCache.begin();
			while(iter != mainCache.end())
			{
				if(iter->second.second == 0)
				{

					// get the hash key
					h256 keys = iter->first;

					// we found a candidate for deletion
					std::string hash = keys.hex();

					// append the hash to the death list
					deathList.append(hash);

					ctrace << "Pruning : Added hash with 0 reference " << keys;
				}

				iter++;
			}

			// b. add the death cache list
			auto iterDeath = deathCache.begin();
			while(iterDeath != deathCache.end())
			{
				// get the hash key
				h256 keys = *iterDeath;

				// do not delete if this is the current state root
				if(f_stateRoot != keys)
				{
					// we found a candidate for deletion
					std::string hash = keys.hex();

					// append the hash to the death list
					deathList.append(hash);

					ctrace << "Pruning : Adding hash for deletion " << keys;
				}

				iterDeath++;
			}

			// 2. insert to db using current blocknumber as key when there is an entry
			if(deathList.size())
			{
				ctrace << "Pruning : saving to database with " << deathList.size()
					<< " entries to be deleted at height" << blockNum + f_range;

				Json::FastWriter fastWriter;
				std::string key = std::to_string(blockNum + f_range);
				std::string entry = fastWriter.write(deathList);
				status = f_db->Put(leveldb::WriteOptions(), key, entry);
			}

			// 3. start pruning, get the death cache list for this block number
			std::string deathCacheDb;
			std::string key = std::to_string(blockNum);
			status = f_db->Get(readOptions, key, &deathCacheDb);

			if (!status.ok()) {
				// no entry to Prune
				return;
			}

			// convert to JSON object
			Json::Reader reader;
			Json::Value root;
			reader.parse(deathCacheDb, root);

			ldb::WriteBatch batch;
			int deleteCount = 0;
			for(unsigned int i = 0; i < root.size(); i++) {
				auto hash = root[i].asString();

				// convert to h256 hash
				h256 entryKey (hash);

				// check from current cache if it is indeed to be deleted or was it re-activated
				iter = mainCache.find(entryKey);
				if(!(iter != mainCache.end() && iter->second.second > 0)) {
					std::string ret;
					auto sliceKey = ldb::Slice((char const*)entryKey.data(), entryKey.size);
					status = stateDb->Get(readOptions, sliceKey, &ret);
					if (status.ok()) {
						ctrace << "Pruning : deleting hash " << entryKey;

						batch.Delete(sliceKey);
						++deleteCount;
					}
				}
			}

			// delete from main DB
			status = stateDb->Write(leveldb::WriteOptions(), &batch);

			// delete the key in prune database
			status = f_db->Delete(leveldb::WriteOptions(), key);

			ctrace << "Pruning : deleted " <<  deleteCount << " entries at height " << blockNum;
		}

		void prune(int64_t blockNum, dev::OverlayDB* overlayDb)
		{
			if(!f_enabled) return;

#ifdef MULTI_THREADED_PRUNING
			// since we are pruning in separate thread, copy the content of the cache as state::commit() deletes them
			CacheType localCache(overlayDb->cache().begin(), overlayDb->cache().end());
			DeathCacheType localDeathCache(overlayDb->deathCache().begin(), overlayDb->deathCache().end());

			std::thread{ std::bind(
					[=]() {
				exec(blockNum, localCache, localDeathCache, overlayDb->db());
			}) }.detach();
#else
			exec(blockNum, overlayDb->cache(), overlayDb->deathCache(), overlayDb->db());
#endif
		}
	}
}
