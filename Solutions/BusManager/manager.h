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
#include <sstream>
#include <unordered_map>
#include <utility>
#include <optional>
#include <cmath>
#include <functional>
using namespace std;

const double PI = 3.1415926535;
const double RADIUS = 6371;

struct Location {
    double Latitude = 0.0;
    double Longitude = 0.0;

    double Distance(const Location& other) const {
        pair<double, double> p1 = { Latitude / 180 * PI, Longitude / 180 * PI };
        pair<double, double> p2 = { other.Latitude / 180 * PI, other.Longitude / 180 * PI };
        return acos(sin(p1.first) * sin(p2.first) +
                cos(p1.first) * cos(p2.first) * cos(p1.second - p2.second)) * RADIUS * 1000;
    }
};

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

struct Stop {
    Location StopLocation;
    set<string> BusesNames;
};

struct Bus {
    Bus() {}
    
    Bus(const vector<string>& path, const map<string, Stop>& stops, 
        const unordered_map<string, unordered_map<string, double>>& distances_between_stops, bool is_round_trip) {
        IsRoundTrip = is_round_trip;
        RouteLength = 0;
        GeoLength = 0;
        set<string> unique_stops;
        for (size_t i = 0; i < path.size(); ++i) {
            unique_stops.insert(path[i]);
            if (i) {
                double geo_dist = stops.at(path[i - 1]).StopLocation.Distance(stops.at(path[i]).StopLocation);
                GeoLength += geo_dist;
                if (distances_between_stops.count(path[i - 1]) && 
                    distances_between_stops.at(path[i - 1]).count(path[i])) {
                    RouteLength += distances_between_stops.at(path[i - 1]).at(path[i]);
                }
                else {
                    string msg;
                    for (const auto& el : distances_between_stops) {
                        msg += el.first + ":\n";
                        for (const auto& e : el.second) {
                            msg += "  " + e.first + ":" + to_string(e.second) + "\n";
                        }
                    }

                    //throw runtime_error("error: " + msg);
                    RouteLength += geo_dist;
                }
            }
        }
        CntUnique = static_cast<int>(unique_stops.size());
        Stops = path;
    }
    double RouteLength = 0;
    double GeoLength = 0;
    int CntUnique = 0;
    bool IsRoundTrip = false;
    vector<string> Stops;

    BusInfoResponse GetInfo(const string& name) const {
        double curvature = RouteLength / GeoLength;
        return { name, BusInfoResponse::MetricsInfo{
            static_cast<int>(Stops.size()), CntUnique, RouteLength, curvature } };
    }
};


struct BusManagerSettings {
    BusManagerSettings() 
        : BusWaitTime(0)
        , BusVelocity(0)
    {}

    BusManagerSettings(int bus_wait_time, int bus_velocity)
        : BusWaitTime(bus_wait_time)
        , BusVelocity(bus_velocity)
    {}

    int BusWaitTime;
    int BusVelocity;
};

class RenderSettings {
public:
    RenderSettings() {}
    RenderSettings(const map<string, Json::Node>& node) {
        width = node.at("width").AsDouble(); 
        height = node.at("height").AsDouble();
        padding = node.at("padding").AsDouble();
        stop_radius = node.at("stop_radius").AsDouble();
        line_width = node.at("line_width").AsDouble();
        stop_label_font_size = static_cast<int>(node.at("stop_label_font_size").AsDouble());
        stop_label_offset = Svg::Point(node.at("stop_label_offset").AsArray()[0].AsDouble(),
            node.at("stop_label_offset").AsArray()[1].AsDouble());
        underlayer_color = ParseColor(node.at("underlayer_color"));
        underlayer_width = node.at("underlayer_width").AsDouble();
        const auto color_palette_ar = node.at("color_palette").AsArray();
        for (const auto& color_node: color_palette_ar) {
            color_palette.emplace_back(ParseColor(color_node));
        }
        bus_label_font_size = static_cast<int>(node.at("bus_label_font_size").AsDouble());
        bus_label_offset = Svg::Point(node.at("bus_label_offset").AsArray()[0].AsDouble(),
            node.at("bus_label_offset").AsArray()[1].AsDouble());
        for (const auto& layer: node.at("layers").AsArray()) {
            layers.emplace_back(layer.AsString());
        }
    }

