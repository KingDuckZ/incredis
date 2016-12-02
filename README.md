IncRedis is a C++ wrapper around the official hiredis library that aims to facilitate its use. It's intended to be easy to use and fast in performance.

Main features:

* async - communication is handled by a separate thread
* easy to use - declare your IncRedis object, connect and you're good to go!
* batch operations made easy - request a batch object, run your commands through it and let the library sort out the details
* synchronous operations still available - just run your commands on the main IncRedis object and the function will only return when the command is complete (communication still happens in a separate thread)

## Example ##

```cpp
    #include "incredis/incredis.hpp"
    #include <iostream>

    int main() {
        //Create main object and connect
        redis::IncRedis incredis("127.0.0.1", 6379);
        incredis.connect();

        //Block until connection is done (you should use
        //is_connected() to check the outcome)
        incredis.wait_for_connect();

        //Request a batch
        auto batch = incredis.make_batch();

        //Run your commands on it - feel free to flood it if needed! :)
        batch.select(2); //Choose DB 2
        batch.client_setname("IncRedisExampleCode");

        //The following line will block until all commands
        //are executed and will throw if any of the commands
        //issued on the batch failed
        batch.throw_if_failed();

        //You can still run individual commands synchronously
        incredis.set("SomeKey", "Hello IncRedis!");

        //And of course you can retrieve data as well
        auto my_value = incredis.get("SomeKey");
        if (my_value)
            std::cout << *my_value << '\n';

        //Not really required since we're about to
        //go out of scope
        incredis.disconnect();
        incredis.wait_for_disconnect();
        return 0;
    }
```

If you want to check the outcome of your commands instead of just generically throwing, you can:

```cpp
    const std::vector<Reply>& replies = batch.replies();
```

Replies will be in the same order as that of the commands that generated them.

### run() ###
Ideally all commands available in the supported version of Redis will have a corresponding C++ method, so for example you can just call set() or dbsize() and get some build-time checks for free. however, please keep in mind that this library is still at an early stage and more development is needed. If you find that the command you need is not implemented yet, you can still call the generic run() method.

```cpp
    //These lines are equivalent
    batch.select(2);
    batch.run("SELECT", 2);
```

Please refer to the [official documentation of Redis](https://redis.io/commands) for more details about the available commands.

