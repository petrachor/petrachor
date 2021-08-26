#pragma once

#include <jsonrpccpp/common/procedure.h>
#include <jsonrpccpp/server/iprocedureinvokationhandler.h>
#include <jsonrpccpp/server/abstractserverconnector.h>
#include <jsonrpccpp/server/requesthandlerfactory.h>
#include <jsonrpccpp/common/exception.h>

namespace jsonrpc{
	class AbstractProtocolHandler : public IProtocolHandler
	{
	public:
		AbstractProtocolHandler(IProcedureInvokationHandler &handler);
		~AbstractProtocolHandler();

		void HandleRequest(const std::string& request, std::string& retValue);

		virtual void AddProcedure(const Procedure& procedure);

		virtual void HandleJsonRequest(const Json::Value& request, Json::Value& response) = 0;
		virtual bool ValidateRequestFields(const Json::Value &val) = 0;
		virtual void WrapResult(const Json::Value& request, Json::Value& response, Json::Value& retValue) = 0;
		virtual void WrapError(const Json::Value& request, int code, const std::string &message, Json::Value& result) = 0;
		virtual procedure_t GetRequestType(const Json::Value& request) = 0;

	protected:
		IProcedureInvokationHandler &handler;
		std::map<std::string, Procedure> procedures;

		void ProcessRequest(const Json::Value &request, Json::Value &retValue);
		int ValidateRequest(const Json::Value &val);
	};

	class CustomRpcProtocolServerV2 : public AbstractProtocolHandler
	{
	public:
		CustomRpcProtocolServerV2(IProcedureInvokationHandler &handler);

		void HandleJsonRequest(const Json::Value& request, Json::Value& response);
		bool ValidateRequestFields(const Json::Value &val);
		void WrapResult(const Json::Value& request, Json::Value& response, Json::Value& retValue);
		void WrapError(const Json::Value& request, int code, const std::string &message, Json::Value& result);
		void WrapException(const Json::Value& request, const JsonRpcException &exception, Json::Value& result);
		procedure_t GetRequestType(const Json::Value& request);

	private:
		void HandleSingleRequest(const Json::Value& request, Json::Value& response);
		void HandleBatchRequest(const Json::Value& requests, Json::Value& response);
	};

}

