#include "test_runner.h"

#include "manager.h"
#include "utils.h"
#include "requests.h"

using namespace std;

const unordered_map<string, Request::ERequestType> ModifyRequestTypeByString = {
    {"Stop", Request::ERequestType::ADD_STOP},
    {"Bus", Request::ERequestType::ADD_BUS}
};

const unordered_map<string, Request::ERequestType> ReadRequestTypeByString = {
    {"Bus", Request::ERequestType::QUERY_BUS},
    {"Stop", Request::ERequestType::QUERY_STOP},
    {"Route", Request::ERequestType::QUERY_ROUTE},
    {"Map", Request::ERequestType::QUERY_MAP}
};

RequestHolder CreateRequestHolder(Request::ERequestType type) {
    switch (type) {
        case Request::ERequestType::ADD_BUS:
            return make_unique<AddBusRequest>();
        case Request::ERequestType::ADD_STOP:
            return make_unique<AddStopRequest>();
        case Request::ERequestType::QUERY_BUS:
            return make_unique<ReadBusInfoRequest>();
        case Request::ERequestType::QUERY_STOP:
            return make_unique<ReadStopInfoRequest>();
        case Request::ERequestType::QUERY_ROUTE:
            return make_unique<ReadRouteInfoRequest>();
        case Request::ERequestType::QUERY_MAP:
            return make_unique<ReadMapInfoRequest>();
        default:
            throw "undefined type";
    }
}

void ReadRequestsJson(vector<RequestHolder>& requests, const Node& node,
    const unordered_map<string, Request::ERequestType>& RequestTypeByString) {
    for (const auto& query_node : node.AsArray()) {
        auto type = RequestTypeByString.at(query_node.AsMap().at("type").AsString());
        requests.push_back(CreateRequestHolder(type));
        requests.back()->ReadInfo(query_node);
    }
}

struct InputData {
    BusManagerSettings bus_manager_settings;
    RenderSettings render_settings;
    vector<RequestHolder> requests;
};

InputData ReadAllRequestsJson() {
    auto document = Load(cin);
    vector<RequestHolder> requests;

    const auto& modify_requests = document.GetRoot().AsMap().at("base_requests");
    ReadRequestsJson(requests, modify_requests, ModifyRequestTypeByString);

    const auto& read_requests = document.GetRoot().AsMap().at("stat_requests");
    ReadRequestsJson(requests, read_requests, ReadRequestTypeByString);

    const auto& settings_info = document.GetRoot().AsMap().at("routing_settings").AsMap();
    auto settings = BusManagerSettings(
        static_cast<int>(settings_info.at("bus_wait_time").AsDouble()),
        static_cast<int>(settings_info.at("bus_velocity").AsDouble())
    );

    auto render_settings = RenderSettings(document.GetRoot().AsMap().at("render_settings").AsMap());

    return { settings, render_settings, move(requests) };
}

vector<unique_ptr<Response>> GetResponses(const InputData& input) {
    BusManager manager(input.bus_manager_settings, input.render_settings);
    vector<unique_ptr<Response>> responses;
    
    for (auto& request_holder : input.requests) {
        if (request_holder->Type == Request::ERequestType::ADD_STOP) {
            const auto& request = static_cast<const ModifyRequest&>(*request_holder);
            request.Process(manager);
        }
    }

    for (auto& request_holder : input.requests) {
        if (request_holder->Type == Request::ERequestType::ADD_BUS) {
            const auto& request = static_cast<const ModifyRequest&>(*request_holder);
            request.Process(manager);
        }
    }

    manager.BuildRoutes();

    for (auto& request_holder : input.requests) {
        if (request_holder->Type == Request::ERequestType::QUERY_BUS) {
            const auto& request = static_cast<const ReadBusInfoRequest&>(*request_holder);
            responses.push_back(make_unique<BusInfoResponse>(request.Process(manager)));
        }
        else if (request_holder->Type == Request::ERequestType::QUERY_STOP) {
            const auto& request = static_cast<const ReadStopInfoRequest&>(*request_holder);
            responses.push_back(make_unique<StopInfoResponse>(request.Process(manager)));
        }
        else if (request_holder->Type == Request::ERequestType::QUERY_ROUTE) {
            const auto& request = static_cast<const ReadRouteInfoRequest&>(*request_holder);
            responses.push_back(make_unique<RouteInfoResponse>(request.Process(manager)));
        }
        else if (request_holder->Type == Request::ERequestType::QUERY_MAP) {
            const auto& request = static_cast<const ReadMapInfoRequest&>(*request_holder);
            responses.push_back(make_unique<MapInfoResponse>(request.Process(manager)));
        }
    }
    return responses;
}

void PrintResponsesJson(const vector<unique_ptr<Response>>& responses) {
    auto result_vec = vector<Node>();

    for (const auto& response_ptr: responses) {
        Node response_node;
        if (response_ptr->Type == Response::EResponseType::BUS_INFO) {
            auto cur_node = map<string, Node>{};
            const auto& response = static_cast<const BusInfoResponse&>(*response_ptr);
            cur_node["request_id"] = Node(static_cast<double>(response.Request_id));
            if (response.Info) {
                cur_node["stop_count"] = Node(static_cast<double>(response.Info.value().CntStops));
                cur_node["unique_stop_count"] = Node(static_cast<double>(response.Info.value().UniqueStops));
                cur_node["curvature"] = Node(response.Info.value().Curvature);
                cur_node["route_length"] = Node(response.Info.value().PathLength);
            }
            else {
                cur_node["error_message"] = Node("not found"s);
            }
            response_node = Node(cur_node);
        }
        else if (response_ptr->Type == Response::EResponseType::STOP_INFO) {
            auto cur_node = map<string, Node>{};
            const auto& response = static_cast<const StopInfoResponse&>(*response_ptr);
            cur_node["request_id"] = Node(static_cast<double>(response.Request_id));
            if (response.Info) {
                auto buses_vector = vector<Node>();
                for (const auto& bus_name : response.Info.value().Buses) {
                    buses_vector.push_back(Node(bus_name));
                }
                cur_node["buses"] = buses_vector;
            }
            else {
                cur_node["error_message"] = Node("not found"s);
            }
            response_node = Node(cur_node);
        }
        else if (response_ptr->Type == Response::EResponseType::ROUTE_INFO) {
            const auto& response = static_cast<const RouteInfoResponse&>(*response_ptr);
            response_node = response.Info;
        }
        else if (response_ptr->Type == Response::EResponseType::MAP_INFO) {
            const auto& response = static_cast<const MapInfoResponse&>(*response_ptr);
            response_node = response.Info;
        }
        else {
            throw runtime_error("Not implemented Response to print");
        }
        result_vec.push_back(response_node);
    }

    auto result_node = Node(result_vec);
    result_node.Print(cout);
}

int main() {
    FILE* file;
	freopen_s(&file, "C:\\Users\\Admin\\source\\repos\\BlackBelt\\Solutions\\BusManager\\a.in", "r", stdin);

    //FILE* file2;
	//freopen_s(&file2, "C:\\Users\\Admin\\source\\repos\\BlackBelt\\Solutions\\BusManager\\map.svg", "w", stdout);

	auto requests = ReadAllRequestsJson();
	const auto responses = GetResponses(move(requests));
	PrintResponsesJson(responses);
}
