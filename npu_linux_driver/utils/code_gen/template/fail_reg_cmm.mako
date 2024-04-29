;;;Debug PCIE APB Register
;AREA.CLEAR REPORT
PRINT "----------------------------------------------------------------------------------------------------------"
% for module, reg in datas.items():
;=================================================================================================
; Register:${module}, Member: ${reg.name}
;=================================================================================================
    % for idx, name in enumerate(reg.name):
<% ## Parsing register name for multi-bit
    if name.find('[') != -1:
        parsed_name = name[:name.find('[')]
    else:
        parsed_name = name
%>[NEWLINE]
        % if "RESERVED" not in name:
&${parsed_name} = ${hex(base_addr).upper().replace("0X","0x")}+${hex(reg.offs)}
            % if reg.has_desc_info[idx]:
<% reg_offs, reg_offs_s, reg_offs_e = 0, 0, 0 %>[NEWLINE]
                % for dIdx, r_desc in enumerate(reg.desc[idx]):
## ;;${r_desc}
## Abnormal Case - Description (start+size>32)
                    % if (int(r_desc[0])+int(r_desc[1])) > 32:
<%
    import math
    s_pos, size, org_mask = int(r_desc[0]), int(r_desc[1]), r_desc[2]
    reg_offs_e = s_pos + size
    loop_cnt = math.ceil((s_pos%32 + size)/32)
    shift_s = (s_pos%32)
    tmp_size = size - shift_s
    if (math.ceil((s_pos+size)/32) > math.ceil(s_pos/32)) and (s_pos>0):
        ## print(f"{math.ceil((s_pos+size)/32)}, {math.ceil(s_pos/32)}")
        size += (s_pos%32)
        s_pos -= (s_pos%32)
    mod_mask = org_mask[2:(len(org_mask)-math.floor(s_pos/4))]
    ## print(f"s_pos:{s_pos}, size:{size}, loop_cnt:{loop_cnt}, org_mask:{org_mask}, mod_mask:{mod_mask}")
    mask_s, mask_e, masks, mask_size, val_pos = 0, 0, [], [], []
    if loop_cnt > 1:
        mask_s = len(mod_mask) - 8
    mask_e = len(mod_mask)
    for i in range(0, loop_cnt):
        masks.append("0x"+mod_mask[mask_s:mask_e])
        mask_size.append(shift_s)
        ## print(f"s:{mask_s}, e:{mask_e}, mask:{masks[i]}, shift_s:{mask_size[i]}")
        mask_e = mask_s
        mask_s -= 8
        if mask_s < 0: 
            mask_s = 0
        shift_s = 0
    for i in range(0, loop_cnt):
        val_pos.append(reg_offs)
        if math.floor((reg_offs_e - reg_offs_s)/32) > 0:
            reg_offs +=4
            reg_offs_s += 32

%>[NEWLINE]
                        % for i, m in enumerate(masks):
&mask_${i} = ${m}
                        %endfor
                        % for i, m in enumerate(masks):
                            % if i==0:                      ## First
                                % if len(masks) > 1:
&val_${i} = ((Data.Long(A:&${parsed_name}+${hex(val_pos[i])})&(&mask_${i}))>>${mask_size[0]}.)\
                                % else:
&val_${i} = ((Data.Long(A:&${parsed_name}+${hex(val_pos[i])})&(&mask_${i}))>>${mask_size[0]}.)
                                % endif
                            % else:
|(((Data.Long(A:&${parsed_name}+${hex(val_pos[i])})&(&mask_${i}))<<${32-mask_size[0]}.)&0xFFFFFFFF)
                                % if len(masks)-1 > i:      ## Middle
&val_${i} = ((Data.Long(A:&${parsed_name}+${hex(val_pos[i])})&(&mask_${i}))>>${mask_size[0]}.)\
                                % else:                     ## Last
## &val_${i} = ((Data.Long(A:&${parsed_name}+${hex(val_pos[i])})&(&mask_${i}))>>${mask_size[0]}.)
                                % endif
                            % endif
                        % endfor
IF \
                        % for i, m in enumerate(masks):
                            % if len(masks) == 1:       ## Fist
(&val_${i}!=0.)
                            % elif len(masks)-2 > i:    ## Middle
(&val_${i}!=0.)||\
                            % elif len(masks)-1 > i:
(&val_${i}!=0.)
                            % else:                     ## Last
## (&val_${i}!=0.)
                            % endif
                        % endfor
    PRINT "[${parsed_name}:${hex(int(reg.offs)+val_pos[0])}] V:",\
                        % for i, m in enumerate(masks):
                            % if (len(masks)==1)or(len(masks)-1 > i):      ## First, Middle
FORMAT.HEX(0.,&val_${i}),"_",\
                            % else:                     ## Last
## val_${i}
                            % endif
                        % endfor
" ${r_desc[3]}"
<%  ## Clear list
    masks.clear()
    mask_size.clear()
    val_pos.clear()
%>[NEWLINE]
                    % else:
## Normal Case - Description
<% reg_offs, reg_offs_s, reg_offs_e = 0, 0, 0 %>[NEWLINE]
&mask = ${r_desc[2]}
&val  = ((Data.Long(A:&${parsed_name})&(&mask))>>${r_desc[0]}.)
IF &val!=0.
    PRINT "[${parsed_name}:${hex(reg.offs)}] V:", &val, " ${r_desc[3]}"
                    % endif
                % endfor
            % else:
## Normal Case - Regitser
&mask = ${reg.mask[idx]}
&val  = ((Data.Long(A:&${parsed_name})&(&mask))>>${reg.bits[idx]}.)
IF &val!=0.
    PRINT "[${parsed_name}:${hex(reg.offs)}] V:", &val, " ${reg.desc[idx]}"
            % endif
        % endif
    % endfor

% endfor