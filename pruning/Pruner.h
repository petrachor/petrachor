#pragma once
#include <leveldb/db.h>
#include <libdevcore/OverlayDB.h>

namespace dev {
	namespace pruning {
		constexpr int64_t MAX_RANGE = 1024;

		namespace fs = boost::filesystem;
		bool init(fs::path const& _basePath, h256 const& _genesisHash, int64_t range = MAX_RANGE);

		void prune(int64_t blockNum, dev::OverlayDB* overlayDb);
	}
}
