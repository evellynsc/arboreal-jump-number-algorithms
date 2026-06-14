```
meson setup build
```

```
ninja -C build
```

## Generate config files
```
./ajns 0 dir algorithm time_limit memory_limit num_threads verbosity
```

## Run option to create config files
``` 
./build/ajns 1 config/teste.json
```

"/mnt/a/Users/evellynsc/Documents/workspace/instances/sop/s04.txt"
"/mnt/a/Users/evellynsc/Documents/workspace/instances/sop/esc07.sop.ajn.txt"
"/mnt/a/Users/evellynsc/Documents/workspace/instances/sop/esc11.sop.ajn.txt"
"/mnt/a/Users/evellynsc/Documents/workspace/instances/sop/esc12.sop.ajn.txt"
"/mnt/a/Users/evellynsc/Documents/workspace/instances/sop/esc25.sop.ajn.txt"
"/mnt/a/Users/evellyns'c/Documents/workspace/instances/sop/br17.12.sop.ajn.txt"

``` json
{
    "infile_name": "/mnt/a/Users/evellynsc/Documents/workspace/instances/sop/br17.12.sop.ajn.txt",
    "algo": {
        "type": "CHARACTERIZATION",
        "options": {
            "relaxed": false,
            "order": "ASC",
            "num_jumps": 3
        }
    },
    "solver": {
        "name": "CPLEX",
        "options": {
            "time_limit": 3600,
            "memory_limit": 20000,
            "num_threads": 4
        }
    },
    "outfile_name": ""
}
```