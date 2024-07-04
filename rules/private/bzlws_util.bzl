def get_full_label_string(label):
    if not label:
        return "@"

    return "@" + label.workspace_name + "//" + label.package + ":" + label.name

def bzlws_check_common_required_attrs(rule_name, name, srcs, out):
    if not name:
        fail("{} - missing name attribute".format(rule_name))

    if not srcs:
        fail("{} - missing srcs attribute".format(rule_name))

    if not out:
        fail("{} - missing out attribute".format(rule_name))

    if out.startswith("/"):
        fail("{} - out attribute cannot start with '/'".format(rule_name))
