let edit_distance a b cutoff =
  let la, lb = String.length a, String.length b in
  let cutoff =
    (* using max_int for cutoff would cause overflows in (i + cutoff + 1);
       we bring it back to the (max la lb) worstcase *)
    min (max la lb) cutoff in
  if abs (la - lb) > cutoff then None
  else begin
    (* initialize with 'cutoff + 1' so that not-yet-written-to cases have
       the worst possible cost; this is useful when computing the cost of
       a case just at the boundary of the cutoff diagonal. *)
    let m = Array.make_matrix (la + 1) (lb + 1) (cutoff + 1) in
    m.(0).(0) <- 0;
    for i = 1 to la do
      m.(i).(0) <- i;
    done;
    for j = 1 to lb do
      m.(0).(j) <- j;
    done;
    for i = 1 to la do
      for j = max 1 (i - cutoff - 1) to min lb (i + cutoff + 1) do
        let cost = if a.[i-1] = b.[j-1] then 0 else 1 in
        let best =
          (* insert, delete or substitute *)
          min (1 + min m.(i-1).(j) m.(i).(j-1)) (m.(i-1).(j-1) + cost)
        in
        let best =
          (* swap two adjacent letters; we use "cost" again in case of
             a swap between two identical letters; this is slightly
             redundant as this is a double-substitution case, but it
             was done this way in most online implementations and
             imitation has its virtues *)
          if not (i > 1 && j > 1 && a.[i-1] = b.[j-2] && a.[i-2] = b.[j-1])
          then best
          else min best (m.(i-2).(j-2) + cost)
        in
        m.(i).(j) <- best
      done;
    done;
    let result = m.(la).(lb) in
    if result > cutoff
    then None
    else Some result
  end



let show_cutoff n =
  if n = max_int then "max_int" else Printf.sprintf "%d" n
;;

let test =
  let counter = ref 0 in
  fun a b cutoff expected ->
    let show_result = function
      | None -> "None"
      | Some d -> "Some " ^ string_of_int d in
    incr counter;
    Printf.printf "[%02d] (edit_distance %S %S %s), expected %s\n"
      !counter a b (show_cutoff cutoff) (show_result expected);
    let result = edit_distance a b cutoff in
    if result = expected
    then print_endline "OK"
    else Printf.printf "FAIL: got %s\n%!" (show_result result)

let () =
  test "a" "a" 1 (Some 0);
  test "a" "a" 0 (Some 0);
  test "a" "b" 1 (Some 1);
  test "a" "b" 0 None;
  test "add" "adad" 3 (Some 1);
  test "delete" "delte" 3 (Some 1);
  test "subst" "sabst" 3 (Some 1);
  test "swap" "sawp" 3 (Some 1);
  test "abbb" "bbba" 3 (Some 2);
  test "abbb" "bbba" 1 None;

  (* check for bugs where a small common suffix, or common prefix, is
     enough to make the distance goes down *)
  test "xyzwabc" "mnpqrabc" 10 (Some 5);
  test "abcxyzw" "abcmnpqr" 10 (Some 5);

  (* check that using "max_int" as cutoff works *)
  test "a" "a" max_int (Some 0);
  test "a" "b" max_int (Some 1);
  test "abc" "ade" max_int (Some 2);

  (* check empty strings*)
  test "" "" 3 (Some 0);
  test "" "abc" 3 (Some 3);
  test "abcd" "" 3 None;
  
  ()

