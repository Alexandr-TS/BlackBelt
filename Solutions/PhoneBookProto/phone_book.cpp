#include "phone_book.h"

PhoneBook::PhoneBook(std::vector<Contact> contacts) 
	: List(contacts) {
	std::sort(List.begin(), List.end(), [](const Contact& lhs, const Contact& rhs) {
		return lhs.name < rhs.name;
	});
}

PhoneBook::ContactRange PhoneBook::FindByNamePrefix(std::string_view name_prefix) const {
	Contact tmp{ std::string(name_prefix), std::nullopt, {} };
	auto it_left = std::lower_bound(List.begin(), List.end(), tmp, [](
		const Contact& lhs, const Contact& rhs) {
			return lhs.name < rhs.name;
		});
	tmp.name += "~";
	auto it_right = std::upper_bound(List.begin(), List.end(), tmp, [](
		const Contact& lhs, const Contact& rhs) {
			return lhs.name < rhs.name;
		});
	return ContactRange(it_left, it_right);
}

void PhoneBook::SaveTo(std::ostream& output) const {
	PhoneBookSerialize::ContactList list;
	for (const auto& contact : List) {
		PhoneBookSerialize::Contact c;
		c.set_name(contact.name);
		if (contact.birthday) {
			PhoneBookSerialize::Date d;
			d.set_year(contact.birthday->year);
			d.set_month(contact.birthday->month);
			d.set_day(contact.birthday->day);
			*c.mutable_birthday() = d;
		}
		for (const auto& phone : contact.phones) {
			c.add_phone_number(phone.c_str());
		}
		auto ptr = list.add_contact();
		*ptr = c;
	}

	list.SerializeToOstream(&output);
}

PhoneBook DeserializePhoneBook(std::istream& input) {
    PhoneBookSerialize::ContactList list;
    list.ParseFromIstream(&input);
    std::vector<Contact> v;
    for (int i = 0; i < list.contact_size(); ++i) {
        auto contact = list.contact(i);
        Contact new_c{ "", std::nullopt, {} };
        new_c.name = contact.name();
        if (contact.has_birthday()) {
            new_c.birthday = { contact.birthday().year(), contact.birthday().month(), contact.birthday().day() };
        }
        for (int j = 0; j < contact.phone_number_size(); ++j) {
            new_c.phones.push_back(contact.phone_number(j));
        }
        v.push_back(new_c);
    }
    return PhoneBook(v);
}

