def capy_vec(root_val, internal_dict):
  size = root_val.GetChildMemberWithName('size').GetValueAsSigned(0)
  capacity = root_val.GetChildMemberWithName('capacity').GetValueAsSigned(0)
  return 'size={} capacity={}'.format(size, capacity)


def capy_string(root_val, internal_dict):
  size = root_val.GetChildMemberWithName('size').GetValueAsSigned(0)
  data = root_val.GetChildMemberWithName('data').GetPointeeData(item_count=size)
  data = bytearray(data.sint8s).decode()
  return '"{}"'.format(data)
