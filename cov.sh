 gcovr --gcov-executable "llvm-cov gcov" --root . --exclude build --exclude tests --xml --cobertura coverage.xml --html --html-details --html-theme github.green --print-summary --output coverage/index.html