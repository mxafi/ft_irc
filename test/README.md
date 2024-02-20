# Unit testing this project with catch2

## Before you start

- Read https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md
- Look at existing tests, for example: ```test/server/testServer.cpp```

## Basic steps and style

1. Create a folder for your module under the test folder. (e.g. test/mymodule)
2. Create ```test*.cpp``` files where ```*``` describes the tests in the file. (e.g. test/server/testServer.cpp)
3. Start your test files with ```#include "../catch2/catch_amalgamated.hpp"``` and include your module header after it.
4. Write your tests: Use ```TEST_CASE("description", "[tag]")``` format, where ```tag``` is the module name.
5. Execute all tests with ```make test```

>You can add arguments to the tester, for example: \
```make test ARGS="--rng-seed 42 --allow-running-no-tests"```
