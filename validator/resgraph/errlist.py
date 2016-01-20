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
