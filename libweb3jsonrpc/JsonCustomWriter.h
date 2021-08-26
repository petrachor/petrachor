#pragma once

#include "json/writer.h"

namespace Json {
	class CustomStreamWriterBuilder : public Json::StreamWriterBuilder {
	public:
		CustomStreamWriterBuilder();

		~CustomStreamWriterBuilder();

		Json::StreamWriter *newStreamWriter() const override;
	};
}