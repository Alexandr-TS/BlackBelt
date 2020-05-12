#include "json.h"
#include <cassert>
#include <iomanip>
#include <cmath>

using namespace std;

namespace Json {
    Document::Document(Node root) : root(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root;
    }
        
    Node LoadNode(istream& input);

    Node LoadArray(istream& input) {
        vector<Node> result;

        for (char c; input >> c && c != ']'; ) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadDouble(istream& input) {
        double result = 0;
        if (input.peek() == 't') {
            string value;
            for (int i = 0; i < 4; ++i) {
                value += static_cast<char>(input.get());
            }
            assert(value == "true");
            return Node(1.0);
        }

        if (input.peek() == 'f') {
            string value;
            for (int i = 0; i < 5; ++i) {
                value += static_cast<char>(input.get());
            }
            assert(value == "false");
            return Node(0.0);
        }

        double sign = 1.0;
        if (input.peek() == '-') {
            input.get();
            sign = -1.0;
        }

        while (isdigit(input.peek())) {
            result *= 10;
            result += static_cast<double>(input.get() - static_cast<long long>('0'));
        }

        if (input.peek() == '.') {
            double coef = 1.0;
            input.get();
			while (isdigit(input.peek())) {
                coef /= 10;
                result += coef * static_cast<double>(input.get() - static_cast<long long>('0'));
			}
        }

        result *= sign;

        return Node(result);
    }

    Node LoadString(istream& input) {
        string line;
        getline(input, line, '"');
        return Node(move(line));
    }

    Node LoadDict(istream& input) {
        map<string, Node> result;

        for (char c; input >> c && c != '}'; ) {
            if (c == ',') {
                input >> c;
            }

            string key = LoadString(input).AsString();
            input >> c;
            result.emplace(move(key), LoadNode(input));
        }

        return Node(move(result));
    }

    Node LoadNode(istream& input) {
        char c;
        input >> c;

        if (c == '[') {
            return LoadArray(input);
        }
        else if (c == '{') {
            return LoadDict(input);
        }
        else if (c == '"') {
            return LoadString(input);
        }
        else {
            input.putback(c);
            return LoadDouble(input);
        }
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Node::Print(ostream& os) const {
        if (holds_alternative<double>(*this)) {
            double value = get<double>(*this);
            if (abs(static_cast<int>(value) - value) < 1e-8) {
                os << static_cast<int>(round(value));
            }
            else {
                os << fixed << setprecision(6) << get<double>(*this);
            }
        }
        else if (holds_alternative<string>(*this)) {
            os << "\"" << get<string>(*this) << "\"";
        }
        else if (holds_alternative<vector<Node>>(*this)) {
            os << "[\n";
            for (size_t i = 0; i < (*this).AsArray().size(); ++i) {
                (*this).AsArray()[i].Print(os);
                if (i + 1 < (*this).AsArray().size()) {
                    os << ",";
                }
                os << "\n";
            }
            os << "]";
        }
        else if (holds_alternative<map<string, Node>>(*this)) {
            os << "{\n";
            const auto& cur_map = (*this).AsMap();
            for (auto it = cur_map.cbegin(); it != cur_map.cend(); ++it) {
                os << "\"" << it->first << "\": ";
                (it->second).Print(os);
                if (next(it) != cur_map.cend()) {
                    os << ",";
                }
                os << "\n";
            }
            os << "}";
        }
    }
}
