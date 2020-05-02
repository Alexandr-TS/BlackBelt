#include "person.pb.h"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <stack>
#include <set>
#include <string>

using namespace std;

int main() {
	Person p;
	p.set_age(12);
	cout << p.age() << " " << p.has_address() << endl;
}
