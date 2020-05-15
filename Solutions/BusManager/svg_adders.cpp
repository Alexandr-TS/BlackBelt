#include "manager.h"

void BusManager::AddPolylinesToSvg(MapInfo map_info, Svg::Document& svg_doc) {
    using namespace Svg;
    size_t cur_color_idx = 0;
    for (const auto& [bus_name, bus] : Buses) {
        Polyline line = Polyline{}
            .SetStrokeColor(RenderSettings_.color_palette[cur_color_idx])
            .SetStrokeWidth(RenderSettings_.line_width)
            .SetStrokeLineCap("round")
            .SetStrokeLineJoin("round");
        for (const auto stop_name : bus.Stops) {
            line.AddPoint({
                map_info[stop_name].lon,
                map_info[stop_name].lat
                });
        }
        cur_color_idx = (cur_color_idx + 1) % RenderSettings_.color_palette.size();
        svg_doc.Add(line);
    }
}

void BusManager::AddBusesNamesToSvg(MapInfo map_info, Svg::Document& svg_doc) {
    using namespace Svg;
    size_t cur_color_idx = 0;
    for (auto iter = Buses.begin(); iter != Buses.end(); ++iter) {
        string bus_name = iter->first;
        const auto& stops = iter->second.Stops;
        auto add_stop = [&](const string& stop_name) {
            Point coords = Point{
                map_info[stop_name].lon,
                map_info[stop_name].lat
            };
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

void BusManager::AddStopCirclesToSvg(MapInfo map_info, Svg::Document& svg_doc) {
    using namespace Svg;
    for (const auto& [stop_name, stop] : Stops) {
        Point coords = Point{
            map_info[stop_name].lon,
            map_info[stop_name].lat
        };
        auto circle = Circle{}
            .SetCenter(coords)
            .SetRadius(RenderSettings_.stop_radius)
            .SetFillColor("white");
        svg_doc.Add(circle);
    }
}

void BusManager::AddStopNamesToSvg(MapInfo map_info, Svg::Document& svg_doc) {
    using namespace Svg;
    for (const auto& [stop_name, stop] : Stops) {
        Point coords = Point{
            map_info[stop_name].lon,
            map_info[stop_name].lat
        };
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

void BusManager::AddOpaqueRectToSvg(Svg::Document& svg_doc) {
    using namespace Svg;
    Rectangle rect;
    Point left_top = Point{
        -RenderSettings_.outer_margin,
        -RenderSettings_.outer_margin
    };
    Point right_down = Point{
        RenderSettings_.width + RenderSettings_.outer_margin,
        RenderSettings_.height + RenderSettings_.outer_margin
    };
    rect.SetPosition(left_top);
    rect.SetSize(right_down.y - left_top.y, right_down.x - left_top.x);
    rect.SetFillColor(RenderSettings_.underlayer_color);
    svg_doc.Add(rect);
}

Svg::Document BusManager::BuildMapSvgDocument(MapInfo& map_info) {
	using namespace Svg; 
	using namespace Json;

	Svg::Document svg_doc;
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
	return svg_doc;
}


void BusManager::AddPathsToSvg(MapInfo map_info, Svg::Document& svg_doc,
        Graph::Router<double>::RouteInfo& route_info) {
	using namespace Svg; 
	using namespace Json;
	for (const auto& layer: RenderSettings_.layers) {
		if (layer == "bus_lines") {
			PathAddPolylinesToSvg(map_info, svg_doc, route_info);
		} else if (layer == "bus_labels") {
			PathAddBusesNamesToSvg(map_info, svg_doc, route_info);
		} else if (layer == "stop_points") {
			PathAddStopCirclesToSvg(map_info, svg_doc, route_info); 
		} else if (layer == "stop_labels") {
			PathAddStopNamesToSvg(map_info, svg_doc, route_info);
		}
	}
}


void BusManager::PathAddPolylinesToSvg(MapInfo map_info, Svg::Document& svg_doc, Graph::Router<double>::RouteInfo& route_info) {
    using namespace Svg;

    map<string, Polyline> line_by_bus;
    size_t cur_color_idx = 0;
    for (const auto& [bus_name, bus] : Buses) {
        Polyline line = Polyline{}
            .SetStrokeColor(RenderSettings_.color_palette[cur_color_idx])
            .SetStrokeWidth(RenderSettings_.line_width)
            .SetStrokeLineCap("round")
            .SetStrokeLineJoin("round");
        line_by_bus[bus_name] = line;
        cur_color_idx = (cur_color_idx + 1) % RenderSettings_.color_palette.size();
    }

	for (size_t i = 0; i < route_info.edge_count; ++i) {
		auto edge_id = RouteBuilder->GetRouteEdge(route_info.id, i);
        Polyline line = line_by_bus[Edges[edge_id].BusName];
        line.AddPoint({
            map_info[Edges[edge_id].StopFrom].lon,
            map_info[Edges[edge_id].StopFrom].lat
        });
        line.AddPoint({
            map_info[Edges[edge_id].StopTo].lon,
            map_info[Edges[edge_id].StopTo].lat
        });
        svg_doc.Add(line);
	}
}

void BusManager::PathAddBusesNamesToSvg(MapInfo map_info, Svg::Document& svg_doc, Graph::Router<double>::RouteInfo& route_info) {

}

void BusManager::PathAddStopCirclesToSvg(MapInfo map_info, Svg::Document& svg_doc, Graph::Router<double>::RouteInfo& route_info) {

}

void BusManager::PathAddStopNamesToSvg(MapInfo map_info, Svg::Document& svg_doc, Graph::Router<double>::RouteInfo& route_info) {

}
