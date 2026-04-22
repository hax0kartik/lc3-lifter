# Linker 

As the name suggests, this is a linker only targetting x86_64 for now.

## Compilation

- meson setup build
- cd build && meson compile

## Testing

- Compile any standard C/C++ hello world application by running the following:

```C
#include <stdio.h>

int main() {
    printf("Hello World\n");
    return 0;
}
```

```bash
gcc -c main.c -o main.o
```

and then pass it through this linker

```bash
./linker main.o
```