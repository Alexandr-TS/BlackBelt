#pragma once
#include "json.h"
#include "router.h"
#include "svg.h"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <optional>
#include <cmath>
#include <set>
#include <functional>

using namespace std;

class Response {
public:
    enum class EResponseType {
        BUS_INFO,
        STOP_INFO,
        ROUTE_INFO,
        MAP_INFO
    } Type;

    Response(EResponseType&& type)
        : Type(type)
    {}

    void SetRequestId(int32_t id) {
        Request_id = id;
    }

    int32_t Request_id = -1;
};

class RouteInfoResponse : public Response {
public:
    RouteInfoResponse() : Response(Response::EResponseType::ROUTE_INFO) {}

    RouteInfoResponse(Json::Node&& node)
        : Response(Response::EResponseType::ROUTE_INFO)
        , Info(node)
    {}

    Json::Node Info;
};

class MapInfoResponse : public Response {
public:
    MapInfoResponse() : Response(Response::EResponseType::MAP_INFO) {}
    
    MapInfoResponse(Json::Node&& node)
        : Response(Response::EResponseType::MAP_INFO)
        , Info(node)
    {}

    Json::Node Info;
};

class BusInfoResponse: public Response {
public:
    BusInfoResponse() : Response(Response::EResponseType::BUS_INFO) {}

    struct MetricsInfo {
        int CntStops;
        int UniqueStops;
        double PathLength;
        double Curvature;
    };

    BusInfoResponse(const string& name, optional<MetricsInfo>&& info)
        : Name(name)
        , Info(info)
        , Response(Response::EResponseType::BUS_INFO)
    {}

    string Name;
    optional<MetricsInfo> Info;
};

class StopInfoResponse : public Response {
public:
	StopInfoResponse() : Response(Response::EResponseType::STOP_INFO) {}
	
	struct BusesInfo {
		set<string> Buses;
	};

	StopInfoResponse(const string& name, optional<BusesInfo>&& info)
		: Name(name)
		, Info(info)
		, Response(Response::EResponseType::STOP_INFO)
	{}

	string Name;
	optional<BusesInfo> Info;
};
