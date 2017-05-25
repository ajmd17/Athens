## Syntax

### Statements

Statements in Ace do not require semicolons. 
Don't use them unless you have to keep multiple statements on the same line (in which case, you should consider changing the code to be clearer anyway).

#### Modules

Currently, all Ace code files must contain at least one `module` declaration.
All statements and declarations in your files must be nested within the curly braces of a `module`.
Example:

```
module ace_example {
    // all statements within here
}
```

This `module` declaration will allow you to use your code from other files, using the syntax: `ace_example::some_global_variable`.

Modules can also be nested within other modules
Example:
```
module ace_example {
    // statements are also allowed in here

    module hidden_details {
        // statements can in here
    }
}
```

#### Imports

In order to use `module`s from other `module`s, use the `import` syntax with the module name after it.
Assuming you are currently writing in a separate file and want to import the module `ace_example` from a file called `ace_example.ace`, you would use this syntax:
```
import ace_example
```

To import a nested module within the `ace_example` module (such as the `hidden_details` module from above), you
would use this syntax:
```
import ace_example::hidden_details
```

This will allow you to only write out `hidden_details` rather than writing out `ace_example::hidden_details` anytime you wish to use something from that module.

You can also import multiple nested modules at the same time:
```
import ace_example::{hidden_details, some_other_module}
```

If the file you wish to import is located in a separate folder than the current file, you may add a library search path using the `use library` syntax.

Assuming you want to be able to import modules from a directory above the current directory, you would use this syntax:
```
use library ['../']
```

Because the library search paths use an array, this means you can add multiple search paths at once.
Example
```
use library ['../', '../acelib', './some_library_folder']
```

#### Hello World

Now that we've created our module 'ace_example', let's make it print something to the screen.
We can print something using the `print` statement, like this:

```
print 'Hello, world!'
```

Notice that we placed `'Hello, world!'` in single quotes. 
In Ace, strings can be typed with either single quotes or double quotes.

The print statement can also take multiple arguments, using commas:

```
print 'Hello ', 'World'
```

#### Variables

Variables can be declared within a module, function, or any other block of code.
Examples:
* Assuming the variable `x` has not been declared, `x = 5` will create a variable named `x` with a value of `5`. The type of `x` will be `Int`, which cannot be changed to a different type once it is created.
* You can explicitly state a type when declaring a variable. If you would want `x` to be compatible with both the `Int` type as well as the `Float` type, you could declare `x` using this syntax: `x: Number = 5`.
* Similarly, `x: Any = 5` will declare a variable named `x` with the value `5`, however `x` will be assignable to any possible type afterwords, meaning that the code: `x = "Hello"` is perfectly valid.
* If you wish to create a new variable with a name that already exists in a parent scope, you may use the `let` keyword to explicitly tell Ace to create a new variable. This will not overwrite the already existing variable, but it will shadowed by the new variable and thus inaccessible until you are back in the parent scope.

When you create a variable, it will be set to the type of whatever you assign it to. If there is no type provided, you must manually enter the type you want it to be after the `:` character. To let a variable be able to be fluid between types, you can manually tell it to be `Any`.

Now that that's out of the way, let's see some example code!

```
b: Int       // creates 'b' with type 'Int' (will be set to 0 by default)

a = 5        // creates 'a' with type 'Int' and sets it to be 5
b = 10       // sets 'b' which was already declared to 10

print a      // prints out the number 5
print a + b  // prints out the number 15

// a = "Hello"    ERROR: cannot change the type from 'Int' to 'String'
c: Any = 100  // creates 'c' and sets it to be 100

print c       // prints out the number 100

c = "Hello, again"

print c       // prints 'Hello, again' onto the screen
```

#### Functions

General explanation on functions: http://www.webopedia.com/TERM/F/function.html

Functions are written using a pair of parentheses which may optionally contain parameters to be passed when the function is called. After the parentheses, the function's statements are written within a set of curly braces.

In Ace, functions are values themseleves. A variable may be assigned to a function in the same way that a variable may be assigned to a number or string.

Example:

```
return_2_plus_2 = () {
    return 2 + 2
}

print return_2_plus_2() // prints out 4
```

Example (with arguments)
```
return_a_plus_2 = (a: Number) {
    return a + 2
}

print return_a_plus_2(10) // prints 12
print return_a_plus_2(16) // prints 18
```

#### Custom Types

Similar to many other languages, you can create your own custom, reusable types with whatever fields and methods you choose.

To create a new type, use the `type` keyword.
Example:

```
type Person {
    name: String

    say_hello = (self) {
        print fmt("% says hello!", self.name)
    }

    say_something = (self, what_to_say) {
        print fmt("% says: '%'", self.name, what_to_say)
    }
}

person: Person
person.name = "Banana Man"
person.say_hello() // Banana Man says hello!
//              ^ Note that the 'person' variable gets passed in as the 'self' argument.

person.say_something("I'm giving up on you") // Banana Man says: 'I'm giving up on you'

```