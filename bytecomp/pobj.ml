
let tag_shift = 4

let lazy_tag     = 246 - tag_shift
let closure_tag  = 247 - tag_shift
let object_tag   = 248 - tag_shift
let infix_tag    = 249 - tag_shift
let forward_tag  = 250 - tag_shift
                     
let no_scan_tag  = 251 - tag_shift
let max_tag      = 245

let abstract_tag = 251
let string_tag   = 252
let double_tag   = 253
let double_array_tag = 254
let custom_tag   = 255
let final_tag    = custom_tag


let int_tag         = 1000
let out_of_heap_tag = 1001
let unaligned_tag   = 1002
