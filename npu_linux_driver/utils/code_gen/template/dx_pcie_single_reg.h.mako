/* Automatically generated files */
/* registers are not duplicated and register unit : 32bits */

#ifndef __PCIE_${base_name}_H
#define __PCIE_${base_name}_H
<%
    if "AXI" in base_name.upper():
        base_offset = 0x2000000
    else:
        base_offset = 0x200000
%>[NEWLINE]
/* Address */
#define PCIE0_${base_name}_BASE     ${hex(base_addr)}
#define PCIE1_${base_name}_BASE     ${hex(base_addr+base_offset)}

/* Register Definition */
% for module, reg in datas.items():
typedef volatile union _${module}_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
    % for idx in range(0, len(reg.name)):
        volatile unsigned int ${reg.name[idx]}:${reg.size[idx]};
        /* Default Value : ${hex(reg.default[idx]) if type(reg.default[idx]) == int else reg.default[idx]}, TYPE : ${reg.type[idx]}, Position : ${reg.bits[idx]} */
        /* Description :
        ${reg.desc[idx]}
        */
    % endfor
    };
} ${module}_t;

% endfor

/* PCIe ${base_name} Registers */
<% offset_dummy, reserved_size =0, 0 %>[NEWLINE]
typedef volatile struct _pcie_${base_name.lower()}_reg_t
{
% for module, reg in datas.items():
    % if offset_dummy != reg.offs: ## Print Reserved Register
<% reserved_size = int((reg.offs-offset_dummy)/4) %>[NEWLINE]
    volatile int reserved_${hex(offset_dummy)}[${reserved_size}];
<% offset_dummy += (4*reserved_size) %>[NEWLINE]
    % endif
    ${module}_t ${module}; /* ${hex(reg.offs)} */
<% offset_dummy += 4 %>[NEWLINE]
% endfor
} pcie_${base_name.lower()}_reg_t;

#endif /* __PCIE_${base_name}_H */