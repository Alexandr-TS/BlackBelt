#pragma once

#include <cstring>
#include <vector>
#include <cstdio>
#include <sstream>
#include <functional>
#include <optional>
#include <memory>
#include <iostream>
#include <iomanip>

using namespace std;

namespace Svg {

template <typename T>
void PrintKeyValue(ostream& os, string key, T value) {
    os << fixed << setprecision(12) << key << "=\"" << value << "\" ";
}

struct Point {
    double x = 0;
    double y = 0;

    Point() {};
    Point(double x, double y)
        : x(x)
        , y(y)
    {
    }
};

struct Rgba {
    int red = 0;
    int green = 0;
    int blue = 0;
    double alpha = 0;
    
    Rgba() {}
    Rgba(int red, int green, int blue, double alpha)
        : red(red)
        , green(green)
        , blue(blue)
        , alpha(alpha)
    {
    }   

    string ToString() const {
        ostringstream os;
        os << alpha;
        return "rgba(" + to_string(red) + "," + to_string(green) + "," + to_string(blue) + "," + os.str() + ")";
    }
};

struct Rgb {
    int red = 0;
    int green = 0;
    int blue = 0;
    
    Rgb() {}
    Rgb(int red, int green, int blue)
        : red(red)
        , green(green)
        , blue(blue)
    {
    }   

    string ToString() const {
        return "rgb(" + to_string(red) + "," + to_string(green) + "," + to_string(blue) + ")";
    }
};

class Color {
public:
    Color(): Value("none") {}
    Color(const Rgb& rgb): Value(rgb.ToString()) {}
    Color(const Rgba& rgba): Value(rgba.ToString()) {}
    Color(const string& value): Value(value) {}
    Color(const char* value): Value(string(value)) {}
    
    string ToString() const {
        return Value;
    }

private:
    string Value;
};

Color NoneColor = Color();

class Figure {
public:
    void FigSetFillColor(const Color& color) {
        FillColor = color; 
    }

    void FigSetStrokeColor(const Color& color) {
        StrokeColor = color;
    }

    void FigSetStrokeWidth(double width) {
        StrokeWidth = width; 
    }

    void FigSetStrokeLineCap(const string& stroke_line_cap) {
        StrokeLineCap = stroke_line_cap; 
    }

    void FigSetStrokeLineJoin(const string& stroke_line_join) {
        StrokeLineJoin = stroke_line_join;
    }

    virtual void Render(ostream& os) {
        PrintKeyValue(os, "fill", FillColor.ToString());
        PrintKeyValue(os, "stroke", StrokeColor.ToString());
        PrintKeyValue(os, "stroke-width", StrokeWidth);
        if (StrokeLineCap) {
            PrintKeyValue(os, "stroke-linecap", *StrokeLineCap);
        }
        if (StrokeLineJoin) {
            PrintKeyValue(os, "stroke-linejoin", *StrokeLineJoin);
        }
    }

private:
    Color FillColor = NoneColor;
    Color StrokeColor = NoneColor;
    double StrokeWidth = 1.0;
    optional<string> StrokeLineCap = nullopt;
    optional<string> StrokeLineJoin = nullopt;
};

class Circle: public Figure {
public:
    Circle& SetCenter(Point point) {
        Center = point;
        return *this;
    }
    
    Circle& SetRadius(double radius) {
        Radius = radius;
        return *this;
    }

    void Render(ostream& os) override {
        os << "<circle ";
        Figure::Render(os);
        PrintKeyValue(os, "cx", Center.x);
        PrintKeyValue(os, "cy", Center.y);
        PrintKeyValue(os, "r", Radius);
        os << "/>";
    }
    
    Circle& SetFillColor(const Color& color) {
        FigSetFillColor(color);
        return *this;
    }

    Circle& SetStrokeColor(const Color& color) {
        FigSetStrokeColor(color);
        return *this;
    }

    Circle& SetStrokeWidth(double width) {
        FigSetStrokeWidth(width);
        return *this;
    }

    Circle& SetStrokeLineCap(const string& stroke_line_cap) {
        FigSetStrokeLineCap(stroke_line_cap);
        return *this;
    }