    double width;
    double height;
    double padding;
    double stop_radius;
    double line_width;
    int stop_label_font_size;
    Svg::Point stop_label_offset;
    Svg::Color underlayer_color;
    double underlayer_width;
    vector<Svg::Color> color_palette;
    int bus_label_font_size;
    Svg::Point bus_label_offset;
    vector<string> layers;

private:
    Svg::Color ParseColor(const Json::Node& node) {
        using namespace Json;
        using namespace Svg;
        if (node.IsArray()) {
            if (node.AsArray().size() == 3) {
                const auto& tmp = node.AsArray();
                return Color(Rgb(static_cast<int>(tmp[0].AsDouble()),
                    static_cast<int>(tmp[1].AsDouble()),
                    static_cast<int>(tmp[2].AsDouble())));
            } else {
                const auto& tmp = node.AsArray();
                return Color(Rgba(static_cast<int>(tmp[0].AsDouble()),
                    static_cast<int>(tmp[1].AsDouble()),
                    static_cast<int>(tmp[2].AsDouble()),
                    tmp[3].AsDouble()));
            }
        } else {
            return Color(node.AsString());
        }
    }
};

class BusManager {
public:
    BusManager(const BusManagerSettings& bus_manager_settings, const RenderSettings& render_settings)
        : BusManagerSettings_(bus_manager_settings)
        , RenderSettings_(render_settings)
    {}

    void AddStop(const string& name, Location location, const unordered_map<string, double>& dist_by_stop) {
        Stops[name] = Stop{ location };
        for (const auto& [stop_name, dist] : dist_by_stop) {
            DistancesBetweenStops[name][stop_name] = dist;
            if (!DistancesBetweenStops.count(stop_name) || !DistancesBetweenStops[stop_name].count(name)) {
                DistancesBetweenStops[stop_name][name] = dist;
            }
        }
    }

    void AddBus(const string& name, const vector<string>& path, bool is_round_trip) {
        Buses[name] = Bus(path, Stops, DistancesBetweenStops, is_round_trip);
        for (const auto& stop_name : path) {
            assert(Stops.count(stop_name));
            Stops[stop_name].BusesNames.insert(name);
        }
    }

    BusInfoResponse GetBusInfoResponse(const string& bus_name) {
        auto iter = Buses.find(bus_name);
        if (iter == Buses.end()) {
            return { bus_name, nullopt };
        }
        return iter->second.GetInfo(bus_name);
    }

    StopInfoResponse GetStopInfoResponse(const string& stop_name) {
        auto iter = Stops.find(stop_name);
        if (iter == Stops.end()) {
            return StopInfoResponse{ stop_name, nullopt };
        }
        return StopInfoResponse{ stop_name, StopInfoResponse::BusesInfo{ iter->second.BusesNames } };
    }

    RouteInfoResponse GetRouteResponse(const string& stop_from, const string& stop_to) {
        using namespace Json;

        size_t from_id = StopIdByName[stop_from];
        size_t to_id = StopIdByName[stop_to];
        auto route = RouteBuilder->BuildRoute(from_id, to_id);
        if (!route) {
            auto node_map = map<string, Node>();
            node_map["error_message"] = Node("not found"s);
            return RouteInfoResponse(Node(node_map));
        }

        auto node_map = map<string, Node>();
        auto result = route.value();
        node_map["total_time"] = Node(result.weight);
        auto node_map_items = vector<Node>();
        vector<int> edges_ids;
        for (size_t i = 0; i < result.edge_count; ++i) {
            auto edge_id = RouteBuilder->GetRouteEdge(result.id, i);
            auto wait_node_map = map<string, Node>();
            wait_node_map["time"] = Node(static_cast<double>(BusManagerSettings_.BusWaitTime));
            wait_node_map["type"] = Node("Wait"s);
            wait_node_map["stop_name"] = Node(Edges[edge_id].StopFrom);
            node_map_items.push_back(Node(wait_node_map));

            auto ride_node_map = map<string, Node>();
            ride_node_map["bus"] = Node(Edges[edge_id].BusName);
            ride_node_map["type"] = Node("Bus"s);
            ride_node_map["time"] = Node(Edges[edge_id].Weight - BusManagerSettings_.BusWaitTime);
            ride_node_map["span_count"] = Node(Edges[edge_id].SpanCount);
            node_map_items.push_back(Node(ride_node_map));
        }
        node_map["items"] = Node(node_map_items);
        return RouteInfoResponse(Node(node_map));
    }

