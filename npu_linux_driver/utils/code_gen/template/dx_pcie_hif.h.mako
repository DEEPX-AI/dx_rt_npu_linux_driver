/* Automatically generated files */

#ifndef __DX_PCIE_${base_name}_H
#define __DX_PCIE_${base_name}_H

/* Address */
#define PCIE0_${base_name}_BASE     ${hex(base_addr)}
#define PCIE1_${base_name}_BASE     ${hex(base_addr+0x200000)}
<%
    struct_lists = dict()   # Key : offset, Data: name
%>[NEWLINE]

/* Register Definition */
% for module, reg in datas.items():
<% struct_split, struct_cnt, struct_cnt_prev, struct_mod, bit_p, bit_s, = 0, 0, 0, 0, 0, 0 %>[NEWLINE]
/* offset : ${hex(reg.offs)} */
typedef volatile union _${module}_${struct_cnt}_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
    % for idx in range(0, len(reg.name)):
## Description Part with Register Information (Start)
        % if reg.has_desc_info[idx]:
            % for idx, desc in enumerate(reg.desc[idx]):
<%
    bit_p, bit_s = int(desc[0]), int(desc[1]) # postion, size
    struct_cnt = int((bit_p+bit_s)/32)
    struct_mod = int((bit_p+bit_s)%32)
    if (struct_cnt>struct_cnt_prev) and (struct_mod>0 or idx<len(reg.desc[0])-1):
        struct_split = 1
        rest_bit = bit_p + bit_s - struct_cnt*32
        if hex(reg.offs) not in struct_lists:
            struct_lists[hex(reg.offs)] = "".join([module, "_", str(0), "_t"])
        struct_lists[str(hex(reg.offs + struct_cnt*4))] = "".join([module, "_", str(struct_cnt), "_t"])
    else:
        struct_split = 0
        if (struct_cnt==1 and struct_mod==0) or (struct_cnt==0 and idx==len(reg.desc[0])-1):
            struct_cnt = 0
            struct_lists[hex(reg.offs)] = "".join([module, "_", str(struct_cnt), "_t"])
    struct_cnt_prev = struct_cnt
%>[NEWLINE]
                % if struct_split==0:
    ## Normal Case (Start)
        volatile unsigned int ${desc[4]}:${desc[1]};        /* [Position:${desc[0]}] */
        /* ${desc[3]} */
    ## Normal Case (End)
                % else:
    ## Split Case (Start)
        volatile unsigned int ${desc[4]}:${bit_s-rest_bit}; /* [Position:${desc[0]}] */
    };
} ${module}_${struct_cnt-1}_t;

typedef volatile union _${module}_${struct_cnt}_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        % if rest_bit != 0:
        volatile unsigned int ${desc[4]}:${rest_bit};       /* [Position:${desc[0]}] */
        % endif
        /* ${desc[3]} */
                % endif
            % endfor
<% # Clear Part
struct_cnt_prev = 0
%>[NEWLINE]
    ## Split Case (End)
## Description Part with Register Information (End)
        % else:
## No-Description Part (Start)
<%
    if hex(reg.offs) not in struct_lists:
        struct_lists[hex(reg.offs)] = "".join([module, "_", str(struct_cnt), "_t"])
%>[NEWLINE]
        volatile unsigned int ${reg.name[idx]}:${reg.size[idx]}; /* [Position:${reg.bits[idx]}] */
        /* Default Value : ${reg.default[idx]}, TYPE : ${reg.type[idx]} */
        /* Description :
        ${reg.desc[idx]}
        */
        % endif
## No-Description Part (End)
    % endfor
    };
} ${module}_${struct_cnt}_t;
% endfor

<% reserved_size, start_addr = 0, 0 %>[NEWLINE]
typedef volatile struct _pcie_hif_reg_t
{
% for addr in range(0, 0x1000, 4):
    % if hex(addr) in struct_lists:
        % if reserved_size > 0:
    volatile unsigned int rsvd_${hex(start_addr)}[${reserved_size}];
        % endif
    ${struct_lists[f"{hex(addr)}"]} ${struct_lists[f"{hex(addr)}"].rsplit('_',1)[0]}; /* ${hex(addr)} */
<% reserved_size = 0 %>[NEWLINE]
    % else:
<% 
    if reserved_size == 0: start_addr = addr
    reserved_size +=1 
%>[NEWLINE]
    % endif
% endfor
} pcie_hif_reg_t;

#endif /* __DX_PCIE_${base_name}_H */