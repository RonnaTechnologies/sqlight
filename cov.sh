 gcovr --gcov-executable "llvm-cov gcov" --root . --exclude build --exclude tests --xml --cobertura cobertura.xml --html --html-details --html-theme github.green --print-summary --output coverage/index.html && find . -name "*.gcda" -print0 | xargs -0