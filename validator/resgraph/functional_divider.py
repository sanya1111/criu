extra_mode_on = False

def enable_extra():
    global extra_mode_on
    extra_mode_on = True

def extra_check(func):
    def wrapper(*args, **kwargs):
        if extra_mode_on:
            func(*args, **kwargs)
    return wrapper