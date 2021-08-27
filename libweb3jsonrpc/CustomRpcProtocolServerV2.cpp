#include <string>
#include <jsonrpccpp/common/errors.h>
#include <libweb3jsonrpc/JsonCustomWriter.h>
#include "CustomRpcProtocolServerV2.h"

#include <iostream>

using namespace jsonrpc;
using namespace std;

#define KEY_REQUEST_METHODNAME  "method"
#define KEY_REQUEST_ID          "id"
#define KEY_REQUEST_PARAMETERS  "params"
#define KEY_RESPONSE_ERROR      "error"
#define KEY_RESPONSE_RESULT     "result"

#define KEY_REQUEST_VERSION     "jsonrpc"
#define JSON_RPC_VERSION2        "2.0"

static bool f_isStandardRPC = true;

AbstractProtocolHandler::AbstractProtocolHandler(
		IProcedureInvokationHandler &handler)
		: handler(handler) {}

AbstractProtocolHandler::~AbstractProtocolHandler()
{}

void AbstractProtocolHandler::AddProcedure(const Procedure &procedure) {
	this->procedures[procedure.GetProcedureName()] = procedure;
}

void AbstractProtocolHandler::HandleRequest(const std::string &request,
											std::string &retValue) {
	Json::Reader reader;
	Json::Value req;
	Json::Value resp;
	Json::StreamWriterBuilder wbuilder;
	Json::CustomStreamWriterBuilder cwbuilder;
	wbuilder["indentation"] = "";
	cwbuilder["indentation"] = "";
	f_isStandardRPC = false;

	try {
		if (reader.parse(request, req, false)) {
			this->HandleJsonRequest(req, resp);
		} else {
			this->WrapError(
					Json::nullValue, Errors::ERROR_RPC_JSON_PARSE_ERROR,
					Errors::GetErrorMessage(Errors::ERROR_RPC_JSON_PARSE_ERROR), resp);
		}
	} catch (const Json::Exception &e) {
		this->WrapError(Json::nullValue, Errors::ERROR_RPC_JSON_PARSE_ERROR,
						Errors::GetErrorMessage(Errors::ERROR_RPC_JSON_PARSE_ERROR),
						resp);
	}

	if (resp != Json::nullValue)
		retValue = Json::writeString(f_isStandardRPC? wbuilder : cwbuilder, resp);

}

void AbstractProtocolHandler::ProcessRequest(const Json::Value &request,
											 Json::Value &response) {
	Procedure &method =
			this->procedures[request[KEY_REQUEST_METHODNAME].asString()];
	Json::Value result;

	std::size_t found= method.GetProcedureName().find("txpool_");
	f_isStandardRPC = (found == std::string::npos);

	if (method.GetProcedureType() == RPC_METHOD) {
		handler.HandleMethodCall(method, request[KEY_REQUEST_PARAMETERS], result);
		if(f_isStandardRPC) {
			this->WrapResult(request, response, result);
		} else {
			response = result;
		}
	} else {
		handler.HandleNotificationCall(method, request[KEY_REQUEST_PARAMETERS]);
		response = Json::nullValue;
	}
}

int AbstractProtocolHandler::ValidateRequest(const Json::Value &request) {
	int error = 0;
	Procedure proc;
	if (!this->ValidateRequestFields(request)) {
		error = Errors::ERROR_RPC_INVALID_REQUEST;
	} else {
		map<string, Procedure>::iterator it =
				this->procedures.find(request[KEY_REQUEST_METHODNAME].asString());
		if (it != this->procedures.end()) {
			proc = it->second;
			if (this->GetRequestType(request) == RPC_METHOD &&
				proc.GetProcedureType() == RPC_NOTIFICATION) {
				error = Errors::ERROR_SERVER_PROCEDURE_IS_NOTIFICATION;
			} else if (this->GetRequestType(request) == RPC_NOTIFICATION &&
					   proc.GetProcedureType() == RPC_METHOD) {
				error = Errors::ERROR_SERVER_PROCEDURE_IS_METHOD;
			} else if (!proc.ValdiateParameters(request[KEY_REQUEST_PARAMETERS])) {
				error = Errors::ERROR_RPC_INVALID_PARAMS;
			}
		} else {
			error = Errors::ERROR_RPC_METHOD_NOT_FOUND;
		}
	}
	return error;
}

