import os
import pycriu


class CriuImgFileReader:
    def __init__(self, dir_img_path):
        self.dir_img_path = dir_img_path

    def open_file(self, file_name, errors_list):
        try:
            img_file = open(file_name, 'r')
            return img_file
        except IOError as exc:
            errors_list.add_context(file_name)
            errors_list.add_error(exc.strerror)
            errors_list.pop_context()
            return None

    def close_file(self, img_file):
        img_file.close()

    def read_img(self, file_name, errors_list):
        path = os.path.join(self.dir_img_path, file_name)
        img_file = self.open_file(path, errors_list)

        if img_file:
            try:
                img = pycriu.images.load(img_file)
                return img
            except pycriu.images.MagicException as exc:
                errors_list.add_context(path)
                error = "unknown magic {0:#x}".format(exc.magic)
                errors_list.add_error(error)
                errors_list.pop_context()
            finally:
                self.close_file(img_file)

        return {}


class ErrorList:
    def __init__(self):
        self.context = []
        self.errors = []

    def add_context(self, context_item):
        self.context.append(context_item)

    def pop_context(self):
        self.context.pop()

    def add_error(self, error_str):  # error_str - string describing current error
        error_item = "Context: {0}, error: {1}.".format(self.context, error_str)
        self.errors.append(error_item)

    def show_errors(self, indent_len=0):
        indent = " " * indent_len
        for item in self.errors:
            print "{0}{1}".format(indent, item)


# helpers
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