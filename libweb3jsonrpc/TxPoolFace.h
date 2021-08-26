#pragma once

#include "ModularServer.h"

namespace dev {
	namespace rpc {
		class TxPoolFace : public ServerInterface<TxPoolFace>
		{
		public:
			TxPoolFace()
			{
				// txpool Namespace
				this->bindAndAddMethod(jsonrpc::Procedure("txpool_content", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_STRING,  NULL), &dev::rpc::TxPoolFace::txpool_contentI);
			}

			inline virtual void txpool_contentI(const Json::Value &request, Json::Value &response)
			{
				(void)request;
				response = this->txpool_content();
			}

			virtual Json::Value txpool_content() = 0;
		};

	}
}