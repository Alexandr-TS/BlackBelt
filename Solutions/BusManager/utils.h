#pragma once

#include <string>
#include <vector>

using namespace std;

namespace StringUtils {

	void Trim(string& s) {
		while (!s.empty() && (s.back() == ' ' || s.back() == '\n')) {
			s.pop_back();
		}
		reverse(s.begin(), s.end());
		while (!s.empty() && (s.back() == ' ' || s.back() == '\n')) {
			s.pop_back();
		}
		reverse(s.begin(), s.end());
	}

	vector<string> SplitString(string& s, set<char>&& delims = {}) {
		vector<string> ret;
		string cur = "";
		for (auto ch : s) {
			if (delims.count(ch)) {
				Trim(cur);
				if (!cur.empty()) {
					ret.push_back(cur);
				}
				cur = "";
			}
			else {
				cur += ch;
			}
		}
		Trim(cur);
		if (!cur.empty()) {
			ret.push_back(cur);
		}
		return ret;
	}

}
