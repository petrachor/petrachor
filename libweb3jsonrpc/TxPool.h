#pragma once

#include "libweb3jsonrpc/TxPoolFace.h"
#include "libethereum/Interface.h"

namespace dev {
	namespace rpc {
		class TxPool : public TxPoolFace {
		public:
			TxPool(eth::Interface& _eth);

			virtual Json::Value txpool_content();

			virtual RPCModules implementedModules() const override
			{
				return RPCModules{RPCModule{"txtpool", "1.0"}};
			}

		private:
			eth::Interface* client() { return &m_eth; }
			eth::Interface& m_eth;
		};
	}
}
