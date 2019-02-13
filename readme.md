# AISE PMI SERVER

To build simply run make, feel free to fork this version for your work !


## Using PMIRUN

```
pmirun [PMI SERVER] [PROCESS] [CMD] [ARGS..]
```
- PMI SERVER: a field exported as PMI_SERVER in the env
- PROCESS: the number of processes to be launched
- CMD: the command
- ARGS: optionnal arguments

example:
```
pmirun 127.0.0.1:9000 16 ./a.out
```

You may change the PMI server to match your implementation.

## PMI Test

Used to validate that your PMI runtime works.

```
make
pmirun 127.0.0.1:9000 16 ./test_value
```

## PMI Perf

Used to validate the performance of your PMI runtime.
Only runs on two processes.

```
make
pmirun 127.0.0.1:9000 2 ./test_perf
```


