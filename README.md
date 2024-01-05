# Autonomic Farm

## Run

```
exe/autonomicfarm -w 4 -minw 1 -maxw 4 --target 8 --stream 500 --service 8 32 24 8 8 --arrival 2
```
## How to run and show plots

```
exe/autonomicfarm -w 4 -minw 1 -maxw 4 --target 8 --stream 500 --service 8 32 24 8 8 --arrival 2; ipython3 -c "%run notebook/plot.ipynb"
```
> Note: Ensure you have activated python's virtual environment by doing `source notebook/.venv/bin/activate`