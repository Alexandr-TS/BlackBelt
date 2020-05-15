#pragma once

#include "json.h"
#include "router.h"
#include "svg.h"
#include "responses.h"

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
#include <functional>
#include <set>

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
        outer_margin = node.at("outer_margin").AsDouble();
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
    double outer_margin;
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
        using namespace Svg;

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


        auto map_info = ComputeMapInfo();
        Svg::Document svg_doc = BuildMapSvgDocument(map_info);
        AddOpaqueRectToSvg(svg_doc);
        AddPathsToSvg(map_info, svg_doc, result);

        stringstream ss;
        svg_doc.Render(ss);
        auto raw_text = ss.str();

        /*
        std::ofstream fout;
        fout.open("C:\\Users\\Admin\\source\\repos\\BlackBelt\\Solutions\\BusManager\\map.svg");
        fout << raw_text << endl;
        fout.close();
        */

        string added_slashes = "";
        for (const auto ch: raw_text) {
            if (ch == '\"') {
                added_slashes += '\\'; 
            }
            added_slashes += ch;
        }

        node_map["map"] = Node(added_slashes);
        return RouteInfoResponse(Node(node_map));
    }


    MapInfoResponse GetMapInfoResponse() {
        using namespace Svg; 
        using namespace Json;

        auto map_info = ComputeMapInfo();
        Svg::Document svg_doc = BuildMapSvgDocument(map_info);
        stringstream ss;
        svg_doc.Render(ss);
        auto raw_text = ss.str();

        /*
        std::ofstream fout;
        fout.open("C:\\Users\\Admin\\source\\repos\\BlackBelt\\Solutions\\BusManager\\map.svg");
        fout << raw_text << endl;
        fout.close();
        */

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
	struct StopInfo {
		double lat;
		double lon;
		string name;
	};

    using MapInfo = map<string, StopInfo>;

    vector<int> GetIdsAfterCompress(
        vector<StopInfo>& stops_points, 
        set<pair<string, string>>& neighbour_stops
    ) {
        vector<int> id_after_compress(stops_points.size(), -1);
        for (size_t i = 0; i < stops_points.size(); ++i) {
            int max_neighbour_id = -1;
            for (size_t j = 0; j < i; ++j) {
                if (neighbour_stops.count({ stops_points[i].name, stops_points[j].name })) {
                    max_neighbour_id = max(max_neighbour_id, id_after_compress[j]);
                }
            }
            id_after_compress[i] = max_neighbour_id + 1;
        }
        return id_after_compress;
    }

    void DistributeUniformly(vector<StopInfo>& stops_points, set<string>& pivot_stops) {
        map<string, pair<double, double>> lat_lon_by_name;
        for (auto& stop : stops_points) {
            lat_lon_by_name[stop.name] = { stop.lat, stop.lon };
        }

        for (auto& [bus_name, bus] : Buses) {
            auto stops = bus.Stops;
            vector<int> pivot_ids;
            for (size_t i = 0; i < stops.size(); ++i) {
                if (pivot_stops.count(stops[i])) {
                    pivot_ids.push_back(i);
                }
            }

            size_t cur_pivot_id = 0;
            for (size_t i = 0; i < stops.size(); ++i) {
                if (pivot_stops.count(stops[i])) {
                    cur_pivot_id++;
                    continue;
                }
                // ids of 2 pivots in stops
                size_t l = pivot_ids[cur_pivot_id - 1];
                size_t r = pivot_ids[cur_pivot_id];
                assert(l < i&& i < r);
                double lat_step = 
                    (lat_lon_by_name[stops[r]].first - lat_lon_by_name[stops[l]].first) / (r - l);
                double lon_step = 
                    (lat_lon_by_name[stops[r]].second - lat_lon_by_name[stops[l]].second) / (r - l);
                lat_lon_by_name[stops[i]] = { 
                    lat_lon_by_name[stops[l]].first + lat_step * (i - l), 
                    lat_lon_by_name[stops[l]].second + lon_step * (i - l) 
                };
            }
        }

        for (auto& stop : stops_points) {
            auto it = lat_lon_by_name.find(stop.name);
            stop.lat = it->second.first;
            stop.lon = it->second.second;
        }
    }

    MapInfo ComputeMapInfo() {
        vector<StopInfo> stops_points;
        for (const auto& [stop_name, stop]: Stops) {
            stops_points.push_back({
                stop.StopLocation.Latitude, 
                stop.StopLocation.Longitude, 
                stop_name
            });
        }

        MapInfo map_info;

        // prepare neigbour_stops graph and pivot_stops set
        set<pair<string, string>> neighbour_stops;
        set<string> pivot_stops; // endpoints and transfer stops
        map<string, int> buses_cnt_by_stop_name; 
        for (auto& [bus_name, bus] : Buses) {
            pivot_stops.insert(bus.Stops.back());
            pivot_stops.insert(bus.Stops[0]);
            if (!bus.IsRoundTrip) {
                pivot_stops.insert(bus.Stops[bus.Stops.size() / 2]);
            }
            map<string, int> stops_set;
            for (auto& stop : bus.Stops) {
                stops_set[stop]++;
                if (stops_set[stop] > 2) {
                    pivot_stops.insert(stop);
                }
                else if (stops_set[stop] == 1) { // first occurence
                    buses_cnt_by_stop_name[stop]++;
                    if (buses_cnt_by_stop_name[stop] >= 2) {
                        pivot_stops.insert(stop);
                    }
                }
            }
            for (size_t i = 0; i + 1 < bus.Stops.size(); ++i) {
                neighbour_stops.insert({ bus.Stops[i], bus.Stops[i + 1] });
                neighbour_stops.insert({ bus.Stops[i + 1], bus.Stops[i] });
            }
        }

        for (auto& [stop_name, stop] : Stops) {
            if (!buses_cnt_by_stop_name.count(stop_name)) {
                pivot_stops.insert(stop_name);
            }
        }

        DistributeUniformly(stops_points, pivot_stops);

        // Longitude
        sort(stops_points.begin(), stops_points.end(), 
            [](const auto& lhs, const auto& rhs) {return lhs.lon < rhs.lon; });
        auto id_after_compress = GetIdsAfterCompress(stops_points, neighbour_stops);
        double step_lon_coor = (RenderSettings_.width - 2 * RenderSettings_.padding) / 
            (max(1, *max_element(id_after_compress.begin(), id_after_compress.end())));
        for (size_t i = 0; i < stops_points.size(); ++i) {
            map_info[stops_points[i].name] = { 
                0, 
                RenderSettings_.padding + step_lon_coor * id_after_compress[i], 
                stops_points[i].name 
            };
        }
        
        // Latitude
        sort(stops_points.begin(), stops_points.end(), 
            [](const auto& lhs, const auto& rhs) {return lhs.lat < rhs.lat; });
        id_after_compress = GetIdsAfterCompress(stops_points, neighbour_stops);

        double step_lat_coor = (RenderSettings_.height - 2 * RenderSettings_.padding) / 
            (max(1, *max_element(id_after_compress.begin(), id_after_compress.end())));
        for (size_t i = 0; i < stops_points.size(); ++i) {
            map_info[stops_points[i].name].lat = 
                RenderSettings_.height - RenderSettings_.padding - step_lat_coor * id_after_compress[i];
        }
        return map_info;
    }

    Svg::Document BuildMapSvgDocument(MapInfo& map_info);
    void AddPolylinesToSvg(MapInfo map_info, Svg::Document& svg_doc);
    void AddBusesNamesToSvg(MapInfo map_info, Svg::Document& svg_doc);
    void AddStopCirclesToSvg(MapInfo map_info, Svg::Document& svg_doc);
    void AddStopNamesToSvg(MapInfo map_info, Svg::Document& svg_doc);
    void AddOpaqueRectToSvg(Svg::Document& svg_doc);

    void AddPathsToSvg(MapInfo map_info, Svg::Document& svg_doc, Graph::Router<double>::RouteInfo& route_info);
    void PathAddPolylinesToSvg(MapInfo map_info, Svg::Document& svg_doc, Graph::Router<double>::RouteInfo& route_info);
    void PathAddBusesNamesToSvg(MapInfo map_info, Svg::Document& svg_doc, Graph::Router<double>::RouteInfo& route_info);
    void PathAddStopCirclesToSvg(MapInfo map_info, Svg::Document& svg_doc, Graph::Router<double>::RouteInfo& route_info);
    void PathAddStopNamesToSvg(MapInfo map_info, Svg::Document& svg_doc, Graph::Router<double>::RouteInfo& route_info);

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
