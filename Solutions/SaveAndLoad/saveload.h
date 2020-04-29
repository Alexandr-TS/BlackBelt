#include <map>
#include <iostream>
#include <string>
#include <vector>

template <typename T>
void Serialize(T pod, std::ostream& out);

void Serialize(const std::string& str, std::ostream& out);

template <typename T>
void Serialize(const std::vector<T>& data, std::ostream& out);

template <typename T1, typename T2>
void Serialize(const std::map<T1, T2>& data, std::ostream& out);

template <typename T>
void Deserialize(std::istream& in, T& pod);

void Deserialize(std::istream& in, std::string& str);

template <typename T>
void Deserialize(std::istream& in, std::vector<T>& data);

template <typename T1, typename T2>
void Deserialize(std::istream& in, std::map<T1, T2>& data);


// Serialization

template <typename T>
void Serialize(T pod, std::ostream& out) {
	out.write(reinterpret_cast<const char*>(&pod), sizeof(pod));
}

template <typename T>
void SerializeSize(const T& container, std::ostream& out) {
	size_t sz = container.size();
	Serialize(sz, out);
}

void Serialize(const std::string& str, std::ostream& out) {
	SerializeSize(str, out);
	for (auto ch : str) {
		Serialize(ch, out);
	}
}

template <typename T>
void Serialize(const std::vector<T>& data, std::ostream& out) {
	SerializeSize(data, out);
	for (const auto& element : data) {
		Serialize(element, out);
	}
}

template <typename T1, typename T2>
void Serialize(const std::map<T1, T2>& data, std::ostream& out) {
	SerializeSize(data, out);
	for (const auto& [key, value] : data) {
		Serialize(key, out);
		Serialize(value, out);
	}
}


// Deserialization


template <typename T>
void Deserialize(std::istream& in, T& pod) {
	in.read(reinterpret_cast<char*>(&pod), sizeof(pod));
}

size_t DeserializeSize(std::istream& in) {
	size_t sz;
	Deserialize(in, sz);
	return sz;
}

void Deserialize(std::istream& in, std::string& str) {
	size_t sz = DeserializeSize(in);
	str.clear();
	for (size_t i = 0; i < sz; ++i) {
		char ch;
		Deserialize(in, ch);
		str += ch;
	}
}

template <typename T>
void Deserialize(std::istream& in, std::vector<T>& data) {
	size_t sz = DeserializeSize(in);
	data.resize(sz);
	for (auto& element : data) {
		Deserialize(in, element);
	}
}

template <typename T1, typename T2>
void Deserialize(std::istream& in, std::map<T1, T2>& data) {
	size_t sz = DeserializeSize(in);
	data.clear();
	for (size_t i = 0; i < sz; ++i) {
		T1 key;
		Deserialize(in, key);
		T2 value;
		Deserialize(in, value);
		data[key] = value;
	}
}