    MapInfoResponse GetMapInfoResponse() {
        using namespace Svg; 
        using namespace Json;

        Svg::Document svg_doc;

        auto map_info = ComputeMapInfo();
  
        for (const auto& layer: RenderSettings_.layers) {
            if (layer == "bus_lines") {
                AddPolylinesToSvg(map_info, svg_doc);
            } else if (layer == "bus_labels") {
                AddBusesNamesToSvg(map_info, svg_doc);
            } else if (layer == "stop_points") {
                AddStopCirclesToSvg(map_info, svg_doc); 
            } else if (layer == "stop_labels") {
                AddStopNamesToSvg(map_info, svg_doc);
            }
        }      
   
        stringstream ss;
        svg_doc.Render(ss);
        auto raw_text = ss.str();

        //cout << endl << raw_text << endl << endl;

        string added_slashes = "";
        for (const auto ch: raw_text) {
            if (ch == '\"') {
                added_slashes += '\\'; 
            }
            added_slashes += ch;
        }

        map<string, Node> result = {{"map", Node(added_slashes)}};
        return MapInfoResponse(Node(result));
        
    }
    
    void BuildRoutes() {
		using namespace Graph;

		size_t cur_stop_idx = 0;
		for (const auto& [name, Stop] : Stops) {
			StopIdByName[name] = cur_stop_idx++;
		}

		GraphPtr = make_shared<DirectedWeightedGraph<double>>(Stops.size());

		map<pair<string, string>, tuple<double, string, int>> best_bus_by_2_stops;

		for (const auto& [bus_name, bus] : Buses) {
			for (size_t first_pos = 0; first_pos + 1 < bus.Stops.size(); ++first_pos) {
				double weight = BusManagerSettings_.BusWaitTime;
				for (size_t second_pos = first_pos + 1; second_pos < bus.Stops.size(); ++second_pos) {
					weight += DistancesBetweenStops[bus.Stops[second_pos - 1]][bus.Stops[second_pos]] /
						(BusManagerSettings_.BusVelocity * 1000 / 60.);
					if (!best_bus_by_2_stops.count({ bus.Stops[first_pos], bus.Stops[second_pos] })) {
						best_bus_by_2_stops[{bus.Stops[first_pos], bus.Stops[second_pos]}] =
							make_tuple(weight, bus_name, static_cast<int>(second_pos - first_pos));
					}
					else {
						best_bus_by_2_stops[{bus.Stops[first_pos], bus.Stops[second_pos]}] =
							min(make_tuple(weight, bus_name, static_cast<int>(second_pos - first_pos)),
								best_bus_by_2_stops[{bus.Stops[first_pos], bus.Stops[second_pos]}]);
					}
				}
			}
		}

		for (const auto& el : best_bus_by_2_stops) {
				const auto& [from_stop, to_stop] = el.first;
				const auto& [dist, bus_name, span_count] = el.second;
				Edge<double> edge{ StopIdByName[from_stop], StopIdByName[to_stop], dist };
				auto edge_id = GraphPtr->AddEdge(edge);
				Edges.push_back({ dist, from_stop, to_stop, bus_name, edge_id, span_count });
		}

		RouteBuilder = make_unique<Graph::Router<double>>(*GraphPtr);
    }

private:
    struct MapInfo {
        double min_lat;
        double max_lat;
        double min_lon;
        double max_lon;
        double zoom_coef;
        map<double, double> lon_to_image_coor;
        map<double, double> lat_to_image_coor;
    };

