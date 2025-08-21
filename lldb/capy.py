def capy_vec(root_val, internal_dict):
  size = root_val.GetChildMemberWithName('size').GetValueAsSigned(0)
  capacity = root_val.GetChildMemberWithName('capacity').GetValueAsSigned(0)
  
  return 'size={} capacity={}'.format(size, capacity)


def capy_string(root_val, internal_dict):
  size_val = root_val.GetChildMemberWithName('size')
  data_val = root_val.GetChildMemberWithName('data')

  size = size_val.GetValueAsSigned(0)
  data = bytearray(data_val.GetPointeeData(item_count=size).sint8s).decode()
  
  return '"{}"'.format(data)
