# Autonomic Farm

## How to run and show plots

```
exe/autonomicfarm -w 4 --stream 500 --service 16 24 5 8 32; ipython3 -c "%run notebook/plot.ipynb"
```
> Note: Ensure you have activated python's virtual environment by doing `source notebook/.venv/bin/activate`