    Circle& SetStrokeLineJoin(const string& stroke_line_join) {
        FigSetStrokeLineJoin(stroke_line_join);
        return *this;
    }

private:
    Point Center = {0, 0};
    double Radius = 1.0;
};

class Polyline: public Figure {
public:
    Polyline& AddPoint(Point point) {
        Points.push_back(point); 
        return *this;
    }

    void Render(ostream& os) override {
        os << "<polyline ";
        Figure::Render(os);
        os << "points=\"";
        for (const auto& p: Points) {
            os << p.x << "," << p.y << " ";
        }
        os << "\" ";
        os << "/>";
    }

    Polyline& SetFillColor(const Color& color) {
        FigSetFillColor(color);
        return *this;
    }

    Polyline& SetStrokeColor(const Color& color) {
        FigSetStrokeColor(color);
        return *this;
    }

    Polyline& SetStrokeWidth(double width) {
        FigSetStrokeWidth(width);
        return *this;
    }

    Polyline& SetStrokeLineCap(const string& stroke_line_cap) {
        FigSetStrokeLineCap(stroke_line_cap);
        return *this;
    }

    Polyline& SetStrokeLineJoin(const string& stroke_line_join) {
        FigSetStrokeLineJoin(stroke_line_join);
        return *this;
    }

private:
    vector<Point> Points;
};

class Text: public Figure {
public:
    Text& SetPoint(Point point) {
        Coords = point; 
        return *this;
    }
    
    Text& SetOffset(Point point) {
        Offset = point; 
        return *this;
    }

    Text& SetFontSize(uint32_t sz) {
        FontSize = sz;
        return *this;
    } 

    Text& SetFontWeight(const string& weight) {
        FontWeight = weight;
        return *this;
    }

    Text& SetFontFamily(const string& font) {
        FontFamily = font;
        return *this;
    }

    Text& SetData(const string& data) {
        Data = data;
        return *this;
    }

    void Render(ostream& os) override {
        os << "<text ";
        Figure::Render(os);
        PrintKeyValue(os, "x", Coords.x);
        PrintKeyValue(os, "y", Coords.y);
        PrintKeyValue(os, "dx", Offset.x);
        PrintKeyValue(os, "dy", Offset.y);
        PrintKeyValue(os, "font-size", FontSize);
        if (FontFamily) {
            PrintKeyValue(os, "font-family", *FontFamily);
        } 
        if (FontWeight) {
            PrintKeyValue(os, "font-weight", *FontWeight);
        } 
        os << ">" << Data << "</text>";
    }

    Text& SetFillColor(const Color& color) {
        FigSetFillColor(color);
        return *this;
    }
    
    Text& SetStrokeColor(const Color& color) {
        FigSetStrokeColor(color);
        return *this;
    }

    Text& SetStrokeWidth(double width) {
        FigSetStrokeWidth(width);
        return *this;
    }

    Text& SetStrokeLineCap(const string& stroke_line_cap) {
        FigSetStrokeLineCap(stroke_line_cap);
        return *this;
    }

    Text& SetStrokeLineJoin(const string& stroke_line_join) {
        FigSetStrokeLineJoin(stroke_line_join);
        return *this;
    }

private:
    Point Coords = {0.0, 0.0};
    Point Offset = {0.0, 0.0};
    uint32_t FontSize = 1;
    optional<string> FontFamily = nullopt;
    optional<string> FontWeight = nullopt;
    string Data = "";
};

class Document {
public:
    Document() {}

    void Add(Circle circle) {
        Figures.push_back(make_unique<Circle>(circle));
    }
    void Add(Polyline polyline) {
        Figures.push_back(make_unique<Polyline>(polyline));
    }
    void Add(Text text) {
        Figures.push_back(make_unique<Text>(text));
    }

    void Render(ostream& os) {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
        os << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">";
        for (const auto& el: Figures) {
            el->Render(os);
        }
        os << "</svg>";
    }

private:
    vector<unique_ptr<Figure>> Figures;
};

} // namespace