    MapInfo ComputeMapInfo() {
        vector<pair<double, double>> stops_points;
        for (const auto& [stop_name, stop]: Stops) {
            stops_points.push_back({stop.StopLocation.Latitude, stop.StopLocation.Longitude});
        }

        double step_lon_coor = (RenderSettings_.width - 2 * RenderSettings_.padding) / (max((size_t)2, stops_points.size()) - 1);
        double step_lat_coor = (RenderSettings_.height - 2 * RenderSettings_.padding) / (max((size_t)2, stops_points.size()) - 1);

        MapInfo map_info;
        sort(stops_points.begin(), stops_points.end(), [](const auto& lhs, const auto& rhs) {return lhs.second < rhs.second; });
        for (size_t i = 0; i < stops_points.size(); ++i) {
            map_info.lon_to_image_coor[stops_points[i].second] = RenderSettings_.padding + step_lon_coor * i;
        }
        sort(stops_points.begin(), stops_points.end());
        for (size_t i = 0; i < stops_points.size(); ++i) {
            map_info.lat_to_image_coor[stops_points[i].first] = RenderSettings_.height - RenderSettings_.padding - step_lat_coor * i;
        }

        /*
        auto min_lat = min_element(stops_points.begin(), stops_points.end())->first;
        auto max_lat = max_element(stops_points.begin(), stops_points.end())->first;
        auto min_lon = min_element(stops_points.begin(), stops_points.end(), [](const auto& lhs, const auto& rhs) {return lhs.second < rhs.second;})->second;
        auto max_lon = max_element(stops_points.begin(), stops_points.end(), [](const auto& lhs, const auto& rhs) {return lhs.second < rhs.second;})->second;
        double zoom_coef = 0;
        double EPS = 1e-9;
        if (abs(max_lon - min_lon) > EPS) {
            zoom_coef = (RenderSettings_.width - 2 * RenderSettings_.padding) / (max_lon - min_lon);
        }
        if (abs(max_lat - min_lat) > EPS) {
            auto height_zoom_coef = (RenderSettings_.height - 2 * RenderSettings_.padding) / (max_lat - min_lat);
            if (zoom_coef == 0 || zoom_coef > height_zoom_coef) {
                zoom_coef = height_zoom_coef;
            }
        }
        return MapInfo{ min_lat, max_lat, min_lon, max_lon, zoom_coef };
        */
        return map_info;
    }

    void AddPolylinesToSvg(MapInfo map_info, Svg::Document& svg_doc) {
        using namespace Svg;
        size_t cur_color_idx = 0;
        for (const auto& [bus_name, bus]: Buses) {
            Polyline line = Polyline{}
                .SetStrokeColor(RenderSettings_.color_palette[cur_color_idx])
                .SetStrokeWidth(RenderSettings_.line_width)
                .SetStrokeLineCap("round")
                .SetStrokeLineJoin("round");
            vector<pair<double, double>> line_points;
            for (const auto stop_name: bus.Stops) {
                const auto& stop = Stops[stop_name];
                line_points.push_back({
                    map_info.lat_to_image_coor[stop.StopLocation.Latitude], 
                    map_info.lon_to_image_coor[stop.StopLocation.Longitude]
                });
            }
            for (const auto& pt: line_points) {
                line.AddPoint({ pt.second, pt.first });
                //line.AddPoint({(pt.second - map_info.min_lon) * map_info.zoom_coef + RenderSettings_.padding, 
                //    (map_info.max_lat - pt.first) * map_info.zoom_coef + RenderSettings_.padding});
            }
            cur_color_idx = (cur_color_idx + 1) % RenderSettings_.color_palette.size();
            svg_doc.Add(line);
        } 
    }

