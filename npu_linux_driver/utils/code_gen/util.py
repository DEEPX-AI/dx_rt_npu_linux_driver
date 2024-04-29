


def convert_bit_to_pos_size_mask(bit):
    pos_s, size, mask = 0, 0, 0
    bit = bit.strip()
    # Multi bit
    bit_s = bit.find(':')
    if bit_s != -1:
        pos_s = bit[bit_s+1 : len(bit)-1]
        pos_e = bit[1 : bit_s]
        size  = int(pos_e) - int(pos_s) + 1
    else:
        pos_s = bit[1:len(bit)-1]
        size  = 1

    for idx in range(0, size):
        mask |= (0x1<<(int(pos_s)+idx))
    return pos_s, size, hex(mask)

print(convert_bit_to_pos_size_mask("[4:1]"))