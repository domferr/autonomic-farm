# Autonomic Farm

## Usage

```
Options:
  -w arg                Number of workers (default: 4)
  -minw arg             Minimum number of workers (default: 2)
  -maxw arg             Maximum number of workers (default: 32)
  --stream arg          Number of input tasks in the stream (default: 300)
  --service arg         Service time values for tasks (space-separated) (default: 8 ms)
  --arrival arg         Arrival time values for tasks (space-separated) (default: 5 ms)
  --target arg          Target service time (default: None)
  --help                Show this usage
```

## How to build

```
cmake -H. -Bbuild
cd build
make
```

## How to run

The executables are:
- Monitored Farm: exe/farm
- Autonomic Farm: exe/autonomicfarm
- Monitored FastFlow Farm: exe/fffarm
- Autonomic FastFlow Farm: exe/ffautonomicfarm

```
exe/autonomicfarm -w 4 -minw 1 -maxw 4 --stream 500 --service 16 24 --arrival 8 4 8
```
## How to run and show plots

```
exe/autonomicfarm -w 4 -minw 1 -maxw 4 --stream 500 --service 16 24 --arrival 8 4 8; ipython3 -c "%run notebook/plot.ipynb"
```
> Note: Ensure you have activated python's virtual environment by doing `source notebook/.venv/bin/activate`