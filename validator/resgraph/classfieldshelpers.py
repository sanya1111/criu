def get_simple_item(raw_data, item, errors_list):
    res = raw_data.get(item)
    if res is None:
        errors_list.add_error("no item " + item)
    return res


def init_items(self, items_list, raw_data, errors_list):
    for item in items_list:
        self.__dict__[item] = get_simple_item(raw_data, item, errors_list)


def init_optional_items(self, items_list, raw_data, errors_list):
    for item in items_list:
        if item in raw_data:
            self.__dict__[item] = get_simple_item(raw_data, item, errors_list)


def print_items(self, items_list, indent):
    for item in items_list:
        print "{0}{1} - {2}".format(indent, item, self.__dict__[item])


def print_optional_items(self, items_list, indent):
    for item in items_list:
        if self.__dict__[item]:
            print "{0}{1} - {2}".format(indent, item, self.__dict__[item])