    void AddBusesNamesToSvg(MapInfo map_info, Svg::Document& svg_doc) {
        using namespace Svg;
        size_t cur_color_idx = 0;
        for (auto iter = Buses.begin(); iter != Buses.end(); ++iter) {
            string bus_name = iter->first;
            const auto& stops = iter->second.Stops;
            auto add_stop = [&](const string& stop_name) {
                const auto& stop = Stops.at(stop_name);
                Point coords = Point{ map_info.lon_to_image_coor[stop.StopLocation.Longitude], map_info.lat_to_image_coor[stop.StopLocation.Latitude] }; 
                /*
                Point coords = Point{
                    (stop.StopLocation.Longitude- map_info.min_lon) * map_info.zoom_coef + RenderSettings_.padding, 
                    (map_info.max_lat - stop.StopLocation.Latitude) * map_info.zoom_coef + RenderSettings_.padding
                };
                */
                auto base_settings = Text{}
                    .SetPoint(coords)
                    .SetOffset(RenderSettings_.bus_label_offset)
                    .SetFontSize(RenderSettings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(bus_name);

                auto underlayer = base_settings;
                underlayer
                    .SetFillColor(RenderSettings_.underlayer_color)
                    .SetStrokeColor(RenderSettings_.underlayer_color)
                    .SetStrokeWidth(RenderSettings_.underlayer_width)
                    .SetStrokeLineCap("round")
                    .SetStrokeLineJoin("round");

                auto main_text = base_settings;
                main_text
                    .SetFillColor(RenderSettings_.color_palette[cur_color_idx]);
                
                svg_doc.Add(underlayer);
                svg_doc.Add(main_text);
            };
            add_stop(stops[0]);
            if (!(iter->second.IsRoundTrip) && stops[0] != stops[stops.size() / 2]) {
                add_stop(stops[stops.size() / 2]);
            }
            cur_color_idx = (cur_color_idx + 1) % RenderSettings_.color_palette.size();
        }
    }

    void AddStopCirclesToSvg(MapInfo map_info, Svg::Document& svg_doc) {
        using namespace Svg;
        for (const auto& [stop_name, stop]: Stops) {
            Point coords = Point{ map_info.lon_to_image_coor[stop.StopLocation.Longitude], map_info.lat_to_image_coor[stop.StopLocation.Latitude] }; 
            /*
            Point coords = Point{
                (stop.StopLocation.Longitude - map_info.min_lon) * map_info.zoom_coef + RenderSettings_.padding, 
                (map_info.max_lat - stop.StopLocation.Latitude) * map_info.zoom_coef + RenderSettings_.padding
            };
            */
            auto circle = Circle{}
                .SetCenter(coords)
                .SetRadius(RenderSettings_.stop_radius)
                .SetFillColor("white");
            svg_doc.Add(circle);
        }
    }
    
    void AddStopNamesToSvg(MapInfo map_info, Svg::Document& svg_doc) {
        using namespace Svg;
        for (const auto& [stop_name, stop]: Stops) {
            Point coords = Point{ map_info.lon_to_image_coor[stop.StopLocation.Longitude], map_info.lat_to_image_coor[stop.StopLocation.Latitude] }; 
            /*
            Point coords = Point{
                (stop.StopLocation.Longitude - map_info.min_lon) * map_info.zoom_coef + RenderSettings_.padding, 
                (map_info.max_lat - stop.StopLocation.Latitude) * map_info.zoom_coef + RenderSettings_.padding
            };
            */
            auto base_sets = Text{}
                .SetPoint(coords)
                .SetOffset(RenderSettings_.stop_label_offset)
                .SetFontSize(RenderSettings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(stop_name);

            auto underlayer = base_sets;
            underlayer
                .SetFillColor(RenderSettings_.underlayer_color)
                .SetStrokeColor(RenderSettings_.underlayer_color)
                .SetStrokeWidth(RenderSettings_.underlayer_width)
                .SetStrokeLineCap("round")
                .SetStrokeLineJoin("round");

            auto main_text = base_sets;
            main_text
                .SetFillColor("black");
            
            svg_doc.Add(underlayer);
            svg_doc.Add(main_text);
        }
    }

    struct EdgeInfo {
        double Weight;
        string StopFrom;
        string StopTo;
        string BusName;
        size_t EdgeId;
        int SpanCount;
    };

    vector<EdgeInfo> Edges;
    unordered_map<string, size_t> StopIdByName;
    unique_ptr<Graph::Router<double>> RouteBuilder;
    shared_ptr<Graph::DirectedWeightedGraph<double>> GraphPtr;

    map<string, Stop> Stops;
    map<string, Bus> Buses;
    unordered_map<string, unordered_map<string, double>> DistancesBetweenStops;
    BusManagerSettings BusManagerSettings_;
    RenderSettings RenderSettings_;
};