//--------------------------------------------------------------------------------------------------------------------
CustomRpcProtocolServerV2 ::CustomRpcProtocolServerV2(IProcedureInvokationHandler &handler)
		: AbstractProtocolHandler(handler) {}

void CustomRpcProtocolServerV2::HandleJsonRequest(const Json::Value &req,
											Json::Value &response) {
	// It could be a Batch Request
	if (req.isArray()) {
		this->HandleBatchRequest(req, response);
	} // It could be a simple Request
	else if (req.isObject()) {
		this->HandleSingleRequest(req, response);
	} else {
		this->WrapError(Json::nullValue, Errors::ERROR_RPC_INVALID_REQUEST,
						Errors::GetErrorMessage(Errors::ERROR_RPC_INVALID_REQUEST),
						response);
	}
}
void CustomRpcProtocolServerV2::HandleSingleRequest(const Json::Value &req,
											  Json::Value &response) {
	int error = this->ValidateRequest(req);
	if (error == 0) {
		try {
			this->ProcessRequest(req, response);
		} catch (const JsonRpcException &exc) {
			this->WrapException(req, exc, response);
		}
	} else {
		this->WrapError(req, error, Errors::GetErrorMessage(error), response);
	}
}
void CustomRpcProtocolServerV2::HandleBatchRequest(const Json::Value &req,
											 Json::Value &response) {
	if (req.empty())
		this->WrapError(Json::nullValue, Errors::ERROR_RPC_INVALID_REQUEST,
						Errors::GetErrorMessage(Errors::ERROR_RPC_INVALID_REQUEST),
						response);
	else {
		for (unsigned int i = 0; i < req.size(); i++) {
			Json::Value result;
			this->HandleSingleRequest(req[i], result);
			if (result != Json::nullValue)
				response.append(result);
		}
	}
}
bool CustomRpcProtocolServerV2::ValidateRequestFields(const Json::Value &request) {
	if (!request.isObject())
		return false;
	if (!(request.isMember(KEY_REQUEST_METHODNAME) &&
		  request[KEY_REQUEST_METHODNAME].isString()))
		return false;
	if (!(request.isMember(KEY_REQUEST_VERSION) &&
		  request[KEY_REQUEST_VERSION].isString() &&
		  request[KEY_REQUEST_VERSION].asString() == JSON_RPC_VERSION2))
		return false;
	if (request.isMember(KEY_REQUEST_ID) &&
		!(request[KEY_REQUEST_ID].isIntegral() ||
		  request[KEY_REQUEST_ID].isString() || request[KEY_REQUEST_ID].isNull()))
		return false;
	if (request.isMember(KEY_REQUEST_PARAMETERS) &&
		!(request[KEY_REQUEST_PARAMETERS].isObject() ||
		  request[KEY_REQUEST_PARAMETERS].isArray() ||
		  request[KEY_REQUEST_PARAMETERS].isNull()))
		return false;
	return true;
}

void CustomRpcProtocolServerV2::WrapResult(const Json::Value &request,
									 Json::Value &response,
									 Json::Value &result) {
	response[KEY_REQUEST_VERSION] = JSON_RPC_VERSION2;
	response[KEY_RESPONSE_RESULT] = result;
	response[KEY_REQUEST_ID] = request[KEY_REQUEST_ID];
}

void CustomRpcProtocolServerV2::WrapError(const Json::Value &request, int code,
									const string &message,
									Json::Value &result) {
	result["jsonrpc"] = "2.0";
	result["error"]["code"] = code;
	result["error"]["message"] = message;

	if (request.isObject() && request.isMember("id") &&
		(request["id"].isNull() || request["id"].isIntegral() ||
		 request["id"].isString())) {
		result["id"] = request["id"];
	} else {
		result["id"] = Json::nullValue;
	}
}

void CustomRpcProtocolServerV2::WrapException(const Json::Value &request,
										const JsonRpcException &exception,
										Json::Value &result) {
	this->WrapError(request, exception.GetCode(), exception.GetMessage(), result);
	result["error"]["data"] = exception.GetData();
}

procedure_t CustomRpcProtocolServerV2::GetRequestType(const Json::Value &request) {
	if (request.isMember(KEY_REQUEST_ID))
		return RPC_METHOD;
	return RPC_NOTIFICATION;
}
