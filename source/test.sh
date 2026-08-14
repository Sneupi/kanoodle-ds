test_file=kanoodle_test
cat > "$test_file.c" <<'EOF'
#include "kanoodle.h"
#include <stdio.h>
void main() 
{ 
    Polyomino sol[12];
    int size = random_solution(sol);
    print_solution(sol, size);
}
EOF
gcc -g polyomino.c kanoodle.h "$test_file.c" -o "$test_file"
time -p valgrind --leak-check=full --track-origins=yes "./$test_file"
rm $test_file*