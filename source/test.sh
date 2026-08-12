test_file=kanoodle_test

cat > "$test_file.c" <<'EOF'
#include <stdio.h>
#include <stdlib.h>
#include "kanoodle.h"
void main() { print_solution( random_solution() ); }
EOF

gcc -g polyomino.c kanoodle.c dlx.c "$test_file.c" -o "$test_file"

rm "$test_file.c"

time -p valgrind --leak-check=full --track-origins=yes "./$test_file"

rm "$test_file"