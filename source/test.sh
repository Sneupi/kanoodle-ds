test_file=kanoodle_test
cat > "$test_file.c" <<'EOF'
#include "kanoodle.h"
#include <stdio.h>
void main() 
{ 
    for (int i = 0; i < 5; i++) {
        Polyomino sol[12];
        int size = random_solution(sol);
        print_solution(sol, size);
        printf("\n");
    }
}
EOF
gcc -g polyomino.c kanoodle.c dlx.c "$test_file.c" -o "$test_file"
time -p valgrind --leak-check=full --track-origins=yes "./$test_file"
rm $test_file*