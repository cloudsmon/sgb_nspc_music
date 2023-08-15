#!/usr/bin/python3
import struct 
import argparse



def get_nspc_data_from_gb_rom(file, nspc_start_addr, data_end_addr):
    with open(file, 'rb') as f:
        raw = f.read(32000)
        assert(len(raw) == 32000)
        end_addr = struct.unpack("h", raw[data_end_addr:data_end_addr+2])[0]
        data_len = end_addr - 0x2b00
        print(f"datalen {data_len}")
        spc_data = raw[nspc_start_addr:nspc_start_addr + data_len]
    return spc_data

def bytes_to_carr_str(arr, carr_name):
    carr = f"const unsigned char {carr_name}[] = {{\n"
    for i in range(len(arr)):
        carr += hex(arr[i]) + ","
        if i > 0 and i % 16 == 0:
            carr += "\n"
    carr += "};\n"
    return carr

def instr_change_addrs(spc_data, nspc_start_addr):
    instr_change_addrs = []
    for i in range(len(spc_data)):
        if spc_data[i] == 0xe0: #FIXME bug: $E0 could also be part of an address...
            print(f"{i}, {hex(nspc_start_addr + i)}")
            instr_change_addrs.append(nspc_start_addr + i +1)
    s = "const unsigned short nspc_instr_addr[] = {\n"
    for addr in instr_change_addrs:
        s += f"{hex(addr)},"
    s += "};\n"
    s += f"#define NSPC_FOUND_INSTR_OPCODES {len(instr_change_addrs)}\n"
    return s

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file", help="gb rom file")
    parser.add_argument("-v", "--verbose", action="store_true", help="print out generated c arr")
    parser.add_argument("-o", "--outfile", help="filename for the generated file")

    args = parser.parse_args()

    nspc_start_addr = 0x2b00
    my_data_end_addr = 0x2afe
    if args.outfile:
        outfile = args.outfile
    else:
        outfile = "nspc_song_data.h"
    carr_name = "nspc_song_data"


    nspc_data = get_nspc_data_from_gb_rom(args.file, nspc_start_addr, my_data_end_addr)

    carr_str= bytes_to_carr_str(nspc_data, carr_name)
    if args.verbose:
        print(carr_str)

    addr_carr = instr_change_addrs(nspc_data, nspc_start_addr) #application specific

    with open(outfile, "w") as f:
        f.write(carr_str)
        f.write(addr_carr)
