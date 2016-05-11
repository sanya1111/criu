import os
import sys
import pycriu

class CriuDumpReader:
    def __init__(self, dir_img_path):
        self.dir_img_path = dir_img_path

    def open_file(self, file_name, errors_list, burning = True):
        try:
            img_file = open(file_name, 'r')
            return img_file
        except IOError as exc:
            if burning:
                errors_list.add_context(file_name)
                errors_list.add_error(exc.strerror)
                errors_list.pop_context()
            return None

    def close_file(self, img_file):
        img_file.close()

    def read_img(self, file_name, errors_list, burning = True):
        path = os.path.join(self.dir_img_path, file_name)
        img_file = self.open_file(path, errors_list, burning)
        

        if img_file:
            try:
                img = pycriu.images.load(img_file)
                return img
            except pycriu.images.MagicException as exc:
                if burning == True:
                    errors_list.add_context(path)
                    error = "unknown magic {0:#x}".format(exc.magic)
                    errors_list.add_error(error)
                    errors_list.pop_context()
            finally:
                self.close_file(img_file)

        return {}

