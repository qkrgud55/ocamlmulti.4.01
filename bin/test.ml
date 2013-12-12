
external test_fun : int -> int = "test_fun" "reentrant";;

print_int (test_fun 10);;
print_endline "";;
Gc.print_stat stdout;;
